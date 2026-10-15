#include "elm/ConsoleUI.h"
#include <iostream>
#include "elm/errors.h"
#include "elm/io.h"

using std::cout;

namespace elm {

template <class F>
void ConsoleUI::guarded(F&& action) {
    try { action(); }
    catch (const InputClosed&) { throw; }
    catch (const ElmError& e) { cout << "  ! " << e.what() << "\n"; }
}

void ConsoleUI::banner() const {
    io::heading("EMPLOYEE LEAVE MANAGEMENT SYSTEM  (IU2541230787 - CPP74)");
    cout << "  Today: " << sys_.today().str() << " (" << sys_.today().weekdayName() << ")   Employees: " << sys_.employees().size()
         << "   Managers: " << sys_.managers().size() << "   Pending requests: " << sys_.pending().size()
         << (sys_.dirty() ? "   [unsaved changes]" : "") << "\n";
}
void ConsoleUI::mainMenu() const {
    cout << "\n  1. Add Record              6. Leave Process (apply / approve / cancel)\n"
            "  2. Display Records         7. Reports\n"
            "  3. Search Record           8. Save / Load\n"
            "  4. Update Record           9. Exit\n"
            "  5. Delete Record\n";
}
void ConsoleUI::table(const std::string& header, const std::vector<std::string>& rows) const {
    if (rows.empty()) { cout << "  (no records)\n"; return; }
    cout << "  " << header << "\n  " << std::string(header.size() < 70 ? 70 : header.size(), '-') << "\n";
    for (const std::string& r : rows) cout << "  " << r << "\n";
    cout << "  " << rows.size() << " record(s)\n";
}
std::string ConsoleUI::askType() const {
    cout << "  Leave types:\n";
    for (int i = 0; i < sys_.policyCount(); ++i) cout << "    " << sys_.policyAt(i).describe() << "\n";
    for (;;) {
        std::string c = io::readLine("  Type code: ");
        try { return sys_.policy(c).code(); } catch (const NotFoundError& e) { cout << "  ! " << e.what() << "\n"; }
    }
}

// ----------------------------------------------------------------- run loop
int ConsoleUI::run() {
    try {
        sys_.load();
        if (!sys_.warnings().empty()) {
            cout << "  Data loaded with " << sys_.warnings().size() << " warning(s):\n";
            for (const std::string& w : sys_.warnings()) cout << "   - " << w << "\n";
        }
    } catch (const ElmError& e) { cout << "  ! Could not load saved data: " << e.what() << "\n  Starting empty.\n"; }
    try {
        for (;;) {
            banner(); mainMenu();
            int c = io::readInt("\n  Choose 1-9: ", 1, 9);
            guarded([&] {
                switch (c) {
                    case 1: addMenu(); break;     case 2: displayMenu(); break;  case 3: searchMenu(); break;
                    case 4: updateMenu(); break;  case 5: deleteMenu(); break;   case 6: processMenu(); break;
                    case 7: reportMenu(); break;  case 8: fileMenu(); break;     default: break;
                }
            });
            if (c == 9 && exitMenu()) return 0;
        }
    } catch (const InputClosed&) {
        cout << "\n  Input closed.";
        if (sys_.dirty()) { try { sys_.save(); cout << " Unsaved changes were auto-saved."; } catch (const ElmError&) {} }
        cout << "\n";
        return 0;
    }
}
bool ConsoleUI::exitMenu() {
    if (sys_.dirty() && io::confirm("  You have unsaved changes. Save before exit?")) {
        try { sys_.save(); cout << "  Saved.\n"; } catch (const ElmError& e) { cout << "  ! " << e.what() << "\n"; return false; }
    }
    cout << "  Goodbye!\n";
    return true;
}

// ----------------------------------------------------------------- 1 Add
void ConsoleUI::addMenu() {
    io::heading("ADD RECORD");
    cout << "  1. Employee   2. Manager   3. Public holiday   4. Leave request   0. Back\n";
    int c = io::readInt("  Choice: ", 0, 4);
    if (c == 1) {
        std::string n = io::readName("  Full name: "), d = io::readText("  Department: ", 25), g = io::readText("  Designation: ", 30);
        std::string m = io::readEmail("  E-mail: ");
        Date j = io::readDate("  Joining date");
        Employee& e = sys_.addEmployee(n, d, g, m, j);
        cout << "  Added employee " << e.id() << ".\n";
    } else if (c == 2) {
        std::string n = io::readName("  Full name: "), d = io::readText("  Department: ", 25), t = io::readText("  Title: ", 30);
        bool hr = io::confirm("  HR manager (can approve every department)?");
        cout << "  Added manager " << sys_.addManager(n, d, t, hr).id() << ".\n";
    } else if (c == 3) {
        Date d = io::readDate("  Holiday date");
        std::string hn = io::readText("  Holiday name: ", 40);
        cout << "  Added holiday " << sys_.addHoliday(d, hn).id() << ".\n";
    } else if (c == 4) applyForLeave();
}

// ----------------------------------------------------------------- 2 Display
void ConsoleUI::displayMenu() {
    io::heading("DISPLAY RECORDS");
    cout << "  1. Employees  2. Managers  3. Leave requests  4. Approvals  5. Holidays\n"
            "  6. Everything (polymorphic)  7. Leave policies  0. Back\n";
    int c = io::readInt("  Choice: ", 0, 7);
    std::vector<std::string> rows;
    if (c == 1) { for (int i = 0; i < sys_.employees().size(); ++i) rows.push_back(sys_.employees()[i]->row());
        table(io::pad("ID", 6) + io::pad("Name", 20) + io::pad("Department", 14) + io::pad("Designation", 20) + "Joined", rows); }
    else if (c == 2) { for (int i = 0; i < sys_.managers().size(); ++i) rows.push_back(sys_.managers()[i]->row());
        table(io::pad("ID", 6) + io::pad("Name", 20) + io::pad("Department", 14) + io::pad("Title", 20) + "Role", rows); }
    else if (c == 3) { for (int i = 0; i < sys_.requests().size(); ++i) rows.push_back(sys_.requests()[i]->row());
        table(io::pad("ID", 7) + io::pad("Emp", 6) + io::pad("Type", 5) + io::pad("From", 11) + io::pad("To", 11) + io::pad("Days", 5) + io::pad("Status", 10) + "Reason", rows); }
    else if (c == 4) { for (int i = 0; i < sys_.approvals().size(); ++i) rows.push_back(sys_.approvals()[i]->row());
        table(io::pad("ID", 7) + io::pad("Req", 7) + io::pad("Mgr", 6) + io::pad("Decision", 10) + io::pad("Date", 11) + "Comment", rows); }
    else if (c == 5) { for (int i = 0; i < sys_.holidays().size(); ++i) rows.push_back(sys_.holidays()[i]->row());
        table(io::pad("ID", 6) + io::pad("Date", 12) + io::pad("Day", 6) + "Name", rows); }
    else if (c == 6) {
        std::vector<const Record*> all = sys_.everything();     // one loop, five different display() bodies
        if (all.empty()) cout << "  (no records)\n";
        for (const Record* r : all) { cout << "  [" << r->kind() << "]\n"; r->display(cout); cout << "\n"; }
    } else if (c == 7) for (int i = 0; i < sys_.policyCount(); ++i) cout << "  " << sys_.policyAt(i).describe() << "\n";
}

// ----------------------------------------------------------------- 3 Search
void ConsoleUI::searchMenu() {
    io::heading("SEARCH");
    cout << "  1. Employee (id/name/dept/e-mail)   2. Manager   3. Leave request (id/employee/type/status/reason)\n"
            "  4. Requests overlapping a date range   5. Holiday   0. Back\n";
    int c = io::readInt("  Choice: ", 0, 5);
    if (c == 0) return;
    if (c == 4) {
        Date a = io::readDate("  From"), b = io::readDate("  To");
        if (b < a) throw ValidationError("end date is before start date");
        std::vector<std::string> rows;
        for (const LeaveRequest* r : sys_.requestsBetween(a, b)) rows.push_back(r->row());
        table("Requests overlapping " + a.str() + " .. " + b.str(), rows);
        return;
    }
    std::string q = io::readText("  Search text: ", 40);
    std::vector<std::string> rows;
    if (c == 1) for (const Employee* e : sys_.searchEmployees(q)) rows.push_back(e->row());
    if (c == 2) for (const Manager* m : sys_.searchManagers(q)) rows.push_back(m->row());
    if (c == 3) for (const LeaveRequest* r : sys_.searchRequests(q)) rows.push_back(r->row());
    if (c == 5) for (const Holiday* h : sys_.searchHolidays(q)) rows.push_back(h->row());
    table("Results for '" + q + "'", rows);
}

// ----------------------------------------------------------------- 4 Update
void ConsoleUI::updateMenu() {
    io::heading("UPDATE RECORD   (press Enter to keep the current value)");
    cout << "  1. Employee   2. Manager   3. Pending leave request   4. Holiday   0. Back\n";
    int c = io::readInt("  Choice: ", 0, 4);
    if (c == 1) {
        Employee& e = sys_.employee(io::readLine("  Employee ID: "));
        e.display(cout);
        auto keep = [](const std::string& v, const std::string& cur) { return v.empty() ? cur : v; };
        std::string n = keep(io::readText("  New name: ", 40, true), e.name()), d = keep(io::readText("  New department: ", 25, true), e.department());
        std::string g = keep(io::readText("  New designation: ", 30, true), e.designation()), m = keep(io::readText("  New e-mail: ", 60, true), e.email());
        sys_.updateEmployee(e.id(), n, d, g, m);
        cout << "  Updated " << e.id() << ".\n";
    } else if (c == 2) {
        Manager& m = sys_.manager(io::readLine("  Manager ID: "));
        m.display(cout);
        auto keep = [](const std::string& v, const std::string& cur) { return v.empty() ? cur : v; };
        std::string n = keep(io::readText("  New name: ", 40, true), m.name()), d = keep(io::readText("  New department: ", 25, true), m.department());
        std::string t = keep(io::readText("  New title: ", 30, true), m.title());
        sys_.updateManager(m.id(), n, d, t, io::confirm("  HR manager?"));
        cout << "  Updated " << m.id() << ".\n";
    } else if (c == 3) {
        LeaveRequest& r = sys_.request(io::readLine("  Request ID: "));
        r.display(cout);
        Date f = io::readDate("  New start"), t = io::readDate("  New end");
        std::string why = io::readText("  Reason (Enter = keep): ", 60, true);
        sys_.updateRequest(r.id(), f, t, why.empty() ? r.reason() : why);
        cout << "  Updated " << r.id() << " - now " << r.days() << " working day(s).\n";
    } else if (c == 4) {
        Holiday& h = sys_.holiday(io::readLine("  Holiday ID: "));
        h.display(cout);
        Date d = io::readDate("  New date");
        std::string n = io::readText("  New name (Enter = keep): ", 40, true);
        sys_.updateHoliday(h.id(), d, n.empty() ? h.name() : n);
        cout << "  Updated " << h.id() << ".\n";
    }
}

// ----------------------------------------------------------------- 5 Delete
void ConsoleUI::deleteMenu() {
    io::heading("DELETE RECORD");
    cout << "  1. Employee (and finished leave history)   2. Manager   3. Holiday   0. Back\n";
    int c = io::readInt("  Choice: ", 0, 3);
    if (c == 0) return;
    std::string id = io::readLine("  ID to delete: ");
    if (c == 1) { Employee& e = sys_.employee(id); e.display(cout);
        if (io::confirm("  Delete this employee permanently?")) { int n = sys_.deleteEmployee(id); cout << "  Deleted. " << n << " history request(s) removed.\n"; } }
    else if (c == 2) { Manager& m = sys_.manager(id); m.display(cout);
        if (io::confirm("  Delete this manager permanently?")) { sys_.deleteManager(id); cout << "  Deleted.\n"; } }
    else { Holiday& h = sys_.holiday(id); h.display(cout);
        if (io::confirm("  Delete this holiday?")) { sys_.deleteHoliday(id); cout << "  Deleted.\n"; } }
}

// ----------------------------------------------------------------- 6 Process
void ConsoleUI::applyForLeave() {
    std::string eid = io::readLine("  Employee ID: ");
    Employee& e = sys_.employee(eid);
    cout << "  Applying for: " << e.name() << " (" << e.department() << ")\n";
    std::string type = askType();
    Date f = io::readDate("  From"), t = io::readDate("  To");
    std::string why = io::readText("  Reason: ", 60);
    LeaveRequest& r = sys_.applyLeave(e.id(), type, f, t, why);
    cout << "  Request " << r.id() << " submitted: " << r.days() << " working day(s), status Pending.\n";
    Balance b = sys_.balance(e.id(), type, f.year());
    if (!b.unlimited) cout << "  Balance after pending requests: " << b.remaining << " of " << b.quota << " day(s).\n";
}
void ConsoleUI::reviewQueue() {
    std::vector<const LeaveRequest*> q = sys_.pending();
    std::vector<std::string> rows;
    for (const LeaveRequest* r : q) rows.push_back(r->row() + "  [" + sys_.employeeName(r->employeeId()) + "]");
    table(io::pad("ID", 7) + io::pad("Emp", 6) + io::pad("Type", 5) + io::pad("From", 11) + io::pad("To", 11) + io::pad("Days", 5) + io::pad("Status", 10) + "Reason", rows);
    if (q.empty()) return;
    std::string rid = io::readLine("  Request ID to decide (Enter = back): ");
    if (rid.empty()) return;
    LeaveRequest& r = sys_.request(rid);
    r.display(cout);
    std::string mid = io::readLine("  Your manager ID: ");
    sys_.manager(mid);
    cout << "  1. Approve   2. Reject\n";
    bool ok = io::readInt("  Decision: ", 1, 2) == 1;
    std::string note = io::readText(ok ? "  Comment (optional): " : "  Reason for rejection: ", 60, ok);
    Approval& a = sys_.decide(r.id(), mid, ok, note);
    cout << "  " << r.id() << " " << statusName(r.status()) << " (approval " << a.id() << ").\n";
}
void ConsoleUI::cancelRequest() {
    LeaveRequest& r = sys_.request(io::readLine("  Request ID to cancel: "));
    r.display(cout);
    if (io::confirm("  Cancel this request?")) { sys_.cancelLeave(r.id()); cout << "  Cancelled. Balance restored.\n"; }
}
void ConsoleUI::showBalance() {
    std::string id = io::readLine("  Employee ID: ");
    sys_.balanceReport(id, sys_.today().year(), cout);
}
void ConsoleUI::processMenu() {
    io::heading("LEAVE PROCESS");
    cout << "  1. Apply for leave   2. Review pending requests (approve / reject)\n"
            "  3. Cancel a request  4. Check leave balance   0. Back\n";
    int c = io::readInt("  Choice: ", 0, 4);
    if (c == 1) applyForLeave(); else if (c == 2) reviewQueue(); else if (c == 3) cancelRequest(); else if (c == 4) showBalance();
}

// ----------------------------------------------------------------- 7 Reports
void ConsoleUI::reportMenu() {
    io::heading("REPORTS");
    cout << "  1. Employee leave balance   2. Department summary   3. Who is on leave on a date\n"
            "  4. Monthly leave calendar   5. Overall summary      6. Export all requests to CSV\n"
            "  7. Audit log (last 15)      0. Back\n";
    int c = io::readInt("  Choice: ", 0, 7);
    int y = sys_.today().year();
    if (c == 1) { std::string id = io::readLine("  Employee ID: "); sys_.balanceReport(id, io::readInt("  Year: ", 2000, 2100), cout); }
    else if (c == 2) sys_.departmentReport(y, cout);
    else if (c == 3) sys_.onLeaveReport(io::readDate("  Date"), cout);
    else if (c == 4) { int yr = io::readInt("  Year: ", 2000, 2100); sys_.monthCalendar(yr, io::readInt("  Month (1-12): ", 1, 12), cout); }
    else if (c == 5) sys_.summaryReport(y, cout);
    else if (c == 6) { std::string p = io::readText("  File name (Enter = leave_report.csv): ", 60, true);
        if (p.empty()) p = "leave_report.csv";
        sys_.exportCsv(p); cout << "  Exported to " << p << "\n"; }
    else if (c == 7) sys_.auditTail(15, cout);
}

// ----------------------------------------------------------------- 8 Save / Load
void ConsoleUI::fileMenu() {
    io::heading("SAVE / LOAD");
    cout << "  1. Save now   2. Reload from disk (discard unsaved)   3. Backup copy\n"
            "  4. Load demo data (replaces memory)   5. Clear all data (memory only)   0. Back\n";
    int c = io::readInt("  Choice: ", 0, 5);
    if (c == 1) { sys_.save(); cout << "  Saved " << sys_.everything().size() << " record(s).\n"; }
    else if (c == 2) { if (io::confirm("  Discard unsaved changes and reload?")) { sys_.load(); cout << "  Reloaded " << sys_.everything().size() << " record(s), "
                       << sys_.warnings().size() << " warning(s).\n"; for (const std::string& w : sys_.warnings()) cout << "   - " << w << "\n"; } }
    else if (c == 3) cout << "  Backup written to " << sys_.backup() << "\n";
    else if (c == 4) { if (io::confirm("  Replace everything in memory with demo data?")) { sys_.loadDemoData(); cout << "  Demo data loaded (" << sys_.everything().size() << " records). Use Save to keep it.\n"; } }
    else if (c == 5) { if (io::confirm("  Clear all records from memory?")) { sys_.clearAll(); cout << "  Cleared. Save to make it permanent.\n"; } }
}

}  // namespace elm

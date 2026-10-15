// unit_tests.cpp - white-box checks of every class and rule. Built with ASan + UBSan (also finds leaks).
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>
#include "elm/ConsoleUI.h"
#include "elm/LeaveSystem.h"
#include "elm/io.h"

using namespace elm;
static int passed = 0, failed = 0;
#define CHECK(name, cond) do { if (cond) { ++passed; std::printf("  [PASS] %s\n", name); } else { ++failed; std::printf("  [FAIL] %s  (line %d)\n", name, __LINE__); } } while (0)
#define THROWS(name, ExType, expr) do { bool ok_ = false; try { expr; } catch (const ExType&) { ok_ = true; } catch (...) {} CHECK(name, ok_); } while (0)

static const char* DIR = "Tests/_work/unit";

static void freshDir() { std::filesystem::remove_all(DIR); }

struct Fixture {                                    // small company, today = Thu 2026-10-15
    LeaveSystem s;
    Fixture() : s(DIR) {
        s.setToday(Date(2026, 10, 15));
        s.addManager("Anita Desai", "Engineering", "Head", false);
        s.addManager("Priya Nair", "HR", "Director", true);
        s.addEmployee("Aarav Sharma", "Engineering", "Engineer", "aarav@x.com", Date(2020, 1, 6));
        s.addEmployee("Diya Patel", "Engineering", "Tester", "diya@x.com", Date(2021, 1, 4));
        s.addEmployee("Kabir Shah", "Engineering", "DevOps", "kabir@x.com", Date(2022, 1, 3));
        s.addEmployee("Neha Gupta", "HR", "Exec", "neha@x.com", Date(2023, 1, 2));
    }
};

int main() {
    freshDir();
    std::puts("== Date ==");
    CHECK("leap years 2024/2000 yes, 1900/2026 no", Date::isLeap(2024) && Date::isLeap(2000) && !Date::isLeap(1900) && !Date::isLeap(2026));
    CHECK("2026-10-15 is a Thursday", Date(2026, 10, 15).weekday() == 4);
    CHECK("2026-10-17 is a weekend", Date(2026, 10, 17).isWeekend() && !Date(2026, 10, 16).isWeekend());
    CHECK("serial round trip for 3000 consecutive days", [] { for (long z = 0; z < 40000; z += 13) if (Date::fromSerial(z).serial() != z) return false; return true; }());
    CHECK("addDays crosses month and year end", Date(2026, 12, 30).addDays(3) == Date(2027, 1, 2));
    CHECK("date difference", Date(2026, 3, 1) - Date(2026, 2, 1) == 28);
    THROWS("2026-02-29 rejected", ValidationError, Date(2026, 2, 29));
    THROWS("month 13 rejected", ValidationError, Date(2026, 13, 1));
    Date tmpd; CHECK("parse good / bad strings", Date::parse("2026-02-28").day() == 28 && !Date::tryParse("2026-2-8", tmpd));
    std::puts("== PtrList / dynamic memory ==");
    {
        PtrList<Holiday> l(2);
        for (int i = 0; i < 20; ++i) l.add(new Holiday("H" + std::to_string(i), Date(2026, 1, 1).addDays(i), "x"));
        CHECK("list grows past initial capacity (2 -> 32)", l.size() == 20 && l.capacity() == 32);
        CHECK("removeAt deletes and shifts", l.removeAt(0) && l.size() == 19 && l[0]->id() == "H1");
        THROWS("operator[] bounds checked", ElmError, l[99]);
        PtrList<Holiday> moved(std::move(l));
        CHECK("move transfers ownership", moved.size() == 19 && l.size() == 0);
    }
    std::puts("== helpers / validation ==");
    CHECK("e-mail validator", io::validEmail("a@b.com") && !io::validEmail("a@b") && !io::validEmail("@b.com") && !io::validEmail("a b@c.com"));
    CHECK("name validator", io::validName("Anne-Marie O'Neil") && !io::validName("R2D2") && !io::validName("A"));
    CHECK("escape/unescape round trip", io::unescape(io::escape("a\tb\nc\\d")) == "a\tb\nc\\d");
    {
        Fixture f;
        THROWS("employee: bad e-mail", ValidationError, f.s.addEmployee("Test User", "HR", "x", "bad", Date(2020, 1, 1)));
        THROWS("employee: bad name", ValidationError, f.s.addEmployee("T3st", "HR", "x", "t@x.com", Date(2020, 1, 1)));
        THROWS("employee: duplicate e-mail", RuleError, f.s.addEmployee("Test User", "HR", "x", "aarav@x.com", Date(2020, 1, 1)));
        THROWS("employee: joining date in future", RuleError, f.s.addEmployee("Test User", "HR", "x", "t@x.com", Date(2027, 1, 1)));
        CHECK("ids are sequential E005", f.s.addEmployee("New Hire", "HR", "Exec", "new@x.com", Date(2026, 1, 1)).id() == "E005");
        CHECK("names are title-cased", f.s.addEmployee("rahul   ", "HR", "Exec", "r@x.com", Date(2026, 1, 1)).name() == "Rahul");
    }
    std::puts("== OOP: inheritance + runtime polymorphism ==");
    {
        Fixture f;
        const Record* r = f.s.findEmployee("E001"); const Person* p = f.s.findManager("M002");
        CHECK("Record* reports dynamic kind", r->kind() == "Employee" && f.s.findManager("M001")->kind() == "Manager");
        CHECK("Person::role() is virtual", p->role() == "HR Manager" && f.s.findManager("M001")->role() == "Manager" && f.s.employee("E001").role() == "Employee");
        std::ostringstream os;
        for (const Record* x : f.s.everything()) x->display(os);
        CHECK("display() through Record* works for all kinds", os.str().find("Manager ID") != std::string::npos && os.str().find("Employee ID") != std::string::npos);
        LeavePolicy* pol[5]; for (int i = 0; i < 5; ++i) pol[i] = makePolicy(kPolicyCodes[i]);
        CHECK("policies answer differently through base pointer", pol[0]->yearlyQuota() == 18 && pol[1]->maxBackdateDays() == 7 && pol[2]->maxConsecutive() == 3 && pol[3]->minNoticeDays() == 14 && !pol[4]->paid());
        for (int i = 0; i < 5; ++i) delete pol[i];                    // virtual destructor path
        CHECK("unknown policy code -> nullptr", makePolicy("XX") == nullptr);
    }
    std::puts("== business rules ==");
    {
        Fixture f;
        LeaveRequest& a = f.s.applyLeave("E001", "AL", Date(2026, 10, 19), Date(2026, 10, 23), "Trip");
        CHECK("apply: 5 working days, Pending, id R0001", a.days() == 5 && a.status() == Status::Pending && a.id() == "R0001");
        CHECK("working days skip weekend", f.s.applyLeave("E002", "AL", Date(2026, 10, 22), Date(2026, 10, 27), "Trip").days() == 4);
        f.s.addHoliday(Date(2026, 11, 4), "Test Holiday");
        CHECK("working days skip public holiday", f.s.applyLeave("E004", "AL", Date(2026, 11, 3), Date(2026, 11, 5), "Trip").days() == 2);
        THROWS("overlap with own request rejected", RuleError, f.s.applyLeave("E001", "SL", Date(2026, 10, 21), Date(2026, 10, 21), "x"));
        THROWS("end before start rejected", RuleError, f.s.applyLeave("E003", "AL", Date(2026, 11, 10), Date(2026, 11, 9), "x"));
        THROWS("weekend-only range rejected", RuleError, f.s.applyLeave("E003", "SL", Date(2026, 10, 17), Date(2026, 10, 18), "x"));
        THROWS("annual leave needs 3 days notice", RuleError, f.s.applyLeave("E003", "AL", Date(2026, 10, 16), Date(2026, 10, 16), "x"));
        THROWS("casual leave max 3 days", RuleError, f.s.applyLeave("E003", "CL", Date(2026, 11, 9), Date(2026, 11, 13), "x"));
        THROWS("sick leave cannot be 8+ days back", RuleError, f.s.applyLeave("E003", "SL", Date(2026, 10, 1), Date(2026, 10, 1), "x"));
        CHECK("sick leave may be back-dated 3 days", f.s.applyLeave("E003", "SL", Date(2026, 10, 12), Date(2026, 10, 12), "Fever").days() == 1);
        THROWS("range spanning two years rejected", RuleError, f.s.applyLeave("E003", "AL", Date(2026, 12, 29), Date(2027, 1, 5), "x"));
        THROWS("unknown employee", NotFoundError, f.s.applyLeave("E099", "AL", Date(2026, 11, 9), Date(2026, 11, 9), "x"));
        THROWS("unknown leave type", NotFoundError, f.s.applyLeave("E003", "ZZ", Date(2026, 11, 9), Date(2026, 11, 9), "x"));
        THROWS("empty reason", ValidationError, f.s.applyLeave("E003", "AL", Date(2026, 11, 9), Date(2026, 11, 9), "  "));
        THROWS("leave before joining date", RuleError, f.s.applyLeave("E004", "SL", Date(2026, 10, 14), Date(2022, 1, 1), "x"));
        Balance b = f.s.balance("E001", "AL", 2026);
        CHECK("balance counts pending: 18 - 5 = 13", b.pending == 5 && b.remaining == 13 && b.used == 0);
        f.s.applyLeave("E001", "AL", Date(2026, 11, 2), Date(2026, 11, 13), "Long");   // 9 days (one holiday) -> 4 left
        THROWS("insufficient balance rejected", RuleError, f.s.applyLeave("E001", "AL", Date(2026, 12, 1), Date(2026, 12, 7), "x"));
        CHECK("unlimited unpaid leave has no balance cap", f.s.balance("E001", "UL", 2026).unlimited);
        f.s.updateRequest("R0001", Date(2026, 10, 19), Date(2026, 10, 21), "Shorter");
        CHECK("update pending request recalculates days", f.s.request("R0001").days() == 3 && f.s.balance("E001", "AL", 2026).pending == 12);
        THROWS("update that would overlap is refused", RuleError, f.s.updateRequest("R0001", Date(2026, 11, 2), Date(2026, 11, 3), "x"));
    }
    std::puts("== approval workflow ==");
    {
        Fixture f;
        f.s.applyLeave("E001", "AL", Date(2026, 10, 19), Date(2026, 10, 21), "A");
        f.s.applyLeave("E002", "AL", Date(2026, 10, 19), Date(2026, 10, 21), "B");
        f.s.applyLeave("E003", "AL", Date(2026, 10, 20), Date(2026, 10, 20), "C");
        f.s.applyLeave("E004", "AL", Date(2026, 10, 19), Date(2026, 10, 19), "D");
        THROWS("manager of another department cannot decide", RuleError, f.s.decide("R0004", "M001", true, ""));
        CHECK("HR manager can approve any department", f.s.decide("R0004", "M002", true, "ok").decision() == Decision::Approved);
        CHECK("approve sets status", (f.s.decide("R0001", "M001", true, "ok"), f.s.request("R0001").status() == Status::Approved));
        THROWS("cover rule: max half of department away", RuleError, (f.s.decide("R0002", "M001", true, ""), f.s.decide("R0003", "M001", true, "")));
        THROWS("reject requires a reason", ValidationError, f.s.decide("R0003", "M001", false, ""));
        CHECK("request stays pending after failed decision", f.s.request("R0003").status() == Status::Pending);
        THROWS("cannot decide twice", RuleError, f.s.decide("R0001", "M001", false, "no"));
        CHECK("rejection recorded", f.s.decide("R0003", "M001", false, "Busy").decision() == Decision::Rejected && f.s.request("R0003").status() == Status::Rejected);
        CHECK("approved days move to 'used'", f.s.balance("E001", "AL", 2026).used == 3);
        f.s.cancelLeave("R0001");
        CHECK("cancel restores balance", f.s.balance("E001", "AL", 2026).remaining == 18);
        THROWS("cancelled request cannot be cancelled again", RuleError, f.s.cancelLeave("R0001"));
        f.s.setToday(Date(2026, 10, 20));
        THROWS("started approved leave cannot be cancelled", RuleError, f.s.cancelLeave("R0004") );
        f.s.setToday(Date(2026, 10, 15));
        THROWS("approval already decided cannot flip", RuleError, f.s.approvals()[0]->approve("x", Date(2026, 1, 1)));
    }
    std::puts("== update / delete / search ==");
    {
        Fixture f;
        f.s.updateEmployee("E001", "aarav kumar", "Engineering", "Lead", "aarav.k@x.com");
        CHECK("update employee", f.s.employee("E001").name() == "Aarav Kumar" && f.s.employee("E001").email() == "aarav.k@x.com");
        THROWS("update to duplicate e-mail refused", RuleError, f.s.updateEmployee("E001", "Aarav Kumar", "Engineering", "Lead", "diya@x.com"));
        CHECK("search by name part, case-insensitive", f.s.searchEmployees("KUMAR").size() == 1 && f.s.searchEmployees("engineering").size() == 3);
        CHECK("search miss returns empty", f.s.searchEmployees("zzzz").empty());
        f.s.applyLeave("E002", "AL", Date(2026, 10, 19), Date(2026, 10, 19), "A");
        THROWS("cannot delete employee with pending leave", RuleError, f.s.deleteEmployee("E002"));
        f.s.cancelLeave("R0001");
        CHECK("delete after cancel removes history", f.s.deleteEmployee("E002") == 1 && f.s.findEmployee("E002") == nullptr && f.s.requests().size() == 0);
        f.s.applyLeave("E001", "AL", Date(2026, 10, 19), Date(2026, 10, 19), "A");
        f.s.decide("R0001", "M001", true, "");
        THROWS("manager with decisions cannot be deleted", RuleError, f.s.deleteManager("M001"));
        f.s.deleteManager("M002");
        CHECK("manager without decisions deletable", f.s.findManager("M002") == nullptr);
        THROWS("delete unknown holiday", NotFoundError, f.s.deleteHoliday("H404"));
        CHECK("requestsBetween finds overlaps", f.s.requestsBetween(Date(2026, 10, 18), Date(2026, 10, 20)).size() == 1 && f.s.requestsBetween(Date(2026, 11, 1), Date(2026, 11, 2)).empty());
    }
    std::puts("== reports ==");
    {
        Fixture f;
        f.s.loadDemoData();
        std::ostringstream a, b, c, d, e;
        f.s.summaryReport(2026, a); f.s.departmentReport(2026, b); f.s.monthCalendar(2026, 10, c); f.s.onLeaveReport(Date(2026, 10, 15), d); f.s.balanceReport("E001", 2026, e);
        CHECK("summary mentions approval rate", a.str().find("Approval rate") != std::string::npos);
        CHECK("department report lists Engineering", b.str().find("Engineering") != std::string::npos);
        CHECK("calendar has one row per employee", c.str().find("Aarav Sharma") != std::string::npos && c.str().find("Neha Gupta") != std::string::npos);
        CHECK("balance report lists all 5 types", e.str().find("AL Annual") != std::string::npos && e.str().find("UL Unpaid") != std::string::npos);
        f.s.exportCsv("Tests/_work/unit_export.csv");
        std::ifstream in("Tests/_work/unit_export.csv"); std::string hdr; std::getline(in, hdr);
        CHECK("CSV export has header + rows", hdr.rfind("RequestID,", 0) == 0 && in.good());
        THROWS("month 13 rejected", ValidationError, f.s.monthCalendar(2026, 13, c));
    }
    std::puts("== file handling ==");
    {
        freshDir();
        { Fixture f; f.s.loadDemoData(); f.s.save(); }
        LeaveSystem s2(DIR); s2.load();
        CHECK("reload restores every record", s2.employees().size() == 8 && s2.managers().size() == 3 && s2.requests().size() == 10 && s2.holidays().size() == 10 && s2.approvals().size() == 6);
        CHECK("no warnings on a clean file", s2.warnings().empty());
        CHECK("statuses survive a round trip", s2.request("R0009").status() == Status::Cancelled && s2.request("R0004").status() == Status::Rejected);
        CHECK("id counter continues after reload", s2.addEmployee("Zed Zed", "HR", "Exec", "zed@x.com", Date(2026, 1, 1)).id() == "E009");
        { std::ofstream o(std::string(DIR) + "/employees.dat", std::ios::app); o << "garbage line\nE077\tBad\n"; }
        LeaveSystem s3(DIR); s3.load();
        CHECK("corrupt lines skipped with warnings, rest loaded", s3.employees().size() == 8 && s3.warnings().size() >= 2);
        { std::ofstream o(std::string(DIR) + "/holidays.dat", std::ios::trunc); o << "not a header\n"; }
        LeaveSystem s4(DIR);
        THROWS("bad header file -> FileError, memory untouched", FileError, s4.load());
        std::filesystem::remove(std::string(DIR) + "/holidays.dat");
        LeaveSystem s5(DIR); s5.load();
        CHECK("missing file is treated as empty (first run)", s5.holidays().size() == 0 && s5.employees().size() == 8);
        CHECK("backup copies data files", std::filesystem::exists(s5.backup()));
    }
    std::printf("\n%d passed, %d failed\n", passed, failed);
    return failed == 0 ? 0 : 1;
}

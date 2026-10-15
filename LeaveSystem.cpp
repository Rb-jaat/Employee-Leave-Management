#include "elm/LeaveSystem.h"
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <map>
#include <memory>
#include <ostream>
#include "elm/errors.h"
#include "elm/io.h"

namespace elm {

static std::string up(const std::string& s) {
    std::string r = io::trim(s);
    for (char& c : r) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    return r;
}

// ---------------------------------------------------------------- construction
LeaveSystem::LeaveSystem(const std::string& dataDir)
    : fm_(dataDir), audit_(dataDir + "/audit.log"), policies_(new LeavePolicy*[5]), today_(Date::today()), dirty_(false) {
    for (int i = 0; i < 5; ++i) policies_[i] = makePolicy(kPolicyCodes[i]);   // each policy is heap-allocated
}
LeaveSystem::~LeaveSystem() {
    for (int i = 0; i < 5; ++i) delete policies_[i];
    delete[] policies_;                                                       // PtrList members free themselves
}

void LeaveSystem::touch(const std::string& action, const std::string& detail) {
    dirty_ = true;
    try { fm_.ensureDir(); audit_.log(today_.str(), action, detail); } catch (const ElmError&) {}
}

template <class T>
std::string LeaveSystem::nextId(const PtrList<T>& l, char prefix, int width) const {
    int mx = 0;
    for (int i = 0; i < l.size(); ++i) {
        const std::string& id = l[i]->id();
        try { mx = std::max(mx, std::stoi(id.substr(1))); } catch (const std::exception&) {}
    }
    std::string n = std::to_string(mx + 1);
    return std::string(1, prefix) + std::string(static_cast<size_t>(std::max(0, width - static_cast<int>(n.size()))), '0') + n;
}

// ---------------------------------------------------------------- lookups
#define ELM_INDEX(NAME, LIST)                                                    \
    int LeaveSystem::NAME(const std::string& raw) const {                        \
        std::string id = up(raw);                                                \
        for (int i = 0; i < LIST.size(); ++i) if (LIST[i]->id() == id) return i; \
        return -1;                                                               \
    }
ELM_INDEX(indexOfEmployee, employees_)
ELM_INDEX(indexOfManager, managers_)
ELM_INDEX(indexOfRequest, requests_)
ELM_INDEX(indexOfHoliday, holidays_)
#undef ELM_INDEX

Employee* LeaveSystem::findEmployee(const std::string& id) const { int i = indexOfEmployee(id); return i < 0 ? nullptr : employees_[i]; }
Manager* LeaveSystem::findManager(const std::string& id) const { int i = indexOfManager(id); return i < 0 ? nullptr : managers_[i]; }
LeaveRequest* LeaveSystem::findRequest(const std::string& id) const { int i = indexOfRequest(id); return i < 0 ? nullptr : requests_[i]; }
Holiday* LeaveSystem::findHoliday(const std::string& id) const { int i = indexOfHoliday(id); return i < 0 ? nullptr : holidays_[i]; }
Employee& LeaveSystem::employee(const std::string& id) const {
    Employee* p = findEmployee(id);
    if (!p) throw NotFoundError("no employee with id '" + id + "'");
    return *p;
}
Manager& LeaveSystem::manager(const std::string& id) const {
    Manager* p = findManager(id);
    if (!p) throw NotFoundError("no manager with id '" + id + "'");
    return *p;
}
LeaveRequest& LeaveSystem::request(const std::string& id) const {
    LeaveRequest* p = findRequest(id);
    if (!p) throw NotFoundError("no leave request with id '" + id + "'");
    return *p;
}
Holiday& LeaveSystem::holiday(const std::string& id) const {
    Holiday* p = findHoliday(id);
    if (!p) throw NotFoundError("no holiday with id '" + id + "'");
    return *p;
}
const LeavePolicy& LeaveSystem::policy(const std::string& code) const {
    std::string c = up(code);
    for (int i = 0; i < 5; ++i) if (policies_[i]->code() == c) return *policies_[i];
    throw NotFoundError("unknown leave type '" + code + "' (use AL, SL, CL, PL or UL)");
}
std::string LeaveSystem::employeeName(const std::string& id) const {
    Employee* e = findEmployee(id);
    return e ? e->name() : "(deleted)";
}
bool LeaveSystem::isHoliday(const Date& d) const {
    for (int i = 0; i < holidays_.size(); ++i) if (holidays_[i]->date() == d) return true;
    return false;
}
std::vector<const Record*> LeaveSystem::everything() const {
    std::vector<const Record*> v;
    for (int i = 0; i < employees_.size(); ++i) v.push_back(employees_[i]);
    for (int i = 0; i < managers_.size(); ++i) v.push_back(managers_[i]);
    for (int i = 0; i < requests_.size(); ++i) v.push_back(requests_[i]);
    for (int i = 0; i < approvals_.size(); ++i) v.push_back(approvals_[i]);
    for (int i = 0; i < holidays_.size(); ++i) v.push_back(holidays_[i]);
    return v;
}

// ---------------------------------------------------------------- 1. add
Employee& LeaveSystem::addEmployee(const std::string& name, const std::string& dept, const std::string& designation,
                                   const std::string& email, const Date& joined) {
    if (joined > today_) throw RuleError("joining date cannot be in the future");
    std::unique_ptr<Employee> e(new Employee(nextId(employees_, 'E', 3), name, dept, designation, email, joined));
    for (int i = 0; i < employees_.size(); ++i)
        if (employees_[i]->email() == e->email()) throw RuleError("e-mail " + e->email() + " already belongs to " + employees_[i]->id());
    Employee& ref = *e;
    employees_.add(e.release());
    touch("ADD_EMPLOYEE", ref.id() + " " + ref.name());
    return ref;
}
Manager& LeaveSystem::addManager(const std::string& name, const std::string& dept, const std::string& title, bool hr) {
    std::unique_ptr<Manager> m(new Manager(nextId(managers_, 'M', 3), name, dept, title, hr));
    Manager& ref = *m;
    managers_.add(m.release());
    touch("ADD_MANAGER", ref.id() + " " + ref.name());
    return ref;
}
Holiday& LeaveSystem::addHoliday(const Date& d, const std::string& name) {
    for (int i = 0; i < holidays_.size(); ++i)
        if (holidays_[i]->date() == d) throw RuleError(d.str() + " is already a holiday (" + holidays_[i]->name() + ")");
    std::unique_ptr<Holiday> h(new Holiday(nextId(holidays_, 'H', 3), d, name));
    Holiday& ref = *h;
    holidays_.add(h.release());
    touch("ADD_HOLIDAY", ref.id() + " " + d.str());
    return ref;
}

// ---------------------------------------------------------------- 3. search
std::vector<const Employee*> LeaveSystem::searchEmployees(const std::string& t) const {
    std::vector<const Employee*> v;
    for (int i = 0; i < employees_.size(); ++i)
        if (employees_[i]->matches(t) || io::containsCI(employees_[i]->designation(), t) || io::containsCI(employees_[i]->email(), t))
            v.push_back(employees_[i]);
    return v;
}
std::vector<const Manager*> LeaveSystem::searchManagers(const std::string& t) const {
    std::vector<const Manager*> v;
    for (int i = 0; i < managers_.size(); ++i) if (managers_[i]->matches(t)) v.push_back(managers_[i]);
    return v;
}
std::vector<const LeaveRequest*> LeaveSystem::searchRequests(const std::string& t) const {
    std::vector<const LeaveRequest*> v;
    for (int i = 0; i < requests_.size(); ++i)
        if (requests_[i]->matches(t) || io::containsCI(employeeName(requests_[i]->employeeId()), t)) v.push_back(requests_[i]);
    return v;
}
std::vector<const LeaveRequest*> LeaveSystem::requestsBetween(const Date& a, const Date& b) const {
    std::vector<const LeaveRequest*> v;
    for (int i = 0; i < requests_.size(); ++i) if (requests_[i]->overlaps(a, b)) v.push_back(requests_[i]);
    return v;
}
std::vector<const Holiday*> LeaveSystem::searchHolidays(const std::string& t) const {
    std::vector<const Holiday*> v;
    for (int i = 0; i < holidays_.size(); ++i) if (holidays_[i]->matches(t)) v.push_back(holidays_[i]);
    return v;
}

// ---------------------------------------------------------------- 4. update
void LeaveSystem::updateEmployee(const std::string& id, const std::string& name, const std::string& dept,
                                 const std::string& designation, const std::string& email) {
    Employee& e = employee(id);
    Employee probe(e.id(), name, dept, designation, email, e.joined());      // validates everything first
    for (int i = 0; i < employees_.size(); ++i)
        if (employees_[i] != &e && employees_[i]->email() == probe.email())
            throw RuleError("e-mail " + probe.email() + " already belongs to " + employees_[i]->id());
    e.setName(name); e.setDepartment(dept); e.setDesignation(designation); e.setEmail(email);
    touch("UPDATE_EMPLOYEE", e.id());
}
void LeaveSystem::updateManager(const std::string& id, const std::string& name, const std::string& dept,
                                const std::string& title, bool hr) {
    Manager& m = manager(id);
    Manager probe(m.id(), name, dept, title, hr);
    m.setName(name); m.setDepartment(dept); m.setTitle(title); m.setHR(hr);
    touch("UPDATE_MANAGER", m.id());
}
void LeaveSystem::updateHoliday(const std::string& id, const Date& d, const std::string& name) {
    Holiday& h = holiday(id);
    for (int i = 0; i < holidays_.size(); ++i)
        if (holidays_[i] != &h && holidays_[i]->date() == d) throw RuleError(d.str() + " is already a holiday");
    h.setName(name); h.setDate(d);
    touch("UPDATE_HOLIDAY", h.id());
}
void LeaveSystem::updateRequest(const std::string& id, const Date& from, const Date& to, const std::string& reason) {
    LeaveRequest& r = request(id);
    if (r.status() != Status::Pending) throw RuleError("only a Pending request can be edited (this one is " + std::string(statusName(r.status())) + ")");
    std::string why = io::trim(reason);
    if (why.empty() || why.size() > 60) throw ValidationError("reason must be 1-60 characters");
    if (to < from) throw RuleError("end date is before start date");
    if (from.year() != to.year()) throw RuleError("a request cannot span two calendar years; split it in two");
    if (from < employee(r.employeeId()).joined()) throw RuleError("leave cannot start before the joining date");
    const LeavePolicy& p = policy(r.type());
    int days = LeaveRequest::countWorkingDays(from, to, holidays_);
    p.check(from, to, days, today_);
    checkNoOverlap(r.employeeId(), from, to, r.id());
    checkBalance(r.employeeId(), p, from, days, r.days());
    r.setDates(from, to); r.setReason(why); r.calculateDays(holidays_);
    touch("UPDATE_REQUEST", r.id());
}

// ---------------------------------------------------------------- 5. delete
int LeaveSystem::deleteEmployee(const std::string& id) {
    Employee& e = employee(id);
    for (int i = 0; i < requests_.size(); ++i) {
        const LeaveRequest* r = requests_[i];
        if (r->employeeId() == e.id() && (r->status() == Status::Pending || (r->status() == Status::Approved && r->to() >= today_)))
            throw RuleError(e.name() + " still has " + std::string(statusName(r->status())) + " leave " + r->id() +
                            "; cancel or finish it first");
    }
    int removed = 0;
    for (int i = requests_.size() - 1; i >= 0; --i) {
        if (requests_[i]->employeeId() != e.id()) continue;
        for (int k = approvals_.size() - 1; k >= 0; --k)
            if (approvals_[k]->requestId() == requests_[i]->id()) approvals_.removeAt(k);
        requests_.removeAt(i);
        ++removed;
    }
    std::string label = e.id() + " " + e.name();
    employees_.removeAt(indexOfEmployee(id));
    touch("DELETE_EMPLOYEE", label + " (+" + std::to_string(removed) + " history rows)");
    return removed;
}
void LeaveSystem::deleteManager(const std::string& id) {
    Manager& m = manager(id);
    for (int i = 0; i < approvals_.size(); ++i)
        if (approvals_[i]->managerId() == m.id())
            throw RuleError(m.name() + " has decided requests (" + approvals_[i]->id() + "); the audit trail must be kept");
    std::string label = m.id() + " " + m.name();
    managers_.removeAt(indexOfManager(id));
    touch("DELETE_MANAGER", label);
}
void LeaveSystem::deleteHoliday(const std::string& id) {
    Holiday& h = holiday(id);
    std::string label = h.id() + " " + h.date().str();
    holidays_.removeAt(indexOfHoliday(id));
    touch("DELETE_HOLIDAY", label);
}

// ---------------------------------------------------------------- 6. transaction
Balance LeaveSystem::balance(const std::string& empId, const std::string& code, int year) const {
    const LeavePolicy& p = policy(code);
    Balance b{p.yearlyQuota() < 0, p.yearlyQuota(), 0, 0, 0};
    std::string eid = up(empId);
    for (int i = 0; i < requests_.size(); ++i) {
        const LeaveRequest* r = requests_[i];
        if (r->employeeId() != eid || r->type() != p.code() || r->from().year() != year) continue;
        if (r->status() == Status::Approved) b.used += r->days();
        else if (r->status() == Status::Pending) b.pending += r->days();
    }
    b.remaining = b.unlimited ? -1 : b.quota - b.used - b.pending;
    return b;
}
void LeaveSystem::checkNoOverlap(const std::string& empId, const Date& f, const Date& t, const std::string& except) const {
    for (int i = 0; i < requests_.size(); ++i) {
        const LeaveRequest* r = requests_[i];
        if (r->employeeId() == empId && r->id() != except && r->isActive() && r->overlaps(f, t))
            throw RuleError("dates overlap with " + std::string(statusName(r->status())) + " request " + r->id() + " (" +
                            r->from().str() + " to " + r->to().str() + ")");
    }
}
void LeaveSystem::checkBalance(const std::string& empId, const LeavePolicy& p, const Date& f, int days, int exceptDays) const {
    if (p.yearlyQuota() < 0) return;
    Balance b = balance(empId, p.code(), f.year());
    int have = b.remaining + exceptDays;
    if (days > have) throw RuleError("insufficient " + p.name() + " balance: need " + std::to_string(days) + " day(s), only " +
                                     std::to_string(have < 0 ? 0 : have) + " left in " + std::to_string(f.year()));
}
LeaveRequest& LeaveSystem::applyLeave(const std::string& empId, const std::string& typeCode, const Date& from,
                                      const Date& to, const std::string& reason) {
    Employee& e = employee(empId);
    const LeavePolicy& p = policy(typeCode);
    std::string why = io::trim(reason);
    if (why.empty() || why.size() > 60) throw ValidationError("reason must be 1-60 characters");
    if (to < from) throw RuleError("end date is before start date");
    if (from.year() != to.year()) throw RuleError("a request cannot span two calendar years; split it in two");
    if (from < e.joined()) throw RuleError("leave cannot start before the joining date (" + e.joined().str() + ")");
    std::unique_ptr<LeaveRequest> r(new LeaveRequest(nextId(requests_, 'R', 4), e.id(), p.code(), from, to, why, today_));
    int days = r->calculateDays(holidays_);
    p.check(from, to, days, today_);                       // virtual: each leave type has its own limits
    checkNoOverlap(e.id(), from, to, "");
    checkBalance(e.id(), p, from, days, 0);
    LeaveRequest& ref = *r;
    requests_.add(r.release());
    touch("APPLY", ref.id() + " " + e.id() + " " + p.code() + " " + from.str() + ".." + to.str() + " (" + std::to_string(days) + "d)");
    return ref;
}

int LeaveSystem::maxConcurrent(const std::string& dept) const {
    int n = 0;
    for (int i = 0; i < employees_.size(); ++i) if (employees_[i]->department() == dept) ++n;
    return std::max(1, n / 2);                             // at most half a department away at once
}
int LeaveSystem::onLeaveCount(const std::string& dept, const Date& d, const std::string& except) const {
    int n = 0;
    for (int i = 0; i < requests_.size(); ++i) {
        const LeaveRequest* r = requests_[i];
        if (r->status() != Status::Approved || r->id() == except || !r->covers(d)) continue;
        const Employee* e = findEmployee(r->employeeId());
        if (e && e->department() == dept) ++n;
    }
    return n;
}

Approval& LeaveSystem::decide(const std::string& reqId, const std::string& mgrId, bool approve, const std::string& comment) {
    LeaveRequest& r = request(reqId);
    Manager& m = manager(mgrId);
    if (r.status() != Status::Pending)
        throw RuleError("request " + r.id() + " is already " + statusName(r.status()) + "; only Pending requests can be decided");
    Employee* e = findEmployee(r.employeeId());
    if (!e) throw RuleError("the employee of " + r.id() + " no longer exists");
    if (!m.authorises(*e))
        throw RuleError(m.name() + " (" + m.department() + ") cannot decide for the " + e->department() + " department");
    if (approve) {                                         // department cover rule
        int limit = maxConcurrent(e->department());
        for (long s = r.from().serial(); s <= r.to().serial(); ++s) {
            Date d = Date::fromSerial(s);
            if (d.isWeekend() || isHoliday(d)) continue;
            if (onLeaveCount(e->department(), d, r.id()) + 1 > limit)
                throw RuleError("cover rule: on " + d.str() + " already " + std::to_string(limit) + " of the " + e->department() +
                                " team is on approved leave (limit " + std::to_string(limit) + ")");
        }
    }
    std::unique_ptr<Approval> a(new Approval(nextId(approvals_, 'A', 4), r.id(), m.id()));
    if (approve) a->approve(comment, today_); else a->reject(comment, today_);   // may throw -> nothing changed yet
    r.setStatus(approve ? Status::Approved : Status::Rejected);
    Approval& ref = *a;
    approvals_.add(a.release());
    touch(approve ? "APPROVE" : "REJECT", r.id() + " by " + m.id() + " -> " + ref.id());
    return ref;
}
void LeaveSystem::cancelLeave(const std::string& reqId) {
    LeaveRequest& r = request(reqId);
    if (r.status() == Status::Approved && r.from() <= today_)
        throw RuleError("approved leave that has already started cannot be cancelled");
    if (!r.isActive()) throw RuleError("request " + r.id() + " is " + statusName(r.status()) + " and cannot be cancelled");
    r.setStatus(Status::Cancelled);
    touch("CANCEL", r.id());
}
std::vector<const LeaveRequest*> LeaveSystem::pending() const {
    std::vector<const LeaveRequest*> v;
    for (int i = 0; i < requests_.size(); ++i) if (requests_[i]->status() == Status::Pending) v.push_back(requests_[i]);
    std::sort(v.begin(), v.end(), [](const LeaveRequest* a, const LeaveRequest* b) { return a->from() < b->from(); });
    return v;
}

// ---------------------------------------------------------------- 7. reports
void LeaveSystem::balanceReport(const std::string& empId, int year, std::ostream& os) const {
    const Employee& e = employee(empId);
    os << "  Leave balance for " << e.name() << " (" << e.id() << ", " << e.department() << ") - year " << year << "\n\n";
    os << "  " << io::pad("Type", 16) << io::pad("Quota", 8) << io::pad("Used", 7) << io::pad("Pending", 9) << "Remaining\n  " << std::string(50, '-') << "\n";
    for (int i = 0; i < 5; ++i) {
        Balance b = balance(e.id(), policies_[i]->code(), year);
        os << "  " << io::pad(policies_[i]->code() + " " + policies_[i]->name(), 16) << io::pad(b.unlimited ? "-" : std::to_string(b.quota), 8)
           << io::pad(std::to_string(b.used), 7) << io::pad(std::to_string(b.pending), 9) << (b.unlimited ? "unlimited" : std::to_string(b.remaining)) << "\n";
    }
    os << "\n  Requests this year:\n";
    bool any = false;
    for (int i = 0; i < requests_.size(); ++i)
        if (requests_[i]->employeeId() == e.id() && requests_[i]->from().year() == year) { os << "  " << requests_[i]->row() << "\n"; any = true; }
    if (!any) os << "  (none)\n";
}
void LeaveSystem::departmentReport(int year, std::ostream& os) const {
    std::map<std::string, std::vector<int>> st;                // dept -> {staff, requests, approved days, pending, rejected}
    for (int i = 0; i < employees_.size(); ++i) { auto& v = st[employees_[i]->department()]; v.resize(5); ++v[0]; }
    for (int i = 0; i < requests_.size(); ++i) {
        const LeaveRequest* r = requests_[i];
        const Employee* e = findEmployee(r->employeeId());
        if (!e || r->from().year() != year) continue;
        auto& v = st[e->department()]; v.resize(5);
        ++v[1];
        if (r->status() == Status::Approved) v[2] += r->days();
        else if (r->status() == Status::Pending) ++v[3];
        else if (r->status() == Status::Rejected) ++v[4];
    }
    os << "  Department summary - " << year << "\n\n  " << io::pad("Department", 15) << io::pad("Staff", 7) << io::pad("Requests", 10)
       << io::pad("Appr.days", 11) << io::pad("Pending", 9) << io::pad("Rejected", 10) << "Days/person\n  " << std::string(72, '-') << "\n";
    if (st.empty()) os << "  (no data)\n";
    for (const auto& kv : st) {
        const auto& v = kv.second;
        char avg[16];
        std::snprintf(avg, sizeof avg, "%.1f", v[0] ? static_cast<double>(v[2]) / v[0] : 0.0);
        os << "  " << io::pad(kv.first, 15) << io::pad(std::to_string(v[0]), 7) << io::pad(std::to_string(v[1]), 10)
           << io::pad(std::to_string(v[2]), 11) << io::pad(std::to_string(v[3]), 9) << io::pad(std::to_string(v[4]), 10) << avg << "\n";
    }
}
void LeaveSystem::onLeaveReport(const Date& d, std::ostream& os) const {
    os << "  Who is on leave on " << d.str() << " (" << d.weekdayName() << ")\n";
    if (d.isWeekend()) os << "  Note: weekend.\n";
    if (isHoliday(d)) os << "  Note: public holiday.\n";
    int n = 0;
    for (int i = 0; i < requests_.size(); ++i) {
        const LeaveRequest* r = requests_[i];
        if (r->status() != Status::Approved || !r->covers(d)) continue;
        os << "  " << io::pad(r->employeeId(), 6) << io::pad(employeeName(r->employeeId()), 20) << io::pad(r->type(), 4) << "until " << r->to().str() << "\n";
        ++n;
    }
    os << "  Total on approved leave: " << n << "\n";
}
void LeaveSystem::monthCalendar(int year, int month, std::ostream& os) const {
    if (month < 1 || month > 12) throw ValidationError("month must be 1-12");
    int dim = Date::daysInMonth(year, month);
    static const char* const mn[12] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
    os << "  Leave calendar - " << mn[month - 1] << " " << year << "   (UPPER = approved, lower = pending, _ = weekend, * = holiday)\n\n  " << std::string(12, ' ');
    for (int d = 1; d <= dim; ++d) os << (d % 10) << ' ';
    os << "\n  " << io::pad("", 12);
    for (int d = 1; d <= dim; ++d) os << "SMTWTFS"[Date(year, month, d).weekday()] << ' ';
    os << "\n";
    for (int i = 0; i < employees_.size(); ++i) {
        const Employee* e = employees_[i];
        os << "  " << io::pad(e->name(), 12);
        for (int d = 1; d <= dim; ++d) {
            Date dt(year, month, d);
            char c = dt.isWeekend() ? '_' : isHoliday(dt) ? '*' : '.';
            for (int k = 0; k < requests_.size(); ++k) {
                const LeaveRequest* r = requests_[k];
                if (r->employeeId() != e->id() || !r->covers(dt) || dt.isWeekend() || isHoliday(dt)) continue;
                if (r->status() == Status::Approved) c = r->type()[0];
                else if (r->status() == Status::Pending) c = static_cast<char>(std::tolower(static_cast<unsigned char>(r->type()[0])));
            }
            os << c << ' ';
        }
        os << "\n";
    }
    if (employees_.empty()) os << "  (no employees)\n";
}
void LeaveSystem::summaryReport(int year, std::ostream& os) const {
    int cnt[4] = {0, 0, 0, 0};
    std::map<std::string, int> byType, byEmp;
    for (int i = 0; i < requests_.size(); ++i) {
        const LeaveRequest* r = requests_[i];
        if (r->from().year() != year) continue;
        ++cnt[static_cast<int>(r->status())];
        if (r->status() == Status::Approved) { byType[r->type()] += r->days(); byEmp[r->employeeId()] += r->days(); }
    }
    int decided = cnt[1] + cnt[2];
    char rate[16];
    std::snprintf(rate, sizeof rate, "%.0f%%", decided ? 100.0 * cnt[1] / decided : 0.0);
    os << "  Overall leave summary - " << year << "\n  " << std::string(40, '-') << "\n"
       << "  Employees / managers / holidays : " << employees_.size() << " / " << managers_.size() << " / " << holidays_.size() << "\n"
       << "  Requests  pending/approved/rejected/cancelled : " << cnt[0] << "/" << cnt[1] << "/" << cnt[2] << "/" << cnt[3] << "\n"
       << "  Approval rate (decided requests) : " << rate << "\n  Approved days by type :";
    if (byType.empty()) os << " none";
    for (const auto& kv : byType) os << "  " << kv.first << "=" << kv.second;
    std::string best; int bestDays = 0;
    for (const auto& kv : byEmp) if (kv.second > bestDays) { best = kv.first; bestDays = kv.second; }
    os << "\n  Most leave taken : " << (best.empty() ? std::string("n/a") : employeeName(best) + " (" + best + ", " + std::to_string(bestDays) + " days)") << "\n";
}
void LeaveSystem::exportCsv(const std::string& path) const {
    std::ofstream f(path);
    if (!f) throw FileError("cannot create '" + path + "'");
    f << "RequestID,EmployeeID,Employee,Department,Type,From,To,WorkingDays,Status,Reason,ApprovedBy,Comment\n";
    for (int i = 0; i < requests_.size(); ++i) {
        const LeaveRequest* r = requests_[i];
        const Employee* e = findEmployee(r->employeeId());
        std::string by, note;
        for (int k = 0; k < approvals_.size(); ++k)
            if (approvals_[k]->requestId() == r->id()) { by = approvals_[k]->managerId(); note = approvals_[k]->comment(); }
        f << r->id() << ',' << r->employeeId() << ',' << io::csvQuote(e ? e->name() : "(deleted)") << ',' << io::csvQuote(e ? e->department() : "") << ','
          << r->type() << ',' << r->from().str() << ',' << r->to().str() << ',' << r->days() << ',' << statusName(r->status()) << ','
          << io::csvQuote(r->reason()) << ',' << by << ',' << io::csvQuote(note) << "\n";
    }
    if (!f) throw FileError("write failed for '" + path + "'");
}
void LeaveSystem::auditTail(int n, std::ostream& os) const {
    std::vector<std::string> t = audit_.tail(n);
    if (t.empty()) os << "  (audit log is empty)\n";
    for (const std::string& l : t) os << "  " << l << "\n";
}

// ---------------------------------------------------------------- 8. save / load
void LeaveSystem::save() {
    fm_.save("employees.dat", employees_, "Employee");
    fm_.save("managers.dat", managers_, "Manager");
    fm_.save("requests.dat", requests_, "LeaveRequest");
    fm_.save("approvals.dat", approvals_, "Approval");
    fm_.save("holidays.dat", holidays_, "Holiday");
    dirty_ = false;
    try { audit_.log(today_.str(), "SAVE", fm_.dir()); } catch (...) {}
}

template <class T>
static PtrList<T> loadList(const FileManager& fm, const std::string& file, const std::string& kind,
                           std::vector<std::string>& warn, T* (*parse)(const std::vector<std::string>&)) {
    bool ok = false;
    std::vector<std::string> lines = fm.readLines(file, kind, ok, warn);
    PtrList<T> out;
    int n = 0;
    for (const std::string& l : lines) {
        ++n;
        try {
            std::unique_ptr<T> p(parse(io::split(l, '\t')));
            for (int i = 0; i < out.size(); ++i)
                if (out[i]->id() == p->id()) throw ParseError("duplicate id " + p->id());
            out.add(p.release());
        } catch (const ElmError& e) {
            warn.push_back(file + " line " + std::to_string(n + 1) + " skipped: " + e.what());
        }
    }
    return out;
}

void LeaveSystem::load() {
    std::vector<std::string> warn;
    PtrList<Employee> e = loadList<Employee>(fm_, "employees.dat", "Employee", warn, &Employee::fromFields);
    PtrList<Manager> m = loadList<Manager>(fm_, "managers.dat", "Manager", warn, &Manager::fromFields);
    PtrList<LeaveRequest> r = loadList<LeaveRequest>(fm_, "requests.dat", "LeaveRequest", warn, &LeaveRequest::fromFields);
    PtrList<Approval> a = loadList<Approval>(fm_, "approvals.dat", "Approval", warn, &Approval::fromFields);
    PtrList<Holiday> h = loadList<Holiday>(fm_, "holidays.dat", "Holiday", warn, &Holiday::fromFields);
    employees_ = std::move(e); managers_ = std::move(m); requests_ = std::move(r);
    approvals_ = std::move(a); holidays_ = std::move(h);
    for (int i = 0; i < requests_.size(); ++i) {              // referential integrity report
        if (!findEmployee(requests_[i]->employeeId())) warn.push_back("request " + requests_[i]->id() + " refers to missing employee " + requests_[i]->employeeId());
        try { policy(requests_[i]->type()); } catch (const NotFoundError&) { warn.push_back("request " + requests_[i]->id() + " has unknown type " + requests_[i]->type()); }
    }
    for (int i = 0; i < approvals_.size(); ++i)
        if (!findRequest(approvals_[i]->requestId())) warn.push_back("approval " + approvals_[i]->id() + " refers to missing request");
    warnings_ = warn;
    dirty_ = false;
}
std::string LeaveSystem::backup() {
    save();
    std::time_t t = std::time(nullptr);
    char hm[16];
    std::strftime(hm, sizeof hm, "%H%M%S", std::localtime(&t));
    return fm_.backup(today_.str() + "_" + hm);
}
void LeaveSystem::clearAll() {
    employees_.clear(); managers_.clear(); requests_.clear(); approvals_.clear(); holidays_.clear();
    dirty_ = true;
}

void LeaveSystem::loadDemoData() {
    clearAll();
    for (int y = today_.year(); y <= today_.year() + 1; ++y) {
        addHoliday(Date(y, 1, 1), "New Year's Day"); addHoliday(Date(y, 1, 26), "Republic Day");
        addHoliday(Date(y, 8, 15), "Independence Day"); addHoliday(Date(y, 10, 2), "Gandhi Jayanti");
        addHoliday(Date(y, 12, 25), "Christmas Day");
    }
    addManager("Anita Desai", "Engineering", "Engineering Head", false);
    addManager("Rohan Mehta", "Sales", "Sales Head", false);
    addManager("Priya Nair", "HR", "HR Director", true);
    struct P { const char *n, *d, *t, *m; int yrs; };
    const P staff[8] = {
        {"Aarav Sharma", "Engineering", "Software Engineer", "aarav@acme.com", 4}, {"Diya Patel", "Engineering", "QA Engineer", "diya@acme.com", 3},
        {"Kabir Shah", "Engineering", "DevOps Engineer", "kabir@acme.com", 2}, {"Meera Iyer", "Engineering", "UI Designer", "meera@acme.com", 5},
        {"Vikram Rao", "Sales", "Account Executive", "vikram@acme.com", 3}, {"Sana Khan", "Sales", "Sales Analyst", "sana@acme.com", 2},
        {"Arjun Verma", "Sales", "Regional Manager", "arjun@acme.com", 6}, {"Neha Gupta", "HR", "HR Executive", "neha@acme.com", 1}};
    for (const P& p : staff) addEmployee(p.n, p.d, p.t, p.m, today_.addDays(-365L * p.yrs - 30));
    auto start = [&](int off) { Date d = today_.addDays(off); while (d.isWeekend()) d = d.addDays(1); return d; };
    auto attempt = [&](const char* emp, const char* type, int off, int span, const char* why, const char* mgr, int verdict, const char* note) {
        try {                                                  // verdict: 0 leave pending, 1 approve, 2 reject, 3 approve then cancel
            Date f = start(off);
            if (std::string(type) == "SL") f = today_.addDays(off);
            LeaveRequest& r = applyLeave(emp, type, f, f.addDays(span), why);
            if (verdict == 1 || verdict == 3) decide(r.id(), mgr, true, note);
            if (verdict == 2) decide(r.id(), mgr, false, note);
            if (verdict == 3) cancelLeave(r.id());
        } catch (const ElmError&) {}                           // demo data must never crash the program
    };
    attempt("E001", "AL", 14, 4, "Family wedding", "M001", 1, "Enjoy!");
    attempt("E002", "SL", -6, 3, "Viral fever", "M001", 1, "Get well soon");
    attempt("E003", "CL", 10, 0, "Bank work", "M001", 0, "");
    attempt("E005", "AL", 30, 4, "Goa trip", "M002", 2, "Quarter-end freeze, please re-apply");
    attempt("E006", "AL", 21, 2, "Long weekend", "M002", 0, "");
    attempt("E004", "PL", 40, 11, "Newborn care", "M001", 0, "");
    attempt("E007", "UL", 35, 9, "Higher studies exam", "M002", 0, "");
    attempt("E008", "AL", 60, 3, "Home visit", "M003", 1, "Approved");
    attempt("E002", "AL", 25, 1, "Plans changed", "M001", 3, "OK");
    attempt("E007", "AL", 5, 0, "Doctor visit", "M002", 1, "Fine");
    touch("DEMO_DATA", "sample company loaded");
}

}  // namespace elm

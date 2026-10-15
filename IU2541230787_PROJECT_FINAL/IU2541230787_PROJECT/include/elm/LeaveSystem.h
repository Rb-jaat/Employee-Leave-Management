// LeaveSystem.h - the controller: owns all data, enforces every business rule, never touches
// the keyboard (ConsoleUI does that), so each rule can be unit-tested on its own.
#pragma once
#include <iosfwd>
#include <string>
#include <vector>
#include "Approval.h"
#include "AuditLog.h"
#include "Date.h"
#include "FileManager.h"
#include "Holiday.h"
#include "LeavePolicy.h"
#include "LeaveRequest.h"
#include "Person.h"
#include "PtrList.h"

namespace elm {

struct Balance {
    bool unlimited;
    int quota, used, pending, remaining;
};

class LeaveSystem {
    FileManager fm_;
    AuditLog audit_;
    PtrList<Employee> employees_;
    PtrList<Manager> managers_;
    PtrList<LeaveRequest> requests_;
    PtrList<Approval> approvals_;
    PtrList<Holiday> holidays_;
    LeavePolicy** policies_;                 // new LeavePolicy*[5], each element new'd
    Date today_;
    bool dirty_;
    std::vector<std::string> warnings_;      // problems met while loading

    void touch(const std::string& action, const std::string& detail);
    template <class T> std::string nextId(const PtrList<T>& l, char prefix, int width) const;
    int indexOfEmployee(const std::string& id) const;
    int indexOfManager(const std::string& id) const;
    int indexOfRequest(const std::string& id) const;
    int indexOfHoliday(const std::string& id) const;
    int maxConcurrent(const std::string& dept) const;
    int onLeaveCount(const std::string& dept, const Date& d, const std::string& exceptReq) const;
    void checkNoOverlap(const std::string& empId, const Date& f, const Date& t, const std::string& exceptReq) const;
    void checkBalance(const std::string& empId, const LeavePolicy& p, const Date& f, int days, int exceptDays) const;
public:
    explicit LeaveSystem(const std::string& dataDir);
    ~LeaveSystem();
    LeaveSystem(const LeaveSystem&) = delete;
    LeaveSystem& operator=(const LeaveSystem&) = delete;

    // ---- clock ----
    Date today() const { return today_; }
    void setToday(const Date& d) { today_ = d; }

    // ---- read access ----
    const PtrList<Employee>& employees() const { return employees_; }
    const PtrList<Manager>& managers() const { return managers_; }
    const PtrList<LeaveRequest>& requests() const { return requests_; }
    const PtrList<Approval>& approvals() const { return approvals_; }
    const PtrList<Holiday>& holidays() const { return holidays_; }
    const LeavePolicy& policy(const std::string& code) const;          // throws NotFoundError
    int policyCount() const { return 5; }
    const LeavePolicy& policyAt(int i) const { return *policies_[i]; }
    Employee* findEmployee(const std::string& id) const;                // nullptr if absent
    Manager* findManager(const std::string& id) const;
    LeaveRequest* findRequest(const std::string& id) const;
    Holiday* findHoliday(const std::string& id) const;
    Employee& employee(const std::string& id) const;                    // throws NotFoundError
    Manager& manager(const std::string& id) const;
    LeaveRequest& request(const std::string& id) const;
    Holiday& holiday(const std::string& id) const;
    std::string employeeName(const std::string& id) const;              // "(deleted)" if gone
    bool isHoliday(const Date& d) const;
    std::vector<const Record*> everything() const;                      // polymorphic view
    bool dirty() const { return dirty_; }
    const std::vector<std::string>& warnings() const { return warnings_; }

    // ---- 1. add ----
    Employee& addEmployee(const std::string& name, const std::string& dept, const std::string& designation,
                          const std::string& email, const Date& joined);
    Manager& addManager(const std::string& name, const std::string& dept, const std::string& title, bool hr);
    Holiday& addHoliday(const Date& d, const std::string& name);

    // ---- 3. search ----
    std::vector<const Employee*> searchEmployees(const std::string& text) const;
    std::vector<const Manager*> searchManagers(const std::string& text) const;
    std::vector<const LeaveRequest*> searchRequests(const std::string& text) const;
    std::vector<const LeaveRequest*> requestsBetween(const Date& a, const Date& b) const;
    std::vector<const Holiday*> searchHolidays(const std::string& text) const;

    // ---- 4. update ----
    void updateEmployee(const std::string& id, const std::string& name, const std::string& dept,
                        const std::string& designation, const std::string& email);
    void updateManager(const std::string& id, const std::string& name, const std::string& dept,
                       const std::string& title, bool hr);
    void updateRequest(const std::string& id, const Date& from, const Date& to, const std::string& reason);
    void updateHoliday(const std::string& id, const Date& d, const std::string& name);

    // ---- 5. delete ----
    int deleteEmployee(const std::string& id);          // returns how many history rows were removed
    void deleteManager(const std::string& id);
    void deleteHoliday(const std::string& id);

    // ---- 6. main transaction: apply -> decide -> cancel ----
    LeaveRequest& applyLeave(const std::string& empId, const std::string& typeCode, const Date& from,
                             const Date& to, const std::string& reason);
    Approval& decide(const std::string& reqId, const std::string& mgrId, bool approve, const std::string& comment);
    void cancelLeave(const std::string& reqId);
    std::vector<const LeaveRequest*> pending() const;
    Balance balance(const std::string& empId, const std::string& typeCode, int year) const;

    // ---- 7. reports (all write to a stream so they can be tested) ----
    void balanceReport(const std::string& empId, int year, std::ostream& os) const;
    void departmentReport(int year, std::ostream& os) const;
    void onLeaveReport(const Date& d, std::ostream& os) const;
    void monthCalendar(int year, int month, std::ostream& os) const;
    void summaryReport(int year, std::ostream& os) const;
    void exportCsv(const std::string& path) const;
    void auditTail(int n, std::ostream& os) const;

    // ---- 8. save / load ----
    void save();
    void load();                                        // replaces in-memory data with disk data
    std::string backup();
    void loadDemoData();                                // sample company, dates relative to today
    void clearAll();
};

}  // namespace elm

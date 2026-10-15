// LeaveRequest.h - one leave application, with its status life-cycle.
#pragma once
#include <string>
#include <vector>
#include "Date.h"
#include "PtrList.h"
#include "Holiday.h"
#include "Record.h"

namespace elm {

enum class Status { Pending, Approved, Rejected, Cancelled };
const char* statusName(Status s);
Status statusFromName(const std::string& s);                 // throws ParseError

class LeaveRequest : public Record {
    std::string employeeId_;
    std::string type_;                                       // policy code
    Date from_, to_, applied_;
    std::string reason_;
    int days_;                                               // working days, fixed at apply time
    Status status_;
public:
    LeaveRequest();
    LeaveRequest(const std::string& id, const std::string& empId, const std::string& type,
                 const Date& from, const Date& to, const std::string& reason,
                 const Date& applied);
    const std::string& employeeId() const { return employeeId_; }
    const std::string& type() const { return type_; }
    const Date& from() const { return from_; }
    const Date& to() const { return to_; }
    const Date& applied() const { return applied_; }
    const std::string& reason() const { return reason_; }
    int days() const { return days_; }
    Status status() const { return status_; }
    bool isActive() const { return status_ == Status::Pending || status_ == Status::Approved; }
    bool covers(const Date& d) const { return from_ <= d && d <= to_; }
    bool overlaps(const Date& a, const Date& b) const { return from_ <= b && a <= to_; }

    // Counts Mon-Fri days in [from,to] that are not public holidays; stores and returns it.
    int calculateDays(const PtrList<Holiday>& holidays);
    static int countWorkingDays(const Date& from, const Date& to, const PtrList<Holiday>& h);
    void setDates(const Date& f, const Date& t) { from_ = f; to_ = t; }
    void setReason(const std::string& r) { reason_ = r; }
    void setStatus(Status s) { status_ = s; }

    std::string kind() const override { return "LeaveRequest"; }
    void display(std::ostream& os) const override;
    std::string row() const override;
    std::string serialize() const override;
    bool matches(const std::string& text) const override;    // id / employee / type / status
    static LeaveRequest* fromFields(const std::vector<std::string>& f);
};

}  // namespace elm

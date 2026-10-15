#include "elm/LeaveRequest.h"
#include <ostream>
#include "elm/errors.h"
#include "elm/io.h"

namespace elm {

const char* statusName(Status s) {
    switch (s) {
        case Status::Pending: return "Pending";
        case Status::Approved: return "Approved";
        case Status::Rejected: return "Rejected";
        case Status::Cancelled: return "Cancelled";
    }
    return "?";
}
Status statusFromName(const std::string& s) {
    for (Status st : {Status::Pending, Status::Approved, Status::Rejected, Status::Cancelled})
        if (s == statusName(st)) return st;
    throw ParseError("unknown status '" + s + "'");
}

LeaveRequest::LeaveRequest() : days_(0), status_(Status::Pending) {}
LeaveRequest::LeaveRequest(const std::string& id, const std::string& e, const std::string& type, const Date& f,
                           const Date& t, const std::string& reason, const Date& applied)
    : Record(id), employeeId_(e), type_(type), from_(f), to_(t), applied_(applied), reason_(reason),
      days_(0), status_(Status::Pending) {}

int LeaveRequest::countWorkingDays(const Date& from, const Date& to, const PtrList<Holiday>& hols) {
    int n = 0;
    for (long s = from.serial(); s <= to.serial(); ++s) {
        Date d = Date::fromSerial(s);
        if (d.isWeekend()) continue;
        bool hol = false;
        for (int i = 0; i < hols.size() && !hol; ++i) hol = (hols[i]->date() == d);
        if (!hol) ++n;
    }
    return n;
}
int LeaveRequest::calculateDays(const PtrList<Holiday>& hols) { return days_ = countWorkingDays(from_, to_, hols); }

void LeaveRequest::display(std::ostream& os) const {
    os << "  Request ID  : " << id() << "\n  Employee    : " << employeeId_ << "\n  Leave type  : " << type_
       << "\n  Period      : " << from_.str() << " (" << from_.weekdayName() << ") to " << to_.str() << " ("
       << to_.weekdayName() << ")\n  Working days: " << days_ << "\n  Reason      : " << reason_
       << "\n  Applied on  : " << applied_.str() << "\n  Status      : " << statusName(status_) << "\n";
}
std::string LeaveRequest::row() const {
    return io::pad(id(), 7) + io::pad(employeeId_, 6) + io::pad(type_, 5) + io::pad(from_.str(), 11) +
           io::pad(to_.str(), 11) + io::pad(std::to_string(days_), 5) + io::pad(statusName(status_), 10) + reason_;
}
std::string LeaveRequest::serialize() const {
    return io::escape(id()) + "\t" + io::escape(employeeId_) + "\t" + type_ + "\t" + from_.str() + "\t" + to_.str() +
           "\t" + applied_.str() + "\t" + io::escape(reason_) + "\t" + std::to_string(days_) + "\t" + statusName(status_);
}
bool LeaveRequest::matches(const std::string& t) const {
    return Record::matches(t) || io::containsCI(employeeId_, t) || io::containsCI(type_, t) ||
           io::containsCI(statusName(status_), t) || io::containsCI(reason_, t);
}
LeaveRequest* LeaveRequest::fromFields(const std::vector<std::string>& f) {
    if (f.size() != 9) throw ParseError("LeaveRequest line needs 9 fields, got " + std::to_string(f.size()));
    try {
        LeaveRequest* r = new LeaveRequest(io::unescape(f[0]), io::unescape(f[1]), f[2], Date::parse(f[3]),
                                           Date::parse(f[4]), io::unescape(f[6]), Date::parse(f[5]));
        r->days_ = io::toInt(f[7], "days");
        try { r->status_ = statusFromName(f[8]); } catch (...) { delete r; throw; }
        return r;
    } catch (const ValidationError& e) { throw ParseError(e.what()); }
}

}  // namespace elm

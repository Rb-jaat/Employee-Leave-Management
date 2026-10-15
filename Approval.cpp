#include "elm/Approval.h"
#include <ostream>
#include "elm/errors.h"
#include "elm/io.h"

namespace elm {

const char* decisionName(Decision d) {
    return d == Decision::Pending ? "Pending" : d == Decision::Approved ? "Approved" : "Rejected";
}

Approval::Approval() : decision_(Decision::Pending) {}
Approval::Approval(const std::string& id, const std::string& rid, const std::string& mid)
    : Record(id), requestId_(rid), managerId_(mid), decision_(Decision::Pending) {}

void Approval::approve(const std::string& comment, const Date& on) {
    if (decision_ != Decision::Pending) throw RuleError("approval " + id() + " is already decided");
    decision_ = Decision::Approved; comment_ = io::trim(comment); decidedOn_ = on;
}
void Approval::reject(const std::string& comment, const Date& on) {
    if (decision_ != Decision::Pending) throw RuleError("approval " + id() + " is already decided");
    if (io::trim(comment).empty()) throw ValidationError("a rejection needs a reason");
    decision_ = Decision::Rejected; comment_ = io::trim(comment); decidedOn_ = on;
}
void Approval::display(std::ostream& os) const {
    os << "  Approval ID : " << id() << "\n  Request     : " << requestId_ << "\n  Manager     : " << managerId_
       << "\n  Decision    : " << decisionName(decision_) << "\n  Decided on  : " << decidedOn_.str()
       << "\n  Comment     : " << comment_ << "\n";
}
std::string Approval::row() const {
    return io::pad(id(), 7) + io::pad(requestId_, 7) + io::pad(managerId_, 6) + io::pad(decisionName(decision_), 10) +
           io::pad(decidedOn_.str(), 11) + comment_;
}
std::string Approval::serialize() const {
    return io::escape(id()) + "\t" + io::escape(requestId_) + "\t" + io::escape(managerId_) + "\t" +
           decisionName(decision_) + "\t" + decidedOn_.str() + "\t" + io::escape(comment_);
}
bool Approval::matches(const std::string& t) const {
    return Record::matches(t) || io::containsCI(requestId_, t) || io::containsCI(managerId_, t) ||
           io::containsCI(decisionName(decision_), t);
}
Approval* Approval::fromFields(const std::vector<std::string>& f) {
    if (f.size() != 6) throw ParseError("Approval line needs 6 fields");
    Approval* a = new Approval(io::unescape(f[0]), io::unescape(f[1]), io::unescape(f[2]));
    try {
        Date d = Date::parse(f[4]);
        if (f[3] == "Approved") a->approve(io::unescape(f[5]), d);
        else if (f[3] == "Rejected") a->reject(io::unescape(f[5]), d);
        else if (f[3] != "Pending") throw ParseError("unknown decision '" + f[3] + "'");
    } catch (const ElmError& e) { delete a; throw ParseError(e.what()); }
    return a;
}

}  // namespace elm

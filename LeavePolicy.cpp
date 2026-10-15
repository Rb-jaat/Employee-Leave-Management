#include "elm/LeavePolicy.h"
#include "elm/errors.h"

namespace elm {

const char* const kPolicyCodes[5] = {"AL", "SL", "CL", "PL", "UL"};

LeavePolicy* makePolicy(const std::string& c) {
    if (c == "AL") return new AnnualLeave;
    if (c == "SL") return new SickLeave;
    if (c == "CL") return new CasualLeave;
    if (c == "PL") return new ParentalLeave;
    if (c == "UL") return new UnpaidLeave;
    return nullptr;
}

void LeavePolicy::check(const Date& from, const Date& to, int workingDays, const Date& today) const {
    if (to < from) throw RuleError("end date is before start date");
    if (workingDays < 1) throw RuleError("the chosen dates contain no working day (weekends/holidays only)");
    if (workingDays > maxConsecutive())
        throw RuleError(name() + " allows at most " + std::to_string(maxConsecutive()) + " working days per request");
    long ahead = from - today;                                   // negative = in the past
    if (ahead < -maxBackdateDays())
        throw RuleError(name() + " cannot start more than " + std::to_string(maxBackdateDays()) + " day(s) in the past");
    if (ahead >= 0 && ahead < minNoticeDays())
        throw RuleError(name() + " needs at least " + std::to_string(minNoticeDays()) + " day(s) notice");
}

std::string LeavePolicy::describe() const {
    return code() + " " + name() + ": quota " + (yearlyQuota() < 0 ? std::string("unlimited") : std::to_string(yearlyQuota())) +
           "/yr, max " + std::to_string(maxConsecutive()) + " days/request, notice " + std::to_string(minNoticeDays()) +
           "d, back-date " + std::to_string(maxBackdateDays()) + "d, " + (paid() ? "paid" : "unpaid");
}

}  // namespace elm

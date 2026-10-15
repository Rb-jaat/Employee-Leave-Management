// LeavePolicy.h - abstract leave type + 5 concrete policies (runtime polymorphism).
// Every policy answers the same questions differently: quota, notice, back-dating, limits.
#pragma once
#include <string>
#include "Date.h"

namespace elm {

class LeavePolicy {
public:
    virtual ~LeavePolicy() = default;
    virtual std::string code() const = 0;                    // "AL", "SL" ...
    virtual std::string name() const = 0;
    virtual int  yearlyQuota() const = 0;                    // working days per year, -1 = unlimited
    virtual int  maxConsecutive() const = 0;                 // max working days in one request
    virtual int  minNoticeDays() const { return 0; }         // apply at least N days ahead
    virtual int  maxBackdateDays() const { return 0; }       // may start up to N days in the past
    virtual bool paid() const { return true; }
    // Throws RuleError when the request breaks this policy (non-virtual template of steps).
    void check(const Date& from, const Date& to, int workingDays, const Date& today) const;
    virtual std::string describe() const;
};

class AnnualLeave : public LeavePolicy {
public:
    std::string code() const override { return "AL"; }
    std::string name() const override { return "Annual Leave"; }
    int yearlyQuota() const override { return 18; }
    int maxConsecutive() const override { return 15; }
    int minNoticeDays() const override { return 3; }
};
class SickLeave : public LeavePolicy {
public:
    std::string code() const override { return "SL"; }
    std::string name() const override { return "Sick Leave"; }
    int yearlyQuota() const override { return 10; }
    int maxConsecutive() const override { return 7; }
    int maxBackdateDays() const override { return 7; }       // illness cannot be planned
};
class CasualLeave : public LeavePolicy {
public:
    std::string code() const override { return "CL"; }
    std::string name() const override { return "Casual Leave"; }
    int yearlyQuota() const override { return 7; }
    int maxConsecutive() const override { return 3; }
    int maxBackdateDays() const override { return 1; }
};
class ParentalLeave : public LeavePolicy {
public:
    std::string code() const override { return "PL"; }
    std::string name() const override { return "Parental Leave"; }
    int yearlyQuota() const override { return 30; }
    int maxConsecutive() const override { return 30; }
    int minNoticeDays() const override { return 14; }
};
class UnpaidLeave : public LeavePolicy {
public:
    std::string code() const override { return "UL"; }
    std::string name() const override { return "Unpaid Leave"; }
    int yearlyQuota() const override { return -1; }
    int maxConsecutive() const override { return 30; }
    int minNoticeDays() const override { return 7; }
    bool paid() const override { return false; }
};

LeavePolicy* makePolicy(const std::string& code);            // factory, nullptr if unknown
extern const char* const kPolicyCodes[5];

}  // namespace elm

// Approval.h - a manager's decision on a request (created Pending, then approve()/reject()).
#pragma once
#include <string>
#include <vector>
#include "Date.h"
#include "Record.h"

namespace elm {

enum class Decision { Pending, Approved, Rejected };
const char* decisionName(Decision d);

class Approval : public Record {
    std::string requestId_, managerId_, comment_;
    Decision decision_;
    Date decidedOn_;
public:
    Approval();
    Approval(const std::string& id, const std::string& requestId, const std::string& managerId);
    const std::string& requestId() const { return requestId_; }
    const std::string& managerId() const { return managerId_; }
    const std::string& comment() const { return comment_; }
    Decision decision() const { return decision_; }
    const Date& decidedOn() const { return decidedOn_; }

    void approve(const std::string& comment, const Date& on);   // throws RuleError if decided
    void reject(const std::string& comment, const Date& on);    // reject needs a comment

    std::string kind() const override { return "Approval"; }
    void display(std::ostream& os) const override;
    std::string row() const override;
    std::string serialize() const override;
    bool matches(const std::string& text) const override;
    static Approval* fromFields(const std::vector<std::string>& f);
};

}  // namespace elm

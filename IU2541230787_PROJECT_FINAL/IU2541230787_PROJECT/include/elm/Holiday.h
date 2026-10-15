// Holiday.h - a public holiday; holidays do not count as leave days.
#pragma once
#include <string>
#include <vector>
#include "Date.h"
#include "Record.h"

namespace elm {

class Holiday : public Record {
    Date date_;
    std::string name_;
public:
    Holiday() = default;
    Holiday(const std::string& id, const Date& d, const std::string& name);
    const Date& date() const { return date_; }
    const std::string& name() const { return name_; }
    void setDate(const Date& d) { date_ = d; }
    void setName(const std::string& n);

    std::string kind() const override { return "Holiday"; }
    void display(std::ostream& os) const override;
    std::string row() const override;
    std::string serialize() const override;
    bool matches(const std::string& text) const override;
    static Holiday* fromFields(const std::vector<std::string>& f);
};

}  // namespace elm

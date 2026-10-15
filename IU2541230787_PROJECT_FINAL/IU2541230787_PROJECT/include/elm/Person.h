// Person.h / Employee / Manager - the people hierarchy (Record -> Person -> Employee | Manager).
#pragma once
#include <string>
#include <vector>
#include "Date.h"
#include "Record.h"

namespace elm {

class Person : public Record {
    std::string name_;
    std::string department_;
public:
    Person() = default;
    Person(const std::string& id, const std::string& name, const std::string& dept);
    const std::string& name() const { return name_; }
    const std::string& department() const { return department_; }
    void setName(const std::string& n);                      // validated
    void setDepartment(const std::string& d);                // validated
    virtual std::string role() const = 0;                    // runtime polymorphism
    bool matches(const std::string& text) const override;    // id / name / department
};

class Employee : public Person {
    std::string designation_;
    std::string email_;
    Date joined_;
public:
    Employee() = default;                                    // default constructor
    Employee(const std::string& id, const std::string& name, const std::string& dept,
             const std::string& designation, const std::string& email, const Date& joined);
    const std::string& designation() const { return designation_; }
    const std::string& email() const { return email_; }
    const Date& joined() const { return joined_; }
    void setDesignation(const std::string& d);
    void setEmail(const std::string& e);

    std::string kind() const override { return "Employee"; }
    std::string role() const override { return "Employee"; }
    void display(std::ostream& os) const override;
    std::string row() const override;
    std::string serialize() const override;
    static Employee* fromFields(const std::vector<std::string>& f);   // new'd, caller owns
};

class Manager : public Person {
    std::string title_;
    bool hr_;                                                // HR managers may approve any department
public:
    Manager() : hr_(false) {}
    Manager(const std::string& id, const std::string& name, const std::string& dept,
            const std::string& title, bool hr);
    const std::string& title() const { return title_; }
    bool isHR() const { return hr_; }
    void setTitle(const std::string& t);
    void setHR(bool v) { hr_ = v; }
    bool authorises(const Employee& e) const;                // same department, or HR

    std::string kind() const override { return "Manager"; }
    std::string role() const override { return hr_ ? "HR Manager" : "Manager"; }
    void display(std::ostream& os) const override;
    std::string row() const override;
    std::string serialize() const override;
    static Manager* fromFields(const std::vector<std::string>& f);
};

}  // namespace elm

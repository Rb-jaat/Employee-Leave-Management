// Record.h - abstract root of every stored entity (id + polymorphic display/serialize).
#pragma once
#include <iosfwd>
#include <string>

namespace elm {

class Record {
    std::string id_;                         // private: only changeable via the constructor
public:
    Record() = default;
    explicit Record(std::string id) : id_(std::move(id)) {}
    virtual ~Record() = default;             // virtual: delete through Record* is safe

    const std::string& id() const { return id_; }

    virtual std::string kind() const = 0;                    // "Employee", "Manager" ...
    virtual void display(std::ostream& os) const = 0;        // full multi-line view
    virtual std::string row() const = 0;                     // one-line table row
    virtual std::string serialize() const = 0;               // tab-separated escaped fields
    virtual bool matches(const std::string& text) const;     // search hook (default: id match)
};

}  // namespace elm

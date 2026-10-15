// AuditLog.h - append-only text log of every change (who/what/when).
#pragma once
#include <string>
#include <vector>

namespace elm {

class AuditLog {
    std::string path_;
    bool enabled_;
public:
    explicit AuditLog(const std::string& path = "", bool enabled = true) : path_(path), enabled_(enabled) {}
    void setPath(const std::string& p) { path_ = p; }
    void log(const std::string& date, const std::string& action, const std::string& detail) const;
    std::vector<std::string> tail(int n) const;              // last n lines
};

}  // namespace elm

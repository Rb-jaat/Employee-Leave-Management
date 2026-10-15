#include "elm/AuditLog.h"
#include <ctime>
#include <fstream>
#include "elm/io.h"

namespace elm {

void AuditLog::log(const std::string& date, const std::string& action, const std::string& detail) const {
    if (!enabled_ || path_.empty()) return;
    std::ofstream f(path_, std::ios::app);
    if (!f) return;                                  // logging must never break the program
    std::time_t t = std::time(nullptr);
    std::tm* lt = std::localtime(&t);
    char hm[16];
    std::strftime(hm, sizeof hm, "%H:%M:%S", lt);
    f << date << ' ' << hm << " | " << action << " | " << io::escape(detail) << "\n";
}
std::vector<std::string> AuditLog::tail(int n) const {
    std::vector<std::string> all, out;
    std::ifstream f(path_);
    for (std::string line; std::getline(f, line);) all.push_back(line);
    size_t start = all.size() > static_cast<size_t>(n) ? all.size() - static_cast<size_t>(n) : 0;
    for (size_t i = start; i < all.size(); ++i) out.push_back(all[i]);
    return out;
}

}  // namespace elm

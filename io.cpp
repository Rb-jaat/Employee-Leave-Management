#include "elm/io.h"
#include <algorithm>
#include <cctype>
#include <iostream>
#include "elm/errors.h"
#ifdef _WIN32
#include <io.h>
#define ELM_ISATTY(f) _isatty(f)
#else
#include <unistd.h>
#define ELM_ISATTY(f) isatty(f)
#endif

namespace elm { namespace io {

std::string trim(const std::string& s) {
    size_t a = 0, b = s.size();
    while (a < b && std::isspace(static_cast<unsigned char>(s[a]))) ++a;
    while (b > a && std::isspace(static_cast<unsigned char>(s[b - 1]))) --b;
    return s.substr(a, b - a);
}
std::string lower(const std::string& s) {
    std::string r = s;
    std::transform(r.begin(), r.end(), r.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return r;
}
bool containsCI(const std::string& hay, const std::string& needle) {
    return lower(hay).find(lower(needle)) != std::string::npos;
}
std::string escape(const std::string& s) {
    std::string r;
    for (char c : s) {
        if (c == '\\') r += "\\\\";
        else if (c == '\t') r += "\\t";
        else if (c == '\n') r += "\\n";
        else if (c == '\r') r += "\\r";
        else r += c;
    }
    return r;
}
std::string unescape(const std::string& s) {
    std::string r;
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '\\' && i + 1 < s.size()) {
            char n = s[++i];
            r += (n == 't') ? '\t' : (n == 'n') ? '\n' : (n == 'r') ? '\r' : n;
        } else r += s[i];
    }
    return r;
}
std::vector<std::string> split(const std::string& s, char sep) {
    std::vector<std::string> out;
    std::string cur;
    for (char c : s) {
        if (c == sep) { out.push_back(cur); cur.clear(); }
        else cur += c;
    }
    out.push_back(cur);
    return out;
}
std::string pad(const std::string& s, size_t w) {
    if (s.size() >= w) return s.substr(0, w);
    return s + std::string(w - s.size(), ' ');
}
std::string csvQuote(const std::string& s) {
    std::string r = "\"";
    for (char c : s) { if (c == '"') r += '"'; r += c; }
    return r + "\"";
}
bool validEmail(const std::string& s) {
    size_t at = s.find('@');
    if (at == std::string::npos || at == 0 || s.find('@', at + 1) != std::string::npos) return false;
    size_t dot = s.rfind('.');
    if (dot == std::string::npos || dot < at + 2 || dot + 3 > s.size()) return false;
    for (char c : s) if (std::isspace(static_cast<unsigned char>(c))) return false;
    return true;
}
bool validName(const std::string& s) {
    if (s.size() < 2 || s.size() > 40) return false;
    bool letter = false;
    for (char c : s) {
        unsigned char u = static_cast<unsigned char>(c);
        if (std::isalpha(u)) letter = true;
        else if (c != ' ' && c != '.' && c != '\'' && c != '-') return false;
    }
    return letter;
}
std::string titleCase(const std::string& s) {
    std::string r = s;
    bool start = true;
    for (char& c : r) {
        unsigned char u = static_cast<unsigned char>(c);
        if (std::isalpha(u)) { c = static_cast<char>(start ? std::toupper(u) : std::tolower(u)); start = false; }
        else start = (c == ' ' || c == '-' || c == '.');
    }
    return r;
}
int toInt(const std::string& s, const char* field) {
    try {
        size_t used = 0;
        int v = std::stoi(s, &used);
        if (used != s.size()) throw std::invalid_argument("x");
        return v;
    } catch (const std::exception&) {
        throw ParseError(std::string("bad number in field ") + field + ": '" + s + "'");
    }
}

std::string readLine(const std::string& prompt) {
    std::cout << prompt << std::flush;
    std::string line;
    if (!std::getline(std::cin, line)) throw InputClosed();
    static const bool interactive = ELM_ISATTY(0) != 0;
    if (!interactive) std::cout << line << "\n";          // piped input (tests): echo it so transcripts read naturally
    return trim(line);
}
std::string readName(const std::string& prompt) {
    for (;;) {
        std::string s = readLine(prompt);
        if (validName(s)) return titleCase(s);
        std::cout << "  ! Name must be 2-40 characters: letters, spaces, . ' -\n";
    }
}
std::string readText(const std::string& prompt, size_t maxLen, bool allowEmpty) {
    for (;;) {
        std::string s = readLine(prompt);
        if (s.empty() && !allowEmpty) { std::cout << "  ! This field cannot be empty.\n"; continue; }
        if (s.size() > maxLen) { std::cout << "  ! Maximum " << maxLen << " characters.\n"; continue; }
        return s;
    }
}
std::string readEmail(const std::string& prompt) {
    for (;;) {
        std::string s = lower(readLine(prompt));
        if (validEmail(s)) return s;
        std::cout << "  ! Enter a valid e-mail such as name@company.com\n";
    }
}
int readInt(const std::string& prompt, int lo, int hi) {
    for (;;) {
        std::string s = readLine(prompt);
        try {
            int v = toInt(s, "input");
            if (v >= lo && v <= hi) return v;
        } catch (const ParseError&) {}
        std::cout << "  ! Enter a whole number between " << lo << " and " << hi << ".\n";
    }
}
Date readDate(const std::string& prompt) {
    for (;;) {
        Date d;
        if (Date::tryParse(readLine(prompt + " (YYYY-MM-DD): "), d)) return d;
        std::cout << "  ! Not a real calendar date. Example: 2026-12-25\n";
    }
}
bool confirm(const std::string& prompt) {
    for (;;) {
        std::string s = lower(readLine(prompt + " [y/n]: "));
        if (s == "y" || s == "yes") return true;
        if (s == "n" || s == "no") return false;
        std::cout << "  ! Please answer y or n.\n";
    }
}
void rule(char c, int n) { std::cout << std::string(static_cast<size_t>(n), c) << "\n"; }
void heading(const std::string& t) { std::cout << "\n"; rule('='); std::cout << "  " << t << "\n"; rule('='); }

}}  // namespace elm::io

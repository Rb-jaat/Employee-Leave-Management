// io.h - console input helpers, validation, string utilities (namespace elm::io).
#pragma once
#include <iosfwd>
#include <string>
#include <vector>
#include "Date.h"

namespace elm { namespace io {

// ---- string helpers ----
std::string trim(const std::string& s);
std::string lower(const std::string& s);
bool containsCI(const std::string& hay, const std::string& needle);   // case-insensitive
std::string escape(const std::string& s);        // makes text safe for tab/newline files
std::string unescape(const std::string& s);
std::vector<std::string> split(const std::string& s, char sep);
std::string pad(const std::string& s, size_t width);                   // left-aligned, truncates
std::string csvQuote(const std::string& s);
bool validEmail(const std::string& s);
bool validName(const std::string& s);            // letters, spaces, . ' -
std::string titleCase(const std::string& s);
int  toInt(const std::string& s, const char* field);                   // throws ParseError

// ---- console input (every function re-asks on bad input; EOF throws InputClosed) ----
std::string readLine(const std::string& prompt);
std::string readName(const std::string& prompt);
std::string readText(const std::string& prompt, size_t maxLen, bool allowEmpty = false);
std::string readEmail(const std::string& prompt);
int  readInt(const std::string& prompt, int lo, int hi);
Date readDate(const std::string& prompt);
bool confirm(const std::string& prompt);
void rule(char c = '-', int n = 78);
void heading(const std::string& title);

}}  // namespace elm::io

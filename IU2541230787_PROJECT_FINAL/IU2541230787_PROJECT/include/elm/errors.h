// errors.h - exception hierarchy used by every layer of the project.
#pragma once
#include <stdexcept>
#include <string>

namespace elm {

struct ElmError : std::runtime_error {            // root of all project errors
    using std::runtime_error::runtime_error;
};
struct ValidationError : ElmError { using ElmError::ElmError; };  // bad user input
struct NotFoundError   : ElmError { using ElmError::ElmError; };  // id does not exist
struct RuleError       : ElmError { using ElmError::ElmError; };  // business rule broken
struct FileError       : ElmError { using ElmError::ElmError; };  // disk problems
struct ParseError      : ElmError { using ElmError::ElmError; };  // corrupt data line
struct InputClosed     : ElmError { InputClosed() : ElmError("input closed") {} };

}  // namespace elm

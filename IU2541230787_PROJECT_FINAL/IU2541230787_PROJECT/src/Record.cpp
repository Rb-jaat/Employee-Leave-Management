#include "elm/Record.h"
#include "elm/io.h"

namespace elm {
bool Record::matches(const std::string& text) const { return io::containsCI(id(), text); }
}

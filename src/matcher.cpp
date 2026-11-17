#include "tagstream/matcher.hpp"
#include <cctype>

namespace tagstream {
std::string to_lower(std::string_view text) {
    std::string result(text);
    for (char& c : result) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return result;
}
}
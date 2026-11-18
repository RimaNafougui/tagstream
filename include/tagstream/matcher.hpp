#pragma once
#include <set>
#include <string>
#include <string_view>
#include <vector>

namespace tagstream {

struct Pattern {
    std::string text;  // E.g. Apple
    std::string tag;   // E.g. APPL
};

std::string to_lower(std::string_view text);

// returns every tag whose pattern appears somewhere in the line
std::set<std::string> naive_match(std::string_view line, const std::vector<Pattern>& patterns);

// reads a tab-seperated file: one "text<TAB>tag" per line
std::vector<Pattern> load_patterns(const std::string& path);
}  // namespace tagstream

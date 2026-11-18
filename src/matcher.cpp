#include "tagstream/matcher.hpp"

#include <cctype>
#include <cstddef>
#include <fstream>
#include <string>

namespace tagstream {

std::set<std::string> naive_match(std::string_view line, const std::vector<Pattern>& patterns) {
    std::set<std::string> tags;
    for (const Pattern& p : patterns) {
        if (p.text.empty()) {
            continue;
        }
        if (line.find(p.text) != std::string_view::npos) {
            tags.insert(p.tag);
        }
    }
    return tags;
}
std::string to_lower(std::string_view text) {
    std::string result(text);
    for (char& c : result) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return result;
}
std::vector<Pattern> load_patterns(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error("cannot open " + path);
    }
    std::vector<Pattern> patterns;
    std::string row;
    while (std::getline(file, row)) {
        std::size_t tab = row.find("\t");
        if (tab == std::string::npos) {
            continue;
        }
        patterns.push_back({row.substr(0, tab), row.substr(tab + 1)});
    }
    return patterns;
}
}  // namespace tagstream

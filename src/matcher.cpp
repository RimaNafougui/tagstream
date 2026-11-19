#include "tagstream/matcher.hpp"

#include <cctype>
#include <cstddef>
#include <fstream>
#include <string>

namespace tagstream {

std::set<std::string> naive_match(std::string_view line, const std::vector<Pattern>& patterns) {
    std::set<std::string> tags;
    std::string lower_line = to_lower(line);
    for (const Pattern& p : patterns) {
        if (p.text.empty()) {
            continue;
        }
        std::string needle = to_lower(p.text);
        std::size_t pos = lower_line.find(needle);
        while (pos != std::string::npos) {
            if (is_whole_word(lower_line, pos, pos + needle.size())) {
                tags.insert(p.tag);
                break;  // one whole-word occurrence is enough
            }
            pos = lower_line.find(needle, pos + 1);  // try the next occurrence
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

bool is_word_char(char c) { return std::isalnum(static_cast<unsigned char>(c)) != 0; }

bool is_whole_word(std::string_view line, std::size_t start, std::size_t end) {
    bool left_ok = (start == 0) || !is_word_char(line[start - 1]);
    bool right_ok = (end == line.size()) || !is_word_char(line[end]);
    return left_ok && right_ok;
}
}  // namespace tagstream

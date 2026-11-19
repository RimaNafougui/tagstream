#include <iostream>
#include <string>

#include "tagstream/automaton.hpp"
#include "tagstream/matcher.hpp"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "usage: tagstream <patterns.tsv>\n";
        return 1;
    }
    std::vector<tagstream::Pattern> patterns = tagstream::load_patterns(argv[1]);
    tagstream::Automaton automaton(patterns);

    std::string line;
    while (std::getline(std::cin, line)) {
        std::set<std::string> tags = automaton.match(line);
        std::string joined;
        for (const std::string& t : tags) {
            if (!joined.empty()) {
                joined += ",";
            }
            joined += t;
        }
        std::cout << (joined.empty() ? "-" : joined) << "\t" << line << '\n';
    }
}
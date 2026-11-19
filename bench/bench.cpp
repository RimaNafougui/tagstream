#include <algorithm>
#include <chrono>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "tagstream/automaton.hpp"
#include "tagstream/matcher.hpp"

using Clock = std::chrono::steady_clock;

namespace {

double seconds_since(Clock::time_point start) {
    return std::chrono::duration<double>(Clock::now() - start).count();
}

std::vector<std::string> read_lines(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error("cannot open " + path);
    }
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file, line)) {
        lines.push_back(line);
    }
    return lines;
}

}  // namespace

int main(int argc, char* argv[]) {
    if (argc != 3 && argc != 4) {
        std::cerr << "usage: tagstream_bench <patterns.tsv> <input.txt> [naive_lines]\n";
        return 1;
    }
    std::vector<tagstream::Pattern> patterns = tagstream::load_patterns(argv[1]);
    std::vector<std::string> lines = read_lines(argv[2]);
    std::size_t naive_lines = (argc == 4) ? std::stoul(argv[3]) : 2000;
    naive_lines = std::min(naive_lines, lines.size());

    std::size_t bytes = 0;
    for (const std::string& l : lines) {
        bytes += l.size() + 1;
    }

    // 1. Build the automaton once.
    Clock::time_point t = Clock::now();
    tagstream::Automaton automaton(patterns);
    double build_s = seconds_since(t);

    // 2. Automaton on every line, best of 3 runs.
    double automaton_s = 1e9;
    std::size_t automaton_found = 0;
    for (int run = 0; run < 3; ++run) {
        std::size_t found = 0;
        t = Clock::now();
        for (const std::string& l : lines) {
            found += automaton.match(l).size();
        }
        automaton_s = std::min(automaton_s, seconds_since(t));
        automaton_found = found;
    }

    // 3. Naive on a subset only (it is too slow for every line).
    std::size_t naive_found = 0;
    t = Clock::now();
    for (std::size_t i = 0; i < naive_lines; ++i) {
        naive_found += tagstream::naive_match(lines[i], patterns).size();
    }
    double naive_s = seconds_since(t);

    // 4. Sanity check: both must find the same tags on that subset.
    std::size_t check = 0;
    for (std::size_t i = 0; i < naive_lines; ++i) {
        check += automaton.match(lines[i]).size();
    }

    double automaton_lps = lines.size() / automaton_s;
    double naive_lps = naive_lines / naive_s;

    std::cout << "patterns:            " << patterns.size() << '\n'
              << "lines:               " << lines.size() << '\n'
              << "automaton nodes:     " << automaton.node_count() << '\n'
              << "build time:          " << build_s * 1000 << " ms\n"
              << "automaton:           " << automaton_lps << " lines/s, "
              << (bytes / 1e6) / automaton_s << " MB/s\n"
              << "naive (" << naive_lines << " lines): " << naive_lps << " lines/s\n"
              << "speedup:             " << automaton_lps / naive_lps << "x\n"
              << "tags found:          " << automaton_found << '\n'
              << "check on subset:     " << (check == naive_found ? "OK" : "MISMATCH") << '\n'
              << "table size:          " << automaton.table_bytes() / 1e6 << " MB\n";
}
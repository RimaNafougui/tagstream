#pragma once
#include <array>
#include <cstddef>
#include <map>
#include <set>
#include <string>
#include <string_view>
#include <vector>

#include "tagstream/matcher.hpp"

namespace tagstream {

class Automaton {
   public:
    explicit Automaton(const std::vector<Pattern>& patterns);

    // Follows "text" from the root. returns the node reached, or -1 if the path doesn't exist
    int walk(std::string_view text) const;

    // Indices(into the pattern list) of patterns that end exactly at 'node'
    const std::vector<int>& outputs_at(int node) const;

    std::size_t node_count() const;

    // returns every tag whose pattern appears in the line
    std::set<std::string> match(std::string_view line) const;

    int fail_of(int node) const;
    std::size_t table_bytes() const;

   private:
    struct Node {
        std::map<char, int> children;  // a char -> index of a child node
        std::vector<int> outputs;      // patterns ending here
        int fail = 0;
    };

    void insert(const std::string& text, int pattern_index);

    void build_failure_links();

    std::vector<Node> nodes_;
    std::vector<Pattern> patterns_;
    void build_table();

    std::array<int, 256> column_{};  // byte -> column (0 = in no pattern)
    int width_ = 1;                  // number of columns per node
    std::vector<int> table_;         // table_[node * width_ + column] = next node
    std::vector<int> bfs_order_;     // nodes in BFS order, filled by build_failure_links
};

}  // namespace tagstream

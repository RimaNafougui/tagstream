#pragma once
#include <cstddef>
#include <map>
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

   private:
    struct Node {
        std::map<char, int> children;  // a char -> index of a child node
        std::vector<int> outputs;      // patterns ending here
    };

    void insert(const std::string& text, int pattern_index);

    std::vector<Node> nodes_;
    std::vector<Pattern> patterns_;
};

}  // namespace tagstream

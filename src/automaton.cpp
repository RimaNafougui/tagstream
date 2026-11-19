#include "tagstream/automaton.hpp"
namespace tagstream {

// five functions

// Constructor
Automaton::Automaton(const std::vector<Pattern>& patterns) : patterns_(patterns) {
    nodes_.emplace_back();
    for (int i = 0; i < static_cast<int>(patterns.size()); ++i) {
        if (patterns[i].text.empty()) {
            continue;
        }
        insert(patterns[i].text, i);
    }
}
// insert
void Automaton::insert(const std::string& text, int pattern_index) {
    int cur = 0;
    for (char c : text) {
        auto it = nodes_[cur].children.find(c);
        if (it == nodes_[cur].children.end()) {
            // no child for this char yet: create one
            int next = static_cast<int>(nodes_.size());  // its index will be the curr size
            nodes_.emplace_back();
            nodes_[cur].children[c] = next;
            cur = next;
        } else {
            cur = it->second;
        }
    }
    nodes_[cur].outputs.push_back(pattern_index);  // a pattern ends here
}
// walk
int Automaton::walk(std::string_view text) const {
    int cur = 0;
    for (char c : text) {
        auto it = nodes_[cur].children.find(c);
        if (it == nodes_[cur].children.end()) {
            return -1;  // path doesn't exist
        }
        cur = it->second;
    }
    return cur;
}
// output_as
const std::vector<int>& Automaton::outputs_at(int node) const { return nodes_[node].outputs; }

// node_count
std::size_t Automaton::node_count() const { return nodes_.size(); }

}  // namespace tagstream
#include "tagstream/automaton.hpp"

#include <cctype>
#include <queue>
namespace tagstream {


Automaton::Automaton(const std::vector<Pattern>& patterns) : patterns_(patterns) {
    nodes_.emplace_back();
    for (int i = 0; i < static_cast<int>(patterns.size()); ++i) {
        if (patterns[i].text.empty()) {
            continue;
        }
        insert(to_lower(patterns[i].text), i);
    }
    build_failure_links();
    build_table();
}

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

const std::vector<int>& Automaton::outputs_at(int node) const { return nodes_[node].outputs; }

std::size_t Automaton::node_count() const { return nodes_.size(); }

void Automaton::build_failure_links() {
    std::queue<int> q;
    bfs_order_.push_back(0);

    // Depth 1: every child of the root fails to the root.
    for (const auto& [c, child] : nodes_[0].children) {
        nodes_[child].fail = 0;
        q.push(child);
    }

    // Deeper levels, in BFS order.
    while (!q.empty()) {
        int u = q.front();
        q.pop();
        bfs_order_.push_back(u);
        for (const auto& [c, v] : nodes_[u].children) {
            // Follow u's failure chain until some node has a child for c.
            int f = nodes_[u].fail;
            while (f != 0 && nodes_[f].children.count(c) == 0) {
                f = nodes_[f].fail;
            }
            auto it = nodes_[f].children.find(c);
            nodes_[v].fail = (it != nodes_[f].children.end()) ? it->second : 0;

            // Inherit the outputs of the failure node.
            const std::vector<int>& inherited = nodes_[nodes_[v].fail].outputs;
            nodes_[v].outputs.insert(nodes_[v].outputs.end(), inherited.begin(), inherited.end());

            q.push(v);
        }
    }
}

std::set<std::string> Automaton::match(std::string_view line) const {
    std::set<std::string> tags;
    int cur = 0;
    for (std::size_t i = 0; i < line.size(); ++i) {
        unsigned char b =
            static_cast<unsigned char>(std::tolower(static_cast<unsigned char>(line[i])));
        cur = table_[static_cast<std::size_t>(cur) * static_cast<std::size_t>(width_) + column_[b]];

        for (int idx : nodes_[cur].outputs) {
            std::size_t end = i + 1;  // match ends after this character
            std::size_t start = end - patterns_[idx].text.size();
            if (is_whole_word(line, start, end)) {
                tags.insert(patterns_[idx].tag);
            }
        }
    }
    return tags;
}

int Automaton::fail_of(int node) const { return nodes_[node].fail; }

void Automaton::build_table() {
    // 1. Give each distinct character used in the trie its own column.
    column_.fill(0);
    width_ = 1;  // column 0 is reserved for characters in no pattern
    for (const Node& node : nodes_) {
        for (const auto& [c, child] : node.children) {
            unsigned char b = static_cast<unsigned char>(c);
            if (column_[b] == 0) {
                column_[b] = width_++;
            }
        }
    }

    // 2. Fill the table in BFS order, so each node's failure target is already done.
    std::size_t w = static_cast<std::size_t>(width_);
    table_.assign(nodes_.size() * w, 0);  // everything starts pointing at the root
    for (int u : bfs_order_) {
        std::size_t row = static_cast<std::size_t>(u) * w;
        if (u != 0) {
            // No child for this character? Do what the failure node would do.
            std::size_t fail_row = static_cast<std::size_t>(nodes_[u].fail) * w;
            for (std::size_t col = 1; col < w; ++col) {
                table_[row + col] = table_[fail_row + col];
            }
        }
        // Real children override the inherited transitions.
        for (const auto& [c, child] : nodes_[u].children) {
            table_[row + column_[static_cast<unsigned char>(c)]] = child;
        }
    }
}
std::size_t Automaton::table_bytes() const { return table_.size() * sizeof(int); }
}  // namespace tagstream

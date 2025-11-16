#pragma once

#include <vector>
#include <stack>
#include <queue>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>

// ----------------------------------------------
// Directed or Undirected Graph
// ----------------------------------------------

template<typename T>
class Graph final {
public:
    // If directed = false, edges are added both ways.
    explicit Graph(const bool directed = false)
        : m_is_directed{directed}
    {}

    // Add an edge u -> v (and v -> u if undirected).
    auto add_edge(const T& u, const T& v) -> void {
        // Note: No duplicate connections allowed.
        m_adjacency[u].add_unique_neighbor(v);

        if (!m_is_directed) {
            m_adjacency[v].add_unique_neighbor(u);
        }
    }

    // Add an isolated vertex with no edges.
    auto add_vertex(const T& v) -> void {
        m_adjacency.try_emplace(v, Connections{});
    }

    // Get neighbors of a vertex.
    auto neighbors(const T& v) const -> const std::vector<T>& {
        static const std::vector<T> s_empty{};
        if (const auto it = m_adjacency.find(v); it != m_adjacency.end()) {
            return it->second.neighbors;
        }
        return s_empty;
    }

    auto has_vertex(const T& v) const -> bool {
        return m_adjacency.contains(v);
    }

    auto has_cycle() const -> bool {
        if (m_is_directed) {
            return has_cycle_directed();
        } else {
            return has_cycle_undirected();
        }
    }

    auto is_directed() const -> bool {
        return m_is_directed;
    }

    // Iterative DFS.
    template<typename VisitorFn>
    auto visit_depth_first(const T& start, const VisitorFn& visitor) {
        std::unordered_set<T> visited{};
        std::stack<T> stack{};

        stack.push(start);

        while (!stack.empty()) {
            auto v = stack.top();
            stack.pop();

            if (visited.contains(v)) {
                continue;
            }

            if (!visitor(v)) {
                return; // Stop traversal.
            }

            visited.insert(v);

            // To match recursive DFS order:
            // - push neighbors in REVERSE so the first neighbor is processed first.
            const auto& n = neighbors(v);
            for (auto it = n.rbegin(); it != n.rend(); ++it) {
                if (!visited.contains(*it)) {
                    stack.push(*it);
                }
            }
        }
    }

    // Iterative BFS.
    template<typename VisitorFn>
    auto visit_breadth_first(const T& start, const VisitorFn& visitor) {
        std::unordered_set<T> visited{};
        std::queue<T> queue{};

        queue.push(start);

        while (!queue.empty()) {
            auto v = queue.front();
            queue.pop();

            if (visited.contains(v)) {
                continue;
            }

            if (!visitor(v)) {
                return; // Stop traversal.
            }

            visited.insert(v);

            for (const auto& n : neighbors(v)) {
                if (!visited.contains(n)) {
                    queue.push(n);
                }
            }
        }
    }

    auto dijkstra_shortest_path(const T& start, const T& goal) -> std::vector<T> {
        std::unordered_map<T, float> dist{};
        std::unordered_map<T, std::optional<T>> prev{};

        using Pair = std::pair<float, T>; // (distance, vertex)
        std::priority_queue<Pair, std::vector<Pair>, std::greater<Pair>> pq{};

        // Initialize all distances:
        for (const auto& [vertex, _] : m_adjacency) {
            dist[vertex] = std::numeric_limits<float>::infinity();
            prev[vertex] = std::nullopt;
        }

        // Ensure start/goal exist in the maps:
        dist[start] = std::numeric_limits<float>::infinity();
        prev[start] = std::nullopt;

        dist[goal] = std::numeric_limits<float>::infinity();
        prev[goal] = std::nullopt;

        dist[start] = 0.0f;
        pq.emplace(0.0f, start);

        while (!pq.empty()) {
            auto [dist_v, v] = pq.top();
            pq.pop();

            if (v == goal) {
                // We found the shortest path to goal.
                break;
            }

            if (dist_v > dist[v]) {
                continue;
            }

            for (const auto& n : neighbors(v)) {
                const auto new_dist = dist_v + 1.0f; // Fixed weight/cost.
                if (new_dist < dist[n]) {
                    dist[n] = new_dist;
                    prev[n] = v;
                    pq.emplace(new_dist, n);
                }
            }
        }

        // If goal is unreachable, return empty path immediately.
        if (dist.find(goal) == dist.end() ||
            dist[goal] == std::numeric_limits<float>::infinity()) {
            return {}; // no path.
        }

        return reconstruct_path(prev, start, goal);
    }

private:
    static auto reconstruct_path(const std::unordered_map<T, std::optional<T>>& prev,
                                 const T& start,
                                 const T& goal) -> std::vector<T> {
        std::vector<T> path{};

        // Walk backwards from goal to start.
        // If goal == start, ensure we still return [start].
        T current = goal;
        for (;;) {
            path.push_back(current);
            if (current == start) {
                break; // done
            }

            // If there's no predecessor for current, the path is malformed.
            const auto it = prev.find(current);
            if (it == prev.end() || !it->second.has_value()) {
                return {}; // no path / inconsistent.
            }

            current = it->second.value();
        }

        std::reverse(path.begin(), path.end());
        return path;
    }

    auto has_cycle_directed() const -> bool {
        std::unordered_set<T> visited{};
        std::unordered_set<T> recursion_stack{};

        for (const auto& [vertex, _] : m_adjacency) {
            if (!visited.contains(vertex)) {
                if (has_cycle_directed_dfs_recursive(visited, recursion_stack, vertex)) {
                    return true;
                }
            }
        }

        return false;
    }

    auto has_cycle_directed_dfs_recursive(std::unordered_set<T>& visited,
                                          std::unordered_set<T>& recursion_stack,
                                          const T& v) const -> bool {
        visited.insert(v);
        recursion_stack.insert(v);

        for (const auto& n : neighbors(v)) {
            if (!visited.contains(n)) {
                if (has_cycle_directed_dfs_recursive(visited, recursion_stack, n)) {
                    return true;
                }
            } else if (recursion_stack.contains(n)) {
                return true; // back edge to node in recursion stack.
            }
        }

        recursion_stack.erase(v);
        return false;
    }

    auto has_cycle_undirected() const -> bool {
        std::unordered_set<T> visited{};

        for (const auto& [vertex, _] : m_adjacency) {
            if (!visited.contains(vertex)) {
                if (has_cycle_undirected_dfs_recursive(visited, vertex, vertex)) {
                    return true;
                }
            }
        }

        return false;
    }

    auto has_cycle_undirected_dfs_recursive(std::unordered_set<T>& visited,
                                            const T& v,
                                            const T& parent) const -> bool {
        visited.insert(v);

        for (const auto& n : neighbors(v)) {
            if (!visited.contains(n)) {
                if (has_cycle_undirected_dfs_recursive(visited, n, v)) {
                    return true;
                }
            } else if (n != parent) {
                return true; // back edge found.
            }
        }

        return false;
    }

private:
    struct Connections {
        Connections() = default;

        std::vector<T> neighbors{};

        auto add_unique_neighbor(const T& neighbor) -> void {
            assert(!has_neighbor(neighbor) && "Neighbor vertex already added!");
            neighbors.push_back(neighbor);
        }

        auto has_neighbor(const T& neighbor) const -> bool {
            return std::find(neighbors.begin(), neighbors.end(), neighbor) != neighbors.end();
        }
    };

    std::unordered_map<T, Connections> m_adjacency{};
    const bool m_is_directed;
};

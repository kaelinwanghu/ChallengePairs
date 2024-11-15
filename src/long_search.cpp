// search.cpp

#include "long_search.hpp"

namespace long_search
{
    inline uint32_t compute_rank(const Graph& graph, uint32_t node_id)
    {
        return graph.get_scc_diameter(node_id);
    }

    std::pair<uint32_t, uint32_t> bfs_search(const Graph& graph, uint32_t start_node)
    {
        constexpr uint32_t TOP_PATHS = 64;

        // Stores sink nodes and their estimated total path lengths
        std::vector<std::pair<uint32_t, uint32_t>> sink_paths; // (sink_node, estimated_length)
        sink_paths.reserve(50000); // Trust me

        // Stores visited nodes
        emhash8::HashSet<uint32_t, XXIntHasher> visited;
        visited.reserve(graph.size());

        // Priority queue to process nodes with largest estimated total path lengths first
        // Priority queue stores pairs of (estimated_length, node_id)
        std::deque<std::pair<uint32_t, uint32_t>> bfs_queue;

        // Initialize the priority queue with the start node, always guaranteed to be a single node
        bfs_queue.emplace_back(1, start_node);
        visited.insert(start_node);

        while (!bfs_queue.empty())
        {
            const auto [current_estimated_length, current_node] = bfs_queue.back();
            bfs_queue.pop_back();

            const auto& successors = graph.successors(current_node);

                // If current node is a sink node (no successors), potentially long path so store it
            if (successors.empty())
            {
                sink_paths.emplace_back(current_node, current_estimated_length);
                continue;
            }

            // For each successor
            for (auto it = successors.rbegin(); it != successors.rend(); ++it)
            {
                const uint32_t successor = *it;
                if (visited.insert(successor).second)
                {
                    uint32_t successor_diameter = graph.get_scc_diameter(successor);
                    bfs_queue.emplace_back(current_estimated_length + successor_diameter, successor);
                }
            }
        }

        // Keep only the top TOP_PATHS sink nodes with the largest estimated lengths
        if (sink_paths.size() > TOP_PATHS)
        {
            std::nth_element(sink_paths.begin(),
                sink_paths.begin() + TOP_PATHS - 1,
                sink_paths.end(),
                [](const auto& a, const auto& b) {
                    return a.second > b.second;
                });
        }
        
        // Evaluate the actual shortest paths for the top sink nodes
        std::pair<uint32_t, uint32_t> best_result = {0, 0}; // (path_length, sink_node)
        for (const auto& [sink_node, estimated_length] : sink_paths)
        {
            const std::deque<uint32_t> path = graph.shortest_path(start_node, sink_node);
            const uint32_t path_length = static_cast<uint32_t>(path.size());
            if (path_length > best_result.first)
            {
                best_result = {path_length, sink_node};
            }
        }
        return best_result;
    }
}
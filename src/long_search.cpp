// search.cpp

#include "long_search.hpp"

namespace long_search
{
    inline uint32_t compute_rank(const Graph& graph, uint32_t node_id)
    {
        return graph.get_scc_diameter(node_id);
    }

    uint32_t bfs_search(const Graph& graph, uint32_t start_node, std::vector<uint32_t>& longest_chain)
    {
        const size_t TOP_PATHS = 20; // Number of top paths to evaluate

        // Store sink nodes and their estimated lengths
        std::vector<std::pair<uint32_t, uint32_t>> sink_nodes_with_estimates;

        emhash8::HashSet<uint32_t, XXIntHasher> visited;

        // Priority queue: (negative estimated length for min-heap, current node, current path length)
        std::priority_queue<std::tuple<int32_t, uint32_t, uint32_t>> bfs_queue;

        // Initialize the BFS with the priority queue
        uint32_t scc_diameter = graph.get_scc_diameter(start_node);
        bfs_queue.emplace(-scc_diameter, start_node, 1);
        visited.insert(start_node);

        while (!bfs_queue.empty())
        {
            auto [negative_estimated_length, current_node, current_length] = bfs_queue.top();
            bfs_queue.pop();

            // Check if current_node is a sink node
            if (graph.out_degree(current_node) == 0)
            {
                // Store sink node and estimated length
                sink_nodes_with_estimates.emplace_back(current_node, -negative_estimated_length);
                continue;
            }

            // Get successors of the current node
            const auto& successors_set = graph.successors(current_node);
            std::vector<uint32_t> successors(successors_set.begin(), successors_set.end());

            // Sort successors based on SCC diameter (probably more accurate than size)
            std::sort(successors.begin(), successors.end(),
                [&graph](uint32_t a, uint32_t b) {
                    return graph.get_scc_diameter(a) > graph.get_scc_diameter(b);
                });

            for (uint32_t successor : successors)
            {
                // Ensure linear runtime by processing it only if it has not already been visited
                if (visited.find(successor) == visited.end())
                {
                    visited.insert(successor);

                    // Estimate the total length
                    uint32_t successor_scc_diameter = graph.get_scc_diameter(successor);
                    uint32_t estimated_length = current_length + successor_scc_diameter;

                    // Push onto the priority queue
                    bfs_queue.emplace(-estimated_length, successor, current_length + 1);
                }
            }
        }

        // Sort sink nodes by estimated length in descending order
        std::sort(sink_nodes_with_estimates.begin(), sink_nodes_with_estimates.end(),
            [](const std::pair<uint32_t, uint32_t>& a, const std::pair<uint32_t, uint32_t>& b) {
                return a.second > b.second;
            });

        // Keep only the top sink node paths
        if (sink_nodes_with_estimates.size() > TOP_PATHS)
        {
            sink_nodes_with_estimates.resize(TOP_PATHS);
        }

        // Then evaluate the actual shortest paths for the top sink nodes
        uint32_t max_path_length = 0;
        std::vector<uint32_t> best_path;

    for (const auto& [sink_node, estimated_length] : sink_nodes_with_estimates)
    {
        // Compute the shortest path from start_node to sink_node
        std::deque<uint32_t> path = graph.shortest_path(start_node, sink_node);

        uint32_t path_length = static_cast<uint32_t>(path.size());

        if (path_length > max_path_length)
        {
            max_path_length = path_length;

            // Store the best path
            best_path.assign(path.begin(), path.end());
        }
    }
        // Set the longest chain to the best path found
        longest_chain = best_path;

        // Return the length of the longest path
        return max_path_length;
    }
}

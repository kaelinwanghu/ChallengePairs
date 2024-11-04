// search.cpp

#include "long_search.hpp"

namespace long_search
{
    // Wrapper function for both of the phases, they are separate function because the secondary select might be discarded
    const std::vector<uint32_t> get_search_nodes(const Graph& graph)
    {
        return rank_nodes(graph);
    }

    // Ranks all initial nodes based on the combined sizes of successor and predecessor lists
    std::vector<uint32_t> rank_nodes(const Graph& graph)
    {
        // Collect all node IDs into a vector
        std::vector<uint32_t> node_ids;
        node_ids.reserve(graph.size());
        for (auto it = graph.node_begin(); it != graph.node_end(); ++it)
        {
            node_ids.emplace_back(it->first);
        }

        // Sort the node_ids based on rank in descending order using a lambda
        std::sort(node_ids.begin(), node_ids.end(),
        [&graph](uint32_t node1, uint32_t node2)
            {
                return compute_rank(graph, node1) > compute_rank(graph, node2);
            });

        // Calculate the number of nodes to select
        size_t num_nodes = node_ids.size();
        size_t num_selected_nodes = static_cast<size_t>(std::ceil(num_nodes * PERCENTILE));
        if (num_selected_nodes > num_nodes)
        {
            num_selected_nodes = num_nodes; // Failsafe
        }

        // Truncate the vector to the desired size
        node_ids.resize(num_selected_nodes);

        return node_ids;
    }

    // Inline function to compute the rank of a node
    inline uint32_t compute_rank(const Graph& graph, uint32_t node_id)
    {
        return graph.out_degree(node_id) + graph.in_degree(node_id);
    }

    // Limited BFS search for a node both backwards and forwards to get the longest_chain
    uint32_t bfs_search(const Graph& graph, uint32_t start_node, std::pair<uint32_t, uint32_t>& longest_chain)
    {
        constexpr size_t MAX_BRANCH_FACTOR = 10;

        // Forward search (successors)
        uint32_t max_forward_length = 1;
        uint32_t forward_end_node = start_node;
        emhash8::HashSet<uint32_t, XXIntHasher> visited;
        {
            std::queue<std::pair<uint32_t, uint32_t>> bfs_queue; // [current_node, path_length]
            bfs_queue.emplace(start_node, 1);
            visited.insert(start_node);
            while (!bfs_queue.empty())
            {
                auto [current_node, path_length] = bfs_queue.front();
                bfs_queue.pop();

                if (path_length > max_forward_length)
                {
                    max_forward_length = path_length;
                    forward_end_node = current_node;
                }
                // Get successors and sort them based on rank
                const auto& successors = graph.successors(current_node);
                std::vector<uint32_t> sorted_successors(successors.begin(), successors.end());
                std::sort(sorted_successors.begin(), sorted_successors.end(),
                    [&graph](uint32_t a, uint32_t b)
                    {
                        return compute_rank(graph, a) > compute_rank(graph, b);
                    });

                // Limit the number of successors to explore
                size_t num_successors = sorted_successors.size();

                // Continually add the successors while they are available and the limit has not been reached
                size_t added_successors = 0;
                size_t i = 0;
                while (added_successors < num_successors && i < sorted_successors.size())
                {
                    uint32_t successor = sorted_successors[i];
                    if (visited.find(successor) == visited.end())
                    {
                        // Add onto the stack with extra path length
                        bfs_queue.emplace(successor, path_length + 1);
                        visited.insert(successor);
                        ++added_successors;
                    }
                    ++i; // Always increment index
                }
            }
        }

        // Backward search (predecessors)
        uint32_t max_backward_length = 1;
        uint32_t backward_end_node = start_node;
        visited.clear();

        {
            std::queue<std::pair<uint32_t, uint32_t>> bfs_queue; // [current_node, path_length]

            bfs_queue.emplace(start_node, 1);
            visited.insert(start_node);

            while (!bfs_queue.empty())
            {
                auto [current_node, path_length] = bfs_queue.front();
                bfs_queue.pop();

                if (path_length > max_backward_length)
                {
                    max_backward_length = path_length;
                    backward_end_node = current_node;
                }

                // Get predecessors and sort them based on rank
                const auto& predecessors = graph.predecessors(current_node);
                std::vector<uint32_t> sorted_predecessors(predecessors.begin(), predecessors.end());
                std::sort(sorted_predecessors.begin(), sorted_predecessors.end(),
                    [&graph](uint32_t a, uint32_t b)
                    {
                        return compute_rank(graph, a) > compute_rank(graph, b);
                    });

                // Limit the number of predecessors to explore
                size_t num_predecessors = sorted_predecessors.size();

                // Continually add the successors while they are available and the limit has not been reached
                size_t added_predecessors = 0;
                size_t i = 0;
                while (added_predecessors < num_predecessors && i < sorted_predecessors.size())
                {
                    uint32_t predecessor = sorted_predecessors[i];
                    if (visited.find(predecessor) == visited.end())
                    {
                        bfs_queue.emplace(predecessor, path_length + 1);
                        visited.insert(predecessor);
                        ++added_predecessors;
                    }
                    ++i;
                }
            }
        }

        // Combine the results
        uint32_t total_length = max_backward_length + max_forward_length - 2; // avoid start_node double counting
        longest_chain.first = backward_end_node; // Start node of the chain
        longest_chain.second = forward_end_node; // End node of the chain

        // port over the shortest path to check afterwards

        return total_length;
    }
}

// search.cpp

#include "long_search.hpp"

namespace long_search
{
    // Wrapper function for both of the phases, they are separate function because the secondary select might be discarded
    const std::vector<uint32_t> get_search_nodes(const Graph& graph)
    {
        return rank_nodes(graph);
        // return refine_nodes(graph, node_ids);
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
        size_t num_selected_nodes = static_cast<size_t>(std::ceil(num_nodes * PERCENTILE * INITIAL_INCLUSION));
        if (num_selected_nodes > num_nodes)
        {
            num_selected_nodes = num_nodes; // Failsafe
        }

        // Truncate the vector to the desired size
        node_ids.resize(num_selected_nodes);

        return node_ids;
    }


    // // Refines the list of nodes by considering second-layer connections as well
    // std::vector<uint32_t> refine_nodes(const Graph& graph, std::vector<uint32_t>& nodes)
    // {
    //     // Again ort nodes in-place using a custom comparator
    //     std::sort(nodes.begin(), nodes.end(), [&graph](uint32_t node1, uint32_t node2)
    //     {
    //         double rank1 = compute_refined_rank(graph, node1); // More thorough rank computation for top nodes
    //         double rank2 = compute_refined_rank(graph, node2);
    //         return rank1 > rank2; // Descending order for another truncation
    //     });

    //     // Calculate the number of nodes to select
    //     size_t num_node = nodes.size();
    //     size_t num_selected_nodes = static_cast<size_t>(std::ceil(num_node / INITIAL_INCLUSION));

    //     // Truncate the vector to final search size
    //     nodes.resize(num_selected_nodes);

    //     return nodes;
    // }

    // Inline function to compute the rank of a node
    inline uint32_t compute_rank(const Graph& graph, uint32_t node_id)
    {
        return graph.out_degree(node_id) + graph.in_degree(node_id);
    }

    /* Inline function to compute the refined rank of a node passing the first round.
    Doing this because the concentration of values will be relatively high (i.e most people will have between 0-20 links) */
    inline uint32_t compute_refined_rank(const Graph& graph, uint32_t node_id)
    {
        uint32_t first_rank = compute_rank(graph, node_id);

        // Second-degree ranks 
        uint32_t largest_predecessor_in_degree = 0;
        uint32_t largest_successor_in_degree = 0;
        // Get predecessors max
        emhash8::HashSet<uint32_t, XXIntHasher> predecessors = graph.predecessors(node_id);
        for (uint32_t predecessor_id : predecessors)
        {
            uint32_t predecessor_in_degree = graph.in_degree(predecessor_id);
            if (predecessor_in_degree > largest_predecessor_in_degree)
            {
                largest_predecessor_in_degree = predecessor_in_degree;
            }
        }
        // Get successors max
        emhash8::HashSet<uint32_t, XXIntHasher> successors = graph.successors(node_id);
        for (uint32_t successor_id : successors)
        {
            uint32_t successor_in_degree = graph.in_degree(successor_id);
            if (successor_in_degree > largest_successor_in_degree)
            {
                largest_successor_in_degree = successor_in_degree;
            }
        }

        // Compute the refined rank with weighting (second weighs less than first but should still be worth something)
        return first_rank + (largest_predecessor_in_degree + largest_successor_in_degree) / 2; // Integer division but it should not matter too much
    }

    // Limited BFS search for a node both backwards and forwards to get the longest_chain
    uint32_t bfs_search(const Graph& graph, uint32_t start_node, std::pair<uint32_t, uint32_t>& longest_chain)
    {
        // Search heuristics
        constexpr size_t MAX_BRANCH_FACTOR = 10;
        constexpr size_t MIN_BRANCH_FACTOR = 2; // Try to ensure at least 2 paths

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
                // size_t num_successors = std::max(MIN_BRANCH_FACTOR, sorted_successors.size() / MAX_BRANCH_FACTOR);
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
                // size_t num_predecessors = std::max(MIN_BRANCH_FACTOR, sorted_predecessors.size() / MAX_BRANCH_FACTOR);
                // num_predecessors = std::min(num_predecessors, MAX_BRANCH_FACTOR);
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

        return total_length;
    }
}

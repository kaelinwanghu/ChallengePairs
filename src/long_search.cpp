// search.cpp

#include "long_search.hpp"

namespace long_search
{
    // Wrapper function for both of the phases, they are separate function because the secondary select might be discarded
    const std::vector<uint32_t> get_search_nodes(const Graph& graph)
    {
        return refine_nodes(graph, rank_nodes(graph));
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


    // Refines the list of nodes by considering second-layer connections as well
    std::vector<uint32_t> refine_nodes(const Graph& graph, std::vector<uint32_t>& nodes)
    {
        // Again ort nodes in-place using a custom comparator
        std::sort(nodes.begin(), nodes.end(), [&graph](uint32_t node1, uint32_t node2)
        {
            double rank1 = compute_refined_rank(graph, node1); // More thorough rank computation for top nodes
            double rank2 = compute_refined_rank(graph, node2);
            return rank1 > rank2; // Descending order for another truncation
        });

        // Calculate the number of nodes to select
        size_t num_node = nodes.size();
        size_t num_selected_nodes = static_cast<size_t>(std::ceil(num_node / INITIAL_INCLUSION));

        // Truncate the vector to final search size
        nodes.resize(num_selected_nodes);

        return nodes;
    }

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

        // Performs DFS from a starting node and stores the longest chain found
    uint32_t dfs_search(const Graph& graph, uint32_t start_node, std::pair<uint32_t, uint32_t>& longest_chain, std::mutex& result_mutex)
    {
        // Stack for iterative DFS: each element is (current_node, path_length, chain_start_node)
        std::stack<std::tuple<uint32_t, uint32_t, uint32_t>> dfs_stack;
        dfs_stack.emplace(start_node, 1, start_node);
        // Keep track of the maximum path length found and the corresponding end node
        uint32_t max_length = 0;
        uint32_t end_node = start_node;
        // Visited set to avoid revisiting nodes
        emhash8::HashSet<uint32_t, XXIntHasher> visited;

        // Iterative DFS
        while (!dfs_stack.empty())
        {
            auto [current_node, path_length, chain_start_node] = dfs_stack.top();
            dfs_stack.pop();

            // If we've found a longer path, update max_length and end_node
            if (path_length > max_length)
            {
                max_length = path_length;
                end_node = current_node;
            }

            // Mark the current node as visited
            visited.insert(current_node);

            // Get the successors of the current node
            const auto& successors = graph.successors(current_node);

            for (uint32_t successor : successors)
            {
                if (visited.find(successor) == visited.end())
                {
                    // Push the successor onto the stack with an incremented path length
                    dfs_stack.emplace(successor, path_length + 1, chain_start_node);
                }
            }

            // Remove the current node from the visited set to allow other paths
            visited.erase(current_node);
        }
        // Update the longest chain using the result mutex for thread safety
        {
            std::lock_guard<std::mutex> lock(result_mutex);
            if (max_length > longest_chain.second)
            {
                longest_chain.first = start_node;
                longest_chain.second = max_length;
            }
        }
        return max_length;
    }
}

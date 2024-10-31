// search.cpp

#include "long_search.hpp"

namespace long_search
{
    // Wrapper function for both of the phases, they are separate function because the secondary select might be discarded
    std::vector<uint32_t> get_search_nodes(const Graph& graph)
    {
        refine_nodes(graph, rank_nodes(graph));
    }

    // Ranks all initial nodes based on the combined sizes of successor and predecessor lists
    std::vector<uint32_t> rank_nodes(const Graph& graph)
    {
        std::vector<uint32_t> node_ids = graph.get_all_nodes();

        // Sort in-place with custom comparator custom comparator
        std::sort(node_ids.begin(), node_ids.end(), [&graph](uint32_t node1, uint32_t node2)
        {
            uint32_t rank1 = compute_rank(graph, node1);
            uint32_t rank2 = compute_rank(graph, node2);
            return rank1 > rank2; // Descending order for easy truncation
        });

        // Calculate the number of nodes to select based on the long_search consts
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
        emhash8::HashSet<uint32_t, XXIntHasher> predecessors = graph.predecessor_set(node_id);
        for (uint32_t pred_id : predecessors)
        {
            uint32_t predecessor_in_degree = graph.in_degree(pred_id);
            if (predecessor_in_degree > largest_predecessor_in_degree)
            {
                largest_predecessor_in_degree = predecessor_in_degree;
            }
        }
        // Get successors max
        emhash8::HashSet<uint32_t, XXIntHasher> successors = graph.successor_set(node_id);
        for (uint32_t pred_id : successors)
        {
            uint32_t successor_in_degree = graph.in_degree(pred_id);
            if (successor_in_degree > largest_successor_in_degree)
            {
                largest_successor_in_degree = successor_in_degree;
            }
        }

        // Compute the refined rank with weighting (second weighs less than first but should still be worth something)
        return first_rank + (largest_predecessor_in_degree + largest_successor_in_degree) / 2;

    }
}

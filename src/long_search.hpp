#pragma once

#include <queue>
#include <thread>
#include <functional>
#include <climits>
#include <cmath>
#include <mutex>
#include <stack>
#include "graph.hpp"

namespace long_search
{

    const std::vector<uint32_t> get_search_nodes(const Graph& graph);

    // Ranks all initial nodes based on the combined sizes of successor and predecessor lists
    std::vector<uint32_t> rank_nodes(const Graph& graph);

    // Refines the list of nodes by considering second-layer connections for the top-connected nodes
    std::vector<uint32_t> refine_nodes(const Graph& graph, const std::vector<uint32_t>& nodes);

    inline uint32_t compute_rank(const Graph& graph, uint32_t node_id);

    inline uint32_t compute_refined_rank(const Graph& graph, uint32_t node_id);

    // Performs DFS from a starting node and stores the longest chain found
    void dfs_search(const Graph& graph, uint32_t start_node, std::vector<uint32_t>& longest_chain, std::mutex& result_mutex);

    // Multithreading DFS search manager to start and manage the dfs_search on multiple threads
    void multithread_search(const Graph& graph, const std::vector<uint32_t>& start_nodes, std::vector<std::vector<uint32_t>>& results);

    constexpr uint32_t MIN_CHAIN_LENGTH = 8;
    constexpr double PERCENTILE = 0.001;
    constexpr double INITIAL_INCLUSION = 1.5;

}
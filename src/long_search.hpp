#pragma once

#include <queue>
#include <thread>
#include <functional>
#include <climits>
#include <cmath>
#include <mutex>
#include "graph.hpp"

namespace long_search
{

    const std::vector<uint32_t> get_search_nodes(const Graph& graph);

    // Ranks all initial nodes based on the combined sizes of successor and predecessor lists
    std::vector<uint32_t> rank_nodes(const Graph& graph);

    inline uint32_t compute_rank(const Graph& graph, uint32_t node_id);


    // Performs BFS from a starting node and stores the longest chain found
    uint32_t bfs_search(const Graph& graph, uint32_t start_node, std::pair<uint32_t, uint32_t>& longest_chain);

    // Multithreading DFS search manager to start and manage the bfs on multiple threads
    uint32_t multithread_search(const Graph& graph, const std::vector<uint32_t>& start_nodes, std::vector<std::pair<uint32_t, uint32_t>>& results);

}
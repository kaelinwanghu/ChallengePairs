#pragma once

#include <thread>
#include <functional>
#include <climits>
#include <cmath>
#include <atomic>
#include <queue>
#include "graph.hpp"

namespace long_search
{
    // Performs BFS from a starting node and stores the longest chain found
    std::pair<uint32_t, uint32_t> enhanced_bfs_search(const Graph& graph, const Graph& full_graph, uint32_t start_node);

    std::pair<uint32_t, uint32_t> bfs_search(const Graph& graph, const uint32_t start_node);

    // Multithreading DFS search manager to start and manage the bfs on multiple threads
    std::vector<std::tuple<uint32_t, uint32_t, uint32_t>> multithread_search(const Graph& graph, const std::vector<uint32_t>& start_nodes);
}
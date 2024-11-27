// search.cpp

#include "long_search.hpp"
#include <iostream>

namespace long_search
{
    std::pair<uint32_t, uint32_t> enhanced_bfs_search(const Graph& graph, const Graph& full_graph, uint32_t start_node) noexcept
    {
        constexpr uint32_t TOP_PATHS = 20;

        // Stores sink nodes and their estimated total path lengths
        std::vector<std::pair<uint32_t, uint32_t>> sink_paths; // (sink_node, estimated_length)
        sink_paths.reserve(50000); // Trust me

        // Stores visited nodes
        std::vector<bool> visited;
        visited.reserve(graph.size());

        // deque stores pairs of (estimated_length, node_id)
        std::deque<std::pair<uint32_t, uint32_t>> bfs_queue;

        // Initialize the priority queue with the start node, always guaranteed to be a single node
        bfs_queue.emplace_back(start_node, 1);
        visited[start_node] = true;

        // Actual BFS search
        while (!bfs_queue.empty())
        {
            const auto [current_node, current_estimated_length] = bfs_queue.front();
            bfs_queue.pop_front();

            const auto& successors = graph.successors(current_node);

            // If current node is a sink node (no successors), potentially long path so store it
            if (successors.empty())
            {
                sink_paths.emplace_back(current_node, current_estimated_length);
                continue;
            }
            // For each successor
            for (auto it = successors.cbegin(); it != successors.cend(); ++it)
            {
                const uint32_t successor = *it;
                if (!visited[successor])
                {
                    visited[successor] = true;
                    bfs_queue.emplace_back(successor, current_estimated_length + graph.get_scc_diameter(successor));
                }
            }
        }

        // Keep only the top TOP_PATHS sink nodes with the largest estimated lengths
        if (sink_paths.size() > TOP_PATHS)
        {
            // Partition of the longest TOP_PATHs
            std::nth_element(sink_paths.begin(),
                sink_paths.begin() + TOP_PATHS - 1,
                sink_paths.end(),
                [](const auto& a, const auto& b) {
                    return a.second > b.second;
                });
            sink_paths.resize(TOP_PATHS);
        }

        // Evaluate the actual shortest paths for the top sink nodes
        std::pair<uint32_t, uint32_t> best_result = {0, 0}; // (path_length, sink_node)
        for (const auto& [sink_node, estimated_length] : sink_paths)
        {
            const std::deque<uint32_t> path = full_graph.shortest_path(start_node, sink_node);
            const uint32_t path_length = static_cast<uint32_t>(path.size());
            if (path_length > best_result.second)
            {
                best_result = {sink_node, path_length};
            }
        }
        return best_result;
    }

    // Standard BFS search
    std::pair<uint32_t, uint32_t> bfs_search(const Graph& graph, const uint32_t start_node) noexcept
    {
        // Stores visited nodes
        std::vector<bool> visited(graph.size(), false);
        // deque stores pairs of (path_length, node_id)
        std::deque<std::pair<uint32_t, uint32_t>> bfs_queue;

        // Initialize the BFS queue with the start node
        bfs_queue.emplace_back(start_node, 1);
        visited[start_node] = true;

        std::pair<uint32_t, uint32_t> best_path = {0, 0};

        // Actual BFS search
        while (!bfs_queue.empty())
        {
            const auto [current_node, current_path_length] = bfs_queue.front();
            bfs_queue.pop_front();

            const std::vector<uint32_t>& successors = graph.successors(current_node, true);

            // If current node is a sink node (no successors), store its path length
            if (successors.empty())
            {
                if (current_path_length > best_path.second)
                {
                    best_path = {current_node, current_path_length};
                }
                continue;
            }

            for (const uint32_t successor_id : successors)
            {
                if (!visited[successor_id])
                {
                    visited[successor_id] = true;
                    bfs_queue.emplace_back(successor_id, current_path_length + 1);
                }
            }

            if (bfs_queue.empty())
            {
                best_path = {current_node, current_path_length};
            }
        }

        return best_path;
    }
    
    // Statistics gathering structure
    struct ThreadStats
    {
        std::vector<double> search_times;  // Time for each individual search
        double total_time{0.0};            // Total time spent searching
        uint32_t searches_completed{0};    // Number of searches completed
        uint32_t thread_id;
        double completion_time{0.0};       // The time of last completion of the thread search
        
        // Reserve space for expected number of searches
        explicit ThreadStats(uint32_t id, size_t expected_searches) : thread_id(id)
        {
            search_times.reserve(expected_searches);
        }
    };

    std::vector<std::tuple<uint32_t, uint32_t, uint32_t>> multithread_search(
        const Graph& graph, const std::vector<uint32_t>& start_nodes) noexcept 
    {
        std::vector<std::tuple<uint32_t, uint32_t, uint32_t>> results;
        const uint32_t num_threads = std::thread::hardware_concurrency();
        const size_t start_nodes_size = start_nodes.size();
        
        std::atomic<size_t> index_counter(0);
        std::vector<std::thread> threads;
        threads.reserve(num_threads);
        
        // Create vectors for results and statistics
        std::vector<std::vector<std::tuple<uint32_t, uint32_t, uint32_t>>> thread_results(num_threads);
        std::vector<ThreadStats> thread_statistics;
        thread_statistics.reserve(num_threads);
        
        // Initialize statistics for each thread
        for (uint32_t i = 0; i < num_threads; ++i)
        {
            thread_statistics.emplace_back(i, start_nodes_size / num_threads);
        }
        
        auto global_start = std::chrono::high_resolution_clock::now();

        auto thread_search = [&](const uint32_t thread_id)
        {
            ThreadStats& stats = thread_statistics[thread_id];
            auto& local_results = thread_results[thread_id];
            local_results.reserve(start_nodes_size / num_threads);
            size_t i;
            while ((i = index_counter.fetch_add(1, std::memory_order_relaxed)) < start_nodes_size)
            {
                const uint32_t start_node = start_nodes[i];
                
                // Time this individual search
                auto search_start = std::chrono::high_resolution_clock::now();
                const std::pair<uint32_t, uint32_t> result = bfs_search(graph, start_node);
                auto search_end = std::chrono::high_resolution_clock::now();
                
                // Record statistics
                double search_time = std::chrono::duration<double, std::milli>(search_end - search_start).count();
                stats.search_times.push_back(search_time);
                stats.total_time += search_time;
                stats.searches_completed++;
                stats.completion_time = std::chrono::duration<double, std::milli>(search_end - global_start).count();
                local_results.emplace_back(start_node, result.first, result.second);
            }
        };

        // Launch threads
        for (uint32_t t = 0; t < num_threads; ++t)
        {
            threads.emplace_back(thread_search, t);
        }
        
        for (std::thread& thread : threads)
        {
            thread.join();
        }

        auto global_end = std::chrono::high_resolution_clock::now();
        
        results.reserve(start_nodes_size);
        for (const auto& local_results : thread_results)
        {
            results.insert(results.end(), std::make_move_iterator(local_results.begin()), std::make_move_iterator(local_results.end()));
        }
        
        // Sort results
        std::sort(results.begin(), results.end(),
            [](const auto& a, const auto& b)
            {
                return std::get<2>(a) > std::get<2>(b);
            });
        
        double total_execution_time = std::chrono::duration<double, std::milli>(global_end - global_start).count();
        
        std::cout << "\nThread Statistics:\n";
        std::cout << "Total execution time: " << total_execution_time << "ms\n\n";
        
        for (const auto& stats : thread_statistics)
        {
            std::cout << "Thread " << stats.thread_id << ":\n";
            std::cout << "    Searches completed: " << stats.searches_completed << "\n";
            std::cout << "    Total search time: " << stats.total_time << "ms\n";
            std::cout << "    Completion time: " << stats.completion_time << "ms\n";
            
            // Calculate statistics for this thread
            if (!stats.search_times.empty())
            {
                double avg_time = stats.total_time / stats.searches_completed;
                auto [min_it, max_it] = std::minmax_element(stats.search_times.begin(), stats.search_times.end());
                std::cout << "    Average search time: " << avg_time << "ms\n";
                std::cout << "    Min search time: " << *min_it << "ms\n";
                std::cout << "    Max search time: " << *max_it << "ms\n";
            }
            std::cout << "\n";
        }
        return results;
    }
}
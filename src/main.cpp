/**
 * Name: Kaelin Wang Hu
 * Date Started: 10/29/2024
 * Description: Main function doubling as file reader and graph populator
 */
#include <fstream>
#include <chrono>
#include <iostream>
#include "graph.hpp"
#include "long_search.hpp"

// Fast C++ style I/O
void fast_io()
{
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);
    std::cout.tie(nullptr);
}

// Function to read and add vertices from the vertices file
uint32_t read_vertices(Graph& graph, const std::string& filename)
{
    uint32_t failed_count = 0;
    std::ifstream vertices_file(filename);
    if (!vertices_file)
    {
        std::cerr << "Failed to open vertices file" << "\n";
        return std::numeric_limits<uint32_t>::max();
    }   

    std::string line;
    // Normalize IDs for faster access and retrieval
    uint32_t normalized_id = 1; // 0 is reserved for function failure
    while (std::getline(vertices_file, line))
    {
        const char* p = line.c_str();
        char* endptr;
        uint32_t node_id = strtoul(p, &endptr, 10);
        ++endptr;
        std::string name = endptr;

        if (graph.add_vertex(node_id, name))
        {
            normalized_id++;
        }
        else
        {
            failed_count++;
        }
    }

    return failed_count;
}

// Read and add edges from the edges file
uint32_t read_edges(Graph& graph, const std::string& filename, const emhash8::HashMap<uint32_t, uint32_t, XXIntHasher>& id_normalizer)
{
    uint32_t failed_count = 0;
    std::ifstream edges_file(filename);
    if (!edges_file)
    {
        std::cerr << "Failed to open edges file" << "\n";
        return std::numeric_limits<uint32_t>::max();
    }

    std::string line;
    while (std::getline(edges_file, line))
    {
        const char* p = line.c_str();
        char* endptr;
        auto from_it = id_normalizer.find(strtoul(p, &endptr, 10));
        ++endptr;
        auto to_it = id_normalizer.find(strtoul(endptr, nullptr, 10));
        if (from_it != id_normalizer.end() && to_it != id_normalizer.end())
        {
            if (!graph.add_edge(from_it->second, to_it->second))
            {
                failed_count++;
            }
        }
        else
        {
            failed_count++;
        }
    }

    return failed_count;
}


int main()
{
    fast_io();
    // Track the time
    auto start_name = std::chrono::high_resolution_clock::now();

    Graph people_graph;

    emhash8::HashMap<uint32_t, uint32_t, XXIntHasher> id_normalizer;

    uint32_t failed_vertices = read_vertices(people_graph, "../data/wiki-livingpeople-names.txt");

    auto end_name = std::chrono::high_resolution_clock::now();

    uint32_t failed_edges = read_edges(people_graph, "../data/wiki-livingpeople-links.txt", id_normalizer);

    auto end_link = std::chrono::high_resolution_clock::now();

    std::cout << "Read " << people_graph.size() << " and failed to read " << failed_vertices << " vertices in " << ((std::chrono::duration_cast<std::chrono::milliseconds>(end_name - start_name)).count()) << " milliseconds\n";
    std::cout << "Read " << people_graph.num_edges() << " and failed to read " << failed_edges << " edges in " << (std::chrono::duration_cast<std::chrono::milliseconds>(end_link - end_name).count()) << " milliseconds\n";

    std::vector<uint32_t> source_nodes;
    source_nodes.reserve(people_graph.size() / 4); // Really rough approximation (still probably better than default size)
    for (auto it = people_graph.node_begin(); it != people_graph.node_end(); ++it)
    {
        if (people_graph.predecessors(it->second).size() == 0)
        {
            source_nodes.emplace_back(it->second);
        }
    }

    auto search_start = std::chrono::high_resolution_clock::now();

    source_nodes.resize(120);
    std::vector<std::tuple<uint32_t, uint32_t, uint32_t>> results = long_search::multithread_search(people_graph, source_nodes);

    auto search_end = std::chrono::high_resolution_clock::now();

    std::cout << "BFS searches of " << source_nodes.size() << " nodes completed in: " 
        << (std::chrono::duration_cast<std::chrono::milliseconds>(search_end - search_start)).count() 
        << " milliseconds\n";

    if (!results.empty())
    {
        const auto& best_chain = results.front();
        std::cout << "Source node: " << people_graph.get_key(std::get<0>(best_chain))
            << " | Sink node: " << people_graph.get_key(std::get<1>(best_chain))
            << " | Length: " << std::get<2>(best_chain) << "\n";
    }

    return 0;
}
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
void fast()
{
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);
    std::cout.tie(nullptr);
}

// Function to read and add vertices from the names file
void read_names(Graph& graph, const std::string& filename)
{
    std::ifstream namesFile(filename);
    if (!namesFile) {
        std::cerr << "Failed to open names file" << "\n";
        return;
    }   

    std::string line;
    while (std::getline(namesFile, line))
    {
        const char* p = line.c_str();
        char* endptr;
        uint32_t node_id = strtoul(p, &endptr, 10);
        ++endptr;
        std::string name = endptr;
        graph.add_vertex(node_id, name);
    }
}

// Read and add edges from the links file
void read_links(Graph& graph, const std::string& filename)
{
    std::ifstream linksFile(filename);
    if (!linksFile) {
        std::cerr << "Failed to open links file" << "\n";
        return;
    }

    std::string line;
    while (std::getline(linksFile, line))
    {
        const char* p = line.c_str();
        char* endptr;
        uint32_t from_id = strtoul(p, &endptr, 10);
        ++endptr;
        uint32_t to_id = strtoul(endptr, nullptr, 10);
        graph.add_edge(from_id, to_id);
    }
}

int main()
{
    fast();

    // Track the time
    auto start = std::chrono::high_resolution_clock::now();

    Graph people_graph;

    read_names(people_graph, "../data/wiki-livingpeople-names.txt");

    auto end_name = std::chrono::high_resolution_clock::now();

    read_links(people_graph, "../data/wiki-livingpeople-links.txt");

    auto end_link = std::chrono::high_resolution_clock::now();

    std::cout << "Read names in " << ((std::chrono::duration_cast<std::chrono::milliseconds>(end_name - start)).count()) << " milliseconds\n";
    std::cout << "Read " << people_graph.size() << " vertices with " << people_graph.num_edges() << " edges" << " in " << (std::chrono::duration_cast<std::chrono::milliseconds>(end_link - start).count()) << " milliseconds\n";

    Graph collapsed_people_graph = people_graph.collapse_cliques();

    auto end_collapse = std::chrono::high_resolution_clock::now();

    std::cout << "clique collapsed in: " << (std::chrono::duration_cast<std::chrono::milliseconds>(end_collapse - start).count()) << " milliseconds\n";

    std::vector<uint32_t> source_nodes;
    source_nodes.reserve(75000); // Trust me (again)
    for (auto it = collapsed_people_graph.node_begin(); it != collapsed_people_graph.node_end(); ++it)
    {
        if (collapsed_people_graph.predecessors(it->first).size() == 0)
        {
            source_nodes.emplace_back(it->first);
        }
    }

    auto search_start = std::chrono::high_resolution_clock::now();

    source_nodes.resize(2000);
    std::vector<std::tuple<uint32_t, uint32_t, uint32_t>> results = long_search::multithread_search(people_graph, source_nodes);

    auto search_end = std::chrono::high_resolution_clock::now();

    std::cout << "BFS searches completed in: " 
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
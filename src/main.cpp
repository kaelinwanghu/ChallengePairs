/**
 * Name: Kaelin Wang Hu
 * Date Started: 10/29/2024
 * Description: Main function doubling as file reader and graph populator
 */
#include <fstream>
#include <chrono>
#include <iostream>
#include "graph.hpp"

// Fast C++ style I/O
void fast() {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);
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

int main() {
    fast();

    // Track the time
    auto start = std::chrono::high_resolution_clock::now();

    Graph peopleGraph;

    read_names(peopleGraph, "../data/wiki-livingpeople-names.txt");

    auto end_name = std::chrono::high_resolution_clock::now();

    read_links(peopleGraph, "../data/wiki-livingpeople-links.txt");

    auto end = std::chrono::high_resolution_clock::now();

    std::cout << "Read names in " << ((std::chrono::duration_cast<std::chrono::milliseconds>(end_name - start)).count()) << "\n";
    std::cout << "Read " << peopleGraph.size() << " vertices with " << peopleGraph.num_edges() << " edges" << " in " << (std::chrono::duration_cast<std::chrono::milliseconds>(end - start)).count() << " milliseconds";
    
    return 0;
}

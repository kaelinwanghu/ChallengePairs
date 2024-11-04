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

    Graph people_graph;

    read_names(people_graph, "../data/wiki-livingpeople-names.txt");

    auto end_name = std::chrono::high_resolution_clock::now();

    read_links(people_graph, "../data/wiki-livingpeople-links.txt");

    auto end_link = std::chrono::high_resolution_clock::now();

    std::cout << "Read names in " << ((std::chrono::duration_cast<std::chrono::milliseconds>(end_name - start)).count()) << " milliseconds\n";
    std::cout << "Read " << people_graph.size() << " vertices with " << people_graph.num_edges() << " edges" << " in " << (std::chrono::duration_cast<std::chrono::milliseconds>(end_link - start).count()) << " milliseconds\n";

    const std::vector<uint32_t> search_nodes =  long_search::get_search_nodes(people_graph);

    Graph collapsed_people_graph = people_graph.collapse_cliques();

    auto end_collapse = std::chrono::high_resolution_clock::now();

    std::cout << "collapsed graph size: " << collapsed_people_graph.size() << " and edge number: " << collapsed_people_graph.num_edges() << "\n";
    std::cout << "clique collapsed in: " << (std::chrono::duration_cast<std::chrono::milliseconds>(end_collapse - start).count()) << " milliseconds";

    // std::vector<std::tuple<uint32_t, uint32_t, uint32_t>> results;

    // std::pair<uint32_t, uint32_t> path;
    // // Loop over search_nodes and perform bfs_search
    // for (size_t i = 0; i < search_nodes.size(); i++)
    // {
    //     uint32_t length = long_search::bfs_search(people_graph, search_nodes[i], path);
    //     results.emplace_back(length, path.first, path.second);
    // }

    // // Sort the results in descending order based on length
    // std::sort(results.begin(), results.end(),
    //     [](const auto& a, const auto& b)
    //     {
    //         return std::get<0>(a) > std::get<0>(b);
    //     });

    // // Output the results
    // std::cout << "\nTop chains found:\n";
    // for (const auto& [length, start_node_id, end_node_id] : results)
    // {
    //     std::string start_name = people_graph.get_key(start_node_id);
    //     std::string end_name = people_graph.get_key(end_node_id);
    //     std::cout << "Length: " << length << " | Starting person: " << start_name
    //               << " | Ending person: " << end_name << "\n";
    // }
    return 0;
}

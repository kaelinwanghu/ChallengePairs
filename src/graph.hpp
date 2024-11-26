/**
 * Name: Kaelin Wang Hu
 * Date Started: 10/29/2024
 * Description: graph class declaration
 */
#pragma once

#include <hash_table8.hpp> // emhash8 hashmap
// The headers use different definitions for these so undefine them after hash_table8
#undef EMH_EMPTY
#undef EMH_EQHASH
#undef EMH_NEW
#include <hash_set8.hpp>   // emhash8 hashset
#include <stdint.h>        // For fixed-size integer types
#include <vector>
#include <string>
#include <stack>
#include <deque>
#include <limits>

// Note that the graph edges are around 3 million, so a uint32_t should suffice

class Graph
{
public:
    Graph();
    ~Graph();
    void initialize_graph(uint32_t num_vertices) noexcept;

    uint32_t size() const noexcept;
    uint32_t num_edges() const noexcept;

    bool add_vertex(const uint32_t node_id, const std::string& key) noexcept;
    bool add_edge(const uint32_t from_id, const uint32_t to_id, bool is_normalized = false) noexcept;

    bool has_vertex(const uint32_t node_id, bool is_normalized = false) const noexcept;
    bool has_edge(const uint32_t from_id, const uint32_t to_id, bool is_normalized = false) const noexcept;

    bool remove_edge(const uint32_t from_id, const uint32_t to_id, bool is_normalized = false) noexcept;

    uint32_t out_degree(const uint32_t node_id, bool is_normalized = false) const noexcept;
    uint32_t in_degree(const uint32_t , bool is_normalized = false) const noexcept;

    uint32_t get_node_id(const std::string& key) const noexcept;
    std::string get_key(const uint32_t node_id, bool is_normalized = false) const noexcept;

    std::string graph_string() const noexcept;

    std::deque<uint32_t> shortest_path(const uint32_t from_id, const uint32_t to_id, bool is_normalized = false) const noexcept;

    const std::vector<uint32_t>& successors(const uint32_t node_id, bool is_normalized = false) const noexcept;
    const std::vector<uint32_t>& predecessors(const uint32_t node_id, bool is_normalized = false) const noexcept;
    
    void compute_scc_diameters() noexcept;

    // Get the diameter of the SCC that the node is in (if any)
    uint32_t get_scc_diameter(uint32_t node_id, bool is_normalized = false) const noexcept;

    Graph collapse_cliques() const noexcept;
    std::vector<emhash8::HashSet<uint32_t>> find_all_strongly_connected_components() const noexcept;

    uint32_t get_normalized_id(uint32_t node_id) const noexcept;
    uint32_t set_normalized_id(uint32_t node_id) noexcept;

    using node_iterator = emhash8::HashMap<std::string, uint32_t>::const_iterator;    // The actual iterators so the entire graph can be traversed
    node_iterator node_begin() const noexcept;
    node_iterator node_end() const noexcept;

private:
    std::vector<std::vector<uint32_t>> successor_list;
    std::vector<std::vector<uint32_t>> predecessor_list;

    // Stores the string keys (person name) to their corresponding node_ids in uint32_t form
    emhash8::HashMap<std::string, uint32_t> key_to_id;
    // Stores the uint32_t node_ids to their corresponding keys (people names)
    std::vector<std::string> id_to_key;

    // Map from node ID to SCC ID
    std::vector<uint32_t> node_to_scc;

    // Map from SCC ID to SCC diameter
    emhash8::HashMap<uint32_t, uint32_t> scc_to_diameter;

    uint32_t edge_count;

    uint32_t current_normalized_id;
    emhash8::HashMap<uint32_t, uint32_t> id_normalizer;
};

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
#include <xxhash.h>        // xxHash
#include <stdint.h>        // For fixed-size integer types
#include <vector>
#include <string>
#include <stack>
#include <deque>
#include <limits>

// Note that the graph edges are around 3 million, so a uint32_t should suffice

// Hash functions for both ints and strings using xxHash
struct XXStringHasher
{
    uint64_t operator()(const std::string& key) const
    {
        return XXH64(key.data(), key.size(), 0);
    }
};

struct XXIntHasher
{
    uint64_t operator()(const uint32_t& key) const
    {
        return XXH64(&key, sizeof(key), 0);
    }
};

class Graph
{
public:
    Graph();
    ~Graph();
    void initialize_graph(uint32_t num_vertices);

    uint32_t size() const;
    uint32_t num_edges() const;

    bool add_vertex(const uint32_t node_id, const std::string& key);
    bool add_edge(const uint32_t from_id, const uint32_t to_id, bool is_normalized = false);

    bool has_vertex(const uint32_t node_id, bool is_normalized = false) const;
    bool has_edge(const uint32_t from_id, const uint32_t to_id, bool is_normalized = false) const;

    bool remove_edge(const uint32_t from_id, const uint32_t to_id, bool is_normalized = false);

    uint32_t out_degree(const uint32_t node_id, bool is_normalized = false) const;
    uint32_t in_degree(const uint32_t , bool is_normalized = false) const;

    uint32_t get_node_id(const std::string& key) const;
    std::string get_key(const uint32_t node_id, bool is_normalized = false) const;

    std::string graph_string() const;

    std::deque<uint32_t> shortest_path(const uint32_t from_id, const uint32_t to_id, bool is_normalized = false) const;

    const std::vector<uint32_t>& successors(const uint32_t node_id, bool is_normalized = false) const;
    const std::vector<uint32_t>& predecessors(const uint32_t node_id, bool is_normalized = false) const;
    
    void compute_scc_diameters();

    // Get the diameter of the SCC that the node is in (if any)
    uint32_t get_scc_diameter(uint32_t node_id, bool is_normalized = false) const;

    Graph collapse_cliques() const;
    std::vector<emhash8::HashSet<uint32_t, XXIntHasher>> find_all_strongly_connected_components() const;

    uint32_t get_normalized_id(uint32_t node_id) const;
    uint32_t set_normalized_id(uint32_t node_id);

    using node_iterator = emhash8::HashMap<std::string, uint32_t, XXStringHasher>::const_iterator;    // The actual iterators so the entire graph can be traversed
    node_iterator node_begin() const;
    node_iterator node_end() const;

private:
    std::vector<std::vector<uint32_t>> successor_list;
    std::vector<std::vector<uint32_t>> predecessor_list;

    // Stores the string keys (person name) to their corresponding node_ids in uint32_t form
    emhash8::HashMap<std::string, uint32_t, XXStringHasher> key_to_id;
    // Stores the uint32_t node_ids to their corresponding keys (people names)
    std::vector<std::string> id_to_key;

    // Map from node ID to SCC ID
    std::vector<uint32_t> node_to_scc;

    // Map from SCC ID to SCC diameter
    emhash8::HashMap<uint32_t, uint32_t, XXIntHasher> scc_to_diameter;

    uint32_t edge_count;

    uint32_t current_normalized_id;
    emhash8::HashMap<uint32_t, uint32_t, XXIntHasher> id_normalizer;
};

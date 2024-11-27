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

    inline uint32_t size() const noexcept
    {
        return id_to_key.size() - 1;
    }
    inline uint32_t num_edges() const noexcept
    {
        return edge_count;
    }

    bool add_vertex(const uint32_t node_id, const std::string& key) noexcept;
    bool add_edge(const uint32_t from_id, const uint32_t to_id, bool is_normalized = false) noexcept;

    inline bool has_vertex(const uint32_t node_id, bool is_normalized = false) const noexcept
    {
        const uint32_t normalized_id = is_normalized ? node_id : get_normalized_id(node_id);
        return normalized_id > 0 && normalized_id < id_to_key.size();
    }
    inline bool has_edge(const uint32_t from_id, const uint32_t to_id, bool is_normalized = false) const 
    {
        const uint32_t normalized_from_id = is_normalized ? from_id : get_normalized_id(from_id);
        const uint32_t normalized_to_id = is_normalized ? to_id : get_normalized_id(to_id);
        if (!has_vertex(normalized_from_id, true) || !has_vertex(normalized_to_id, true))
        {
            return false;
        }

        const std::vector<uint32_t>& successors = successor_list[normalized_from_id];
        return std::find(successors.begin(), successors.end(), normalized_to_id) != successors.end();
    }

    bool remove_edge(const uint32_t from_id, const uint32_t to_id, bool is_normalized = false) noexcept;

    inline uint32_t out_degree(const uint32_t node_id, bool is_normalized = false) const noexcept
    {
        const uint32_t normalized_id = is_normalized ? node_id : get_normalized_id(node_id);
        return successor_list[normalized_id].size();
    }
    inline uint32_t in_degree(const uint32_t node_id, bool is_normalized = false) const noexcept
    {
        const uint32_t normalized_id = is_normalized ? node_id : get_normalized_id(node_id);
        return predecessor_list[normalized_id].size();
    }

    inline uint32_t get_node_id(const std::string& key) const noexcept
    {
        const auto it = key_to_id.find(key);
        if (it != key_to_id.end())
        {
            return it->second;
        }
        else
        {
            return 0;
        }
    }
    inline std::string get_key(const uint32_t node_id, bool is_normalized = false) const noexcept
    {
        const uint32_t normalized_id = is_normalized ? node_id : get_normalized_id(node_id);
        return id_to_key[normalized_id];
    }

    std::string graph_string() const noexcept;

    std::deque<uint32_t> shortest_path(const uint32_t from_id, const uint32_t to_id, bool is_normalized = false) const noexcept;

    inline const std::vector<uint32_t>& successors(const uint32_t node_id, bool is_normalized = false) const noexcept
    {
        const uint32_t normalized_id = is_normalized ? node_id : get_normalized_id(node_id);
        return successor_list[normalized_id];
    }
    inline const std::vector<uint32_t>& predecessors(const uint32_t node_id, bool is_normalized = false) const noexcept
    {
        const uint32_t normalized_id = is_normalized ? node_id : get_normalized_id(node_id);
        return predecessor_list[normalized_id];
    }
    
    void compute_scc_diameters() noexcept;

    // Get the diameter of the SCC that the node is in (if any)
    uint32_t get_scc_diameter(uint32_t node_id, bool is_normalized = false) const noexcept;

    Graph collapse_cliques() const noexcept;
    std::vector<emhash8::HashSet<uint32_t>> find_all_strongly_connected_components() const noexcept;

    inline uint32_t get_normalized_id(uint32_t node_id) const noexcept
    {
        const auto it = id_normalizer.find(node_id);
        return it != id_normalizer.end() ? it->second : 0;
    }
    uint32_t set_normalized_id(uint32_t node_id) noexcept;

    using node_iterator = emhash8::HashMap<std::string, uint32_t>::const_iterator;    // The actual iterators so the entire graph can be traversed
    // Iterators using id_to_key to go through the entire graph (order not guaranteed)
     node_iterator node_begin() const noexcept
    {
        return key_to_id.cbegin();
    }
    inline node_iterator node_end() const noexcept
    {
        return key_to_id.cend();
    }

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

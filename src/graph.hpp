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
#include <mutex>           // For multithreading
#include <vector>
#include <iostream>
#include <string>

// Note that the graph edges are around 3 million, so a uint32_t should suffice

// Hash functions for both ints and strings using xxHash
struct XXStringHasher {
    uint64_t operator()(const std::string& key) const {
        return XXH64(key.data(), key.size(), 0);
    }
};

struct XXIntHasher {
    uint64_t operator()(const uint32_t& key) const {
        return XXH64(&key, sizeof(key), 0);
    }
};

class Graph {
public:
    Graph();
    ~Graph();

    uint32_t size() const;
    uint32_t num_edges() const;

    bool add_vertex(const uint32_t node_id, const std::string& key);
    bool add_edge(const uint32_t from_id, const uint32_t to_id);

    bool has_vertex(const uint32_t node_id) const;
    bool has_edge(const uint32_t from_id, const uint32_t to_id) const;

    bool remove_edge(const uint32_t from_id, const uint32_t to_id);

    uint32_t out_degree(const uint32_t node_id) const;
    uint32_t in_degree(const uint32_t node_id) const;

    uint32_t get_node_id(const std::string& key) const;
    std::string get_key(const uint32_t node_id) const;

    std::string graph_string() const;

    emhash8::HashSet<uint32_t, XXIntHasher> successor_set(const uint32_t node_id) const;
    emhash8::HashSet<uint32_t, XXIntHasher> predecessor_set(const uint32_t node_id) const;

private:
    // Internal functions without locking
    uint32_t size_lockless() const;
    uint32_t num_edges_lockless() const;
    bool has_vertex_lockless(const uint32_t node_id) const;
    bool has_edge_lockless(const uint32_t from_id, const uint32_t to_id) const;
    uint32_t out_degree_lockless(const uint32_t node_id) const;
    uint32_t in_degree_lockless(const uint32_t node_id) const;
    uint32_t get_node_id_lockless(const std::string& key) const;
    std::string get_key_lockless(const uint32_t node_id) const;

    // Might be changed in the future into a HashSet instead of a vector if it causes efficiency issues
    emhash8::HashMap<uint32_t, std::vector<uint32_t>, XXIntHasher> successor_list;
    emhash8::HashMap<uint32_t, std::vector<uint32_t>, XXIntHasher> predecessor_list;

    emhash8::HashMap<std::string, uint32_t, XXStringHasher> key_to_id;
    emhash8::HashMap<uint32_t, std::string, XXIntHasher> id_to_key;

    uint32_t edge_count;

    mutable std::mutex graph_mutex;
};

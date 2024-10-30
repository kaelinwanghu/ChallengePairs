/**
 * Name: Kaelin Wang Hu
 * Date Started: 10/29/2024
 * Description: Efficient graph class implementation
 */
#include "graph.hpp"

// The containers all handle it so no need to do much for graph constructor and destructor
Graph::Graph() : edge_count(0)
{ }

Graph::~Graph()
{ }

// Gets the size of the graph in terms of vertices
uint32_t Graph::size() const
{
    std::lock_guard<std::mutex> lock(graph_mutex);
    return size_lockless();
}

uint32_t Graph::size_lockless() const
{
    return key_to_id.size();
}

// Gets the number of edges of the graph
uint32_t Graph::num_edges() const
{
    std::lock_guard<std::mutex> lock(graph_mutex);
    return num_edges_lockless();
}

uint32_t Graph::num_edges_lockless() const
{
    return edge_count;
}

// Adds a vertex to the graph if it does not already exist (names should all be unique)
bool Graph::add_vertex(const uint32_t node_id, const std::string& key)
{
    std::lock_guard<std::mutex> lock(graph_mutex);

    // Check that they don't exist yet before adding vertex
    if (key_to_id.find(key) != key_to_id.end() || id_to_key.find(node_id) != id_to_key.end())
    {
        return false;
    }

    key_to_id[key] = node_id;
    id_to_key[node_id] = key;

    // Initialize vector successors and predecessors as well
    successor_list[node_id] = std::vector<uint32_t>();
    predecessor_list[node_id] = std::vector<uint32_t>();

    return true;
}

// Adds an edge from a node to another if they do not exist already
bool Graph::add_edge(const uint32_t from_id, const uint32_t to_id)
{
    std::lock_guard<std::mutex> lock(graph_mutex);

    if (!has_vertex_lockless(from_id) || !has_vertex_lockless(to_id))
    {
        return false;
    }

    // Rather inefficient find, change in the future (auto is left for now)
    auto& successors = successor_list[from_id];
    if (std::find(successors.begin(), successors.end(), to_id) != successors.end())
    {
        return false;
    }

    // Emplace for both successors and predecessors
    successors.emplace_back(to_id);
    predecessor_list[to_id].emplace_back(from_id);

    ++edge_count;

    return true;
}

// Checks whether the graph has a particular vertex with the node Id
bool Graph::has_vertex(const uint32_t node_id) const
{
    std::lock_guard<std::mutex> lock(graph_mutex);
    return has_vertex_lockless(node_id);
}

// Checks whether the graph has a particular vertex without locking
bool Graph::has_vertex_lockless(const uint32_t node_id) const
{
    return id_to_key.find(node_id) != id_to_key.end();
}

// Checks whether the graph has a particular edge with the node Id
bool Graph::has_edge(const uint32_t from_id, const uint32_t to_id) const
{
    std::lock_guard<std::mutex> lock(graph_mutex);
    return has_edge_lockless(from_id, to_id);
}

// Checks whether the graph has a particular edge without locking
bool Graph::has_edge_lockless(const uint32_t from_id, const uint32_t to_id) const
{
    if (!has_vertex_lockless(from_id) || !has_vertex_lockless(to_id))
    {
        return false;
    }

    // The graph is not extremely sparse but not extremely filled, so the change into HashSet might have to wait
    const auto& successors = successor_list.at(from_id);
    return std::find(successors.begin(), successors.end(), to_id) != successors.end();
}

// Removes an edge from the graph if it exists
bool Graph::remove_edge(const uint32_t from_id, const uint32_t to_id)
{
    std::lock_guard<std::mutex> lock(graph_mutex);

    if (!has_vertex_lockless(from_id) || !has_vertex_lockless(to_id))
    {
        return false;
    }

    // Remove for both successors...
    auto& successors = successor_list[from_id];
    const auto successor_find = std::find(successors.begin(), successors.end(), to_id);
    if (successor_find != successors.end())
    {
        successors.erase(successor_find);
    }
    else
    {
        return false;
    }

    // And predecessors
    auto& predecessors = predecessor_list[to_id];
    const auto predecessor_find = std::find(predecessors.begin(), predecessors.end(), from_id);
    if (predecessor_find != predecessors.end())
    {
        predecessors.erase(predecessor_find);
    }

    --edge_count;

    return true;
}

// Calculates the out_degree of a node (how many edges originate from it)
uint32_t Graph::out_degree(const uint32_t node_id) const
{
    std::lock_guard<std::mutex> lock(graph_mutex);
    return out_degree_lockless(node_id);
}

// Calculates the out_degree of a node without locking
uint32_t Graph::out_degree_lockless(const uint32_t node_id) const
{
    const auto it = successor_list.find(node_id);
    if (it != successor_list.end())
    {
        return static_cast<uint32_t>(it->second.size());
    }
    else
    {
        return 0;
    }
}

// Calculates the in_degree of a node (how many edges end at it)
uint32_t Graph::in_degree(const uint32_t node_id) const
{
    std::lock_guard<std::mutex> lock(graph_mutex);
    return in_degree_lockless(node_id);
}

// Calculates the in_degree of a node without locking
uint32_t Graph::in_degree_lockless(const uint32_t node_id) const
{
    const auto it = predecessor_list.find(node_id);
    if (it != predecessor_list.end())
    {
        return static_cast<uint32_t>(it->second.size());
    }
    else
    {
        return 0;
    }
}

// Gets the node_id based on a particular string key (person name)
uint32_t Graph::get_node_id(const std::string& key) const
{
    std::lock_guard<std::mutex> lock(graph_mutex);
    return get_node_id_lockless(key);
}

// Gets the node_id without locking
uint32_t Graph::get_node_id_lockless(const std::string& key) const
{
    const auto it = key_to_id.find(key);
    if (it != key_to_id.end())
    {
        return it->second;
    }
    else
    {
        return 0; // maybe throw an error instead? Not sure if there is node 0
    }
}

// Gets the key (person name) based on the specified node_id, opposite of function above
std::string Graph::get_key(const uint32_t node_id) const
{
    std::lock_guard<std::mutex> lock(graph_mutex);
    return get_key_lockless(node_id);
}

// Gets the key without locking
std::string Graph::get_key_lockless(const uint32_t node_id) const
{
    const auto it = id_to_key.find(node_id);
    if (it != id_to_key.end())
    {
        return it->second;
    }
    else
    {
        return std::string(); // empty string return otherwise
    }
}

// Gets the successor set of a certain node
emhash8::HashSet<uint32_t, XXIntHasher> Graph::successor_set(const uint32_t node_id) const
{
    std::lock_guard<std::mutex> lock(graph_mutex);

    // Create a new HashSet and add all the relevant nodes to that successors HashSet
    emhash8::HashSet<uint32_t, XXIntHasher> successors;
    const auto it = successor_list.find(node_id);
    if (it != successor_list.end())
    {
        for (const uint32_t successor_id : it->second)
        {
            successors.insert(successor_id);
        }
    }

    return successors;
}

// Gets the predecessor set of a certain node
emhash8::HashSet<uint32_t, XXIntHasher> Graph::predecessor_set(const uint32_t node_id) const
{
    std::lock_guard<std::mutex> lock(graph_mutex);

    // Create a new HashSet and add all the relevant nodes to that predecessors HashSet
    emhash8::HashSet<uint32_t, XXIntHasher> predecessors;
    const auto it = predecessor_list.find(node_id);
    if (it != predecessor_list.end())
    {
        for (const uint32_t predecessor_id : it->second)
        {
            predecessors.insert(predecessor_id);
        }
    }

    return predecessors;
}

// Returns a string representation of the graph
std::string Graph::graph_string() const
{
    std::lock_guard<std::mutex> lock(graph_mutex);

    std::string result;
    result.reserve(num_edges_lockless() + size_lockless());

    const auto end = successor_list.end();
    for (auto it = successor_list.begin(); it != end; ++it)
    {
        const std::string& node_name = get_key_lockless(it->first);
        result += node_name + ": ";

        const std::vector<uint32_t>& successors = it->second;
        for (size_t i = 0; i < successors.size(); ++i)
        {
            const std::string& successor_name = get_key_lockless(successors[i]);
            result += successor_name;
            if (i != successors.size() - 1)
            {
                result += ", ";
            }
        }
        result += "\n";
    }

    return result;
}
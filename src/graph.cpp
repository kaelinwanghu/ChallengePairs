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
    return key_to_id.size();
}


// Gets the number of edges of the graph
uint32_t Graph::num_edges() const
{
    return edge_count;
}


// Adds a vertex to the graph if it does not already exist (names should all be unique)
bool Graph::add_vertex(const uint32_t node_id, const std::string& key)
{
    // Check that they don't exist yet before adding vertex
    if (key_to_id.find(key) != key_to_id.end() || id_to_key.find(node_id) != id_to_key.end())
    {
        return false;
    }

    key_to_id[key] = node_id;
    id_to_key[node_id] = key;

    // Initialize vector successors and predecessors as well
    successor_set[node_id] = emhash8::HashSet<uint32_t, XXIntHasher>();
    predecessor_set[node_id] = emhash8::HashSet<uint32_t, XXIntHasher>();

    return true;
}

// Adds an edge from a node to another if they do not exist already
bool Graph::add_edge(const uint32_t from_id, const uint32_t to_id)
{
    if (!has_vertex(from_id) || !has_vertex(to_id))
    {
        return false;
    }

    // Rather inefficient find, change in the future (auto is left for now)
    auto& successors = successor_set[from_id];
    if (successors.find(to_id) != successors.end())
    {
        return false;
    }

    // Emplace for both successors and predecessors
    successors.emplace(to_id);
    predecessor_set[to_id].emplace(from_id);

    ++edge_count;

    return true;
}

// Checks whether the graph has a particular vertex with the node Id
bool Graph::has_vertex(const uint32_t node_id) const
{
    return id_to_key.find(node_id) != id_to_key.end();
}

// Checks whether the graph has a particular edge with the node Id
bool Graph::has_edge(const uint32_t from_id, const uint32_t to_id) const
{
    if (!has_vertex(from_id) || !has_vertex(to_id))
    {
        return false;
    }

    // The graph is not extremely sparse but not extremely filled, so the change into HashSet might have to wait
    const auto& successors = successor_set.at(from_id);
    return successors.find(to_id) != successors.end();
}

// Removes an edge from the graph if it exists
bool Graph::remove_edge(const uint32_t from_id, const uint32_t to_id)
{

    if (!has_vertex(from_id) || !has_vertex(to_id))
    {
        return false;
    }

    // Remove for both successors...
    auto& successors = successor_set[from_id];
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
    auto& predecessors = predecessor_set[to_id];
    const auto predecessor_find = predecessors.find(from_id);
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
    const auto it = successor_set.find(node_id);
    if (it != successor_set.end())
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
    const auto it = predecessor_set.find(node_id);
    if (it != predecessor_set.end())
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
const emhash8::HashSet<uint32_t, XXIntHasher>& Graph::successors(const uint32_t node_id) const
{
    static const emhash8::HashSet<uint32_t, XXIntHasher> empty_set;

    const auto it = successor_set.find(node_id);
    if (it != successor_set.end())
    {
        return it->second;
    }
    else
    {
        return empty_set;
    }
}

// Gets the predecessor set of a certain node
const emhash8::HashSet<uint32_t, XXIntHasher>& Graph::predecessors(const uint32_t node_id) const
{
    static const emhash8::HashSet<uint32_t, XXIntHasher> empty_set;

    const auto it = predecessor_set.find(node_id);
    if (it != predecessor_set.end())
    {
        return it->second;
    }
    else
    {
        return empty_set;
    }
}

// Returns a string representation of the graph
std::string Graph::graph_string() const
{
    std::string result;
    result.reserve(num_edges() + size());

    for (const auto& pair : successor_set)
    {
        const uint32_t node_id = pair.first;
        const std::string& node_name = get_key(node_id);
        result += node_name + ": ";

        const emhash8::HashSet<uint32_t, XXIntHasher>& successors = pair.second;

        // Formatting separator doesn't activate until after first element
        std::string separator = "";
        for (const uint32_t successor_id : successors)
        {
            const std::string& successor_name = get_key(successor_id);
            result += separator + successor_name;
            separator = ", ";
        }
        result += "\n";
    }

    return result;
}

// Iterators using id_to_key to go through the entire graph

Graph::node_iterator Graph::node_begin() const
{
    return id_to_key.cbegin();
}

Graph::node_iterator Graph::node_end() const
{
    return id_to_key.cend();
}
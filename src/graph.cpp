/**
 * Name: Kaelin Wang Hu
 * Date Started: 10/29/2024
 * Description: Efficient graph class implementation
 */
#include "graph.hpp"
#include <iostream>

// The containers all handle it so no need to do much for graph constructor and destructor
Graph::Graph() : edge_count(0), current_normalized_id(1)
{
    // Dummy values for the "failed index" 0
    successor_list.emplace_back(std::vector<uint32_t>());
    predecessor_list.emplace_back(std::vector<uint32_t>());
    id_to_key.emplace_back(std::string());
    node_to_scc.emplace_back(0);
    scc_to_diameter.emplace(0, 0);
}

Graph::~Graph()
{ }

void Graph::initialize_graph(uint32_t num_vertices) noexcept
{
    successor_list.reserve(num_vertices);
    predecessor_list.reserve(num_vertices);
    key_to_id.reserve(num_vertices);
    id_to_key.reserve(num_vertices);
    node_to_scc.reserve(num_vertices);
}

// Gets the size of the graph in terms of vertices
uint32_t Graph::size() const noexcept
{
    return id_to_key.size() - 1;
}


// Gets the number of edges of the graph
uint32_t Graph::num_edges() const noexcept
{
    return edge_count;
}


// Adds a vertex to the graph if it does not already exist (names should all be unique)
bool Graph::add_vertex(const uint32_t node_id, const std::string& key) noexcept
{
    const uint32_t normalized_id = set_normalized_id(node_id);
    // Check that the node doesn't exist yet before adding vertex
    if (has_vertex(normalized_id, true))
    {
        return false;
    }

    key_to_id.emplace(key, normalized_id);
    id_to_key.emplace_back(key);

    // Initialize vector successors and predecessors as well
    successor_list.emplace_back(std::vector<uint32_t>());
    predecessor_list.emplace_back(std::vector<uint32_t>());

    return true;
}

// Adds an edge from a node to another if they do not exist already
bool Graph::add_edge(const uint32_t from_id, const uint32_t to_id, bool is_normalized /*= false*/) noexcept
{
    const uint32_t normalized_from_id = is_normalized ? from_id : get_normalized_id(from_id);
    const uint32_t normalized_to_id = is_normalized ? to_id : get_normalized_id(to_id);
    if (!has_vertex(normalized_from_id, true) || !has_vertex(normalized_to_id, true))
    {
        return false;
    }

    std::vector<uint32_t>& successors = successor_list[normalized_from_id];
    if (std::find(successors.begin(), successors.end(), normalized_to_id) != successors.end())
    {
        return false;
    }

    // Emplace for both successors and predecessors
    successors.emplace_back(normalized_to_id);
    predecessor_list[normalized_to_id].emplace_back(normalized_from_id);

    ++edge_count;

    return true;
}

// Checks whether the graph has a particular vertex with the node Id
bool Graph::has_vertex(const uint32_t node_id, bool is_normalized /*= false*/) const noexcept
{
    const uint32_t normalized_id = is_normalized ? node_id : get_normalized_id(node_id);
    return normalized_id > 0 && normalized_id < id_to_key.size();
}

// Checks whether the graph has a particular edge with the node Id
bool Graph::has_edge(const uint32_t from_id, const uint32_t to_id, bool is_normalized /*= false*/ ) const noexcept
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

// Removes an edge from the graph if it exists
bool Graph::remove_edge(const uint32_t from_id, const uint32_t to_id, bool is_normalized /*= false*/) noexcept
{
    const uint32_t normalized_from_id = is_normalized ? from_id : get_normalized_id(from_id);
    const uint32_t normalized_to_id = is_normalized ? to_id : get_normalized_id(to_id);
    if (!has_vertex(normalized_from_id, true) || !has_vertex(normalized_to_id, true))
    {
        return false;
    }

    // Remove for both successors...
    std::vector<uint32_t>& successors = successor_list[normalized_from_id];
    const auto successor_find = std::find(successors.begin(), successors.end(), normalized_to_id);
    if (successor_find != successors.end())
    {
        successors.erase(successor_find);
    }
    else
    {
        return false;
    }

    // And predecessors
    std::vector<uint32_t>& predecessors = predecessor_list[normalized_to_id];
    const auto predecessor_find = std::find(predecessors.begin(), predecessors.end(), normalized_from_id);
    if (predecessor_find != predecessors.end())
    {
        predecessors.erase(predecessor_find);
    }
    else
    {
        // If failed, add the normalized_to_id back
        successors.emplace_back(normalized_to_id);
        return false;
    }

    --edge_count;

    return true;
}

// Calculates the out_degree of a node (how many edges originate from it)
uint32_t Graph::out_degree(const uint32_t node_id, bool is_normalized /*= false*/) const noexcept
{
    const uint32_t normalized_id = is_normalized ? node_id : get_normalized_id(node_id);
    return successor_list[normalized_id].size();
}


// Calculates the in_degree of a node (how many edges end at it)
uint32_t Graph::in_degree(const uint32_t node_id, bool is_normalized /*= false*/) const noexcept
{
    const uint32_t normalized_id = is_normalized ? node_id : get_normalized_id(node_id);
    return predecessor_list[normalized_id].size();
}


// Gets the node_id based on a particular string key (person name)
uint32_t Graph::get_node_id(const std::string& key) const noexcept
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

// Gets the key (person name) based on the specified node_id, opposite of function above
std::string Graph::get_key(const uint32_t node_id, bool is_normalized /*= false*/) const noexcept
{
    const uint32_t normalized_id = is_normalized ? node_id : get_normalized_id(node_id);
    return id_to_key[normalized_id];
}

// Gets the successor set of a certain node
const std::vector<uint32_t>& Graph::successors(const uint32_t node_id, bool is_normalized /*= false*/) const noexcept
{
    const uint32_t normalized_id = is_normalized ? node_id : get_normalized_id(node_id);
    return successor_list[normalized_id];
}

const std::vector<uint32_t>& Graph::predecessors(const uint32_t node_id, bool is_normalized /*= false*/) const noexcept
{
    const uint32_t normalized_id = is_normalized ? node_id : get_normalized_id(node_id);
    return predecessor_list[normalized_id];
}

// Returns a string representation of the graph
std::string Graph::graph_string() const noexcept
{
    std::string result;
    // *8 for the characters, and + *2 because of the extra values
    result.reserve((num_edges() + size()) * 10);

    const uint32_t graph_size = size();
    for (size_t i = 1; i < graph_size; ++i)
    {
        const std::string& node_name = get_key(i, true);
        result += node_name + ": ";

        const std::vector<uint32_t>& successor_ids = successors(i, true);

        // Formatting separator doesn't activate until after first element
        std::string separator = "";
        for (const uint32_t successor_id : successor_ids)
        {
            const std::string& successor_name = get_key(successor_id, true);
            result += separator + successor_name;
            separator = ", ";
        }
        result += "\n";
    }

    return result;
}

std::deque<uint32_t> Graph::shortest_path(const uint32_t from_id, const uint32_t to_id, bool is_normalized /*= false*/) const noexcept
{
    const uint32_t normalized_from_id = is_normalized ? from_id : get_normalized_id(from_id);
    const uint32_t normalized_to_id = is_normalized ? to_id : get_normalized_id(to_id);
    // Early validation
    if (!has_vertex(normalized_from_id, true) || !has_vertex(normalized_to_id, true))
    {
        return std::deque<uint32_t>();
    }

    // If start and end are the same, return single-node path
    if (normalized_from_id == normalized_to_id)
    {
        return std::deque<uint32_t>{normalized_from_id};
    }

    // Pre-allocate with reasonable sizes
    emhash8::HashMap<uint32_t, uint32_t> parent_map;
    emhash8::HashSet<uint32_t> visited_nodes;
    std::deque<uint32_t> bfs_queue;
    
    parent_map.reserve(size() / 4);
    visited_nodes.reserve(size() / 2);
    
    // Initialize search
    bfs_queue.emplace_back(normalized_from_id);
    visited_nodes.insert(normalized_from_id);
    
    while (!bfs_queue.empty())
    {
        const uint32_t current_node = bfs_queue.front();
        bfs_queue.pop_front();
        
        // Get successors once
        const auto successor_vector = successors(current_node, true);
        
        // Process each successor
        for (const uint32_t successor : successor_vector)
        {
            // Check if we've found the end node before processing
            if (successor == normalized_to_id)
            {
                std::deque<uint32_t> path;
                parent_map[normalized_to_id] = current_node;

                // Reconstruct path
                uint32_t node = normalized_to_id;
                while (node != normalized_from_id)
                {
                    path.emplace_front(node);
                    auto it = parent_map.find(node);
                    node = it->second;
                }
                path.emplace_front(normalized_from_id);
                return path;
            }
            // Process unvisited nodes
            if (visited_nodes.insert(successor).second)
            {
                parent_map[successor] = current_node;
                bfs_queue.emplace_back(successor);
            }
        }
    }
    
    // No path found
    return std::deque<uint32_t>();
}

Graph Graph::collapse_cliques() const noexcept
{
    std::vector<emhash8::HashSet<uint32_t>> all_sccs = find_all_strongly_connected_components();

    emhash8::HashMap<uint32_t, uint32_t> collapsed_node_id;
    collapsed_node_id.reserve(size());
    
    uint32_t next_scc_node_id = size() + 1;
    Graph collapsed_graph;
    
    // Process each SCC
    for (const auto& scc : all_sccs)
    {
        if (scc.size() == 1)
        {
            const uint32_t node_id = *scc.begin();
            collapsed_node_id[node_id] = node_id;
            collapsed_graph.add_vertex(node_id, get_key(node_id, true));
        }
        else
        {
            bool has_predecessors_outside = false;
            bool has_successors_outside = false;
            
            // Check for external connections
            for (const uint32_t node_id : scc)
            {
                // Check successors
                if (const std::vector<uint32_t>& successor_vector = successor_list[node_id]; !successor_vector.empty())
                {
                    for (const uint32_t successor : successor_vector)
                    {
                        if (scc.find(successor) == scc.end())
                        {
                            has_successors_outside = true;
                            break;
                        }
                    }
                }
                
                // Check predecessors
                if (const std::vector<uint32_t>& predecessor_vector = predecessor_list[node_id]; !predecessor_vector.empty())
                {
                    for (const uint32_t predecessor : predecessor_vector)
                    {
                        if (scc.find(predecessor) == scc.end())
                        {
                            has_predecessors_outside = true;
                            break;
                        }
                    }
                }
                if (has_predecessors_outside && has_successors_outside)
                {
                    break;
                }
            }

            if (!has_predecessors_outside || !has_successors_outside)
            {
                // Do not collapse if it will either become a sink or source node
                for (const uint32_t node_id : scc)
                {
                    collapsed_node_id[node_id] = node_id;
                    collapsed_graph.add_vertex(node_id, get_key(node_id, true));
                }
            }
            else
            {
                // Collapse the SCC (entry points already stored above)
                for (const uint32_t node_id : scc)
                {
                    collapsed_node_id[node_id] = next_scc_node_id;
                }
                collapsed_graph.add_vertex(next_scc_node_id, "SCC_" + std::to_string(next_scc_node_id));
                ++next_scc_node_id;
            }
        }
    }

    uint32_t successor_list_size = successor_list.size();
    // Add edges to the collapsed graph
    for (size_t from_id = 1; from_id < successor_list_size; ++from_id)
    {
        for (const uint32_t to_id : successor_list[from_id])
        {
            const uint32_t from_collapsed_id = collapsed_node_id[from_id];
            const uint32_t to_collapsed_id = collapsed_node_id[to_id];
            if (from_collapsed_id != to_collapsed_id)
            {
                collapsed_graph.add_edge(from_collapsed_id, to_collapsed_id, true);
            }
        }
    }

    return collapsed_graph;
}


std::vector<emhash8::HashSet<uint32_t>> Graph::find_all_strongly_connected_components() const noexcept
{
    // Stack frame for recursion simulator in the stack
    struct stack_frame
    {
        uint32_t node_id;
        std::vector<uint32_t>::const_iterator successor_it;
        std::vector<uint32_t>::const_iterator successors_end;
        bool visited;
        stack_frame(const uint32_t _node_id, const std::vector<uint32_t>::const_iterator _begin,
        const std::vector<uint32_t>::const_iterator _end, const bool _visited)
        : node_id(_node_id), successor_it(_begin), successors_end(_end), visited(_visited) {}
    };

    // Map each node to its index in the search order
    emhash8::HashMap<uint32_t, uint32_t> node_index;
    // Map each node to the lowest index reachable from it (lowlink value)
    emhash8::HashMap<uint32_t, uint32_t> node_lowlink;
    // Set of nodes that have been visited
    emhash8::HashSet<uint32_t> visited_nodes;
    // Stack to store the data of current nodes
    std::stack<uint32_t> data_stack;
    // Set to quickly check if a node is on the DFS stack
    emhash8::HashSet<uint32_t> on_stack;
    // Current index used in ordering DFS traversal
    uint32_t current_index = 0;
    // Vector to store all Strongly Connected Components found
    std::vector<emhash8::HashSet<uint32_t>> strongly_connected_components;

    const size_t graph_size = size();
    node_index.reserve(size());
    node_lowlink.reserve(size());
    visited_nodes.reserve(size());
    strongly_connected_components.reserve(size() / 2);  // Around half the nodes are SCCs
    
    // Iterate over all nodes in the graph
    for (size_t it = 1; it != graph_size; ++it)
    {
        if (visited_nodes.find(static_cast<uint32_t>(it)) == visited_nodes.end())
        {
            // Stack to simulate recursive DFS iteratively
            std::stack<stack_frame> dfs_stack;

            // Start from the current node 
            const auto& successor_vector = successors(it);
            dfs_stack.emplace(it, successor_vector.begin(), successor_vector.end(), false);
            while (!dfs_stack.empty())
            {
                stack_frame& current_frame = dfs_stack.top();
                const uint32_t current_node = current_frame.node_id;
                bool has_successors = false; // Controls the recursion flow

                // First time visiting this node
                if (!current_frame.visited)
                {
                    node_index[current_node] = current_index;
                    node_lowlink[current_node] = current_index;
                    ++current_index;
                    visited_nodes.insert(current_node);
                    data_stack.emplace(current_node);
                    on_stack.insert(current_node);
                    current_frame.visited = true;
                }
                // Process all successors of the current node
                while (current_frame.successor_it != current_frame.successors_end)
                {
                    const uint32_t successor_node = *(current_frame.successor_it++);
                    if (visited_nodes.find(successor_node) == visited_nodes.end())
                    {
                        // Successor node has not been visited; recurse on it
                        const auto& successor_successors = successor_list[successor_node];
                        dfs_stack.emplace(successor_node, successor_successors.begin(), successor_successors.end(), false);
                        has_successors = true;
                        break; // Pause processing current node for the next node
                    }
                    else if (on_stack.find(successor_node) != on_stack.end())
                    {
                        // If node has been visited is still on the stack, update the current's lowlink with that node
                        node_lowlink[current_node] = std::min(node_lowlink[current_node], node_index[successor_node]);
                    }
                }

                // So that if a successor was being emplaced it skips the popping
                if (has_successors)
                {
                    continue;
                }

                // Found a root node of a component if lowlink is same as the index
                if (node_lowlink[current_node] == node_index[current_node])
                {
                    // Start popping off the data stack
                    emhash8::HashSet<uint32_t> current_component;
                    uint32_t top_node;
                    do
                    {
                        top_node = data_stack.top();
                        data_stack.pop();
                        on_stack.erase(top_node);
                        current_component.emplace(top_node);
                    }
                    while (top_node != current_node);

                    // Add the found SCC to the list of all SCCs
                    strongly_connected_components.emplace_back(current_component);
                }
                dfs_stack.pop();
                if (!dfs_stack.empty())
                {
                    // Update the lowlink value of the parent node potentially
                    const uint32_t parent_node = dfs_stack.top().node_id;
                    node_lowlink[parent_node] = std::min(node_lowlink[parent_node], node_lowlink[current_node]);
                }
            }
        }
    }

    return strongly_connected_components;
}

void Graph::compute_scc_diameters() noexcept
{
    std::vector<emhash8::HashSet<uint32_t>> all_sccs = find_all_strongly_connected_components();
    
    // Pre-allocate maps
    node_to_scc.clear();
    node_to_scc.resize(size());
    scc_to_diameter.clear();
    scc_to_diameter.reserve(all_sccs.size());
    
    constexpr uint32_t SMALL_SCC_THRESHOLD = 120;
    constexpr uint32_t NUM_SAMPLES = 10;
    
    for (uint32_t scc_id = 0; scc_id < all_sccs.size(); ++scc_id) {
        const auto& scc = all_sccs[scc_id];
        uint32_t diameter = 0;
        
        // Map nodes to SCC IDs
        for (uint32_t node_id : scc)
        {
            node_to_scc[node_id] = scc_id;
        }
        
        if (scc.size() <= SMALL_SCC_THRESHOLD)
        {
            // For small components, run BFS from every node for exact diameter
            for (const uint32_t start_node : scc)
            {
                std::deque<std::pair<uint32_t, uint32_t>> bfs_queue;
                emhash8::HashSet<uint32_t> visited;
                visited.reserve(scc.size());  // Pre-allocate visited set
                bfs_queue.emplace_back(start_node, 0);
                visited.insert(start_node);
                
                while (!bfs_queue.empty())
                {
                    auto [current_node, distance] = bfs_queue.front();
                    bfs_queue.pop_front();
                    diameter = std::max(diameter, distance);
                    // Only look at successors within the same SCC
                    for (const uint32_t successor : successor_list[current_node])
                    {
                        if (scc.find(successor) != scc.end() && visited.insert(successor).second)
                        {
                            bfs_queue.emplace_back(successor, distance + 1);
                        }
                    }
                }
            }
        }
        else
        {
            // For large ones, sample nodes for approximate diameter
            const auto& current_component = std::vector<uint32_t>(scc.begin(), scc.end());
            // Select evenly spaced samples for better
            const size_t current_component_size = current_component.size();
            size_t step = current_component_size / NUM_SAMPLES;
            
            for (size_t i = 0; i < current_component_size && i < NUM_SAMPLES * step; i += step)
            {
                uint32_t start_node = current_component[i];
                std::deque<std::pair<uint32_t, uint32_t>> bfs_queue;
                emhash8::HashSet<uint32_t> visited;
                visited.reserve(scc.size());
                
                bfs_queue.emplace_back(start_node, 0);
                visited.insert(start_node);
                
                while (!bfs_queue.empty())
                {
                    auto [current_node, distance] = bfs_queue.front();
                    bfs_queue.pop_front();
                    diameter = std::max(diameter, distance);
                    for (const uint32_t successor : successor_list[current_node])
                    {
                        if (scc.find(successor) != scc.end() && visited.insert(successor).second)
                        {
                            bfs_queue.emplace_back(successor, distance + 1);
                        }
                    }
                }
            }
        }
        scc_to_diameter[scc_id] = diameter > 0 ? diameter : 1;
    }
}

uint32_t Graph::get_scc_diameter(uint32_t node_id, bool is_normalized /*= false*/) const noexcept
{
    const uint32_t normalized_id = is_normalized ? node_id : get_normalized_id(node_id);
    auto scc_id = node_to_scc[normalized_id];
    if (scc_id != 0)
    {
        const auto diameter_scc_id = scc_to_diameter.find(scc_id);
        if (diameter_scc_id != scc_to_diameter.end())
        {
            return diameter_scc_id->second;
        }
    }
    // diameter for nodes not in SCCs is just 1
    return 1;
}

uint32_t Graph::get_normalized_id(uint32_t node_id) const noexcept
{
    const auto it = id_normalizer.find(node_id);
    return it != id_normalizer.end() ? it->second : 0;
}

uint32_t Graph::set_normalized_id(uint32_t node_id) noexcept
{
    if (current_normalized_id == std::numeric_limits<uint32_t>::max())
    {
        abort();
    }

    id_normalizer[node_id] = current_normalized_id;
    return current_normalized_id++;
}

// Iterators using id_to_key to go through the entire graph (order not guaranteed)

Graph::node_iterator Graph::node_begin() const noexcept
{
    return key_to_id.cbegin();
}

Graph::node_iterator Graph::node_end() const noexcept
{
    return key_to_id.cend();
}
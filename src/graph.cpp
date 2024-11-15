/**
 * Name: Kaelin Wang Hu
 * Date Started: 10/29/2024
 * Description: Efficient graph class implementation
 */
#include "graph.hpp"
#include <iostream>

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
    // Check that the node doesn't exist yet before adding vertex
    if (has_vertex(node_id))
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
    if (!has_vertex(from_id) || !has_vertex(to_id))
    {
        return false;
    }

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
    const auto& successors = successor_list.at(from_id);
    return std::find(successors.begin(), successors.end(), to_id) != successors.end();
}

// Removes an edge from the graph if it exists
bool Graph::remove_edge(const uint32_t from_id, const uint32_t to_id)
{

    if (!has_vertex(from_id) || !has_vertex(to_id))
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
const std::vector<uint32_t>& Graph::successors(const uint32_t node_id) const
{
    return successor_list.at(node_id);
}

const std::vector<uint32_t>& Graph::predecessors(const uint32_t node_id) const
{
    return predecessor_list.at(node_id);
}

// Returns a string representation of the graph
std::string Graph::graph_string() const
{
    std::string result;
    result.reserve((num_edges() + size()) * 8);

    for (const auto& pair : successor_list)
    {
        const uint32_t node_id = pair.first;
        const std::string& node_name = get_key(node_id);
        result += node_name + ": ";

        const std::vector<uint32_t>& successor_ids = pair.second;

        // Formatting separator doesn't activate until after first element
        std::string separator = "";
        for (const uint32_t successor_id : successor_ids)
        {
            const std::string& successor_name = get_key(successor_id);
            result += separator + successor_name;
            separator = ", ";
        }
        result += "\n";
    }

    return result;
}

std::deque<uint32_t> Graph::shortest_path(const uint32_t start, const uint32_t end) const
{
    // Early validation
    if (!has_vertex(start) || !has_vertex(end))
    {
        return std::deque<uint32_t>();
    }

    // If start and end are the same, return single-node path
    if (start == end)
    {
        return std::deque<uint32_t>{start};
    }

    // Pre-allocate with reasonable sizes
    emhash8::HashMap<uint32_t, uint32_t, XXIntHasher> parent_map;
    emhash8::HashSet<uint32_t, XXIntHasher> visited_nodes;
    std::deque<uint32_t> bfs_queue;
    
    parent_map.reserve(size() / 4);
    visited_nodes.reserve(size() / 2);
    
    // Initialize search
    bfs_queue.emplace_back(start);
    visited_nodes.insert(start);
    
    while (!bfs_queue.empty())
    {
        const uint32_t current_node = bfs_queue.front();
        bfs_queue.pop_front();
        
        // Get successors once
        const auto successor_it = successor_list.find(current_node);
        
        // Process each successor
        for (const uint32_t successor : successor_it->second)
        {
            // Check if we've found the end node before processing
            if (successor == end)
            {
                std::deque<uint32_t> path;
                parent_map[end] = current_node;

                // Reconstruct path
                uint32_t node = end;
                while (node != start)
                {
                    path.emplace_front(node);
                    auto it = parent_map.find(node);
                    node = it->second;
                }
                path.emplace_front(start);
                return path;
            }
            // Process unvisited nodes
            if (visited_nodes.insert(successor).second)
            {
                parent_map[successor] = current_node;
                bfs_queue.push_back(successor);
            }
        }
    }
    
    // No path found
    return std::deque<uint32_t>();
}

Graph Graph::collapse_cliques() const
{
    std::vector<emhash8::HashSet<uint32_t, XXIntHasher>> all_sccs = find_all_strongly_connected_components();

    emhash8::HashMap<uint32_t, uint32_t, XXIntHasher> collapsed_node_id;
    collapsed_node_id.reserve(size());
    
    uint32_t next_scc_node_id = num_edges() * 4; // inconsistent size and id correlation so I have to do this
    Graph collapsed_graph;
    
    // Process each SCC
    for (const auto& scc : all_sccs)
    {
        if (scc.size() == 1)
        {
            uint32_t node_id = *scc.begin();
            collapsed_node_id[node_id] = node_id;
            collapsed_graph.add_vertex(node_id, get_key(node_id));
        }
        else
        {
            bool has_predecessors_outside = false;
            bool has_successors_outside = false;
            
            // Check for external connections
            for (uint32_t node_id : scc)
            {
                // Check successors
                if (const auto successor_it = successor_list.find(node_id); successor_it != successor_list.end())
                {
                    for (uint32_t successor : successor_it->second)
                    {
                        if (scc.find(successor) == scc.end())
                        {
                            has_successors_outside = true;
                            break;
                        }
                    }
                }
                
                // Check predecessors
                if (const auto predecessor_it = predecessor_list.find(node_id); predecessor_it != predecessor_list.end())
                {
                    for (uint32_t pred : predecessor_it->second)
                    {
                        if (scc.find(pred) == scc.end())
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
                for (uint32_t node_id : scc)
                {
                    collapsed_node_id[node_id] = node_id;
                    collapsed_graph.add_vertex(node_id, get_key(node_id));
                }
            }
            else
            {
                // Collapse the SCC (entry points already stored above)
                for (uint32_t node_id : scc)
                {
                    collapsed_node_id[node_id] = next_scc_node_id;
                }
                collapsed_graph.add_vertex(next_scc_node_id, "SCC_" + std::to_string(next_scc_node_id));
                next_scc_node_id++;
            }
        }
    }

    // Add edges to the collapsed graph
    for (const auto& [node_id, successors_list] : successor_list)
    {
        uint32_t from_collapsed_id = collapsed_node_id[node_id];
        for (uint32_t successor_id : successors_list)
        {
            uint32_t to_collapsed_id = collapsed_node_id[successor_id];
            if (from_collapsed_id != to_collapsed_id)  // Avoid self-loops
            {
                collapsed_graph.add_edge(from_collapsed_id, to_collapsed_id);
            }
        }
    }

    return collapsed_graph;
}


std::vector<emhash8::HashSet<uint32_t, XXIntHasher>> Graph::find_all_strongly_connected_components() const
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
    emhash8::HashMap<uint32_t, uint32_t, XXIntHasher> node_index;
    // Map each node to the lowest index reachable from it (lowlink value)
    emhash8::HashMap<uint32_t, uint32_t, XXIntHasher> node_lowlink;
    // Set of nodes that have been visited
    emhash8::HashSet<uint32_t, XXIntHasher> visited_nodes;
    // Stack to store the data of current nodes
    std::stack<uint32_t> data_stack;
    // Set to quickly check if a node is on the DFS stack
    emhash8::HashSet<uint32_t, XXIntHasher> on_stack;
    // Current index used in ordering DFS traversal
    uint32_t current_index = 0;
    // Vector to store all Strongly Connected Components found
    std::vector<emhash8::HashSet<uint32_t, XXIntHasher>> strongly_connected_components;
    node_index.reserve(size());
    node_lowlink.reserve(size());
    visited_nodes.reserve(size());
    strongly_connected_components.reserve(size() / 2);  // Around half the nodes are SCCs
    
    // Iterate over all nodes in the graph
    for (auto it = node_begin(); it != node_end(); ++it)
    {
        uint32_t node_id = it->first;
        if (visited_nodes.find(node_id) == visited_nodes.end())
        {
            // Stack to simulate recursive DFS iteratively
            std::stack<stack_frame> dfs_stack;

            // Start from the current node 
            const auto& successors = successor_list.at(node_id);
            dfs_stack.emplace(node_id, successors.begin(), successors.end(), false);
            while (!dfs_stack.empty())
            {
                stack_frame& current_frame = dfs_stack.top();
                uint32_t current_node = current_frame.node_id;
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
                    uint32_t successor_node = *(current_frame.successor_it++);
                    if (visited_nodes.find(successor_node) == visited_nodes.end())
                    {
                        // Successor node has not been visited; recurse on it
                        const auto& successor_successors = successor_list.at(successor_node);
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
                    emhash8::HashSet<uint32_t, XXIntHasher> current_component;
                    uint32_t top_node;
                    do
                    {
                        top_node = data_stack.top();
                        data_stack.pop();
                        on_stack.erase(top_node);
                        current_component.emplace(top_node);
                    } while (top_node != current_node);

                    // Add the found SCC to the list of all SCCs
                    strongly_connected_components.emplace_back(current_component);
                }
                dfs_stack.pop();
                if (!dfs_stack.empty())
                {
                    // Update the lowlink value of the parent node potentially
                    uint32_t parent_node = dfs_stack.top().node_id;
                    node_lowlink[parent_node] = std::min(node_lowlink[parent_node], node_lowlink[current_node]);
                }
            }
        }
    }

    return strongly_connected_components;
}

void Graph::compute_scc_diameters()
{
    std::vector<emhash8::HashSet<uint32_t, XXIntHasher>> all_sccs = find_all_strongly_connected_components();
    
    // Pre-allocate maps
    node_to_scc.clear();
    node_to_scc.reserve(size());
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
            for (uint32_t start_node : scc)
            {
                std::deque<std::pair<uint32_t, uint32_t>> bfs_queue;
                emhash8::HashSet<uint32_t, XXIntHasher> visited;
                visited.reserve(scc.size());  // Pre-allocate visited set
                
                bfs_queue.emplace_back(start_node, 0);
                visited.insert(start_node);
                
                while (!bfs_queue.empty())
                {
                    auto [current_node, distance] = bfs_queue.front();
                    bfs_queue.pop_front();
                    diameter = std::max(diameter, distance);
                    // Only look at successors within the same SCC
                    for (uint32_t successor : successor_list.at(current_node))
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
            auto current_component = std::vector<uint32_t>(scc.begin(), scc.end());
            // Select evenly spaced samples for better 
            size_t step = current_component.size() / NUM_SAMPLES;
            
            for (size_t i = 0; i < current_component.size() && i < NUM_SAMPLES * step; i += step)
            {
                uint32_t start_node = current_component[i];
                std::deque<std::pair<uint32_t, uint32_t>> bfs_queue;
                emhash8::HashSet<uint32_t, XXIntHasher> visited;
                visited.reserve(scc.size());
                
                bfs_queue.emplace_back(start_node, 0);
                visited.insert(start_node);
                
                while (!bfs_queue.empty())
                {
                    auto [current_node, distance] = bfs_queue.front();
                    bfs_queue.pop_front();
                    diameter = std::max(diameter, distance);
                    for (uint32_t successor : successor_list.at(current_node))
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

uint32_t Graph::get_scc_diameter(uint32_t node_id) const
{
    auto it = node_to_scc.find(node_id);
    if (it != node_to_scc.end())
    {
        uint32_t scc_id = it->second;
        auto diameter_it = scc_to_diameter.find(scc_id);
        if (diameter_it != scc_to_diameter.end())
        {
            return diameter_it->second > 0 ? diameter_it->second : 1;
        }
    }
    // diameter for nodes not in SCCs is just 1
    return 1;
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
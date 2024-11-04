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
const emhash8::HashSet<uint32_t, XXIntHasher> Graph::successors(const uint32_t node_id) const
{
    // Create a new HashSet and add all the relevant nodes to that successors HashSet
    emhash8::HashSet<uint32_t, XXIntHasher> successor_set;
    const auto it = successor_list.find(node_id);
    if (it != successor_list.end())
    {
        for (const uint32_t successor_id : it->second)
        {
            successor_set.insert(successor_id);
        }

    }

    return successor_set;
}

// Gets the predecessor set of a certain node
const emhash8::HashSet<uint32_t, XXIntHasher> Graph::predecessors(const uint32_t node_id) const
{
    // Create a new HashSet and add all the relevant nodes to that successors HashSet
    emhash8::HashSet<uint32_t, XXIntHasher> predecessor_set;
    const auto it = predecessor_list.find(node_id);
    if (it != predecessor_list.end())
    {
        for (const uint32_t predecessor_id : it->second)
        {
            predecessor_set.insert(predecessor_id);
        }

    }
    return predecessor_set;
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

// Finds the shortest path from one node to another
std::deque<uint32_t> Graph::shortest_path(const uint32_t start, const uint32_t end) const
{
    if (!has_vertex(start) || !has_vertex(end))
    {
        return std::deque<uint32_t>(); // Return an empty path if start or end node doesn't exist
    }

    // Map to keep track of the path (child node -> parent node)
    emhash8::HashMap<uint32_t, uint32_t, XXIntHasher> parent_map;
    // Set to keep track of visited nodes
    emhash8::HashSet<uint32_t, XXIntHasher> visited_nodes;
    // Queue for BFS traversal
    std::queue<uint32_t> bfs_queue;

    // Initialize BFS
    bfs_queue.emplace(start);
    visited_nodes.emplace(start);

    while (!bfs_queue.empty())
    {
        uint32_t current_node = bfs_queue.front();
        bfs_queue.pop();

        // If we have reached the end node, reconstruct the path
        if (current_node == end)
        {
            std::deque<uint32_t> path;
            uint32_t node = end;
            while (node != start)
            {
                path.emplace_front(node);
                node = parent_map[node];
            }
            path.emplace_front(start); // Add the start node at the beginning

            return path;
        }

        // Explore successors of the current node
        const auto& successors_set = successors(current_node);
        for (uint32_t successor : successors_set)
        {
            if (visited_nodes.find(successor) == visited_nodes.end())
            {
                visited_nodes.emplace(successor);
                parent_map[successor] = current_node; // Record the parent of the successor
                bfs_queue.emplace(successor);
            }
        }
    }

    // If the end node is not reachable from the start node, return an empty path
    return std::deque<uint32_t>();
}

Graph Graph::collapse_cliques() const
{
    // Get all SCCs in the graph
    std::vector<emhash8::HashSet<uint32_t, XXIntHasher>> all_sccs = find_all_strongly_connected_components();

    // Map each node to its collapsed node ID
    emhash8::HashMap<uint32_t, uint32_t, XXIntHasher> collapsed_node_id;
    collapsed_node_id.reserve(size());  // Reserve space for efficiency

    // Counter for new node IDs for collapsed SCCs
    uint32_t next_scc_node_id = size();  // Start after existing node IDs to avoid conflicts

    // Create a new graph to represent the collapsed SCCs
    Graph collapsed_graph;

    // Process each SCC
    for (const auto& scc : all_sccs)
    {
        if (scc.size() == 1)
        {
            // For SCCs of size 1, do not collapse
            uint32_t node_id = *scc.begin();
            collapsed_node_id[node_id] = node_id;

            // Add the node to the collapsed graph if not already added
            if (!collapsed_graph.has_vertex(node_id))
            {
                collapsed_graph.add_vertex(node_id, get_key(node_id));
            }
        }
        else
        {
            // Determine if SCC has predecessors or successors outside the SCC
            bool has_predecessors_outside = false;
            bool has_successors_outside = false;

            // Check for external predecessors and successors
            for (uint32_t node_id : scc)
            {
                // Check successors of the node
                const auto successor_it = successor_list.find(node_id);
                if (successor_it != successor_list.end())
                {
                    const std::vector<uint32_t>& successors = successor_it->second;
                    for (uint32_t succ : successors)
                    {
                        // Check that it is not an internal SCC nodes
                        if (scc.find(succ) == scc.end())
                        {
                            has_successors_outside = true;
                            break;  // No need to check more successors
                        }
                    }
                }

                // Check predecessors of the node
                const auto predecessor_it = predecessor_list.find(node_id);
                if (predecessor_it != predecessor_list.end())
                {
                    const std::vector<uint32_t>& predecessors = predecessor_it->second;
                    for (uint32_t pred : predecessors)
                    {
                        if (scc.find(pred) == scc.end())
                        {
                            has_predecessors_outside = true;
                            break;  // No need to check more predecessors
                        }
                    }
                }

                // If both external predecessors and successors are found, we can collapse
                if (has_predecessors_outside && has_successors_outside)
                {
                    break;  // No need to check further nodes in the SCC
                }
            }

            if (!has_predecessors_outside || !has_successors_outside)
            {
                // Do not collapse the SCC; map nodes to themselves
                for (uint32_t node_id : scc)
                {
                    collapsed_node_id[node_id] = node_id;
                    // Add the node to the collapsed graph
                    collapsed_graph.add_vertex(node_id, get_key(node_id));
                }
            }
            else
            {
                // Collapse the SCC into a single node
                uint32_t scc_node_id = next_scc_node_id++;
                for (uint32_t node_id : scc)
                {
                    collapsed_node_id[node_id] = scc_node_id;
                }
                collapsed_graph.add_vertex(scc_node_id, "SCC_" + std::to_string(scc_node_id));
            }
        }
    }

    // Define a struct for edge pairs to track unique edges
    struct edge_pair
    {
        uint32_t from, to;
        bool operator==(const edge_pair& other) const
        {
            return from == other.from && to == other.to;
        }
        // Had to add this for internal hashmap comparisons
        bool operator<(const edge_pair& other) const
        {
            if (from != other.from) return from < other.from;
            return to < other.to;
        }
    };
    
    // Hasher for edge pairs
    struct edge_hasher
    {
        size_t operator()(const edge_pair& edge) const
        {
            uint64_t data[2] = { edge.from, edge.to };
            return XXH64(&data, sizeof(data), 0);
        }
    };
    
    // HashSet to track unique edges
    emhash8::HashSet<edge_pair, edge_hasher> unique_edges;
    unique_edges.reserve(num_edges());  // Reserve space for efficiency
    
    // Add edges to the collapsed graph
    for (const auto& [node_id, successors_list] : successor_list)
    {
        uint32_t from_collapsed_id = collapsed_node_id[node_id];
        for (uint32_t successor_id : successors_list)
        {
            uint32_t to_collapsed_id = collapsed_node_id[successor_id];

            // Avoid adding self-loops
            if (from_collapsed_id == to_collapsed_id)
            {
                continue;
            }

            // Create an edge pair
            edge_pair edge{ from_collapsed_id, to_collapsed_id };

            // Add edge only if it's unique
            if (unique_edges.insert(edge).second)  // If insertion is successful
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
    struct stack_frame {
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
    // Get all SCCs
    std::vector<emhash8::HashSet<uint32_t, XXIntHasher>> all_sccs = find_all_strongly_connected_components();

    node_to_scc_id.clear();
    scc_id_to_diameter.clear();

    uint32_t scc_id_counter = 0;
    const uint32_t SMALL_SCC_THRESHOLD = 100; // Threshold for SCC precise diameter measurement

    for (const auto& scc : all_sccs)
    {
        uint32_t scc_id = scc_id_counter++;
        uint32_t diameter = 0;

        // Map nodes to SCC ID
        for (uint32_t node_id : scc)
        {
            node_to_scc_id[node_id] = scc_id;
        }

        // Compute diameter
        if (scc.size() <= SMALL_SCC_THRESHOLD)
        {
            // Exact computation for small SCCs
            for (uint32_t node_id : scc)
            {
                // BFS from node_id
                std::queue<std::pair<uint32_t, uint32_t>> bfs_queue;
                emhash8::HashSet<uint32_t, XXIntHasher> visited;
                bfs_queue.emplace(node_id, 0);
                visited.insert(node_id);

                while (!bfs_queue.empty())
                {
                    auto [current_node, distance] = bfs_queue.front();
                    bfs_queue.pop();

                    diameter = std::max(diameter, distance);

                    const auto& successors = successor_list.at(current_node);
                    for (uint32_t successor : successors)
                    {
                        if (scc.find(successor) != scc.end() && visited.find(successor) == visited.end())
                        {
                            visited.insert(successor);
                            bfs_queue.emplace(successor, distance + 1);
                        }
                    }
                }
            }
        }
        else
        {
            // Approximate computation for large SCCs
            const size_t NUM_SAMPLES = 5;
            size_t samples = 0;
            for (uint32_t node_id : scc)
            {
                // BFS from node_id
                std::queue<std::pair<uint32_t, uint32_t>> bfs_queue;
                emhash8::HashSet<uint32_t, XXIntHasher> visited;
                bfs_queue.emplace(node_id, 0);
                visited.insert(node_id);

                while (!bfs_queue.empty())
                {
                    auto [current_node, distance] = bfs_queue.front();
                    bfs_queue.pop();

                    diameter = std::max(diameter, distance);

                    const auto& successors = successor_list.at(current_node);
                    for (uint32_t successor : successors)
                    {
                        if (scc.find(successor) != scc.end() && visited.find(successor) == visited.end())
                        {
                            visited.insert(successor);
                            bfs_queue.emplace(successor, distance + 1);
                        }
                    }
                }

                if (++samples >= NUM_SAMPLES)
                {
                    break;
                }
            }
        }

        scc_id_to_diameter[scc_id] = diameter;
    }
}

uint32_t Graph::get_scc_diameter(uint32_t node_id) const
{
    auto it = node_to_scc_id.find(node_id);
    if (it != node_to_scc_id.end())
    {
        uint32_t scc_id = it->second;
        auto diameter_it = scc_id_to_diameter.find(scc_id);
        if (diameter_it != scc_id_to_diameter.end())
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
#include <iostream>
#include <vector>
#include <fstream>
#include <map>
#include <unordered_set>
#include <queue>
#include <algorithm>
using namespace std;

class Edge
{
public:
    class Node *To;
    class Node *From;
    Edge *reverse;
    int original;
    int residual;
};

class Node
{
public:
    Node()
    {
        id = -1;
        type = DICE;
        letters = vector<bool>(26, false); // initialize every letter to false
        visited = 0;
        backedge = nullptr;
    }

    int id;
    enum Node_Type
    {
        SOURCE,
        SINK,
        DICE,
        LETTER
    } type;

    vector<bool> letters;
    int visited;
    vector<Edge *> adj;
    Edge *backedge;
};

class Graph
{
public:
    vector<Node*> nodes;
    vector<Edge*> BFS();
    int can_i_spell(int size);
    int minnodes;
    void delete_half_graph(); /* Resize nodes to minnodes, deletes WORD nodes,
                                the SINK, and all edges except the SOURCE to
                                DICE edges */
};

vector<Edge*> Graph::BFS()
{
    Node *source = nodes[0];
    Node *sink = nodes.back();

    // Initialize to unvisited and backedges to null
    for (Node *node : nodes)
    {
        node->visited = 0;
        node->backedge = nullptr;
    }

    vector<Edge *> path;
    multimap<int, Node *> q;  // Use of multimap allows distance storing
    q.insert({0, source});    // Start with source node at distance 0

    while (!q.empty())
    {
        // Extract node with minimum distance from the multimap
        auto it = q.begin();
        Node *current = it->second;
        current->visited = 1;
        q.erase(q.begin());

        if (current == sink)
        {
            Node *currNode = current;
            while (currNode != source)
            {
                path.push_back(currNode->backedge);
                currNode = currNode->backedge->From;
            }
            reverse(path.begin(), path.end());
            return path;
        }

        // Check adjacent nodes
        for (Edge *edge : current->adj)
        {
            if (edge->original == 1 && edge->To->visited == 0)
            {
                edge->To->backedge = edge;
                q.insert({edge->residual, edge->To});
            }
        }
    }

    // No path found
    path.clear();
    return path;
}

int Graph::can_i_spell(int size)
{
    Node *source = nodes[0];
    Node *sink = nodes.back();

    // Continue finding augmenting paths until no more are found
    while (true)
    {
        vector<Edge *> path = BFS(); // Find current augmenting path

        // If no path is found, exit the loop
        if (path.empty())
        {
            break;
        }

        // Update edge capacities along the path
        for (Edge *edge : path)
        {
            edge->original = 0;
            edge->residual = 1;
            edge->reverse->residual = 0;
            edge->reverse->original = 1;
        }
    }

    int count = 0;
    vector<int> diceIDs; // Vector to store dice IDs for the letters
    for (Edge *edge : sink->adj)
    {
        // Check for validity
        if (edge && edge->reverse)
        {
            Node *from = edge->reverse->From;
            if (from && from->type == Node::LETTER && edge->reverse->residual == 1)
            {
                Node *currDice;
                for (Edge *edge_maybe : from->adj)
                {
                    if (edge_maybe->original == 1)
                    {
                        currDice = edge_maybe->To;

                        for (size_t i = 0; i < nodes.size(); i++)
                        {
                            if (nodes[i] == currDice)
                            {
                                diceIDs.push_back(i - 1);
                            }
                        }
                    }
                }
                count++;
            }
        }
    }

    // If number of letters grabbed matches word size
    if (count == size)
    {

        for (int i = 0; i < diceIDs.size() - 1; i++)
        {
            cout << diceIDs[i] << ",";
        }
        cout << diceIDs[diceIDs.size() - 1];
        return 1;
    }
    else
    {
        return 0;
    }
}

void Graph::delete_half_graph()
{
    // Resize the nodes vector to minnodes
    nodes.resize(minnodes);

    for (size_t i = minnodes; i < nodes.size(); i++)
    {
        Node *node = nodes[i];

        // Delete Edges
        for (Edge *edge : node->adj)
        {
            delete edge;
        }

        node->adj.clear(); // Clear adj list

        delete node;
    }

    for (Node *node : nodes)
    {
        if (node->type != Node::SOURCE)
        {
            for (Edge *edge : node->adj)
            {
                delete edge;
            }
            node->adj.clear();
        }
    }
}

int main(int argc, char *argv[])
{
    ifstream dicefile(argv[1]);
    ifstream wordfile(argv[2]);

    if (!dicefile.is_open() || !wordfile.is_open())
    {
        cerr << "Failed to open one or both of the files." << endl;
        return 1;
    }

    vector<string> dice;
    string line;

    while (getline(dicefile, line))
    {
        dice.push_back(line);
    }

    while (getline(wordfile, line))
    {
        int word_size = line.size();
        Graph graph;

        // Create SOURCE node
        Node *sourceNode = new Node();
        sourceNode->type = Node::SOURCE;
        sourceNode->id = 0;
        graph.nodes.push_back(sourceNode);

        // Create DICE nodes
        int count = 1;
        for (const string &die : dice)
        {
            Node *diceNode = new Node();
            diceNode->type = Node::DICE;
            diceNode->id = count;
            count++;

            for (char letter : die)
            {
                int index = letter - 'A'; // Calculate index by letter
                diceNode->letters[index] = true;
            }

            graph.nodes.push_back(diceNode);
        }

        // Set minnodes to the amount of DICE plus one SOURCE for retainment
        graph.minnodes = count;

        // Create LETTER nodes
        for (char letter : line)
        {
            Node *letterNode = new Node();
            letterNode->type = Node::LETTER;
            letterNode->id = count;
            count++;

            int index = letter - 'A';
            letterNode->letters[index] = true;

            graph.nodes.push_back(letterNode);
        }

        // Create SINK node
        Node *sinkNode = new Node();
        sinkNode->type = Node::SINK;
        sinkNode->id = count;
        graph.nodes.push_back(sinkNode);

        // Connect SOURCE node to DICE nodes
        for (Node *sourceNode : graph.nodes)
        {
            if (sourceNode->type == Node::SOURCE)
            {
                for (Node *diceNode : graph.nodes)
                {
                    if (diceNode->type == Node::DICE)
                    {
                        Edge *edge = new Edge();
                        edge->From = sourceNode;
                        edge->To = diceNode;
                        edge->original = 1;
                        edge->residual = 0;
                        sourceNode->adj.push_back(edge); // Add edge to SOURCE node's adj list

                        // Create reverse edge
                        Edge *reverseEdge = new Edge();
                        reverseEdge->From = diceNode;
                        reverseEdge->To = sourceNode;
                        reverseEdge->original = 0;
                        reverseEdge->residual = 1;
                        edge->reverse = reverseEdge;
                        reverseEdge->reverse = edge;
                        diceNode->adj.push_back(reverseEdge); // Add reverse edge to LETTER node's adj list
                    }
                }
            }
        }

        // Connect DICE nodes to LETTER nodes
        for (Node *diceNode : graph.nodes)
        {
            if (diceNode->type == Node::DICE)
            {
                for (Node *letterNode : graph.nodes)
                {
                    if (letterNode->type == Node::LETTER)
                    {
                        bool letter_is_present = false;
                        for (size_t i = 0; i < diceNode->letters.size(); i++)
                        {
                            if (diceNode->letters[i] && letterNode->letters[i])
                            {
                                letter_is_present = true;
                                break;
                            }
                        }

                        if (letter_is_present)
                        {
                            Edge *edge = new Edge();
                            edge->From = diceNode;
                            edge->To = letterNode;
                            edge->original = 1;
                            edge->residual = 0;
                            diceNode->adj.push_back(edge); // Add edge to DICE node's adj list

                            // Create reverse edge
                            Edge *reverseEdge = new Edge();
                            reverseEdge->From = letterNode;
                            reverseEdge->To = diceNode;
                            reverseEdge->original = 0;
                            reverseEdge->residual = 1;
                            edge->reverse = reverseEdge;
                            reverseEdge->reverse = edge;
                            letterNode->adj.push_back(reverseEdge); // Add reverse edge to LETTER node's adj list
                        }
                    }
                }
            }
        }

        // Connect LETTER nodes to SINK node
        for (Node *letterNode : graph.nodes)
        {
            if (letterNode->type == Node::LETTER)
            {
                for (Node *sinkNode : graph.nodes)
                {
                    if (sinkNode->type == Node::SINK)
                    {
                        Edge *edge = new Edge();
                        edge->From = letterNode;
                        edge->To = sinkNode;
                        edge->original = 1;
                        edge->residual = 0;
                        letterNode->adj.push_back(edge);

                        // Create reverse edge
                        Edge *reverseEdge = new Edge();
                        reverseEdge->From = sinkNode;
                        reverseEdge->To = letterNode;
                        reverseEdge->original = 0;
                        reverseEdge->residual = 1;
                        edge->reverse = reverseEdge;
                        reverseEdge->reverse = edge;
                        sinkNode->adj.push_back(reverseEdge);
                    }
                }
            }
        }

        if (graph.can_i_spell(word_size) == 1)
        {
            cout << ": " << line << endl;
        }
        else
        {
            cout << "Cannot spell " << line << endl;
        }

        graph.delete_half_graph();
    }

    dicefile.close();
    wordfile.close();
    return 0;
}

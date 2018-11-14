#ifndef GRAPH_H
#define GRAPH_H

#include <map>
#include <vector>
#include <memory>

struct Edge
{
    int v1;
    int v2;
};

class Graph
{
public:
    Graph();

    Graph(const int _num_nodes, const int _num_edges, const int* edges_from,
          const int* edges_to, const int* vertex_weights);
    Graph(const int _num_nodes, const int _num_edges, const int* edges_from,
          const int* edges_to);
    int num_nodes;
    int num_edges;
    std::vector< std::vector< int > > adj_list;
    std::vector< std::pair < int, int > > edge_list;

    int v_num;
    int e_num;
    std::vector< Edge > edge;

    // These vectors are all indexed by vertex number.
    std::vector< int > edge_weight;
    std::vector< int > dscore;
    std::vector< long long > time_stamp;

    std::vector< int > v_weight;
    // The numbers here are edge indices.
    std::vector< std::vector< int > > v_edges;
    // v_adj is essentially just adj_list, but the 0 index is different.
    std::vector< std::vector< int > > v_adj;
    std::vector< int > v_degree;

    int c_size;
    std::vector< int > v_in_c;
    std::vector< int > remove_cand;
    std::vector< int > index_in_remove_cand;
    long long now_weight;

    // Don't need to remember best, since that's handled elsewhere.
  
    // These vectors are indexed by edge number.
    std::vector< int > uncov_stack;
    std::vector< int > index_in_uncov_stack;

    // These vectors are indexed by vertex number again.
    std::vector< int > conf_change;
    std::vector< int > tabu_list;

    int ave_weight;
    int delta_total_weight;
    int threshold;
    double p_scale;

    unsigned int seed;
    int mode;
};

class GSet
{
public:
    GSet();

    void InsertGraph(int gid, std::shared_ptr<Graph> graph);
    std::shared_ptr<Graph> Sample();
    std::shared_ptr<Graph> Get(int gid);
    std::map<int, std::shared_ptr<Graph> > graph_pool;
};

extern GSet GSetTrain;
extern GSet GSetTest;

#endif

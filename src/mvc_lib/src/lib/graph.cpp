#include "graph.h"
#include <cassert>
#include <iostream>
#include <random>

Graph::Graph() : num_nodes(0), num_edges(0), c_size(0), now_weight(0),
  ave_weight(1), delta_total_weight(0), threshold(0 /* (int)(0.5 * v_num) */),
  p_scale(0.3), seed(0), mode(0)
{
    edge_list.clear();
    adj_list.clear();
    edge_weight.clear();
    dscore.clear();
    time_stamp.clear();
    v_weight.clear();
    v_edges.clear();
    v_degree.clear();
    v_in_c.clear();
    remove_cand.clear();
    index_in_remove_cand.clear();
    uncov_stack.clear();
    index_in_uncov_stack.clear();
    conf_change.clear();
    tabu_list.clear();
}
Graph::Graph(const int _num_nodes, const int _num_edges, const int* edges_from,
             const int* edges_to)
{
  std::vector<int> vertex_weights(_num_nodes, 0);
  Graph(_num_nodes, _num_edges, edges_from, edges_to, vertex_weights.data());
}

Graph::Graph(const int _num_nodes, const int _num_edges, const int* edges_from,
             const int* edges_to, const int* vertex_weights)
        : num_nodes(_num_nodes), num_edges(_num_edges), v_num(_num_nodes),
        e_num(_num_edges), c_size(0), now_weight(0), ave_weight(1),
        delta_total_weight(0), threshold((int)(0.5 * _num_nodes)),
        p_scale(0.3), seed(0), mode(0)
{
    int e;
    int v1, v2;

    edge_list.resize(num_edges);
    adj_list.resize(num_nodes, std::vector<int>());

    for (int i = 0; i < num_edges; ++i)
    {
        int x = edges_from[i], y = edges_to[i];
        adj_list[x].push_back(y);
        adj_list[y].push_back(x);
        edge_list[i] = std::make_pair(edges_from[i], edges_to[i]);
    }

    v_weight = std::vector<int>(vertex_weights, vertex_weights + v_num);

    for (e = 0; e < e_num; e++)
    {
        edge.push_back({edges_from[e], edges_to[e]});
    }

    v_degree = std::vector<int>(v_num, 0);
    for (e = 0; e < e_num; e++)
    {
        v_degree[edge[e].v1]++;
        v_degree[edge[e].v2]++;
    }

    num_nodes = v_num;
    num_edges = e_num;

    edge.resize(e_num, { 0, 0 });
    index_in_uncov_stack.resize(e_num, 0);
    v_edges.resize(v_num + 1, std::vector<int>());
    remove_cand.resize(v_num + 1, 0);
    index_in_remove_cand.resize(v_num + 1, 0);

    v_degree.resize(v_num + 1, 0);
    tabu_list.resize(v_num + 1, 0);
    v_in_c.resize(v_num + 1, 0);
    dscore.resize(v_num + 1, 0);
    conf_change.resize(v_num + 1, 1);
    time_stamp.resize(v_num + 1, 0);
    edge_weight.resize(e_num, 1);

    v_adj.resize(v_num + 1, std::vector<int>());

    for (e = 0; e < e_num; e++)
    {
        v1 = edge[e].v1;
        v2 = edge[e].v2;

        v_edges[v1].push_back(e);
        v_edges[v2].push_back(e);

        v_adj[v1].push_back(v2);
        v_adj[v2].push_back(v1);

    }
}

GSet::GSet()
{
    graph_pool.clear();
}

void GSet::InsertGraph(int gid, std::shared_ptr<Graph> graph)
{
    assert(graph_pool.count(gid) == 0);

    graph_pool[gid] = graph;
}

std::shared_ptr<Graph> GSet::Get(int gid)
{
    assert(graph_pool.count(gid));
    return graph_pool[gid];
}

std::shared_ptr<Graph> GSet::Sample()
{
    assert(graph_pool.size());
    int gid = rand() % graph_pool.size();
    assert(graph_pool[gid]);
    return graph_pool[gid];
}

GSet GSetTrain, GSetTest;

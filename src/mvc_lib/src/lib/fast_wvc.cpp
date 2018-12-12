#include <graph.h>
#include <fast_wvc.h>

#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <string>
#include <algorithm>
#include <random>
#include <cmath>
#include <cstring>
#include <cassert>

using namespace std;

Graph BuildInstance(string filename)
{
    string tmp;
    string sign;
    int v, e;
    int v1, v2;

    ifstream infile(filename);
    if (!infile)
    {
        cerr << "Could not open " << filename << endl;
        abort();
    }

    int v_num, e_num;
    infile >> sign >> tmp >> v_num >> e_num;
    std::vector<int> v_weight(v_num, 0);

    for (v = 0; v < v_num; v++)
    {
        infile >> sign >> tmp >> v_weight[v];
    }
    std::vector<int> edges_from;
    std::vector<int> edges_to;

    for (e = 0; e < e_num; e++)
    {
        infile >> tmp >> v1 >> v2;

        // Translate from base 1 to base 0 indexing.
        edges_from.push_back(v1 - 1);
        edges_to.push_back(v2 - 1);
    }
    infile.close();

    return Graph(v_num, e_num, edges_from.data(), edges_to.data(), v_weight.data());
}

void ResetRemoveCand(Graph &g)
{
    int v;

    g.remove_cand.clear();

    for (v = 0; v < g.v_num; v++)
    {
        if (g.v_in_c[v] == 1)
        {
            g.index_in_remove_cand[v] = g.remove_cand.size();
            g.remove_cand.push_back(v);
        }
        else
        {
            g.index_in_remove_cand[v] = -1;
        }
    }
}

void Uncover(Graph &g, int e)
{
    g.index_in_uncov_stack[e] = g.uncov_stack.size();
    g.uncov_stack.push_back(e);
}

void Cover(Graph &g, int e)
{
    int index, last_uncov_edge;
    assert(g.uncov_stack.size() > 0);
    last_uncov_edge = g.uncov_stack.back();
    g.uncov_stack.pop_back();
    index = g.index_in_uncov_stack[e];
    g.uncov_stack[index] = last_uncov_edge;
    g.index_in_uncov_stack[last_uncov_edge] = index;
    g.index_in_uncov_stack[e] = -1;
}

void Add(Graph &g, int v)
{
    int i, e, n;
    int edge_count = g.v_degree[v];

    g.v_in_c[v] = 1;
    g.c_size++;
    g.dscore[v] = -g.dscore[v];
    g.now_weight += g.v_weight[v];

    assert(g.index_in_remove_cand[v] == -1);
    g.index_in_remove_cand[v] = g.remove_cand.size();
    g.remove_cand.push_back(v);

    for (i = 0; i < edge_count; i++)
    {
        e = g.v_edges[v][i];
        n = g.v_adj[v][i];

        if (g.v_in_c[n] == 0)
        {
            g.dscore[n] -= g.edge_weight[e];
            g.conf_change[n] = 1;
            if (g.index_in_uncov_stack[e] >= 0) {
              Cover(g, e);
            }
        }
        else
        {
            g.dscore[n] += g.edge_weight[e];
        }
    }
}

void Remove(Graph &g, int v)
{
    int i, e, n;
    int edge_count = g.v_degree[v];

    g.v_in_c[v] = 0;
    g.c_size--;
    g.dscore[v] = -g.dscore[v];
    g.conf_change[v] = 0;

    int last_remove_cand_v = g.remove_cand.back();
    g.remove_cand.pop_back();
    int index = g.index_in_remove_cand[v];
    g.remove_cand[index] = last_remove_cand_v;
    g.index_in_remove_cand[last_remove_cand_v] = index;
    g.index_in_remove_cand[v] = -1;

    g.now_weight -= g.v_weight[v];

    for (i = 0; i < edge_count; i++)
    {
        e = g.v_edges[v][i];
        n = g.v_adj[v][i];

        if (g.v_in_c[n] == 0)
        {
            g.dscore[n] += g.edge_weight[e];
            g.conf_change[n] = 1;
            Uncover(g, e);
        }
        else
        {
            g.dscore[n] -= g.edge_weight[e];
        }
    }
}

int UpdateTargetSize(Graph &g)
{
    int v;
    int best_remove_v;
    double best_dscore;
    double dscore_v;
    assert(g.remove_cand.size() > 0);

    best_remove_v = g.remove_cand[0];
    best_dscore = (double)g.v_weight[best_remove_v] / (double)abs(g.dscore[best_remove_v]);

    if (g.dscore[best_remove_v] != 0)
    {
        for (unsigned int i = 1; i < g.remove_cand.size(); i++)
        {
            v = g.remove_cand[i];
            if (g.dscore[v] == 0) break;
            dscore_v = (double)g.v_weight[v] / (double)abs(g.dscore[v]);
            if (dscore_v > best_dscore)
            {
                best_dscore = dscore_v;
                best_remove_v = v;
            }
        }
    }

    Remove(g, best_remove_v);

    return best_remove_v;
}

int ChooseRemoveV(Graph &g)
{
    assert(g.remove_cand.size() > 0);
    int i, v;
    double dscore_v, dscore_remove_v;
    int remove_v = g.remove_cand[rand() % g.remove_cand.size()];
    int to_try = 50;

    for (i = 1; i < to_try; i++)
    {
        v = g.remove_cand[rand() % g.remove_cand.size()];
        dscore_v = (double)g.v_weight[v] / (double)abs(g.dscore[v]);
        dscore_remove_v = (double)g.v_weight[remove_v] / (double)abs(g.dscore[remove_v]);

        if (g.tabu_list[v] == 1)
        {
            continue;
        }
        if (dscore_v < dscore_remove_v)
        {
            continue;
        }
        if (dscore_v > dscore_remove_v)
        {
            remove_v = v;
        }
        else if (g.time_stamp[v] < g.time_stamp[remove_v])
        {
            remove_v = v;
        }
    }
    return remove_v;
}

int ChooseAddFromV(Graph &g)
{
    int v;
    int add_v = 0;
    double improvemnt = 0.0;
    double dscore_v;

    for (v = 0; v < g.v_num; v++)
    {
        if (g.v_in_c[v] == 1)
        {
            continue;
        }
        if (g.conf_change[v] == 0)
        {
            continue;
        }
        dscore_v = (double)g.dscore[v] / (double)g.v_weight[v];
        if (dscore_v > improvemnt)
        {
            improvemnt = dscore_v;
            add_v = v;
        }
        else if (dscore_v == improvemnt)
        {
            if (g.time_stamp[v] < g.time_stamp[add_v])
            {
                add_v = v;
            }
        }
    }
    return add_v;
}

int ChooseAddV(Graph &g, int remove_v, int update_v = -1)
{
    int i, v;
    int add_v = -1;
    double improvemnt = -g.max_v_weight;
    double dscore_v;
    bool config_checking_enabled = true;

    do {
      int tmp_degree = g.v_degree[remove_v];
      std::vector<int> *adjp = &g.v_adj[remove_v];
      for (i = 0; i < tmp_degree; i++)
      {
          v = (*adjp)[i];
          if (g.v_in_c[v] == 1)
          {
              continue;
          }
          if (g.conf_change[v] == 0 && config_checking_enabled)
          {
              continue;
          }
          dscore_v = (double)g.dscore[v] / (double)g.v_weight[v];
          if (dscore_v > improvemnt)
          {
              improvemnt = dscore_v;
              add_v = v;
          }
          else if (dscore_v == improvemnt)
          {
              if (g.time_stamp[v] < g.time_stamp[add_v])
              {
                  add_v = v;
              }
          }
      }
      v = remove_v;
      if (g.conf_change[v] == 1 && g.v_in_c[v] == 0)
      {
          dscore_v = (double)g.dscore[v] / (double)g.v_weight[v];
          if (dscore_v > improvemnt)
          {
              improvemnt = dscore_v;
              add_v = v;
          }
          else if (dscore_v == improvemnt)
          {
              if (g.time_stamp[v] < g.time_stamp[add_v])
              {
                  add_v = v;
              }
          }

      }

      if (update_v != -1)
      {
          tmp_degree = g.v_degree[update_v];
          adjp = &g.v_adj[update_v];
          for (i = 0; i < tmp_degree; i++)
          {
              v = (*adjp)[i];
              if (g.v_in_c[v] == 1)
              {
                  continue;
              }
              if (g.conf_change[v] == 0 && config_checking_enabled)
              {
                  continue;
              }
              dscore_v = (double)g.dscore[v] / (double)g.v_weight[v];
              if (dscore_v > improvemnt)
              {
                  improvemnt = dscore_v;
                  add_v = v;
              }
              else if (dscore_v == improvemnt)
              {
                  if (g.time_stamp[v] < g.time_stamp[add_v])
                  {
                      add_v = v;
                  }
              }
          }
          v = update_v;
          if (g.conf_change[v] == 1 && g.v_in_c[v] == 0)
          {
              dscore_v = (double)g.dscore[v] / (double)g.v_weight[v];
              if (dscore_v > improvemnt)
              {
                  improvemnt = dscore_v;
                  add_v = v;
              }
              else if (dscore_v == improvemnt)
              {
                  if (g.time_stamp[v] < g.time_stamp[add_v])
                  {
                      add_v = v;
                  }
              }
          }

      }
      if (add_v == -1 && config_checking_enabled == false) {
        for (int v2 = 0; v2 < g.v_num; v2++) {
          if (g.v_in_c[v2] == 0) {
            add_v = v2;
          }
        }
      }
      config_checking_enabled = false;
    } while(add_v == -1);
    assert(add_v != -1);
    assert(g.v_in_c[add_v] == 0);

    return add_v;
}

void UpdateBestSolution(Graph &g_best, Graph &g)
{
    int v;

    if (g.now_weight < g_best.now_weight)
    {
        for (v = 0; v < g.v_num; v++)
        {
            g_best.v_in_c[v] = g.v_in_c[v];
        }
        g_best.now_weight = g.now_weight;
        g_best.c_size = g.c_size;
    }
}

void RemoveRedundant(Graph &g)
{
    int v;
    for (unsigned int i = 0; i < g.remove_cand.size(); i++)
    {
        v = g.remove_cand[i];
        if (g.v_in_c[v] == 1 && g.dscore[v] == 0)
        {
            Remove(g, v);
            i--;
        }
    }
}

void ConstructVC(Graph &g)
{
    int e;
    int v1, v2;
    double v1dd, v2dd;

    g.uncov_stack.clear();
    g.c_size = 0;
    //best_weight = (int)(~0U >> 1);
    g.now_weight = 0;

    for (e = 0; e < g.e_num; e++)
    {
        v1 = g.edge[e].v1;
        v2 = g.edge[e].v2;

        if (g.v_in_c[v1] == 0 && g.v_in_c[v2] == 0)
        {
            v1dd = (double)g.v_degree[v1] / (double)g.v_weight[v1];
            v2dd = (double)g.v_degree[v2] / (double)g.v_weight[v2];
            if (v1dd > v2dd)
            {
                g.v_in_c[v1] = 1;
                g.now_weight += g.v_weight[v1];
            }
            else
            {
                g.v_in_c[v2] = 1;
                g.now_weight += g.v_weight[v2];
            }
            g.c_size++;
        }
    }

    std::vector<int> save_v_in_c = g.v_in_c;

    int save_c_size = g.c_size;
    long long save_weight = g.now_weight;

    int times = 50;
    vector<int> blocks(g.e_num / 1024 + 1);
    for (int i = 0; i < g.e_num / 1024 + 1; i++)
    {
        blocks[i] = i;
    }

    while (times-- > 0)
    {
        g.v_in_c.clear();
        g.v_in_c.resize(g.v_num, 0);

        g.c_size = 0;
        g.now_weight = 0;
        shuffle(blocks.begin(), blocks.end(), default_random_engine(g.seed));

        for (auto &block : blocks)
        {
            auto begin = block * 1024;
            auto end = block == g.e_num / 1024 ? g.e_num : begin + 1024;
            int tmpsize = end - begin + 1;
            vector<int> idx(tmpsize);
            for (int i = begin; i < end; i++)
            {
                idx[i - begin] = i;
            }
            while (tmpsize > 0)
            {
                int i = rand() % tmpsize;
                Edge e = g.edge[idx[i]];
                v1 = e.v1;
                v2 = e.v2;
                swap(idx[i], idx[--tmpsize]);
                if (g.v_in_c[v1] == 0 && g.v_in_c[v2] == 0)
                {
                    v1dd = (double)g.v_degree[v1] / (double)g.v_weight[v1];
                    v2dd = (double)g.v_degree[v2] / (double)g.v_weight[v2];
                    if (v1dd > v2dd)
                    {
                        g.v_in_c[v1] = 1;
                        g.now_weight += g.v_weight[v1];
                    }
                    else
                    {
                        g.v_in_c[v2] = 1;
                        g.now_weight += g.v_weight[v2];
                    }
                    g.c_size++;
                }
            }
        }
        if (g.now_weight < save_weight)
        {
            save_weight = g.now_weight;
            save_c_size = g.c_size;
            save_v_in_c = g.v_in_c;
        }
    }

    g.now_weight = save_weight;
    g.c_size = save_c_size;
    g.v_in_c = save_v_in_c;

    for (e = 0; e < g.e_num; e++)
    {
        v1 = g.edge[e].v1;
        v2 = g.edge[e].v2;

        if (g.v_in_c[v1] == 1 && g.v_in_c[v2] == 0)
        {
            g.dscore[v1] -= g.edge_weight[e];
        }
        else if (g.v_in_c[v2] == 1 && g.v_in_c[v1] == 0)
        {
            g.dscore[v2] -= g.edge_weight[e];
        }
    }

    ResetRemoveCand(g);
    for (int v = 0; v < g.v_num; v++)
    {
        if (g.v_in_c[v] == 1 && g.dscore[v] == 0)
        {
            Remove(g, v);
        }
    }
}

void RandVC(Graph &g)
{
    CheckUncovStack(g);
    g.uncov_stack.clear();
    g.c_size = 0;
    g.now_weight = 0;

    for (int e = 0; e < g.e_num; e++)
    {
        int v1 = g.edge[e].v1;
        int v2 = g.edge[e].v2;

        if (g.v_in_c[v1] == 0 && g.v_in_c[v2] == 0)
        {
            if (rand() % 2 == 0) {
                Add(g, v1);
            } else {
                Add(g, v2);
            }
        }
    }
    CheckUncovStack(g);
    ResetRemoveCand(g);
    CheckUncovStack(g);
    CheckSolution(g);
}

int CheckSolution(Graph &g)
{
    int e;

    for (e = 0; e < g.e_num; ++e)
    {
        if (g.v_in_c[g.edge[e].v1] != 1 && g.v_in_c[g.edge[e].v2] != 1)
        {
            cout << ", uncovered edge " << e;
            return 0;
        }
    }
    return 1;
}

void ForgetEdgeWeights(Graph &g)
{
    int v, e;
    int new_total_weitght = 0;

    for (v = 0; v < g.v_num; v++)
    {
        g.dscore[v] = 0;
    }

    for (e = 0; e < g.e_num; e++)
    {
        g.edge_weight[e] = g.edge_weight[e] * g.p_scale;
        new_total_weitght += g.edge_weight[e];

        if (g.v_in_c[g.edge[e].v1] + g.v_in_c[g.edge[e].v2] == 0)
        {
            g.dscore[g.edge[e].v1] += g.edge_weight[e];
            g.dscore[g.edge[e].v2] += g.edge_weight[e];
        }
        else if (g.v_in_c[g.edge[e].v1] + g.v_in_c[g.edge[e].v2] == 1)
        {
            if (g.v_in_c[g.edge[e].v1] == 1)
            {
                g.dscore[g.edge[e].v1] -= g.edge_weight[e];
            }
            else
            {
                g.dscore[g.edge[e].v2] -= g.edge_weight[e];
            }
        }
    }
    g.ave_weight = new_total_weitght / g.e_num;
}

void UpdateEdgeWeight(Graph &g)
{
    unsigned int i, e;

    for (i = 0; i < g.uncov_stack.size(); i++)
    {
        e = g.uncov_stack[i];
        g.edge_weight[e] += 1;
        g.dscore[g.edge[e].v1] += 1;
        g.dscore[g.edge[e].v2] += 1;
        if (g.mode % 2 == 1)
        {
            g.conf_change[g.edge[e].v1] = 1;
            g.conf_change[g.edge[e].v2] = 1;
        }
    }

    g.delta_total_weight += g.uncov_stack.size();

    if (g.mode / 2 == 1)
    {
        if (g.delta_total_weight >= g.e_num)
        {
            g.ave_weight += 1;
            g.delta_total_weight -= g.e_num;
        }

        if (g.ave_weight >= g.threshold)
        {
            ForgetEdgeWeights(g);
        }
    }
}

void LocalSearch(Graph &g_best, chrono::steady_clock::time_point deadline, int max_steps)
{
    int add_v, remove_v, update_v = 0;
    int step = 1;
    int try_step = 100;

    Graph g = g_best;

    while (true)
    {
        UpdateBestSolution(g_best, g);

        if (g.remove_cand.size() == 0) {
          while (g.uncov_stack.size() > 0)
          {
            add_v = ChooseAddFromV(g);
            Add(g, add_v);
            UpdateEdgeWeight(g);
            g.tabu_list[add_v] = 1;
            g.time_stamp[add_v] = step;
          }
          continue;
        }

        update_v = UpdateTargetSize(g);

        if (max_steps >= 0 && step > max_steps)
        {
          return;
        }

        if (step % try_step == 0)
        {
            if (chrono::steady_clock::now() >= deadline)
            {
                return;
            }
        }

        if (g.remove_cand.size() == 0) {
          while (g.uncov_stack.size() > 0)
          {
            add_v = ChooseAddFromV(g);
            Add(g, add_v);
            UpdateEdgeWeight(g);
            g.tabu_list[add_v] = 1;
            g.time_stamp[add_v] = step;
          }
          continue;
        }

        remove_v = ChooseRemoveV(g);
        Remove(g, remove_v);
        g.time_stamp[remove_v] = step;

        g.tabu_list.clear();
        g.tabu_list.resize(g.v_num, 0);

        while (g.uncov_stack.size() > 0)
        {
            add_v = ChooseAddV(g, remove_v, update_v);
            Add(g, add_v);
            UpdateEdgeWeight(g);
            g.tabu_list[add_v] = 1;
            g.time_stamp[add_v] = step;
        }
        RemoveRedundant(g);
        step++;
        update_v = 0;
    }
}

void CheckUncovStack(Graph &g)
{
  for (int v = 0; v < g.v_num; v++) {
    if (g.v_in_c[v]) {
      for (size_t i = 0; i < g.v_edges[v].size(); i++) {
        int e = g.v_edges[v][i];
        assert(g.index_in_uncov_stack[e] == -1);
      }
      assert(g.index_in_remove_cand[v] != -1);
      assert(g.remove_cand[g.index_in_remove_cand[v]] == v);
    } else {
      assert(g.index_in_remove_cand[v] == -1);
    }
  }
  for (size_t j = 0; j < g.uncov_stack.size(); j++) {
    int e = g.uncov_stack[j];
    assert(g.index_in_uncov_stack[e] == j);
  }
  for (int e = 0; e < g.e_num; e++) {
    int index = g.index_in_uncov_stack[e];
    if (index != -1) {
      assert(g.uncov_stack[index] == e);
    }
  }
  for (int i = 0; i < g.remove_cand.size(); i++) {
    int v = g.remove_cand[i];
    assert(g.index_in_remove_cand[v] == i);
  }
}

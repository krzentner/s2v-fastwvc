#include "mvc_env.h"
#include "graph.h"
#include <cassert>
#include <random>
#include <config.h>

#include <fast_wvc.h>

MvcEnv::MvcEnv(double _norm) : IEnv(_norm),
  update_v(0), remove_v(0), current_step(0), max_steps(cfg::max_steps)
{

}

void MvcEnv::s0(Graph _g)
{
    graph = _g;
    if (cfg::use_randvc) {
        RandVC(graph);
    } else {
        ConstructVC(graph);
    }
    current_step = 0;
    assert(graph.uncov_stack.size() == 0);
    recordCheckpoint();
    removeVertices();
    assert(graph.uncov_stack.size() != 0);
}

void MvcEnv::recordCheckpoint()
{
  checkpoint = Graph(graph);
}

Graph& MvcEnv::getCheckpoint()
{
  return checkpoint;
}

double MvcEnv::removeVertices()
{
    double reward = 0.0;
    while (graph.uncov_stack.size() == 0) {
      update_v = UpdateTargetSize(graph);

      remove_v = ChooseRemoveV(graph);
      Remove(graph, remove_v);
      graph.time_stamp[remove_v] = current_step;

      graph.tabu_list.clear();
      graph.tabu_list.resize(graph.v_num, 0);
      reward += graph.v_weight[update_v] + graph.v_weight[remove_v];
    }
    return reward;
}

double MvcEnv::step(int a)
{
    state_seq.push_back(graph);
    act_seq.push_back(a);

    double reward = stepInner(a);

    reward_seq.push_back(reward);
    sum_rewards.push_back(reward);
    return reward;
}

double MvcEnv::stepInner(int a)
{
    // Every few hours, we get an action to add a vertex that's already in the
    // cover. Until I can debug why, just give those actions the highest
    // possible negative reward.
    // TODO(krzentner): Figure out why this is necessary.
    if (graph.v_in_c[a]) {
      return -graph.max_v_weight;
    }

    assert(a < graph.v_num);
    assert(graph.uncov_stack.size() != 0);
    assert(!graph.v_in_c[a]);

    double reward = 0.0;
    Add(graph, a);
    UpdateEdgeWeight(graph);
    graph.tabu_list[a] = 1;
    graph.time_stamp[a] = current_step;
    reward -= graph.v_weight[a];

    if (graph.uncov_stack.size() == 0) {
      ++current_step;
      recordCheckpoint();
      if (isTerminal()) {
        removeVertices();
      } else {
        reward += removeVertices();
      }
    }
    return reward;
}

int MvcEnv::randomAction()
{
    std::vector<int> avail_list;
    assert(graph.uncov_stack.size() != 0);

    for (int i = 0; i < graph.num_nodes; ++i)
        if (graph.v_in_c[i] == 0)
        {
            bool useful = false;
            for (auto& neigh : graph.adj_list[i])
                if (graph.v_in_c[neigh] == 0)
                {
                    useful = true;
                    break;
                }
            if (useful)
                avail_list.push_back(i);
        }
    
    assert(avail_list.size());
    int idx = rand() % avail_list.size();
    assert(graph.uncov_stack.size() != 0);
    return avail_list[idx];
}

bool MvcEnv::isTerminal()
{
    // Unless we modify mvc_lib.cpp, we need this method to eventually return
    // true.
    return current_step > max_steps;
}

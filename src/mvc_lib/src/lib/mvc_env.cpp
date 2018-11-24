#include "mvc_env.h"
#include "graph.h"
#include <cassert>
#include <random>

#include <fast_wvc.h>

MvcEnv::MvcEnv(double _norm) : IEnv(_norm),
  next_action_type(ActionTypeUpdateTarget), update_v(0), current_step(0),
  max_steps(2)
{

}

void MvcEnv::s0(std::shared_ptr<Graph> _g)
{
    graph = _g;
}

double MvcEnv::step(int a)
{
    state_seq.push_back(graph->remove_cand);
    act_seq.push_back(a);

    double reward = stepInner(a);

    reward_seq.push_back(reward);
    sum_rewards.push_back(reward);
    return reward;
}

double MvcEnv::stepInner(int a)
{
    // TODO: Fix this by recording the best state in mvc_lib.cpp
    // We never record the best solution so far. It's not completely clear how
    // a "best solution so far" fits into the s2v framework.
    // In greedy algorithms, there's just a current state, which is always an
    // "improvemnt" from the previous state.
    // But as implemented here, our graph can actually get worse over time.
    // The way we relate our ultimate goal (a low total weight vertex cover) to
    // each steps reward requires more thought.
    assert(graph);
    double invalid_action_reward = -1.0;
    double repeat_action_reward = -1.0;
    ++current_step;
    // We often know which actions are valid ahead of time, but we never tell
    // the qnet. We should add masking or something.
    if (next_action_type == ActionTypeUpdateTarget) {
      // a is best_remove_v from UpdateTargetSize.
      update_v = a;
      if (graph->v_in_c[update_v] == 1)
      {
        Remove(*graph, a);
        next_action_type = ActionTypeRemoveV;
        return 0.0;
      }
      else
      {
        return invalid_action_reward;
      }
    } else if (next_action_type == ActionTypeRemoveV) {
      // FastWVC usually tries out 50 random v's here and takes the best.
      // In principle, we're looking at *every* v, but this feels like it's not
      // quite right.
      int remove_v = a;
      if (graph->v_in_c[update_v] == 1 && graph->tabu_list[remove_v] != 1)
      {
        Remove(*graph, remove_v);
        graph->time_stamp[remove_v] = current_step;

        graph->tabu_list.clear();
        graph->tabu_list.resize(graph->v_num, 0);

        // TODO(krzentner): Investigate using ActionTypeAddV
        //next_action_type = ActionTypeAddV;

        next_action_type = ActionTypeUpdateTarget;

        while (graph->uncov_stack.size() > 0)
        {
            int add_v = ChooseAddV(*graph, remove_v, update_v);
            Add(*graph, add_v);
            UpdateEdgeWeight(*graph);
            graph->tabu_list[add_v] = 1;
            graph->time_stamp[add_v] = current_step;
        }
        RemoveRedundant(*graph);
        update_v = 0;

        return getReward();
      }
      else
      {
        return repeat_action_reward;
      }
    } else if (next_action_type == ActionTypeAddV) {
      // TODO(krzentner): Make this code path actually active.
      assert(false);
      int add_v = a;
      if (graph->v_in_c[add_v] == 1) {
        return invalid_action_reward;
      }
      Add(*graph, add_v);
      UpdateEdgeWeight(*graph);
      graph->tabu_list[add_v] = 1;
      graph->time_stamp[add_v] = current_step;
      if (graph->uncov_stack.size() == 0) {
        next_action_type = ActionTypeUpdateTarget;
        RemoveRedundant(*graph);
        update_v = 0;
        return getReward();
      }
      // I don't think we should return 0 here, since that enourages the agent
      // to uncover many vertices.
      // However, -1.0 doesn't seem quite right either, since uncovering nodes
      // with low weight isn't that bad.
      return -1.0;
    }
    return -1.0;
}

int MvcEnv::randomAction()
{
    assert(graph);
    return rand() % graph->v_num;
}

bool MvcEnv::isTerminal()
{
    assert(graph);
    // Unless we modify mvc_lib.cpp, we need this method to eventually return
    // true.
    return current_step > max_steps;
}

double MvcEnv::getReward()
{
    return -1.0 / graph->now_weight;
}

std::vector<int>* MvcEnv::getState()
{
  assert(graph);
  return &graph->remove_cand;
}

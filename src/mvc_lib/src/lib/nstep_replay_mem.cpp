#include "nstep_replay_mem.h"
#include "i_env.h"
#include "config.h"
#include <cassert>

#define max(x, y) (x > y ? x : y)

std::vector< Graph > NStepReplayMem::graphs;
std::vector<int> NStepReplayMem::actions;
std::vector<double> NStepReplayMem::rewards;
std::vector<bool> NStepReplayMem::terminals;
int NStepReplayMem::current;
int NStepReplayMem::count;
int NStepReplayMem::memory_size;
std::default_random_engine NStepReplayMem::generator;
std::uniform_int_distribution<int>* NStepReplayMem::distribution;

void NStepReplayMem::Init(int _memory_size)
{
    memory_size = _memory_size;
    graphs.resize(memory_size);
    actions.resize(memory_size);
    rewards.resize(memory_size);
    terminals.resize(memory_size);

    current = 0;
    count = 0;
    distribution = new std::uniform_int_distribution<int>(0, memory_size - 1);
}

void NStepReplayMem::Add(Graph g, 
                        int a_t, 
                        double r_t,
                        bool terminal)
{
    graphs[current] = g;
    actions[current] = a_t;
    rewards[current] = r_t;
    terminals[current] = terminal;

    count = max(count, current + 1);
    current = (current + 1) % memory_size; 
}

void NStepReplayMem::Add(IEnv* env)
{
    assert(env->isTerminal());
    int num_steps = env->act_seq.size();
    assert(num_steps);

    env->sum_rewards[num_steps - 1] = env->reward_seq[num_steps - 1];
    for (int i = num_steps - 1; i >= 0; --i)
        if (i < num_steps - 1)
            env->sum_rewards[i] = env->sum_rewards[i + 1] + env->reward_seq[i];

    for (int i = 0; i < num_steps; ++i)
    {
        bool term_t = false;
        double cur_r;
        if (i + cfg::n_step >= num_steps)
        {
            cur_r = env->sum_rewards[i];
            term_t = true;
        } else {
            cur_r = env->sum_rewards[i] - env->sum_rewards[i + cfg::n_step];
        }
        Add(env->state_seq[i], env->act_seq[i], cur_r, term_t);
    }
}

void NStepReplayMem::Sampling(int batch_size, ReplaySample& result)
{
    assert(count >= batch_size);

    result.g_list.resize(batch_size);
    result.list_at.resize(batch_size);
    result.list_rt.resize(batch_size);
    result.list_term.resize(batch_size);
    auto& dist = *distribution;
    for (int i = 0; i < batch_size; ++i)
    {
        int idx = dist(generator) % count;
        result.g_list[i] = std::make_shared<Graph>(graphs[idx]);
        result.list_at[i] = actions[idx];
        result.list_rt[i] = rewards[idx];
        result.list_term[i] = terminals[idx];
    }
}

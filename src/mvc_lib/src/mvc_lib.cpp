#include "config.h"
#include "mvc_lib.h"
#include "graph.h"
#include "nn_api.h"
#include "qnet.h"
#include "nstep_replay_mem.h"
#include "simulator.h"
#include "mvc_env.h"
#include "fast_wvc.h"
#include <random>
#include <algorithm>
#include <cstdlib>
#include <signal.h>

using namespace gnn;

void intHandler(int dummy) {
    exit(0);
}

int LoadModel(const char* filename)
{
    ASSERT(net, "please init the lib before use");    
    net->model.Load(filename);
    return 0;
}

int SaveModel(const char* filename)
{
    ASSERT(net, "please init the lib before use");
    net->model.Save(filename);
    return 0;
}

std::vector< std::vector<double>* > list_pred;
MvcEnv* test_env;
int Init(const int argc, const char** argv)
{
    signal(SIGINT, intHandler);
    
    cfg::LoadParams(argc, argv);
    GpuHandle::Init(cfg::dev_id, 1);

    net = new QNet();    
    net->BuildNet();

    NStepReplayMem::Init(cfg::mem_size);
    
    Simulator::Init(cfg::num_env);
    for (int i = 0; i < cfg::num_env; ++i)
        Simulator::env_list[i] = new MvcEnv(cfg::max_n);
    test_env = new MvcEnv(cfg::max_n);

    list_pred.resize(cfg::batch_size);
    for (int i = 0; i < cfg::batch_size; ++i)
        list_pred[i] = new std::vector<double>(cfg::max_n + 10);
    return 0;
}

int UpdateSnapshot()
{
    net->old_model.DeepCopyFrom(net->model);
    return 0;
}

int InsertGraph(bool isTest, const int g_id, const int num_nodes, const int num_edges, const int* edges_from, const int* edges_to, const int* node_weights)
{
    auto g = std::make_shared<Graph>(num_nodes, num_edges, edges_from, edges_to, node_weights);
    if (isTest)
        GSetTest.InsertGraph(g_id, g);
    else
        GSetTrain.InsertGraph(g_id, g);
    return 0;
}

int ClearTrainGraphs()
{
    GSetTrain.graph_pool.clear();
    return 0;
}

int PlayGame(const int n_traj, const double eps)
{
    Simulator::run_simulator(n_traj, eps);
    return 0;
}

ReplaySample sample;
std::vector<double> list_target;
double Fit()
{
    NStepReplayMem::Sampling(cfg::batch_size, sample);
    bool ness = false;
    for (int i = 0; i < cfg::batch_size; ++i)
        if (!sample.list_term[i])
        {
            ness = true;
            break;
        }
    if (ness)
        PredictWithSnapshot(sample.g_list, list_pred);
    
    list_target.resize(cfg::batch_size);

    for (int i = 0; i < cfg::batch_size; ++i)
    {
        double q_rhs = 0;
        if (!sample.list_term[i])
            q_rhs = max(list_pred[i]->size(), list_pred[i]->data());
        q_rhs += sample.list_rt[i];
        list_target[i] = q_rhs;
    }

    return Fit(sample.g_list, sample.list_at, list_target);
}

double GetSolInner(Graph g, int* sol)
{
    test_env->s0(g);
    Graph g_best = test_env->getCheckpoint();

    int new_action;
    while (!test_env->isTerminal())
    {
        std::vector< std::shared_ptr< Graph > > g_list;
        g_list.push_back(std::make_shared<Graph>(test_env->graph));
        Predict(g_list, list_pred);
        new_action = arg_max(test_env->graph.num_nodes, list_pred[0]->data());
        test_env->step(new_action);

        UpdateBestSolution(g_best, test_env->getCheckpoint());
    }
    if (sol) {
      memcpy(sol, g_best.remove_cand.data(), g_best.remove_cand.size() * sizeof(int));
    }
    return g_best.now_weight;    
}

double Test(const int gid)
{
  return GetSolInner(*GSetTest.Get(gid), nullptr);
}

double GetSol(const int gid, int* sol)
{
  return GetSolInner(*GSetTrain.Get(gid), sol);
}

double FastWVC(bool isTest, const int gid, int* sol, int timeout_seconds, int max_steps,
               bool use_randvc) {
  Graph g;
  if (isTest) {
    g = *GSetTest.Get(gid);
  } else {
    g = *GSetTrain.Get(gid);
  }
  if (use_randvc) {
    RandVC(g);
  } else {
    ConstructVC(g);
  }
  std::chrono::steady_clock::time_point deadline =
    std::chrono::steady_clock::now() + std::chrono::seconds(timeout_seconds);
  LocalSearch(g, deadline, max_steps);
  if (sol) {
    memcpy(sol, g.remove_cand.data(), g.remove_cand.size() * sizeof(int));
  }
  return g.now_weight;    
}

#ifndef I_ENV_H
#define I_ENV_H

#include <vector>
#include <set>

#include "graph.h"

class IEnv
{
public:

    IEnv(double _norm) : norm(_norm), graph() {}

    virtual void s0(Graph _g) = 0;

    virtual double step(int a) = 0;

    virtual int randomAction() = 0;

    virtual bool isTerminal() = 0;

    virtual double getReward() = 0;

    double norm;
    Graph graph;
    
    std::vector<int> act_seq;
    std::vector<double> reward_seq, sum_rewards;
};

#endif

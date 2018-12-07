#ifndef MVC_ENV_H
#define MVC_ENV_H

#include "i_env.h"

enum FastWVCActionType {
  ActionTypeRemoveV,
  ActionTypeAddV,
};

class MvcEnv : public IEnv
{
public:

    MvcEnv(double _norm);

    virtual void s0(Graph _g) override;

    virtual double step(int a) override;

    virtual int randomAction() override;

    virtual bool isTerminal() override;

    Graph& getCheckpoint();
    void recordCheckpoint();

    double stepInner(int a);
    double removeVertices();
    FastWVCActionType next_action_type;
    int update_v;
    int remove_v;
    int current_step;
    int max_steps;
    Graph checkpoint;
};

#endif

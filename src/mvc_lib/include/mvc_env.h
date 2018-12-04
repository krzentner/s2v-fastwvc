#ifndef MVC_ENV_H
#define MVC_ENV_H

#include "i_env.h"

enum FastWVCActionType {
  ActionTypeUpdateTarget,
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

    virtual double getReward() override;

    double stepInner(int a);
    FastWVCActionType next_action_type;
    int update_v;
    int current_step;
    int max_steps;
};

#endif

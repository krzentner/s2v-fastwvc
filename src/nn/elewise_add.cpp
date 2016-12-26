#include "nn/elewise_add.h"

namespace gnn
{
template<typename mode, typename Dtype>
ElewiseAdd<mode, Dtype>::ElewiseAdd(std::string _name, PropErr _properr) 
		: Factor(_name, _properr)
{

}

template<typename mode, typename Dtype>
void ElewiseAdd<mode, Dtype>::Forward(std::vector< std::shared_ptr<Variable> >& operands, 
						 			std::vector< std::shared_ptr<Variable> >& outputs)
{
	ASSERT(operands.size() >= 2, "unexpected input size for " << StrType());
	ASSERT(outputs.size() == 1, "unexpected output size for " << StrType()); 

	auto& output = dynamic_cast<DTensorVar<mode, Dtype>*>(outputs[0].get())->value;

	auto& op1 = dynamic_cast<DTensorVar<mode, Dtype>*>(operands[0].get())->value;
	output.CopyFrom(op1);
	for (size_t i = 1; i < operands.size(); ++i)
	{
		auto& op_i = dynamic_cast<DTensorVar<mode, Dtype>*>(operands[i].get())->value;
		ASSERT(op_i.shape == output.shape, "no broadcasting is supported right now");
		output.Axpy(1.0, op_i);
	}
}

template<typename mode, typename Dtype>
void ElewiseAdd<mode, Dtype>::Backward(std::vector< std::shared_ptr<Variable> >& operands, 
									std::vector< bool >& isConst, 
						 			std::vector< std::shared_ptr<Variable> >& outputs)
{
	ASSERT(operands.size() >= 2, "unexpected input size for " << StrType());
	ASSERT(outputs.size() == 1, "unexpected output size for " << StrType()); 

	auto& cur_grad = dynamic_cast<DTensorVar<mode, Dtype>*>(outputs[0].get())->grad;
	for (size_t i = 0; i < operands.size(); ++i)
	{
		if (isConst[i])
			continue;
		auto& grad_i = dynamic_cast<DTensorVar<mode, Dtype>*>(operands[i].get())->grad;
		ASSERT(grad_i.shape == cur_grad.shape, "no broadcasting is supported right now");
		grad_i.Axpy(1.0, cur_grad);
	}
}

template class ElewiseAdd<CPU, float>;
template class ElewiseAdd<CPU, double>;
template class ElewiseAdd<GPU, float>;
template class ElewiseAdd<GPU, double>;
}
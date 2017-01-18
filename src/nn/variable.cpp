#include "nn/variable.h"
#include "util/graph_struct.h"

namespace gnn
{

Variable::Variable(std::string _name) : name(_name)
{

}

//================ graph variable =====================

GraphVar::GraphVar(std::string _name) : Variable(_name), graph(nullptr)
{

}

EleType GraphVar::GetEleType()
{
	return EleType::UNKNOWN;
}

void GraphVar::SetRef(void* p)
{
	graph = static_cast< GraphStruct* >(p);
}

template<typename mode, typename Dtype>
TensorVarTemplate<mode, DENSE, Dtype>::TensorVarTemplate(std::string _name)
			 : TensorVar<mode, Dtype>(_name)
{
}

template<typename mode, typename Dtype>
TensorVarTemplate<mode, DENSE, Dtype>::TensorVarTemplate(std::string _name, std::vector<size_t> l)
				 : TensorVar<mode, Dtype>(_name)
{
	value.Reshape(l);
}

template<typename mode, typename Dtype>
void TensorVarTemplate<mode, DENSE, Dtype>::SetRef(void* p)
{
	auto pt = static_cast< DTensor<mode, Dtype>* >(p);
	this->value.ShallowCopy(*pt);
}

template<typename mode, typename Dtype>
Dtype TensorVarTemplate<mode, DENSE, Dtype>::AsScalar()
{
	return value.AsScalar();
}

template<typename mode, typename Dtype>
MatType TensorVarTemplate<mode, DENSE, Dtype>::GetMatType()
{
	return MatType::dense;
}

template<typename mode, typename Dtype>
void TensorVarTemplate<mode, DENSE, Dtype>::ZeroGrad()
{
	grad.Reshape(value.shape.dims);
	grad.Zeros();
}

template<typename mode, typename Dtype>
void TensorVarTemplate<mode, DENSE, Dtype>::OnesGrad()
{
	grad.Reshape(value.shape.dims);
	grad.Fill(1);
}

template<typename mode, typename Dtype>
void TensorVarTemplate<mode, DENSE, Dtype>::Serialize(FILE* fid)
{
	value.Serialize(fid);
}

template<typename mode, typename Dtype>
void TensorVarTemplate<mode, DENSE, Dtype>::Deserialize(FILE* fid)
{
	value.Deserialize(fid);
}

template class TensorVarTemplate<CPU, DENSE, float>;
template class TensorVarTemplate<CPU, DENSE, double>;
template class TensorVarTemplate<CPU, DENSE, int>;
template class TensorVarTemplate<GPU, DENSE, float>;
template class TensorVarTemplate<GPU, DENSE, double>;
template class TensorVarTemplate<GPU, DENSE, int>;

//============ SPARSE Tensor Variable ==================

template<typename mode, typename Dtype>
TensorVarTemplate<mode, SPARSE, Dtype>::TensorVarTemplate(std::string _name) 
			: TensorVar<mode, Dtype>(_name)
{

}

template<typename mode, typename Dtype>
void TensorVarTemplate<mode, SPARSE, Dtype>::SetRef(void* p)
{
	auto* pt = static_cast< SpTensor<mode, Dtype>* >(p);
	this->value.ShallowCopy(*pt);
}

template<typename mode, typename Dtype>
Dtype TensorVarTemplate<mode, SPARSE, Dtype>::AsScalar()
{
	return 0;
}

template<typename mode, typename Dtype>
MatType TensorVarTemplate<mode, SPARSE, Dtype>::GetMatType()
{
	return MatType::sparse;
}

template class TensorVarTemplate<CPU, SPARSE, float>;
template class TensorVarTemplate<CPU, SPARSE, double>;
template class TensorVarTemplate<CPU, SPARSE, int>;
template class TensorVarTemplate<GPU, SPARSE, float>;
template class TensorVarTemplate<GPU, SPARSE, double>;
template class TensorVarTemplate<GPU, SPARSE, int>;

}
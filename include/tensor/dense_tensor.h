#ifndef DENSE_TENSOR_H
#define DENSE_TENSOR_H

#include "tensor.h"
#include "t_data.h"

namespace gnn
{

/**
 * @brief      CPU DENSE specialization of tensor
 *
 * @tparam     Dtype   float/double
 */
template<typename Dtype>
class TensorTemplate<CPU, DENSE, Dtype> : public Tensor
{
public:

	TensorTemplate();

	virtual void Reshape(std::vector<size_t> l) override;
	virtual MatType GetMatType() override;
	virtual MatMode GetMatMode() override;

	/**
	 * @brief      deeply copy src to this tensor
	 *
	 * @param      src   the CPU dense tensor with same data type
	 */
	void CopyFrom(DTensor<CPU, Dtype>& src);		
	/**
	 * @brief      deeply copy src to this tensor
	 *
	 * @param      src   the GPU dense tensor with same data type
	 */
	void CopyFrom(DTensor<GPU, Dtype>& src);

	/**
	 * @brief      deeply copy src to this tensor
	 *
	 * @param      src        the CPU dense tensor with different data type
	 *
	 * @tparam     otherType  the src's data type
	 */
	template<typename otherType> 
	void CopyFrom(DTensor<CPU, otherType>& src)
	{
		Reshape(src.shape.dims);
		for (size_t i = 0; i < src.shape.Count(); ++i)
			data->ptr[i] = src.data->ptr[i];
	}

	/**
	 * @brief      shallow copy (only set the shared_ptr)
	 *
	 * @param      src   The source
	 */
	void ShallowCopy(DTensor<CPU, Dtype>& src);

	/**
	 * @brief      set this tensor to be zero
	 */
	void Zeros(); 

	/**
	 * @brief      if this tensor is actually a scalar, return it; otherwise raise error
	 *
	 * @return     the only element in this tensor
	 */
	Dtype AsScalar(); 

	/**
	 * @brief      Sets this tensor from random normal
	 *
	 * @param[in]  mean  The mean
	 * @param[in]  std   The standard deviation
	 */
	void SetRandN(Dtype mean, Dtype std);

	/**
	 * @brief      fill this tensor with same scalar
	 *
	 * @param[in]  scalar  The scalar to be set
	 */
	void Fill(Dtype scalar);

	/**
	 * @brief      the absolute sum of this tensor 
	 *
	 * @return     the absolute sum
	 */
	Dtype ASum();

	/**
	 * @brief      find the max index along dimensions other than axis
	 *
	 * @param      dst   used to store the results
	 * @param[in]  axis  The axis to be kept
	 */
	void ArgMax(DTensor<CPU, int>& dst, uint axis = 0);

	/**
	 * @brief      compute and store the result (a dense matrix) of matrix multiplication; 
	 *				this = alpha * A x B + beta * this
	 * @param      a       operand A (dense matrix)
	 * @param      b       operand B (dense matrix)
	 * @param[in]  transA  whether to transpose A
	 * @param[in]  transB  whether to transpose B
	 * @param[in]  alpha   The alpha
	 * @param[in]  beta    The beta
	 */
	void MM(DTensor<CPU, Dtype>& a, DTensor<CPU, Dtype>& b, Trans transA, Trans transB, Dtype alpha, Dtype beta);

	/**
	 * @brief      compute and store the result (a dense matrix) of matrix multiplication; 
	 *				this = alpha * A x B + beta * this
	 * @param      a       operand A (sparse matrix)
	 * @param      b       operand B (dense matrix)
	 * @param[in]  transA  whether to transpose A
	 * @param[in]  transB  whether to transpose B
	 * @param[in]  alpha   The alpha
	 * @param[in]  beta    The beta
	 */
	void MM(SpTensor<CPU, Dtype>& a, DTensor<CPU, Dtype>& b, Trans transA, Trans transB, Dtype alpha, Dtype beta);

	/**
	 * @brief      do the softmax inplace, keep rows
	 */
	void Softmax();
	
	/**
	 * @brief      store the result of mean reduction
	 *
	 * @param      a    the operand
	 * @param[in]  axis  The axis to be reduced; by default it is -1, which will do global reduce
	 */
	void Mean(DTensor<CPU, Dtype>& a, int axis = -1);

	/**
	 * the shared ptr to the data structure (which is used to keep the data of this tensor)
	 */
	std::shared_ptr< DenseData<CPU, Dtype> > data;
};

/**
 * @brief      CPU DENSE int tensor specialization; this tensor is not used for heavy computation
 * 				(e.g., matmul)
 */
template<>
class TensorTemplate<CPU, DENSE, int> : public Tensor
{
public:

	TensorTemplate();

	virtual void Reshape(std::vector<size_t> l) override;
	virtual MatType GetMatType() override;
	virtual MatMode GetMatMode() override;

	/**
	 * @brief      shallow copy (only set the shared_ptr)
	 *
	 * @param      src   The source
	 */
	void ShallowCopy(DTensor<CPU, int>& src);

	/**
	 * @brief      set this tensor to be zero
	 */
	void Zeros(); 

	/**
	 * @brief      if this tensor is actually a scalar, return it; otherwise raise error
	 *
	 * @return     the only element in this tensor
	 */
	int AsScalar(); 

	/**
	 * @brief      fill this tensor with same scalar
	 *
	 * @param[in]  scalar  The scalar to be set
	 */
	void Fill(int scalar);

	/**
	 * the shared ptr to the data structure (which is used to keep the data of this tensor)
	 */
	std::shared_ptr< DenseData<CPU, int> > data;
};

/**
 * @brief      GPU DENSE specialization of tensor
 *
 * @tparam     Dtype   float/double
 */
template<typename Dtype>
class TensorTemplate<GPU, DENSE, Dtype> : public Tensor
{
public:

	TensorTemplate();

	virtual void Reshape(std::vector<size_t> l) override;
	virtual MatType GetMatType() override;
	virtual MatMode GetMatMode() override;

	/**
	 * the shared ptr to the data structure (which is used to keep the data of this tensor)
	 */
	std::shared_ptr< DenseData<GPU, Dtype> > data;
};

/**
 * @brief      GPU DENSE int tensor specialization; this tensor is not used for heavy computation
 * 				(e.g., matmul)
 */
template<>
class TensorTemplate<GPU, DENSE, int> : public Tensor
{
public:

	TensorTemplate();

	virtual void Reshape(std::vector<size_t> l) override;
	virtual MatType GetMatType() override;
	virtual MatMode GetMatMode() override;
	
	/**
	 * the shared ptr to the data structure (which is used to keep the data of this tensor)
	 */	
	std::shared_ptr< DenseData<GPU, int> > data;
};

}
#endif
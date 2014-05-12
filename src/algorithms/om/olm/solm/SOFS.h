/*************************************************************************
> File Name: Sparse Diagonal AROW
> Copyright (C) 2013 Yue Wu<yuewu@outlook.com>
> Created Time: 2013/8/18 Sunday 17:25:54
> Functions: second order online feature selection
************************************************************************/

#ifndef HEADER_SOFS
#define HEADER_SOFS

#include "SparseOnlineLinearModel.h"
#include "../../../../utils/MaxHeap.h"

/**
*  namespace: Batch and Online Classification
*/
namespace BOC {
	template <typename FeatType, typename LabelType>
	class SOFS : public SparseOnlineLinearModel<FeatType, LabelType> {
		DECLARE_CLASS
	protected:
		float r;
		s_array<float> sigma_w;
		MaxHeap<float> heap;
		IndexType K; //keep top K elemetns

	public:
		SOFS(LossFunction<FeatType, LabelType> *lossFunc) :
			SparseOnlineLinearModel<FeatType, LabelType>(lossFunc) {
				this->id_str = "Second Order Online Feature Selection";
				this->r = 0;
				this->K = 0;
				this->sigma_w.resize(this->weightDim);

			}
		virtual ~SOFS(){
		}

		/**
		 * @Synopsis inherited functions
		 */
	public:
		/**
		 * PrintOptInfo print the info of optimization algorithm
		 */
		virtual void PrintOptInfo() const {
			printf("--------------------------------------------------\n");
			printf("Algorithm: %s\n\n", this->Id_Str().c_str());
			printf("\tk:\t%d\n", this->K);
			printf("\tr:\t%g\n", this->r);
		}

		/**
		 * @Synopsis SetParameter set parameters for the learning model
		 *
		 * @Param param
		 */
		virtual void SetParameter(BOC::Params &param) {
			OnlineLinearModel<FeatType, LabelType>::SetParameter(param);
			this->K = param.IntValue("-k");
			this->r = param.FloatValue("-r");
			if (this->K < 1) {
				cerr << "Please specify a valid number of weights to keep!\n";
				cerr << "current number: " << this->K << endl;
				exit(0);
			}
		}

		/**
		 * @Synopsis BeginTrain Reset the optimizer to the initialization status of training
		 */
		virtual void BeginTrain() {
			SparseOnlineLinearModel<FeatType, LabelType>::BeginTrain();

			if (this->K < 1){
				cerr << "Please specify a valid number of weights to keep!\n";
				cerr << "current number: " << this->K << endl;
				exit(0);
			}
			if (this->weightDim < this->K + 1){
				this->UpdateModelDimention(this->K); //remove the bais term
			}
			this->sigma_w.set_value(1);
			heap.Init(this->weightDim - 1, this->K, this->sigma_w.begin + 1);
		}

		/**
		 * @Synopsis Iterate Iteration of online learning
		 *
		 * @Param x current input data example
		 *
		 * @Returns  prediction of the current example
		 */
		virtual float Iterate(const DataPoint<FeatType, LabelType> &x) {
			float y = this->Predict(x);
			//y /= this->curIterNum;
			float alpha_t = 1 - x.label * y;
			if (alpha_t > 0){
				IndexType index_i = 0;
				size_t featDim = x.indexes.size();
				//calculate beta_t
				float beta_t = this->r;
				for (size_t i = 0; i < featDim; i++){
					beta_t += x.features[i] * x.features[i] * this->sigma_w[x.indexes[i]];
				}
				beta_t = 1.f / beta_t;
				alpha_t *= beta_t;
				for (size_t i = 0; i < featDim; i++){
					index_i = x.indexes[i];
					if (this->heap.is_topK(index_i - 1)){
						//update u_t
						this->weightVec[index_i] += alpha_t *
							this->sigma_w[index_i] * x.label * x.features[i];
					}
					else{
						this->weightVec[index_i] = 0;
					}
					//update sigma_w
					//this->sigma_w[index_i] -= beta_t * this->sigma_w[index_i] * this->sigma_w[index_i] * x.features[i] * x.features[i];
					this->sigma_w[index_i] *= this->r / (this->r +
						this->sigma_w[index_i] * x.features[i] * x.features[i]);
					IndexType ret_id;
					this->heap.UpdateHeap(index_i - 1, ret_id);
					//heap.Output(); 
				}
				//bias term
				this->weightVec[0] += alpha_t * this->sigma_w[0] * x.label;
				//this->sigma_w[0] -= beta_t * this->sigma_w[0] * this->sigma_w[0];
				this->sigma_w[0] *= this->r / (this->r + this->sigma_w[0]);
			}
			return y;
		}

		/**
		 * @Synopsis UpdateModelDimention update dimension of the model,
		 * often caused by the increased dimension of data
		 *
		 * @Param new_dim new dimension
		 */
		virtual void UpdateModelDimention(IndexType new_dim) {
			if (new_dim < this->weightDim)
				return;
			else {
				this->sigma_w.reserve(new_dim + 1);
				this->sigma_w.resize(new_dim + 1);  //reserve the 0-th
				//set the rest to 1
				this->sigma_w.set_value(this->sigma_w.begin + this->weightDim,
					this->sigma_w.end, 1);
				heap.UpdateDataNum(new_dim, this->sigma_w.begin + 1);

				SparseOnlineLinearModel<FeatType, LabelType>::UpdateModelDimention(new_dim);
			}
		}

	};

	IMPLEMENT_CLASS(SOFS)
}
#endif

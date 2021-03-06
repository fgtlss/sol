/*************************************************************************
	> File Name: NSOFS.h
	> Copyright (C) 2013 Yue Wu<yuewu@outlook.com>
	> Created Time: 3/10/2014 1:22:03 PM
    > Functions: normalized second order online feature selection
 ************************************************************************/

#ifndef HEADER_NOFS
#define HEADER_NOFS

#include "../utils/util.h"
#include "Optimizer.h"
#include "MaxHeap.h"
#include <algorithm>
#include <math.h>
#include <vector>

namespace SOL {
	template <typename FeatType, typename LabelType>
	class NSOFS: public Optimizer<FeatType, LabelType> {
	protected:
		float r;
		s_array<float> sigma_w;

		MaxHeap<float> heap;

		IndexType K; //keep top K elemetns

		float delta;
		float norm_coeff;

	public:
		NSOFS(DataSet<FeatType, LabelType> &dataset, 
			LossFunction<FeatType, LabelType> &lossFunc);
		virtual ~NSOFS();

	public:
		void SetParameterEx(int k, float delta, float r = -1);
		//select the best parameters for the model
		virtual void BestParameter();
		/**
		* PrintOptInfo print the info of optimization algorithm
		*/
		virtual void PrintOptInfo() const {
			printf("--------------------------------------------------\n");
			printf("Algorithm: %s\n\n",this->Id_Str().c_str());
			printf("k:\t%d\n", this->K);
			printf("r:\t%g\n\n", this->r);
		}
	protected:
		//this is the core of different updating algorithms
		virtual float UpdateWeightVec(const DataPoint<FeatType, LabelType> &x);
		//reset the optimizer to this initialization
		virtual void BeginTrain();
		//called when a train ends
		virtual void EndTrain();

		//Change the dimension of weights
		virtual void UpdateWeightSize(IndexType newDim);
	};

	template <typename FeatType, typename LabelType>
	NSOFS<FeatType, LabelType>::NSOFS(DataSet<FeatType, LabelType> &dataset, 
		LossFunction<FeatType, LabelType> &lossFunc):
	Optimizer<FeatType, LabelType>(dataset, lossFunc){
		this->id_str = "Second Order Online Feature Selection";
		this->r = init_r;
		this->K = 0;
		this->delta = init_ofs_delta;
		this->sigma_w.resize(this->weightDim);
	}

	template <typename FeatType, typename LabelType>
	NSOFS<FeatType, LabelType>::~NSOFS() {
	}

	//this is the core of different updating algorithms
	//return the predict
	template <typename FeatType, typename LabelType>
	float NSOFS<FeatType,LabelType>::UpdateWeightVec(
		const DataPoint<FeatType, LabelType> &x) {
			float y = this->Predict(x);
			//y /= this->curIterNum;
			float alpha_t = 1 - x.label * y;
			if(alpha_t > 0){
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
				}
				for (size_t i = 0; i < featDim; i++){
					index_i = x.indexes[i];
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

				float w_norm = this->weightVec[0];
				for (IndexType j = 0; j < this->K; j++){
					index_i = this->heap.get_id(j);
					w_norm += this->weightVec[index_i] * this->weightVec[index_i];
				}
				//shrinkage
				float coeff = this->norm_coeff / sqrtf(w_norm);
				if (coeff < 1){
					this->weightVec[0] *= coeff;
					for (IndexType j = 0; j < this->K; j++){
						index_i = this->heap.get_id(j);
						this->weightVec[index_i] *= coeff;
					}
				}
			}
			return y;
	}

	//reset the optimizer to this initialization
	template <typename FeatType, typename LabelType>
	void NSOFS<FeatType, LabelType>::BeginTrain() {
		Optimizer<FeatType, LabelType>::BeginTrain();
		if (this->K < 1){
			cerr << "Please specify a valid number of weights to keep!\n";
			cerr << "current number: " << this->K << endl;
			exit(0);
		}
		if (this->weightDim < this->K + 1){
			this->UpdateWeightSize(this->K); //remove the bais term
		}
		this->sigma_w.set_value(1);
		heap.Init(this->weightDim - 1, this->K, this->sigma_w.begin + 1);
		//this->is_normalize = true; //AROW requires normalization
		this->norm_coeff = 1.f / sqrtf(this->delta);
	}

	//called when a train ends
	template <typename FeatType, typename LabelType>
	void NSOFS<FeatType, LabelType>::EndTrain() {
		Optimizer<FeatType, LabelType>::EndTrain();
	}

	template <typename FeatType, typename LabelType>
	void NSOFS<FeatType, LabelType>::SetParameterEx(int k, float Delta, float r) {
		if (k < 1){
			cerr << "Please specify a valid number of weights to keep!\n";
			cerr << "current number: " << this->K << endl;
			exit(0);
		}
		else
			this->K = k;
		this->delta = Delta > 0 ? Delta : this->delta;
		this->r = r > 0 ? r : this->r;
	}

	//Change the dimension of weights
	template <typename FeatType, typename LabelType>
	void NSOFS<FeatType, LabelType>::UpdateWeightSize(IndexType newDim) {
		if (newDim < this->weightDim)
			return;
		else {
			this->sigma_w.reserve(newDim + 1);
			this->sigma_w.resize(newDim + 1);  //reserve the 0-th
			//set the rest to 1
			this->sigma_w.set_value(this->sigma_w.begin + this->weightDim,
				this->sigma_w.end, 1);

			heap.UpdateDataNum(newDim, this->sigma_w.begin + 1);
			Optimizer<FeatType, LabelType>::UpdateWeightSize(newDim);
		}
	}

	//get the best model parameter
	template <typename FeatType, typename LabelType>
	void NSOFS<FeatType, LabelType>::BestParameter() {
		//first learn the best learning rate
		float prevLambda = this->lambda;
		this->lambda = 0;

		//Select the best eta0
		float min_errorRate = 1;
		float bestr = 1;

		for (float r_temp = init_r_min; r_temp <= init_r_max; r_temp *= init_r_step) {
			cout << "r = " << r_temp << "\n";
			this->r = r_temp;
			float errorRate(0);
			errorRate = this->Train();

			if (errorRate < min_errorRate) {
				bestr = r_temp;
				min_errorRate = errorRate;
			}
			cout << " mistake rate: " << errorRate * 100 << " %\n";
		}

		this->r = bestr;
		this->lambda = prevLambda;
		cout << "Best Parameter:\tr = " << this->r << "\n\n";
	}
}
#endif

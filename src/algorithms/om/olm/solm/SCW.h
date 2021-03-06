/*************************************************************************
> File Name: SCW.h
> Copyright (C) 2013 Yue Wu<yuewu@outlook.com>
> Created Time: 2013/11/27 17:10:06
> Functions: Exact Soft Confidence-Weighted Learning
************************************************************************/
#ifndef HEADER_SCW
#define HEADER_SCW

#include "Optimizer.h"
#include <cmath>

namespace SOL {
	template <typename FeatType, typename LabelType>
	class SCW: public Optimizer<FeatType, LabelType> {
	protected:
		s_array<float> sigma_w;
		float C;

		float phi;
		float phi_sq;
		float phi_44;
		float psi;
		float zeta;

	public:
		SCW(DataSet<FeatType, LabelType> &dataset, 
			LossFunction<FeatType, LabelType> &lossFunc);
		~SCW();

	public:
		//set parameters for specific optimizers
		void SetParameterEx(float phi = -1,float r = -1);

	protected:
		//this is the core of different updating algorithms
		virtual float UpdateWeightVec(const DataPoint<FeatType, LabelType> &x);

		//Change the dimension of weights
		virtual void UpdateWeightSize(IndexType newDim);

		//reset
		virtual void BeginTrain();
		//called when a train ends
		virtual void EndTrain();

		//try and get the best parameter
		virtual void BestParameter(){}

	protected:
		//update phi: related probability of correct prediction
		void update_phi(float new_val){
			this->phi = new_val;
			this->phi_sq = this->phi * this->phi;
			this->phi_44 = this->phi_sq * this->phi_sq / 4.f;
			this->zeta = 1 + this->phi_sq;
			this->psi = 1 + this->phi_sq / 2.f;
		}
	};

	template <typename FeatType, typename LabelType>
	SCW<FeatType, LabelType>::SCW(DataSet<FeatType, LabelType> &dataset, 
		LossFunction<FeatType, LabelType> &lossFunc):
	Optimizer<FeatType, LabelType>(dataset, lossFunc){
		this->id_str = "Soft Confidence Weighted";
		this->sigma_w.resize(this->weightDim);

		this->C = 0.5f / init_r;
		this->update_phi(init_phi);
	}

	template <typename FeatType, typename LabelType>
	SCW<FeatType, LabelType>::~SCW() {
	}

	//this is the core of different updating algorithms
	template <typename FeatType, typename LabelType>
	float SCW<FeatType,LabelType>::UpdateWeightVec(
		const DataPoint<FeatType, LabelType> &x) {
			size_t featDim = x.indexes.size();
			IndexType index_i = 0;

			float vt = 0;
			for (size_t i = 0; i < featDim; i++){
				vt += x.features[i] * x.features[i] * this->sigma_w[x.indexes[i]];
			}
			//predict 
			float y = this->Predict(x);
			float mt = x.label * y;
			float alpha_t = (-mt * this->psi + sqrtf(mt * mt * this->phi_44 + vt * this->phi_sq * this->zeta));
			alpha_t  /= (vt * this->zeta);
			alpha_t = (std::max)(0.f, alpha_t);
			alpha_t = (std::min)(C, alpha_t);

			float ut = -alpha_t * vt * this->phi + sqrtf(alpha_t * alpha_t * vt * vt * this->phi_sq + 4.f * vt);
			ut = ut * ut / 4.f;
			float beta_t = alpha_t * this->phi / (sqrtf(ut) + vt * alpha_t * this->phi);
			if (alpha_t > 0){
				//update
				for (size_t i = 0; i < featDim; i++) {
					index_i = x.indexes[i];
					this->weightVec[index_i] += alpha_t * x.label * x.features[i] * this->sigma_w[index_i];
					//update sigma_w
					this->sigma_w[index_i] -= beta_t * this->sigma_w[index_i] * this->sigma_w[index_i] 
					* x.features[i] * x.features[i];
				}
				//bias term
				this->weightVec[0] +=  alpha_t * x.label * this->sigma_w[0];
				this->sigma_w[0] -= beta_t * this->sigma_w[0] * this->sigma_w[0]; 
			}
			return y;
	}

	//reset the optimizer to this initialization
	template <typename FeatType, typename LabelType>
	void SCW<FeatType, LabelType>::BeginTrain() {
		Optimizer<FeatType, LabelType>::BeginTrain();
		//reset time stamp
		this->sigma_w.set_value(1);
	}

	//called when a train ends
	template <typename FeatType, typename LabelType>
	void SCW<FeatType, LabelType>::EndTrain() {
		Optimizer<FeatType,LabelType>::EndTrain();
	}

	//set parameters for specific optimizers
	template <typename FeatType, typename LabelType>
	void SCW<FeatType, LabelType>::SetParameterEx(float phi,float r) {
		if (phi > 0)
			this->update_phi(phi);
		this->C = r > 0 ? 0.5f / r : this->C;
	}

	//Change the dimension of weights
	template <typename FeatType, typename LabelType>
	void SCW<FeatType, LabelType>::UpdateWeightSize(IndexType newDim) {
		if (newDim < this->weightDim)
			return;
		else {
			this->sigma_w.reserve(newDim + 1);
			this->sigma_w.resize(newDim + 1);
			//set the rest to one
			this->sigma_w.set_value(this->sigma_w.begin + this->weightDim, 
				this->sigma_w.end,1);

			Optimizer<FeatType,LabelType>::UpdateWeightSize(newDim);
		}
	}
}

#endif

/*************************************************************************
> File Name: Diagonal AROW
> Copyright (C) 2013 Yue Wu<yuewu@outlook.com>
> Created Time: 2013/8/18 星期日 17:25:54
> Functions: Diagonal Adaptive Regularization of Weight Vectors
> Reference: 
Crammer, Koby, Alex Kulesza, and Mark Dredze. "Adaptive regularization 
of weight vectors." Machine Learning (2009): 1-33.
 ************************************************************************/

#pragma once


#include "../common/util.h"
#include "Optimizer.h"
#include <cmath>
#include <limits>

namespace SOL {
    template <typename FeatType, typename LabelType>
        class SSAROW: public Optimizer<FeatType, LabelType> {
            protected:
                float r;
                s_array<float> sigma_w;
                s_array<size_t> timeStamp;

            public:
                SSAROW(DataSet<FeatType, LabelType> &dataset, 
                        LossFunction<FeatType, LabelType> &lossFunc);
                virtual ~SSAROW();

            public:
                void SetParameterEx(float lambda = -1,float r = -1);
            protected:
                //this is the core of different updating algorithms
                virtual float UpdateWeightVec(const DataPoint<FeatType, LabelType> &x);
                //reset the optimizer to this initialization
                virtual void BeginTrain();
                //called when a train ends
                virtual void EndTrain();

                //Change the dimension of weights
                virtual void UpdateWeightSize(IndexType newDim);

                //try and get the best parameter
                virtual void BestParameter(){}

        };

    template <typename FeatType, typename LabelType>
        SSAROW<FeatType, LabelType>::SSAROW(DataSet<FeatType, LabelType> &dataset, 
                LossFunction<FeatType, LabelType> &lossFunc):
            Optimizer<FeatType, LabelType>(dataset, lossFunc) {
                this->id_str = "SSAROW";
                this->r = init_r;
				this->sigma_w.resize(this->weightDim);
				this->timeStamp.resize(this->weightDim);
            }

    template <typename FeatType, typename LabelType>
        SSAROW<FeatType, LabelType>::~SSAROW() {
        }

    //this is the core of different updating algorithms
    //return the predict
    template <typename FeatType, typename LabelType>
        float SSAROW<FeatType,LabelType>::UpdateWeightVec(
                const DataPoint<FeatType, LabelType> &x) {
            size_t featDim = x.indexes.size();
            float y = this->Predict(x);
            //y /= this->curIterNum;
            float alpha_t = 1 - x.label * y;
            //calculate beta_t
            float beta_t = this->r;
            for (size_t i = 0; i < featDim; i++){
                beta_t += x.features[i] * x.features[i] * this->sigma_w[x.indexes[i]];
            }
            beta_t = 1.0 / beta_t;
            this->eta = beta_t / 2.f;
            float temp_beta = beta_t * this->lambda / 2.f;

            IndexType index_i = 0;
            //update w_t
            if(alpha_t > 0){

                alpha_t *= beta_t; 
                for (size_t i = 0; i < featDim; i++){
                    index_i = x.indexes[i];
                    //update u_t
                    this->weightVec[index_i] += alpha_t * 
                        this->sigma_w[index_i] * x.label * x.features[i];

                    //L1 lazy update
                    size_t stepK = this->curIterNum - this->timeStamp[index_i];
                    this->timeStamp[index_i] = this->curIterNum;

                    //a trick here: actually we should save all the beta_t for those not-occuring
                    //features
                    this->weightVec[index_i]= 
                        trunc_weight(this->weightVec[index_i],
                                stepK * temp_beta * this->sigma_w[index_i]);

                    //update sigma_w
                    this->sigma_w[index_i] -= beta_t * 
                        this->sigma_w[index_i] * this->sigma_w[index_i] * 
                        x.features[i] * x.features[i];
                }
                //bias term
                this->weightVec[0] += alpha_t * this->sigma_w[0] * x.label;
                this->sigma_w[0] -= beta_t * this->sigma_w[0] * this->sigma_w[0];
            }
            
            return y;
        }

    //reset the optimizer to this initialization
    template <typename FeatType, typename LabelType>
        void SSAROW<FeatType, LabelType>::BeginTrain() {
            Optimizer<FeatType, LabelType>::BeginTrain();

			this->timeStamp.zeros();
			this->sigma_w.set_value(1);
        }

    //called when a train ends
    template <typename FeatType, typename LabelType>
        void SSAROW<FeatType, LabelType>::EndTrain() {
            float beta_t = 1.f / this->r;
            float temp_beta = beta_t * this->lambda /2.f;

            for (IndexType index_i = 1; index_i < this->weightDim; index_i++) {
                //L1 lazy update
                size_t stepK = this->curIterNum - this->timeStamp[index_i];
                if (stepK == 0)
                    continue;
                this->timeStamp[index_i] = this->curIterNum;

                this->weightVec[index_i] = trunc_weight(this->weightVec[index_i],
                        stepK * temp_beta);
            }
            Optimizer<FeatType, LabelType>::EndTrain();
        }

    template <typename FeatType, typename LabelType>
        void SSAROW<FeatType, LabelType>::SetParameterEx(float lambda,float r) {
            this->lambda  = lambda >= 0 ? lambda : this->lambda;
            this->r = r > 0 ? r : this->r;
        }

    //Change the dimension of weights
    template <typename FeatType, typename LabelType>
        void SSAROW<FeatType, LabelType>::UpdateWeightSize(IndexType newDim) {
            if (newDim < this->weightDim)
                return;
            else {
				this->sigma_w.reserve(newDim + 1);
				this->sigma_w.resize(newDim + 1);
                //set the rest to one
				this->sigma_w.set_value(this->sigma_w.begin + this->weightDim, 
					this->sigma_w.end,1);

				this->timeStamp.reserve(newDim + 1);
				this->timeStamp.resize(newDim + 1);
				//set the rest to zero
				this->timeStamp.zeros(this->timeStamp.begin + this->weightDim,
					this->timeStamp.end);

				Optimizer<FeatType,LabelType>::UpdateWeightSize(newDim);
			}
		}
}

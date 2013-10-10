/*************************************************************************
> File Name: STG.h
> Copyright (C) 2013 Yue Wu<yuewu@outlook.com>
> Created Time: 2013/8/18 星期日 17:25:54
> Functions: Sparse Online Learning With Truncated Gradient
> Reference:
Langford J, Li L, Zhang T. Sparse online learning via truncated 
gradient[J]. The Journal of Machine Learning Research, 2009, 10: 
777-801. 
************************************************************************/

#pragma once


#include "../common/util.h"
#include "Optimizer.h"
#include <cmath>
#include <limits>

namespace SOL
{
	template <typename FeatType, typename LabelType>
	class STG: public Optimizer<FeatType, LabelType>
	{
        protected:
            int K;
            double theta; //truncate threshold

        protected:
            size_t*timeStamp;

        public:
            STG(DataSet<FeatType, LabelType> &dataset, 
                    LossFunction<FeatType, LabelType> &lossFunc);
            virtual ~STG();

        public:
            void SetParameterEx(double lambda = -1,int K = -1, 
                    double eta = -1,double theta = -1);
        protected:
            //this is the core of different updating algorithms
            virtual double UpdateWeightVec(const DataPoint<FeatType, LabelType> &x);
            //reset the optimizer to this initialization
            virtual void BeginTrain();
			 //called when a train ends
            virtual void EndTrain();

            //Change the dimension of weights
            virtual void UpdateWeightSize(int newDim);
    };

    template <typename FeatType, typename LabelType>
        STG<FeatType, LabelType>::STG(DataSet<FeatType, LabelType> &dataset, 
                LossFunction<FeatType, LabelType> &lossFunc):
            Optimizer<FeatType, LabelType>(dataset, lossFunc) , timeStamp(NULL)
    {
        this->id_str = "STG";
        this->K = init_k;
        this->theta = (std::numeric_limits<double>::max)();
        this->timeStamp = new size_t[this->weightDim];
		if (this->K != 1)
			this->sparse_soft_thresh = init_sparse_soft_thresh;
		else
			this->sparse_soft_thresh = 0;
    }

    template <typename FeatType, typename LabelType>
        STG<FeatType, LabelType>::~STG()
        {
            if(this->timeStamp != NULL)
                delete []this->timeStamp;
        }

    //this is the core of different updating algorithms
    //return the predict
    template <typename FeatType, typename LabelType>
        double STG<FeatType,LabelType>::UpdateWeightVec(
                const DataPoint<FeatType, LabelType> &x)
        {
            double y = this->Predict(x);
            int featDim = x.indexes.size();
            double gt_i = this->lossFunc->GetGradient(x,y);

            int index_i = 0;
            double w_abs = 0, alpha = 0;
            for (int i = 0; i < featDim; i++)
            {
                index_i = x.indexes[i];
                //update the weight
                this->weightVec[index_i] -= this->eta * gt_i * x.features[i];

                //lazy update
                //truncated gradient
                int stepK = ((this->curIterNum - this->timeStamp[index_i]) / this->K) 
                    * this->K;

                if (this->timeStamp[index_i] == 0)
                {
                    this->timeStamp[index_i] = this->curIterNum;
                    stepK = 0;
                    continue;
                }
                else
                    this->timeStamp[index_i] += stepK;
				if (stepK == 0)
					continue;

                w_abs = std::abs(this->weightVec[index_i]); 
                if (w_abs > this->theta)
                    continue;
                else 
                {
                    alpha =  stepK * this->eta * this->lambda;
                    if (w_abs > alpha)
                        this->weightVec[index_i] -= alpha * Sgn(this->weightVec[index_i]);
                    else
                        this->weightVec[index_i] = 0;
                }
            }

            //bias term
            this->weightVec[0] -= this->eta * gt_i;

            return y;
        }
    //reset the optimizer to this initialization
    template <typename FeatType, typename LabelType>
        void STG<FeatType, LabelType>::BeginTrain()
        {
            Optimizer<FeatType, LabelType>::BeginTrain();
            //reset time stamp
            memset(this->timeStamp,0,sizeof(size_t) * this->weightDim);
        }

		//called when a train ends
    template <typename FeatType, typename LabelType>
        void STG<FeatType, LabelType>::EndTrain()
        {
			double w_abs = 0, alpha = 0;
			for (int index_i = 1; index_i < this->weightDim; index_i++)
			{
				//truncated gradient
				int stepK = ((this->curIterNum - this->timeStamp[index_i]) / this->K) 
					* this->K;

				if (stepK == 0)
					continue;

				w_abs = std::abs(this->weightVec[index_i]); 
				if (w_abs > this->theta)
					continue;
				else 
				{
					alpha =  stepK * this->eta * this->lambda;
					if (w_abs > alpha)
						this->weightVec[index_i] -= alpha * Sgn(this->weightVec[index_i]);
					else
						this->weightVec[index_i] = 0;
				}
			}
            Optimizer<FeatType, LabelType>::EndTrain();
        }

    template <typename FeatType, typename LabelType>
        void STG<FeatType, LabelType>::SetParameterEx(double lambda , int k,
                double Eta,  double Theta) 
        {
            this->lambda  = lambda >= 0 ? lambda : this->lambda;
            this->K = k > 0 ? k : this->K;
            this->theta = Theta > 0 ? Theta : this->theta;
            this->eta = Eta > 0 ? Eta : this->eta;
			if (this->K == 1)
				this->sparse_soft_thresh = 0;
			else
				this->sparse_soft_thresh = init_sparse_soft_thresh;
        }

    //Change the dimension of weights
    template <typename FeatType, typename LabelType>
        void STG<FeatType, LabelType>::UpdateWeightSize(int newDim)
        {
            if (newDim < this->weightDim)
                return;
            else
            {
                newDim++; //reserve the 0-th

                size_t* newT = new size_t[newDim];
                //copy info
                memcpy(newT,this->timeStamp,sizeof(size_t) * this->weightDim); 
                //set the rest to zero
                memset(newT + this->weightDim,0,sizeof(size_t) * (newDim - this->weightDim)); 

                delete []this->timeStamp;
                this->timeStamp = newT;

                Optimizer<FeatType,LabelType>::UpdateWeightSize(newDim - 1);
            }
        }
}



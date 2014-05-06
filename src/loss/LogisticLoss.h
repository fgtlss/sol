/*************************************************************************
	> File Name: LogisticLoss.h
	> Copyright (C) 2013 Yue Wu<yuewu@outlook.com>
	> Created Time: 2013/8/18 Sunday 17:11:42
	> Functions: Logistic loss for binary classification
 ************************************************************************/

#ifndef HEADER_LOGISTIC_FUNCTIONS
#define HEADER_LOGISTIC_FUNCTIONS
#include "LossFunction.h"

namespace BOC {
	template <typename FeatType, typename LabelType>
	class LogisticLoss: public LossFunction<FeatType, LabelType> {
		public:
			virtual float GetLoss(LabelType label, float predict) {
				float tmp = -predict * label;
				if (tmp > 100.f) return tmp; 
				else if (tmp < -100.f) return 0.f;
				else
					return log(1.f + exp(tmp));
			}

            //aggressive learning 
			virtual float GetGradient(LabelType label, float predict) {
				float tmp = predict * label;
				if (tmp > 100.f) //to reject numeric problems
					return 0.f;
				else if (tmp  < -100.f)
					return (float)(-label); 
				else
					return -label / (1.f + exp(tmp)); 
			}
 	};
}

#endif

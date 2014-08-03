/*************************************************************************
> File Name: Optimizer.h
> Copyright (C) 2013 Yue Wu<yuewu@outlook.com>
> Created Time: 2013/8/18 Sunday 16:04:18
> Functions: Base class for different algorithms to do machine learning
************************************************************************/

#ifndef HEADER_OPTIMIZER
#define HEADER_OPTIMIZER

#include "../io/DataSet.h"
#include "../algorithms/LearnModel.h"

#include "../utils/reflector.h"


#include <fstream>

/**
*  namespace: Batch and Online Classification
*/
namespace BOC{

	template <typename FeatType, typename LabelType>
	class Optimizer : public Registry {

	protected:
		typedef DataPoint<FeatType, LabelType> PointType;

	protected:
		//learning model
		LearnModel<FeatType, LabelType> *learnModel;
		//input dataset
		DataSet<FeatType, LabelType> *dataSet;
		//number of iterations
		size_t update_times;
		/**
		 * @Synopsis Constructors
		 */
	public:
		Optimizer(LearnModel<FeatType, LabelType> *model, DataSet<FeatType, LabelType> *dataset) :
			learnModel(model), dataSet(dataset) {
			this->update_times = 0;
		}

		virtual ~Optimizer() {
		}

		/**
		 * @Synopsis Interfaces
		 */
	public:
		//train the data
		virtual float Train() = 0;

		/**
		 * @Synopsis Test test the performance on the given set
		 *
		 * @Param testSet
		 *
		 * @Returns
		 */
		float Test(DataSet<FeatType, LabelType> &testSet ) {
			testSet.Rewind();
			float errorRate(0);
			//double test_time = 0;
			//test
			float* predicts = new float[this->learnModel->GetClassfierNum()];
			while (1) {
				const DataChunk<PointType> &chunk = testSet.GetChunk();
				//double time1 = get_current_time();
				if (chunk.dataNum == 0) //"all the data has been processed!"
					break;
				for (size_t i = 0; i < chunk.dataNum; i++) {
					const PointType &data = chunk.data[i];
					//predict
					int predict = this->learnModel->Predict(data, predicts);
					if (predict != data.label)
						errorRate++;
				}
				//double time2 = get_current_time();
				//test_time += time2 - time1;
				testSet.FinishRead();
			}
			//printf("accumulated test time %lf ms\n",test_time);
			errorRate /= testSet.size();
			return errorRate;
		}

		/**
		 * @Synopsis Test test the performance on the given set
		 *
		 * @Param testSet
		 * @Param os output stream to save the predicted values
		 *
		 * @Returns
		 */
		float Test(DataSet<FeatType, LabelType> &testSet, std::ostream& os) {
			testSet.Rewind();
			float errorRate(0);
			//double test_time = 0;
			//test
			float* predicts = new float[this->learnModel->GetClassfierNum()];
			while (1) {
				const DataChunk<PointType> &chunk = testSet.GetChunk();
				//double time1 = get_current_time();
				if (chunk.dataNum == 0) //"all the data has been processed!"
					break;
				for (size_t i = 0; i < chunk.dataNum; i++) {
					const PointType &data = chunk.data[i];
					//predict
					int predict = this->learnModel->Predict(data, predicts);
					os << predict << "\t"<<(int)(data.label)<<"\n";
					if (predict != data.label){
						errorRate++;
					}
				}
				//double time2 = get_current_time();
				//test_time += time2 - time1;
				testSet.FinishRead();
			}
			//printf("accumulated test time %lf ms\n",test_time);
			errorRate /= testSet.size();
			return errorRate;
		}

		/**
		 * @Synopsis GetUpdateTimes get the number of iterations
		 *
		 * @Returns number of iterations
		 */
		inline size_t GetUpdateTimes() const { return this->update_times; }

	protected:
		/**
		 * @Synopsis Reset reset the optimizer
		 *
		 * @Returns  true if reset successfully
		 */
		bool Reset() {
			this->update_times = 0;
			this->dataSet->Rewind();
			return true;
		}
	};
}

#endif

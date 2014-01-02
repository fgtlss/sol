/*************************************************************************
	> File Name: main.cpp
	> Copyright (C) 2013 Yue Wu<yuewu@outlook.com>
	> Created Time: 2013/9/20 13:18:02
	> Functions: 
 ************************************************************************/
#define DLL_HEADER _declspec(dllexport)
#include "SOL_interface.h"
#include "Params.h"

#include "utils/util.h"

#include "io/DataSet.h"
#include "io/libsvm_io.h"

#include "algorithms/SGD.h"
#include "algorithms/STG.h"
#include "algorithms/RDA_L1.h"
#include "algorithms/FOBOS.h"
#include "algorithms/Ada_FOBOS.h"
#include "algorithms/Ada_RDA.h"
#include "algorithms/DAROW.h"
#include "algorithms/SSAROW.h"
#include "algorithms/ASAROW.h"
#include "algorithms/SCW.h"
#include "algorithms/CW_RDA.h"
#include "algorithms/SCW_RDA.h"
#include "algorithms/OFSGD.h"

#include "loss/LogisticLoss.h"
#include "loss/HingeLoss.h"
#include "loss/SquareLoss.h"
#include "loss/SquaredHingeLoss.h"

#include <string>
#include <iostream>
#include <fstream>
#include <cmath>


using namespace std;
using namespace SOL;

#define FeatType float
#define LabelType char

///////////////////////////function declarications/////////////////////
void FakeInput(int &argc, char **args, char** &argv);
template <typename T1, typename T2> LossFunction<T1,T2>* GetLossFunc(const Params &param);
template <typename T1, typename T2>
Optimizer<T1,T2>* GetOptimizer(const Params &param, DataSet<T1,T2> &dataset, LossFunction<T1,T2> &lossFun);

int main(int argc, const char** args) {
	//check memory leak in VC++
#if defined(_MSC_VER) && defined(_DEBUG)
	int tmpFlag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );
	tmpFlag |= _CRTDBG_LEAK_CHECK_DF;
	_CrtSetDbgFlag( tmpFlag );
	//_CrtSetBreakAlloc(2208);
#endif
	Params param;
	if (param.Parse(argc, args) == false){
		return -1;
	}
	LossFunction<FeatType, LabelType> *lossFunc = GetLossFunc<FeatType, LabelType>(param);
	if(lossFunc == NULL)
		return -1;

	DataSet<FeatType, LabelType> dataset(param.passNum,param.buf_size);
	if (dataset.Load(param.fileName, param.cache_fileName) == false){
		cerr<<"ERROR: Load dataset "<<param.fileName<<" failed!"<<endl;
		delete lossFunc;
		return -1;
	}

	Optimizer<FeatType, LabelType> *opti = GetOptimizer(param,dataset,*lossFunc);
	if (opti == NULL) {
		delete lossFunc;
		return -1;
	}

	opti->SetParameter(param.lambda,param.eta, param.power_t, param.initial_t);
	if (param.is_normalize == true)
		opti->SetNormalize(param.is_normalize);

	//learn the best parameters
	if (param.is_learn_best_param == true){
		opti->BestParameter();
	}
	else{
		opti->PrintOptInfo();

		float l_errRate(0), l_varErr(0);	//learning error rate
		float sparseRate(0);

		//learning the model
		double time1 = get_current_time();

		opti->Learn(l_errRate,l_varErr,sparseRate);

		double time2 = get_current_time();

		printf("data number: %lu\n",dataset.size());
		printf("Learn error rate: %.2f +/- %.2f %%\n",l_errRate * 100, l_varErr * 100);

		double time3 = 0;
		//test the model
		bool is_test = param.test_cache_fileName.length() > 0 || param.test_fileName.length() > 0;
		if ( is_test) {
			DataSet<FeatType, LabelType> testset(1,param.buf_size);
			if (testset.Load(param.test_fileName, param.test_cache_fileName) == true) {
				float t_errRate(0);	//test error rate
				t_errRate = opti->Test(testset);
				time3 = get_current_time();

				printf("Test error rate: %.2f %%\n",t_errRate * 100); 
			}
			else
				cout<<"load test set failed!"<<endl;
		}

		printf("Sparsification Rate: %.2f %%\n", sparseRate * 100);
		printf("Learning time: %.3f s\n", (float)(time2 - time1));
		if (is_test)
			printf("Test time: %.3f s\n", (float)(time3 - time2));
	}

	delete lossFunc;
	delete opti;

	return 0;
}

template <typename T1, typename T2>
LossFunction<T1,T2>* GetLossFunc(const Params &param) {
	if (param.str_loss == "Hinge")
		return new HingeLoss<T1,T2>();
	else if (param.str_loss == "Logit")
		return new LogisticLoss<T1,T2>();
	else if (param.str_loss == "Square")
		return new SquareLoss<T1,T2>();
	else if (param.str_loss == "SquareHinge")
		return new SquaredHingeLoss<T1, T2>();
	else{
		cerr<<"ERROR: unrecognized Loss function "<<param.str_loss<<endl;
		return NULL;
	}
}

template <typename T1, typename T2>
Optimizer<T1,T2>* GetOptimizer(const Params &param, DataSet<T1,T2> &dataset, LossFunction<T1,T2> &lossFunc) {
	if (param.str_opt == "SGD")
		return new SGD<T1,T2>(dataset,lossFunc);
	else if (param.str_opt == "STG") {
		STG<T1,T2> *opti = new STG<T1,T2>(dataset,lossFunc);
		opti->SetParameterEx(param.K);
		return opti;
	}
	else if (param.str_opt == "RDA") 
		return new RDA_L1<T1,T2>(dataset,lossFunc,false);
	else if (param.str_opt == "RDA_E") {
		RDA_L1<T1,T2> *opti = new RDA_L1<T1,T2>(dataset,lossFunc,true);
		opti->SetParameterEx(param.gamma_rou);
		return opti;
	}
	else if (param.str_opt == "FOBOS") 
		return new FOBOS<T1,T2>(dataset,lossFunc);
	else if (param.str_opt == "Ada-RDA") {
		Ada_RDA<T1,T2> *opti = new Ada_RDA<T1,T2>(dataset,lossFunc);
		opti->SetParameterEx(param.delta);
		return opti;
	}
	else if (param.str_opt == "Ada-FOBOS") {
		Ada_FOBOS<T1,T2> *opti = new Ada_FOBOS<T1,T2>(dataset,lossFunc);
		opti->SetParameterEx(param.delta);
		return opti;
	}
	else if (param.str_opt == "AROW") {
		DAROW<T1,T2> *opti = new DAROW<T1, T2>(dataset,lossFunc);
		opti->SetParameterEx(param.r);
		return opti;
	}
	else if (param.str_opt == "AROW-TG") {
		SSAROW<T1,T2> *opti = new SSAROW<T1, T2>(dataset,lossFunc);
		opti->SetParameterEx(param.r);
		return opti;
	}
	else if (param.str_opt == "AROW-FS") {
		ASAROW<T1,T2> *opti = new ASAROW<T1, T2>(dataset,lossFunc);
		opti->SetParameterEx(param.K, param.r);
		return opti;
	}
	else if (param.str_opt == "AROW-DA") {
		CW_RDA<T1,T2> *opti = new CW_RDA<T1, T2>(dataset,lossFunc);
		opti->SetParameterEx(param.r);
		return opti;
	}
    else if (param.str_opt == "SCW"){
        SCW<T1, T2> *opti = new SCW<T1, T2>(dataset,lossFunc);
        opti->SetParameterEx(param.phi,param.r);
        return opti;
    }
    else if (param.str_opt == "SCW-RDA") {
        SCW_RDA<T1,T2> *opti = new SCW_RDA<T1, T2>(dataset,lossFunc);
        opti->SetParameterEx(param.phi, param.r);
        return opti;
    }
    else if (param.str_opt == "OFSGD"){
        OFSGD<T1, T2> *opti = new OFSGD<T1, T2>(dataset, lossFunc);
        opti->SetParameterEx(param.K);
        return opti;
    }
    else{
        cerr<<"ERROR: unrecognized optimization method "<<param.str_opt<<endl;
        return NULL;
    }
}
/**
 *  sol_initialize : initialize an sol instance
 *
 * @Param:  argc: number of arguments
 * @Param:  args: char* array, command line
 *
 * @Returns:  pointer of the initialized object
 */

long sol_init_dataset(int argc, const char** args){
    Params param;
    if (param.Parse(argc, args) == false){
        return 0;
    }

    DataSet<FeatType, LabelType> *dataset = new DataSet<FeatType, LabelType>(param.passNum,param.buf_size);
    if (dataset->Load(param.fileName, param.cache_fileName) == false){
        cerr<<"ERROR: Load dataset "<<param.fileName<<" failed!"<<endl;
        delete dataset;
        return 0;
    }
    return (long)dataset;
}

long sol_init_loss(int argc, const char** args){
    Params param;
    if (param.Parse(argc, args) == false){
        return 0;
    }
    LossFunction<FeatType, LabelType> *lossFunc = GetLossFunc<FeatType, LabelType>(param);
    return (long)lossFunc;
}

long sol_init_optimizer(long dataset, long loss_func, int argc, const char** args){
    Params param;
    if (param.Parse(argc, args) == false){
        return 0;
    }
    Optimizer<FeatType, LabelType> *opti = GetOptimizer(param,
            *((DataSet<FeatType, LabelType>*)dataset),
            *((LossFunction<FeatType, LabelType>*)loss_func));
    if (opti == NULL) {
        return 0;
    }

    opti->SetParameter(param.lambda,param.eta, param.power_t, param.initial_t);
    if (param.is_normalize == true)
        opti->SetNormalize(param.is_normalize);
    if (param.is_learn_best_param == true){
		opti->BestParameter();
	}
    return (long)opti;
}

/**
 *  sol_train: train a model
 *
 * @Param: optimizer: initialized sol instance
 * @Param:  t_errRate: average test error rate
 * @Param:  var_t_errRate: variance of the test error rate
 * @Param:  sparse_rate: sparse rate of the model
 * @Param:  time_cost: time cost of the training
 *
 * @Returns:    
 */
void sol_train(long optimizer, float* learn_errRate, float* var_l_errRate, float* sparse_rate, float* time_cost){
    Optimizer<FeatType, LabelType> *opti = (Optimizer<FeatType, LabelType> *)optimizer;
    if (opti == NULL)
        return;

    opti->PrintOptInfo();

    float l_errRate(0), l_varErr(0);	//learning error rate
    float sparseRate(0);

    //learning the model
    double time1 = get_current_time();

    opti->Learn(l_errRate,l_varErr,sparseRate);

    double time2 = get_current_time();

    *learn_errRate = l_errRate;
    *var_l_errRate = l_varErr;
    *sparse_rate = sparseRate;
    *time_cost = (float)(time2 - time1);
}

void sol_test(long optimizer, long test_dataset, float* t_errRate, float* time_cost){
    Optimizer<FeatType, LabelType> *opti = (Optimizer<FeatType, LabelType> *)optimizer;
    if (opti == NULL)
        return;
    double time2 = get_current_time();
    //test the model
    *t_errRate = opti->Test(*((DataSet<FeatType, LabelType>*)test_dataset));
    double time3 = get_current_time();

    *time_cost = (float)(time3 - time2);
}

/**
 *  sol_release : release initialized objects
 *
 * @Param:  handler
 */
void sol_release_dataset(long dataset){
    delete (DataSet<FeatType, LabelType>*)dataset;
}


void sol_release_loss(long loss_func){
   delete (LossFunction<FeatType, LabelType> *)loss_func; 
}

void sol_release_optimizer(long optimizer){
    Optimizer<FeatType, LabelType> *opti = (Optimizer<FeatType, LabelType> *)optimizer;
    delete opti;
}



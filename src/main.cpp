/*************************************************************************
	> File Name: main.cpp
	> Copyright (C) 2013 Yue Wu<yuewu@outlook.com>
	> Created Time: 2013/9/20 13:18:02
	> Functions: 
 ************************************************************************/

#if WIN32
#include <windows.h>
#endif

#include "data/DataSet.h"
#include "data/libsvmread.h"

#include "optimizer/SGD.h"
#include "optimizer/STG.h"
#include "optimizer/RDA_L1.h"
#include "optimizer/FOBOS.h"
#include "optimizer/Ada_FOBOS.h"
#include "optimizer/Ada_RDA.h"
#include "optimizer/DAROW.h"
#include "optimizer/SSAROW.h"
#include "optimizer/ASAROW.h"
#include "optimizer/CW_RDA.h"
#include "optimizer/SCW_RDA.h"

#include "loss/LogisticLoss.h"
#include "loss/HingeLoss.h"
#include "loss/SquareLoss.h"
#include "loss/SquaredHingeLoss.h"

#include "common/util.h"

#include "Params.h"

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

int main(int argc, char** args) {

	//check memory leak in VC++
#if defined(_MSC_VER) && defined(_DEBUG)
	int tmpFlag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );
	tmpFlag |= _CRTDBG_LEAK_CHECK_DF;
	_CrtSetDbgFlag( tmpFlag );
#endif
	/*
	int new_argc = 4;
	char** argv = new char*[argc + new_argc];
	for(int i = 0; i < argc; i++)
	argv[i] = args[i];
	argv[argc + 0] = "-opt";
	argv[argc + 1] = "SGD";
	argv[argc + 2] = "-i";
	argv[argc + 3] = "/home/matthew/work/Data/epsilon/epsilon_sample";
	argc += new_argc;
	Params param(argc, argv);
	delete []argv;
	*/
	Params param(argc, args);

	DataSet<FeatType, LabelType> dataset(param.passNum,param.buf_size);
	if (dataset.Load(param.fileName, param.cache_fileName) == false){
		cerr<<"Load dataset "<<param.fileName<<" failed!"<<endl;
		return -1;
	}
	LossFunction<FeatType, LabelType> *lossFunc = GetLossFunc<FeatType, LabelType>(param);
	if(lossFunc == NULL)
		return -1;
	Optimizer<FeatType, LabelType> *opti = GetOptimizer(param,dataset,*lossFunc);
	if (opti == NULL)
		return -1;

	opti->SetParameter(param.lambda,param.eta, param.power_t, param.initial_t);

	//learn the best parameters
	if (param.is_learn_best_param == true){
		opti->BestParameter();
	}

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

	delete lossFunc;
	delete opti;

	return 0;
}

template <typename T1, typename T2>
LossFunction<T1,T2>* GetLossFunc(const Params &param) {
	switch(param.loss_type) {
	case Loss_Type_Hinge:
		return new HingeLoss<T1,T2>();
	case Loss_Type_Logit:
		return new LogisticLoss<T1,T2>();
	case Loss_Type_Square:
		return new SquareLoss<T1,T2>();
	case Loss_Type_SquareHinge:
		return new SquaredHingeLoss<T1, T2>();
	default:
		cout<<"Unrecognized Loss function!"<<endl;
		return NULL;
	}
}

template <typename T1, typename T2>
Optimizer<T1,T2>* GetOptimizer(const Params &param, DataSet<T1,T2> &dataset, LossFunction<T1,T2> &lossFunc) {
	switch(param.opti_method) {
	case Opti_SGD: 
		{
			SGD<T1,T2> *opti = new SGD<T1,T2>(dataset,lossFunc);
			opti->SetParameter(param.lambda,param.eta);
			return opti;
			break;
		}
	case Opti_STG: 
		{
			STG<T1,T2> *opti = new STG<T1,T2>(dataset,lossFunc);
			opti->SetParameterEx(param.lambda,param.K, param.eta);
			return opti;
			break;
		}
	case Opti_RDA: 
		{
			RDA_L1<T1,T2> *opti = new RDA_L1<T1,T2>(dataset,lossFunc,false);
			opti->SetParameterEx(param.lambda);
			return opti;
			break;
		}
	case Opti_RDA_E: 
		{
			RDA_L1<T1,T2> *opti = new RDA_L1<T1,T2>(dataset,lossFunc,true);
			opti->SetParameterEx(param.lambda,param.gamma_rou);
			return opti;
			break;
		}
	case Opti_FOBOS: 
		{
			FOBOS<T1,T2> *opti = new FOBOS<T1,T2>(dataset,lossFunc);
			opti->SetParameter(param.lambda,param.eta);
			return opti;
			break;
		}
	case Opti_Ada_RDA: 
		{
			Ada_RDA<T1,T2> *opti = new Ada_RDA<T1,T2>(dataset,lossFunc);
			opti->SetParameterEx(param.lambda,param.delta,param.eta);
			return opti;
			break;
		}
	case Opti_Ada_FOBOS: 
		{
			Ada_FOBOS<T1,T2> *opti = new Ada_FOBOS<T1,T2>(dataset,lossFunc);
			opti->SetParameterEx(param.lambda,param.delta,param.eta);
			return opti;
			break;
		}
	case Opti_DAROW: 
		{
			DAROW<T1,T2> *opti = new DAROW<T1, T2>(dataset,lossFunc);
			opti->SetParameterEx(param.lambda, param.r);
			return opti;
			break;
		}
	case Opti_SSAROW: 
		{
			SSAROW<T1,T2> *opti = new SSAROW<T1, T2>(dataset,lossFunc);
			opti->SetParameterEx(param.lambda, param.r);
			return opti;
			break;
		}
	case Opti_ASAROW: 
		{
			ASAROW<T1,T2> *opti = new ASAROW<T1, T2>(dataset,lossFunc);
			opti->SetParameterEx(param.K, param.lambda, param.r);
			return opti;
			break;
		}
	case Opti_CW_RDA:
		{
			CW_RDA<T1,T2> *opti = new CW_RDA<T1, T2>(dataset,lossFunc);
			opti->SetParameterEx(param.lambda, param.r);
			return opti;
			break;
		}
	case Opti_SCW_RDA:
		{
			SCW_RDA<T1,T2> *opti = new SCW_RDA<T1, T2>(dataset,lossFunc);
			opti->SetParameterEx(param.lambda, param.r);
			return opti;
			break;
		}

	default:
		break;
	}
	return NULL;
}

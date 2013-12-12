/*************************************************************************
> File Name: Params.cpp
> Copyright (C) 2013 Yue Wu<yuewu@outlook.com>
> Created Time: Thu 26 Sep 2013 05:49:18 PM SGT
> Functions: Class for Parsing parameters
************************************************************************/
#include "Params.h"
#include "common/util.h"
#include "common/init_param.h"

#include <iostream>
#include <cstdlib>

using namespace std;
using namespace ez;

namespace SOL {
	Params::Params() {
		this->vfloat = new ezOptionValidator("f");
		this->vint = new ezOptionValidator("u4");
		this->vbool = new ezOptionValidator("t","in","true,false",false);

		this->Init();
	}

	Params::~Params(){
	}
	void Params::Init(){
		//initialize params
		opt.overview = "Sparse Online Learning Library";
		opt.syntax	= "SOL [options] -i train_file" ;
		opt.example = "SOL -i train_file -opt SGD";

		opt.add("",0,0,',',"help message","-h","--help");

		this->add_option("",0,1,"train file","-i", &this->fileName);
		this->add_option("",0,1,"test file name","-t",&this->test_fileName);
		this->add_option("",0,1,"cached train file name","-c",&this->cache_fileName);
		this->add_option("",0,1,"cached test file name","-tc",&this->test_cache_fileName);

		this->add_option(init_data_type,0,1,"data type format","-dt",&this->str_data_type);
		this->add_option(init_buf_size,0,1,"number of chunks for buffering","-bs",&this->buf_size);

		this->add_option(init_loss_type,0,1,"loss function type:\nHinge, Logit, Square, SquareHinge","-loss",&this->str_loss);

		this->add_option(init_opti_method,0,1,
			"optimization method:\nSGD, STG, RDA, RDA_E, FOBOS, Ada-RDA, Ada-FOBOS, AROW, SAROW, CW-RDA, SCW-RDA","-opt", &this->str_opt);
		this->add_option(init_is_learn_best_param,0,0,"learn best parameter", 
			"-lbp", &this->is_learn_best_param);
		this->add_option(init_eta,0,1,"learning rate", "-eta",&this->eta); 
		this->add_option(init_power_t,0,1,"power t of decaying learning rate","-power_t",&this->power_t); 
		this->add_option(init_initial_t,0,1,"initial iteration number","-t0",&this->initial_t);
		this->add_option(init_lambda,0,1,"l1 regularization","-l1", &this->lambda);
		this->add_option(1,0,1,"number of passes","-passes", &this->passNum);
		this->add_option(init_k,0,1,
			"number of k in truncated gradient descent or feature selection","-k", &this->K);
		this->add_option(init_gammarou,0,1,"gamma times rou in enhanced RDA (RDA_E)", "-grou",&this->gamma_rou); 
		this->add_option(init_delta,0,1,"delta in Adaptive algorithms(Ada-)","-delta", &this->delta);
		this->add_option(init_r,0,1,"r in Confidence weighted algorithms","-r",&this->r);
		this->add_option(init_phi,0,1,"phi in SCW","-phi",&this->phi); 
		this->add_option(init_normalize,0,0,"whether normalize the data","-norm",&this->is_normalize);
	}

	void Params::add_option(float default_val, bool is_required, int expectArgs, 
		const char* descr, const char* flag, float *storage){
			*storage = default_val;
			this->opt.add("",is_required,expectArgs,0,descr,flag,this->vfloat);
			this->flag2storage_float[flag] = storage;
	}
	void Params::add_option(int default_val, bool is_required, int expectArgs, 
		const char* descr, const char* flag, int *storage){
			*storage = default_val;
			this->opt.add("",is_required,expectArgs,0,descr,flag,this->vint);
			this->flag2storage_int[flag] = storage;
	}
	void Params::add_option(bool default_val, bool is_required, int expectArgs, 
		const char* descr, const char* flag, bool *storage){
			*storage = default_val;
			this->opt.add("",is_required,expectArgs,0,descr,flag, this->vbool);
			this->flag2storage_bool[flag] = storage;
	}

	void Params::add_option(const char* default_val, bool is_required, int expectArgs, 
		const char* descr, const char* flag, string *storage){
			*storage = default_val;
			this->opt.add("",is_required,expectArgs,0,descr,flag);
			this->flag2storage_str[flag] = storage;
	}

	bool Params::Parse(int argc, const char** args) {
		if (opt.isSet("-h")){
			this->Help();
			return false;
		}
		opt.parse(argc, args);
		vector<string> badOptions;
		if (!opt.gotRequired(badOptions)){
			for (size_t i = 0; i < badOptions.size(); i++)
				cerr<<"ERROR: Missing required option "<<badOptions[i]<<".\n\n";
			this->Help();
			return false;
		}
		if (!opt.gotExpected(badOptions)){
			for (size_t i = 0; i < badOptions.size(); i++)
				cerr<<"ERROR: Got unexpected number of arguments for option "<<badOptions[i]<<".\n\n";
			this->Help();
			return false;
		}
		for (map_float_iter iter = this->flag2storage_float.begin();
			iter != this->flag2storage_float.end(); iter++){
				if (opt.isSet(iter->first.c_str()))
					opt.get(iter->first.c_str())->getFloat(*(iter->second));
		}

		for (map_int_iter iter = this->flag2storage_int.begin();
			iter != this->flag2storage_int.end(); iter++){
				if (opt.isSet(iter->first.c_str()))
					opt.get(iter->first.c_str())->getInt(*(iter->second));
		}
		for (map_bool_iter iter = this->flag2storage_bool.begin();
			iter != this->flag2storage_bool.end(); iter++){
				if (opt.isSet(iter->first.c_str()))
					if (opt.get(iter->first.c_str())->expectArgs == 0)
						*(iter->second) = true;
					else{
						string out;
						opt.get(iter->first.c_str())->getString(out);
						ToLowerCase(out);
						if (out == "true")
							*(iter->second) = true;
						else
							*(iter->second) = false;
					}
		}
		for (map_str_iter iter = this->flag2storage_str.begin();
			iter != this->flag2storage_str.end(); iter++){
				if (opt.isSet(iter->first.c_str()))
					opt.get(iter->first.c_str())->getString(*(iter->second));
		}

		if (this->cache_fileName.size() == 0 && this->fileName.length() == 0){
			cerr<<"you must specify the training data"<<endl;
			return false;
		}
		return true;
	}

	void Params::Help() {
		string usage;
		opt.getUsage(usage);
		cout<<usage<<endl;
	}
}

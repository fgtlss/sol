/*************************************************************************
	> File Name: main.cpp
	> Copyright (C) 2013 Yue Wu<yuewu@outlook.com>
	> Created Time: 2013/9/20 13:18:02
	> Functions:
	************************************************************************/
#include "BOC.h"
#include "utils/init_param.h"

#include <string>
#include <fstream>

using namespace std;
using namespace BOC;

#define FeatType float
#define LabelType char

int main(int argc, const char** args) {
	//check memory leak in VC++
#if defined(_MSC_VER) && defined(_DEBUG)
	int tmpFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
	tmpFlag |= _CRTDBG_LEAK_CHECK_DF;
	_CrtSetDbgFlag(tmpFlag);
	//_CrtSetBreakAlloc(968);
#endif

	Params param;
	LibBOC<FeatType, LabelType> libBoc;
	libBoc.InitParams(param);

	if (param.Parse(argc, args) == false){
		return -1;
	}

	libBoc.ShowHelpInfo(param);

    //exit if no input is specified!
	if (param.StringValue("-i").length() == 0 && param.StringValue("-c").length() == 0){
		return 0;
	}

	int errCode = libBoc.Initialize(param);
	if ( errCode != 0){
		fprintf(stderr, "Initialization failed, error code %d\n", errCode);
		return errCode;
	}
	else{
		libBoc.Run();
	}

	return 0;
}

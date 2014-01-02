/*************************************************************************
	> File Name: util.h
	> Copyright (C) 2013 Yue Wu<yuewu@outlook.com>
	> Created Time: 8/19/2013 Monday 2:17:56 PM
	> Functions: 
 ************************************************************************/
#ifndef HEADER_UTIL
#define HEADER_UTIL

#include "init_param.h"

#include <cstring>
#include <numeric>
#include <math.h>
#include <ctype.h>

#if WIN32
#include <direct.h>
#include <io.h>
#include <windows.h>
#define SOL_ACCESS(x) _access(x,0)
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#define SOL_ACCESS(x) access(x,F_OK)
#endif

#include <iostream>

template <typename T>
inline char Sgn(T x) {
	if (x > 0) return 1;
	else if (x < 0) return -1;
	else  return 0;
}

//#define ABS(x) (x > 0 ? x : -x)
template <typename T>
inline T ABS(T x) {
    return x > 0 ? x : -x;
}

template <typename T>
inline float Average(const T* data, int dim) {
	return std::accumulate(data,data + dim, 0.f) / (float)dim;
}

template <typename T>
float Variance(const T* data, int dim) {
	if (dim <= 1)
		return 0;
	float ave = std::accumulate(data, data + dim,0.f) / (float)dim;
	double var(0);
	for (int i = 0; i < dim; i++)
		var += (data[i] - ave) * (data[i] - ave);
	return (float)(sqrt(var / (dim - 1)));
}

inline float trunc_weight(float w, float gravity){
    if (w > 0)
        return (gravity < w) ? w - gravity : 0.f;
    else
        return (gravity < -w) ? w + gravity : 0.f;
}
inline float trunc_weight2(float w, float gravity){
    if (w > 0)
        return (gravity < w) ?  -gravity : -w;
    else
        return (gravity < -w) ? gravity : -w;
}

inline void ToUpperCase(char* c_str) {
    size_t i = 0;
    while(1){
        if (c_str[i] >= 'a' && c_str[i] <= 'z'){
            c_str[i] -=  32;
            i++;
        }
        else if (c_str[i] >= 'A' && c_str[i] <= 'Z'){
            i++;
        }
        else
            break;
    }
}
inline void ToLowerCase(char* c_str) {
    size_t i = 0;
    while(1){
        if (c_str[i] >= 'a' && c_str[i] <= 'z'){
            i++;
        }
        else if (c_str[i] >= 'A' && c_str[i] <= 'Z'){
            c_str[i] += 32;
            i++;
        }
        else
            break;
    }
}

inline void ToUpperCase(string &str) {
	string dst_str;
	int len = str.length();
	for (int i = 0; i < len; i++)
		dst_str.push_back(toupper(str[i]));
	std::swap(str,dst_str);
}

inline void ToLowerCase(string &str) {
	string dst_str;
	int len = str.length();
	for (int i = 0; i < len; i++)
		dst_str.push_back(tolower(str[i]));
	std::swap(str,dst_str);
}

inline bool rename_file(const string& src_filename, const string& dst_filename){
	string cmd;
	if (SOL_ACCESS(dst_filename.c_str()) == 0){
		//del the original cache_file
#if WIN32
		cmd = "del \"" + dst_filename + "\"";
#else
		cmd = "rm \"" + dst_filename + "\"";
#endif
		system(cmd.c_str());
	}
	//rename
#if WIN32
	cmd = "ren \"";
	cmd = cmd + src_filename + "\" \"";
	//in windows, the second parameter of ren should not include path
	cmd = cmd + dst_filename.substr(dst_filename.find_last_of("/\\") + 1) + "\"";
#else
	string cmd = "mv \"";
	cmd = cmd + src_filename + "\" \"";
	cmd = cmd + dst_filename + "\"";
#endif

	if(system(cmd.c_str()) != 0){
		std::cerr<<"rename cahe file name failed!"<<std::endl;
		return false;
	}
	return true;
}

inline double get_current_time(){
#if _WIN32
	return GetTickCount() / 1000.0;
#else
	struct timeval tim;
	gettimeofday(&tim, NULL);
	return tim.tv_sec + tim.tv_usec / 1000000.0;
#endif
}
#endif

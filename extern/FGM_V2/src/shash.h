#ifndef _SHASH_H_
#define _SHASH_H_

#include <stdio.h>
//#include <conio.h>
//#include <iostream>

//#include <map>

#include <hash_map>

//#include <algorithm>

 
//
//#include <ace/OS.h>
//
//#include <ace/Time_Value.h>

 

const size_t TEST_NUMBER         = 200000;

const size_t INIT_BUCKETS_NUMBER = 262144;

 

class test_hash_compare

{

public:

   //ע������ط��������¶�������2����������ע�⡿

   static const size_t bucket_size = 1;

   static const size_t min_buckets = INIT_BUCKETS_NUMBER;

   //static const size_t min_buckets = 8;

   test_hash_compare()

   {

   }

   size_t operator( )( const size_t& Key ) const

   {

       return Key;

   }

   bool operator( )( 

      const size_t& _Key1,

      const size_t& _Key2

   ) const

   {

       return (_Key1<_Key2)?true:false;

   }

};

 

void test_hash_map()

{

 

//    ACE_Time_Value tvStart(0);
//
//    ACE_Time_Value tvEnd(0);
//
//    ACE_Time_Value tvPassTime(0);
//
// 
//
//tvStart = ACE_OS::gettimeofday(); 

 

    //����ʹ��STLPORT����еĺ�

#if defined _STLPORT_VERSION

    std::hash_map<size_t,int>   int_hash_map;

    //ע�����д���,VS.NETĬ�ϵ�STLû����������ģ���STLPort��ʵ�����������

    int_hash_map.resize(INIT_BUCKETS_NUMBER); 

#else 

    stdext::hash_map<size_t,double,test_hash_compare>   int_hash_map;

#endif //

 

    //����20���

    //˳�����һ������

    for (size_t i= 0;i<TEST_NUMBER;++i)
    {
        int_hash_map[i]=1.0;
    }

 

    //��ѯ40��Σ�һ���ܲ�ѯ����һ�벻�ܲ�ѯ��
    double v = 0.0;
    for (size_t i= 0;i<2*TEST_NUMBER;++i)

    {
        int_hash_map.find(i);
		v = int_hash_map[i];
		int_hash_map[i] = v + 0.5;
		v = int_hash_map[i];
		int_hash_map.erase(i);
		if(int_hash_map[i])
		{
			v = int_hash_map[i];
		}

    }

 

    //�õ������ʱ���

 

   /* tvEnd = ACE_OS::gettimeofday(); 

    tvPassTime = tvEnd - tvStart;

 

    std::cout<<"test_hash_map gettimeofday :"<<tvPassTime.msec()<<" "<<std::endl;*/

};
#endif
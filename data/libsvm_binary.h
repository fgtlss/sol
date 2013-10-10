/*************************************************************************
  > File Name: libsvm_binary.h
  > Copyright (C) 2013 Yue Wu<yuewu@outlook.com>
  > Created Time: Sat 21 Sep 2013 10:52:41 PM SGT
  > Functions:  io for binary libsvm dataset
 ************************************************************************/

#pragma once

#include "DataReader.h"

#include <iostream>
#include <fstream>
#include <string>

using namespace std;
namespace SOL
{
    template <typename FeatType, typename LabelType>
    class libsvm_binary_:public DataReader<FeatType, LabelType>
    {
        private:
            std::string fileName;
            std::fstream file;

        public:
            libsvm_binary_(const std::string &fileName)
            {
                this->fileName = fileName;
            }
            ~libsvm_binary_()
            {
                this->Close();
            }

            //////////////////online mode//////////////////
        public:
            bool OpenReading()
            {
                this->Close();

                file.open(fileName.c_str(),ios::binary | ios::in);

                if (file.good() == false)
                {
                    printf("can't open input file %s\n",fileName.c_str());
                    return false;
                }
                return true;
            }
            bool OpenWriting()
            {
                if (file.is_open())
                    this->Close();
                file.open(fileName.c_str(),std::ios::binary | std::ios::out | std::ios::trunc);
                if (file.good() == false)
                {
                    printf("can't open output file %s\n",fileName.c_str());
                    return false;
                }
                return true;
            }

            void Rewind()
            {
                file.clear();
                file.seekg(0,std::ios::beg);
            }
            void Close()
            {
                file.close();
            }
            inline bool Good()
            {
                if (file.good() == false)
                {
                    if ((file.rdstate() & std::ifstream::eofbit) != 0)
                    {
                        cout<<"unexpected error"<<endl;
                    }
                    else
                        cout<<"reach end of file" <<endl;
                    return false;
                }
                return file.good();
            }

            virtual int GetMinIndex()  const {return 1;}
            
            bool GetNextData(DataPoint<FeatType, LabelType> &data)
            {
				data.erase();
                int featNum = 0;
                file.read((char*)&featNum,sizeof(int));
                if (featNum > 0)
                {
                    file.read((char*)&(data.max_index),sizeof(int));
                    file.read((char*)&data.label,sizeof(LabelType));
                    data.indexes.resize(featNum);
                    file.read((char*)data.indexes.begin,sizeof(int) * featNum);
                    data.features.resize(featNum);
                    file.read((char*)data.features.begin,sizeof(FeatType) * featNum);
                    //file.read((char*)data.weights.begin,sizeof(float) * featNum);
                    return true;
                }
                else
                    return false;
            }

            bool GetNextData(DataChunk<FeatType, LabelType> &chunk)
            {
				chunk.erase();
                for (int i = 0; i < init_chunk_size; i++)
                {
                    DataPoint<FeatType,LabelType> &data = chunk.data[i];

                    int featNum = 0;
                    file.read((char*)&featNum,sizeof(int));
                    if (featNum > 0)
                    {
                        file.read((char*)&(data.max_index),sizeof(int));
                        file.read((char*)&data.label,sizeof(LabelType));
                        data.indexes.resize(featNum);
                        file.read((char*)data.indexes.begin,sizeof(int) * featNum);
                        data.features.resize(featNum);
                        file.read((char*)data.features.begin,sizeof(FeatType) * featNum);
                        //file.read((char*)data.weights.begin,sizeof(float) * featNum);
                        chunk.dataNum++;
                    }
                    else
                    {
                        return false;
                    }
                }
                return true;
            }

            bool WriteData(DataPoint<FeatType, LabelType> &data)
            {
                int featNum = data.indexes.size();
                file.write((const char*)&featNum,sizeof(int));
                file.write((const char*)&(data.max_index),sizeof(int));
                file.write((const char*)&data.label,sizeof(LabelType));
                file.write((const char*)data.indexes.begin,sizeof(int) * featNum);
                file.write((const char*)data.features.begin,sizeof(FeatType) * featNum);
                //file.write((const char*)data.weights.begin,sizeof(float) * featNum);
                return true;
            }
    };

    //for special definition
    typedef libsvm_binary_<float, char> libsvm_binary;
}

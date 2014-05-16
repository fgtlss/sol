/*************************************************************************
  > File Name: binary_io.h
  > Copyright (C) 2013 Yue Wu<yuewu@outlook.com>
  > Created Time: Sat 21 Sep 2013 10:52:41 PM SGT
  > Functions:  io for binary libsvm dataset
  ************************************************************************/

#ifndef HEADER_BINARY_IO
#define HEADER_BINARY_IO


#include "DataHandler.h"
#include "basic_io.h"
//#include "zlib_io.h"
//#include "gzip_io.h"

#include "comp.h"

#include <new>

using namespace std;

namespace BOC {
	template <typename FeatType, typename LabelType>
	class binary_io : public DataHandler<FeatType, LabelType> {
		//dynamic binding
		DECLARE_CLASS
	private:
		basic_io io_handler;
		//gzip_io io_handler.
		//zlib_io io_handler.

		//compressed codes of indexes
		s_array<char> comp_codes;

		bool is_good; //indicate if the current reader is good

	public:
		binary_io(const std::string &fileName) : DataHandler<FeatType, LabelType>(fileName){
			this->is_good = true;
		}

		~binary_io() {
			this->Close();
		}
		const std::string& get_filename() const {
			return this->fileName;
		}

		//////////////////online mode//////////////////
	public:
		bool OpenReading() {
			this->Close();
			this->is_good = io_handler.open_file(this->fileName.c_str(), "rb");
			return this->is_good;
		}

		bool OpenWriting() {
			this->Close();
			this->is_good = io_handler.open_file(this->fileName.c_str(), "wb");
			return this->is_good;
		}

		void Rewind() {
			io_handler.rewind();
		}

		void Close() {
			io_handler.close_file();
		}

		inline bool Good() {
			return this->is_good == true && io_handler.good() == 0 ? true : false;
		}

		bool GetNextData(DataPoint<FeatType, LabelType> &data) {
			data.erase();
			if (io_handler.read_data((char*)&(data.label), sizeof(LabelType)) == false){
				if (this->Good() == true){
					return false;
				}
				else{
					cerr << "unexpected error occured when loading data!" << endl;
					this->is_good = false;
					return false;
				}
			}
			//assert(data.label == 1 || data.label == -1);

			size_t featNum = 0;
			if (io_handler.read_data((char*)&featNum, sizeof(size_t)) == false){
				cerr << "load feature number failed!" << endl;
				this->is_good = false;
				return false;
			}
			if (featNum > 0){
				if (io_handler.read_data((char*)&data.max_index, sizeof(IndexType)) == false){
					cerr << "load max index failed!" << endl;
					this->is_good = false;
					return false;
				}
				unsigned int code_len = 0;
				if (io_handler.read_data((char*)&code_len,
					sizeof(unsigned int)) == false){
					cerr << "read coded index length failed!" << endl;
					this->is_good = false;
					return false;
				}
				this->comp_codes.resize(code_len);
				if (io_handler.read_data(this->comp_codes.begin,
					code_len) == false){
					cerr << "read coded index failed!" << endl;
					this->is_good = false;
					return false;
				}
				decomp_index(this->comp_codes, data.indexes);
				if (data.indexes.size() != featNum){
					cerr << "decoded index number is not correct!" << endl;
					this->is_good = false;
					return false;
				}
				data.features.resize(featNum);
				if (io_handler.read_data((char*)(data.features.begin),
					sizeof(float)* featNum) == false){
					cerr << "load features failed!" << endl;
					this->is_good = false;
					return false;
				}
				if (io_handler.read_data((char*)&(data.sum_sq), sizeof(float)) == false){
					cerr << "load sum of square failed!" << endl;
					this->is_good = false;
					return false;
				}
			}
			return true;
		}

		bool WriteData(DataPoint<FeatType, LabelType> &data) {
			size_t featNum = data.indexes.size();
			if (io_handler.write_data((char*)&data.label, sizeof(LabelType)) == false){
				cerr << "write label failed!" << endl;
				this->is_good = false;
				return false;
			}

			if (io_handler.write_data((char*)&featNum, sizeof(size_t)) == false){
				cerr << "write feat number failed!" << endl;
				this->is_good = false;
				return false;
			}
			if (featNum > 0){
				if (io_handler.write_data((char*)&data.max_index,
					sizeof(IndexType)) == false){
					cerr << "write max index failed!" << endl;
					this->is_good = false;
					return false;
				}
				this->comp_codes.erase();
				comp_index(data.indexes, this->comp_codes);
				unsigned int code_len = (unsigned int)(this->comp_codes.size());
				if (io_handler.write_data((char*)&code_len,
					sizeof(unsigned int)) == false){
					cerr << "write coded index length failed!" << endl;
					this->is_good = false;
					return false;
				}
				if (io_handler.write_data(this->comp_codes.begin,
					code_len) == false){
					cerr << "write coded index failed!" << endl;
					this->is_good = false;
					return false;
				}
				if (io_handler.write_data((char*)(data.features.begin),
					sizeof(float)* featNum) == false){
					cerr << "write features failed!" << endl;
					this->is_good = false;
					return false;
				}
				if (io_handler.write_data((char*)&(data.sum_sq), sizeof(float)) == false){
					cerr << "write sum of square failed!" << endl;
					this->is_good = false;
					return false;
				}
			}
			return true;
		}
	};

	//dynamic binding
	IMPLEMENT_DATA_CLASS(binary_io, "binary", "io class for binary compressed data");

}
#endif

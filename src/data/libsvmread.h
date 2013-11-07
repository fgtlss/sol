/*************************************************************************
> File Name: libsvmread.h
> Copyright (C) 2013 Yue Wu<yuewu@outlook.com>
> Created Time: 2013/8/18 星期日 20:25:28
> Functions: libsvm reader
************************************************************************/
#pragma once

#if _WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "DataReader.h"
#include "basic_io.h"
#include "parser.h"

#include <stdio.h>
#include <vector>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits>
#include <cmath>

using namespace std;

namespace SOL {
	template <typename FeatType, typename LabelType>
	class LibSVMReader_: public DataReader<FeatType, LabelType> { 
	private:
		string fileName;

        basic_io reader;

		char *line;
		size_t max_line_len;

	public:
		LibSVMReader_(const string &fileName) {
			this->max_line_len = 4096;
			this->fileName = fileName;
			line = (char *) malloc(max_line_len*sizeof(char));
		}
		~LibSVMReader_() {
            this->Close();
			if (line != NULL)
				free(line);
		}

		//////////////////online mode//////////////////
	public:
		virtual bool OpenReading() {
            this->Close();
            return reader.open_file(this->fileName.c_str(), "r");
		}
		virtual void Rewind() {
            reader.rewind();
		}
		virtual void Close() {
            reader.close_file();
        }

		virtual inline bool Good() {
            return reader.good() == 0 ? true: false;
		}

		virtual bool GetNextData(DataPoint<FeatType, LabelType> &data) {
			if(reader.read_line(line, max_line_len) == NULL)
				return false;

			LabelType labelVal;
			char* p = line, *endptr = NULL;
			if (*p == '\0')
				return false;
			labelVal = (LabelType)parseInt(p,&endptr);
			if (endptr == p) {
				fprintf(stderr, "parse label failed.\n");
				exit(0);
			}
			 
			data.erase();
			unsigned int index;
			FeatType feat;
			// features
			while(1) {
				p = strip_line(endptr);
				if (*p == '\0')
					break;
				index = parseUint(p,&endptr);
				if (endptr == p) { //parse index failed
					fprintf(stderr,"parse index value failed!\n%s", p);
                    
					exit(0);
				}
				
				p = endptr;
				feat = parseFloat(p,&endptr);
				//feat =(float)(strtod(val,&endptr));
				if (endptr == p) {
					printf("parse feature value failed!\n");
					exit(0);
				}

				data.AddNewFeat(index,feat);
			}
			data.label = labelVal;
			return true;
		}
		
                /* Parse S into tokens separated by characters in DELIM.
           If S is NULL, the saved pointer in SAVE_PTR is used as
           the next starting point.  For example:
           char s[] = "-abc-=-def";
           char *sp;
           x = strtok_r(s, "-", &sp);      // x = "abc", sp = "=-def"
           x = strtok_r(NULL, "-=", &sp);  // x = "def", sp = NULL
           x = strtok_r(NULL, "=", &sp);   // x = NULL
        // s = "abc\0-def\0"
        thread safe
        */
        char *ts_strtok(char *s, const char *delim, char **save_ptr) {
            char *token;

            if (s == NULL) s = *save_ptr;

            /* Scan leading delimiters.  */
            s += strspn(s, delim);
            if (*s == '\0') 
                return NULL;

            /* Find the end of the token.  */
            token = s;
            s = strpbrk(token, delim);
            if (s == NULL)
                /* This token finishes the string.  */
                *save_ptr = strchr(token, '\0');
            else {
                /* Terminate the token and make *SAVE_PTR point past it.  */
                *s = '\0';
                *save_ptr = s + 1;
            }

            return token;
        }



    };

    //for special definition
    typedef LibSVMReader_<float, char> LibSVMReader;
}

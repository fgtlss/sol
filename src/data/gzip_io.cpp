/*************************************************************************
  > File Name: gzip_io.cpp
  > Copyright (C) 2013 Yue Wu<yuewu@outlook.com>
  > Created Time: Wed 06 Nov 2013 04:01:46 PM
  > Descriptions: read and write file in gzip format
 ************************************************************************/
#include "gzip_io.h"

#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#else
#  define SET_BINARY_MODE(file)
#endif

namespace SOL{
    bool gzip_io::open_file(const char* filename, const char* mode){
        this->close_file();

        file = gzopen(filename, mode);
        if (file == NULL){
            cerr<<"open file failed!"<<endl;
            return false;
        }
        return this->good();
    }
    // bind_stdin: bind the input to stdin
    bool gzip_io::open_stdin(){
        file = gzdopen(fileno(stdin),"rb");
        return true;
    }

    // bind_stdin: bind the output to stdout
    bool gzip_io::open_stdout(){
        file = gzdopen(fileno(stdout),"wb");
        return true;
    }

    void gzip_io::close_file(){
        if (file != NULL && file != stdin && file != stdout){
            gzclose(file);
        }
        file = NULL;
    }

    void gzip_io::rewind(){
        if (file != NULL)
            gzrewind(file);
    }

    /**
     * good : test if the io is good
     *
     * @Return: zero if correct, else zero code
     */
    int gzip_io::good(){
        int errCode;
        const char* errmsg = gzerror(file ,&errCode);;
        if (errCode != Z_OK){
            printf("%s\n",errmsg);
        }
        return errCode;
    }

    /**
     * read_data : read the data from file
     *
     * @Param dst: container to place the read data
     * @Param length: length of data of read in bytes
     *
     * @Return: size of data read in bytes
     */
    size_t gzip_io::read_data(char* dst, size_t length){
        return gzread(dst, length, file);
    }

    /**
     * read_line : read a line from disk
     *
     * @Param dst: container to place the read data
     * @Param dst_len: length of dst
     *
     * @Return: size of data read in bytes
     */
    char* gzip_io::read_line(char* &dst, size_t &dst_len){
        printf("error: no read line is supported in gzip io\n");
        return NULL;
    }

    /**
     * write_data : write content to disk
     *
     * @Param src: source of the data
     * @Param length: length to write the data
     *
     * @Return: size of data written to disk in bytes
     */
    size_t gzip_io::write_data(char* src, size_t length){
        return fwrite(src, length, file);
    }
}



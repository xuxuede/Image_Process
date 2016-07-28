#ifndef BITTRANSFORMATION_H
#define BITTRANSFORMATION_H
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <iostream>

namespace  Image{
namespace FormatTransformation {

class BitTransformation
{
public:
    BitTransformation();

    void do_tranformation(const char *BMP_filename, const char *BMP_filename_new);

public:
    unsigned char* ReadBitFile(const char *FileName,int *width,int *height);
    bool Write8BitImg(unsigned char *img,int width,int height,const char * filename);

private:
    unsigned char* PixelBitsChange(unsigned char *img,int width,int height);

};


}}


#endif//BITTRANSFORMATION_H

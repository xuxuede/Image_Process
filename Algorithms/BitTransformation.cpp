#include "BitTransformation.h"
#include <assert.h>
#include<fstream>
#include <windows.h>


namespace  Image{
namespace FormatTransformation {

BitTransformation::BitTransformation()
{

}

void BitTransformation::do_tranformation(const char *BMP_filename, const char *BMP_filename_new)//转换程序
{
    int width;
    int height;
    unsigned char *img=NULL;

    img = this->ReadBitFile(BMP_filename,&width,&height);

    if(img==NULL)
        std::cout<<"image is null"<<std::endl;

    if(!this->Write8BitImg(img,width,height,BMP_filename_new))
        std::cerr<<"cant not write the image!"<<std::endl;

    delete [] img;
}

unsigned char* BitTransformation::ReadBitFile(const char *FileName,int *width,int *height)
{
    FILE *file;
    BITMAPFILEHEADER FileHeader;
    BITMAPINFOHEADER BmpHeader;
    unsigned char *img=NULL;
	unsigned int size=0;
	int Src=0;
	if((file=fopen(FileName,"rb"))==NULL)
		return NULL;
	if(fread((void*)&FileHeader,1,sizeof(BITMAPFILEHEADER),file)!=sizeof(BITMAPFILEHEADER))
		Src=-1;
	if(fread((void*)&BmpHeader,1,sizeof(BITMAPINFOHEADER),file)!=sizeof( BITMAPINFOHEADER))
		Src=-1;
	if(Src==-1)
	{
		fclose(file);
		return NULL;
	}
   *width=BmpHeader.biWidth;
   *height=BmpHeader.biHeight;
//   size=(*width)*(*height);
   size = (BmpHeader.biWidth*3+3)/4*4*BmpHeader.biHeight;
   fseek(file , 54, SEEK_SET);
   img=new unsigned char[size];
   memset(img,255,size);
   assert(img!=NULL);
   if(img==NULL)
   {
	   fclose(file);
	   return NULL;
   }
   if(fread(img,1,size,file)!= size)
   {
	   fclose(file);
	   return NULL;
   }

   return   PixelBitsChange(img,*width,*height);
}
unsigned char* BitTransformation::PixelBitsChange(unsigned char *img,int width,int height)
{
   int _width = (width*3+3)/4*4;
        width=(width+3)/4*4;
   unsigned char *changedimg=new unsigned char[width*height];
   assert(changedimg!=NULL);

   RGBTRIPLE *rgbtriple;
   for(int i=0;i<height;++i)        
	{
		for(int j=0,k=0;j<_width;j=j+3,++k)
		{
			rgbtriple=(RGBTRIPLE*)(img+(i)*_width+j);
            *(changedimg+i*width+k)=(unsigned char)((0.299*rgbtriple->rgbtRed)+(0.587*rgbtriple->rgbtGreen)+(0.114*rgbtriple->rgbtBlue));//灰度处理
		}
	}            
   if(img!=NULL)
	   delete [] img;
   img=NULL;
   return changedimg;
}

bool BitTransformation::Write8BitImg(unsigned char *img,int width,int height,const char * filename)
{   FILE * BinFile;
    BITMAPFILEHEADER FileHeader;
    BITMAPINFOHEADER BmpHeader;
    unsigned char p[4];
	bool Suc=true;
    int i=0;

	width=(width+3)/4*4;

    for(int j=0;j<height;++j)
	{
		*(img+j*width+width-1)=255;
	}

    // Open File
    if((BinFile=fopen(filename,"w+b"))==NULL) {  return false; }
    // Fill the FileHeader位图文件头
    FileHeader.bfType= ((unsigned short) ('M' << 8) | 'B');
	FileHeader.bfOffBits=sizeof(BITMAPFILEHEADER)+sizeof(BmpHeader)+256*4L;
    //FileHeader.bfOffBits=sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFO);
    FileHeader.bfSize=FileHeader.bfOffBits+width*height;
    FileHeader.bfReserved1=0;
    FileHeader.bfReserved2=0;
	if (fwrite((void *)&FileHeader,1,sizeof(FileHeader),BinFile)!=sizeof(FileHeader)) Suc=false;
    // Fill the ImgHeader位图信息头
	BmpHeader.biSize = 40;
    BmpHeader.biWidth = width;
	BmpHeader.biHeight = height;
	BmpHeader.biPlanes = 1 ;
    BmpHeader.biBitCount = 8 ;
	BmpHeader.biCompression = 0 ;
	BmpHeader.biSizeImage = 0;
	BmpHeader.biXPelsPerMeter = 0;
	BmpHeader.biYPelsPerMeter = 0;
	BmpHeader.biClrUsed = 0;
	BmpHeader.biClrImportant = 0;
	if (fwrite((void *)&BmpHeader,1,sizeof(BmpHeader),BinFile)!=sizeof(BmpHeader)) Suc=false;
    // write Palette写入调色板
    for (i=0,p[3]=0;i<256;i++) 
    {  p[0]=p[1]=p[2]=i; // blue,green,red;
       if (fwrite((void *)p,1,4,BinFile)!=4) { Suc=false; break; }
	}
    // write image data写图像数据
	if (fwrite((void *)img,1,width*height,BinFile)!=width*height) Suc=false;
	// return;
	fclose(BinFile);
	return Suc;
}

}
}

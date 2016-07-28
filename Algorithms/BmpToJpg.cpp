#include "BmpToJpg.h"

namespace  Image{
namespace FormatTransformation {


BmpToJpg::BmpToJpg()
{
    this-> init_APP0info();
    this-> init_SOF0info();
    this-> init_SOSinfo();
    this-> init_zigzag();

    this-> init_std_luminance_qt();
    this-> init_std_chrominance_qt();

    this-> init_std_dc_luminance_nrcodes();
    this-> init_std_dc_luminance_values();

    this-> init_std_dc_chrominance_nrcodes();
    this-> init_std_dc_chrominance_values();

    this-> init_std_ac_luminance_nrcodes();
    this-> init_std_ac_luminance_values();

    this-> init_std_ac_chrominance_nrcodes();
    this-> init_std_ac_chrominance_values();

    this-> init_mask();

    bytenew=0; // The byte that will be written in the JPG file
    bytepos=7; //bit position in the byte we write (bytenew)
}

void BmpToJpg::do_tranformation(char *JPG_filename, char *BMP_filename)
{

    WORD Ximage_original,Yimage_original; //the original image dimensions,
    // before we made them divisible by 8
    bitstring fillbits; //filling bitstring for the bit alignment of the EOI marker

    load_bitmap(BMP_filename, &Ximage_original, &Yimage_original);

    fp_jpeg_stream = fopen(JPG_filename,"wb");

    if(fp_jpeg_stream == NULL)return;

    init_all();

    SOF0info.width=Ximage_original;
    SOF0info.height=Yimage_original;

    writeword(0xFFD8); //SOI
    write_APP0info();
    // write_comment("Cris made this JPEG with his own encoder");
    write_DQTinfo();
    write_SOF0info();
    write_DHTinfo();
    write_SOSinfo();

    bytenew=0;
    bytepos=7;

    main_encoder();

    //Do the bit alignment of the EOI marker
    if (bytepos>=0)
    {
        fillbits.length=bytepos+1;
        fillbits.value=(1<<(bytepos+1))-1;
        writebits(fillbits);
    }

    writeword(0xFFD9); //EOI

    free(RGB_buffer);
    free(category_alloc);
    free(bitcode_alloc);

    fclose(fp_jpeg_stream);
}

void BmpToJpg::init_APP0info()
{
    //BmpToJpg::APP0info = {0xFFE0,16,'J','F','I','F',0,1,1,0,1,1,0,0};

    this->APP0info.marker = 0xFFE0;
    this->APP0info.length = 16;
    this->APP0info.JFIFsignature[0] = 'J';
    this->APP0info.JFIFsignature[1] = 'F';
    this->APP0info.JFIFsignature[2] = 'I';
    this->APP0info.JFIFsignature[3] = 'F';
    this->APP0info.JFIFsignature[4] = '/0';
    this->APP0info.versionhi = 1; // 1
    this->APP0info.versionlo = 1; // 1
    this->APP0info.xyunits = 0;   // 0 = no units, normal density
    this->APP0info.xdensity = 1;  // 1
    this->APP0info.ydensity = 1;  // 1
    this->APP0info.thumbnwidth = 0; // 0
    this->APP0info.thumbnheight = 0; // 0
}

void BmpToJpg::init_SOF0info()
{
    //BmpToJpg::SOF0info = { 0xFFC0,17,8,0,0,3,1,0x11,0,2,0x11,1,3,0x11,1};
    this->SOF0info.marker = 0xFFC0; // = 0xFFC0
    this->SOF0info.length = 17; // = 17 for a truecolor YCbCr JPG
    this->SOF0info.precision = 8;// Should be 8: 8 bits/sample
    this->SOF0info.height = 0;
    this->SOF0info.width = 0;
    this->SOF0info.nrofcomponents = 3;//Should be 3: We encode a truecolor JPG
    this->SOF0info.IdY = 1;  // = 1
    this->SOF0info.HVY = 0x11; // sampling factors for Y (bit 0-3 vert., 4-7 hor.)
    this->SOF0info.QTY = 0;  // Quantization Table number for Y = 0
    this->SOF0info.IdCb = 2; // = 2
    this->SOF0info.HVCb = 0x11;
    this->SOF0info.QTCb = 1; // 1
    this->SOF0info.IdCr = 3; // = 3
    this->SOF0info.HVCr = 0x11;
    this->SOF0info.QTCr = 1; // Normally equal to QTCb = 1
}

void BmpToJpg::init_SOSinfo()
{
//    SOSinfo={0xFFDA,12,3,1,0,2,0x11,3,0x11,0,0x3F,0};
    this->SOSinfo.marker = 0xFFDA;  // = 0xFFDA
    this->SOSinfo.length = 12; // = 12
    this->SOSinfo.nrofcomponents = 3; // Should be 3: truecolor JPG
    this->SOSinfo.IdY = 1; //1
    this->SOSinfo.HTY = 0; //0 // bits 0..3: AC table (0..3)
    // bits 4..7: DC table (0..3)
    this->SOSinfo.IdCb = 2; //2
    this->SOSinfo.HTCb = 0x11; //0x11
    this->SOSinfo.IdCr = 3; //3
    this->SOSinfo.HTCr = 0x11; //0x11
    this->SOSinfo.Ss = 0;
    this->SOSinfo.Se = 0x3F;
    this->SOSinfo.Bf = 0; // not interesting, they should be 0,63,0
}

void BmpToJpg::init_zigzag()
{
    for(int i = 0; i < 64; i++)
    {
        this->zigzag[i] = i;
    }
}

void BmpToJpg::init_std_luminance_qt()//亮度量化数据
{
    //    BYTE std_luminance_qt[64] =
    //    {
//            16,  11,  10,  16,  24,  40,  51,  61,
//            12,  12,  14,  19,  26,  58,  60,  55,
//            14,  13,  16,  24,  40,  57,  69,  56,
//            14,  17,  22,  29,  51,  87,  80,  62,
//            18,  22,  37,  56,  68, 109, 103,  77,
//            24,  35,  55,  64,  81, 104, 113,  92,
//            49,  64,  78,  87, 103, 121, 120, 101,
//            72,  92,  95,  98, 112, 100, 103,  99
    //    };
    this->std_luminance_qt[0] = 16;
    this->std_luminance_qt[1] = 11;
    this->std_luminance_qt[2] = 10;
    this->std_luminance_qt[3] = 16;
    this->std_luminance_qt[4] = 24;
    this->std_luminance_qt[5] = 40;
    this->std_luminance_qt[6] = 51;
    this->std_luminance_qt[7] = 61;
    this->std_luminance_qt[8] = 12;
    this->std_luminance_qt[9] = 12;
    this->std_luminance_qt[10] = 14;
    this->std_luminance_qt[11] = 19;
    this->std_luminance_qt[12] = 26;
    this->std_luminance_qt[13] = 58;
    this->std_luminance_qt[14] = 60;
    this->std_luminance_qt[15] = 55;
    this->std_luminance_qt[16] = 14;
    this->std_luminance_qt[17] = 13;
    this->std_luminance_qt[18] = 16;
    this->std_luminance_qt[19] = 24;
    this->std_luminance_qt[20] = 40;
    this->std_luminance_qt[21] = 57;
    this->std_luminance_qt[22] = 69;
    this->std_luminance_qt[23] = 56;
    this->std_luminance_qt[24] = 14;
    this->std_luminance_qt[25] = 17;
    this->std_luminance_qt[26] = 22;
    this->std_luminance_qt[27] = 29;
    this->std_luminance_qt[28] = 51;
    this->std_luminance_qt[29] = 87;
    this->std_luminance_qt[30] = 80;
    this->std_luminance_qt[31] = 62;
    this->std_luminance_qt[32] = 18;
    this->std_luminance_qt[33] = 22;
    this->std_luminance_qt[34] = 37;
    this->std_luminance_qt[35] = 56;
    this->std_luminance_qt[36] = 68;
    this->std_luminance_qt[37] = 109;
    this->std_luminance_qt[38] = 103;
    this->std_luminance_qt[39] = 77;
    this->std_luminance_qt[40] = 24;
    this->std_luminance_qt[41] = 35;
    this->std_luminance_qt[42] = 55;
    this->std_luminance_qt[43] = 64;
    this->std_luminance_qt[44] = 81;
    this->std_luminance_qt[45] = 104;
    this->std_luminance_qt[46] = 113;
    this->std_luminance_qt[47] = 92;
    this->std_luminance_qt[48] = 49;
    this->std_luminance_qt[49] = 64;
    this->std_luminance_qt[50] = 78;
    this->std_luminance_qt[51] = 87;
    this->std_luminance_qt[52] = 103;
    this->std_luminance_qt[53] = 121;
    this->std_luminance_qt[54] = 120;
    this->std_luminance_qt[55] = 101;
    this->std_luminance_qt[56] = 72;
    this->std_luminance_qt[57] = 92;
    this->std_luminance_qt[58] = 95;
    this->std_luminance_qt[59] = 98;
    this->std_luminance_qt[60] = 112;
    this->std_luminance_qt[61] = 100;
    this->std_luminance_qt[62] = 103;
    this->std_luminance_qt[63] = 99;


}

void BmpToJpg::init_std_chrominance_qt()//色差量化数据
{
    //    BYTE std_chrominance_qt[64] =
    //    {
    //        17,  18,  24,  47,  99,  99,  99,  99,
    //        18,  21,  26,  66,  99,  99,  99,  99,
    //        24,  26,  56,  99,  99,  99,  99,  99,
    //        47,  66,  99,  99,  99,  99,  99,  99,
    //        99,  99,  99,  99,  99,  99,  99,  99,
    //        99,  99,  99,  99,  99,  99,  99,  99,
    //        99,  99,  99,  99,  99,  99,  99,  99,
    //        99,  99,  99,  99,  99,  99,  99,  99
    //    };
    this->std_chrominance_qt[0] = 17;
    this->std_chrominance_qt[1] = 18;
    this->std_chrominance_qt[2] = 24;
    this->std_chrominance_qt[3] = 47;
    this->std_chrominance_qt[4] = 99;
    this->std_chrominance_qt[5] = 99;
    this->std_chrominance_qt[6] = 99;
    this->std_chrominance_qt[7] = 99;
    this->std_chrominance_qt[8] = 18;
    this->std_chrominance_qt[9] = 21;

    this->std_chrominance_qt[10] = 26;
    this->std_chrominance_qt[11] = 66;
    this->std_chrominance_qt[12] = 99;
    this->std_chrominance_qt[13] = 99;
    this->std_chrominance_qt[14] = 99;
    this->std_chrominance_qt[15] = 99;
    this->std_chrominance_qt[16] = 24;
    this->std_chrominance_qt[17] = 26;
    this->std_chrominance_qt[18] = 56;
    this->std_chrominance_qt[19] = 99;

    this->std_chrominance_qt[20] = 99;
    this->std_chrominance_qt[21] = 99;
    this->std_chrominance_qt[22] = 99;
    this->std_chrominance_qt[23] = 99;
    this->std_chrominance_qt[24] = 47;
    this->std_chrominance_qt[25] = 66;
    this->std_chrominance_qt[26] = 99;
    this->std_chrominance_qt[27] = 99;
    this->std_chrominance_qt[28] = 99;
    this->std_chrominance_qt[29] = 99;

    this->std_chrominance_qt[30] = 99;
    this->std_chrominance_qt[31] = 99;
    this->std_chrominance_qt[32] = 99;
    this->std_chrominance_qt[33] = 99;
    this->std_chrominance_qt[34] = 99;
    this->std_chrominance_qt[35] = 99;
    this->std_chrominance_qt[36] = 99;
    this->std_chrominance_qt[37] = 99;
    this->std_chrominance_qt[38] = 99;
    this->std_chrominance_qt[39] = 99;

    this->std_chrominance_qt[40] = 99;
    this->std_chrominance_qt[41] = 99;
    this->std_chrominance_qt[42] = 99;
    this->std_chrominance_qt[43] = 99;
    this->std_chrominance_qt[44] = 99;
    this->std_chrominance_qt[45] = 99;
    this->std_chrominance_qt[46] = 99;
    this->std_chrominance_qt[47] = 99;
    this->std_chrominance_qt[48] = 99;
    this->std_chrominance_qt[49] = 99;

    this->std_chrominance_qt[50] = 99;
    this->std_chrominance_qt[51] = 99;
    this->std_chrominance_qt[52] = 99;
    this->std_chrominance_qt[53] = 99;
    this->std_chrominance_qt[54] = 99;
    this->std_chrominance_qt[55] = 99;
    this->std_chrominance_qt[56] = 99;
    this->std_chrominance_qt[57] = 99;
    this->std_chrominance_qt[58] = 99;
    this->std_chrominance_qt[59] = 99;

    this->std_chrominance_qt[60] = 99;
    this->std_chrominance_qt[61] = 99;
    this->std_chrominance_qt[62] = 99;
    this->std_chrominance_qt[63] = 99;
}

void BmpToJpg::init_std_dc_luminance_nrcodes()
{
//    static BYTE std_dc_luminance_nrcodes[17]={0,0,1,5,1,1,1,1,1,1,0,0,0,0,0,0,0};
    for(int i = 0; i < 17; i++)
    {
        this->std_dc_luminance_nrcodes[i] = 0;
    }

    this->std_dc_luminance_nrcodes[2] = 1;
    this->std_dc_luminance_nrcodes[3] = 5;

    for(int i = 4; i < 10; i++)
    {
        this->std_dc_luminance_nrcodes[i] = 1;
    }
}

void BmpToJpg::init_std_dc_luminance_values()
{
//    static BYTE std_dc_luminance_values[12]={0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
    for(int i = 0; i < 12; i++)
    {
        this->std_dc_luminance_values[i] = i;
    }
}

void BmpToJpg::init_std_dc_chrominance_nrcodes()
{
//    static BYTE std_dc_chrominance_nrcodes[17]={0,0,3,1,1,1,1,1,1,1,1,1,0,0,0,0,0};
    for(int i = 0; i < 17; i++)
    {
        this->std_dc_chrominance_nrcodes[i] = 1;
    }

    this->std_dc_chrominance_nrcodes[0] = 0;
    this->std_dc_chrominance_nrcodes[1] = 0;
    this->std_dc_chrominance_nrcodes[2] = 3;

    for(int i = 12; i < 17; i++)
    {
        this->std_dc_chrominance_nrcodes[i] = 0;
    }
}

void BmpToJpg::init_std_dc_chrominance_values()
{
//    static BYTE std_dc_chrominance_values[12]={0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
    for(int i = 0; i < 12; i++)
    {
        this->std_dc_chrominance_values[i] = i;
    }
}

void BmpToJpg::init_std_ac_luminance_nrcodes()
{
//    static BYTE std_ac_luminance_nrcodes[17]={0,0,2,1,
//    3,3,2,4,
//    3,5,5,4,4,
//    0,0,1,0x7d };

    this->std_ac_luminance_nrcodes[0] = 0;
    this->std_ac_luminance_nrcodes[1] = 0;
    this->std_ac_luminance_nrcodes[2] = 2;
    this->std_ac_luminance_nrcodes[3] = 1;

    this->std_ac_luminance_nrcodes[4] = 3;
    this->std_ac_luminance_nrcodes[5] = 3;
    this->std_ac_luminance_nrcodes[6] = 2;
    this->std_ac_luminance_nrcodes[7] = 4;

    this->std_ac_luminance_nrcodes[8] = 3;
    this->std_ac_luminance_nrcodes[9] = 5;
    this->std_ac_luminance_nrcodes[10] = 5;
    this->std_ac_luminance_nrcodes[11] = 4;
    this->std_ac_luminance_nrcodes[12] = 4;

    this->std_ac_luminance_nrcodes[13] = 0;
    this->std_ac_luminance_nrcodes[14] = 0;
    this->std_ac_luminance_nrcodes[15] = 1;
    this->std_ac_luminance_nrcodes[16] = 0x7d;
}

void BmpToJpg::init_std_ac_luminance_values()
{
//    static BYTE std_ac_luminance_values[162]= {
//          0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12,
//          0x21, 0x31, 0x41, 0x06, 0x13, 0x51, 0x61, 0x07,
//          0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xa1, 0x08,
//          0x23, 0x42, 0xb1, 0xc1, 0x15, 0x52, 0xd1, 0xf0,
//          0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0a, 0x16,
//          0x17, 0x18, 0x19, 0x1a, 0x25, 0x26, 0x27, 0x28,
//          0x29, 0x2a, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
//          0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
//          0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
//          0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
//          0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,
//          0x7a, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
//          0x8a, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98,
//          0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
//          0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6,
//          0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3, 0xc4, 0xc5,
//          0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4,
//          0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xe1, 0xe2,
//          0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea,
//          0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
    //          0xf9, 0xfa };
    this->std_ac_luminance_values[0] = 0x01;
    this->std_ac_luminance_values[1] = 0x02;
    this->std_ac_luminance_values[2] = 0x03;
    this->std_ac_luminance_values[3] = 0x00;
    this->std_ac_luminance_values[4] = 0x04;
    this->std_ac_luminance_values[5] = 0x11;
    this->std_ac_luminance_values[6] = 0x05;
    this->std_ac_luminance_values[7] = 0x12;
    this->std_ac_luminance_values[8] = 0x21;
    this->std_ac_luminance_values[9] = 0x31;

    this->std_ac_luminance_values[10] = 0x41;
    this->std_ac_luminance_values[11] = 0x06;
    this->std_ac_luminance_values[12] = 0x13;
    this->std_ac_luminance_values[13] = 0x51;
    this->std_ac_luminance_values[14] = 0x61;
    this->std_ac_luminance_values[15] = 0x07;
    this->std_ac_luminance_values[16] = 0x22;
    this->std_ac_luminance_values[17] = 0x71;
    this->std_ac_luminance_values[18] = 0x14;
    this->std_ac_luminance_values[19] = 0x32;

    this->std_ac_luminance_values[20] = 0x81;
    this->std_ac_luminance_values[21] = 0x91;
    this->std_ac_luminance_values[22] = 0xa1;
    this->std_ac_luminance_values[23] = 0x08;
    this->std_ac_luminance_values[24] = 0x23;
    this->std_ac_luminance_values[25] = 0x42;
    this->std_ac_luminance_values[26] = 0xb1;
    this->std_ac_luminance_values[27] = 0xc1;
    this->std_ac_luminance_values[28] = 0x15;
    this->std_ac_luminance_values[29] = 0x52;

    this->std_ac_luminance_values[30] = 0xd1;
    this->std_ac_luminance_values[31] = 0xf0;
    this->std_ac_luminance_values[32] = 0x24;
    this->std_ac_luminance_values[33] = 0x33;
    this->std_ac_luminance_values[34] = 0x62;
    this->std_ac_luminance_values[35] = 0x72;
    this->std_ac_luminance_values[36] = 0x82;
    this->std_ac_luminance_values[37] = 0x09;
    this->std_ac_luminance_values[38] = 0x0a;
    this->std_ac_luminance_values[39] = 0x16;

    this->std_ac_luminance_values[40] = 0x17;
    this->std_ac_luminance_values[41] = 0x18;
    this->std_ac_luminance_values[42] = 0x19;
    this->std_ac_luminance_values[43] = 0x1a;
    this->std_ac_luminance_values[44] = 0x25;
    this->std_ac_luminance_values[45] = 0x26;
    this->std_ac_luminance_values[46] = 0x27;
    this->std_ac_luminance_values[47] = 0x28;
    this->std_ac_luminance_values[48] = 0x29;
    this->std_ac_luminance_values[49] = 0x2a;

    this->std_ac_luminance_values[50] = 0x34;
    this->std_ac_luminance_values[51] = 0x35;
    this->std_ac_luminance_values[52] = 0x36;
    this->std_ac_luminance_values[53] = 0x37;
    this->std_ac_luminance_values[54] = 0x38;
    this->std_ac_luminance_values[55] = 0x39;
    this->std_ac_luminance_values[56] = 0x3a;
    this->std_ac_luminance_values[57] = 0x43;
    this->std_ac_luminance_values[58] = 0x44;
    this->std_ac_luminance_values[59] = 0x45;

    this->std_ac_luminance_values[60] = 0x46;
    this->std_ac_luminance_values[61] = 0x47;
    this->std_ac_luminance_values[62] = 0x48;
    this->std_ac_luminance_values[63] = 0x49;
    this->std_ac_luminance_values[64] = 0x4a;
    this->std_ac_luminance_values[65] = 0x53;
    this->std_ac_luminance_values[66] = 0x54;
    this->std_ac_luminance_values[67] = 0x55;
    this->std_ac_luminance_values[68] = 0x56;
    this->std_ac_luminance_values[69] = 0x57;

    this->std_ac_luminance_values[70] = 0x58;
    this->std_ac_luminance_values[71] = 0x59;
    this->std_ac_luminance_values[72] = 0x5a;
    this->std_ac_luminance_values[73] = 0x63;
    this->std_ac_luminance_values[74] = 0x64;
    this->std_ac_luminance_values[75] = 0x65;
    this->std_ac_luminance_values[76] = 0x66;
    this->std_ac_luminance_values[77] = 0x67;
    this->std_ac_luminance_values[78] = 0x68;
    this->std_ac_luminance_values[79] = 0x69;

    this->std_ac_luminance_values[80] = 0x6a;
    this->std_ac_luminance_values[81] = 0x73;
    this->std_ac_luminance_values[82] = 0x74;
    this->std_ac_luminance_values[83] = 0x75;
    this->std_ac_luminance_values[84] = 0x76;
    this->std_ac_luminance_values[85] = 0x77;
    this->std_ac_luminance_values[86] = 0x78;
    this->std_ac_luminance_values[87] = 0x79;
    this->std_ac_luminance_values[88] = 0x7a;
    this->std_ac_luminance_values[89] = 0x83;

    this->std_ac_luminance_values[90] = 0x84;
    this->std_ac_luminance_values[91] = 0x85;
    this->std_ac_luminance_values[92] = 0x86;
    this->std_ac_luminance_values[93] = 0x87;
    this->std_ac_luminance_values[94] = 0x88;
    this->std_ac_luminance_values[95] = 0x89;
    this->std_ac_luminance_values[96] = 0x8a;
    this->std_ac_luminance_values[97] = 0x92;
    this->std_ac_luminance_values[98] = 0x93;
    this->std_ac_luminance_values[99] = 0x94;

    this->std_ac_luminance_values[100] = 0x95;
    this->std_ac_luminance_values[101] = 0x96;
    this->std_ac_luminance_values[102] = 0x97;
    this->std_ac_luminance_values[103] = 0x98;
    this->std_ac_luminance_values[104] = 0x99;
    this->std_ac_luminance_values[105] = 0x9a;
    this->std_ac_luminance_values[106] = 0xa2;
    this->std_ac_luminance_values[107] = 0xa3;
    this->std_ac_luminance_values[108] = 0xa4;
    this->std_ac_luminance_values[109] = 0xa5;

    this->std_ac_luminance_values[110] = 0xa6;
    this->std_ac_luminance_values[111] = 0xa7;
    this->std_ac_luminance_values[112] = 0xa8;
    this->std_ac_luminance_values[113] = 0xa9;
    this->std_ac_luminance_values[114] = 0xaa;
    this->std_ac_luminance_values[115] = 0xb2;
    this->std_ac_luminance_values[116] = 0xb3;
    this->std_ac_luminance_values[117] = 0xb4;
    this->std_ac_luminance_values[118] = 0xb5;
    this->std_ac_luminance_values[119] = 0xb6;

    this->std_ac_luminance_values[120] = 0xb7;
    this->std_ac_luminance_values[121] = 0xb8;
    this->std_ac_luminance_values[122] = 0xb9;
    this->std_ac_luminance_values[123] = 0xba;
    this->std_ac_luminance_values[124] = 0xc2;
    this->std_ac_luminance_values[125] = 0xc3;
    this->std_ac_luminance_values[126] = 0xc4;
    this->std_ac_luminance_values[127] = 0xc5;
    this->std_ac_luminance_values[128] = 0xc6;
    this->std_ac_luminance_values[129] = 0xc7;

    this->std_ac_luminance_values[130] = 0xc8;
    this->std_ac_luminance_values[131] = 0xc9;
    this->std_ac_luminance_values[132] = 0xca;
    this->std_ac_luminance_values[133] = 0xd2;
    this->std_ac_luminance_values[134] = 0xd3;
    this->std_ac_luminance_values[135] = 0xd4;
    this->std_ac_luminance_values[136] = 0xd5;
    this->std_ac_luminance_values[137] = 0xd6;
    this->std_ac_luminance_values[138] = 0xd7;
    this->std_ac_luminance_values[139] = 0xd8;

    this->std_ac_luminance_values[140] = 0xd9;
    this->std_ac_luminance_values[141] = 0xda;
    this->std_ac_luminance_values[142] = 0xe1;
    this->std_ac_luminance_values[143] = 0xe2;
    this->std_ac_luminance_values[144] = 0xe3;
    this->std_ac_luminance_values[145] = 0xe4;
    this->std_ac_luminance_values[146] = 0xe5;
    this->std_ac_luminance_values[147] = 0xe6;
    this->std_ac_luminance_values[148] = 0xe7;
    this->std_ac_luminance_values[149] = 0xe8;

    this->std_ac_luminance_values[150] = 0xe9;
    this->std_ac_luminance_values[151] = 0xea;
    this->std_ac_luminance_values[152] = 0xf1;
    this->std_ac_luminance_values[153] = 0xf2;
    this->std_ac_luminance_values[154] = 0xf3;
    this->std_ac_luminance_values[155] = 0xf4;
    this->std_ac_luminance_values[156] = 0xf5;
    this->std_ac_luminance_values[157] = 0xf6;
    this->std_ac_luminance_values[158] = 0xf7;
    this->std_ac_luminance_values[159] = 0xf8;

    this->std_ac_luminance_values[160] = 0xf9;
    this->std_ac_luminance_values[161] = 0xfa;

}

void BmpToJpg::init_std_ac_chrominance_nrcodes()
{
//    static BYTE std_ac_chrominance_nrcodes[17]={0,0,2,1,2,4,4,3,4,7,5,4,4,0,1,2,0x77};
    this->std_ac_chrominance_nrcodes[0] = 0;
    this->std_ac_chrominance_nrcodes[1] = 0;
    this->std_ac_chrominance_nrcodes[2] = 2;
    this->std_ac_chrominance_nrcodes[3] = 1;

    this->std_ac_chrominance_nrcodes[4] = 2;
    this->std_ac_chrominance_nrcodes[5] = 4;
    this->std_ac_chrominance_nrcodes[6] = 4;
    this->std_ac_chrominance_nrcodes[7] = 3;

    this->std_ac_chrominance_nrcodes[8] = 4;
    this->std_ac_chrominance_nrcodes[9] = 7;
    this->std_ac_chrominance_nrcodes[10] = 5;
    this->std_ac_chrominance_nrcodes[11] = 4;
    this->std_ac_chrominance_nrcodes[12] = 4;

    this->std_ac_chrominance_nrcodes[13] = 0;
    this->std_ac_chrominance_nrcodes[14] = 1;
    this->std_ac_chrominance_nrcodes[15] = 2;
    this->std_ac_chrominance_nrcodes[16] = 0x77;
}

void BmpToJpg::init_std_ac_chrominance_values()
{
//    static BYTE std_ac_chrominance_values[162]={
//          0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21,
//          0x31, 0x06, 0x12, 0x41, 0x51, 0x07, 0x61, 0x71,
//          0x13, 0x22, 0x32, 0x81, 0x08, 0x14, 0x42, 0x91,
//          0xa1, 0xb1, 0xc1, 0x09, 0x23, 0x33, 0x52, 0xf0,
//          0x15, 0x62, 0x72, 0xd1, 0x0a, 0x16, 0x24, 0x34,
//          0xe1, 0x25, 0xf1, 0x17, 0x18, 0x19, 0x1a, 0x26,
//          0x27, 0x28, 0x29, 0x2a, 0x35, 0x36, 0x37, 0x38,
//          0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
//          0x49, 0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
//          0x59, 0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
//          0x69, 0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
//          0x79, 0x7a, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
//          0x88, 0x89, 0x8a, 0x92, 0x93, 0x94, 0x95, 0x96,
//          0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5,
//          0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4,
//          0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3,
//          0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2,
//          0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda,
//          0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9,
//          0xea, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
    //          0xf9, 0xfa };
    this->std_ac_chrominance_values[0] = 0x00;
    this->std_ac_chrominance_values[1] = 0x01;
    this->std_ac_chrominance_values[2] = 0x02;
    this->std_ac_chrominance_values[3] = 0x03;
    this->std_ac_chrominance_values[4] = 0x11;
    this->std_ac_chrominance_values[5] = 0x04;
    this->std_ac_chrominance_values[6] = 0x05;
    this->std_ac_chrominance_values[7] = 0x21;
    this->std_ac_chrominance_values[8] = 0x31;
    this->std_ac_chrominance_values[9] = 0x06;

    this->std_ac_chrominance_values[10] = 0x12;
    this->std_ac_chrominance_values[11] = 0x41;
    this->std_ac_chrominance_values[12] = 0x51;
    this->std_ac_chrominance_values[13] = 0x07;
    this->std_ac_chrominance_values[14] = 0x61;
    this->std_ac_chrominance_values[15] = 0x71;
    this->std_ac_chrominance_values[16] = 0x13;
    this->std_ac_chrominance_values[17] = 0x22;
    this->std_ac_chrominance_values[18] = 0x32;
    this->std_ac_chrominance_values[19] = 0x81;

    this->std_ac_chrominance_values[20] = 0x08;
    this->std_ac_chrominance_values[21] = 0x14;
    this->std_ac_chrominance_values[22] = 0x42;
    this->std_ac_chrominance_values[23] = 0x91;
    this->std_ac_chrominance_values[24] = 0xa1;
    this->std_ac_chrominance_values[25] = 0xb1;
    this->std_ac_chrominance_values[26] = 0xc1;
    this->std_ac_chrominance_values[27] = 0x09;
    this->std_ac_chrominance_values[28] = 0x23;
    this->std_ac_chrominance_values[29] = 0x33;

    this->std_ac_chrominance_values[30] = 0x52;
    this->std_ac_chrominance_values[31] = 0xf0;
    this->std_ac_chrominance_values[32] = 0x15;
    this->std_ac_chrominance_values[33] = 0x62;
    this->std_ac_chrominance_values[34] = 0x72;
    this->std_ac_chrominance_values[35] = 0xd1;
    this->std_ac_chrominance_values[36] = 0x0a;
    this->std_ac_chrominance_values[37] = 0x16;
    this->std_ac_chrominance_values[38] = 0x24;
    this->std_ac_chrominance_values[39] = 0x34;

    this->std_ac_chrominance_values[40] = 0xe1;
    this->std_ac_chrominance_values[41] = 0x25;
    this->std_ac_chrominance_values[42] = 0xf1;
    this->std_ac_chrominance_values[43] = 0x17;
    this->std_ac_chrominance_values[44] = 0x18;
    this->std_ac_chrominance_values[45] = 0x19;
    this->std_ac_chrominance_values[46] = 0x1a;
    this->std_ac_chrominance_values[47] = 0x26;
    this->std_ac_chrominance_values[48] = 0x27;
    this->std_ac_chrominance_values[49] = 0x28;

    this->std_ac_chrominance_values[50] = 0x29;
    this->std_ac_chrominance_values[51] = 0x2a;
    this->std_ac_chrominance_values[52] = 0x35;
    this->std_ac_chrominance_values[53] = 0x36;
    this->std_ac_chrominance_values[54] = 0x37;
    this->std_ac_chrominance_values[55] = 0x38;
    this->std_ac_chrominance_values[56] = 0x39;
    this->std_ac_chrominance_values[57] = 0x3a;
    this->std_ac_chrominance_values[58] = 0x43;
    this->std_ac_chrominance_values[59] = 0x44;

    this->std_ac_chrominance_values[60] = 0x45;
    this->std_ac_chrominance_values[61] = 0x46;
    this->std_ac_chrominance_values[62] = 0x47;
    this->std_ac_chrominance_values[63] = 0x48;
    this->std_ac_chrominance_values[64] = 0x49;
    this->std_ac_chrominance_values[65] = 0x4a;
    this->std_ac_chrominance_values[66] = 0x53;
    this->std_ac_chrominance_values[67] = 0x54;
    this->std_ac_chrominance_values[68] = 0x55;
    this->std_ac_chrominance_values[69] = 0x56;

    this->std_ac_chrominance_values[70] = 0x57;
    this->std_ac_chrominance_values[71] = 0x58;
    this->std_ac_chrominance_values[72] = 0x59;
    this->std_ac_chrominance_values[73] = 0x5a;
    this->std_ac_chrominance_values[74] = 0x63;
    this->std_ac_chrominance_values[75] = 0x64;
    this->std_ac_chrominance_values[76] = 0x65;
    this->std_ac_chrominance_values[77] = 0x66;
    this->std_ac_chrominance_values[78] = 0x67;
    this->std_ac_chrominance_values[79] = 0x68;

    this->std_ac_chrominance_values[80] = 0x69;
    this->std_ac_chrominance_values[81] = 0x6a;
    this->std_ac_chrominance_values[82] = 0x73;
    this->std_ac_chrominance_values[83] = 0x74;
    this->std_ac_chrominance_values[84] = 0x75;
    this->std_ac_chrominance_values[85] = 0x76;
    this->std_ac_chrominance_values[86] = 0x77;
    this->std_ac_chrominance_values[87] = 0x78;
    this->std_ac_chrominance_values[88] = 0x79;
    this->std_ac_chrominance_values[89] = 0x7a;

    this->std_ac_chrominance_values[90] = 0x82;
    this->std_ac_chrominance_values[91] = 0x83;
    this->std_ac_chrominance_values[92] = 0x84;
    this->std_ac_chrominance_values[93] = 0x85;
    this->std_ac_chrominance_values[94] = 0x86;
    this->std_ac_chrominance_values[95] = 0x87;
    this->std_ac_chrominance_values[96] = 0x88;
    this->std_ac_chrominance_values[97] = 0x89;
    this->std_ac_chrominance_values[98] = 0x8a;
    this->std_ac_chrominance_values[99] = 0x92;

    this->std_ac_chrominance_values[100] = 0x93;
    this->std_ac_chrominance_values[101] = 0x94;
    this->std_ac_chrominance_values[102] = 0x95;
    this->std_ac_chrominance_values[103] = 0x96;
    this->std_ac_chrominance_values[104] = 0x97;
    this->std_ac_chrominance_values[105] = 0x98;
    this->std_ac_chrominance_values[106] = 0x99;
    this->std_ac_chrominance_values[107] = 0x9a;
    this->std_ac_chrominance_values[108] = 0xa2;
    this->std_ac_chrominance_values[109] = 0xa3;
    this->std_ac_chrominance_values[110] = 0xa4;

    this->std_ac_chrominance_values[111] = 0xa5;
    this->std_ac_chrominance_values[112] = 0xa6;
    this->std_ac_chrominance_values[113] = 0xa7;
    this->std_ac_chrominance_values[114] = 0xa8;
    this->std_ac_chrominance_values[115] = 0xa9;
    this->std_ac_chrominance_values[116] = 0xaa;
    this->std_ac_chrominance_values[117] = 0xb2;
    this->std_ac_chrominance_values[118] = 0xb3;
    this->std_ac_chrominance_values[119] = 0xb4;

    this->std_ac_chrominance_values[120] = 0xb5;
    this->std_ac_chrominance_values[121] = 0xb6;
    this->std_ac_chrominance_values[122] = 0xb7;
    this->std_ac_chrominance_values[123] = 0xb8;
    this->std_ac_chrominance_values[124] = 0xb9;
    this->std_ac_chrominance_values[125] = 0xba;
    this->std_ac_chrominance_values[126] = 0xc2;
    this->std_ac_chrominance_values[127] = 0xc3;
    this->std_ac_chrominance_values[128] = 0xc4;
    this->std_ac_chrominance_values[129] = 0xc5;

    this->std_ac_chrominance_values[130] = 0xc6;
    this->std_ac_chrominance_values[131] = 0xc7;
    this->std_ac_chrominance_values[132] = 0xc8;
    this->std_ac_chrominance_values[133] = 0xc9;
    this->std_ac_chrominance_values[134] = 0xca;
    this->std_ac_chrominance_values[135] = 0xd2;
    this->std_ac_chrominance_values[136] = 0xd3;
    this->std_ac_chrominance_values[137] = 0xd4;
    this->std_ac_chrominance_values[138] = 0xd5;
    this->std_ac_chrominance_values[139] = 0xd6;

    this->std_ac_chrominance_values[140] = 0xd7;
    this->std_ac_chrominance_values[141] = 0xd8;
    this->std_ac_chrominance_values[142] = 0xd9;
    this->std_ac_chrominance_values[143] = 0xda;
    this->std_ac_chrominance_values[144] = 0xe2;
    this->std_ac_chrominance_values[145] = 0xe3;
    this->std_ac_chrominance_values[146] = 0xe4;
    this->std_ac_chrominance_values[147] = 0xe5;
    this->std_ac_chrominance_values[148] = 0xe6;
    this->std_ac_chrominance_values[149] = 0xe7;

    this->std_ac_chrominance_values[150] = 0xe8;
    this->std_ac_chrominance_values[151] = 0xe9;
    this->std_ac_chrominance_values[152] = 0xea;
    this->std_ac_chrominance_values[153] = 0xf2;
    this->std_ac_chrominance_values[154] = 0xf3;
    this->std_ac_chrominance_values[155] = 0xf4;
    this->std_ac_chrominance_values[156] = 0xf5;
    this->std_ac_chrominance_values[157] = 0xf6;
    this->std_ac_chrominance_values[158] = 0xf7;
    this->std_ac_chrominance_values[159] = 0xf8;

    this->std_ac_chrominance_values[160] = 0xf9;
    this->std_ac_chrominance_values[161] = 0xfa;

}

void BmpToJpg::init_mask()
{
//    static WORD mask[16]={1,2,4,8,16,32,64,128,256,512,1024,2048,4096,8192,16384,32768};
    this->mask[0] = 1;
    this->mask[1] = 2;
    this->mask[2] = 4;
    this->mask[3] = 8;

    this->mask[4] = 16;
    this->mask[5] = 32;
    this->mask[6] = 64;
    this->mask[7] = 128;

    this->mask[8] = 256;
    this->mask[9] = 512;
    this->mask[10] = 1024;
    this->mask[11] = 2048;

    this->mask[12] = 4096;
    this->mask[13] = 8192;
    this->mask[14] = 16384;
    this->mask[15] = 32768;
}

void BmpToJpg::init_all()
{
    set_DQTinfo();
    set_DHTinfo();
    init_Huffman_tables();
    set_numbers_category_and_bitcode();
    precalculate_YCbCr_tables();
    prepare_quant_tables();
}

void BmpToJpg::write_APP0info()
//Nothing to overwrite for APP0info
{
    writeword(APP0info.marker);
    writeword(APP0info.length);
    writebyte('J');
    writebyte('F');
    writebyte('I');
    writebyte('F');
    writebyte(0);

    writebyte(APP0info.versionhi);
    writebyte(APP0info.versionlo);
    writebyte(APP0info.xyunits);
    writeword(APP0info.xdensity);
    writeword(APP0info.ydensity);
    writebyte(APP0info.thumbnwidth);
    writebyte(APP0info.thumbnheight);
}

void BmpToJpg::write_SOF0info()
// We should overwrite width and height
{
    writeword(SOF0info.marker);
    writeword(SOF0info.length);
    writebyte(SOF0info.precision);
    writeword(SOF0info.height);writeword(SOF0info.width);
    writebyte(SOF0info.nrofcomponents);
    writebyte(SOF0info.IdY);writebyte(SOF0info.HVY);writebyte(SOF0info.QTY);
    writebyte(SOF0info.IdCb);writebyte(SOF0info.HVCb);writebyte(SOF0info.QTCb);
    writebyte(SOF0info.IdCr);writebyte(SOF0info.HVCr);writebyte(SOF0info.QTCr);
}

void BmpToJpg::write_DQTinfo()
{
    BYTE i;
    writeword(DQTinfo.marker);
    writeword(DQTinfo.length);
    writebyte(DQTinfo.QTYinfo);
    for (i=0;i<64;i++)
        writebyte(DQTinfo.Ytable[i]);
    writebyte(DQTinfo.QTCbinfo);for (i=0;i<64;i++) writebyte(DQTinfo.Cbtable[i]);
}

void BmpToJpg::set_quant_table(BYTE *basic_table,BYTE scale_factor,BYTE *newtable)
// Set quantization table and zigzag reorder it
{
    BYTE i;
    long temp;
    for (i = 0; i < 64; i++) {
        temp = ((long) basic_table[i] * scale_factor + 50L) / 100L;
        /* limit the values to the valid range */
        if (temp <= 0L) temp = 1L;
        if (temp > 255L) temp = 255L; /* limit to baseline range if requested */
        newtable[zigzag[i]] = (WORD) temp;
              }
}

void BmpToJpg::set_DQTinfo()
{
    BYTE scalefactor=50;// scalefactor controls the visual quality of the image
    // the smaller is, the better image we'll get, and the smaller
    // compression we'll achieve
    DQTinfo.marker=0xFFDB;
    DQTinfo.length=132;
    DQTinfo.QTYinfo=0;
    DQTinfo.QTCbinfo=1;
    set_quant_table(std_luminance_qt,scalefactor,DQTinfo.Ytable);
    set_quant_table(std_chrominance_qt,scalefactor,DQTinfo.Cbtable);
}

void BmpToJpg::write_DHTinfo()
{
    BYTE i;
    writeword(DHTinfo.marker);
    writeword(DHTinfo.length);
    writebyte(DHTinfo.HTYDCinfo);
    for (i=0;i<16;i++)  writebyte(DHTinfo.YDC_nrcodes[i]);
    for (i=0;i<=11;i++) writebyte(DHTinfo.YDC_values[i]);
    writebyte(DHTinfo.HTYACinfo);
    for (i=0;i<16;i++)  writebyte(DHTinfo.YAC_nrcodes[i]);
    for (i=0;i<=161;i++) writebyte(DHTinfo.YAC_values[i]);
    writebyte(DHTinfo.HTCbDCinfo);
    for (i=0;i<16;i++)  writebyte(DHTinfo.CbDC_nrcodes[i]);
    for (i=0;i<=11;i++)  writebyte(DHTinfo.CbDC_values[i]);
    writebyte(DHTinfo.HTCbACinfo);
    for (i=0;i<16;i++)  writebyte(DHTinfo.CbAC_nrcodes[i]);
    for (i=0;i<=161;i++) writebyte(DHTinfo.CbAC_values[i]);
}

void BmpToJpg::set_DHTinfo()
{
    BYTE i;
    DHTinfo.marker=0xFFC4;
    DHTinfo.length=0x01A2;
    DHTinfo.HTYDCinfo=0;
    for (i=0;i<16;i++)  DHTinfo.YDC_nrcodes[i]=std_dc_luminance_nrcodes[i+1];
    for (i=0;i<=11;i++)  DHTinfo.YDC_values[i]=std_dc_luminance_values[i];
    DHTinfo.HTYACinfo=0x10;
    for (i=0;i<16;i++)  DHTinfo.YAC_nrcodes[i]=std_ac_luminance_nrcodes[i+1];
    for (i=0;i<=161;i++) DHTinfo.YAC_values[i]=std_ac_luminance_values[i];
    DHTinfo.HTCbDCinfo=1;
    for (i=0;i<16;i++)  DHTinfo.CbDC_nrcodes[i]=std_dc_chrominance_nrcodes[i+1];
    for (i=0;i<=11;i++)  DHTinfo.CbDC_values[i]=std_dc_chrominance_values[i];
    DHTinfo.HTCbACinfo=0x11;
    for (i=0;i<16;i++)  DHTinfo.CbAC_nrcodes[i]=std_ac_chrominance_nrcodes[i+1];
    for (i=0;i<=161;i++) DHTinfo.CbAC_values[i]=std_ac_chrominance_values[i];
}

void BmpToJpg::write_SOSinfo()
//Nothing to overwrite for SOSinfo
{
    writeword(SOSinfo.marker);
    writeword(SOSinfo.length);
    writebyte(SOSinfo.nrofcomponents);
    writebyte(SOSinfo.IdY);writebyte(SOSinfo.HTY);
    writebyte(SOSinfo.IdCb);writebyte(SOSinfo.HTCb);
    writebyte(SOSinfo.IdCr);writebyte(SOSinfo.HTCr);
    writebyte(SOSinfo.Ss);writebyte(SOSinfo.Se);writebyte(SOSinfo.Bf);
}

void BmpToJpg::write_comment(BYTE *comment)
{
    WORD i,length;
    writeword(0xFFFE); //The COM marker
    length=strlen((const char *)comment);
    writeword(length+2);
    for (i=0;i<length;i++) writebyte(comment[i]);
}

void BmpToJpg::writebits(bitstring bs)
// A portable version; it should be done in assembler
{
    WORD value;
    SBYTE posval;//bit position in the bitstring we read, should be<=15 and >=0
    value=bs.value;
    posval=bs.length-1;
    while (posval>=0)
    {
        if (value & mask[posval]) bytenew|=mask[bytepos];
        posval--;bytepos--;
        if (bytepos<0) { if (bytenew==0xFF) {writebyte(0xFF);writebyte(0);}
            else {writebyte(bytenew);}
            bytepos=7;bytenew=0;
        }
    }
}

void BmpToJpg::compute_Huffman_table(BYTE *nrcodes,BYTE *std_table,bitstring *HT)
{
    BYTE k,j;
    BYTE pos_in_table;
    WORD codevalue;
    codevalue=0; pos_in_table=0;
    for (k=1;k<=16;k++)
    {
        for (j=1;j<=nrcodes[k];j++) {HT[std_table[pos_in_table]].value=codevalue;
            HT[std_table[pos_in_table]].length=k;
            pos_in_table++;
            codevalue++;
        }
        codevalue*=2;
    }
}
void BmpToJpg::init_Huffman_tables()
{
    compute_Huffman_table(std_dc_luminance_nrcodes,std_dc_luminance_values,YDC_HT);
    compute_Huffman_table(std_dc_chrominance_nrcodes,std_dc_chrominance_values,CbDC_HT);
    compute_Huffman_table(std_ac_luminance_nrcodes,std_ac_luminance_values,YAC_HT);
    compute_Huffman_table(std_ac_chrominance_nrcodes,std_ac_chrominance_values,CbAC_HT);
}

void BmpToJpg::exitmessage(char *error_message)
{
    printf("%s\n",error_message);exit(EXIT_FAILURE);
}

void BmpToJpg::set_numbers_category_and_bitcode()
{
    SDWORD nr;
    SDWORD nrlower,nrupper;
    BYTE cat,value;

    category_alloc=(BYTE *)malloc(65535*sizeof(BYTE));
    if (category_alloc==NULL) exitmessage("Not enough memory.");
    category=category_alloc+32767; //allow negative subscripts
    bitcode_alloc=(bitstring *)malloc(65535*sizeof(bitstring));
    if (bitcode_alloc==NULL) exitmessage("Not enough memory.");
    bitcode=bitcode_alloc+32767;
    nrlower=1;nrupper=2;
    for (cat=1;cat<=15;cat++) {
        //Positive numbers
        for (nr=nrlower;nr<nrupper;nr++)
        { category[nr]=cat;
            bitcode[nr].length=cat;
            bitcode[nr].value=(WORD)nr;
        }
        //Negative numbers
        for (nr=-(nrupper-1);nr<=-nrlower;nr++)
        { category[nr]=cat;
            bitcode[nr].length=cat;
            bitcode[nr].value=(WORD)(nrupper-1+nr);
        }
        nrlower<<=1;
        nrupper<<=1;
    }
}

void BmpToJpg::precalculate_YCbCr_tables()//RGB转YCbCr色彩空间，转换公式
{
    WORD R,G,B;
    for (R=0;R<=255;R++) {YRtab[R]=(SDWORD)(65536*0.299+0.5)*R;
        CbRtab[R]=(SDWORD)(65536*-0.16874+0.5)*R;
        CrRtab[R]=(SDWORD)(32768)*R;
    }
    for (G=0;G<=255;G++) {YGtab[G]=(SDWORD)(65536*0.587+0.5)*G;
        CbGtab[G]=(SDWORD)(65536*-0.33126+0.5)*G;
        CrGtab[G]=(SDWORD)(65536*-0.41869+0.5)*G;
    }
    for (B=0;B<=255;B++) {YBtab[B]=(SDWORD)(65536*0.114+0.5)*B;
        CbBtab[B]=(SDWORD)(32768)*B;
        CrBtab[B]=(SDWORD)(65536*-0.08131+0.5)*B;
    }
}


// Using a bit modified form of the FDCT routine from IJG's C source:
// Forward DCT routine idea taken from Independent JPEG Group's C source for
// JPEG encoders/decoders

/* For float AA&N IDCT method, divisors are equal to quantization
   coefficients scaled by scalefactor[row]*scalefactor[col], where
   scalefactor[0] = 1
   scalefactor[k] = cos(k*PI/16) * sqrt(2)    for k=1..7
   We apply a further scale factor of 8.
   What's actually stored is 1/divisor so that the inner loop can
   use a multiplication rather than a division. */
void BmpToJpg::prepare_quant_tables()
{
    double aanscalefactor[8] = {1.0, 1.387039845, 1.306562965, 1.175875602,
                                1.0, 0.785694958, 0.541196100, 0.275899379};
    BYTE row, col;
    BYTE i = 0;
    for (row = 0; row < 8; row++)
    {
        for (col = 0; col < 8; col++)
        {
            fdtbl_Y[i] = (float) (1.0 / ((double) DQTinfo.Ytable[zigzag[i]] *
                                  aanscalefactor[row] * aanscalefactor[col] * 8.0));
            fdtbl_Cb[i] = (float) (1.0 / ((double) DQTinfo.Cbtable[zigzag[i]] *
                                   aanscalefactor[row] * aanscalefactor[col] * 8.0));

            i++;
        }
    }
}

void BmpToJpg::fdct_and_quantization(SBYTE *data,float *fdtbl,SWORD *outdata)
{
    float tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
    float tmp10, tmp11, tmp12, tmp13;
    float z1, z2, z3, z4, z5, z11, z13;
    float *dataptr;
    float datafloat[64];
    float temp;
    SBYTE ctr;
    BYTE i;
    for (i=0;i<64;i++) datafloat[i]=data[i];

    /* Pass 1: process rows. */
    dataptr=datafloat;
    for (ctr = 7; ctr >= 0; ctr--) {
        tmp0 = dataptr[0] + dataptr[7];
        tmp7 = dataptr[0] - dataptr[7];
        tmp1 = dataptr[1] + dataptr[6];
        tmp6 = dataptr[1] - dataptr[6];
        tmp2 = dataptr[2] + dataptr[5];
        tmp5 = dataptr[2] - dataptr[5];
        tmp3 = dataptr[3] + dataptr[4];
        tmp4 = dataptr[3] - dataptr[4];

        /* Even part */

        tmp10 = tmp0 + tmp3;    /* phase 2 */
        tmp13 = tmp0 - tmp3;
        tmp11 = tmp1 + tmp2;
        tmp12 = tmp1 - tmp2;

        dataptr[0] = tmp10 + tmp11; /* phase 3 */
        dataptr[4] = tmp10 - tmp11;

        z1 = (tmp12 + tmp13) * ((float) 0.707106781); /* c4 */
        dataptr[2] = tmp13 + z1;    /* phase 5 */
        dataptr[6] = tmp13 - z1;

        /* Odd part */

        tmp10 = tmp4 + tmp5;    /* phase 2 */
        tmp11 = tmp5 + tmp6;
        tmp12 = tmp6 + tmp7;

        /* The rotator is modified from fig 4-8 to avoid extra negations. */
        z5 = (tmp10 - tmp12) * ((float) 0.382683433); /* c6 */
        z2 = ((float) 0.541196100) * tmp10 + z5; /* c2-c6 */
        z4 = ((float) 1.306562965) * tmp12 + z5; /* c2+c6 */
        z3 = tmp11 * ((float) 0.707106781); /* c4 */

        z11 = tmp7 + z3;        /* phase 5 */
        z13 = tmp7 - z3;

        dataptr[5] = z13 + z2;    /* phase 6 */
        dataptr[3] = z13 - z2;
        dataptr[1] = z11 + z4;
        dataptr[7] = z11 - z4;

        dataptr += 8;        /* advance pointer to next row */
    }

    /* Pass 2: process columns. */

    dataptr = datafloat;
    for (ctr = 7; ctr >= 0; ctr--) {
        tmp0 = dataptr[0] + dataptr[56];
        tmp7 = dataptr[0] - dataptr[56];
        tmp1 = dataptr[8] + dataptr[48];
        tmp6 = dataptr[8] - dataptr[48];
        tmp2 = dataptr[16] + dataptr[40];
        tmp5 = dataptr[16] - dataptr[40];
        tmp3 = dataptr[24] + dataptr[32];
        tmp4 = dataptr[24] - dataptr[32];

        /* Even part */

        tmp10 = tmp0 + tmp3;    /* phase 2 */
        tmp13 = tmp0 - tmp3;
        tmp11 = tmp1 + tmp2;
        tmp12 = tmp1 - tmp2;

        dataptr[0] = tmp10 + tmp11; /* phase 3 */
        dataptr[32] = tmp10 - tmp11;

        z1 = (tmp12 + tmp13) * ((float) 0.707106781); /* c4 */
        dataptr[16] = tmp13 + z1; /* phase 5 */
        dataptr[48] = tmp13 - z1;

        /* Odd part */

        tmp10 = tmp4 + tmp5;    /* phase 2 */
        tmp11 = tmp5 + tmp6;
        tmp12 = tmp6 + tmp7;

        /* The rotator is modified from fig 4-8 to avoid extra negations. */
        z5 = (tmp10 - tmp12) * ((float) 0.382683433); /* c6 */
        z2 = ((float) 0.541196100) * tmp10 + z5; /* c2-c6 */
        z4 = ((float) 1.306562965) * tmp12 + z5; /* c2+c6 */
        z3 = tmp11 * ((float) 0.707106781); /* c4 */

        z11 = tmp7 + z3;        /* phase 5 */
        z13 = tmp7 - z3;

        dataptr[40] = z13 + z2; /* phase 6 */
        dataptr[24] = z13 - z2;
        dataptr[8] = z11 + z4;
        dataptr[56] = z11 - z4;

        dataptr++;            /* advance pointer to next column */
    }

    // Quantize/descale the coefficients, and store into output array
    for (i = 0; i < 64; i++) {
        /* Apply the quantization and scaling factor */
        temp = datafloat[i] * fdtbl[i];
        /* Round to nearest integer.
   Since C does not specify the direction of rounding for negative
   quotients, we have to force the dividend positive for portability.
   The maximum coefficient size is +-16K (for 12-bit data), so this
   code should work for either 16-bit or 32-bit ints.
*/
        outdata[i] = (SWORD) ((SWORD)(temp + 16384.5) - 16384);
    }
}

void BmpToJpg::process_DU(SBYTE *ComponentDU,float *fdtbl,SWORD *DC,
        bitstring *HTDC,bitstring *HTAC)
{
    bitstring EOB=HTAC[0x00];
    bitstring M16zeroes=HTAC[0xF0];
    BYTE i;
    BYTE startpos;
    BYTE end0pos;
    BYTE nrzeroes;
    BYTE nrmarker;
    SWORD Diff;

    fdct_and_quantization(ComponentDU,fdtbl,DU_DCT);
    //zigzag reorder
    for (i=0;i<=63;i++) DU[zigzag[i]]=DU_DCT[i];
    Diff=DU[0]-*DC;
    *DC=DU[0];
    //Encode DC
    if (Diff==0) writebits(HTDC[0]); //Diff might be 0
    else {writebits(HTDC[category[Diff]]);
        writebits(bitcode[Diff]);
    }
    //Encode ACs
    for (end0pos=63;(end0pos>0)&&(DU[end0pos]==0);end0pos--) ;
    //end0pos = first element in reverse order !=0
    if (end0pos==0) {writebits(EOB);return;}

    i=1;
    while (i<=end0pos)
    {
        startpos=i;
        for (; (DU[i]==0)&&(i<=end0pos);i++) ;
        nrzeroes=i-startpos;
        if (nrzeroes>=16) {
            for (nrmarker=1;nrmarker<=nrzeroes/16;nrmarker++) writebits(M16zeroes);
            nrzeroes=nrzeroes%16;
        }
        writebits(HTAC[nrzeroes*16+category[DU[i]]]);writebits(bitcode[DU[i]]);
        i++;
    }
    if (end0pos!=63) writebits(EOB);
}

void BmpToJpg::load_data_units_from_RGB_buffer(WORD xpos,WORD ypos)
{
    BYTE x,y;
    BYTE pos=0;
    DWORD location;
    BYTE R,G,B;
    location=ypos*Ximage+xpos;
    for (y=0;y<8;y++)
    {
        for (x=0;x<8;x++)
        {
            R=RGB_buffer[location].R;
            G=RGB_buffer[location].G;
            B=RGB_buffer[location].B;

            YDU[pos]=Y(R,G,B);
            CbDU[pos]=Cb(R,G,B);
            CrDU[pos]=Cr(R,G,B);

            location++;pos++;
        }
        location+=Ximage-8;
    }
}

void BmpToJpg::main_encoder()
{
    SWORD DCY=0,DCCb=0,DCCr=0; //DC coefficients used for differential encoding
    WORD xpos,ypos;
    for (ypos=0;ypos<Yimage;ypos+=8)
        for (xpos=0;xpos<Ximage;xpos+=8)
        {
            load_data_units_from_RGB_buffer(xpos,ypos);
            process_DU(YDU,fdtbl_Y,&DCY,YDC_HT,YAC_HT);
            process_DU(CbDU,fdtbl_Cb,&DCCb,CbDC_HT,CbAC_HT);
            process_DU(CrDU,fdtbl_Cb,&DCCr,CbDC_HT,CbAC_HT);
        }
}

void BmpToJpg::load_bitmap(char *bitmap_name, WORD *Ximage_original, WORD *Yimage_original)
{
    WORD Xdiv8,Ydiv8;
    BYTE nr_fillingbytes;//The number of the filling bytes in the BMP file
    // (the dimension in bytes of a BMP line on the disk is divisible by 4)
    colorRGB lastcolor;
    WORD column;
    BYTE TMPBUF[256];
    WORD nrline_up,nrline_dn,nrline;
    WORD dimline;
    colorRGB *tmpline;

    FILE *fp_bitmap=fopen(bitmap_name,"rb");

    if (fp_bitmap==NULL)
    {
        exitmessage("Cannot open bitmap file.File not found ?");
    }

    if (fread(TMPBUF,1,54,fp_bitmap)!=54)
    {
        exitmessage("Need a truecolor BMP to encode.");
    }

    if ((TMPBUF[0]!='B')||(TMPBUF[1]!='M')||(TMPBUF[28]!=24))
        exitmessage("Need a truecolor BMP to encode.");
    Ximage=(WORD)TMPBUF[19]*256+TMPBUF[18];
    Yimage=(WORD)TMPBUF[23]*256+TMPBUF[22];
    *Ximage_original=Ximage;
    *Yimage_original=Yimage; //Keep the old dimensions
    // of the image
    if (Ximage%8!=0) Xdiv8=(Ximage/8)*8+8;
    else Xdiv8=Ximage;
    if (Yimage%8!=0) Ydiv8=(Yimage/8)*8+8;
    else Ydiv8=Yimage;
    // The image we encode shall be filled with the last line and the last column
    // from the original bitmap, until Ximage and Yimage are divisible by 8
    // Load BMP image from disk and complete X
    RGB_buffer=(colorRGB *)(malloc(3*Xdiv8*Ydiv8));
    if (RGB_buffer==NULL) exitmessage("Not enough memory for the bitmap image.");
    if (Ximage%4!=0) nr_fillingbytes=4-(Ximage%4);
    else nr_fillingbytes=0;
    for (nrline=0;nrline<Yimage;nrline++)
    {
        fread(RGB_buffer+nrline*Xdiv8,1,Ximage*3,fp_bitmap);
        fread(TMPBUF,1,nr_fillingbytes,fp_bitmap);
        memcpy(&lastcolor,RGB_buffer+nrline*Xdiv8+Ximage-1,3);
        for (column=Ximage;column<Xdiv8;column++)
        {memcpy(RGB_buffer+nrline*Xdiv8+column,&lastcolor,3);}
    }
    Ximage=Xdiv8;
    dimline=Ximage*3;tmpline=(colorRGB *)malloc(dimline);
    if (tmpline==NULL) exitmessage("Not enough memory.");
    //Reorder in memory the inversed bitmap
    for (nrline_up=Yimage-1,nrline_dn=0;nrline_up>nrline_dn;nrline_up--,nrline_dn++)
    {
        memcpy(tmpline,RGB_buffer+nrline_up*Ximage, dimline);
        memcpy(RGB_buffer+nrline_up*Ximage,RGB_buffer+nrline_dn*Ximage,dimline);
        memcpy(RGB_buffer+nrline_dn*Ximage,tmpline,dimline);
    }
    // Y completion:
    memcpy(tmpline,RGB_buffer+(Yimage-1)*Ximage,dimline);
    for (nrline=Yimage;nrline<Ydiv8;nrline++)
    {memcpy(RGB_buffer+nrline*Ximage,tmpline,dimline);}
    Yimage=Ydiv8;
    free(tmpline);fclose(fp_bitmap);
}
}}







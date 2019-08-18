
#include "global612.h"
#include "utiltypedef.h"
#define XL 182
#define TL 40
BYTE Red1X[XL]={0,0,0,0,0,0,0,0,0,0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 18, 18};
BYTE Red1Y[XL]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,177,178,179,180,181};
BYTE Red2X[XL]={0,0,0,0,0,1,1,1,1,1, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9,10,10,10,10,10,11,11,11,11,11,12,12,12,12,12,13,13,13,13,13,14,14,14,14,14,15,15,15,15,15,16,16,16,16,16,17,17,17,17,17,18,18,18,18,18,19,19,19,19,19, 20, 20, 20, 20, 20, 21, 21, 21, 21, 21, 22, 22, 22, 22, 22, 23, 23, 23, 23, 23, 24, 24, 24, 24, 24, 25, 25, 25, 25, 25, 26, 26, 26, 26, 26, 27, 27, 27, 27, 27, 28, 28, 28, 28, 28, 29, 29, 29, 29, 29, 30, 30, 30, 30, 30, 31, 31, 31, 31, 31, 32, 32, 32, 32, 32, 33, 33, 33, 33, 33, 34, 34, 34, 34, 34, 35, 35, 35, 35, 35, 36, 36};
BYTE Red2Y[XL]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,177,178,179,180,181};
BYTE Red3X[XL]={0,0,0,0,1,1,1,2,2,2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 6, 6, 6, 7, 7, 7, 8, 8, 8, 9, 9, 9,10,10,10,11,11,11,12,12,12,13,13,13,13,14,14,14,15,15,15,16,16,16,17,17,17,18,18,18,19,19,19,20,20,20,21,21,21,22,22,22,23,23,23,24,24,24,25,25,25,26,26,26,26,27,27,27,28,28,28,29,29,29,30,30,30,31,31,31,32, 32, 32, 33, 33, 33, 34, 34, 34, 35, 35, 35, 36, 36, 36, 37, 37, 37, 38, 38, 38, 39, 39, 39, 39, 40, 40, 40, 41, 41, 41, 42, 42, 42, 43, 43, 43, 44, 44, 44, 45, 45, 45, 45, 46, 46, 46, 47, 47, 47, 48, 48, 48, 49, 49, 49, 50, 50, 50, 51, 51, 51, 52, 52, 52, 53, 53, 53, 54, 54, 54, 55, 55, 55, 56, 56, 56, 57, 57, 57, 58, 58, 58};
BYTE Red3Y[XL]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,177,178,179,180,181};
BYTE Red4X[XL]={0,0,1,1,2,2,2,3,3,4, 4, 5, 5, 5, 6, 6, 7, 7, 8, 8, 8, 9, 9,10,10,11,11,11,12,12,13,13,14,14,14,15,15,16,16,17,17,17,18,18,19,19,20,20,20,21,21,22,22,23,23,23,24,24,25,25,26,26,26,27,27,28,28,29,29,29,30,30,31,31,32,32,32,33,33,34,34,35,35,35,36,36,37,37,38,38,38,39,39,40,40,41,41,41,42,42, 43, 43, 44, 44, 44, 45, 45, 46, 46, 47, 47, 47, 48, 48, 49, 49, 50, 50, 51, 51, 51, 52, 52, 53, 53, 53, 54, 54, 55, 55, 56, 56, 56, 57, 57, 58, 58, 59, 59, 59, 60, 60, 61, 61, 62, 62, 62, 63, 63, 64, 64, 65, 65, 65, 66, 66, 67, 67, 68, 68, 68, 69, 69, 70, 70, 71, 71, 71, 72, 72, 73, 73, 74, 74, 74, 75, 75, 76, 76, 77, 77, 77};
BYTE Red4Y[XL]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,177,178,179,180,181};
BYTE Red5X[XL]={0,0,1,1,2,2,3,4,4,5, 5, 6, 6, 7, 8, 8, 9, 9,10,10,11,12,12,13,13,14,14,15,16,17,17,18,18,19,19,20,21,21,22,22,23,23,24,24,25,25,26,27,27,28,28,29,29,30,31,31,32,32,33,33,34,35,35,36,36,37,37,38,39,39,40,40,41,41,42,43,43,44,44,45,46,46,47,47,48,48,49,50,50,51,51,52,52,53,54,54,55,55,56,56, 57, 58, 58, 59, 59, 60, 60, 61, 62, 62, 63, 63, 64, 64, 65, 66, 66, 67, 67, 68, 69, 69, 70, 70, 71, 71, 72, 73, 73, 74, 74, 75, 75, 76, 77, 77, 78, 78, 79, 79, 80, 81, 81, 82, 82, 83, 83, 84, 85, 85, 86, 86, 87, 87, 88, 89, 89, 90, 90, 91, 92, 92, 93, 93, 94, 94, 95, 96, 96, 97, 97, 98, 98, 99, 99,100,101,101,102,102,103,103};
BYTE Red5Y[XL]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,177,178,179,180,181};
BYTE Red6X[XL]={0,0,1,2,2,3,4,4,5,6, 7, 7, 8, 9, 9,10,11,11,12,13,14,14,15,16,16,17,18,18,19,20,21,21,22,23,23,24,25,25,26,27,28,28,29,30,30,31,32,32,33,34,35,35,36,37,37,38,39,39,40,41,42,42,43,44,44,45,46,46,47,48,49,49,50,51,51,52,53,53,54,55,56,56,57,58,58,59,60,60,61,62,63,63,64,65,65,66,67,67,68,69, 70, 70, 71, 72, 72, 73, 74, 74, 75, 76, 77, 77, 78, 79, 79, 80, 81, 81, 82, 83, 84, 84, 85, 86, 86, 87, 88, 88, 89, 90, 91, 91, 92, 93, 93, 94, 95, 95, 96, 97, 98, 98, 99,100,100,101,102,102,103,104,105,105,106,107,107,108,109,109,110,111,112,112,113,114,114,115,116,116,117,118,119,119,120,121,121,122,123,123,124,125,126,126};
BYTE Red6Y[XL]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,177,178,179,180,181};
//BYTE Red7X[XL]={0,1,2,3,4,5,6,7,8,8, 9,10,11,12,13,14,15,16,17,17,18,19,20,21,22,23,24,25,26,26,27,28,29,30,31,32,33,34,35,35,36,37,38,39,40,41,42,43,44,44,45,46,47,48,49,50,51,52,53,53,54,55,56,57,58,59,60,61,62,62,63,64,65,66,67,68,69,70,71,71,72,73,74,75,76,77,78,79,80,80,81,82,83,84,85,86,87,88,89,89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 98, 99,100,101,102,103,104,105,106,107,107,108,109,110,111,112,113,114,115,116,116,117,118,119,120,121,122,123,124,125,125,126,127,128,129,130,131,132,133,134,134,135,136,137,138,139,140,141,142,143,143,144,145,146,147,148,149,150,151,152,152,153,154,155,156,157,158,159,160,161,161,162,163};
BYTE Red7X[XL]={0,1,2,3,4,5,6,7,8,8, 9,10,11,12,13,14,15,16,17,17,18,19,20,21,22,23,24,25,26,26,27,28,29,30,31,32,33,34,35,35,36,37,38,39,40,41,42,43,44,44,45,46,47,48,49,50,51,52,53,53,54,55,56,57,58,59,60,61,62,62,63,64,65,66,67,68,69,70,71,71,72,73,74,75,76,77,78,79,80,80,81,82,83,84,85,86,87,88,89,89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 98, 99,100,101,102,103,104,105,106,107,107,108,109,110,111,112,113,114,115,116,116,117,118,119,120,121,122,123,124,125,125,126,127,128,129,130,131,132,133,134,134,135,136,137,138,139,140,141,142,143,143,144,145,146,147,148,149,150,151,152,152,153,154,155,156,157,158,159,160,161,161,162,163};
BYTE Red7Y[XL]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,177,178,179,180,181};

#define  SECPIXEL   2   
WORD x, y; //x=512, y=300;   
void DrawSecondRed0()
{
    BYTE i;
    for(i=0;i<180;i++) Idu_OsdPaintArea(x-Red1X[i], y-Red1Y[i], SECPIXEL, 1, 0);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x+Red1X[i], y+Red1Y[i], SECPIXEL, 1, 0);
    for(i=0;i<XL;i++) Idu_OsdPaintArea(x+0, y-i, SECPIXEL, 1, 6);
    for(i=0;i<TL;i++) Idu_OsdPaintArea(x+0, y+i, SECPIXEL, 1, 6);
}

void DrawSecondRed1()
{
    BYTE i;
    for(i=0;i<XL;i++) Idu_OsdPaintArea(x+0, y-i, SECPIXEL, 1, 0);
    for(i=0;i<TL;i++) Idu_OsdPaintArea(x+0, y+i, SECPIXEL, 1, 0);
    for(i=0;i<180;i++) Idu_OsdPaintArea(x+Red1X[i], y-Red1Y[i], SECPIXEL, 1, 6);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x-Red1X[i], y+Red1Y[i], SECPIXEL, 1, 6);
}

void DrawSecondRed2()
{
    BYTE i;
    for(i=0;i<180;i++) Idu_OsdPaintArea(x+Red1X[i], y-Red1Y[i], SECPIXEL, 1, 0);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x-Red1X[i], y+Red1Y[i], SECPIXEL, 1, 0);
    for(i=0;i<175;i++) Idu_OsdPaintArea(x+Red2X[i], y-Red2Y[i], SECPIXEL, 1, 6);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x-Red2X[i], y+Red2Y[i], SECPIXEL, 1, 6);
}

void DrawSecondRed3()
{
    BYTE i;
    for(i=0;i<175;i++) Idu_OsdPaintArea(x+Red2X[i], y-Red2Y[i], SECPIXEL, 1, 0);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x-Red2X[i], y+Red2Y[i], SECPIXEL, 1, 0);
    for(i=0;i<170;i++) Idu_OsdPaintArea(x+Red3X[i], y-Red3Y[i], SECPIXEL, 1, 6);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x-Red3X[i], y+Red3Y[i], SECPIXEL, 1, 6); 
}

void DrawSecondRed4()
{
    BYTE i;
    for(i=0;i<170;i++) Idu_OsdPaintArea(x+Red3X[i], y-Red3Y[i], SECPIXEL, 1, 0);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x-Red3X[i], y+Red3Y[i], SECPIXEL, 1, 0); 
    for(i=0;i<165;i++) Idu_OsdPaintArea(x+Red4X[i], y-Red4Y[i], SECPIXEL, 1, 6);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x-Red4X[i], y+Red4Y[i], SECPIXEL, 1, 6);
}

void DrawSecondRed5()
{
    BYTE i;
    for(i=0;i<165;i++) Idu_OsdPaintArea(x+Red4X[i], y-Red4Y[i], SECPIXEL, 1, 0);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x-Red4X[i], y+Red4Y[i], SECPIXEL, 1, 0);
    for(i=0;i<160;i++) Idu_OsdPaintArea(x+Red5X[i], y-Red5Y[i], SECPIXEL, 1, 6);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x-Red5X[i], y+Red5Y[i], SECPIXEL, 1, 6);
}

void DrawSecondRed6()
{
    BYTE i;
    for(i=0;i<160;i++) Idu_OsdPaintArea(x+Red5X[i], y-Red5Y[i], SECPIXEL, 1, 0);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x-Red5X[i], y+Red5Y[i], SECPIXEL, 1, 0);
    for(i=0;i<150;i++) Idu_OsdPaintArea(x+Red6X[i], y-Red6Y[i], SECPIXEL, 1, 6);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x-Red6X[i], y+Red6Y[i], SECPIXEL, 1, 6);
}

void DrawSecondRed7()
{
    BYTE i;
    for(i=0;i<150;i++) Idu_OsdPaintArea(x+Red6X[i], y-Red6Y[i], SECPIXEL, 1, 0);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x-Red6X[i], y+Red6Y[i], SECPIXEL, 1, 0);
    for(i=0;i<135;i++) Idu_OsdPaintArea(x+Red7X[i], y-Red7Y[i], SECPIXEL, 1, 6);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x-Red7X[i], y+Red7Y[i], SECPIXEL, 1, 6);
}

void DrawSecondRed8()
{
    BYTE i;
    for(i=0;i<135;i++) Idu_OsdPaintArea(x+Red7X[i], y-Red7Y[i], SECPIXEL, 1, 0);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x-Red7X[i], y+Red7Y[i], SECPIXEL, 1, 0);
    for(i=0;i<135;i++) Idu_OsdPaintArea(x+Red7Y[i], y-Red7X[i], 1, SECPIXEL, 6);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x-Red7Y[i], y+Red7X[i], 1, SECPIXEL, 6);
}

void DrawSecondRed9()
{
    BYTE i;
    for(i=0;i<135;i++) Idu_OsdPaintArea(x+Red7Y[i], y-Red7X[i], 1, SECPIXEL, 0);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x-Red7Y[i], y+Red7X[i], 1, SECPIXEL, 0);   
    for(i=0;i<150;i++) Idu_OsdPaintArea(x+Red6Y[i], y-Red6X[i], 1, SECPIXEL, 6);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x-Red6Y[i], y+Red6X[i], 1, SECPIXEL, 6);
}

void DrawSecondRed10()
{
    BYTE i;
    for(i=0;i<150;i++) Idu_OsdPaintArea(x+Red6Y[i], y-Red6X[i], 1, SECPIXEL, 0);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x-Red6Y[i], y+Red6X[i], 1, SECPIXEL, 0);
    for(i=0;i<160;i++) Idu_OsdPaintArea(x+Red5Y[i], y-Red5X[i], 1, SECPIXEL, 6);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x-Red5Y[i], y+Red5X[i], 1, SECPIXEL, 6);
}

void DrawSecondRed11()
{
    BYTE i;
    for(i=0;i<160;i++) Idu_OsdPaintArea(x+Red5Y[i], y-Red5X[i], 1, SECPIXEL, 0);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x-Red5Y[i], y+Red5X[i], 1, SECPIXEL, 0);
    for(i=0;i<165;i++) Idu_OsdPaintArea(x+Red4Y[i], y-Red4X[i], 1, SECPIXEL, 6);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x-Red4Y[i], y+Red4X[i], 1, SECPIXEL, 6);
}

void DrawSecondRed12()
{
    BYTE i;
    for(i=0;i<165;i++) Idu_OsdPaintArea(x+Red4Y[i], y-Red4X[i], 1, SECPIXEL, 0);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x-Red4Y[i], y+Red4X[i], 1, SECPIXEL, 0);  
    for(i=0;i<170;i++) Idu_OsdPaintArea(x+Red3Y[i], y-Red3X[i], 1, SECPIXEL, 6);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x-Red3Y[i], y+Red3X[i], 1, SECPIXEL, 6);
}

void DrawSecondRed13()
{
    BYTE i;
    for(i=0;i<170;i++) Idu_OsdPaintArea(x+Red3Y[i], y-Red3X[i], 1, SECPIXEL, 0);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x-Red3Y[i], y+Red3X[i], 1, SECPIXEL, 0);
    for(i=0;i<175;i++) Idu_OsdPaintArea(x+Red2Y[i], y-Red2X[i], 1, SECPIXEL, 6);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x-Red2Y[i], y+Red2X[i], 1, SECPIXEL, 6);
}

void DrawSecondRed14()
{
    BYTE i;
    for(i=0;i<175;i++) Idu_OsdPaintArea(x+Red2Y[i], y-Red2X[i], 1, SECPIXEL, 0);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x-Red2Y[i], y+Red2X[i], 1, SECPIXEL, 0);
    for(i=0;i<180;i++) Idu_OsdPaintArea(x+Red1Y[i], y-Red1X[i], 1, SECPIXEL, 6);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x-Red1Y[i], y+Red1X[i], 1, SECPIXEL, 6);
}

void DrawSecondRed15()
{
    BYTE i;
    for(i=0;i<180;i++) Idu_OsdPaintArea(x+Red1Y[i], y-Red1X[i], 1, SECPIXEL, 0);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x-Red1Y[i], y+Red1X[i], 1, SECPIXEL, 0);
    for(i=0;i<XL;i++) Idu_OsdPaintArea(x+i, y, 1, SECPIXEL, 6);
    for(i=0;i<TL;i++) Idu_OsdPaintArea(x-i, y, 1, SECPIXEL, 6);
}
  
void DrawSecondRed16()
{
    BYTE i;
    for(i=0;i<XL;i++)  Idu_OsdPaintArea(x+i, y, 1, SECPIXEL, 0);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x-i, y, 1, SECPIXEL, 0);
    for(i=0;i<180;i++) Idu_OsdPaintArea(x+Red1Y[i], y+Red1X[i], 1, SECPIXEL, 6);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x-Red1Y[i], y-Red1X[i], 1, SECPIXEL, 6);
}

void DrawSecondRed17()
{
    BYTE i;
    for(i=0;i<180;i++) Idu_OsdPaintArea(x+Red1Y[i], y+Red1X[i], 1, SECPIXEL, 0);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x-Red1Y[i], y-Red1X[i], 1, SECPIXEL, 0);
    for(i=0;i<175;i++) Idu_OsdPaintArea(x+Red2Y[i], y+Red2X[i], 1, SECPIXEL, 6);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x-Red2Y[i], y-Red2X[i], 1, SECPIXEL, 6);
}

void DrawSecondRed18()
{
    BYTE i;
    for(i=0;i<175;i++) Idu_OsdPaintArea(x+Red2Y[i], y+Red2X[i], 1, SECPIXEL, 0);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x-Red2Y[i], y-Red2X[i], 1, SECPIXEL, 0);
    for(i=0;i<170;i++) Idu_OsdPaintArea(x+Red3Y[i], y+Red3X[i], 1, SECPIXEL, 6);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x-Red3Y[i], y-Red3X[i], 1, SECPIXEL, 6);
}

void DrawSecondRed19()
{
    BYTE i;
    for(i=0;i<170;i++) Idu_OsdPaintArea(x+Red3Y[i], y+Red3X[i], 1, SECPIXEL, 0);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x-Red3Y[i], y-Red3X[i], 1, SECPIXEL, 0);
    for(i=0;i<165;i++) Idu_OsdPaintArea(x+Red4Y[i], y+Red4X[i], 1, SECPIXEL, 6);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x-Red4Y[i], y-Red4X[i], 1, SECPIXEL, 6);
}

void DrawSecondRed20()
{
    BYTE i;
    for(i=0;i<165;i++) Idu_OsdPaintArea(x+Red4Y[i], y+Red4X[i], 1, SECPIXEL, 0);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x-Red4Y[i], y-Red4X[i], 1, SECPIXEL, 0);
    for(i=0;i<160;i++) Idu_OsdPaintArea(x+Red5Y[i], y+Red5X[i], 1, SECPIXEL, 6);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x-Red5Y[i], y-Red5X[i], 1, SECPIXEL, 6);
}

void DrawSecondRed21()
{
    BYTE i;
    for(i=0;i<160;i++) Idu_OsdPaintArea(x+Red5Y[i], y+Red5X[i], 1, SECPIXEL, 0);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x-Red5Y[i], y-Red5X[i], 1, SECPIXEL, 0);
    for(i=0;i<150;i++) Idu_OsdPaintArea(x+Red6Y[i], y+Red6X[i], 1, SECPIXEL, 6);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x-Red6Y[i], y-Red6X[i], 1, SECPIXEL, 6);
}

void DrawSecondRed22()
{
    BYTE i;
    for(i=0;i<150;i++) Idu_OsdPaintArea(x+Red6Y[i], y+Red6X[i], 1, SECPIXEL, 0);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x-Red6Y[i], y-Red6X[i], 1, SECPIXEL, 0);
    for(i=0;i<135;i++) Idu_OsdPaintArea(x+Red7Y[i], y+Red7X[i], 1, SECPIXEL, 6);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x-Red7Y[i], y-Red7X[i], 1, SECPIXEL, 6);
}

void DrawSecondRed23()
{
    BYTE i;
    for(i=0;i<135;i++) Idu_OsdPaintArea(x+Red7Y[i], y+Red7X[i], 1, SECPIXEL, 0);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x-Red7Y[i], y-Red7X[i], 1, SECPIXEL, 0); 
    for(i=0;i<135;i++) Idu_OsdPaintArea(x+Red7X[i], y+Red7Y[i], SECPIXEL, 1, 6);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x-Red7X[i], y-Red7Y[i], SECPIXEL, 1, 6);
}

void DrawSecondRed24()
{
    BYTE i;
    for(i=0;i<135;i++) Idu_OsdPaintArea(x+Red7X[i], y+Red7Y[i], SECPIXEL, 1, 0);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x-Red7X[i], y-Red7Y[i], SECPIXEL, 1, 0);
    for(i=0;i<150;i++) Idu_OsdPaintArea(x+Red6X[i], y+Red6Y[i], SECPIXEL, 1, 6);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x-Red6X[i], y-Red6Y[i], SECPIXEL, 1, 6);
}

void DrawSecondRed25()
{
    BYTE i;
    for(i=0;i<150;i++) Idu_OsdPaintArea(x+Red6X[i], y+Red6Y[i], SECPIXEL, 1, 0);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x-Red6X[i], y-Red6Y[i], SECPIXEL, 1, 0);
    for(i=0;i<160;i++) Idu_OsdPaintArea(x+Red5X[i], y+Red5Y[i], SECPIXEL, 1, 6);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x-Red5X[i], y-Red5Y[i], SECPIXEL, 1, 6);
}

void DrawSecondRed26()
{
    BYTE i;
    for(i=0;i<160;i++) Idu_OsdPaintArea(x+Red5X[i], y+Red5Y[i], SECPIXEL, 1, 0);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x-Red5X[i], y-Red5Y[i], SECPIXEL, 1, 0);
    for(i=0;i<165;i++) Idu_OsdPaintArea(x+Red4X[i], y+Red4Y[i], SECPIXEL, 1, 6);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x-Red4X[i], y-Red4Y[i], SECPIXEL, 1, 6);
}

void DrawSecondRed27()
{
    BYTE i;
    for(i=0;i<165;i++) Idu_OsdPaintArea(x+Red4X[i], y+Red4Y[i], SECPIXEL, 1, 0);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x-Red4X[i], y-Red4Y[i], SECPIXEL, 1, 0); 
    for(i=0;i<170;i++) Idu_OsdPaintArea(x+Red3X[i], y+Red3Y[i], SECPIXEL, 1, 6);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x-Red3X[i], y-Red3Y[i], SECPIXEL, 1, 6);
}

void DrawSecondRed28()
{
    BYTE i;
    for(i=0;i<170;i++) Idu_OsdPaintArea(x+Red3X[i], y+Red3Y[i], SECPIXEL, 1, 0);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x-Red3X[i], y-Red3Y[i], SECPIXEL, 1, 0);
    for(i=0;i<175;i++) Idu_OsdPaintArea(x+Red2X[i], y+Red2Y[i], SECPIXEL, 1, 6);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x-Red2X[i], y-Red2Y[i], SECPIXEL, 1, 6);
}

void DrawSecondRed29()
{
    BYTE i;
    for(i=0;i<175;i++) Idu_OsdPaintArea(x+Red2X[i], y+Red2Y[i], SECPIXEL, 1, 0);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x-Red2X[i], y-Red2Y[i], SECPIXEL, 1, 0);
    for(i=0;i<180;i++) Idu_OsdPaintArea(x+Red1X[i], y+Red1Y[i], SECPIXEL, 1, 6);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x-Red1X[i], y-Red1Y[i], SECPIXEL, 1, 6);
}

void DrawSecondRed30()
{
    BYTE i;
    for(i=0;i<180;i++) Idu_OsdPaintArea(x+Red1X[i], y+Red1Y[i], SECPIXEL, 1, 0);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x-Red1X[i], y-Red1Y[i], SECPIXEL, 1, 0);
    for(i=0;i<XL;i++) Idu_OsdPaintArea(x+0, y+i, SECPIXEL, 1, 6);
    for(i=0;i<TL;i++) Idu_OsdPaintArea(x+0, y-i, SECPIXEL, 1, 6);
}

void DrawSecondRed31()
{
    BYTE i;
    for(i=0;i<XL;i++) Idu_OsdPaintArea(x+0, y+i, SECPIXEL, 1, 0);
    for(i=0;i<TL;i++) Idu_OsdPaintArea(x+0, y-i, SECPIXEL, 1, 0);
    for(i=0;i<180;i++) Idu_OsdPaintArea(x-Red1X[i], y+Red1Y[i], SECPIXEL, 1, 6);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x+Red1X[i], y-Red1Y[i], SECPIXEL, 1, 6);
}
   
void DrawSecondRed32()
{
    BYTE i;
    for(i=0;i<180;i++) Idu_OsdPaintArea(x-Red1X[i], y+Red1Y[i], SECPIXEL, 1, 0);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x+Red1X[i], y-Red1Y[i], SECPIXEL, 1, 0);
    for(i=0;i<175;i++) Idu_OsdPaintArea(x-Red2X[i], y+Red2Y[i], SECPIXEL, 1, 6);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x+Red2X[i], y-Red2Y[i], SECPIXEL, 1, 6);
}
 
void DrawSecondRed33()
{
    BYTE i;
    for(i=0;i<175;i++) Idu_OsdPaintArea(x-Red2X[i], y+Red2Y[i], SECPIXEL, 1, 0);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x+Red2X[i], y-Red2Y[i], SECPIXEL, 1, 0);
    for(i=0;i<170;i++) Idu_OsdPaintArea(x-Red3X[i], y+Red3Y[i], SECPIXEL, 1, 6);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x+Red3X[i], y-Red3Y[i], SECPIXEL, 1, 6);
}
 
void DrawSecondRed34()
{
    BYTE i;
    for(i=0;i<170;i++) Idu_OsdPaintArea(x-Red3X[i], y+Red3Y[i], SECPIXEL, 1, 0);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x+Red3X[i], y-Red3Y[i], SECPIXEL, 1, 0);
    for(i=0;i<165;i++) Idu_OsdPaintArea(x-Red4X[i], y+Red4Y[i], SECPIXEL, 1, 6);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x+Red4X[i], y-Red4Y[i], SECPIXEL, 1, 6);
}

void DrawSecondRed35()
{
    BYTE i;
    for(i=0;i<165;i++) Idu_OsdPaintArea(x-Red4X[i], y+Red4Y[i], SECPIXEL, 1, 0);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x+Red4X[i], y-Red4Y[i], SECPIXEL, 1, 0);
    for(i=0;i<160;i++) Idu_OsdPaintArea(x-Red5X[i], y+Red5Y[i], SECPIXEL, 1, 6);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x+Red5X[i], y-Red5Y[i], SECPIXEL, 1, 6);
}

void DrawSecondRed36()
{
    BYTE i;
    for(i=0;i<160;i++) Idu_OsdPaintArea(x-Red5X[i], y+Red5Y[i], SECPIXEL, 1, 0);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x+Red5X[i], y-Red5Y[i], SECPIXEL, 1, 0);
    for(i=0;i<150;i++) Idu_OsdPaintArea(x-Red6X[i], y+Red6Y[i], SECPIXEL, 1, 6);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x+Red6X[i], y-Red6Y[i], SECPIXEL, 1, 6);
}

void DrawSecondRed37()
{
    BYTE i;
    for(i=0;i<150;i++) Idu_OsdPaintArea(x-Red6X[i], y+Red6Y[i], SECPIXEL, 1, 0);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x+Red6X[i], y-Red6Y[i], SECPIXEL, 1, 0);
    for(i=0;i<135;i++) Idu_OsdPaintArea(x-Red7X[i], y+Red7Y[i], SECPIXEL, 1, 6);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x+Red7X[i], y-Red7Y[i], SECPIXEL, 1, 6);
}

void DrawSecondRed38()
{
    BYTE i;
    for(i=0;i<135;i++) Idu_OsdPaintArea(x-Red7X[i], y+Red7Y[i], SECPIXEL, 1, 0);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x+Red7X[i], y-Red7Y[i], SECPIXEL, 1, 0);
    for(i=0;i<135;i++) Idu_OsdPaintArea(x-Red7Y[i], y+Red7X[i], 1, SECPIXEL, 6);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x+Red7Y[i], y-Red7X[i], 1, SECPIXEL, 6);
}

void DrawSecondRed39()
{
    BYTE i;
    for(i=0;i<135;i++) Idu_OsdPaintArea(x-Red7Y[i], y+Red7X[i], 1, SECPIXEL, 0);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x+Red7Y[i], y-Red7X[i], 1, SECPIXEL, 0);
    for(i=0;i<150;i++) Idu_OsdPaintArea(x-Red6Y[i], y+Red6X[i], 1, SECPIXEL, 6);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x+Red6Y[i], y-Red6X[i], 1, SECPIXEL, 6);
}

void DrawSecondRed40()
{
    BYTE i;
    for(i=0;i<150;i++) Idu_OsdPaintArea(x-Red6Y[i], y+Red6X[i], 1, SECPIXEL, 0);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x+Red6Y[i], y-Red6X[i], 1, SECPIXEL, 0);
    for(i=0;i<160;i++) Idu_OsdPaintArea(x-Red5Y[i], y+Red5X[i], 1, SECPIXEL, 6);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x+Red5Y[i], y-Red5X[i], 1, SECPIXEL, 6);
}

void DrawSecondRed41()
{
    BYTE i;
    for(i=0;i<160;i++) Idu_OsdPaintArea(x-Red5Y[i], y+Red5X[i], 1, SECPIXEL, 0);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x+Red5Y[i], y-Red5X[i], 1, SECPIXEL, 0);
    for(i=0;i<165;i++) Idu_OsdPaintArea(x-Red4Y[i], y+Red4X[i], 1, SECPIXEL, 6);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x+Red4Y[i], y-Red4X[i], 1, SECPIXEL, 6);
}

void DrawSecondRed42()
{
    BYTE i;
    for(i=0;i<165;i++) Idu_OsdPaintArea(x-Red4Y[i], y+Red4X[i], 1, SECPIXEL, 0);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x+Red4Y[i], y-Red4X[i], 1, SECPIXEL, 0);
    for(i=0;i<170;i++) Idu_OsdPaintArea(x-Red3Y[i], y+Red3X[i], 1, SECPIXEL, 6);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x+Red3Y[i], y-Red3X[i], 1, SECPIXEL, 6);
}

void DrawSecondRed43()
{
    BYTE i;
    for(i=0;i<170;i++) Idu_OsdPaintArea(x-Red3Y[i], y+Red3X[i], 1, SECPIXEL, 0);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x+Red3Y[i], y-Red3X[i], 1, SECPIXEL, 0);
    for(i=0;i<175;i++) Idu_OsdPaintArea(x-Red2Y[i], y+Red2X[i], 1, SECPIXEL, 6);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x+Red2Y[i], y-Red2X[i], 1, SECPIXEL, 6);
}

void DrawSecondRed44()
{
    BYTE i;
    for(i=0;i<175;i++) Idu_OsdPaintArea(x-Red2Y[i], y+Red2X[i], 1, SECPIXEL, 0);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x+Red2Y[i], y-Red2X[i], 1, SECPIXEL, 0);
    for(i=0;i<180;i++) Idu_OsdPaintArea(x-Red1Y[i], y+Red1X[i], 1, SECPIXEL, 6);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x+Red1Y[i], y-Red1X[i], 1, SECPIXEL, 6);
}

void DrawSecondRed45()
{
    BYTE i;
    for(i=0;i<180;i++) Idu_OsdPaintArea(x-Red1Y[i], y+Red1X[i], 1, SECPIXEL, 0);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x+Red1Y[i], y-Red1X[i], 1, SECPIXEL, 0);
    for(i=0;i<XL;i++) Idu_OsdPaintArea(x-i, y, 1, SECPIXEL, 6);
    for(i=0;i<TL;i++) Idu_OsdPaintArea(x+i, y, 1, SECPIXEL, 6);
}
  
void DrawSecondRed46()
{
    BYTE i;
    for(i=0;i<XL;i++) Idu_OsdPaintArea(x-i, y+0, 1, SECPIXEL, 0);
    for(i=0;i<TL;i++) Idu_OsdPaintArea(x+i, y+0, 1, SECPIXEL, 0);
    for(i=0;i<180;i++) Idu_OsdPaintArea(x-Red1Y[i], y-Red1X[i], 1, SECPIXEL, 6);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x+Red1Y[i], y+Red1X[i], 1, SECPIXEL, 6);
}

void DrawSecondRed47()
{
    BYTE i;
    for(i=0;i<180;i++) Idu_OsdPaintArea(x-Red1Y[i], y-Red1X[i], 1, SECPIXEL, 0);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x+Red1Y[i], y+Red1X[i], 1, SECPIXEL, 0);
    for(i=0;i<175;i++) Idu_OsdPaintArea(x-Red2Y[i], y-Red2X[i], 1, SECPIXEL, 6);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x+Red2Y[i], y+Red2X[i], 1, SECPIXEL, 6);
}

void DrawSecondRed48()
{
    BYTE i;
    for(i=0;i<175;i++) Idu_OsdPaintArea(x-Red2Y[i], y-Red2X[i], 1, SECPIXEL, 0);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x+Red2Y[i], y+Red2X[i], 1, SECPIXEL, 0);
    for(i=0;i<170;i++) Idu_OsdPaintArea(x-Red3Y[i], y-Red3X[i], 1, SECPIXEL, 6);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x+Red3Y[i], y+Red3X[i], 1, SECPIXEL, 6);
}

void DrawSecondRed49()
{
    BYTE i;
    for(i=0;i<170;i++) Idu_OsdPaintArea(x-Red3Y[i], y-Red3X[i], 1, SECPIXEL, 0);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x+Red3Y[i], y+Red3X[i], 1, SECPIXEL, 0);
    for(i=0;i<165;i++) Idu_OsdPaintArea(x-Red4Y[i], y-Red4X[i], 1, SECPIXEL, 6);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x+Red4Y[i], y+Red4X[i], 1, SECPIXEL, 6);
}


void DrawSecondRed50()
{
    BYTE i;
    for(i=0;i<165;i++) Idu_OsdPaintArea(x-Red4Y[i], y-Red4X[i], 1, SECPIXEL, 0);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x+Red4Y[i], y+Red4X[i], 1, SECPIXEL, 0);
    for(i=0;i<160;i++) Idu_OsdPaintArea(x-Red5Y[i], y-Red5X[i], 1, SECPIXEL, 6);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x+Red5Y[i], y+Red5X[i], 1, SECPIXEL, 6);
}

void DrawSecondRed51()
{
    BYTE i;
    for(i=0;i<160;i++) Idu_OsdPaintArea(x-Red5Y[i], y-Red5X[i], 1, SECPIXEL, 0);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x+Red5Y[i], y+Red5X[i], 1, SECPIXEL, 0);
    for(i=0;i<150;i++) Idu_OsdPaintArea(x-Red6Y[i], y-Red6X[i], 1, SECPIXEL, 6);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x+Red6Y[i], y+Red6X[i], 1, SECPIXEL, 6);
}

void DrawSecondRed52()
{
    BYTE i;
    for(i=0;i<150;i++) Idu_OsdPaintArea(x-Red6Y[i], y-Red6X[i], 1, SECPIXEL, 0);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x+Red6Y[i], y+Red6X[i], 1, SECPIXEL, 0);
    for(i=0;i<135;i++) Idu_OsdPaintArea(x-Red7Y[i], y-Red7X[i], 1, SECPIXEL, 6);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x+Red7Y[i], y+Red7X[i], 1, SECPIXEL, 6);
}

void DrawSecondRed53()
{
    BYTE i;
    for(i=0;i<135;i++) Idu_OsdPaintArea(x-Red7Y[i], y-Red7X[i], 1, SECPIXEL, 0);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x+Red7Y[i], y+Red7X[i], 1, SECPIXEL, 0);
    for(i=0;i<135;i++) Idu_OsdPaintArea(x-Red7X[i], y-Red7Y[i], SECPIXEL, 1, 6);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x+Red7X[i], y+Red7Y[i], SECPIXEL, 1, 6);
}

void DrawSecondRed54()
{
    BYTE i;
    for(i=0;i<135;i++) Idu_OsdPaintArea(x-Red7X[i], y-Red7Y[i], SECPIXEL, 1, 0);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x+Red7X[i], y+Red7Y[i], SECPIXEL, 1, 0);
    for(i=0;i<150;i++) Idu_OsdPaintArea(x-Red6X[i], y-Red6Y[i], SECPIXEL, 1, 6);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x+Red6X[i], y+Red6Y[i], SECPIXEL, 1, 6);
}

void DrawSecondRed55()
{
    BYTE i;
    for(i=0;i<150;i++) Idu_OsdPaintArea(x-Red6X[i], y-Red6Y[i], SECPIXEL, 1, 0);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x+Red6X[i], y+Red6Y[i], SECPIXEL, 1, 0);
    for(i=0;i<160;i++) Idu_OsdPaintArea(x-Red5X[i], y-Red5Y[i], SECPIXEL, 1, 6);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x+Red5X[i], y+Red5Y[i], SECPIXEL, 1, 6);
}

void DrawSecondRed56()
{
    BYTE i;
    for(i=0;i<160;i++) Idu_OsdPaintArea(x-Red5X[i], y-Red5Y[i], SECPIXEL, 1, 0);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x+Red5X[i], y+Red5Y[i], SECPIXEL, 1, 0);
    for(i=0;i<165;i++) Idu_OsdPaintArea(x-Red4X[i], y-Red4Y[i], SECPIXEL, 1, 6);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x+Red4X[i], y+Red4Y[i], SECPIXEL, 1, 6);
}
  
void DrawSecondRed57()
{
    BYTE i;
    for(i=0;i<165;i++) Idu_OsdPaintArea(x-Red4X[i], y-Red4Y[i], SECPIXEL, 1, 0);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x+Red4X[i], y+Red4Y[i], SECPIXEL, 1, 0);
    for(i=0;i<170;i++) Idu_OsdPaintArea(x-Red3X[i], y-Red3Y[i], SECPIXEL, 1, 6);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x+Red3X[i], y+Red3Y[i], SECPIXEL, 1, 6);
}
  
void DrawSecondRed58()
{
    BYTE i;
    for(i=0;i<170;i++) Idu_OsdPaintArea(x-Red3X[i], y-Red3Y[i], SECPIXEL, 1, 0);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x+Red3X[i], y+Red3Y[i], SECPIXEL, 1, 0);
    for(i=0;i<175;i++) Idu_OsdPaintArea(x-Red2X[i], y-Red2Y[i], SECPIXEL, 1, 6);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x+Red2X[i], y+Red2Y[i], SECPIXEL, 1, 6);
}
  
void DrawSecondRed59()
{
    BYTE i;
    for(i=0;i<175;i++) Idu_OsdPaintArea(x-Red2X[i], y-Red2Y[i], SECPIXEL, 1, 0);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x+Red2X[i], y+Red2Y[i], SECPIXEL, 1, 0);
    for(i=0;i<180;i++) Idu_OsdPaintArea(x-Red1X[i], y-Red1Y[i], SECPIXEL, 1, 6);
    for(i=0;i<TL;i++)  Idu_OsdPaintArea(x+Red1X[i], y+Red1Y[i], SECPIXEL, 1, 6);
}  

void DrawSecondRed(WORD xIn, WORD yIn, BYTE rotateSecond, BYTE indexSecond)
{
    x = xIn;
    y = yIn;
    switch(rotateSecond)
    {
    case 0: switch(indexSecond)
            {
            case 0 : DrawSecondRed0(); break;
            case 1 : DrawSecondRed1(); break;
            case 2 : DrawSecondRed2(); break;
            case 3 : DrawSecondRed3(); break;
            case 4 : DrawSecondRed4(); break;
            case 5 : DrawSecondRed5(); break;
            case 6 : DrawSecondRed6(); break;
            case 7 : DrawSecondRed7(); break;
            case 8 : DrawSecondRed8(); break;
            case 9 : DrawSecondRed9(); break;
            case 10: DrawSecondRed10(); break;
            case 11: DrawSecondRed11(); break;
            case 12: DrawSecondRed12(); break;
            case 13: DrawSecondRed13(); break;
            case 14: DrawSecondRed14(); break;
            }
            break;
    case 1 :switch(indexSecond)
            {
            case 0 : DrawSecondRed15(); break;
            case 1 : DrawSecondRed16(); break;
            case 2 : DrawSecondRed17(); break;
            case 3 : DrawSecondRed18(); break;
            case 4 : DrawSecondRed19(); break;
            case 5 : DrawSecondRed20(); break;
            case 6 : DrawSecondRed21(); break;
            case 7 : DrawSecondRed22(); break;
            case 8 : DrawSecondRed23(); break;
            case 9 : DrawSecondRed24(); break;
            case 10: DrawSecondRed25(); break;
            case 11: DrawSecondRed26(); break;
            case 12: DrawSecondRed27(); break;
            case 13: DrawSecondRed28(); break;
            case 14: DrawSecondRed29(); break;
            } 
            break;
    case 2 :switch(indexSecond)
            {
            case 0 : DrawSecondRed30(); break;
            case 1 : DrawSecondRed31(); break;
            case 2 : DrawSecondRed32(); break;
            case 3 : DrawSecondRed33(); break;
            case 4 : DrawSecondRed34(); break;
            case 5 : DrawSecondRed35(); break;
            case 6 : DrawSecondRed36(); break;
            case 7 : DrawSecondRed37(); break;
            case 8 : DrawSecondRed38(); break;
            case 9 : DrawSecondRed39(); break;
            case 10: DrawSecondRed40(); break;
            case 11: DrawSecondRed41(); break;
            case 12: DrawSecondRed42(); break;
            case 13: DrawSecondRed43(); break;
            case 14: DrawSecondRed44(); break;
            } 
            break;
    case 3 :switch(indexSecond)
            {
            case 0 : DrawSecondRed45(); break;
            case 1 : DrawSecondRed46(); break;
            case 2 : DrawSecondRed47(); break;
            case 3 : DrawSecondRed48(); break;
            case 4 : DrawSecondRed49(); break;
            case 5 : DrawSecondRed50(); break;
            case 6 : DrawSecondRed51(); break;
            case 7 : DrawSecondRed52(); break;
            case 8 : DrawSecondRed53(); break;
            case 9 : DrawSecondRed54(); break;
            case 10: DrawSecondRed55(); break;
            case 11: DrawSecondRed56(); break;
            case 12: DrawSecondRed57(); break;
            case 13: DrawSecondRed58(); break;
            case 14: DrawSecondRed59(); break;
            } 
            break;
    }
    
}

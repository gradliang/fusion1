#ifndef _PDFAPI_H
#define _PDFAPI_H

// PPM format
#define PPM_FORMAT_BLACK	1
#define PPM_FORMAT_GRAY		2
#define PPM_FORMAT_COLOR	3

extern int pdf2ppm(char* fileName, char mode, int PageNum, int *TotalPage, int* BufAddr, int *Width, int *Height);
extern int pdf2xml(char* filename, int PageNum, int *TotalPage);

#endif // _PDFAPI_H

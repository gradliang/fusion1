#ifndef ID3_TAG_H
#define ID3_TAG_H

typedef struct id3_tag_st {
  char Title[32];
  char Artist[32];
  char Album[32];
  char Comment[32];
  char Genre[32];
  char Year[5];
  char Track;
  char bReserve[2];
  int  PicPostion;
  int  PicLength;  
} id3_tag_t;

#endif


#ifndef	__PP___H
#define	__PP___H

#include "../libsrc/Codec/inc/ppapi.h"
#include "../libsrc/display/include/displaystructure.h"

PPResult Img_Rotate_PP(ST_IMGWIN * const target, const ST_IMGWIN * const source, const unsigned int angle, const int FullScreenOutput);
void Video_rotation_display(const unsigned int _rotation_display);
unsigned int Video_rotation_display_get();


#endif


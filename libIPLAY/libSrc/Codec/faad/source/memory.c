/*
** FAAD2 - Freeware Advanced Audio (AAC) Decoder including SBR decoding
** Copyright (C) 2003 M. Bakker, Ahead Software AG, http://www.nero.com
**  
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software 
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
**
** Any non-GPL usage of this software or parts of this software is strictly
** forbidden.
**
** Commercial non-GPL licensing of this software is possible.
** For more info contact Ahead Software through Mpeg4AAClicense@nero.com.
**
** $Id: memory.c,v 1.0 2008/03/03 08:20:11 menno Exp $
**/

#include "global612.h"
#include "common.h"
#include "structs.h"

extern element *sce, *cpe;
extern int16_t *spec_data, *spec_data1, *spec_data2;
extern real_t *spec_coef, *spec_coef1, *spec_coef2;
extern real_t *transf_buf;

extern void faad_free(void *);

void faad_working_space_init(void)
{
    sce = cpe = (element *)faad_malloc(sizeof(element));
    spec_data = spec_data1 = (int16_t *)faad_malloc(1024 * sizeof(int16_t));
    spec_data2 = (int16_t *)faad_malloc(1024 * sizeof(int16_t));
    spec_coef = spec_coef1 = (real_t *)faad_malloc(1024 * sizeof(real_t));
    spec_coef2 = (real_t *)faad_malloc(1024 * sizeof(real_t));
    transf_buf = (real_t *)faad_malloc(2048 * sizeof(real_t));
}

void faad_working_space_uninit(void)
{
    faad_free(sce);
    faad_free(spec_data);
    faad_free(spec_data2);
    faad_free(spec_coef);
    faad_free(spec_coef2);
    faad_free(transf_buf);
}

void faad_decode_sram_init(void)
{
}

void faad_filtbank_sram_init(fb_info *fb)
{
}

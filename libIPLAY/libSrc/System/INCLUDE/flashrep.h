#ifndef	__FLASHREP_H_
#define	__FLASHREP_H_

#include "utiltypedef.h"

#define	DEVI_MX29LV800T			0xDA
#define	DEVI_MX29LV800B			0x5B
#define	DEVI_MX29LV160T			0xC4
#define	DEVI_MX29LV160B			0x49
#define DEVI_MX29LV320CB    0xA8
#define DEVI_MX29LV640B                     0xCB

typedef struct _flashrep {
		WORD		tag;
		WORD		maxSecNum;
	
		void	(*free)(struct _flashrep*);
		WORD	(*getSecNum)(struct _flashrep*, DWORD);
		WORD	(*getMaxSecNum)(struct _flashrep*);
		BOOL	(*eraseSector)(struct _flashrep*, WORD);
}FLASHREP;

#endif //__FLASHREP_H_

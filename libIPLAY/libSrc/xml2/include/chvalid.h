/*
 * chvalid.h: this header exports interfaces for the character
 *		 range validation APIs
 *
 * This file is automatically generated from the cvs source
 * definition files using the genChRanges.py Python script
 *
 * Generation date: Sat Oct 18 20:32:35 2003
 * Sources: chvalid.def
 * William Brack <wbrack@mmm.com.hk>
 */

#ifndef __XML_CHVALID_H__
#define __XML_CHVALID_H__

#include "xmlversion.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Define our typedefs and structures
 *
 */
typedef struct _xmlChSRange xmlChSRange;
typedef xmlChSRange *xmlChSRangePtr;
struct _xmlChSRange {
    unsigned short	low;
    unsigned short	high;
};

typedef struct _xmlChLRange xmlChLRange;
typedef xmlChLRange *xmlChLRangePtr;
struct _xmlChLRange {
    unsigned int	low;
    unsigned int	high;
};

typedef struct _xmlChRangeGroup xmlChRangeGroup;
typedef xmlChRangeGroup *xmlChRangeGroupPtr;
struct _xmlChRangeGroup {
    int			nbShortRange;
    int			nbLongRange;
    xmlChSRangePtr	shortRange;	/* points to an array of ranges */
    xmlChLRangePtr	longRange;
};

/**
 * Range checking routine
 */
XMLPUBFUN int XMLCALL
		xmlCharInRange(unsigned int val, const xmlChRangeGroupPtr group);


/**
 * xmlIsBaseChar_ch:
 *
 * @c: char to validate
 *
 * Automatically generated by genChRanges.py
 */
#define xmlIsBaseChar_ch(c)	(((0x41 <= (c)) && ((c) <= 0x5a)) || \
				 ((0x61 <= (c)) && ((c) <= 0x7a)) || \
				 ((0xc0 <= (c)) && ((c) <= 0xd6)) || \
				 ((0xd8 <= (c)) && ((c) <= 0xf6)) || \
				 ((0xf8 <= (c)) && ((c) <= 0xff)))

/**
 * xmlIsBaseCharQ:
 * @c: char to validate
 *
 * Automatically generated by genChRanges.py
 */
#define xmlIsBaseCharQ(c)	(((c) < 0x100) ? \
				 xmlIsBaseChar_ch((c)) : \
				 xmlCharInRange((c), &xmlIsBaseCharGroup))

XMLPUBVAR xmlChRangeGroup xmlIsBaseCharGroup;

/**
 * xmlIsBlank_ch:
 *
 * @c: char to validate
 *
 * Automatically generated by genChRanges.py
 */
#define xmlIsBlank_ch(c)	(((c) == 0x20) || \
				 ((0x9 <= (c)) && ((c) <= 0xa)) || \
				 ((c) == 0xd))

/**
 * xmlIsBlankQ:
 * @c: char to validate
 *
 * Automatically generated by genChRanges.py
 */
#define xmlIsBlankQ(c)		(((c) < 0x100) ? \
				 xmlIsBlank_ch((c)) : 0)


/**
 * xmlIsChar_ch:
 *
 * @c: char to validate
 *
 * Automatically generated by genChRanges.py
 */
#define xmlIsChar_ch(c)		(((0x9 <= (c)) && ((c) <= 0xa)) || \
				 ((c) == 0xd) || \
				 ((0x20 <= (c)) && ((c) <= 0xff)))

/**
 * xmlIsCharQ:
 * @c: char to validate
 *
 * Automatically generated by genChRanges.py
 */
#define xmlIsCharQ(c)		(((c) < 0x100) ? \
				 xmlIsChar_ch((c)) :\
				(((0x100 <= (c)) && ((c) <= 0xd7ff)) || \
				 ((0xe000 <= (c)) && ((c) <= 0xfffd)) || \
				 ((0x10000 <= (c)) && ((c) <= 0x10ffff))))

XMLPUBVAR xmlChRangeGroup xmlIsCharGroup;

/**
 * xmlIsCombiningQ:
 * @c: char to validate
 *
 * Automatically generated by genChRanges.py
 */
#define xmlIsCombiningQ(c)	(((c) < 0x100) ? \
				 0 : \
				 xmlCharInRange((c), &xmlIsCombiningGroup))

XMLPUBVAR xmlChRangeGroup xmlIsCombiningGroup;

/**
 * xmlIsDigit_ch:
 *
 * @c: char to validate
 *
 * Automatically generated by genChRanges.py
 */
#define xmlIsDigit_ch(c)	(((0x30 <= (c)) && ((c) <= 0x39)))

/**
 * xmlIsDigitQ:
 * @c: char to validate
 *
 * Automatically generated by genChRanges.py
 */
#define xmlIsDigitQ(c)		(((c) < 0x100) ? \
				 xmlIsDigit_ch((c)) : \
				 xmlCharInRange((c), &xmlIsDigitGroup))

XMLPUBVAR xmlChRangeGroup xmlIsDigitGroup;

/**
 * xmlIsExtender_ch:
 *
 * @c: char to validate
 *
 * Automatically generated by genChRanges.py
 */
#define xmlIsExtender_ch(c)	(((c) == 0xb7))

/**
 * xmlIsExtenderQ:
 * @c: char to validate
 *
 * Automatically generated by genChRanges.py
 */
#define xmlIsExtenderQ(c)	(((c) < 0x100) ? \
				 xmlIsExtender_ch((c)) : \
				 xmlCharInRange((c), &xmlIsExtenderGroup))

XMLPUBVAR xmlChRangeGroup xmlIsExtenderGroup;

/**
 * xmlIsIdeographicQ:
 * @c: char to validate
 *
 * Automatically generated by genChRanges.py
 */
#define xmlIsIdeographicQ(c)	(((c) < 0x100) ? \
				 0 :\
				(((0x4e00 <= (c)) && ((c) <= 0x9fa5)) || \
				 ((c) == 0x3007) || \
				 ((0x3021 <= (c)) && ((c) <= 0x3029))))

XMLPUBVAR xmlChRangeGroup xmlIsIdeographicGroup;
XMLPUBVAR unsigned char xmlIsPubidChar_tab[256];

/**
 * xmlIsPubidChar_ch:
 * @c: char to validate
 *
 * Automatically generated by genChRanges.py
 */
#define xmlIsPubidChar_ch(c)	(xmlIsPubidChar_tab[(c)])

/**
 * xmlIsPubidCharQ:
 * @c: char to validate
 *
 * Automatically generated by genChRanges.py
 */
#define xmlIsPubidCharQ(c)	(((c) < 0x100) ? \
				 xmlIsPubidChar_ch((c)) : 0)

XMLPUBFUN int XMLCALL
		xmlIsBaseChar(unsigned int ch);
XMLPUBFUN int XMLCALL
		xmlIsBlank(unsigned int ch);
XMLPUBFUN int XMLCALL
		xmlIsChar(unsigned int ch);
XMLPUBFUN int XMLCALL
		xmlIsCombining(unsigned int ch);
XMLPUBFUN int XMLCALL
		xmlIsDigit(unsigned int ch);
XMLPUBFUN int XMLCALL
		xmlIsExtender(unsigned int ch);
XMLPUBFUN int XMLCALL
		xmlIsIdeographic(unsigned int ch);
XMLPUBFUN int XMLCALL
		xmlIsPubidChar(unsigned int ch);

#ifdef __cplusplus
}
#endif
#endif /* __XML_CHVALID_H__ */

/*
*******************************************************************************
*                               Magic Pixel
*                  5F, No.3, Creation Road III, Science_Based 
*                   Industrial Park, Hsinchu, Taiwan, R.O.C
*               (c) Copyright 2004, Magic Pixel Inc, Hsinchu, Taiwan
*
* All rights reserved. Magic Pixel's source code is an unpublished work and the 
* use of a copyright notice does not imply otherwise. This source code contains
* confidential, trad secret material. Any attempt or participation in 
* deciphering, decoding, reverse engineering or in ay way altering the source 
* code is strictly prohibited, unless the prior written consent of Magic 
* Pixel is obtained.
*
* Filename      : util_CharString.c
* Programmers   : 
* Created       : 
* Descriptions  : 
*******************************************************************************
*/

/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0


#include "global612.h"
#include "mpTrace.h"



void CharUpperCase08(BYTE * pChar)
{
	if (pChar == NULL)
	{
		MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
		return;
	}

	if (*pChar >= 0x61 && *pChar <= 0x7a)
		*pChar -= 0x20;
}



void UpperCase08(BYTE * string)
{
	if (string == NULL)
	{
		MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
		return;
	}

	while (*string)
	{
		if (*string >= 0x61 && *string <= 0x7a)
			*string -= 0x20;
		string++;
	}
}



ST_COMPARE_RESULT  ArrayCompare08(BYTE * a, BYTE * b, DWORD count)
{
	if ((a == NULL) || (b == NULL))
	{
		MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
		return E_COMPARE_ERROR;
	}

	while (count)
	{
		if (*a != *b)
			return E_COMPARE_DIFFERENT;
		a++;
		b++;
		count--;
	}

	return E_COMPARE_EQUAL;
}



ST_COMPARE_RESULT  StringCompare08(BYTE * a, BYTE * b)
{
	if ((a == NULL) || (b == NULL))
	{
		MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
		return E_COMPARE_ERROR;
	}

	while (*a && *b)
	{
		if (*a != *b)
			return E_COMPARE_DIFFERENT;
		a++;
		b++;
	}

	if (*a || *b)
		return E_COMPARE_DIFFERENT;
	else
		return E_COMPARE_EQUAL;
}



ST_COMPARE_RESULT  StringNCompare08(BYTE * a, BYTE * b, DWORD len)
{
	if ((a == NULL) || (b == NULL))
	{
		MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
		return E_COMPARE_ERROR;
	}

	while (*a && *b && len)
	{
		if (*a != *b)
			return E_COMPARE_DIFFERENT;
		a++;
		b++;
		len--;
	}

	if (len > 0)
		return E_COMPARE_DIFFERENT;
	else
		return E_COMPARE_EQUAL;
}



ST_COMPARE_RESULT  StringCompare16(WORD * a, WORD * b)
{
	if ((a == NULL) || (b == NULL))
	{
		MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
		return E_COMPARE_ERROR;
	}

	while (*a && *b)
	{
		if (*a != *b)
			return E_COMPARE_DIFFERENT;
		a++;
		b++;
	}

	if (*a || *b)
		return E_COMPARE_DIFFERENT;
	else
		return E_COMPARE_EQUAL;
}



ST_COMPARE_RESULT  StringNCompare16(WORD * a, WORD * b, DWORD len)
{
	if ((a == NULL) || (b == NULL))
	{
		MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
		return E_COMPARE_ERROR;
	}

	while (*a && *b && len)
	{
		if (*a != *b)
			return E_COMPARE_DIFFERENT;
		a++;
		b++;
		len--;
	}

	if (len > 0)
		return E_COMPARE_DIFFERENT;
	else
		return E_COMPARE_EQUAL;
}



void CharUpperCase16(WORD * pChar)
{
	if (pChar == NULL)
	{
		MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
		return;
	}

	if (*pChar >= 0x0061 && *pChar <= 0x007a)
		*pChar -= 0x0020;
}



void UpperCase16(WORD * string)
{
	if (string == NULL)
	{
		MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
		return;
	}

	while (*string)
	{
		if (*string >= 0x0061 && *string <= 0x007a)
			*string -= 0x0020;
		string++;
	}
}



ST_COMPARE_RESULT  ArrayCompare16(WORD * a, WORD * b, DWORD count)
{
	if ((a == NULL) || (b == NULL))
	{
		MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
		return E_COMPARE_ERROR;
	}

	while (count)
	{
		if (*a != *b)
			return E_COMPARE_DIFFERENT;
		a++;
		b++;
		count--;
	}

	return E_COMPARE_EQUAL;
}



ST_COMPARE_RESULT  StringCompare08_CaseInsensitive(BYTE * a, BYTE * b)
{
	BYTE ch_a, ch_b;

	if ((a == NULL) || (b == NULL))
	{
		MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
		return E_COMPARE_ERROR;
	}

	while (*a && *b)
	{
		ch_a = *a;
		ch_b = *b;
		CharUpperCase08(&ch_a);
		CharUpperCase08(&ch_b);

		if (ch_a != ch_b)
			return E_COMPARE_DIFFERENT;
		a++;
		b++;
	}

	if (*a || *b)
		return E_COMPARE_DIFFERENT;
	else
		return E_COMPARE_EQUAL;
}



ST_COMPARE_RESULT  StringNCompare08_CaseInsensitive(BYTE * a, BYTE * b, DWORD len)
{
	BYTE ch_a, ch_b;

	if ((a == NULL) || (b == NULL))
	{
		MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
		return E_COMPARE_ERROR;
	}

	while (*a && *b && len)
	{
		ch_a = *a;
		ch_b = *b;
		CharUpperCase08(&ch_a);
		CharUpperCase08(&ch_b);
		if (ch_a != ch_b)
			return E_COMPARE_DIFFERENT;
		a++;
		b++;
		len--;
	}

	if (len > 0)
		return E_COMPARE_DIFFERENT;
	else
		return E_COMPARE_EQUAL;
}



ST_COMPARE_RESULT  StringCompare16_CaseInsensitive(WORD * a, WORD * b)
{
	WORD wCh_a, wCh_b;

	if ((a == NULL) || (b == NULL))
	{
		MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
		return E_COMPARE_ERROR;
	}

	while (*a && *b)
	{
		wCh_a = *a;
		wCh_b = *b;
		CharUpperCase16(&wCh_a);
		CharUpperCase16(&wCh_b);
		if (wCh_a != wCh_b)
			return E_COMPARE_DIFFERENT;
		a++;
		b++;
	}

	if (*a || *b)
		return E_COMPARE_DIFFERENT;
	else
		return E_COMPARE_EQUAL;
}



ST_COMPARE_RESULT  StringNCompare16_CaseInsensitive(WORD * a, WORD * b, DWORD len)
{
	WORD wCh_a, wCh_b;

	if ((a == NULL) || (b == NULL))
	{
		MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
		return E_COMPARE_ERROR;
	}

	while (*a && *b && len)
	{
		wCh_a = *a;
		wCh_b = *b;
		CharUpperCase16(&wCh_a);
		CharUpperCase16(&wCh_b);
		if (wCh_a != wCh_b)
			return E_COMPARE_DIFFERENT;
		a++;
		b++;
		len--;
	}

	if (len > 0)
		return E_COMPARE_DIFFERENT;
	else
		return E_COMPARE_EQUAL;
}



// convert 'hex' to a hexdecimal string start at the position pointed
// by 'string' and this string will have 'length' characters
// return the BYTE position after the last character
BYTE *HexString(BYTE * string, DWORD hex, BYTE length)
{
	if (string == NULL)
	{
		MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
		return NULL;
	}

	hex = hex << ((8 - length) * 4);

	while (length)
	{
		*string = hex >> 28;
		hex = hex << 4;
		*string = *string + 0x30;
		if (*string > 0x39)
			*string = *string + 7;
		string++;
		length--;
	}
	*string = 0;

	return string;
}



// convert the 'value' from binary to BCD
DWORD Bin2Bcd(DWORD value)
{
	register DWORD bcd;
	register DWORD base;

	if (value > 99999999)
		return 0x99999999;

	bcd = 0;
	base = 10000000;
	while (value >= base)
	{
		value -= base;
		bcd++;
	}

	bcd <<= 4;
	base = 1000000;
	while (value >= base)
	{
		value -= base;
		bcd++;
	}

	bcd <<= 4;
	base = 100000;
	while (value >= base)
	{
		value -= base;
		bcd++;
	}

	bcd <<= 4;
	base = 10000;
	while (value >= base)
	{
		value -= base;
		bcd++;
	}

	bcd <<= 4;
	base = 1000;
	while (value >= base)
	{
		value -= base;
		bcd++;
	}

	bcd <<= 4;
	base = 100;
	while (value >= base)
	{
		value -= base;
		bcd++;
	}

	bcd <<= 4;
	base = 10;
	while (value >= base)
	{
		value -= base;
		bcd++;
	}

	bcd <<= 4;
	bcd += value;

	return bcd;
}



// convert 'dec' to a decimal string start at the position pointed
// by 'string' and this string will have 'length' characters
// return the BYTE position after the last character
BYTE *DecString(BYTE * string, DWORD dec, BYTE length, BYTE decimal)
{
	if (string == NULL)
	{
		MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
		return NULL;
	}

	dec = Bin2Bcd(dec);
	string = HexString(string, dec >> decimal, length);

	if (decimal)
	{
		*string = '.';
		string++;
		string = HexString(string, dec, decimal);
	}

	return string;
}



BYTE *SkipLeadingZero(BYTE * string)
{
	if (string == NULL)
	{
		MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
		return NULL;
	}

	while (*string == '0')
		string++;
	if (!*string)
		string--;

	return string;
}



void *TrimOffString08TailSpaces(BYTE * string)
{
	DWORD str_len;
	BYTE *bCh_ptr;

	if (string == NULL)
	{
		MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
		return;
	}

	str_len = StringLength08(string);
	while (str_len)
	{
		bCh_ptr = (BYTE *) string + (str_len - 1);
		if ((*bCh_ptr == ' ') || (*bCh_ptr == '\t')) /* check Space and Tab */
		{
			*bCh_ptr = '\0';
			str_len--;
		}
		else
			break;
	}
}



void *TrimOffString16TailSpaces(WORD * string)
{
	DWORD str_len;
	WORD *wCh_ptr;

	if (string == NULL)
	{
		MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
		return;
	}

	str_len = StringLength16(string);
	while (str_len)
	{
		wCh_ptr = (WORD *) ((WORD *) string + (str_len - 1));
		if ((*wCh_ptr == 0x0020) || (*wCh_ptr == 0x0009)) /* check Space and Tab */
		{
			*wCh_ptr = 0x0000;
			str_len--;
		}
		else
			break;
	}
}



BYTE *DigitStringSeparate(BYTE * string)
{
	BYTE length;
	BYTE commaNumber;
	BYTE commaCounter;
	BYTE *source, *target, *ret;


	if (string == NULL)
	{
		MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
		return NULL;
	}

	length = StringLength08(string);
	commaNumber = length / 3;
	if (length == (commaNumber * 3))
		commaNumber--;

	source = string + length - 1;
	target = source + commaNumber + 1;
	*target = 0;
	ret = target;
	target--;

	commaCounter = 0;
	while (length)
	{
		*target = *source;
		target--;
		source--;
		commaCounter++;
		if (commaCounter == 3 && commaNumber)
		{
			*target = ',';
			target--;
			commaCounter = 0;
			commaNumber--;
		}
		length--;
	}

	return ret;
}



BYTE *StringNCopy08(BYTE * target, BYTE * source, DWORD count)
{
	if ((target == NULL) || (source == NULL))
	{
		MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
		return NULL;
	}

	while ((count) && (*source))
	{
		*target = *source;
		target++;
		source++;
		count--;
	}

	return target;
}



BYTE *StringCopy08(BYTE * target, BYTE * source)
{
	if ((target == NULL) || (source == NULL))
	{
		MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
		return NULL;
	}

	while (*source)
	{
		*target = *source;
		target++;
		source++;
	}
	*target = '\0';

	return target;
}



WORD *StringNCopy16(WORD * target, WORD * source, DWORD count)
{
	if ((target == NULL) || (source == NULL))
	{
		MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
		return NULL;
	}

	while ((count) && (*source))
	{
		*target = *source;
		target++;
		source++;
		count--;
	}

	return target;
}



WORD *StringCopy16(WORD * target, WORD * source)
{
	if ((target == NULL) || (source == NULL))
	{
		MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
		return NULL;
	}

	while (*source)
	{
		*target = *source;
		target++;
		source++;
	}
	*target = 0;

	return target;
}



BYTE *StringNCopy1608(BYTE * target, WORD * source, DWORD count)
{
	if ((target == NULL) || (source == NULL))
	{
		MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
		return NULL;
	}

	while ((*source) && (count))
	{
		*target = (BYTE) * source;
		target++;
		source++;
		count--;
	}
	//*target = 0;

	return target;
}



BYTE *StringCopy1608(BYTE * target, WORD * source)
{
	if ((target == NULL) || (source == NULL))
	{
		MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
		return NULL;
	}

	while (*source)
	{
		*target = (BYTE) * source;
		target++;
		source++;
	}
	*target = 0;

	return target;
}



WORD *StringNCopy0816(WORD * target, BYTE * source, DWORD count)
{
	if ((target == NULL) || (source == NULL))
	{
		MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
		return NULL;
	}

	while ((*source) && (count))
	{
		*target = (WORD) * source;
		target++;
		source++;
		count--;
	}
	//*target = 0;

	return target;
}



WORD *StringCopy0816(WORD * target, BYTE * source)
{
	if ((target == NULL) || (source == NULL))
	{
		MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
		return NULL;
	}

	while (*source)
	{
		*target = (WORD) * source;
		target++;
		source++;
	}
	*target = 0;

	return target;
}



WORD StringLength08(BYTE * string)
{
	WORD length = 0;


	if (string == NULL)
	{
		MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
		return 0;
	}

	while (*string)
	{
		length++;
		string++;
	}

	return length;
}



WORD StringLength16(WORD * string)
{
	WORD length = 0;


	if (string == NULL)
	{
		MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
		return 0;
	}

	while (*string)
	{
		length++;
		string++;
	}

	return length;
}



//Same utility as strrchr but used for WORD case
WORD *String16rchr(WORD * string, WORD wchar)
{
	WORD* rtv = NULL;


	if (string == NULL)
	{
		MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
		return NULL;
	}

	while(*string)
	{
		if(*string == wchar)
			rtv = string;

		string++;
	}

	return rtv;
}



BYTE *String08_strstr(BYTE * haystack, BYTE * needle)
{
	DWORD nlen, hlen;


	if ((haystack == NULL) || (needle == NULL))
	{
		MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
		return NULL;
	}

	nlen = StringLength08(needle);
	hlen = StringLength08(haystack);

	while (hlen-- >= nlen)
	{
		if (StringNCompare08(haystack, needle, nlen) == E_COMPARE_EQUAL)
			return (BYTE *)haystack;
		haystack++;
	}
	return NULL;
}



WORD *String16_strstr(WORD * haystack, WORD * needle)
{
	DWORD nlen, hlen;


	if ((haystack == NULL) || (needle == NULL))
	{
		MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
		return NULL;
	}

	nlen = StringLength16(needle);
	hlen = StringLength16(haystack);

	while (hlen-- >= nlen)
	{
		if (StringNCompare16(haystack, needle, nlen) == E_COMPARE_EQUAL)
			return (WORD *)haystack;
		((WORD *)haystack)++;
	}
	return NULL;
}



BOOL IsDigit08(BYTE ch)
{
	if ((ch >= '0') && (ch <= '9'))
		return TRUE;
	else
		return FALSE;
}



BOOL IsDigit16(WORD wCh)
{
	if ((wCh >= 0x0030) && (wCh <= 0x0039))
		return TRUE;
	else
		return FALSE;
}



BOOL IsAlpha08(BYTE ch)
{
	if (((ch >= 'A') && (ch <= 'Z')) || ((ch >= 'a') && (ch <= 'z')))
		return TRUE;
	else
		return FALSE;
}



BOOL IsAlpha16(WORD wCh)
{
	if (((wCh >= 0x0041) && (wCh <= 0x005A)) || ((wCh >= 0x0061) && (wCh <= 0x007A)))
		return TRUE;
	else
		return FALSE;
}




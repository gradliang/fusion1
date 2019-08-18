#include	<stdarg.h>
#include	<stdlib.h>
//#include  <float.h>
#include	<math.h>


/* #define PF_FLOATS  to include printf support for floats */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef neAR
#define neAR
#endif
#ifndef fAR
#define fAR
#endif

#define MP_U      0x01			/* upper */
#define MP_L      0x02			/* lower */
#define MP_D      0x04			/* digit */
#define MP_C      0x08			/* cntrl */
#define MP_P      0x10			/* punct */
#define MP_S      0x20			/* white space (space/lf/tab) */
#define MP_X      0x40			/* hex digit */
#define MP_SP     0x80			/* hard space (0x20) */

#define mp__ismask(x) (mp_ctype[(int)(unsigned char)(x)])

//#define isalnum(c)      ((mp__ismask(c)&(MP_U|MP_L|MP_D)) != 0)
//#define isalpha(c)      ((mp__ismask(c)&(MP_U|MP_L)) != 0)
//#define iscntrl(c)      ((mp__ismask(c)&(MP_C)) != 0)
#define mp_isdigit(c)      ((mp__ismask(c)&(MP_D)) != 0)
//#define isgraph(c)      ((mp__ismask(c)&(MP_P|MP_U|MP_L|MP_D)) != 0)
#define mp_islower(c)      ((mp__ismask(c)&(MP_L)) != 0)
//#define isprint(c)      ((mp__ismask(c)&(MP_P|MP_U|MP_L|MP_D|MP_SP)) != 0)
//#define ispunct(c)      ((mp__ismask(c)&(MP_P)) != 0)
#define mp_isspace(c)      ((mp__ismask(c)&(MP_S)) != 0)
#define mp_isupper(c)      ((mp__ismask(c)&(MP_U)) != 0)
#define mp_isxdigit(c)     ((mp__ismask(c)&(MP_D|MP_X)) != 0)

//#define isascii(c) (((unsigned char)(c))<=0x7f)
//#define toascii(c) (((unsigned char)(c))&0x7f)

//#define tolower(c) __tolower(c)
#define mp_toupper(c) __mp_toupper(c)

#define INT_MAX         ((int)(~0U>>1))
// #define mp_isspace(c)      ((c) == ' ')

#define __builtin_expect(x, expected_value) (x)
#define unlikely(x)     __builtin_expect(!!(x), 0)

unsigned char mp_ctype[] = {
	MP_C, MP_C, MP_C, MP_C, MP_C, MP_C, MP_C, MP_C,	/* 0-7 */
	MP_C, MP_C | MP_S, MP_C | MP_S, MP_C | MP_S, MP_C | MP_S, MP_C | MP_S, MP_C, MP_C,	/* 8-15 */
	MP_C, MP_C, MP_C, MP_C, MP_C, MP_C, MP_C, MP_C,	/* 16-23 */
	MP_C, MP_C, MP_C, MP_C, MP_C, MP_C, MP_C, MP_C,	/* 24-31 */
	MP_S | MP_SP, MP_P, MP_P, MP_P, MP_P, MP_P, MP_P, MP_P,	/* 32-39 */
	MP_P, MP_P, MP_P, MP_P, MP_P, MP_P, MP_P, MP_P,	/* 40-47 */
	MP_D, MP_D, MP_D, MP_D, MP_D, MP_D, MP_D, MP_D,	/* 48-55 */
	MP_D, MP_D, MP_P, MP_P, MP_P, MP_P, MP_P, MP_P,	/* 56-63 */
	MP_P, MP_U | MP_X, MP_U | MP_X, MP_U | MP_X, MP_U | MP_X, MP_U | MP_X, MP_U | MP_X, MP_U,	/* 64-71 */
	MP_U, MP_U, MP_U, MP_U, MP_U, MP_U, MP_U, MP_U,	/* 72-79 */
	MP_U, MP_U, MP_U, MP_U, MP_U, MP_U, MP_U, MP_U,	/* 80-87 */
	MP_U, MP_U, MP_U, MP_P, MP_P, MP_P, MP_P, MP_P,	/* 88-95 */
	MP_P, MP_L | MP_X, MP_L | MP_X, MP_L | MP_X, MP_L | MP_X, MP_L | MP_X, MP_L | MP_X, MP_L,	/* 96-103 */
	MP_L, MP_L, MP_L, MP_L, MP_L, MP_L, MP_L, MP_L,	/* 104-111 */
	MP_L, MP_L, MP_L, MP_L, MP_L, MP_L, MP_L, MP_L,	/* 112-119 */
	MP_L, MP_L, MP_L, MP_P, MP_P, MP_P, MP_P, MP_C,	/* 120-127 */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* 128-143 */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* 144-159 */
	MP_S | MP_SP, MP_P, MP_P, MP_P, MP_P, MP_P, MP_P, MP_P, MP_P, MP_P, MP_P, MP_P, MP_P, MP_P, MP_P, MP_P,	/* 160-175 */
	MP_P, MP_P, MP_P, MP_P, MP_P, MP_P, MP_P, MP_P, MP_P, MP_P, MP_P, MP_P, MP_P, MP_P, MP_P, MP_P,	/* 176-191 */
	MP_U, MP_U, MP_U, MP_U, MP_U, MP_U, MP_U, MP_U, MP_U, MP_U, MP_U, MP_U, MP_U, MP_U, MP_U, MP_U,	/* 192-207 */
	MP_U, MP_U, MP_U, MP_U, MP_U, MP_U, MP_U, MP_P, MP_U, MP_U, MP_U, MP_U, MP_U, MP_U, MP_U, MP_L,	/* 208-223 */
	MP_L, MP_L, MP_L, MP_L, MP_L, MP_L, MP_L, MP_L, MP_L, MP_L, MP_L, MP_L, MP_L, MP_L, MP_L, MP_L,	/* 224-239 */
	MP_L, MP_L, MP_L, MP_L, MP_L, MP_L, MP_L, MP_P, MP_L, MP_L, MP_L, MP_L, MP_L, MP_L, MP_L, MP_L
};								/* 240-255 */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

union argdef
{
	long sni;
	unsigned long mp_uni;
	void *vp;
	void neAR *mp_nvp;
	void fAR *mp_fvp;
};

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
static inline unsigned char __tolower(unsigned char c)
{
	if (mp_isupper(c))
		c -= 'A' - 'a';
	return c;
}

static inline unsigned char __mp_toupper(unsigned char c)
{
	if (mp_islower(c))
		c -= 'a' - 'A';
	return c;
}


static int padlft(char *srcbeg, char *srcbuf, int minsze, char padchr)
{
/*
    padlft:     left pad string
    entry:      &beginning of buffer
                &current buffer loc
                minimum buffer size
                pad character
    exit:       return value is number of chars in output buffer
                zero terminated output string in output buffer
*/
	int shfcnt, padcnt;
	int bufcnt;
	char *dstptr;

	bufcnt = srcbuf - srcbeg;	/* number in buffer */
	padcnt = minsze - bufcnt;	/* adjustment */
	if (padcnt > 0)
	{
		shfcnt = bufcnt + 1;	/* number to shift */
		bufcnt += padcnt;		/* new buffer count */
		dstptr = srcbuf + padcnt;	/* rightmost destination */
		while (shfcnt--)		/* move string right */
			*dstptr-- = *srcbuf--;
		while (padcnt--)		/* pad left */
			*dstptr-- = padchr;
	}
	return bufcnt;
}

static int ckwdth(char *srcbeg, char *srcbuf, int minsze, char lftjst, char padchr)
{
/*
    ckwdth:     check width
    entry:      &beginning of buffer
                &current buffer loc
                minimum buffer size
                justification
                pad character
    exit:       return value is number of chars in output buffer
                zero terminated output string in output buffer
*/
	int padcnt;
	int bufcnt;

	bufcnt = srcbuf - srcbeg;	/* number in buffer */
	padcnt = minsze - bufcnt;	/* adjustment */
	if (padcnt > 0)
	{
		bufcnt += padcnt;		/* new buffer count */
		if (lftjst)
		{						/* right pad buffer */
			while (padcnt--)
				*srcbuf++ = padchr;
		}
		else
			bufcnt = padlft(srcbeg, srcbuf, minsze, padchr);	/* left pad buffer */
	}
	return bufcnt;
}


static int putuni(char *obuf, unsigned long unsint, int prec)
{
/*
    putuni:     unsigned integer to string
    entry:      &output buffer
                unsigned integer
                precision
    exit:       return value is number of chars in output buffer
                zero terminated output string in output buffer
*/
#define     maxten  1000000000	/* max ten power */
	char *begbuf;				/* begin of buf */
	char ochr;					/* output char */
	int ocnt;					/* output count */
	char dmpdig;				/* digit flag */
	unsigned long divisor;		/* numeric divisor */

	begbuf = obuf;
	dmpdig = 0;
	divisor = maxten;
	while (divisor > 1)
	{
		ochr = '0';
		while (unsint >= divisor)
		{
			ochr++;
			unsint = unsint - divisor;
		}
		divisor = divisor / 10;
		if (dmpdig || (ochr != '0'))
		{
			dmpdig = 1;
			*obuf++ = ochr;
		}
	}
	ochr = (char) ('0' + unsint);
	*obuf++ = ochr;
	*obuf = 0;

	ocnt = padlft(begbuf, obuf, prec, '0');	/* left pad buffer */

	return ocnt;
}


static int putsni(char *obuf, long sgnint, int prec, char sign)
{
/*
    putsni:     signed integer to string
    entry:      &output buffer
                signed integer
                character for positive sign
                precision
    exit:       return value is number of chars in output buffer
                zero terminated output string in output buffer
*/
	int ocnt;					/* output count */

	ocnt = 0;
	if (sgnint < 0)
	{
		sgnint = -sgnint;
		sign = '-';
	}
	if (sign != 0)
	{
		*obuf++ = sign;
		ocnt++;
	}
	ocnt += putuni(obuf, sgnint, prec);
	return ocnt;
}


static int putunh(char *obuf, unsigned long unshex, int prec, char shfcnt, char lwrflg, char ptrflg)
{
/*
    putunh:     unsigned hex to string
    entry:      &output buffer
                unsigned hex
                starting shift count
                prefix flag
                upper/lower case flag
                pointer flag
    exit:       return value is number of chars in output buffer
                zero terminated output string in output buffer
*/
	char *begbuf;				/* begin of buf */
	char ochr;					/* output char */
	int ocnt;					/* output count */
	char dmpdig;

	begbuf = obuf;
	if (ptrflg)
		dmpdig = 1;
	else
		dmpdig = 0;
	while (shfcnt)
	{
		ochr = (char) ((unshex >> shfcnt) & 15);
		if (ochr > 9)
		{
			if (lwrflg)
				ochr += 'a' - 10;
			else
				ochr += 'A' - 10;
		}
		else
			ochr += '0';
		if (dmpdig || (ochr != '0'))
		{
			dmpdig = 1;
			*obuf++ = ochr;
		}
#ifdef CPU_8086
		if (ptrflg && (shfcnt == 16))
		{
			*obuf++ = ':';
		}
#endif
		shfcnt = (char) (shfcnt - 4);
	}
	ochr = (char) (unshex & 15);
	if (lwrflg && (ochr > 9))
		ochr += 'a' - 10;
	else if (ochr > 9)
		ochr += 'A' - 10;
	else
		ochr += '0';
	*obuf++ = ochr;
	*obuf = 0;

	ocnt = padlft(begbuf, obuf, prec, '0');	/* left pad buffer */
	return ocnt;
}

static int putuno(char *obuf, unsigned long unsoct, int prec, char shfcnt, char frchsh)
{
/*
    putuno:     unsigned octal to string
    entry:      &output buffer
                unsigned octal
                precision
                starting shift count
                prefix flag
    exit:       return value is number of chars in output buffer
                zero terminated output string in output buffer
*/
	char *begbuf;				/* begin of buf */
	char ochr;					/* output char */
	int ocnt;					/* output count */
	char dmpdig;

	begbuf = obuf;
	if (frchsh && (unsoct != 0))
	{
		*obuf++ = '0';
	}

	dmpdig = 0;
	while (shfcnt)
	{
		ochr = (char) ('0' + ((unsoct >> shfcnt) & 7));
		if (dmpdig || (ochr != '0'))
		{
			dmpdig = 1;
			*obuf++ = ochr;
		}
		shfcnt = (char) (shfcnt - 3);
	}
	ochr = (char) ('0' + (unsoct & 7));
	*obuf++ = ochr;
	*obuf = 0;

	ocnt = padlft(begbuf, obuf, prec, '0');	/* left pad buffer */
	return ocnt;
}

#ifdef PF_FLOATS
#define DIGS 18
double tentoi(int i1);

static int putdig(char *buf, double fno)
{
	int texp, i1, i2;
	long l1;
	char ochr;
	double d1, d2;

	if (fno == 0.)
	{
		for (i1 = 0; i1 < DIGS; i1++)
			buf[i1] = '0';
		return 0;
	}
	frexp(fno, &i1);
	l1 = (long) i1 *0x4d10;

	i2 = l1 >> 16;
	texp = i2;
	if (i2 < DIGS)
	{
		if (i2 < DIGS - 308)
		{
			fno *= 1e308;
			fno *= tentoi(DIGS - i2 - 308);
		}
		else
			fno *= tentoi(DIGS - i2);
		i2 = DIGS;
		while (fno < tentoi(DIGS))
		{
			fno *= 10.;
			texp--;
		}
	}
	if (i2 < 308)
		if (tentoi(i2 + 1) <= fno)
		{
			i2++;
			texp++;
		}
	for (i1 = 0; i1 < DIGS + 1; i1++)
	{
		ochr = '0';
		d2 = tentoi(i2);
		while (fno >= d2)
		{
			ochr++;
			fno = fno - d2;
		}
		i2--;
		buf[i1] = ochr;
	}
	return texp;
}

static int bround(char *buf, int texp, int ndigs)
{
	int i1 = ndigs, i2;
	char cc;

	if (buf[i1] == '5')
	{
		for (i2 = i1 + 1; i2 < DIGS; i2++)
			if (buf[i2] != '0')
				break;
		if (i2 == DIGS)
		{
			cc = i1 ? buf[i1 - 1] : '0';
			if (!(cc & 1))
				goto xx1;
		}
	}
	if (buf[i1] >= '5')
	{
		buf[i1] = '0';
		while (i1--)
		{
			if (++buf[i1] == '9' + 1)
				buf[i1] = '0';
			else
				break;
		}
		if (buf[0] == '0')
		{
			buf[0] = '1';
			texp++;
		}
	}
  xx1:
	return texp;
}

static int putfpf(char *obuf, char *wbuf, int texp, int prec, char frchsh)
{
/*
    putfpf:     floating point number to string
    entry:      &output buffer
                &ASCII scratch buffer
                exponent of 10
                precision
                force decimal point flag
    exit:       return value is number of chars in output buffer
                string in output buffer
*/
	int ocnt, i1, i2, i3;
	char *cp1, *cp2;

	cp1 = obuf;
	i1 = i2 = 0;
	if (texp < 0)
		*cp1++ = '0';
	else
	{
		for (; i1 < DIGS && i1 < texp + 1; i1++)
			*cp1++ = *wbuf++;
		for (; i2 < texp - DIGS + 1; i2++)
			*cp1++ = '0';
	}
	if (prec || frchsh)
	{
		*cp1++ = '.';
		for (i2 = 0; i2 < -texp - 1 && i2 < prec; i2++)
			*cp1++ = '0';
		for (; i2 < prec; i1++, i2++)
			*cp1++ = i1 < DIGS ? *wbuf++ : '0';
	}
	return cp1 - obuf;
}

static int putfpe(char *obuf, char *wbuf, int texp, int prec, char expchr, char frchsh)
{
/*
    putfpe:     floating point number to string
    entry:      &output buffer
                &ASCII scratch buffer
                exponent of 10
                precision
                exponent character
                force decimal point flag
    exit:       return value is number of chars in output buffer
                string in output buffer
*/
	int ocnt, i1, i2, i3;
	char *cp1, *cp2;
	div_t remq;

	cp1 = obuf;
	*cp1++ = *wbuf++;
	if (prec || frchsh)
	{
		*cp1++ = '.';
		for (i2 = 0; i2 < prec; i2++)
			*cp1++ = *wbuf++;
	}
	*cp1++ = expchr;
	if (texp < 0)
	{
		texp = -texp;
		*cp1++ = '-';
	}
	else
		*cp1++ = '+';
	remq = div(texp, 100);
	if (remq.quot)
		*cp1++ = remq.quot | '0';
	remq = div(remq.rem, 10);
	*cp1++ = remq.quot | '0';
	*cp1++ = remq.rem | '0';
	return (cp1 - obuf);
}

double tentoi(int i1)
{
	double d1, d2;

	d1 = i1 & 1 ? 10. : 1.;
	i1 >>= 1;
	for (d2 = 10.; i1; i1 >>= 1)
	{
		d2 *= d2;
		if (i1 & 1)
			d1 *= d2;
	}
	return d1;
}

#endif

int mp_vsprintf(char *buf, const char *mp_fmt, va_list args)
{
/*
    vsprintf:   Reentrant Print Routine
    entry:      &output buffer
                &format string
                &argument list
    exit:       return value is number of chars in output buffer
                zero terminated output string in output buffer
*/
	char lftjst;				/* left/right justify flag */
	char jstchr;				/* justification char */
	char presgn;				/* positive sign prefix */
	char frchsh;				/* force prefix */
	char intsze, ptrsze;		/* arg size in bytes */
	char lwrflg;				/* lower/upper case flag */
	int width, precision;		/* field width & precision */

	char fch;					/* format char */
	char *bufbeg;				/* beginning of buf */
	int bufidx;					/* number chars in buf */
	char part;					/* field part / shift count */
	int tcnt;					/* temporary count */
	char fAR *chrptr;			/* character pointer */
	int *intptr;				/* integer pointer */
	union argdef arg;			/* argument */

#ifdef PF_FLOATS
	int i1, i2, i3;
	char wbuf[20], fltsze;
	double fltarg;				/* floating point argument */
#endif

/*
    process format string
*/
	bufidx = 0;					/* no chars in buf */
	while ((fch = *mp_fmt++) != 0)
	{							/* process the format */
		switch (fch)
		{
		case '%':				/* field specification */
			/* set defaults */
			bufbeg = buf;		/* beginning of field */
			lftjst = 0;			/* 0: df  right justify */
			/* 1:     left justify */
			jstchr = ' ';		/* 0: df  blank justify */
			/* 1:     zero justify */
			presgn = 0;			/* 0: df no >0 sign prfx */
			/* 1:    + prfx >0 with '+' */
			/* 2:      prfx >0 with ' ' */
			frchsh = 0;			/* 0: df  no frc */
			/* 1:     prfx based on type */
			/*        o prfx 0 */
			/*        x prefix 0x */
			/*        X prefix 0X */
			/*        e|E|f frc dcml pt */
			/*        g|G frc dc, trl 0s */
			/*        ? ignored */
			intsze = 2;			/* 2: df int */
			/* 1:    short */
#ifdef PF_FLOATS				/* 4:    long */
			fltsze = 8;			/* 8: df double */
#endif /* 10:   long double */
			ptrsze = 0;			/* 4: df memory model */
			/* 2:    neAR */
			/* 4:    fAR */
			lwrflg = 1;			/* 1: df use lower case */
			/* 0:    use upper case */

			precision = -1;		/* 0: df precision */
			width = 0;			/* 0: df width */


			part = 1;			/* prcs optional flags */
			while (part == 1)
			{
				switch (fch = *mp_fmt++)
				{
				case '-':		/* left justify */
					lftjst = 1;
					break;
				case '+':		/* prefix with + */
					presgn = '+';
					break;
				case ' ':		/* prefix with space */
					if (presgn != '+')
						presgn = ' ';
					break;
				case '#':		/* prefix for hash */
					frchsh = 1;
					break;
				default:		/* done with field flags */
					part = 2;
					break;
				}
			}

			while (part != 6)
			{					/* prcs optional */
				/*   width,precision,prefix */
				switch (fch)
				{
				case '*':		/* dynamic width | precision */
					if (part == 2)
					{			/* dynamic width */
						fch = *mp_fmt++;
						width = va_arg(args, int);

						if (width < 0)
							width = 0;
						part++;
					}
					else if (part == 4)
					{			/* dynamic precision */
						fch = *mp_fmt++;
						precision = va_arg(args, int);

						if (precision < 0)
							precision = 0;
						part++;
					}
					else
						part = 6;
					break;
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
					if (part == 2)
					{			/* static width */
						if (fch == '0')
							jstchr = '0';
						width = 0;
						while ((fch >= '0') && (fch <= '9'))
						{
							width = width * 10 + fch - '0';
							fch = *mp_fmt++;
						}
						part++;
					}
					else if (part == 4)
					{			/* static precision */
						precision = 0;
						while ((fch >= '0') && (fch <= '9'))
						{
							precision = precision * 10 + fch - '0';
							fch = *mp_fmt++;
						}
						if (precision < 0)
							precision = 0;
						part++;
					}
					else
						part = 6;
					break;
				case '.':		/* start precision */
					if (part <= 3)
					{
						precision = 0;
						fch = *mp_fmt++;
						part = 4;
					}
					else
						part = 6;
					break;
				case 'h':		/* arg is short */
					fch = *mp_fmt++;
					intsze = 1;
					part = 6;
					break;
				case 'l':		/* arg is long or double */
					fch = *mp_fmt++;
					intsze = 4;
#ifdef PF_FLOATS
					fltsze = 8;
#endif
					part = 6;
					break;
				case 'L':		/* arg is long double */
					fch = *mp_fmt++;
#ifdef PF_FLOATS
					fltsze = 10;
#endif
					part = 6;
					break;
				case 'F':		/* arg is fAR */
					fch = *mp_fmt++;
					ptrsze = 4;
					part = 6;
					break;
				case 'N':		/* arg is neAR */
					fch = *mp_fmt++;
					ptrsze = 2;
					part = 6;
					break;
				default:
					part = 6;
				}
			}

			switch (fch)
			{					/* type */
			case 'd':			/* default signed integer */
			case 'i':
				if (intsze == 1)
				{
					arg.sni = (short) va_arg(args, int);
				}
				else if (intsze == 2)
				{
					arg.sni = va_arg(args, int);
				}
				else
				{
					arg.sni = va_arg(args, long);
				}
				if (precision <= 0)
					precision = 1;
				buf += putsni(buf, arg.sni, precision, presgn);
				tcnt = ckwdth(bufbeg, buf, width, lftjst, jstchr);
				buf = bufbeg + tcnt;
				bufidx += tcnt;
				break;

			case 'u':			/* unsigned integer */
				if (intsze == 1)
				{
					arg.mp_uni = (unsigned short) va_arg(args, unsigned);
				}
				else if (intsze == 2)
				{
					arg.mp_uni = va_arg(args, unsigned int);
				}
				else
				{
					arg.mp_uni = va_arg(args, unsigned long);
				}
				if (precision <= 0)
					precision = 1;
				buf += putuni(buf, arg.mp_uni, precision);
				tcnt = ckwdth(bufbeg, buf, width, lftjst, jstchr);
				buf = bufbeg + tcnt;
				bufidx += tcnt;
				break;

			case 'o':			/* unsigned octal */
				if (intsze == 1)
				{
					arg.mp_uni = (unsigned short) va_arg(args, unsigned int);
				}
				else if (intsze == 2)
				{
					arg.mp_uni = va_arg(args, unsigned int);
				}
				else
				{
					arg.mp_uni = va_arg(args, unsigned long);
				}
				if (precision <= 0)
					precision = 1;
				buf += putuno(buf, arg.mp_uni, precision, 30, frchsh);
				tcnt = ckwdth(bufbeg, buf, width, lftjst, jstchr);
				buf = bufbeg + tcnt;
				bufidx += tcnt;
				break;

			case 'X':			/* unsigned hexidecimal (uc) */
				lwrflg = 0;
			case 'x':			/* unsigned hexidecimal (lc) */
				if (intsze == 1)
				{
					arg.mp_uni = (unsigned short) va_arg(args, unsigned);
				}
				else if (intsze == 2)
				{
					arg.mp_uni = va_arg(args, unsigned int);
				}
				else
				{
					arg.mp_uni = va_arg(args, unsigned long);
				}
				if (frchsh && (arg.mp_uni != 0))
				{
					*buf++ = '0';
					*buf++ = lwrflg ? 'x' : 'X';	/* prepend 0X */
				}
				if (precision <= 0)
					precision = 1;
				buf += putunh(buf, arg.mp_uni, precision, 28, lwrflg, 0);
				tcnt = ckwdth(bufbeg, buf, width, lftjst, jstchr);
				buf = bufbeg + tcnt;
				bufidx += tcnt;	/* total buffer len += field len */
				break;

#ifdef PF_FLOATS
			case 'f':			/* fltsze d.d */
				if (fltsze == 8)
					fltarg = va_arg(args, double);

				else
					fltarg = va_arg(args, long double);

				if (precision < 0)
					precision = 6;
				if (fltarg < 0)
				{
					fltarg = -fltarg;
					presgn = '-';
				}
				if (presgn)
					*buf++ = presgn;
				if (fltarg > DBL_MAX)
				{
					if (!presgn)
						*buf++ = '+';
					*buf++ = 'I';
					*buf++ = 'N';
					*buf++ = 'F';
					goto xx1;
				}
				i1 = putdig(wbuf, fltarg);
				i2 = precision + i1 + 1;
				if ((i2 >= 0) && (i2 <= DIGS))
					i1 = bround(wbuf, i1, i2);
				buf += putfpf(buf, wbuf, i1, precision, frchsh);
			  xx1:
				tcnt = ckwdth(bufbeg, buf, width, lftjst, jstchr);
				buf = bufbeg + tcnt;
				bufidx += tcnt;
				break;

			case 'E':			/* fltsze d.dE+ddd */
			case 'e':			/* fltsze d.de+ddd */
				if (fltsze == 8)
					fltarg = va_arg(args, double);

				else
					fltarg = va_arg(args, long double);

				if (precision < 0)
					precision = 6;
				if (fltarg < 0)
				{
					fltarg = -fltarg;
					presgn = '-';
				}
				if (presgn)
					*buf++ = presgn;
				if (fltarg > DBL_MAX)
				{
					if (!presgn)
						*buf++ = '+';
					*buf++ = 'I';
					*buf++ = 'N';
					*buf++ = 'F';
					goto xx2;
				}
				i1 = putdig(wbuf, fltarg);
				i1 = bround(wbuf, i1, precision + 1);
				buf += putfpe(buf, wbuf, i1, precision, fch, frchsh);
			  xx2:
				tcnt = ckwdth(bufbeg, buf, width, lftjst, jstchr);
				buf = bufbeg + tcnt;
				bufidx += tcnt;
				break;

			case 'G':			/* fltsze pck mp_fmt (G not e) */
			case 'g':			/* fltsze pck mp_fmt */
				if (fltsze == 8)
					fltarg = va_arg(args, double);

				else
					fltarg = va_arg(args, long double);

				if (precision < 0)
					precision = 6;
				if (fltarg < 0)
				{
					fltarg = -fltarg;
					presgn = '-';
				}
				if (presgn)
					*buf++ = presgn;
				if (fltarg > DBL_MAX)
				{
					if (!presgn)
						*buf++ = '+';
					*buf++ = 'I';
					*buf++ = 'N';
					*buf++ = 'F';
					goto xx3;
				}
				i1 = putdig(wbuf, fltarg);
				i1 = bround(wbuf, i1, precision);
				i2 = precision;
				if ((i1 < -4) || (i1 >= precision))
				{
					if (!frchsh)
						for (; wbuf[i2 - 1] == '0' && i2 > 1; i2--);
					buf += putfpe(buf, wbuf, i1, i2 - 1, fch - 2, frchsh);
				}
				else
				{
					if (!frchsh)
						for (; wbuf[i2 - 1] == '0' && i2 > i1 + 1; i2--);
					buf += putfpf(buf, wbuf, i1, i2 - i1 - 1, frchsh);
				}
			  xx3:
				tcnt = ckwdth(bufbeg, buf, width, lftjst, jstchr);
				buf = bufbeg + tcnt;
				bufidx += tcnt;
				break;
#endif

			case 'c':			/* single character */
				fch = (char) (va_arg(args, int));

				*buf++ = fch;
				*buf = 0;
				tcnt = ckwdth(bufbeg, buf, width, lftjst, jstchr);
				buf = bufbeg + tcnt;
				bufidx += tcnt;
				break;

			case 's':			/* string */
				if (ptrsze == 0)
					chrptr = (char fAR *) va_arg(args, char *);

				else if (ptrsze == 2)
					chrptr = (char fAR *) va_arg(args, char neAR *);

				else
					chrptr = (char fAR *) va_arg(args, char fAR *);

				if (!chrptr)
					chrptr = "NULL";	/* don't crash with NULL %s ptr */
				while (((*buf = *chrptr++) != 0) && (precision--))
					buf++;
				tcnt = ckwdth(bufbeg, buf, width, lftjst, jstchr);
				buf = bufbeg + tcnt;
				bufidx += tcnt;
				break;

			case 'n':			/* store # chars in buf to */
				/* int pointed to by arg */
				intptr = va_arg(args, int *);

				*intptr = bufidx;
				break;

			case 'p':			/* pointer ssss:oooo */
				if (ptrsze == 0)
				{
					arg.mp_fvp = va_arg(args, void *);
					part = sizeof(void *) * 8 - 4;	/* bit cnt - 4 */
				}
				else if (ptrsze == 2)
				{
					arg.mp_nvp = va_arg(args, void neAR *);

					part = 12;
				}
				else
				{
					arg.mp_fvp = va_arg(args, void fAR *);

					part = 28;
				}
				buf += putunh(buf, arg.mp_uni, 1, part, 0, 1);
				tcnt = ckwdth(bufbeg, buf, width, lftjst, jstchr);
				buf = bufbeg + tcnt;
				bufidx += tcnt;
				break;

			default:			/* mp_fmt character to buf */
				*buf++ = fch;
				bufidx++;
				break;
			}					/* switch */
			break;

		default:				/* character to buf */
			*buf++ = fch;
			bufidx++;
			break;

		}						/* switch */
	}
	*buf = 0;					/* zero terminate */
	return bufidx;
}


int mp_sprintf(char *buf, const char *mp_fmt, ...)
{
/*
    mp_sprintf:    Reentrant mp_sprintf
    entry:      address of output buffer
                address of format string
                format string arguments
    exit:       returns of chars in output buffer
                zero terminated output string in output buffer
*/
	int count;
	va_list args;

	va_start(args, mp_fmt);		/* get variable arg list address */
	count = mp_vsprintf(buf, mp_fmt, args);	/* process mp_fmt & args into buf */
	return count;
}

static int skip_atoi(const char **s)
{
	int i = 0;

	while (mp_isdigit(**s))
		i = i * 10 + *((*s)++) - '0';
	return i;
}

/**
* simple_mp_strtoull - convert a string to an unsigned long long
* @cp: The start of the string
* @endp: A pointer to the end of the parsed string will be placed here
* @mp_base: The number base to use
*/
unsigned long long simple_mp_strtoull(const char *cp, char **endp, unsigned int mp_base)
{
	unsigned long long result = 0, value;

	if (!mp_base)
	{
		mp_base = 10;
		if (*cp == '0')
		{
			mp_base = 8;
			cp++;
			if ((mp_toupper(*cp) == 'X') && mp_isxdigit(cp[1]))
			{
				cp++;
				mp_base = 16;
			}
		}
	}
	else if (mp_base == 16)
	{
		if (cp[0] == '0' && mp_toupper(cp[1]) == 'X')
			cp += 2;
	}
	while (mp_isxdigit(*cp) && (value = mp_isdigit(*cp) ? *cp - '0' : (mp_islower(*cp)
																 ? mp_toupper(*cp) : *cp) - 'A' + 10) <
		   mp_base)
	{
		result = result * mp_base + value;
		cp++;
	}
	if (endp)
		*endp = (char *) cp;
	return result;
}


/**
* simple_mp_strtoll - convert a string to a signed long long
* mp@cp: The start of the string
* mp@endp: A pointer to the end of the parsed string will be placed here
* @mp_base: The number base to use
*/
long long simple_mp_strtoll(const char *cp, char **endp, unsigned int mp_base)
{
	if (*cp == '-')
		return -simple_mp_strtoull(cp + 1, endp, mp_base);
	return simple_mp_strtoull(cp, endp, mp_base);
}

/**
 * simple_mp_strtoul - convert a string to an unsigned long
 *mp_ @cp: The start of the string
 * mp_@endp: A pointer to the end of the parsed string will be placed here
 * @mp_base: The number base to use
*/
unsigned long simple_mp_strtoul(const char *cp, char **endp, unsigned int mp_base)
{
	unsigned long result = 0, value;

	if (!mp_base)
	{
		mp_base = 10;
		if (*cp == '0')
		{
			mp_base = 8;
			cp++;
			if ((mp_toupper(*cp) == 'X') && mp_isxdigit(cp[1]))
			{
				cp++;
				mp_base = 16;
			}
		}
	}
	else if (mp_base == 16)
	{
		if (cp[0] == '0' && mp_toupper(cp[1]) == 'X')
			cp += 2;
	}
	while (mp_isxdigit(*cp) && (value = mp_isdigit(*cp) ? *cp - '0' : mp_toupper(*cp) - 'A' + 10) < mp_base)
	{
		result = result * mp_base + value;
		cp++;
	}
	if (endp)
		*endp = (char *) cp;
	return result;
}


/**
 * simple_mp_strtol - convert a string to a signed long
 *mp_ @cp: The start of the string
 *mp_ @endp: A pointer to the end of the parsed string will be placed here
 * @mp_base: The number base to use
 */
long simple_mp_strtol(const char *cp, char **endp, unsigned int mp_base)
{
	if (*cp == '-')
		return -simple_mp_strtoul(cp + 1, endp, mp_base);
	return simple_mp_strtoul(cp, endp, mp_base);
}

/**
 * mp_vsscanf - Unformat a buffer into a list of arguments
 * @buf:        input buffer
 * @mp_fmt:        format of buffer
 * @args:       arguments
 */
int mp_vsscanf(const char *buf, const char *mp_fmt, va_list args)
{
	const char *mp_str = buf;
	char *next;
	char digit;
	int num = 0;
	int qualifier;
	int mp_base;
	int field_width;
	int is_sign = 0;

	while (*mp_fmt && *mp_str)
	{
		/* skip any white space in format mp_*/
		/* white space in format matchs any amount ofmp_
		 * white space, including none, in the input.mp_
		 */
		if (mp_isspace(*mp_fmt))
		{
			while (mp_isspace(*mp_fmt))
				++mp_fmt;
			while (mp_isspace(*mp_str))
				++mp_str;
		}

		/* anything that is not a conversion must match exactly mp_*/
		if (*mp_fmt != '%' && *mp_fmt)
		{
			if (*mp_fmt++ != *mp_str++)
				break;
			continue;
		}

		if (!*mp_fmt)
			break;
		++mp_fmt;

		/* skip this conversion.mp_
		 * advance both mp_strings to next white spacemp_
		 */
		if (*mp_fmt == '*')
		{
			while (!mp_isspace(*mp_fmt) && *mp_fmt)
				mp_fmt++;
			while (!mp_isspace(*mp_str) && *mp_str)
				mp_str++;
			continue;
		}

		/* get field width mp_*/
		field_width = -1;
		if (mp_isdigit(*mp_fmt))
			field_width = skip_atoi(&mp_fmt);

		/* get conversion qualifier mp_*/
		qualifier = -1;
		if (*mp_fmt == 'h' || *mp_fmt == 'l' || *mp_fmt == 'L' || *mp_fmt == 'Z' || *mp_fmt == 'z')
		{
			qualifier = *mp_fmt++;
			if (unlikely(qualifier == *mp_fmt))
			{
				if (qualifier == 'h')
				{
					qualifier = 'H';
					mp_fmt++;
				}
				else if (qualifier == 'l')
				{
					qualifier = 'L';
					mp_fmt++;
				}
			}
		}
		mp_base = 10;
		is_sign = 0;

		if (!*mp_fmt || !*mp_str)
			break;

		switch (*mp_fmt++)
		{
		case 'c':
			{
				char *s = (char *) va_arg(args, char *);

				if (field_width == -1)
					field_width = 1;
				do
				{
					*s++ = *mp_str++;
				}
				while (--field_width > 0 && *mp_str);
				num++;
			}
			continue;
		case 's':
			{
				char *s = (char *) va_arg(args, char *);

				if (field_width == -1)
					field_width = INT_MAX;
				/* first, skip leading white space in buffer mp_*/
				while (mp_isspace(*mp_str))
					mp_str++;

				/* now copy until next white space mp_*/
				while (*mp_str && !mp_isspace(*mp_str) && field_width--)
				{
					*s++ = *mp_str++;
				}
				*s = '\0';
				num++;
			}
			continue;
		case 'n':
			/* return number of characters read so far mp_*/
			{
				int *i = (int *) va_arg(args, int *);

				*i = mp_str - buf;
			}
			continue;
		case 'o':
			mp_base = 8;
			break;
		case 'x':
		case 'X':
			mp_base = 16;
			break;
		case 'i':
			mp_base = 0;
		case 'd':
			is_sign = 1;
		case 'u':
			break;
		case '%':
			/* looking for '%' in mp_str mp_*/
			if (*mp_str++ != '%')
				return num;
			continue;
		default:
			/* invalid format; stop here mp_mp_*/
			return num;
		}

		/* have some sort of integer conversion.mp_
		 * first, skip white space in buffer.mp_
		 */
		while (mp_isspace(*mp_str))
			mp_str++;

		digit = *mp_str;
		if (is_sign && digit == '-')
			digit = *(mp_str + 1);

		if (!digit
			|| (mp_base == 16 && !mp_isxdigit(digit))
			|| (mp_base == 10 && !mp_isdigit(digit))
			|| (mp_base == 8 && (!mp_isdigit(digit) || digit > '7')) || (mp_base == 0 && !mp_isdigit(digit)))
			break;

		switch (qualifier)
		{
		case 'H':				/* that's 'hh' in format mp_*/
			if (is_sign)
			{
				signed char *s = (signed char *) va_arg(args, signed char *);

				*s = (signed char) simple_mp_strtol(mp_str, &next, mp_base);
			}
			else
			{
				unsigned char *s = (unsigned char *) va_arg(args, unsigned char *);

				*s = (unsigned char) simple_mp_strtoul(mp_str, &next, mp_base);
			}
			break;
		case 'h':
			if (is_sign)
			{
				short *s = (short *) va_arg(args, short *);

				*s = (short) simple_mp_strtol(mp_str, &next, mp_base);
			}
			else
			{
				unsigned short *s = (unsigned short *) va_arg(args, unsigned short *);

				*s = (unsigned short) simple_mp_strtoul(mp_str, &next, mp_base);
			}
			break;
		case 'l':
			if (is_sign)
			{
				long *l = (long *) va_arg(args, long *);

				*l = simple_mp_strtol(mp_str, &next, mp_base);
			}
			else
			{
				unsigned long *l = (unsigned long *) va_arg(args, unsigned long *);

				*l = simple_mp_strtoul(mp_str, &next, mp_base);
			}
			break;
		case 'L':
			if (is_sign)
			{
				long long *l = (long long *) va_arg(args, long long *);

				*l = simple_mp_strtoll(mp_str, &next, mp_base);
			}
			else
			{
				unsigned long long *l = (unsigned long long *) va_arg(args, unsigned long long *);

				*l = simple_mp_strtoull(mp_str, &next, mp_base);
			}
			break;
		case 'Z':
		case 'z':
			{
				size_t *s = (size_t *) va_arg(args, size_t *);

				*s = (size_t) simple_mp_strtoul(mp_str, &next, mp_base);
			}
			break;
		default:
			if (is_sign)
			{
				int *i = (int *) va_arg(args, int *);

				*i = (int) simple_mp_strtol(mp_str, &next, mp_base);
			}
			else
			{
				unsigned int *i = (unsigned int *) va_arg(args, unsigned int *);

				*i = (unsigned int) simple_mp_strtoul(mp_str, &next, mp_base);
			}
			break;
		}
		num++;

		if (!next)
			break;
		mp_str = next;
	}
	return num;
}


/**
 * mp_sscanf - Unformat a buffer into a list of arguments
 * @buf:        input buffer
 * @mp_fmt:        formatting of buffer
 * @...:        resulting arguments
 */
int mp_sscanf(const char *buf, const char *mp_fmt, ...)
{
	va_list args;
	int i;

	va_start(args, mp_fmt);
	i = mp_vsscanf(buf, mp_fmt, args);
	va_end(args);
	return i;
}

int mp_strcmp(const char *str1, const char *str2)
{
	int ret = 1;

	while ((*str1 != 0) && (*str2 != 0))
	{
		if (*str1 != *str2)
		{
			ret = 0;
			break;
		}
		else
		{
			str1++;
			str2++;
		}
	}

	return ret;
}

int mp_strlen(const char *str)
{
	int ret;

	for (ret = 0; str[ret] != 0; ret++);
	return ret;
}

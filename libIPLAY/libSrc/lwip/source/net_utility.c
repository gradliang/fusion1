#define LOCAL_DEBUG_ENABLE 1

#include <stdio.h>
#include <stdarg.h>
#include "typedef.h"
#include "linux/types.h"
#include "os.h"
#include "socket.h"
#include "net_socket.h"
#include "ndebug.h"

#define     DEBUGADD(l,body)    mpDebugPrint body

void bin2aschex(char *digits, unsigned long value, int len);
void *print_asc(int level, const uint8_t *buf,int len);

U16 InetCsum(void *start, U16 length)
{
    register U32 checksum=0;
    register U16 carry, count;
    register U16 *point;

//    DPrintf("InetCsum: start=0x%x", start);
    count = length >> 1;
    point = (U16*)start;
    
    while(count)
    {
        checksum += *point;
        point ++;
        count --;
    }
    
    if(length & 1)  checksum += (*(U08*)point << 8);
    
    /*
    checksum = (checksum >> 16) + (checksum & 0x0000ffff);
    if((checksum & 0xffff0000) != 0)
        checksum = (checksum >> 16) + (checksum & 0x0000ffff);
    */
    
    while(carry = (checksum >> 16))
        checksum = (checksum & 0x0000ffff) + carry;
        
    return (U16)~checksum;
}



inline U32 NetGetDW(U08* addr)
{
    return (((*((U08*)addr))<<24) | ((*(((U08*)addr)+1))<<16) | ((*(((U08*)addr)+2))<<8) | (*(((U08*)addr)+3)));
}



inline U16 NetGetW(U08* addr)
{
    return (((*((U08*)addr))<<8) | (*(((U08*)addr)+1)));
}



inline void NetPutDW(U08* addr, U32 value)
{
    *addr = (value >> 24) & 0xff;
    *(addr+1) = (value >> 16) & 0xff;
    *(addr+2) = (value >> 8) & 0xff;
    *(addr+3) = value & 0xff;
}



inline void NetPutW(U08* addr, U16 value)
{
    *addr = (value >> 8) & 0xff;
    *(addr+1) = value & 0xff;
}



BOOL NetMacAddrComp(U08* addr1, U08* addr2)
{
    U08 i;
    for(i = 0; i < 6; i++)
    {
        if(addr1[i] != addr2[i])
            return FALSE;
    }
    
    return TRUE;
}



void NetMacAddrCopy(U08* destAddr, U08* srcAddr)
{
    U08 i;
    for(i = 0; i < 6; i++)
        destAddr[i] = srcAddr[i];
}

void GetDebugPrintMac(BYTE *str, BYTE* pMac)
{
	sprintf(str,"%02x:%02x:%02x:%02x:%02x:%02x", pMac[0], pMac[1], pMac[2], pMac[3], pMac[4], pMac[5]);
}
	
void GetDebugPrintIP(BYTE *str,U32 ip)
{
	sprintf(str,"%u.%u.%u.%u", (ip>>24)&0xff, (ip>>16)&0xff, (ip>>8)&0xff, ip&0xff);
}

void NetDebugPrintIP(U32 ip)
{
    DPrintf("%u.%u.%u.%u", (ip>>24)&0xff, (ip>>16)&0xff, (ip>>8)&0xff, ip&0xff);
}



BOOL NetIpMaskCompare(U32 ip1, U32 ip2, U32 mask)
{
    ip1 &= mask;
    ip2 &= mask;
    
    if(ip1 == ip2)
        return TRUE;
    else
        return FALSE;
}



U32 StringToIp(U08* ipString)
{
    U32 ipAddr = 0;
    U08 i, j = 0;
    U08 tempVal;
    
    for(i = 0; i < 4; i++)
    {
        tempVal = 0;
        
        while((ipString[j] != '.') && (ipString[j] != 0))
        {
            if((ipString[j] > '9') || (ipString[j] < '0'))
            {
                DPrintf("[NET UTIL] invalid ip string");
                return 0;
            }
            
            tempVal = tempVal * 10 + (ipString[j] - '0');
            j++;
        }
        
        ipAddr = (ipAddr << 8) | tempVal;
        
        if(ipString[j] == '0')
            break;
            
        j++;
    }
    
    if(i != 4)
        return 0;
    else
        return ipAddr;
}



void IpToString(U32 ipAddr, U08* stringBuffer)
{
    sprintf(stringBuffer, "%d.%d.%d.%d", (ipAddr>>24)&0xff, (ipAddr>>16)&0xff, (ipAddr>>8)&0xff, ipAddr&0xff);
}



void NetDebugPrintThroughput(U32 totalLength, U32 timeInMS)
{
    U08* printBuf = mpx_Malloc(512);
    U08 type = 0;
    U32 intNum = 0, floatNum = 0;
    U32 rate;
    
    DPrintf(" ");
    
    printBuf[0] = 0;
    
    floatNum = (timeInMS % 1000) / 10;
    intNum = timeInMS / 1000;
    
    rate = (U32)(((U64)totalLength * 1000) / timeInMS);
    
    if(rate >= 1000)
    {
        rate /= 1000;
        sprintf(printBuf, "%d bytes transferred in %d.%02dseconds, %dKBytes/sec.", totalLength, intNum, floatNum, rate);
    }
    else
        sprintf(printBuf, "%d bytes transferred in %d.%02d seconds, %dBytes/sec.", totalLength, intNum, floatNum, rate);
    
    DPrintf("%s", printBuf);
    mpx_Free(printBuf);
}

/**
 * @ingroup NET_SOCKET
 * Check whether @p cp is a valid ASCII representation
 * of an Internet address and convert to a binary address.
 * Returns 1 if the address is valid, 0 if not.
 *
 * @param cp IP address in ASCII represenation (e.g. "127.0.0.1")
 * @param addr pointer to which to save the IP address in network order
 * @return 1 if @p cp could be converted to @p addr, 0 on failure
 */
int inet_aton(const char *cp, struct in_addr *addr)
{
  u32_t val;
  u8_t base;
  char c;
  u32_t parts[4];
  u32_t *pp = parts;

  c = *cp;
  for (;;) {
    /*
     * Collect number up to ``.''.
     * Values are specified as for C:
     * 0x=hex, 0=octal, 1-9=decimal.
     */
    if (!isdigit(c))
      return (0);
    val = 0;
    base = 10;
    if (c == '0') {
      c = *++cp;
      if (c == 'x' || c == 'X') {
        base = 16;
        c = *++cp;
      } else
        base = 8;
    }
    for (;;) {
      if (isdigit(c)) {
        val = (val * base) + (int)(c - '0');
        c = *++cp;
      } else if (base == 16 && isxdigit(c)) {
        val = (val << 4) | (int)(c + 10 - (islower(c) ? 'a' : 'A'));
        c = *++cp;
      } else
        break;
    }
    if (c == '.') {
      /*
       * Internet format:
       *  a.b.c.d
       *  a.b.c   (with c treated as 16 bits)
       *  a.b (with b treated as 24 bits)
       */
      if (pp >= parts + 3)
        return (0);
      *pp++ = val;
      c = *++cp;
    } else
      break;
  }
  /*
   * Check for trailing characters.
   */
  if (c != '\0' && !isspace(c))
    return (0);
  /*
   * Concoct the address according to
   * the number of parts specified.
   */
  switch (pp - parts + 1) {

  case 0:
    return (0);       /* initial nondigit */

  case 1:             /* a -- 32 bits */
    break;

  case 2:             /* a.b -- 8.24 bits */
    if (val > 0xffffffUL)
      return (0);
    val |= parts[0] << 24;
    break;

  case 3:             /* a.b.c -- 8.8.16 bits */
    if (val > 0xffff)
      return (0);
    val |= (parts[0] << 24) | (parts[1] << 16);
    break;

  case 4:             /* a.b.c.d -- 8.8.8.8 bits */
    if (val > 0xff)
      return (0);
    val |= (parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8);
    break;
  }
  if (addr)
    addr->s_addr = htonl(val);
  return (1);
}

/**
 * @ingroup NET_SOCKET
 *
 * Convert an Internet (IPv4) network address into a string in Internet 
 * standard dotted format, "a.b.c.d".
 *
 * @note This function is not re-entrant.
 */
char *inet_ntoa(struct in_addr in)
{
   static char str_inaddr[16];
   U08 *addr;

   addr = (U08 *)&in.s_addr;
   sprintf(str_inaddr, "%d.%d.%d.%d", addr[0], addr[1], addr[2], addr[3]);

   return str_inaddr;
}

/** 
 * @ingroup NET_SOCKET
 * Converts a string containing an (IPv4) dotted address into
 * a proper address for the <b>struct in_addr</b>.
 *
 * The function converts a string containing an (IPv4) Internet 
 * Protocol dotted address into a actual IP address in a binary 
 * representation.  The address is returned in network byte order.
 * 
 * If the string does not contain a legitimate IP address, <b>INADDR_NONE</b>
 * is returned.
 */
unsigned long inet_addr(const char *cp)
{
    const char *p = cp;
    U32 addr[4], maxvals[4];
    U32 net_addr = 0, last_val;
    int n = 0, i = 0;
    
    memset(maxvals, 0xFF, sizeof maxvals);
    
    while(1)
    {
        if (*p == 0 || *p == '.' || n >= 4)
            return INADDR_NONE;
        
        addr[n]  = 0;
        last_val = 0;
        
        if (n)
        {
            maxvals[n] = maxvals[n - 1] >> 8;
            maxvals[n - 1] = 255;
        }
        
        do 
        {
            if (!((*p >= '0')  &&  (*p <= '9')))
                return INADDR_NONE;
            
            addr[n] *= 10;
            addr[n] += (*p - '0');
            if (addr[n] < last_val)  /* overflow check  */
                return INADDR_NONE;
            last_val = addr[n];
            
            p++;
            
            if (*p == 0) goto stop;
        } while (*p != '.');
        
        p++;
        n++;
    } 
    
stop:
    
    for (i = 0; i <= n; i++)
        if (addr[n] > maxvals[n])
            return INADDR_NONE;
        
    net_addr = addr[n];
    if (n >= 1) net_addr += (addr[0] << 24);
    if (n >= 2) net_addr += (addr[1] << 16);
    if (n >= 3) net_addr += (addr[2] <<  8);

    return htonl(net_addr);
}

/* Convert from presentation format of an Internet number in buffer
   starting at CP to the binary network format and store result for
   interface type AF in buffer starting at BUF.  */
int inet_pton (int __af, __const char *__restrict __cp,
		      void *__restrict __buf)
{
    struct in_addr *ia;		/* Internet address.  */
    switch (__af)
    {
        case AF_INET:
            inet_aton(__cp, (struct in_addr *)__buf);
            break;
    }

}

/* Convert a Internet address in binary network format for interface
   type AF in buffer starting at CP to presentation form and place
   result in buffer of length LEN astarting at BUF.  */
const char *inet_ntop (int __af, __const void *__restrict __cp,
				char *__restrict __buf, socklen_t __len)
{
    struct in_addr *ia;		/* Internet address.  */

    switch (__af)
    {
        case AF_INET:
            ia = (struct in_addr *)__cp;
            strncpy(__buf, inet_ntoa(*ia), __len);
            __buf[__len -1] = '\0';
            break;
    }


}

/* 
 * A better printf than DPrintf()
 */
#if 0
void DPRINTF(const char * format, ...)
{
	va_list ap;
    char buf[256];
    int len = strlen(format);

	va_start(ap, format);
    vsprintf(buf, format, ap);
	va_end(ap);

#ifndef MP600
    DPrintf("%s", buf);
#else
    mpDebugPrint("%s", buf);
#endif
}
#endif
/**
 * wpa_ssid_txt - Convert SSID to a printable string
 * @ssid: SSID (32-octet string)
 * @ssid_len: Length of ssid in octets
 * Returns: Pointer to a printable string
 *
 * This function can be used to convert SSIDs into printable form. In most
 * cases, SSIDs do not use unprintable characters, but IEEE 802.11 standard
 * does not limit the used character set, so anything could be used in an SSID.
 *
 * This function is based on wpa_supplicant's wpa_ssid_txt().  One difference is
 * wpa_ssid_string doesn't use static variable.  The destination SSID string 
 * buffer is passed as the parameter, ssid_txt.  Make sure ssid_txt is at 
 * least 33 octets long.
 */
const char * wpa_ssid_string(char *ssid_txt, u8 *ssid, size_t ssid_len)
{
	char *pos;
	static char ssid_buf[33];

	if (ssid_len > 32)
		ssid_len = 32;
	if (!ssid_txt)
		ssid_txt = ssid_buf;
	memcpy(ssid_txt, ssid, ssid_len);
	ssid_txt[ssid_len] = '\0';
	for (pos = ssid_txt; *pos != '\0'; pos++) {
		if ((u8) *pos < 32 || (u8) *pos >= 127)
			*pos = '_';
	}
	return ssid_txt;
}

void udelay(unsigned long usecs)
{
	unsigned long xi;

#if 1
    IODelay(usecs);
#else
	for (xi = 0; xi < usecs; xi++)
		__asm("nop");
#endif
}
void mdelay(unsigned long msecs)
{
	unsigned long xi;

	for (xi = 0; xi < msecs; xi++)
		udelay(1000);
}

/**
 * msleep - sleep 
 * @msecs: Time in milliseconds to sleep for
 */
void msleep(unsigned int msecs)
{
	TaskSleep(msecs);
}

static int net_dump_level = 255;
static void _NetDumpData(int level, const uint8_t *buf, int len,
		       bool omit_zero_bytes)
{
	int i=0;
	const uint8_t empty[16];
	bool skipped = false;
	char str_line[64];
	int k=0;
	unsigned long val;

    mpDebugPrint("%s: len=%d", __func__, len);
	if (len<=0) return;

	if (level > net_dump_level) return;

	memset(&empty, '\0', 16);

	for (i=0;i<len;) {

		if (i%16 == 0) {
			if ((omit_zero_bytes == true) &&
			    (i > 0) &&
			    (len > i+16) &&
			    (memcmp(&buf[i], &empty, 16) == 0))
			{
				i +=16;
				continue;
			}

			if (i<len)  {
				k = 0;
				snprintf(&str_line[k], sizeof(str_line), "[%04X] ", i);
				k += 7;
			}
		}

		i++;
		if (i%16 == 0) {

			memcpy(&val, &buf[i-16], sizeof(val));
			bin2aschex(&str_line[k], val, 4);
			k += 8;

			memcpy(&val, &buf[i-12], sizeof(val));
			bin2aschex(&str_line[k], val, 4);
			k += 8;

			str_line[k++] = ' ';
			str_line[k++] = ' ';
			memcpy(&val, &buf[i-8], sizeof(val));
			bin2aschex(&str_line[k], val, 4);
			k += 8;

			memcpy(&val, &buf[i-4], sizeof(val));
			bin2aschex(&str_line[k], val, 4);
			k += 8;

			str_line[k++] = ' ';
			str_line[k++] = ' ';

			memcpy(&str_line[k], print_asc(level,&buf[i-16],8), 8);
            k += 8;
            str_line[k++] = ' ';
			memcpy(&str_line[k], print_asc(level,&buf[i-8],8), 8);
            k += 8;
            str_line[k] = '\0';
			DEBUGADD(level,("%s", str_line));

			if ((omit_zero_bytes == true) &&
			    (len > i+16) &&
			    (memcmp(&buf[i], &empty, 16) == 0)) {
				if (!skipped) {
					DEBUGADD(level,("skipping zero buffer bytes\n"));
					skipped = true;
				}
			}
		}
	}

	if (i%16) {
		int n;
		n = 16 - (i%16);
		int m;
		m = i%16;
		while (m >= 4)
		{
			memcpy(&val, &buf[i-m], sizeof(val));
			bin2aschex(&str_line[k], val, 4);
			k += 8;
			m -= 4;
			if ((i%16) == (m + 8)) { str_line[k++] = ' '; str_line[k++] = ' '; }
		}
//	__asm("break 100");
		if (m > 0)
		{
			memcpy(&val, &buf[i-m], m);
			bin2aschex(&str_line[k], val, m);
			k += m*2;
		}
		if (n>8) str_line[k++] = ' ';
		while (n--) { str_line[k++] = ' '; str_line[k++] = ' '; str_line[k++] = ' ';}
		n = MIN(8,i%16);
		memcpy(&str_line[k], print_asc(level,&buf[i-(i%16)],n), n);
		k += n;
		str_line[k++] = ' ';
		n = (i%16) - n;
		if (n>0) { memcpy(&str_line[k], print_asc(level,&buf[i-n],n), n); k += n; }
		str_line[k++] = '\0';
		DEBUGADD(level,("%s", str_line));
	}

}

void NetDumpData(int level, const uint8_t *buf, int len)
{
	_NetDumpData(level, buf, len, false);
}

#ifdef LINUX
/*
 * little endian
 */
void bin2aschex(char *digits, unsigned long value, int len)
{
    int size = len * 2;
    unsigned int hexDigit;
    unsigned int factor = 0x37;

    while (size >0)
    {
        hexDigit = value % 16;
        if (hexDigit < 10)
            digits[size-1] = hexDigit + 0x30;
        else
            digits[size-1] = hexDigit + factor;
        value = value / 16;
        size--;
    }
    return;
}
#else
/*
 * big endian
 */
void bin2aschex(char *digits, unsigned long value, int len)
{
    int size = len * 2;
    unsigned int hexDigit;
//    unsigned int factor = 0x37;
    unsigned int factor = 0x57;                 /* use small case */
    short i = 0;

    while (len < 4)
    {
        value = value / 256;
        len++;
    }
    while (size >0)
    {
        hexDigit = value % 16;
        if (hexDigit < 10)
            digits[size-1] = hexDigit + 0x30;
        else
            digits[size-1] = hexDigit + factor;
        value = value / 16;
        i++;
        size--;
    }
    return;
}
#endif

void *print_asc(int level, const uint8_t *buf,int len)
{
	int i;
    static char str[1024];
	MP_ASSERT(len < 1024);
	for (i=0;i<len;i++)
        str[i] = isprint(buf[i])?buf[i]:'.';
//	__asm("break 100");
    return str;
}

void NetDie(const char *file,int line,const char *assertion)
{
    mpDebugPrint("NET_ASSERT FAILED '%s' "
			       "%s:%d\n", assertion, file, line);
    BREAK_POINT();
}
const char *gai_strerror(int ecode)
{

    return "gai_strerror";
}

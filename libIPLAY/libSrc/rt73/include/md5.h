/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2006, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************
	Module Name:
	md5.h

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	jan			10-28-03		Initial
	Rita		11-23-04		Modify MD5 and SHA-1
*/



#ifndef	__MD5_H__
#define	__MD5_H__

#define MD5_MAC_LEN 16

typedef struct _MD5_CTX {
    unsigned int   Buf[4];             // buffers of four states
	unsigned char   Input[64];          // input message
	unsigned int   LenInBitCount[2];   // length counter for input message, 0 up to 64 bits	                            
}   MD5_CTX;

VOID MD5Init(MD5_CTX *pCtx);
VOID MD5Update(MD5_CTX *pCtx, unsigned char *pData, unsigned int LenInBytes);
VOID MD5Final(unsigned char Digest[16], MD5_CTX *pCtx);
VOID MD5Transform(unsigned int Buf[4], unsigned int Mes[16]);

void md5_mac(unsigned char *key, size_t key_len, unsigned char *data, size_t data_len, unsigned char *mac);
void hmac_md5(unsigned char *key, size_t key_len, unsigned char *data, size_t data_len, unsigned char *mac);

//
// SHA context
//
typedef	struct _SHA_CTX
{
	unsigned int   Buf[5];             // buffers of five states
	unsigned char   Input[80];          // input message
	unsigned int   LenInBitCount[2];   // length counter for input message, 0 up to 64 bits
	
}	SHA_CTX;

VOID SHAInit(SHA_CTX *pCtx);
unsigned char SHAUpdate(SHA_CTX *pCtx, unsigned char *pData, unsigned int LenInBytes);
VOID SHAFinal(SHA_CTX *pCtx, unsigned char Digest[20]);
VOID SHATransform(unsigned int Buf[5], unsigned int Mes[20]);

#define SHA_DIGEST_LEN 20
#endif // __MD5_H__

/******************************************************************************/
#ifndef	_AES_H
#define	_AES_H

typedef	struct
{
	unsigned int erk[64];		/* encryption round	keys */
	unsigned int drk[64];		/* decryption round	keys */
	int	nr;				/* number of rounds	*/
}
aes_context;

int	 aes_set_key( aes_context *ctx,	unsigned char *key,	int	nbits );
void aes_encrypt( aes_context *ctx,	unsigned char input[16], unsigned char output[16] );
void aes_decrypt( aes_context *ctx,	unsigned char input[16], unsigned char output[16] );

void F(char *password, unsigned char *ssid, int ssidlength, int iterations, int count, unsigned char *output);
int PasswordHash(char *password, unsigned char *ssid, int ssidlength, unsigned char *output);

#endif /* aes.h	*/


/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0 

#include "global612.h"
#include "mpTrace.h"
#include "netware.h"
#include "net_sys.h"
 
static Net_Photo_Entry jpeg_info;

static const char   *mpx_base_dir;


int SmbCli_GetShares(char *url);
int smb_init(const char *url, const char *base_dir)
{
    int ret = 0;
    
    mpx_base_dir = base_dir;

#ifdef HAVE_SMB
	SmbCli_GetShares(url);
#endif
	return ret;
}

void smb_exit(const char *base_dir)
{
}

int smb_add_file(char *fname, char *url)
{
	mpDebugPrint("%s: fname=%s,url=%s", __func__, fname, url);
//	snprintf(jpeg_info.pathname, NETFS_MAX_PATHNAME, "%s/%s",  mpx_base_dir, fname);
	snprintf(jpeg_info.pathname, NETFS_MAX_PATHNAME, "%s",  fname);
	snprintf(jpeg_info.url, NETFS_MAX_URL, "%s",url);
	jpeg_info.length[0] = '0';
	jpeg_info.length[0] = '\0';

	Net_Xml_parseFileList(&jpeg_info);
	return 0;
}



#include <net/if.h>

#include <netlink-local.h>
#include <netlink/netlink.h>
#include <netlink/utils.h>
#include <netlink/handlers.h>

#ifdef PLATFORM_MPIXEL
#include "mpTrace_copy.h"
#include "log.h"
#endif

/**
 * @name Callback Handle Management
 * @{
 */

/** @} */

/**
 * @name Callback Setup
 * @{
 */

/** @} */

/** @} */


/* if_indextoname - map a network interface index to its corresponding name */
char *if_indextoname(unsigned ifindex, char *ifname)
{
    if (ifindex == 1)
    {
        strncpy(ifname, "wlan0", IF_NAMESIZE-1);
        return ifname;
    }

    return NULL;
}

/* map a network interface name to its corresponding index */
unsigned if_nametoindex(const char *ifname)
{
    if (strcmp(ifname, "wlan0") == 0)
		return 2;   /* NIC_INDEX_WIFI */
    else if (strcmp(ifname, "mon.wlan0") == 0)
        return 1;
    else
        MP_ASSERT(0);
}

struct protoent *getprotobynumber(int proto)
{
    return 0;
}
struct protoent *getprotobyname(const char *name)
{
    return 0;
}

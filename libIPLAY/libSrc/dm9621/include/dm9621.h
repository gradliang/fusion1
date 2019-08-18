
#ifndef __DM9621_H
#define __DM9621_H

struct dm9621_statistics {
	u32 rx_pkts;
	u32 rx_ucast;
	u32 rx_bcast;
	u32 rx_dropped;
};

extern struct dm9621_statistics dm9621_stat;

#endif


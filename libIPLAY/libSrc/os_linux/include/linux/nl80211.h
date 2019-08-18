
#ifndef __LINUX_NL80211_H
#define __LINUX_NL80211_H

/**
 * enum nl80211_sta_flags - station flags
 *
 * Station flags. When a station is added to an AP interface, it is
 * assumed to be already associated (and hence authenticated.)
 *
 * @NL80211_STA_FLAG_AUTHORIZED: station is authorized (802.1X)
 * @NL80211_STA_FLAG_SHORT_PREAMBLE: station is capable of receiving frames
 *	with short barker preamble
 * @NL80211_STA_FLAG_WME: station is WME/QoS capable
 */
enum nl80211_sta_flags {
	__NL80211_STA_FLAG_INVALID,
	NL80211_STA_FLAG_AUTHORIZED,
	NL80211_STA_FLAG_SHORT_PREAMBLE,
	NL80211_STA_FLAG_WME,

	/* keep last */
	__NL80211_STA_FLAG_AFTER_LAST,
	NL80211_STA_FLAG_MAX = __NL80211_STA_FLAG_AFTER_LAST - 1
};

/**
 * enum nl80211_mpath_info - mesh path information
 *
 * These attribute types are used with %NL80211_ATTR_MPATH_INFO when getting
 * information about a mesh path.
 *
 * @__NL80211_MPATH_INFO_INVALID: attribute number 0 is reserved
 * @NL80211_ATTR_MPATH_FRAME_QLEN: number of queued frames for this destination
 * @NL80211_ATTR_MPATH_DSN: destination sequence number
 * @NL80211_ATTR_MPATH_METRIC: metric (cost) of this mesh path
 * @NL80211_ATTR_MPATH_EXPTIME: expiration time for the path, in msec from now
 * @NL80211_ATTR_MPATH_FLAGS: mesh path flags, enumerated in
 * 	&enum nl80211_mpath_flags;
 * @NL80211_ATTR_MPATH_DISCOVERY_TIMEOUT: total path discovery timeout, in msec
 * @NL80211_ATTR_MPATH_DISCOVERY_RETRIES: mesh path discovery retries
 */
enum nl80211_mpath_info {
	__NL80211_MPATH_INFO_INVALID,
	NL80211_MPATH_INFO_FRAME_QLEN,
	NL80211_MPATH_INFO_DSN,
	NL80211_MPATH_INFO_METRIC,
	NL80211_MPATH_INFO_EXPTIME,
	NL80211_MPATH_INFO_FLAGS,
	NL80211_MPATH_INFO_DISCOVERY_TIMEOUT,
	NL80211_MPATH_INFO_DISCOVERY_RETRIES,

	/* keep last */
	__NL80211_MPATH_INFO_AFTER_LAST,
	NL80211_MPATH_INFO_MAX = __NL80211_MPATH_INFO_AFTER_LAST - 1
};

/**
 * enum nl80211_mntr_flags - monitor configuration flags
 *
 * Monitor configuration flags.
 *
 * @__NL80211_MNTR_FLAG_INVALID: reserved
 *
 * @NL80211_MNTR_FLAG_FCSFAIL: pass frames with bad FCS
 * @NL80211_MNTR_FLAG_PLCPFAIL: pass frames with bad PLCP
 * @NL80211_MNTR_FLAG_CONTROL: pass control frames
 * @NL80211_MNTR_FLAG_OTHER_BSS: disable BSSID filtering
 * @NL80211_MNTR_FLAG_COOK_FRAMES: report frames after processing.
 *	overrides all other flags.
 *
 * @__NL80211_MNTR_FLAG_AFTER_LAST: internal use
 * @NL80211_MNTR_FLAG_MAX: highest possible monitor flag
 */
enum nl80211_mntr_flags {
	__NL80211_MNTR_FLAG_INVALID,
	NL80211_MNTR_FLAG_FCSFAIL,
	NL80211_MNTR_FLAG_PLCPFAIL,
	NL80211_MNTR_FLAG_CONTROL,
	NL80211_MNTR_FLAG_OTHER_BSS,
	NL80211_MNTR_FLAG_COOK_FRAMES,

	/* keep last */
	__NL80211_MNTR_FLAG_AFTER_LAST,
	NL80211_MNTR_FLAG_MAX = __NL80211_MNTR_FLAG_AFTER_LAST - 1
};

/**
 * enum nl80211_iftype - (virtual) interface types
 *
 * @NL80211_IFTYPE_UNSPECIFIED: unspecified type, driver decides
 * @NL80211_IFTYPE_ADHOC: independent BSS member
 * @NL80211_IFTYPE_STATION: managed BSS member
 * @NL80211_IFTYPE_AP: access point
 * @NL80211_IFTYPE_AP_VLAN: VLAN interface for access points
 * @NL80211_IFTYPE_WDS: wireless distribution interface
 * @NL80211_IFTYPE_MONITOR: monitor interface receiving all frames
 * @NL80211_IFTYPE_MESH_POINT: mesh point
 * @NL80211_IFTYPE_MAX: highest interface type number currently defined
 * @__NL80211_IFTYPE_AFTER_LAST: internal use
 *
 * These values are used with the %NL80211_ATTR_IFTYPE
 * to set the type of an interface.
 *
 */
enum nl80211_iftype {
	NL80211_IFTYPE_UNSPECIFIED,
	NL80211_IFTYPE_ADHOC,
	NL80211_IFTYPE_STATION,
	NL80211_IFTYPE_AP,
	NL80211_IFTYPE_AP_VLAN,
	NL80211_IFTYPE_WDS,
	NL80211_IFTYPE_MONITOR,
	NL80211_IFTYPE_MESH_POINT,

	/* keep last */
	__NL80211_IFTYPE_AFTER_LAST,
	NL80211_IFTYPE_MAX = __NL80211_IFTYPE_AFTER_LAST - 1
};

#define NL80211_MAX_SUPP_RATES			32
#define NL80211_TKIP_DATA_OFFSET_ENCR_KEY	0
#define NL80211_TKIP_DATA_OFFSET_TX_MIC_KEY	16
#define NL80211_TKIP_DATA_OFFSET_RX_MIC_KEY	24
#endif /* __LINUX_NL80211_H */


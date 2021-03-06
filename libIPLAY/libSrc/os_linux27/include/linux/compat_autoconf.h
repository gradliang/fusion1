#ifndef COMPAT_AUTOCONF_INCLUDED
#define COMPAT_AUTOCONF_INCLUDED
/*
 * Automatically generated C config: don't edit
 * Fri Apr 30 17:42:03 IST 2010 
 * compat-wireless-2.6: next-20100415-1-g1ad9cb7
 * linux-2.6: 3.0.0.4-2-ga881bd0
 */
#define COMPAT_RELEASE "next-20100415-1-g1ad9cb7"
#define COMPAT_KERNEL_RELEASE "3.0.0.4-2-ga881bd0"
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,25))
#error Compat-wireless requirement: Linux >= 2,6,25
#endif /* (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,25) */ 
#ifndef CONFIG_WIRELESS_EXT
#error Compat-wireless requirement: CONFIG_WIRELESS_EXT must be enabled in your kernel
#endif /* CONFIG_WIRELESS_EXT */
#ifndef CONFIG_MAC80211
#define CONFIG_MAC80211 1
#endif /* CONFIG_MAC80211 */ 
#ifndef CONFIG_MAC80211_DEBUGFS
//#define CONFIG_MAC80211_DEBUGFS 1
#endif /* CONFIG_MAC80211_DEBUGFS */ 
#ifndef CONFIG_MAC80211_HT_DEBUG
#define CONFIG_MAC80211_HT_DEBUG 1
#endif /* CONFIG_MAC80211_HT_DEBUG */ 
#ifndef CONFIG_MAC80211_DEBUG_COUNTERS
#define CONFIG_MAC80211_DEBUG_COUNTERS 1
#endif /* CONFIG_MAC80211_DEBUG_COUNTERS */ 
#ifndef CONFIG_MAC80211_RC_DEFAULT
#define CONFIG_MAC80211_RC_DEFAULT "minstrel"
#endif /* CONFIG_MAC80211_RC_DEFAULT */ 
#ifndef CONFIG_MAC80211_RC_DEFAULT_MINSTREL
#define CONFIG_MAC80211_RC_DEFAULT_MINSTREL 1
#endif /* CONFIG_MAC80211_RC_DEFAULT_MINSTREL */ 
#ifndef CONFIG_COMPAT_MAC80211_RC_DEFAULT
#define CONFIG_COMPAT_MAC80211_RC_DEFAULT "minstrel"
#endif /* CONFIG_COMPAT_MAC80211_RC_DEFAULT */ 
#ifndef CONFIG_MAC80211_RC_PID
#define CONFIG_MAC80211_RC_PID 1
#endif /* CONFIG_MAC80211_RC_PID */ 
#ifndef CONFIG_MAC80211_RC_MINSTREL
#define CONFIG_MAC80211_RC_MINSTREL 1
#endif /* CONFIG_MAC80211_RC_MINSTREL */ 
#ifndef CONFIG_MAC80211_LEDS
#define CONFIG_MAC80211_LEDS 1
#endif /* CONFIG_MAC80211_LEDS */ 
#ifndef CONFIG_MAC80211_MESH
//#define CONFIG_MAC80211_MESH 1
#endif /* CONFIG_MAC80211_MESH */ 
#ifndef CONFIG_CFG80211
#define CONFIG_CFG80211 1
#endif /* CONFIG_CFG80211 */ 
#ifndef CONFIG_CFG80211_DEFAULT_PS
#define CONFIG_CFG80211_DEFAULT_PS 1
#endif /* CONFIG_CFG80211_DEFAULT_PS */ 
#ifndef CONFIG_LIB80211
#define CONFIG_LIB80211 1
#endif /* CONFIG_LIB80211 */ 
#ifndef CONFIG_LIB80211_CRYPT_WEP
#define CONFIG_LIB80211_CRYPT_WEP 1
#endif /* CONFIG_LIB80211_CRYPT_WEP */ 
#ifndef CONFIG_LIB80211_CRYPT_CCMP
#define CONFIG_LIB80211_CRYPT_CCMP 1
#endif /* CONFIG_LIB80211_CRYPT_CCMP */ 
#ifndef CONFIG_LIB80211_CRYPT_TKIP
#define CONFIG_LIB80211_CRYPT_TKIP 1
#endif /* CONFIG_LIB80211_CRYPT_TKIP */ 
#ifndef CONFIG_BT
#define CONFIG_BT 1
#endif /* CONFIG_BT */ 
#ifndef CONFIG_BT_L2CAP
#define CONFIG_BT_L2CAP 1
#endif /* CONFIG_BT_L2CAP */ 
#ifndef CONFIG_BT_SCO
#define CONFIG_BT_SCO 1
#endif /* CONFIG_BT_SCO */ 
#ifndef CONFIG_BT_RFCOMM
#define CONFIG_BT_RFCOMM 1
#endif /* CONFIG_BT_RFCOMM */ 
#ifndef CONFIG_BT_BNEP
#define CONFIG_BT_BNEP 1
#endif /* CONFIG_BT_BNEP */ 
#ifndef CONFIG_BT_ATH3K
#define CONFIG_BT_ATH3K 1
#endif /* CONFIG_BT_ATH3K */ 
#ifndef CONFIG_BT_CMTP
#define CONFIG_BT_CMTP 1
#endif /* CONFIG_BT_CMTP */ 
#ifndef CONFIG_BT_HIDP
#define CONFIG_BT_HIDP 1
#endif /* CONFIG_BT_HIDP */ 
#ifndef CONFIG_CFG80211_WEXT
#define CONFIG_CFG80211_WEXT 1
#endif /* CONFIG_CFG80211_WEXT */ 
#ifndef CONFIG_MAC80211_HWSIM
#define CONFIG_MAC80211_HWSIM 1
#endif /* CONFIG_MAC80211_HWSIM */ 
#ifndef CONFIG_ATH5K
#define CONFIG_ATH5K 1
#endif /* CONFIG_ATH5K */ 
#ifndef CONFIG_ATH9K_HW
#define CONFIG_ATH9K_HW 1
#endif /* CONFIG_ATH9K_HW */ 
#ifndef CONFIG_ATH9K
#define CONFIG_ATH9K 1
#endif /* CONFIG_ATH9K */ 
#ifndef CONFIG_ATH9K_COMMON
#define CONFIG_ATH9K_COMMON 1
#endif /* CONFIG_ATH9K_COMMON */ 
#ifndef CONFIG_ATH9K_DEBUGFS
#define CONFIG_ATH9K_DEBUGFS 1
#endif /* CONFIG_ATH9K_DEBUGFS */ 
#ifndef CONFIG_IWLWIFI
#define CONFIG_IWLWIFI 1
#endif /* CONFIG_IWLWIFI */ 
#ifndef CONFIG_IWLAGN
#define CONFIG_IWLAGN 1
#endif /* CONFIG_IWLAGN */ 
#ifndef CONFIG_COMPAT_IWL4965
#define CONFIG_COMPAT_IWL4965 1
#endif /* CONFIG_COMPAT_IWL4965 */ 
#ifndef CONFIG_IWL5000
#define CONFIG_IWL5000 1
#endif /* CONFIG_IWL5000 */ 
#ifndef CONFIG_IWL3945
#define CONFIG_IWL3945 1
#endif /* CONFIG_IWL3945 */ 
#ifndef CONFIG_B43
#define CONFIG_B43 1
#endif /* CONFIG_B43 */ 
#ifndef CONFIG_B43_HWRNG
#define CONFIG_B43_HWRNG 1
#endif /* CONFIG_B43_HWRNG */ 
#ifndef CONFIG_B43_PCI_AUTOSELECT
#define CONFIG_B43_PCI_AUTOSELECT 1
#endif /* CONFIG_B43_PCI_AUTOSELECT */ 
#ifndef CONFIG_B43_PCMCIA
#define CONFIG_B43_PCMCIA 1
#endif /* CONFIG_B43_PCMCIA */ 
#ifndef CONFIG_B43_LEDS
#define CONFIG_B43_LEDS 1
#endif /* CONFIG_B43_LEDS */ 
#ifndef CONFIG_B43_PHY_LP
#define CONFIG_B43_PHY_LP 1
#endif /* CONFIG_B43_PHY_LP */ 
#ifndef CONFIG_B43_NPHY
#define CONFIG_B43_NPHY 1
#endif /* CONFIG_B43_NPHY */ 
#ifndef CONFIG_B43LEGACY
#define CONFIG_B43LEGACY 1
#endif /* CONFIG_B43LEGACY */ 
#ifndef CONFIG_B43LEGACY_HWRNG
#define CONFIG_B43LEGACY_HWRNG 1
#endif /* CONFIG_B43LEGACY_HWRNG */ 
#ifndef CONFIG_B43LEGACY_PCI_AUTOSELECT
#define CONFIG_B43LEGACY_PCI_AUTOSELECT 1
#endif /* CONFIG_B43LEGACY_PCI_AUTOSELECT */ 
#ifndef CONFIG_B43LEGACY_LEDS
#define CONFIG_B43LEGACY_LEDS 1
#endif /* CONFIG_B43LEGACY_LEDS */ 
#ifndef CONFIG_B43LEGACY_DMA
#define CONFIG_B43LEGACY_DMA 1
#endif /* CONFIG_B43LEGACY_DMA */ 
#ifndef CONFIG_B43LEGACY_PIO
#define CONFIG_B43LEGACY_PIO 1
#endif /* CONFIG_B43LEGACY_PIO */ 
#ifndef CONFIG_LIBIPW
#define CONFIG_LIBIPW 1
#endif /* CONFIG_LIBIPW */ 
#ifndef CONFIG_IPW2100
#define CONFIG_IPW2100 1
#endif /* CONFIG_IPW2100 */ 
#ifndef CONFIG_IPW2100_MONITOR
#define CONFIG_IPW2100_MONITOR 1
#endif /* CONFIG_IPW2100_MONITOR */ 
#ifndef CONFIG_IPW2200
#define CONFIG_IPW2200 1
#endif /* CONFIG_IPW2200 */ 
#ifndef CONFIG_IPW2200_MONITOR
#define CONFIG_IPW2200_MONITOR 1
#endif /* CONFIG_IPW2200_MONITOR */ 
#ifndef CONFIG_IPW2200_RADIOTAP
#define CONFIG_IPW2200_RADIOTAP 1
#endif /* CONFIG_IPW2200_RADIOTAP */ 
#ifndef CONFIG_IPW2200_PROMISCUOUS
#define CONFIG_IPW2200_PROMISCUOUS 1
#endif /* CONFIG_IPW2200_PROMISCUOUS */ 
#ifndef CONFIG_IPW2200_QOS
#define CONFIG_IPW2200_QOS 1
#endif /* CONFIG_IPW2200_QOS */ 
#ifndef CONFIG_SSB_SPROM
#define CONFIG_SSB_SPROM 1
#endif /* CONFIG_SSB_SPROM */ 
#ifndef CONFIG_SSB_BLOCKIO
#define CONFIG_SSB_BLOCKIO 1
#endif /* CONFIG_SSB_BLOCKIO */ 
#ifndef CONFIG_SSB_PCIHOST
#define CONFIG_SSB_PCIHOST 1
#endif /* CONFIG_SSB_PCIHOST */ 
#ifndef CONFIG_SSB_B43_PCI_BRIDGE
#define CONFIG_SSB_B43_PCI_BRIDGE 1
#endif /* CONFIG_SSB_B43_PCI_BRIDGE */ 
#ifndef CONFIG_SSB_PCMCIAHOST
#define CONFIG_SSB_PCMCIAHOST 1
#endif /* CONFIG_SSB_PCMCIAHOST */ 
#ifndef CONFIG_SSB_DRIVER_PCICORE
#define CONFIG_SSB_DRIVER_PCICORE 1
#endif /* CONFIG_SSB_DRIVER_PCICORE */ 
#ifndef CONFIG_P54_PCI
#define CONFIG_P54_PCI 1
#endif /* CONFIG_P54_PCI */ 
#ifndef CONFIG_B44
#define CONFIG_B44 1
#endif /* CONFIG_B44 */ 
#ifndef CONFIG_B44_PCI
#define CONFIG_B44_PCI 1
#endif /* CONFIG_B44_PCI */ 
#ifndef CONFIG_RTL8180
#define CONFIG_RTL8180 1
#endif /* CONFIG_RTL8180 */ 
#ifndef CONFIG_ADM8211
#define CONFIG_ADM8211 1
#endif /* CONFIG_ADM8211 */ 
#ifndef CONFIG_RT2X00_LIB_PCI
#define CONFIG_RT2X00_LIB_PCI 1
#endif /* CONFIG_RT2X00_LIB_PCI */ 
#ifndef CONFIG_RT2400PCI
#define CONFIG_RT2400PCI 1
#endif /* CONFIG_RT2400PCI */ 
#ifndef CONFIG_RT2500PCI
#define CONFIG_RT2500PCI 1
#endif /* CONFIG_RT2500PCI */ 
#ifndef CONFIG_RT2800PCI
#define CONFIG_RT2800PCI 1
#endif /* CONFIG_RT2800PCI */ 
#ifndef CONFIG_RT2800PCI_PCI
#define CONFIG_RT2800PCI_PCI 1
#endif /* CONFIG_RT2800PCI_PCI */ 
#ifndef CONFIG_RT61PCI
#define CONFIG_RT61PCI 1
#endif /* CONFIG_RT61PCI */ 
#ifndef CONFIG_MWL8K
#define CONFIG_MWL8K 1
#endif /* CONFIG_MWL8K */ 
#ifndef CONFIG_ATL1
#define CONFIG_ATL1 1
#endif /* CONFIG_ATL1 */ 
#ifndef CONFIG_ATL2
#define CONFIG_ATL2 1
#endif /* CONFIG_ATL2 */ 
#ifndef CONFIG_ATL1E
#define CONFIG_ATL1E 1
#endif /* CONFIG_ATL1E */ 
#ifndef CONFIG_ATL1C
#define CONFIG_ATL1C 1
#endif /* CONFIG_ATL1C */ 
#ifndef CONFIG_HERMES
#define CONFIG_HERMES 1
#endif /* CONFIG_HERMES */ 
#ifndef CONFIG_HERMES_CACHE_FW_ON_INIT
#define CONFIG_HERMES_CACHE_FW_ON_INIT 1
#endif /* CONFIG_HERMES_CACHE_FW_ON_INIT */ 
#ifndef CONFIG_APPLE_AIRPORT
#define CONFIG_APPLE_AIRPORT 1
#endif /* CONFIG_APPLE_AIRPORT */ 
#ifndef CONFIG_PLX_HERMES
#define CONFIG_PLX_HERMES 1
#endif /* CONFIG_PLX_HERMES */ 
#ifndef CONFIG_TMD_HERMES
#define CONFIG_TMD_HERMES 1
#endif /* CONFIG_TMD_HERMES */ 
#ifndef CONFIG_NORTEL_HERMES
#define CONFIG_NORTEL_HERMES 1
#endif /* CONFIG_NORTEL_HERMES */ 
#ifndef CONFIG_PCI_HERMES
#define CONFIG_PCI_HERMES 1
#endif /* CONFIG_PCI_HERMES */ 
#ifndef CONFIG_PCMCIA_HERMES
#define CONFIG_PCMCIA_HERMES 1
#endif /* CONFIG_PCMCIA_HERMES */ 
#ifndef CONFIG_PCMCIA_SPECTRUM
#define CONFIG_PCMCIA_SPECTRUM 1
#endif /* CONFIG_PCMCIA_SPECTRUM */ 
#undef CONFIG_LIBERTAS
#undef CONFIG_LIBERTAS_CS
#ifndef CONFIG_LIBERTAS_CS
#define CONFIG_LIBERTAS_CS 1
#endif /* CONFIG_LIBERTAS_CS */ 
#ifndef CONFIG_EEPROM_93CX6
#define CONFIG_EEPROM_93CX6 1
#endif /* CONFIG_EEPROM_93CX6 */ 
#ifndef CONFIG_ZD1211RW
#define CONFIG_ZD1211RW 1
#endif /* CONFIG_ZD1211RW */ 
#undef CONFIG_USB_COMPAT_USBNET
#undef CONFIG_USB_NET_COMPAT_RNDIS_HOST
#undef CONFIG_USB_NET_COMPAT_RNDIS_WLAN
#undef CONFIG_USB_NET_COMPAT_CDCETHER
#ifndef CONFIG_USB_COMPAT_USBNET
#define CONFIG_USB_COMPAT_USBNET 1
#endif /* CONFIG_USB_COMPAT_USBNET */ 
#ifndef CONFIG_USB_NET_COMPAT_RNDIS_HOST
#define CONFIG_USB_NET_COMPAT_RNDIS_HOST 1
#endif /* CONFIG_USB_NET_COMPAT_RNDIS_HOST */ 
#ifndef CONFIG_USB_NET_COMPAT_RNDIS_WLAN
#define CONFIG_USB_NET_COMPAT_RNDIS_WLAN 1
#endif /* CONFIG_USB_NET_COMPAT_RNDIS_WLAN */ 
#ifndef CONFIG_USB_NET_COMPAT_CDCETHER
#define CONFIG_USB_NET_COMPAT_CDCETHER 1
#endif /* CONFIG_USB_NET_COMPAT_CDCETHER */ 
#ifndef CONFIG_P54_USB
#define CONFIG_P54_USB 1
#endif /* CONFIG_P54_USB */ 
#ifndef CONFIG_RTL8187
#define CONFIG_RTL8187 1
#endif /* CONFIG_RTL8187 */ 
#ifndef CONFIG_RTL8187_LEDS
#define CONFIG_RTL8187_LEDS 1
#endif /* CONFIG_RTL8187_LEDS */ 
#ifndef CONFIG_AT76C50X_USB
#define CONFIG_AT76C50X_USB 1
#endif /* CONFIG_AT76C50X_USB */ 
#ifndef CONFIG_AR9170_USB
#define CONFIG_AR9170_USB 1
#endif /* CONFIG_AR9170_USB */ 
#ifndef CONFIG_AR9170_LEDS
#define CONFIG_AR9170_LEDS 1
#endif /* CONFIG_AR9170_LEDS */ 
#ifndef CONFIG_ATH9K_HTC
#define CONFIG_ATH9K_HTC 1
#endif /* CONFIG_ATH9K_HTC */ 
#ifndef CONFIG_ATH9K_HTC_DEBUGFS
//#define CONFIG_ATH9K_HTC_DEBUGFS 1
#endif /* CONFIG_ATH9K_HTC_DEBUGFS */ 
#ifndef CONFIG_RT2500USB
#define CONFIG_RT2500USB 1
#endif /* CONFIG_RT2500USB */ 
#ifndef CONFIG_RT2800USB
#define CONFIG_RT2800USB 1
#endif /* CONFIG_RT2800USB */ 
#ifndef CONFIG_RT2X00_LIB_USB
#define CONFIG_RT2X00_LIB_USB 1
#endif /* CONFIG_RT2X00_LIB_USB */ 
#ifndef CONFIG_RT73USB
#define CONFIG_RT73USB 1
#endif /* CONFIG_RT73USB */ 
#undef CONFIG_LIBERTAS_THINFIRM_USB
#undef CONFIG_LIBERTAS_USB
#ifndef CONFIG_LIBERTAS_THINFIRM_USB
#define CONFIG_LIBERTAS_THINFIRM_USB 1
#endif /* CONFIG_LIBERTAS_THINFIRM_USB */ 
#ifndef CONFIG_LIBERTAS_USB
#define CONFIG_LIBERTAS_USB 1
#endif /* CONFIG_LIBERTAS_USB */ 
#ifndef CONFIG_WL1251
#define CONFIG_WL1251 1
#endif /* CONFIG_WL1251 */ 
#ifndef CONFIG_WL1251_SPI
#define CONFIG_WL1251_SPI 1
#endif /* CONFIG_WL1251_SPI */ 
#ifndef CONFIG_WL1271_SPI
#define CONFIG_WL1271_SPI 1
#endif /* CONFIG_WL1271_SPI */ 
#ifndef CONFIG_P54_SPI
#define CONFIG_P54_SPI 1
#endif /* CONFIG_P54_SPI */ 
#undef CONFIG_LIBERTAS_SPI
#ifndef CONFIG_LIBERTAS_SPI
#define CONFIG_LIBERTAS_SPI 1
#endif /* CONFIG_LIBERTAS_SPI */ 
#ifndef CONFIG_SSB_SDIOHOST
#define CONFIG_SSB_SDIOHOST 1
#endif /* CONFIG_SSB_SDIOHOST */ 
#ifndef CONFIG_B43_SDIO
#define CONFIG_B43_SDIO 1
#endif /* CONFIG_B43_SDIO */ 
#ifndef CONFIG_WL1251_SDIO
#define CONFIG_WL1251_SDIO 1
#endif /* CONFIG_WL1251_SDIO */ 
#ifndef CONFIG_WL1271_SDIO
#define CONFIG_WL1271_SDIO 1
#endif /* CONFIG_WL1271_SDIO */ 
#undef CONFIG_LIBERTAS_SDIO
#ifndef CONFIG_LIBERTAS_SDIO
#define CONFIG_LIBERTAS_SDIO 1
#endif /* CONFIG_LIBERTAS_SDIO */ 
#ifndef CONFIG_IWM
#define CONFIG_IWM 1
#endif /* CONFIG_IWM */ 
#ifndef CONFIG_RT2X00
#define CONFIG_RT2X00 1
#endif /* CONFIG_RT2X00 */ 
#ifndef CONFIG_RT2X00_LIB
#define CONFIG_RT2X00_LIB 1
#endif /* CONFIG_RT2X00_LIB */ 
#ifndef CONFIG_RT2800_LIB
#define CONFIG_RT2800_LIB 1
#endif /* CONFIG_RT2800_LIB */ 
#ifndef CONFIG_RT2X00_LIB_HT
#define CONFIG_RT2X00_LIB_HT 1
#endif /* CONFIG_RT2X00_LIB_HT */ 
#ifndef CONFIG_RT2X00_LIB_FIRMWARE
#define CONFIG_RT2X00_LIB_FIRMWARE 1
#endif /* CONFIG_RT2X00_LIB_FIRMWARE */ 
#ifndef CONFIG_RT2X00_LIB_CRYPTO
#define CONFIG_RT2X00_LIB_CRYPTO 1
#endif /* CONFIG_RT2X00_LIB_CRYPTO */ 
#ifndef CONFIG_RT2X00_LIB_LEDS
#define CONFIG_RT2X00_LIB_LEDS 1
#endif /* CONFIG_RT2X00_LIB_LEDS */ 
#ifndef CONFIG_RT2X00_LIB_FIRMWARE
#define CONFIG_RT2X00_LIB_FIRMWARE 1
#endif /* CONFIG_RT2X00_LIB_FIRMWARE */ 
#ifndef CONFIG_P54_COMMON
#define CONFIG_P54_COMMON 1
#endif /* CONFIG_P54_COMMON */ 
#ifndef CONFIG_P54_LEDS
#define CONFIG_P54_LEDS 1
#endif /* CONFIG_P54_LEDS */ 
#ifndef CONFIG_ATH_COMMON
#define CONFIG_ATH_COMMON 1
#endif /* CONFIG_ATH_COMMON */ 
#ifndef CONFIG_ATH_DEBUG
#define CONFIG_ATH_DEBUG 1
#endif /* CONFIG_ATH_DEBUG */ 
#ifndef CONFIG_WL12XX
#define CONFIG_WL12XX 1
#endif /* CONFIG_WL12XX */ 
#ifndef CONFIG_WL1251
#define CONFIG_WL1251 1
#endif /* CONFIG_WL1251 */ 
#ifndef CONFIG_WL1271
#define CONFIG_WL1271 1
#endif /* CONFIG_WL1271 */ 
#undef CONFIG_LIBERTAS
#ifndef CONFIG_LIBERTAS_THINFIRM
#define CONFIG_LIBERTAS_THINFIRM 1
#endif /* CONFIG_LIBERTAS_THINFIRM */ 
#ifndef CONFIG_LIBERTAS
#define CONFIG_LIBERTAS 1
#endif /* CONFIG_LIBERTAS */ 
#ifndef CONFIG_LIBERTAS_MESH
#define CONFIG_LIBERTAS_MESH 1
#endif /* CONFIG_LIBERTAS_MESH */ 
#ifndef CONFIG_RFKILL_BACKPORT
//#define CONFIG_RFKILL_BACKPORT 1
#endif /* CONFIG_RFKILL_BACKPORT */ 
#ifndef CONFIG_RFKILL_BACKPORT_LEDS
#define CONFIG_RFKILL_BACKPORT_LEDS 1
#endif /* CONFIG_RFKILL_BACKPORT_LEDS */ 
#ifndef CONFIG_RFKILL_BACKPORT_INPUT
#define CONFIG_RFKILL_BACKPORT_INPUT 1
#endif /* CONFIG_RFKILL_BACKPORT_INPUT */ 
#ifdef CONFIG_NET_SCHED
#ifdef CONFIG_NETDEVICES_MULTIQUEUE
#ifndef CONFIG_MAC80211_QOS
#define CONFIG_MAC80211_QOS 1
#endif /* CONFIG_MAC80211_QOS */ 
#endif
#endif
#endif /* COMPAT_AUTOCONF_INCLUDED */

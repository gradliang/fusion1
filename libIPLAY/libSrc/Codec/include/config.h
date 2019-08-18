/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      config.h
*
* Programmer:    Johnny Chen
*                MPX E120 division
*
* Created: 	 03/30/2005
*
* Description: 
*              
*        
* Change History (most recent first):
*     <1>     03/30/2005    Johnny Chen    first file
****************************************************************
*/
/* Protect against multiple inclusion */
#ifndef MPLAYER_CONFIG_H
#define MPLAYER_CONFIG_H 1

#include "UtilTypeDef.h"
#include "os.h"

/* use GNU internationalization */
// #define USE_I18N 1
#undef USE_I18N


/* Meng 05142005 starts */
#ifdef HW_MPEG_ACCE
#define COE_DIRECT2REG
#endif
/* Meng 05142005 ends */

/* missing mmap function on libc5 systems */
#ifndef MAP_FAILED
# define MAP_FAILED     ((void *) -1)
#endif

/* use setlocale() function */
#define USE_SETLOCALE 1

/* Runtime CPU detection */
#undef RUNTIME_CPUDETECT

/* Dynamic a/v plugins */
#undef DYNAMIC_PLUGINS

/* "restrict" keyword */
// #define restrict __restrict
#define restrict

#define PREFIX "/export/home/demingli/build-mplayer"

/* define this to use simple idct with patched libavcodec */
#define SIMPLE_IDCT 1

#define USE_OSD 1
#define USE_SUB 1

/* enable/disable SIGHANDLER */
#define ENABLE_SIGHANDLER 1

/* Toggles debugging informations */
//#define MP_DEBUG 1
#undef MP_DEBUG

/* Indicates that Ogle's libdvdread is available for DVD playback */
#define USE_DVDREAD 1

/* Indicates that dvdread is from libmpdvdkit */
#define USE_MPDVDKIT 2

/* Additional options for libmpdvdkit*/
#undef DVD_STRUCT_IN_DVD_H
#define DVD_STRUCT_IN_LINUX_CDROM_H 1
#undef DVD_STRUCT_IN_SYS_CDIO_H
#undef DVD_STRUCT_IN_SYS_DVDIO_H
#undef DVD_STRUCT_IN_BSDI_DVDIOCTL_DVD_H
#undef HAVE_BSD_DVD_STRUCT
#define HAVE_LINUX_DVD_STRUCT 1
#undef HAVE_OPENBSD_DVD_STRUCT
#undef DARWIN_DVD_IOCTL
#undef SOLARIS_USCSI
#undef HPUX_SCTL_IO
#define HAVE_STDDEF_H 1

/* Common data directory (for fonts, etc) */
#define MPLAYER_DATADIR "/export/home/demingli/build-mplayer/share/mplayer"
#define MPLAYER_CONFDIR "/export/home/demingli/build-mplayer/etc/mplayer"
#define MPLAYER_LIBDIR "/export/home/demingli/build-mplayer/lib"

/* Define this to compile stream-caching support, it can be enabled via
   -cache <kilobytes> */
#define USE_STREAM_CACHE 1

/* Define to include support for XviD/Divx4Linux/OpenDivx */
#undef USE_DIVX

/* Define to use the new XviD/DivX4Linux library instead of open source OpenDivX */
/* You have to change DECORE_LIBS in config.mak, too! */
#undef NEW_DECORE

/* Define if you are using DivX5Linux Decore library */
#undef DECORE_DIVX5

/* Define if you are using XviD library */
#undef HAVE_XVID3
#undef HAVE_XVID4
#undef DECORE_XVID
#undef ENCORE_XVID

/* Define to include support for libdv-0.9.5 */
#undef HAVE_LIBDV095

/* If build mencoder */
//#define HAVE_MENCODER
#undef HAVE_MENCODER

/* Indicates if XviD/Divx4linux encore is available
   Note: for mencoder */
#undef HAVE_DIVX4ENCORE

/* Indicates if libmp3lame is available
   Note: for mencoder */
#undef HAVE_MP3LAME
#undef CONFIG_MP3LAME

/* Define libmp1e for realtime mpeg encoding (for DXR3 and DVB cards) */
#undef USE_MP1E

/* Define this to enable avg. byte/sec-based AVI sync method by default:
   (use -bps or -nobps commandline option for run-time method selection)
   -bps gives better sync for vbr mp3 audio, it is now default */
#define AVI_SYNC_BPS 1

/* Undefine this if you do not want to select mono audio (left or right)
   with a stereo MPEG layer 2/3 audio stream. The command line option
   -stereo has three possible values (0 for stereo, 1 for left-only, 2 for
   right-only), with 0 being the default.
   */
#define USE_FAKE_MONO 1

/* Undefine this if your sound card driver has no working select().
   If you have kernel Oops, player hangups, or just no audio, you should
   try to recompile MPlayer with this option disabled! */
#define HAVE_AUDIO_SELECT 1

/* define this to use iconv(3) function to codepage conversions */
#define USE_ICONV 1

/* define this to use RTC (/dev/rtc) for video timers (LINUX only) */
#define HAVE_RTC 1

/* set up max. outburst. use 65536 for ALSA 0.5, for others 16384 is enough */
#define MAX_OUTBURST 65536

/* set up audio OUTBURST. Do not change this! */
#define OUTBURST 512

/* Define this if your system has the header file for the OSS sound interface */
#define HAVE_SYS_SOUNDCARD_H 1

/* Define this if your system has the header file for the OSS sound interface
 * in /usr/include */
#undef HAVE_SOUNDCARD_H

/* Define this if your system has the sysinfo header */
#define HAVE_SYS_SYSINFO_H 1

/* Define this if your system uses ftello() for DWORD seeking */

#define HAVE_FTELLO 1
#ifndef HAVE_FTELLO
# define ftello(a) ftell(a)
#endif

/* Define this if your system has the "malloc.h" header file */
#define HAVE_MALLOC_H 1

/* memalign is mapped to malloc if unsupported */
#define HAVE_MEMALIGN 1
#ifndef HAVE_MEMALIGN
# define memalign(a,b) malloc(b)
#endif

/* Define this if your system has the "alloca.h" header file */
#define HAVE_ALLOCA_H 1

/* Define this if your system has the "sys/mman.h" header file */
#define HAVE_SYS_MMAN_H 1

/* Define this if you have the elf dynamic linker -ldl library */
#define HAVE_LIBDL 1

/* Define this if you have the kstat kernel statistics library */
#undef HAVE_LIBKSTAT

/* Define this if you have zlib */
#define HAVE_ZLIB 1

/* Define this if you have shm support */
#define HAVE_SHM 1

/* Define this if your system has scandir & alphasort */
#define HAVE_SCANDIR 1

/* Define this if your system has strsep */
#define HAVE_STRSEP 1

/* Define this if your system has vsscanf */
#define HAVE_VSSCANF 1

/* Define this if your system has no posix select */
#undef HAVE_NO_POSIX_SELECT

/* Define this if your system has gettimeofday */
#define HAVE_GETTIMEOFDAY 1

/* Define this if your system has glob */
#define HAVE_GLOB 1

/* LIRC (remote control, see www.lirc.org) support: */
#undef HAVE_LIRC

/*
 * LIRCCD (LIRC client daemon)
 * See http://www.dolda2000.cjb.net/~fredrik/lirccd/
 */
#undef HAVE_LIRCC

/*
 * FLAC decoding
 */
#define HAVE_FLAC 1
#define USE_MPFLAC_DECODER 1

/* DVD navigation support using libdvdnav */



/* Define this to enable MPEG 1/2 image postprocessing (requires a FAST CPU!) */
#define MPEG12_POSTPROC 1

/* Define this to enable image postprocessing in libavcodec (requires a FAST CPU!) */
#define FF_POSTPROCESS 1

/* Define to include support for OpenDivx postprocessing */
#undef HAVE_ODIVX_POSTPROCESS

/* Win32 DLL support */
#undef USE_WIN32DLL
#define WIN32_PATH ""

/* DirectShow support */
#undef USE_DIRECTSHOW

/* Mac OS X specific features */
#undef MACOSX

/* Build our Win32-loader */


/* ffmpeg's libavcodec support (requires libavcodec source) */
#define USE_LIBAVCODEC 1
#undef USE_LIBAVCODEC_SO

/* risky codecs */
#define CONFIG_RISKY 1
//#undef CONFIG_RISKY

/* Use libavcodec's decoders */
#define CONFIG_DECODERS 1
/* Use libavcodec's encoders */
//#define CONFIG_ENCODERS 1
#undef CONFIG_ENCODERS

/* Use codec libs included in mplayer CVS / source dist: */
//#define USE_MP3LIB
//#define USE_LIBA52
//#define USE_LIBMPEG2
#undef USE_MP3LIB
#undef USE_LIBA52
#undef USE_LIBMPEG2

/* Use the SVQ1 decoder in libmpcodecs - we don't want/need it with libavcodec */
#ifndef USE_LIBAVCODEC
#define USE_SVQ1
#endif

/* Use libfame encoder filter */
#undef USE_LIBFAME

/* XAnim DLL support */
#undef USE_XANIM
/* Default search path */
#undef XACODEC_PATH

/* RealPlayer DLL support */
#undef USE_REALCODECS
/* Default search path */
#undef REALCODEC_PATH

/* LIVE.COM Streaming Media library support */
#undef STREAMING_LIVE_DOT_COM

/* Use 3dnow/mmxext/sse/mmx optimized fast memcpy() [maybe buggy... signal 4]*/
// #define USE_FASTMEMCPY 1
#undef USE_FASTMEMCPY

/* Use unrarlib for Vobsubs */
//#define USE_UNRARLIB 1
#undef USE_UNRARLIB 


/* gui support, please do not edit this option */
#undef HAVE_NEW_GUI

/* Audio output drivers */
#define USE_OSS_AUDIO 1
#define PATH_DEV_DSP "/dev/dsp"
#define PATH_DEV_MIXER "/dev/mixer"
#undef HAVE_ALSA5
#undef HAVE_ALSA9
#define USE_ARTS 1
#define USE_ESD 1
#define HAVE_ESD_LATENCY
#undef HAVE_SYS_ASOUNDLIB_H
#undef HAVE_ALSA_ASOUNDLIB_H
#undef USE_SUN_AUDIO
#undef USE_SGI_AUDIO
#undef HAVE_WIN32WAVEOUT
#undef HAVE_NAS

/* Enable fast OSD/SUB renderer (looks ugly, but uses less CPU power) */
#undef FAST_OSD
#undef FAST_OSD_TABLE

/* Enable TV Interface support */
#define USE_TV 1

/* Enable EDL support */
#define USE_EDL

/* Enable Video 4 Linux TV interface support */
#define HAVE_TV_V4L 1

/* Enable Video 4 Linux 2 TV interface support */
#define HAVE_TV_V4L2 1

/* Enable *BSD BrookTree TV interface support */
#undef HAVE_TV_BSDBT848

/* Define if your processor stores words with the most significant
   byte first (like Motorola and SPARC, unlike Intel and VAX).  */

// #define ARCH_X86 1
#undef ARCH_X86

/* libmpeg2 wants ARCH_PPC and the rest of mplayer use ARCH_POWERPC,
 * define ARCH_PPC if ARCH_POWERPC is set to cope with that.
 */
#ifdef ARCH_POWERPC
#define ARCH_PPC 1
#endif

/* the same issue as with ARCH_POWERPC but with ffmpeg/libavcodec */
#ifdef ARCH_ARMV4L
#define ARCH_ARM 1
#endif

/* only gcc3 can compile mvi instructions */


/* Define this for Cygwin build for win32 */


/* Define this to any prefered value from 386 up to infinity with step 100 */
#define __CPU__ 686

#define MP_WORDSIZE 32

#define TARGET_LINUX 1

#define HAVE_VCD 1

#ifdef sun
#define	DEFAULT_CDROM_DEVICE	"/vol/dev/aliases/cdrom0"
#define DEFAULT_DVD_DEVICE	DEFAULT_CDROM_DEVICE
#elif defined(HPUX)
#define DEFAULT_CDROM_DEVICE    "/dev/cdrom"
#define DEFAULT_DVD_DEVICE     "/dev/dvd"
#elif defined(WIN32)
#define DEFAULT_CDROM_DEVICE    "D:"
#define DEFAULT_DVD_DEVICE	"D:"
#elif defined(SYS_DARWIN)
#define DEFAULT_CDROM_DEVICE    "/dev/rdiskN"
#define DEFAULT_DVD_DEVICE	DEFAULT_CDROM_DEVICE
#else
#define DEFAULT_CDROM_DEVICE    "/dev/cdrom"
#define DEFAULT_DVD_DEVICE	"/dev/dvd"
#endif


/*----------------------------------------------------------------------------
**
** NOTE: Instead of modifying these definitions here, use the
**       --enable/--disable options of the ./configure script!
**       See ./configure --help for details.
**
*---------------------------------------------------------------------------*/

/* C99 lrintf function available */
#define HAVE_LRINTF 1

/* int_fastXY_t emulation */


/* nanosleep support */
#define HAVE_NANOSLEEP 1

/* SMB support */
#define LIBSMBCLIENT

/* termcap flag for getch2.c */
#define USE_TERMCAP 1

/* termios flag for getch2.c */
#define HAVE_TERMIOS 1
#undef HAVE_TERMIOS_H
#define HAVE_SYS_TERMIOS_H 1

/* enable PNG support */
#define HAVE_PNG 0

/* enable JPEG support */
#define HAVE_JPEG 1

/* enable GIF support */
#define HAVE_GIF 0
#define HAVE_GIF_4 0
#undef HAVE_GIF_TVT_HACK

/* enable FreeType support */
#define HAVE_FREETYPE

/* enable Fontconfig support */
#define HAVE_FONTCONFIG

/* enable FriBiDi usage */
#undef USE_FRIBIDI

/* liblzo support */
#undef USE_LIBLZO

/* libmad support */
#undef USE_LIBMAD

/* enable OggVorbis support */
#define HAVE_OGGVORBIS 1

/* enable Tremor as vorbis decoder */
#undef TREMOR

/* enable OggTheora support */
#undef HAVE_OGGTHEORA

/* enable Matroska support */
#undef HAVE_MATROSKA

/* enable FAAD (AAC) support */
#define HAVE_FAAD    1
#define USE_INTERNAL_FAAD  1
/*
#undef HAVE_FAAD
#undef USE_INTERNAL_FAAD
*/


/* enable network */
#define MPLAYER_NETWORK 1

/* enable ftp support */
#define HAVE_FTP 1

/* enable winsock2 instead of Unix functions*/
#undef HAVE_WINSOCK2

/* define this to use inet_aton() instead of inet_pton() */
#undef USE_ATON

/* enables / disables cdparanoia support */
#define HAVE_CDDA

/* enables / disables VIDIX usage */
#define CONFIG_VIDIX 1

/* enables / disables new input joystick support */
#undef HAVE_JOYSTICK

/* enables / disables QTX codecs */
#undef USE_QTX_CODECS

/* enables / disables osd menu */
#undef HAVE_MENU

/* enables / disables subtitles sorting */
#define USE_SORTSUB 1

/* XMMS input plugin support */
#undef HAVE_XMMS
#define XMMS_INPUT_PLUGIN_DIR ""

/* enables inet6 support */
#define HAVE_AF_INET6 1

/* do we have gethostbyname2? */
#define HAVE_GETHOSTBYNAME2 1

/* Extension defines */
#undef HAVE_3DNOW	// only define if you have 3DNOW (AMD k6-2, AMD Athlon, iDT WinChip, etc.)
#undef HAVE_3DNOWEX	// only define if you have 3DNOWEX (AMD Athlon, etc.)
/*
#define HAVE_MMX 1	// only define if you have MMX (newer x86 chips, not P54C/PPro)
#define HAVE_MMX2 1	// only define if you have MMX2 (Athlon/PIII/4/CelII)
#define HAVE_SSE 1	// only define if you have SSE (Intel Pentium III/4 or Celeron II)
#define HAVE_SSE2 1	// only define if you have SSE2 (Intel Pentium 4)
*/
#undef HAVE_MMX	// only define if you have MMX (newer x86 chips, not P54C/PPro)
#undef HAVE_MMX2	// only define if you have MMX2 (Athlon/PIII/4/CelII)
#undef HAVE_SSE	// only define if you have SSE (Intel Pentium III/4 or Celeron II)
#undef HAVE_SSE2	// only define if you have SSE2 (Intel Pentium 4)

	// only define if you have Altivec (G4)

//#undef HAVE_MMX
//#undef HAVE_MMX2


#ifdef HAVE_MMX
#define USE_MMX_IDCT 1
#endif

	// enables usage of altivec.h


#undef HAVE_MLIB  // Sun mediaLib, available only on solaris

/* libmpeg2 uses a different feature test macro for mediaLib */
#ifdef HAVE_MLIB
#define LIBMPEG2_MLIB 1
#endif

/* libvo options */
#define SCREEN_SIZE_X 1
#define SCREEN_SIZE_Y 1
#define HAVE_X11 1
#define HAVE_XV 1
#undef HAVE_XVMC
#define HAVE_XF86VM 1
#define HAVE_XINERAMA 1
#define HAVE_GL 1

#define HAVE_DGA 1
#define HAVE_DGA2 1
#define HAVE_SDL 1
/* defined for SDLlib with keyrepeat bugs (before 1.2.1) */
#undef BUGGY_SDL
#undef HAVE_DIRECTX
#undef HAVE_GGI
#undef HAVE_3DFX
#undef HAVE_TDFXFB
#undef HAVE_TDFX_VID
#undef HAVE_DIRECTFB

#undef HAVE_ZR
#undef HAVE_BL
#undef HAVE_MGA
#undef HAVE_XMGA

#define HAVE_FBDEV 1
#undef HAVE_DXR2
#undef HAVE_DXR3
#undef HAVE_DVB
#undef HAS_DVBIN_SUPPORT 
#undef HAVE_SVGALIB
#define HAVE_VESA 1
#define HAVE_XDPMS 1
#undef HAVE_AA
#define HAVE_TGA 1

/* used by GUI: */


#if defined(HAVE_GL) || defined(HAVE_X11) || defined(HAVE_XV)
#define X11_FULLSCREEN 1
#endif

#endif /* MPLAYER_CONFIG_H */

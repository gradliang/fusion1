ROOTPATH=../../../

include $(ROOTPATH)config.mak

TARGET = $(LIB)libfontengine.a

CC = $(BIN)gcc

CC_OPTION = -march=r3000 -c -mno-abicalls -O2 -G 8       \
            -DFT2_BUILD_LIBRARY                                 \
            -DFT_OPTION_AUTOFIT2                                \
            -D__mips__                                          \
            -D__mips_soft_float                                 \
            -I$(INC)                                            \
            -I../../fontengine/INCLUDE                          \
            -I../../fontengine/INCLUDE/freetype                 \
            -I../../fontengine/INCLUDE/freetype/config          \
            -I../../fontengine/INCLUDE/freetype/internal        \
            -I./fontengine/INCLUDE                              \
            -I./fontengine/INCLUDE/freetype                     \
            -I./fontengine/INCLUDE/freetype/config              \
            -I./fontengine/INCLUDE/freetype/internal            \
            -I./freetype/                                       \
            -I./freetype/INCLUDE                                \
            -I./freetype/INCLUDE/freetype                       \
            -I./freetype/INCLUDE/freetype/config                \
            -I./freetype/INCLUDE/freetype/internal              \
            -I../INCLUDE                                        \
            -I../INCLUDE/freetype                               \
            -I../INCLUDE/freetype/config                        \
            -I../INCLUDE/freetype/internal                      \
            -I$(G_INC)                                          \
            -Wa,-ahl=
            
#CC_OPTION = $(MAIN_FLAG) -march=r3000 -c -mno-abicalls -O2 -ggdb -G 8 -I$(INC) -I$(G_INC) -DBOOT_ROM_TYPE -Wa,-ahl=

AS = $(BIN)as
AS_OPTION = -march=r3000 -I$(INC) -I$(G_INC) -ahl=

LD = $(BIN)ld

AR = $(BIN)ar

truetype_OBJ = $(OBJ)truetype/
type1_OBJ    = $(OBJ)type1/
type42_OBJ   = $(OBJ)type42/
winfonts_OBJ = $(OBJ)winfonts/
autofit_OBJ  = $(OBJ)autofit/
base_OBJ     = $(OBJ)base/
bdf_OBJ      = $(OBJ)bdf/
cache_OBJ    = $(OBJ)cache/
cff_OBJ      = $(OBJ)cff/
cid_OBJ      = $(OBJ)cid/
gxvalid_OBJ  = $(OBJ)gxvalid/
gzip_OBJ     = $(OBJ)gzip/
lzw_OBJ      = $(OBJ)lzw/
otvalid_OBJ  = $(OBJ)otvalid/
pcf_OBJ      = $(OBJ)pcf/
pfr_OBJ      = $(OBJ)pfr/
psaux_OBJ    = $(OBJ)psaux/
pshinter_OBJ = $(OBJ)pshinter/
psnames_OBJ  = $(OBJ)psnames/
raster_OBJ   = $(OBJ)raster/
sfnt_OBJ     = $(OBJ)sfnt/
smooth_OBJ   = $(OBJ)smooth/
tools_OBJ    = $(OBJ)tools/

S_OBJS =

C_OBJS = \
		$(OBJ)Font_API.o                          \
         $(OBJ)GlyphContainer.o                    \
         $(OBJ)GlyphContainerLinkedList.o          \
         $(OBJ)CacheManager.o                      \
         $(autofit_OBJ)afhints.o   $(autofit_OBJ)afglobal.o  $(autofit_OBJ)afmodule.o  $(autofit_OBJ)afloader.o  \
         $(autofit_OBJ)afangles.o  $(autofit_OBJ)afdummy.o   $(autofit_OBJ)aflatin2.o  $(autofit_OBJ)afindic.o   \
         $(autofit_OBJ)aflatin.o   $(autofit_OBJ)afcjk.o     $(autofit_OBJ)afpic.o \
         $(base_OBJ)ftinit.o       $(base_OBJ)ftobjs.o       $(base_OBJ)ftsystem.o     $(base_OBJ)ftutil.o       \
         $(base_OBJ)ftgloadr.o     $(base_OBJ)ftoutln.o      $(base_OBJ)ftcalc.o       $(base_OBJ)ftrfork.o      \
         $(base_OBJ)fttype1.o      $(base_OBJ)fttrigon.o     $(base_OBJ)ftstream.o     $(base_OBJ)ftglyph.o      \
         $(base_OBJ)ftbitmap.o     $(base_OBJ)basepic.o      $(base_OBJ)ftpic.o        $(base_OBJ)ftsnames.o     \
         $(base_OBJ)ftadvanc.o \
         $(bdf_OBJ)bdfdrivr.o      $(bdf_OBJ)bdflib.o        \
         $(cache_OBJ)ftcache.o     \
         $(cff_OBJ)cffdrivr.o      $(cff_OBJ)cffgload.o      $(cff_OBJ)cffload.o       $(cff_OBJ)cffcmap.o       \
         $(cff_OBJ)cffobjs.o       $(cff_OBJ)cffparse.o      $(cff_OBJ)cffpic.o  \
         $(cid_OBJ)cidparse.o      $(cid_OBJ)cidobjs.o       $(cid_OBJ)cidgload.o      $(cid_OBJ)cidload.o       \
         $(cid_OBJ)cidriver.o      \
         $(gzip_OBJ)ftgzip.o       \
         $(lzw_OBJ)ftlzw.o         \
         $(pcf_OBJ)pcfutil.o       $(pcf_OBJ)pcfread.o       $(pcf_OBJ)pcfdrivr.o      \
         $(pfr_OBJ)pfrdrivr.o      $(pfr_OBJ)pfrsbit.o       $(pfr_OBJ)pfrobjs.o       $(pfr_OBJ)pfrload.o       \
         $(pfr_OBJ)pfrcmap.o       $(pfr_OBJ)pfrgload.o      \
         $(psaux_OBJ)psobjs.o      $(psaux_OBJ)psauxmod.o    $(psaux_OBJ)t1decode.o    $(psaux_OBJ)t1cmap.o      \
         $(psaux_OBJ)psconv.o      $(psaux_OBJ)afmparse.o    \
         $(psnames_OBJ)psmodule.o  $(psnames_OBJ)pspic.o\
         $(pshinter_OBJ)pshmod.o   $(pshinter_OBJ)pshrec.o   $(pshinter_OBJ)pshalgo.o  $(pshinter_OBJ)pshglob.o  \
         $(pshinter_OBJ)pshpic.o  \
         $(raster_OBJ)ftrend1.o    $(raster_OBJ)ftraster.o   $(raster_OBJ)rastpic.o\
         $(sfnt_OBJ)sfdriver.o     $(sfnt_OBJ)ttload.o       $(sfnt_OBJ)ttsbit.o       $(sfnt_OBJ)ttmtx.o        \
         $(sfnt_OBJ)ttkern.o       $(sfnt_OBJ)ttbdf.o        $(sfnt_OBJ)ttpost.o       $(sfnt_OBJ)ttcmap.o       \
         $(sfnt_OBJ)sfobjs.o       $(sfnt_OBJ)sfntpic.o  \
         $(smooth_OBJ)ftsmooth.o   $(smooth_OBJ)ftgrays.o    $(smooth_OBJ)ftspic.o \
         $(truetype_OBJ)ttgxvar.o  $(truetype_OBJ)ttdriver.o $(truetype_OBJ)ttgload.o  $(truetype_OBJ)ttobjs.o   \
         $(truetype_OBJ)ttpload.o  $(truetype_OBJ)ttinterp.o $(truetype_OBJ)ttpic.o \
         $(type1_OBJ)t1objs.o      $(type1_OBJ)t1gload.o     $(type1_OBJ)t1parse.o     $(type1_OBJ)t1afm.o       \
         $(type1_OBJ)t1load.o      $(type1_OBJ)t1driver.o    \
         $(type42_OBJ)t42objs.o    $(type42_OBJ)t42drivr.o   $(type42_OBJ)t42parse.o   \
         $(winfonts_OBJ)winfnt.o   \
         #$(OBJ)big5toUnicode.o     \

ALL_OBJS =  $(aa) $(S_OBJS) $(C_OBJS)

all: $(TARGET) makefile 
$(TARGET)	: $(ALL_OBJS)
	$(AR) -rsv $(TARGET) $(ALL_OBJS)

$(S_OBJS):	$(OBJ)%.o: $(SRC)%.s makefile
	$(AS) $(AS_OPTION)$(@:.o=.lst) -o $@ $<

$(C_OBJS):	$(OBJ)%.o: $(SRC)%.c makefile 
	$(CC) $(CC_OPTION)$(@:.o=.lst) -o $@ $<

clean:
	rm -f $(ALL_OBJS) $(OBJ)/*.lst $(TARGET)
	rm -f $(truetype_OBJ)*.lst
	rm -f $(type1_OBJ)*.lst
	rm -f $(type42_OBJ)*.lst
	rm -f $(winfonts_OBJ)*.lst
	rm -f $(autofit_OBJ)*.lst
	rm -f $(base_OBJ)*.lst  
	rm -f $(bdf_OBJ)*.lst
	rm -f $(cache_OBJ)*.lst
	rm -f $(cff_OBJ)*.lst
	rm -f $(cid_OBJ)*.lst
	rm -f $(gxvalid_OBJ)*.lst
	rm -f $(gzip_OBJ)*.lst
	rm -f $(lzw_OBJ)*.lst
	rm -f $(otvalid_OBJ)*.lst
	rm -f $(pcf_OBJ)*.lst
	rm -f $(pfr_OBJ)*.lst
	rm -f $(psaux_OBJ)*.lst
	rm -f $(pshinter_OBJ)*.lst
	rm -f $(psnames_OBJ)*.lst
	rm -f $(raster_OBJ)*.lst
	rm -f $(sfnt_OBJ)*.lst
	rm -f $(smooth_OBJ)*.lst
	rm -f $(tools_OBJ)*.lst
	
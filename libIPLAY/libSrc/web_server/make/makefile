# Global Variables
include ../../../config.mak


# ------------------------------------------------------------------------------
# Change the following variables if necessary
# ------------------------------------------------------------------------------

# target library name
WEBS_LIB = $(LIB)libwebs.a

# C source files
WEBS_C_SRC = \
			 asp.c         \
			 balloc.c      \
			 base64.c      \
			 default.c     \
			 ejlex.c       \
			 ejparse.c     \
			 form.c        \
			 h.c           \
			 handler.c     \
			 mime.c        \
			 misc.c        \
			 net_utility.c \
			 page.c        \
			 ringq.c       \
			 rom.c         \
			 security.c    \
			 sock.c        \
			 sockGen.c     \
			 sym.c         \
			 uemf.c        \
			 url.c         \
			 value.c       \
			 webs.c        \
			 websuemf.c    \
			 unixfs.c

# include directories
WEBS_INC = -I$(INC) -I$(G_INC) -I$(LIB)../libSrc/os_linux/include \
		   -I$(LIB)../libSrc/lwip/include


# ------------------------------------------------------------------------------
# In most cases, you don't need to change the following settings.
# ------------------------------------------------------------------------------

# compiler options
WEBS_CC_OPT = -O2 -march=r3000 -c -mno-abicalls -G 8 
#WEBS_CC_OPT = -O0 -ggdb -march=r3000 -c -mno-abicalls -G 8

# make targets
WEBS_C_OBJ = $(patsubst %.c,$(OBJ)%.o, $(WEBS_C_SRC))

.PHONY: all
all: $(WEBS_LIB)

$(WEBS_LIB): $(WEBS_C_OBJ)
	$(AR) -rsv $(WEBS_LIB) $(WEBS_C_OBJ)

$(OBJ)%.o: $(SRC)%.c
	$(CC) $(WEBS_CC_OPT) $(WEBS_INC) -Wa,-al=$(@:.o=.lst) -o $@ $<

.PHONY: clean
clean:
	rm -f $(WEBS_C_OBJ) $(OBJ)/*.lst  $(WEBS_LIB)


# $Id$

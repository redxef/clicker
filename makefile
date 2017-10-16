CC    := clang
LD    := clang
CFLGS := -g -Wall
LFLGS := -framework ApplicationServices -framework CoreFoundation -framework CoreGraphics
FILEN := clicker

# LDIRS ... directories which contain library files
# IDIRS ... include directories (header locations)
# LIBS  ... libraries to link against
# PKGS  ... include dirs, library dirs and libraries which should be resolved via pkg-config
LDIRS := 
IDIRS := 
LIBS  :=
PKGS  := 

D_SRC := ./src/
D_OBJ := ./obj/

SRCS  := $(shell find $(D_SRC) -name *.c)
OBJS  := $(subst $(D_SRC), $(D_OBJ), $(SRCS:%.c=%.o))
DIRS  := $(sort $(dir $(OBJS)))

# create the flags from the user input
_OLDIRS:= $(foreach libdir, $(LDIRS), -L$(libdir))	# used when linking
_OIDIRS:= $(foreach incdir, $(LDIRS), -L$(incdir))	# used when compiling
_OLIB  := $(foreach lib, $(LIBS), -l$(lib))		# used when linking

# add the flags from pkg-config
OLDIRS := $(shell pkg-config --libs-only-L $(PKGS) 2>/dev/null) $(_OLDIRS)
OIDIRS := $(shell pkg-config --cflags $(PKGS) 2>/dev/null) $(_OIDIRS)
OLIB := $(shell pkg-config --libs-only-l $(PKGS) 2>/dev/null) $(_OLIB)

ifneq ($(FILEN), main)
main: $(FILEN)
endif

# link
$(FILEN): $(OBJS)
	$(LD) $(LFLGS) $(OLDIRS) $(OLIB) -o $@ $^

# the | means, that objdirs target should be existent, not more recent
# build the objects
$(D_OBJ)%.o: $(D_SRC)%.c | objdirs
	$(CC) $(CFLGS) $(OIDIRS) -c $< -o $@

## support

# clean the built files
clean:
	$(RM) -r $(D_OBJ)/*
	$(RM) -r $(FILEN)

# check that object output directories exist
objdirs:
	@mkdir -p $(DIRS)

# PHONY: run every time
.PHONY: clean


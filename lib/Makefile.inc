ROMLIB_SRCS := checksum.c err.c locate_drvr.c romdisk.c
ROMLIB_A := lib/libmacrom.a

ROMLIB_SRCS := $(patsubst %, lib/%, $(ROMLIB_SRCS))

lib_all: $(ROMLIB_A)
lib_clean:
	rm -f $(ROMLIB_SRCS:%.c=%.o)
	rm -f $(ROMLIB_A)

$(ROMLIB_A): $(ROMLIB_SRCS:%.c=%.o)
	$(AR) cvr $@ $+
#	$(RANLIB) $@

lib/%.o: lib/%.c

lib/%.c: lib/macrompatcher.h
lib/romdisk.c: lib/macrompatcher.h lib/romdrv.h

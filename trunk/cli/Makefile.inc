ROMPATCHER_SRCS := main.c
ROMPATCHER_SRCS := $(patsubst %, cli/%, $(ROMPATCHER_SRCS))

ROMPATCHER_EXE := cli/macrompatcher

cli_all: $(ROMPATCHER_EXE) 
cli_clean:
	rm -f $(ROMPATCHER_SRCS:%.c=%.o)
	rm -f $(ROMPATCHER_EXE)

$(ROMPATCHER_EXE): $(ROMPATCHER_SRCS:%.c=%.o) $(ROMLIB_A)
	$(CC) $(CFLAGS) -o $@ $< $(ROMLIB_A) $(LDFLAGS)

cli/%o: cli/%.c

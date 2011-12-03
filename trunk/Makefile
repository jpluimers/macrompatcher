CFLAGS := -Wall -Werror "-Ilib/"

dir_names := lib cli
all: $(dir_names:%=%_all)
clean: $(dir_names:%=%_clean)

include $(dir_names:%=%/Makefile.inc)

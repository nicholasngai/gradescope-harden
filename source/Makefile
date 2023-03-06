TARGET = run_autograder
OBJS = \
	run_autograder.o \
	config.o \
	disable_networking.o
DEPS = $(OBJS:.o=.d)

CPPFLAGS = -std=c11 -pedantic -pedantic-errors -MMD -D_POSIX_C_SOURCE=200809L
CFLAGS = -O3 -Wall -Wextra -Werror
LDFLAGS =
LDLIBS = \
	-lseccomp \
	-lyaml

.PHONY: all
all: $(TARGET)

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $@

.PHONY: clean
clean:
	rm -rf $(TARGET) $(OBJS) $(DEPS)

-include $(DEPS)

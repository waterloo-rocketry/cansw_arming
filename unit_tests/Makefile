CC=gcc

CFLAGS+=-Wall
CFLAGS+=-Werror

objects  =altitude_unit_testing.o
objects +=altitude_parsing.o
objects +=can_tx_buffer.o
objects +=safe_ring_buffer.o

VPATH +=../
VPATH +=../canlib/util/

DEFINES+=-DBOARD_UNIQUE_ID=3
DEFINES+=-DUNIT_TEST

program_name=altitude_unit_test

INCLUDES+=-I../
INCLUDES+=-I../canlib/util/
INCLUDES+=-I../canlib/
INCLUDES+=-I.

all: $(objects)
	@$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) $(objects) -o $(program_name)
	@./$(program_name)

.PHONY:clean
clean:
	-rm $(objects)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) -c $< -o $@

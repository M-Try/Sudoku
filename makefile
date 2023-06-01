CC:=gcc
CFLAGS:=-O0 -pedantic -Wall -fanalyzer
LIBFLAGS:=-shared -fPIC # Note that this doesnt work because -fpic has to be applied at compiletime, not linktime
BIN:=./bin
OBJS:=sudoku.o
TARGET_LIBSTATIC:=$(BIN)/libsudoku.a
TARGET_LIBDYNAMIC:=$(BIN)/sudoku.so
TARGET_GAME:=Sudoku.exe
RM:=del

default: lib_static

lib_dynamic: $(TARGET_LIBDYNAMIC)

lib_static: $(TARGET_LIBSTATIC)

game: $(TARGET_GAME)

$(TARGET_LIBDYNAMIC): $(OBJS)
	$(CC) $(LIBFLAGS) $(OBJS) -o $(BIN)/sudoku.so

$(TARGET_LIBSTATIC): $(OBJS)
	ar rcs $(TARGET_LIBSTATIC) $(OBJS)

$(TARGET_GAME): $(TARGET_LIBSTATIC)
	$(CC) ./game/run.c -o $(TARGET_GAME) -lsudoku -L$(BIN)

%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	-@$(RM) $(TARGET_GAME)
	-@$(RM) $(TARGET_LIBSTATIC)
	-@$(RM) $(TARGET_LIBDYNAMIC)
	-@$(RM) $(OBJS)

.PHONY: default lib_dynamic lib_static game clean

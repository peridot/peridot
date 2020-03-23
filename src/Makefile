CC = clang
CFLAGS = -Wall -Wextra
LDFLAGS = -luv
OBJS = obj/gc.o obj/pvm.o obj/chunk.o obj/value.o obj/main.o obj/debug.o obj/str.o obj/parser.o obj/lexer.o obj/compiler.o obj/ast.o obj/object.o obj/runtime.o obj/table.o obj/function.o obj/builtin.o
LEX = flex
YACC = bison

# Debug builds are faster to compile and easier to debug but is not optimized.
ifeq ($(MODE),debug)
        CFLAGS += -O0 -DPD_DEBUG -g -DDEBUG_TRACE_GC -DDEBUG_TRACE_EXECUTION
else
        CFLAGS += -O3 -flto
				LDFLAGS += -flto
endif

peridot: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o peridot

obj/main.o: main.c
	$(CC) $(CFLAGS) -c main.c -o obj/main.o

obj/gc.o: gc.c gc.h
	$(CC) $(CFLAGS) -c gc.c -o obj/gc.o

obj/pvm.o: pvm.c pvm.h
	$(CC) $(CFLAGS) -c pvm.c -o obj/pvm.o

obj/chunk.o: chunk.c chunk.h
	$(CC) $(CFLAGS) -c chunk.c -o obj/chunk.o

obj/compiler.o: compiler.c compiler.h
	$(CC) $(CFLAGS) -c compiler.c -o obj/compiler.o

obj/builtin.o: builtin.c runtime.h
	$(CC) $(CFLAGS) -c builtin.c -o obj/builtin.o

obj/value.o: value.c value.h
	$(CC) $(CFLAGS) -c value.c -o obj/value.o

obj/debug.o: debug.c debug.h
	$(CC) $(CFLAGS) -c debug.c -o obj/debug.o

obj/object.o: object.c object.h
	$(CC) $(CFLAGS) -c object.c -o obj/object.o

obj/str.o: str.c str.h
	$(CC) $(CFLAGS) -c str.c -o obj/str.o

parser.c: parser.y
	$(YACC) parser.y

lexer.c: lexer.l
	$(LEX) lexer.l

obj/lexer.o: lexer.c
	$(CC) $(CFLAGS) -c lexer.c -o obj/lexer.o

obj/parser.o: parser.c
	$(CC) $(CFLAGS) -c parser.c -o obj/parser.o

obj/ast.o: ast.c ast.h
	$(CC) $(CFLAGS) -c ast.c -o obj/ast.o

obj/runtime.o: runtime.c runtime.h
	$(CC) $(CFLAGS) -c runtime.c -o obj/runtime.o

obj/table.o: table.c table.h
	$(CC) $(CFLAGS) -c table.c -o obj/table.o

obj/function.o: function.c function.h
	$(CC) $(CFLAGS) -c function.c -o obj/function.o

.PHONY clean:
clean:
	$(RM) $(OBJS)

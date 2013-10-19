
OPT  ?= -O3
WARN ?= -Wall -Wextra -pedantic

CFL = $(OPT) $(WARN) $(CFLAGS)

test_avl: test_avl.c avl_test.c avl_test.h ../avl.c ../avl.h avm.c avm.h test_utils.c test_utils.h
	$(CC) $(CFL) -o test_avl test_avl.c avl_test.c avm.c test_utils.c

clean:
	$(RM) test_avl

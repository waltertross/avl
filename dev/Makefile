
all: lx/.done cfunx/.done

lx/.done: cfunx.awk ../avl.c
	awk -f cfunx.awk dir=lx 'fun=[a-z][a-z0-9_]*_[lx_]+' ../avl.c
	touch lx/.done

cfunx/.done: cfunx.awk ../avl.c
	awk -f cfunx.awk ../avl.c
	touch cfunx/.done

clean:
	if test -d lx;    then rm -f    lx/*.c    lx/.done; fi
	if test -d cfunx; then rm -f cfunx/*.c cfunx/.done; fi

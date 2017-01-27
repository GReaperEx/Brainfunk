.PHONY: test

test: $(patsubst %.bf,%.out,$(wildcard *.bf))

%.c.out: %.c.bf %.in %.use %.val
	echo Testing: $*'-c'
	../bfk -c $(cat $*'.use') $< > /dev/null -o $*'.exe' && \
	sync $*'.exe' && \
	./$*'.exe' < $*'.in' > $@ && \
	sync $@ && \
	diff  $*'.val' $@

%.out: %.bf %.in %.use %.val
	echo Testing: $*
	../bfk $(cat $*'.use') $*'.bf' < $*'.in' > $*'.out' && \
	sync $@ && \
	diff  $*'.val' $@


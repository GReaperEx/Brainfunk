SRCS := $(shell find ./ -name '*.bf')
OBJS := $(SRCS:.bf=.out)

.PHONY: test clean

test: $(OBJS)
	echo Finished testing.

clean:
	rm -f $(OBJS) $(OBJS:.c.out=.exe)

%.c.out: %.c.bf %.in %.use %.val
	echo Testing: $(*F)'-c'
	../bfk -c $(cat $*'.use') $< > /dev/null -o $*'.exe' && \
	sync $*'.exe' && \
	./$*'.exe' < $*'.in' > $@ && \
	sync $@ && \
	diff  $*'.val' $@

%.out: %.bf %.in %.use %.val
	echo Testing: $(*F)
	../bfk $(cat $*'.use') $*'.bf' < $*'.in' > $*'.out' && \
	sync $@ && \
	diff  $*'.val' $@

%.in:
	touch $@

%.use:
	touch $@



SUB_DIRS:=pcre iniparser

all: sub-all 

clean: sub-clean

sub-all: $(SUB_DIRS)
	for DIR in $(SUB_DIRS); do $(MAKE) -C $$DIR; done

sub-clean:
	for DIR in $(SUB_DIRS); do $(MAKE) -C $$DIR clean; done


SUB_DIRS:=pcre iniparser wxwidgets

all: sub-all 

fetch: sub-fetch

clean: sub-clean

sub-all: $(SUB_DIRS)
	for DIR in $(SUB_DIRS); do $(MAKE) -C $$DIR; done

sub-fetch: $(SUB_DIRS)
	for DIR in $(SUB_DIRS); do $(MAKE) -C $$DIR fetch; done

sub-clean:
	for DIR in $(SUB_DIRS); do $(MAKE) -C $$DIR clean; done

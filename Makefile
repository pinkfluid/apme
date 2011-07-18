
SUB_DIRS:=src

all: subdirs

subdirs: $(SUB_DIRS)
	for DIR in $(SUB_DIRS); do $(MAKE) -C $$DIR; done

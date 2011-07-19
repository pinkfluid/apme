include ../../sys.mk

PCRE_VER:=8.12
PCRE_DIR:=$(EXTERN_BUILD)/pcre-$(PCRE_VER)
PCRE_ARCHIVE:=pcre-$(PCRE_VER).tar.gz

#PCRE_URL:=ftp://ftp.csx.cam.ac.uk/pub/software/programming/pcre/$(PCRE_ARCHIVE)
PCRE_URL:=http://noom.ath.cx/mitja/$(PCRE_ARCHIVE)

all: build
	echo $(TOP_DIR)

build: pcre_build

pcre_build: $(PCRE_DIR)
	cd $(PCRE_DIR) ; ./configure
	cd $(PCRE_DIR) ; make

$(PCRE_DIR): $(EXTERN_BUILD)/$(PCRE_ARCHIVE)
	cd $(EXTERN_BUILD) ; tar xzvf $(PCRE_ARCHIVE)

$(EXTERN_BUILD)/$(PCRE_ARCHIVE):
	@mkdir $(EXTERN_BUILD)
	(cd $(EXTERN_BUILD); $(FETCH_CMD) $(PCRE_URL))

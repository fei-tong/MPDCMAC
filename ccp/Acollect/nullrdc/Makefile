CONTIKI = ../../../../contiki


all: example-abc example-mesh example-collect example-trickle example-polite \
     example-rudolph1 example-rudolph2 example-rucb \
     example-runicast example-unicast example-neighbors JF-collect

DEFINES=PROJECT_CONF_H=\"project-conf.h\"


MODULES += core/net/mac/cxmac core/net/mac/xmac


DATA_INTERVAL ?= 5
CFLAGS+= -DCMD_DATA_INTERVAL=$(DATA_INTERVAL)


CONTIKI_WITH_RIME = 1
include $(CONTIKI)/Makefile.include

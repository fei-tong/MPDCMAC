CONTIKI = ../../../../contiki
CONTIKI_PROJECT = example-mesh example-collect example-trickle example-polite hello-world\
				  example-rudolph0 example-rudolph1 example-rudolph2 example-rucb \
			      example-runicast example-unicast example-neighbors hello-world-micaz
APPS+=powertrace
DEFINES=PROJECT_CONF_H=\"project-conf.h\"

MPDCMACDIR=..
PROJECTDIRS += $(MPDCMACDIR) 
TIMEDATE=`date +%Y%m%d-%H%M`

#for local version pdcadc 
CONTIKIDIRS += $(MPDCMACDIR)/net/mac/mpdcmac

CONTIKI_SOURCEFILES += mpdcmac.c 
				
# to configure CFLAGS
CFLAGS+= -iquote -Werror

#every PERIODIC_CYCLES cycles, send RTS
PERIODIC_CYCLES ?= 3 #every 3 cycles, send RTS 
CFLAGS+= -DCMD_PERIODIC_CYCLES=$(PERIODIC_CYCLES)
#maximum cycles for keeping active after determining its grade.
MAX_ACTIVE_CYCLES ?= 3
CFLAGS+= -DCMD_MAX_ACTIVE_CYCLES=$(MAX_ACTIVE_CYCLES)
#when the node grade is or becomes -1, the node will wait such number +1 of receivings to determine its new grade
RCV_GRADE_NUM_TO_SET ?= 2
CFLAGS+= -DCMD_RCV_GRADE_NUM_TO_SET=$(RCV_GRADE_NUM_TO_SET)
#START_GRADE, unit: second
#START_GRADE ?= 10
#CFLAGS+= -DCMD_START_GRADE=$(START_GRADE)
#GEN_DATA_IN_R: 1: generate data in R status; 0: generate data in function pri_qsend_packet;*/
GEN_DATA_IN_R ?=1
CFLAGS+= -DCMD_GEN_DATA_IN_R=$(GEN_DATA_IN_R)

#Data related
DATA_TX_SUPPORT ?= 1 #1:has data to be sent; 0: no data
CFLAGS+= -DCMD_DATA_TX_SUPPORT=$(DATA_TX_SUPPORT)

##############################################################
RETX_LIMIT ?= 6 #6,allow 5 retransmissions, =0: no limit; >0: transmission times, i.e., (RETX_LIMIT-1) retransmissions
CFLAGS+= -DCMD_RETX_LIMIT=$(RETX_LIMIT)

#queue buffer size
MAX_DATA_QUEUE_BUFS ?= 10 
CFLAGS+= -DCMD_MAX_DATA_QUEUE_BUFS=$(MAX_DATA_QUEUE_BUFS)

#for sim, 15pkt/node, for board, 90pkt/node, if DATA_TX_SUPPORT==1, this varialbe indicates how many data a node can generate
#if TOTAL_DATA_NUM<0, node will generate data endlessly
TOTAL_DATA_NUM ?= -1
CFLAGS+= -DCMD_TOTAL_DATA_NUM=$(TOTAL_DATA_NUM) 

################################################# 
#sleep factor(18, 22, 26)
SF ?= 6
CFLAGS+= -DCMD_SF=$(SF)

#unit seconds  
DATA_INTERVAL ?= 10
CFLAGS+= -DCMD_DATA_INTERVAL=$(DATA_INTERVAL)
################################################# 
#==========mpdc module related variables========#

#1:MPDC, support multi-channels, 0: DCPF, doesn't support and
MPDC_SUPPORT ?= 0
CFLAGS+= -DCMD_MPDC_SUPPORT=$(MPDC_SUPPORT)

#IS_NOT_STAGGER: 0: don't stagger the schedules
#                0:Stagger schedules of paths allocated to the same channel
# when MPDC SUPPORT = 1,this variable will work
IS_NOT_STAGGER ?= 1

#Number of channels
NUM_CHANNELS ?= 1
CFLAGS+= -DCMD_NUM_CHANNELS=$(NUM_CHANNELS)

#Number of paths
NUM_PATHS ?= 2
CFLAGS+= -DCMD_NUM_PATHS=$(NUM_PATHS)

#MIN_HOPS_BETWEEN_ADJCENT_NODES
MIN_HOPS_BETWEEN_ADJCENT_NODES ?= 4 
CFLAGS+= -DCMD_MIN_HOPS_BETWEEN_ADJCENT_NODES=$(MIN_HOPS_BETWEEN_ADJCENT_NODES)

CFLAGS+= -DCMD_IS_NOT_STAGGER=$(IS_NOT_STAGGER)


#=======mpdc module related variables==================#
##########################################################
IS_COOJA_SIM ?= 1
CFLAGS+= -DCMD_IS_COOJA_SIM=$(IS_COOJA_SIM)

CFLAGS += -ffunction-sections 
LDFLAGS +=-Wl,--gc-sections,--undefined=_reset_vector__,--undefined=InterruptVectors,--undefined=_copy_data_init__,--undefined=_clear_bss_init__,--undefined=_end_of_init__

all: $(CONTIKI_PROJECT)

CONTIKI_WITH_RIME = 1
CONTIKI_NO_NET = 1 #to exclude all the networking code. 
include $(CONTIKI)/Makefile.include


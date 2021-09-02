CONTIKI = ../../../../contiki

CONTIKI_PROJECT = hello-world

APPS += powertrace
DEFINES=PROJECT_CONF_H=\"project-conf.h\"

MPDCMACDIR=..
PROJECTDIRS += $(MPDCMACDIR) 
TIMEDATE=`date +%Y%m%d-%H%M`

#for local version pdcadc 
CONTIKIDIRS += $(MPDCMACDIR)/net/mac/mpdcmac

CONTIKI_SOURCEFILES += mpdcmac.c 
				
# to configure CFLAGS
CFLAGS+= -iquote -Werror


#Synchronization settings
#1: will do synchronization based on the proposed strategy; 0: no synchronization
PRI_SYNC_SUPPORT ?= 0  
CFLAGS+= -DCMD_PRI_SYNC_SUPPORT=$(PRI_SYNC_SUPPORT)

#for cooja simulation, if PRI_SYNC_SUPPORT==0, this variable will not work, 1: generate random drift; 0: no drift
RANDOM_DRIFT_IN_COOJA_SUPPORT ?= 1
CFLAGS+= -DCMD_RANDOM_DRIFT_IN_COOJA_SUPPORT=$(RANDOM_DRIFT_IN_COOJA_SUPPORT)

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
GEN_DATA_IN_R ?= 0
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
MPDC_SUPPORT ?= 1
CFLAGS+= -DCMD_MPDC_SUPPORT=$(MPDC_SUPPORT)

#IS_NOT_STAGGER: 0: don't stagger the schedules
#                1:Stagger schedules of paths allocated to the same channel
# when MPDC SUPPORT = 1,this variable will work
IS_NOT_STAGGER ?= 0

#Number of channels
NUM_CHANNELS ?= 2
CFLAGS+= -DCMD_NUM_CHANNELS=$(NUM_CHANNELS)

#MIN_HOPS_BETWEEN_ADJCENT_NODES
MIN_HOPS_BETWEEN_ADJCENT_NODES ?= 4 
CFLAGS+= -DCMD_MIN_HOPS_BETWEEN_ADJCENT_NODES=$(MIN_HOPS_BETWEEN_ADJCENT_NODES)

CFLAGS+= -DCMD_IS_NOT_STAGGER=$(IS_NOT_STAGGER)


#=======mpdc module related variables==================#
##########################################################

# RE-mote related

#Address related
#process the grade message according to address for real board (set to 1) or transmission range, for cooja simulation (set to 0)
PROC_GRADE_BASED_ON_ADDR_SUPPORT ?= 1
CFLAGS+= -DCMD_PROC_GRADE_BASED_ON_ADDR_SUPPORT=$(PROC_GRADE_BASED_ON_ADDR_SUPPORT)
#NODES_NUM_PER_GRADE is valid only when PROC_GRADE_BASED_ON_ADDR_SUPPORT = 1
NODES_NUM_PER_GRADE ?= 3
CFLAGS+= -DCMD_NODES_NUM_PER_GRADE=$(NODES_NUM_PER_GRADE)

ID_FOR_RE_1 ?= 1 #The ID assigned to the RE 1.0 mote
CFLAGS+= -DCMD_ID_FOR_RE_1=$(ID_FOR_RE_1)
ID_FOR_RE_2 ?= 2 #The ID assigned to the RE 2.0 mote
CFLAGS+= -DCMD_ID_FOR_RE_2=$(ID_FOR_RE_2)
ID_FOR_RE_3 ?= 3 #The ID assigned to the RE 3.0 mote
CFLAGS+= -DCMD_ID_FOR_RE_3=$(ID_FOR_RE_3)
ID_FOR_RE_4 ?= 4 #The ID assigned to the RE 4.0 mote
CFLAGS+= -DCMD_ID_FOR_RE_4=$(ID_FOR_RE_4)
ID_FOR_RE_5 ?= 5 #The ID assigned to the RE 5.0 mote
CFLAGS+= -DCMD_ID_FOR_RE_5=$(ID_FOR_RE_5)
ID_FOR_RE_6 ?= 6 #The ID assigned to the RE 6.0 mote
CFLAGS+= -DCMD_ID_FOR_RE_6=$(ID_FOR_RE_6)
ID_FOR_RE_7 ?= 7 #The ID assigned to the RE 7.0 mote
CFLAGS+= -DCMD_ID_FOR_RE_7=$(ID_FOR_RE_7)
ID_FOR_RE_8 ?= 8 #The ID assigned to the RE 8.0 mote
CFLAGS+= -DCMD_ID_FOR_RE_8=$(ID_FOR_RE_8)
ID_FOR_RE_9 ?= 9 #The ID assigned to the RE 9.0 mote
CFLAGS+= -DCMD_ID_FOR_RE_9=$(ID_FOR_RE_9)
ID_FOR_RE_10 ?= 10 #The ID assigned to the RE 10.0 mote
CFLAGS+= -DCMD_ID_FOR_RE_10=$(ID_FOR_RE_10)
ID_FOR_RE_11 ?= 11 #The ID assigned to the RE 11.0 mote
CFLAGS+= -DCMD_ID_FOR_RE_11=$(ID_FOR_RE_11)
ID_FOR_RE_12 ?= 12 #The ID assigned to the RE 12.0 mote
CFLAGS+= -DCMD_ID_FOR_RE_12=$(ID_FOR_RE_12)
ID_FOR_RE_13 ?= 13 #The ID assigned to the RE 13.0 mote
CFLAGS+= -DCMD_ID_FOR_RE_13=$(ID_FOR_RE_13)
ID_FOR_RE_14 ?= 14 #The ID assigned to the RE 14.0 mote
CFLAGS+= -DCMD_ID_FOR_RE_14=$(ID_FOR_RE_14)
ID_FOR_RE_15 ?= 15 #The ID assigned to the RE 15.0 mote
CFLAGS+= -DCMD_ID_FOR_RE_15=$(ID_FOR_RE_15)


IS_COOJA_SIM ?= 1
CFLAGS+= -DCMD_IS_COOJA_SIM=$(IS_COOJA_SIM)

CFLAGS += -ffunction-sections 
LDFLAGS +=-Wl,--gc-sections,--undefined=_reset_vector__,--undefined=InterruptVectors,--undefined=_copy_data_init__,--undefined=_clear_bss_init__,--undefined=_end_of_init__

all: $(CONTIKI_PROJECT)

CONTIKI_WITH_RIME = 1
CONTIKI_NO_NET = 1 #to exclude all the networking code. 
include $(CONTIKI)/Makefile.include


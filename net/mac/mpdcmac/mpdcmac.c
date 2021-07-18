/**    
 * \file
 *		Implementation: A Multi-Channel Pipel ined Data Colle ction MAC (MPDC)
 * 		based on the PRIMAC power-saving radio duty cycling protocol
 * \author
 *		F. Jiang <fangjiangff@163.com>      
 * \date 
 *		start:	Feb. 26, 2021
 *		update:	May. 19, 2021
 *		New update: May. 19, 2021       
 * \description:
 *		Current functions: 
 *						duty-cycled, pipelined scheduleing; 
 * 						schedule synchronization; 
 *						data-gathering tree establishment and maintenance; 
 *						address-free;
 *						dynamic duty cycle: single-channel;
 *						multiple-channel
 */ 
 
#if ZOLERTIA_Z1
#include "dev/i2cmaster.h"	// Include IC driver
#include "dev/tmp102.h"     // Include sensor driver
#endif    
  
#include "contiki-conf.h"
#include "dev/leds.h"
#include "dev/radio.h"
#include "dev/watchdog.h"
#include "lib/random.h"
#include "net/mac/mpdcmac/mpdcmac.h"
#include "net/netstack.h"
#include "net/rime/rime.h"
#include "sys/compower.h"
#include "sys/pt.h"
#include "sys/rtimer.h"  
#include "sys/energest.h" 
#include <math.h>   
   
#include <string.h>   
#include <stdio.h>    
       
/*  
#if NETSTACK_RDC_CHANNEL_CHECK_RATE >= 64
#undef WITH_PHASE_OPTIMIZATION
#define WITH_PHASE_OPTIMIZATION 0  
#endif     
*/            
  
/* CYCLE_TIME for channel cca checks, in rtimer ticks. */
#ifdef CONTIKIMAC_CONF_CYCLE_TIME
#define CYCLE_TIME (CONTIKIMAC_CONF_CYCLE_TIME)
#else
#define CYCLE_TIME (RTIMER_SECOND / NETSTACK_RDC_CHANNEL_CHECK_RATE)
#endif
  
#ifdef PRIMAC_CONF_COMPOWER
	#define PRIMAC_CONF_COMPOWER_SUPPORT 	PRIMAC_CONF_COMPOWER
#else
	#define PRIMAC_CONF_COMPOWER_SUPPORT 0
#endif

#if PRIMAC_CONF_COMPOWER_SUPPORT
	static struct compower_activity current_packet;
	struct pri_compower_t
	{
		uint32_t	all_ctr_listen; //all control listen
		uint32_t	all_ctr_transmit; //all control transmit
		uint32_t	all_data_listen; //all data listen
		uint32_t	all_data_transmit;// all data transmit
	};
	static volatile	struct pri_compower_t	pri_compower;
#endif /*PRIMAC_CONF_COMPOWER_SUPPORT CONTIKIMAC_CONF_COMPOWER */


/************************************************************************************************/
/*------------------Begin: PRIMAC------------------*/ 
/*Define*/

//#define MAX_GRADE_SIZE		50
#define DISPATCH			0
#define NONDISPATCH		1


//#define GRADE_BROADCAST_NUM	1 /*braodcasting or forwarding times of the grade message*/
/*define packet type*/
#define TYPE_GRADE		   0x10 //grade message
#define TYPE_RTS		   0x11 //rts packet
#define TYPE_CTS		   0x12 //cts packet
//#define TYPE_ACK  		   0x13 //ack packet
#define TYPE_DATA		   0x14 //data packet
/*packet type related with MPDC*/
#define TYPE_PROBE 			0x15 //probe message
#define TYPE_PROBE_R		 0x16//probe ack message
#define TYPE_NODEID  		 0x17//node id packet
#define TYPE_GRADE_SINK      0x18//grade message from sink

/*Varialbes*/
static struct rtimer rt;
static struct ctimer grade_start_ctimer; /*ctimer for sink to broadcast grade message*/
static struct ctimer probe_start_ctimer;
static struct pt pt;

static volatile uint8_t pdcadc_is_on = 0; /*typedef unsigned char   uint8_t;*/
static volatile uint8_t pdcadc_keep_radio_on = 0;
static volatile uint16_t num_paths = 0;/*the number of transmission paths*/

static volatile unsigned char we_are_sending = 0;
static volatile unsigned char radio_is_on = 0;



enum{
	NOUSE,
		
	/*node type*/
	PRI_SINK, /*node type: sink*/
	PRI_SENSOR, /*node type: sensor*/

	CHILD_LIST, //The sender neighbor list
	PARENT_LIST, //The receiver neighbor list
};
//#if ZOLERTIA_Z1
#if LINKADDR_SIZE == 2
	const linkaddr_t linkaddr_sink = { { 1, 0 } };
#else /*LINKADDR_SIZE == 2*/
#if LINKADDR_SIZE == 8
	const linkaddr_t linkaddr_sink = { { 0x01,0,0,0,0,0,0,0 } };
#endif /*LINKADDR_SIZE == 8*/
#endif /*LINKADDR_SIZE == 2*/

/*Function Declaration*/
static void on(void);
static void off(void);
static uint32_t pri_state_period();
static char pri_powercycle(struct rtimer *t, void *ptr); /*function declaration*/

#ifdef PRIMAC_CONF_CCA_COUNT_MAX_TX
#define PRI_CCA_COUNT_MAX_TX                   (PRIMAC_CONF_CCA_COUNT_MAX_TX)
#else
#define PRI_CCA_COUNT_MAX_TX                 10 // 1 // 6
#endif

#ifdef PRIMAC_CONF_CCA_CHECK_TIME
#define PRI_CCA_CHECK_TIME                     (PRIMAC_CONF_CCA_CHECK_TIME)
#else
#define PRI_CCA_CHECK_TIME                     RTIMER_SECOND / 8192 // 2000
#endif

	/* CCA_SLEEP_TIME is the time between two successive CCA checks. */
	/* Add 1 when rtimer ticks are coarse */

#if RTIMER_SECOND > 8000
#define PRI_CCA_SLEEP_TIME                     RTIMER_SECOND / 2000// 8192
#else
#define PRI_CCA_SLEEP_TIME                     (RTIMER_SECOND / 2000) + 1 
#endif

#ifndef IS_COOJA_SIM
	#define	IS_COOJA_SIM	0
#endif

#ifndef CROSS_PLATFORM_SUPPORT
	#define CROSS_PLATFORM_SUPPORT	0
#endif

#ifndef  SF
#define SF 6 // Sleep Factor, the least value of sleep factor is 2 (sleep_factor in the paper)
#endif


/* CHECK_TIME_TX is the total time it takes to perform CCA_COUNT_MAX_TX CCAs.  20*6=120*/
//#define PRI_CHECK_TIME_TX                      (PRI_CCA_COUNT_MAX_TX * (PRI_CCA_CHECK_TIME + PRI_CCA_SLEEP_TIME))

/*Begin: PRIMAC protocol parameters*/

/*begin:The propagation time of grade message*/
#if IS_COOJA_SIM
	#define PRI_TM			(RTIMER_SECOND/455)//=72, 0.00219s
#else
	#define PRI_TM			0//0//(RTIMER_SECOND/455)//=72, 0.00219s
#endif
#define WAKEUP_ADVANCE_TIME		(RTIMER_SECOND/100) // 10ms wakeup advance time at the beginning of R state
#define	R_END_GUARD_TIME	WAKEUP_ADVANCE_TIME // 10 ms guard time at the end of R state of the receiver 

#define DURGRADE	(RTIMER_SECOND/1023) //=364, 0.011s (1/91) propagation time of RTS (durRTS in the paper)
#define DURRTS		(RTIMER_SECOND/1023) //=364, 0.011s (1/91) propagation time of RTS (durRTS in the paper)
#define DURCTS		(RTIMER_SECOND/1023) //=364, 0.011s (durCTS)
//#define DURACK		(RTIMER_SECOND/1203) //=364, 0.011s
#define DURDATA		(RTIMER_SECOND/46) //=819, 28,1170,0.036s (1/23=, 0.043s)
#define CW			64//(RTIMER_SECOND/512) //=520, 0.016s 
#define SIGMA		(RTIMER_SECOND/1000) // equivalent to 1 ms =1/1000 s
#define DIFS		(RTIMER_SECOND/100)//=327, 0.01s
#define SIFS		(RTIMER_SECOND/200)//=163, 0.005s
#define GUARDTIME	(RTIMER_SECOND/100)//=163, 0.005s

#define WAIT_TO_SEND_RTS			(DIFS+(random_rand() % CW)*SIGMA) //time duration for waiting to send RTS
#define WAIT_TO_SEND_GRADE		WAIT_TO_SEND_RTS //time duration for waiting to send the GRADE message
//#define WAIT_TO_RCV_RTS			(DIFS+CW*SIGMA+PRI_CHECK_TIME_TX+DURRTS+ WAKEUP_ADVANCE_TIME+R_END_GUARD_TIME + GUARDTIME) //time duration for waiting to receiving RTS
#define WAIT_TO_RCV_RTS			(DIFS+CW*SIGMA+DURRTS+ WAKEUP_ADVANCE_TIME+R_END_GUARD_TIME + GUARDTIME) //time duration for waiting to receiving RTS

#define WAIT_TO_REG_BCAST_CTS			(SIFS+(random_rand()% CW)*SIGMA) //time duration for waiting to contend for replying with CTS gregularly
//#define	WAIT_TO_BCAST_CTS_FOR_UNIT_RTS		(SIFS+random_rand() % UCAST_CTS_CW)//time duration for waiting to contend for replying with CTS after receiving a unicasted RTS not for me
#define	WAIT_TO_UCAST_CTS			(SIFS) //time duration for waiting to unicast CTS

//#define WAIT_TO_RCV_CTS			(SIFS+CW*SIGMA+DURCTS+GUARDTIME+PRI_CHECK_TIME_TX)
#define WAIT_TO_RCV_CTS			(SIFS+CW*SIGMA+DURCTS+GUARDTIME)
#define WAIT_TO_RCV_DATA		(SIFS+DURDATA+GUARDTIME)//+PRI_CHECK_TIME_TX) 

#define DURSLOT		((uint32_t)(DIFS+2*CW*SIGMA+3*SIFS+DURRTS+DURCTS+DURDATA+R_END_GUARD_TIME))//+DURACK)) //3367,duration of R or T (durR=durT)
/*There will be no 'T' state in the new version, but 'T' belongs to 'S'*/
#define DURSLEEP	((uint32_t)(SF+1)*(uint32_t)DURSLOT) //37037, duration of sleep (T_S in the paper, the 'T' state period is included).
#define DURCYCLE	((uint32_t)(SF+2)*(uint32_t)DURSLOT) //cycle time duration, (T_cycle in the paper).
/*end: The propagation time of grade message*/

#define RTMAXTIMER	((uint32_t)(RTIMER_SECOND-1)) // ((uint32_t)(RTIMER_SECOND-1)*2)

static volatile uint8_t state_left_second_round;


#define PRI_SHOW_LED_SUPPORT 1

#if ZOLERTIA_Z1 || TMOTE_SKY || WISEMOTE || EXP5438
	#define MAX_CLOCK_DRIFT		(WAKEUP_ADVANCE_TIME/3)// 3 //if the clock drift amount is larger than this number, then adjust the cycle period.
#else
	#define MAX_CLOCK_DRIFT		(WAKEUP_ADVANCE_TIME/3)// 3 //if the clock drift amount is larger than this number, then adjust the cycle period.
#endif

#define MAX_CYCLE_COUNTER	(((int32_t)(MAX_CLOCK_DRIFT))<<8)//(((uint32_t)(2)<<(sizeof(int16_t)*8-1))-1)// (uint32_t)5//   /*if the two boards sync with each other very well, then we need to set this MAX_CYCLE_COUNTER*/
#define MAX_COUNT_NO_RTS_CTS		-1	//maximum times there are no RTS/CTS received, then keep active, -1: don't do such a counting, i.e., always keep the current schedule
#define	MAX_CHILD_NUM		10 //The maximum number of senders a node can have (in its neighbor table)
#define	MAX_CHILD_NUM_AT_SINK	15
#define	MAX_TRY_TO_FIND_PARENT  3 //	5 // for each valid "pri_attribute.sender_num_in_receivers", the maximum try to find receiver. 
									  //When failing in all tries, increase "pri_attribute.sender_num_in_receivers" until to MAX_CHILD_NUM

//#define PERIODIC_SECOND		10	/*everty 10s, send RTS*/
#ifndef PERIODIC_CYCLES
	#define	PERIODIC_CYCLES		3 /*every 10 cycles, send RTS*/
#endif
	
/*everty the following cycles (corresponding to PERIODIC_SECOND), send RTS*/
#define PERIODIC_BCAST_CYCLES_SENSOR	PERIODIC_CYCLES// ((PERIODIC_SECOND*(uint32_t)(RTIMER_SECOND))/DURCYCLE)// 10 //-1// 3 //20 cycles: after these cycles, broadcast the RTS again
#define PERIODIC_BCAST_CYCLES_SINK	 (PERIODIC_CYCLES)//(((PERIODIC_SECOND/2)*(uint32_t)(RTIMER_SECOND))/DURCYCLE)// 3 //-1//20 cycles: after these cycles, broadcast the RTS again
#define NORTS_CYCLES	(100*PERIODIC_BCAST_CYCLES_SENSOR)


#ifndef MAX_ACTIVE_CYCLES
	#define MAX_ACTIVE_CYCLES		5	//maximum cycles for keeping active after determining its grade.
#endif

#ifndef RCV_GRADE_NUM_TO_SET
	#define	RCV_GRADE_NUM_TO_SET	1 // 2 // 5 //when the node grade is or becomes -1, the node will wait such number +1 of receivings to determine its new grade
#endif
//#define MAX_CYCLE_COUNT	30

/**
*	switch of schedule synchronization module
*/ 
#ifndef PRI_SYNC_SUPPORT  
    #define PRI_SYNC_SUPPORT 	1 // 1: pdc_sync; 0: pdc_idea
#endif
#if PRI_SYNC_SUPPORT
	#ifndef	RANDOM_DRIFT_IN_COOJA_SUPPORT
		#define	RANDOM_DRIFT_IN_COOJA_SUPPORT	0
	#endif 
	#if RANDOM_DRIFT_IN_COOJA_SUPPORT && IS_COOJA_SIM
		#define	MAX_DRIFT_PER_CYCLE		20
	#endif
#else
	#ifdef RANDOM_DRIFT_IN_COOJA_SUPPORT 
		#undef RANDOM_DRIFT_IN_COOJA_SUPPORT  
	#endif
	#define	RANDOM_DRIFT_IN_COOJA_SUPPORT	0
#endif

/**
*	switch of address-free module
*/ 

#ifndef PROC_GRADE_BASED_ON_ADDR_SUPPORT //this variable is also defined in Makefile
	#define PROC_GRADE_BASED_ON_ADDR_SUPPORT	0  //process the grade message according to address (1) or transmission range (0)
#endif

#ifndef ADDRESS_FREE_SUPPORT
	#define	ADDRESS_FREE_SUPPORT	0 //this variable is also defined in Makefile
#endif
  
#if ADDRESS_FREE_SUPPORT   
	#define MAX_RID		254//The maximum random ID (2^8). Don't use 255 and 256 since 0 and 1 won't be used.
#endif
            
#if	PROC_GRADE_BASED_ON_ADDR_SUPPORT    
	#ifndef	NODES_NUM_PER_GRADE
		#define NODES_NUM_PER_GRADE		2
	#endif
#endif

/**
*	switch of mulitple channels module
*/
#ifndef MPDC_SUPPORT
    #define MPDC_SUPPORT 1 // 1: mulitiple support; 0: pdc_idea
#endif

#if MPDC_SUPPORT
    #ifndef NUM_CHANNELS
        #define NUM_CHANNELS 2	  //denoted by M in MPDC paper
    #endif
//    #ifndef NUM_PATHS
//		#define NUM_PATHS  3//denoted by N in MPDC paper
//	#endif
    #ifndef MIN_HOPS_BETWEEN_ADJCENT_NODES
		#define MIN_HOPS_BETWEEN_ADJCENT_NODES 4 //denoted by /sigma in MPDC paper
    #endif
    #ifndef IS_NOT_STAGGER
        #define IS_NOT_STAGGER 0 //0:with schedule staggering /1: without schedule staggering
    #endif
#else
    #define IS_NOT_STAGGER 0
#endif



/* The cycle time for announcements. */
#ifndef START_GRADE
	//#define START_GRADE (2 * CLOCK_SECOND) //0//(2 * CLOCK_SECOND) /*2s, 等待时间必须<=1s (why?)*/
//	#ifdef MPDC_SUPPORT
//		#define START_GRADE (2 * DURCYCLE)
//	#else
		#define START_GRADE  (2 * CLOCK_SECOND)
//	#endif
#endif
#ifndef START_PROBE
	#define START_PROBE (2 * CLOCK_SECOND)
#endif



/* the header of packet */
struct pri_hdr_t {
  uint8_t dispatch; /* whether dispatch the packet */
  uint8_t type;/*the type of packet,such as RTS, CTS,DATA, etc.*/
}__attribute__((packed));
static const int hdr_len = sizeof(struct pri_hdr_t);

typedef int8_t pri_grade_t; //pdcadc grade type
/*the structure of the grade message*/
struct grade_msg_t{
	pri_grade_t grade;//The grade of the current sending node
	uint16_t state_dur;//The time that the sending node has been in the T state when sending IM
#if MPDC_SUPPORT
	linkaddr_t source;//Node ID or address of the current sending node
	uint8_t channel_id;//The ID of the channel that the node is allocated with
	uint8_t order;//The sending order of the message sent by the sink node.
#endif
}__attribute__((packed));


struct rts_msg_t{//the structure of the rts message
	pri_grade_t grade;
	uint16_t state_dur;
	int8_t not_for_data; //if this flag is set, the RTS/CTS is not for data transmission.
	//uint8_t has_receiver; // indicate whether the neighbor list is empty

	int8_t sender_num_in_receivers; // if lrts is broadcasted: if you have such a number of senders or less, please reply me, I'd like to be your sender
									// if rts is unicasted: this is the number of senders my receiver has
	int16_t clock_drift; //send the clock_drift between me and the receiver to the receiver
}__attribute__((packed));

struct cts_msg_t{//the structure of the cts message
	pri_grade_t grade;
	uint16_t state_dur;

	int16_t time_offset; //this is to tell my sender, what's the time_offset I used to sync with my receiver
	uint8_t sender_num;

#if ADDRESS_FREE_SUPPORT
	linkaddr_t	addr_for_child; ///this address is prepared by the current parent for the child (which will receiver this CTS) 
								/// if address conflict is found by the current parent after it receives a broadcasted RTS from that child.
								/// if addr_for_child is set to 0 by the current parent, the child does not need to regenerate a new address.
#endif
	uint8_t	queue_is_full;
 
}__attribute__((packed));



//DATA Transmission
#ifndef DATA_TX_SUPPORT
	#define DATA_TX_SUPPORT		1 //this variable is also defined in makefile
#endif  
#if DATA_TX_SUPPORT
	#ifndef GEN_DATA_IN_R
		/*GEN_DATA_IN_R: 1: generate data in R status; 0: generate data in function pri_qsend_packet;*/
		#define GEN_DATA_IN_R	1
	#endif
	#ifndef MAX_DATA_QUEUE_BUFS
		#define MAX_DATA_QUEUE_BUFS 10
	#endif
	#ifndef TOTAL_DATA_NUM
		#define TOTAL_DATA_NUM			5 // <0: infinite number of packets //this variable is also defined in makefile
	#endif
	#ifndef DATA_INTERVAL
		#define	DATA_INTERVAL	10
	#endif
	/// CYCLES_PER_PACKET: used to control how many packets to be sent per second
	#define	CYCLES_PER_PACKET		(((DATA_INTERVAL+random_rand()%10)*(uint32_t)(RTIMER_SECOND))/DURCYCLE)/*unit: cycles/packet, send one packet every such cycles (30 s)*/
	#ifndef RETX_LIMIT
		#define RETX_LIMIT	11 // 11/*retransmission limit: =0: no limit; >0: transmission times, i.e., (RETX_LIMIT-1) retransmissions*/
	#endif
	struct data_msg_t{
		pri_grade_t src_grade;
		linkaddr_t	src_addr; //indicate that the current node is the source who generates the packet or not.
		uint32_t	gen_or_rcv_time; //generated or received time
		uint32_t	delay;
		uint32_t	ID; //data ID
		uint8_t		has_temp; //indicate whether it has temperature or not
	#if ZOLERTIA_Z1
		/*temperature:*/
		char minus; 
		int16_t  tempint; 
    	uint16_t tempfrac;
	#endif
	}__attribute__((packed));
	/* List of packets to be sent by pdcadc */
	struct rdc_pri_buf {
	  	struct rdc_pri_buf *next;
	  	struct data_msg_t data;
	};
#endif


typedef int8_t pri_int_t;
struct pri_control_t{
	pri_int_t is_sending_data; //to send rts
	pri_int_t is_receiving_data;//to rcv cts
	pri_int_t channel_is_busy;
	pri_int_t sent_rts;
	pri_int_t rcvd_rts;
	pri_int_t sent_cts;
	pri_int_t rcvd_cts;
	pri_int_t wait_tosend_timer_over;
	pri_int_t wait_cts_timer_over;
	pri_int_t wait_rts_timer_over;
	pri_int_t rcvd_rts_not_for_data;
	pri_int_t wait_data_timer_over;
	linkaddr_t sender_addr;
	linkaddr_t receiver_addr;
	pri_int_t rcvd_bro_rts; //flag: the node received a broadcasted rts;
	
	uint16_t node_state_dur;//state duration of the node
	uint16_t msg_state_dur;// the state duration of a node in its sending message
	int16_t clock_drift;
	uint8_t	queue_is_full;
	
	int pri_ret;
	
};
static volatile struct pri_control_t pri_control;


/*attributes maintained by pdcadc for scheduling*/
struct pri_attribute_t 
{
	pri_grade_t grade;/*grade the node is in.*/
	pri_grade_t temp_grade; /*temporary grade*/
	//pri_grade_t pre_grade;
	char state;  /*node's current state \in {PRI_R, PRI_T,PRI_S}*/
	rtimer_clock_t state_start; /*the start time of the current state*/

	uint8_t	pri_type; /*the node type \in {PRI_SINK, PRI_SENSOR}*/
	int8_t	forward_grade; /*flag: should forward grade message?*/
	int8_t	periodic_rts_not_for_data_flag;
	int8_t	clock_drift_rts_not_for_data_flag;
	//int8_t  rcvd_rts_not_for_data;
	int8_t	rcvd_cts_after_rts; //indicate whether the node has received a CTS after it broadcasted a RTS. Only in this case, the node can reply CTS for the received RTS.
	//int8_t retx_time;
	uint32_t data_gen_num; //the number of generated data
	uint32_t data_rcvd_num_at_sink;
	//uint16_t cycle_count;
	int8_t	random_active_cycles; //don't use uint8_t, since this variable can be negative
	uint8_t multiple_for_no_cts;
	//uint8_t count_no_cts; //# of times that there is no CTS after broadcasting RTS
	uint8_t	try_num_to_find_receiver; //Duing Broadcast, for each valid "pri_attribute.sender_num_in_receivers", 
									  //record the number of try to find a receiver, until to its maximum: MAX_TRY_TO_FIND_PARENT. 
	uint8_t cycle_count_periodic_bcast;//periodic send RTS
	uint8_t cycle_count_no_rts; //if there is no rts received in R state, increase this counter, otherwise, set it to 0

	uint8_t to_broadcast_rts; //flag to indicate the RTS is broadcasted or Unicasted. 0: unicast, 1: broadcast
	uint8_t has_receiver;	// indicate whether the neighbor list of the current node is empty, used when sending RTS


	uint8_t	rcv_grade_num_to_setgrade;//when the node grade becomes -1, the node will wait such number of receivings to determine its new grade
	
	int8_t sender_num_in_receivers; // if you have such a number of senders or less, please reply me, I'd like to be your sender;
									//starting from 0;
									// if it's -1, it means I have found a relay.
#if DATA_TX_SUPPORT
	uint8_t	data_cycles_counter; //every CYCLES_PER_PACKET cycles, send one packet
	uint16_t total_data_num;//total number of data that will be generated
	uint8_t retx_limit;
#endif
	int8_t	drift_per_cycle; // 0 or drift_direct
#if RANDOM_DRIFT_IN_COOJA_SUPPORT && IS_COOJA_SIM
	uint8_t drift_speed; //clock drift speed: every such cycles, +1/-1 drift generated
	uint8_t	cycle_counter_for_cd; //cycle counter for clock drift
	int8_t	drift_direct; // 1 or -1
#endif
	uint32_t	durslot;
	uint32_t	dursleep;
	uint32_t	durcycle;
	uint8_t		fail_in_R;

#if MPDC_SUPPORT
	uint8_t		node_id;//node id
	uint8_t 	channel_id;//The ID of the channel that the node is allocated with
	uint8_t 	temp_channel;
	
	uint8_t 	cnt;
	uint8_t     order;
	linkaddr_t next_hop;// Node address of the next hop of the node
	linkaddr_t temp_next_hop;
	packetbuf_attr_t rssi;
	uint16_t temp_msg_state_dur;
	rtimer_clock_t temp_now;
	
#endif
};


#if MPDC_SUPPORT
	typedef int8_t pri_nodeid_t;
	struct nodeid_msg_t{
		pri_nodeid_t node_id;
	}__attribute__((packed));
#endif


/*End: MPDC protocol parameters*/

static volatile struct pri_attribute_t pri_attribute;



/* List of neighborhood: receivers*/
struct pri_neighbor_parent {
	struct pri_neighbor_parent *next;
	linkaddr_t receiver; //the next hop address
	uint8_t	count_no_cts; //the times there are no cts received after unicasting RTS to that next hop node
	int16_t clock_drift; //the clock_drift recorded last time when receiving a CTS
	int16_t	cd_base; //clock_drift_base, initialized to 0, used when calculating clock_drift_speed
	int32_t	cycle_counter; //(range: - (-2^31) ~ +  (+2^31-1) )
	uint8_t sender_num; //the number of senders the receiver has
};

/* List of neighborhood: senders*/
struct pri_neighbor_child {
	struct pri_neighbor_child *next;
	linkaddr_t sender; //the sender hop address
	uint8_t	count_no_rts; //the times there are not RTS unicasted to me. Don't count when receiving a broadcasted RTS
	int16_t time_offset; //record the time_offset I used to sync with my receiver. I will embed it into the CTS replied to each sender

	int16_t clock_drift_to_me; //this is to record the clock_drift between the sender and me. It's obtained from the RTS
};


#define MAX_PARENT_BUFS 1
#define MAX_CHILD_BUFS	  MAX_CHILD_NUM_AT_SINK

#if DATA_TX_SUPPORT
	
	/*for both List and MEMB, need to define the structure first: rdc_pri_buf*/
	LIST(rdc_pri_buf_list);
	MEMB(rdc_pri_buf_memb, struct rdc_pri_buf, MAX_DATA_QUEUE_BUFS);
#else
	/*for both List and MEMB, need to define the structure first: rdc_pri_buf*/
	LIST(rdc_pri_buf_list);
	MEMB(rdc_pri_buf_memb, int, 0);
#endif

LIST(pri_neighbor_parent_list);
MEMB(pri_neighbor_parent_memb, struct pri_neighbor_parent, MAX_PARENT_BUFS);

LIST(pri_neighbor_child_list);
MEMB(pri_neighbor_child_memb, struct pri_neighbor_child, MAX_CHILD_BUFS);

//#define TRANSFER_RTIMER_SECOND(remote_stick, remote_rtimer_second) 	((int16_t)((int32_t)remote_stick * (int32_t)RTIMER_SECOND / (int32_t)remote_rtimer_second))

#define DEBUGA 0
#if DEBUGA
//#include <stdio.h>
#define PRINTFA(...) printf(__VA_ARGS__)
#define PRINTDEBUGA(...) printf(__VA_ARGS__)
#else
#define PRINTFA(...) 		
#define PRINTDEBUGA(...)	
#endif

#define DEBUGB 0 // current printf
#if DEBUGB
//#include <stdio.h>
#define PRINTFB(...) printf(__VA_ARGS__)
#define PRINTDEBUGB(...) printf(__VA_ARGS__)
#else
#define PRINTFB(...) 		
#define PRINTDEBUGB(...)	
#endif

#define DEBUGLIST 1 // print list
#if DEBUGLIST
//#include <stdio.h>
#define PRTLIST(...) printf(__VA_ARGS__)
#define PRTLISTDEBUGB(...) printf(__VA_ARGS__)
#else
#define PRTLIST(...) 		
#define PRTLISTDEBUGB(...)	
#endif

#define DEBUG_OUTPUT 1 //print delay and packet reception ratio when a packet arrives at the sink node
#if DEBUG_OUTPUT
//#include <stdio.h>
#define PRT_OUTPUT(...) printf(__VA_ARGS__)
#define PRINTDEBUG_OUTPUT(...) printf(__VA_ARGS__)
#else
#define PRT_OUTPUT(...) 		
#define PRINTDEBUG_OUTPUT(...)	
#endif

#define DEBUG_OUTPUT_POWER 1 //print delay and packet reception ratio when a packet arrives at the sink node
#if DEBUG_OUTPUT_POWER
//#include <stdio.h>
#define PRT_OUTPUT_POWER(...) printf(__VA_ARGS__)
#define PRINTDEBUG_OUTPUT_POWER(...) printf(__VA_ARGS__)
#else
#define PRT_OUTPUT_POWER(...) 		
#define PRINTDEBUG_OUTPUT_POWER(...)	
#endif 
 
 
#define DEBUG_DEF 1 //print definition
#if DEBUG_DEF
//#include <stdio.h>
#define PRT_DEF(...) printf(__VA_ARGS__)
#define PRINTDEBUG_DEF(...) printf(__VA_ARGS__)
#else
#define PRT_DEF(...) 		
#define PRINTDEBUG_DEF(...)	
#endif


#define DEBUG_SYNC 1 //printf for synchronization
#if DEBUG_SYNC
//#include <stdio.h>
#define PRT_FORSYNC(...) printf(__VA_ARGS__)
#define PRINTDEBUG_FORSYNC(...) printf(__VA_ARGS__)
#else
#define PRT_FORSYNC(...) 		
#define PRINTDEBUG_FORSYNC(...)	
#endif


#define DEBUG_JF 0 //
#if DEBUG_JF
//#include <stdio.h>
#define PRTJF(...) printf(__VA_ARGS__)
#else
#define PRTJF(...) 		
#endif

/*------------------End: PRIMAC------------------*/

/*---------------------------------------------------------------------------*/
static void get_z1_temperature_to_set_data(struct data_msg_t * data_msg)
{
#if ZOLERTIA_Z1 && DATA_TX_SUPPORT
	int16_t  sign;
	int16_t  raw;
	uint16_t absraw;
	
	data_msg->minus = ' ';

	sign = 1;
 
	raw = tmp102_read_temp_raw();  // Reading from the sensor
 
    absraw = raw;
    if (raw < 0) { // Perform 2C's if sensor returned negative data
        absraw = (raw ^ 0xFFFF) + 1;
        sign = -1;
    }
	data_msg->tempint  = (absraw >> 8) * sign;
    data_msg->tempfrac = ((absraw>>4) % 16) * 625; // Info in 1/10000 of degree
    data_msg->minus = ((data_msg->tempint == 0) & (sign == -1)) ? '-'  : ' ' ;
	data_msg->has_temp = 1;
#endif
}


/*---------------------------------------------------------------------------*/
/*
*  \brief: 		show led according to node state
*  \param: 	node state
*/
//#include "lib/list.h"
static void pri_print_list(list_t list, uint8_t list_type)
{
#if DEBUGLIST
	//printf("Print a List:\n"); 
	if(list_type == CHILD_LIST)
	{
		struct pri_neighbor_child *l;
		if(list_head(list) == NULL)
		{
			//Empty Child Table (Sender List)
			//PRTLIST("EmptyCT\n");
			return;
		}
		PRTLIST("CT ");//Child Table, i.e., SL: sender list
		for(l = list_head(list); l != NULL; l = l->next)
		{
			//node address, cnr:count of no rts, time offset, clock_drift_to_me
			PRTLIST("[%u %u %d %d]",l->sender.u8[0],l->count_no_rts,l->time_offset,l->clock_drift_to_me);
		}
		//PRTLIST("\n");
		//ClockTime:
		PRTLIST(" %lu\n",clock_time());
	}else if(list_type == PARENT_LIST)
	{
		struct pri_neighbor_parent *l;
		if(list_head(list) == NULL)
		{
			//Empty Parent Table (Receiver List)
			//PRTLIST("EemptyPT\n");
			return;
		}
		//PRTLIST("PT "); //Parent Table, i.e., RL: Receiver List
		for(l = list_head(list); l != NULL; l = l->next)
		{
			//node address, count of no cts, cycle counter, clock drift, sender number of my receiver ([%u.%u cnc: %u \/ cc: %ld  cd: %d sn: %u])
			PRTLIST("PT [%u %u %ld %d %u]\n",l->receiver.u8[0],l->count_no_cts,
				l->cycle_counter, l->clock_drift, l->sender_num);
		}
		//ClockTime:
		//PRTLIST(" %lu\n",clock_time());
	}
 #endif 
}

static void
pri_show_led()
{
#if PRI_SHOW_LED_SUPPORT
	unsigned char led_color;
	if(pri_attribute.state == 'R'
	)
	{
		//led_color = LEDS_RED;
		leds_off(LEDS_ALL);
		leds_on(LEDS_RED);
		//printf("i am here.\n");
	}else if(pri_attribute.state == 'S'
	)
	{
		//led_color = LEDS_BLUE;
		leds_off(LEDS_ALL);
		leds_on(LEDS_BLUE);
	}else
	{
		PRINTFA("\n ERROR: In show_led: have a wrong node state");
	}
	//leds_off(LEDS_ALL);
	//leds_on(led_color);
#endif
}

	
/*---------------------------------------------------------------------------*/
static void
pri_compower_func(uint8_t	pkt_type)
{
#if PRIMAC_CONF_COMPOWER_SUPPORT
	/* Accumulate the power consumption for the packet transmission. */
	compower_accumulate(&current_packet);
	if(pkt_type == TYPE_RTS || pkt_type == TYPE_CTS || pkt_type == TYPE_GRADE)
	{
		pri_compower.all_ctr_listen += current_packet.listen;
		pri_compower.all_ctr_transmit += current_packet.transmit;
	}else if(pkt_type == TYPE_DATA)
	{
		pri_compower.all_data_listen += current_packet.listen;
		pri_compower.all_data_transmit += current_packet.transmit;
	}else{
	   pri_compower.all_ctr_listen += current_packet.listen;
	   pri_compower.all_ctr_transmit += current_packet.transmit;
	}

 	//pri_compower.all = energest_type_time(ENERGEST_TYPE_CPU) + energest_type_time(ENERGEST_TYPE_LPM);
	/* Convert the accumulated power consumption for the transmitted
	   packet to packet attributes so that the higher levels can keep
	   track of the amount of energy spent on transmitting the
	   packet. */
	compower_attrconv(&current_packet);

	/* Clear the accumulated power consumption so that it is ready for
	   the next packet. */
	compower_clear(&current_packet);
#endif /* PRIMAC_CONF_COMPOWER_SUPPORT */
}

/*---------------------------------------------------------------------------*/
static void
start_grade(void *ptr) 
{
    printf("hhctimer is expired,now send grade message\n");
	//watchdog_stop();
	uint32_t state_period;
	state_period = pri_state_period();
	pri_attribute.state_start = RTIMER_NOW();
	if(pdcadc_is_on)
	{
		rtimer_set(&rt, pri_attribute.state_start+ state_period, 1, (void (*)(struct rtimer *, void *))pri_powercycle, NULL); /*enter into 'S' state*/
	}
}
static void
start_test(void *ptr)
{
    printf("ctimer is expired,now send grade message\n");
}
/*---------------------------------------------------------------------------*/

/*sink node(1.0) send probe message to learn all N neighbors*/
#if MPDC_SUPPORT
static int
send_probe_message(){
	PRTJF("broadcast probe message\n");
	struct pri_hdr_t *hdr_ptr;
	rtimer_clock_t now;
	int ret_value;
	/*Set up the control header*/
	packetbuf_clear();
	if(packetbuf_hdralloc(hdr_len))/*set the header of the packetbuf*/
	{
		hdr_ptr = packetbuf_hdrptr();
		hdr_ptr->dispatch = DISPATCH;
		hdr_ptr->type = TYPE_PROBE;
	}				
	packetbuf_set_addr(PACKETBUF_ADDR_SENDER, &linkaddr_node_addr);
	packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER, &linkaddr_null);
	
	packetbuf_compact();
	ret_value = RADIO_TX_COLLISION;
	if(NETSTACK_FRAMER.create() >= 0) 
	{
		if(NETSTACK_RADIO.channel_clear() != 0)
		{
			ret_value = NETSTACK_RADIO.send(packetbuf_hdrptr(), packetbuf_totlen());
		}
		
		pri_compower_func(TYPE_PROBE);		
	}
	// 探测信息广播结束了，设置定时器，定时器过期后，发送等级划分信息
	ctimer_set(&grade_start_ctimer, START_GRADE, start_grade, NULL);
	printf("ctimer is set,now time is %lu\n",RTIMER_NOW());
	return ret_value;
}
#endif


#if MPDC_SUPPORT
//send ack to the probe messgae 
static int
send_probe_message_ack()
{
	PRTJF("sending the ack of probe message\n");
	struct pri_hdr_t *hdr_ptr;
	rtimer_clock_t now;
	int ret_value;

	packetbuf_clear();
	if(packetbuf_hdralloc(hdr_len))/*set the header of the packetbuf*/
	{
		hdr_ptr = packetbuf_hdrptr();
		hdr_ptr->dispatch = DISPATCH;
		hdr_ptr->type = TYPE_PROBE_R;
	}		
	packetbuf_set_addr(PACKETBUF_ADDR_SENDER, &linkaddr_node_addr);
	packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER, &linkaddr_sink);
	linkaddr_t linkaddr_t;
	linkaddr_copy(&linkaddr_t,packetbuf_addr(PACKETBUF_ADDR_RECEIVER));
	PRTJF("receiver address is %u.%u\n",linkaddr_t.u8[0],linkaddr_t.u8[1]);
	
	packetbuf_compact();
	ret_value = RADIO_TX_COLLISION;
	if(NETSTACK_FRAMER.create() >= 0) 
	{
		if(NETSTACK_RADIO.channel_clear() != 0)
		{
			ret_value = NETSTACK_RADIO.send(packetbuf_hdrptr(), packetbuf_totlen());
		}

		pri_compower_func(TYPE_PROBE_R);	
	}
	return ret_value;
}
#endif

#if MPDC_SUPPORT
static int
send_nodeid_message() //只在pri_powercycle里调用
{

	struct pri_hdr_t *hdr_ptr;
	struct nodeid_msg_t nodid_msg, *data_ptr;
	rtimer_clock_t now;
	int ret_value;
	/*Set up the control header*/
	packetbuf_clear();
	if(packetbuf_hdralloc(hdr_len))/*set the header of the packetbuf*/
	{
		hdr_ptr = packetbuf_hdrptr();
		hdr_ptr->dispatch = DISPATCH;
		hdr_ptr->type = TYPE_NODEID;
	}	
	pri_attribute.cnt++;
	nodid_msg.node_id = pri_attribute.cnt;
	packetbuf_set_datalen(sizeof(struct nodeid_msg_t));
	packetbuf_set_addr(PACKETBUF_ADDR_SENDER, &linkaddr_node_addr);
	packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER, &pri_control.sender_addr); 
	linkaddr_t linkaddr_t;
	linkaddr_copy(&linkaddr_t,packetbuf_addr(PACKETBUF_ADDR_RECEIVER));
	
	packetbuf_compact();
	ret_value = RADIO_TX_COLLISION;
	if(NETSTACK_FRAMER.create() >= 0) 
	{
		memcpy(packetbuf_dataptr(),&nodid_msg,sizeof(struct nodeid_msg_t));
		data_ptr = (struct nodeid_msg_t*)packetbuf_dataptr();
		ret_value = NETSTACK_RADIO.send(packetbuf_hdrptr(), packetbuf_totlen());
		pri_compower_func(TYPE_NODEID);
	}
	return ret_value;
}
#endif


#if MPDC_SUPPORT
send_grade_mc(uint16_t state_dur,uint8_t order){
	PRTJF("now state dur is %u \n",state_dur);
	struct pri_hdr_t *hdr_ptr;
	struct grade_msg_t grade_msg, *data_ptr;
	rtimer_clock_t now;
	int ret_value;
	/*Set up the control header*/
	packetbuf_clear();
	if(packetbuf_hdralloc(hdr_len))/*set the header of the packetbuf*/
	{
		hdr_ptr = packetbuf_hdrptr();
		hdr_ptr->dispatch = DISPATCH;
		hdr_ptr->type = TYPE_GRADE;
	}		
	packetbuf_set_datalen(sizeof(struct grade_msg_t));
			
	packetbuf_set_addr(PACKETBUF_ADDR_SENDER, &linkaddr_node_addr);
	packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER, &linkaddr_null); //broadcast
	if(linkaddr_cmp(&linkaddr_node_addr,&linkaddr_sink)){
		hdr_ptr->type = TYPE_GRADE_SINK;//grade message send by sink node!
	}
	packetbuf_compact();
	ret_value = RADIO_TX_COLLISION;
	if(NETSTACK_FRAMER.create() >= 0) 
	{
		grade_msg.grade = pri_attribute.grade;
		grade_msg.source = linkaddr_node_addr;
		grade_msg.channel_id = pri_attribute.channel_id;//0+MINCHAEENL
		PRTJF("channel id is%u\n",grade_msg.channel_id);
		grade_msg.order = order;
		now = RTIMER_NOW();
		grade_msg.state_dur = now - pri_attribute.state_start;
		if(linkaddr_cmp(&linkaddr_node_addr,&linkaddr_sink)){
			grade_msg.state_dur = state_dur;
		}
		memcpy(packetbuf_dataptr(),&grade_msg,sizeof(struct grade_msg_t));
		data_ptr = (struct grade_msg_t*)packetbuf_dataptr();
		ret_value = NETSTACK_RADIO.send(packetbuf_hdrptr(), packetbuf_totlen());

		pri_compower_func(TYPE_GRADE_SINK);
	}
	//change channel after send grade message
	const radio_value_t channel_t = cc2420_get_channel() - pri_attribute.channel_id;
	int rd;
	rd = NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, channel_t);
	return ret_value;
}
#endif
/*---------------------------------------------------------------------------*/
/*
*  To send GRADE message
*/
static int
send_grade() //只在pri_powercycle里调用
{
	struct pri_hdr_t *hdr_ptr;
	struct grade_msg_t grade_msg, *data_ptr;
	rtimer_clock_t now;
	int ret_value;
	/*Set up the control header*/
	packetbuf_clear();
	if(packetbuf_hdralloc(hdr_len))/*set the header of the packetbuf*/
	{
		hdr_ptr = packetbuf_hdrptr();
		hdr_ptr->dispatch = DISPATCH;
		hdr_ptr->type = TYPE_GRADE;
	}		
	packetbuf_set_datalen(sizeof(struct grade_msg_t));
			
	packetbuf_set_addr(PACKETBUF_ADDR_SENDER, &linkaddr_node_addr);
	packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER, &linkaddr_null); //broadcast
	
	packetbuf_compact();
	ret_value = RADIO_TX_COLLISION;
	if(NETSTACK_FRAMER.create() >= 0) 
	{
		grade_msg.grade = pri_attribute.grade;
		//grade_msg.local_rtimer_second = RTIMER_SECOND;
		memcpy(packetbuf_dataptr(),&grade_msg,sizeof(struct grade_msg_t));
		data_ptr = (struct grade_msg_t*)packetbuf_dataptr();
		now = RTIMER_NOW();
		data_ptr->state_dur = now - pri_attribute.state_start;

		ret_value = NETSTACK_RADIO.send(packetbuf_hdrptr(), packetbuf_totlen());

		pri_compower_func(TYPE_GRADE);
	}
	return ret_value;
}

/*---------------------------------------------------------------------------*/
/*
*  To send RTS message
*/
static int
send_rts(const int8_t not_for_data) //只在pri_powercycle里调用
{
	struct pri_hdr_t *hdr_ptr;
	struct rts_msg_t rts_msg, *data_ptr;
	rtimer_clock_t now;
	int ret_value;
	/*Set up the control header*/
	packetbuf_clear();
	if(packetbuf_hdralloc(hdr_len))/*set the header of the packetbuf*/
	{
		hdr_ptr = packetbuf_hdrptr();
		hdr_ptr->dispatch = DISPATCH;
		hdr_ptr->type = TYPE_RTS;
	}		
	packetbuf_set_datalen(sizeof(struct rts_msg_t));
			
	packetbuf_set_addr(PACKETBUF_ADDR_SENDER, &linkaddr_node_addr);

#if MPDC_SUPPORT && !IS_NOT_STAGGER
	packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER, &pri_attribute.next_hop);
#else
	packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER, &linkaddr_null);
#endif

	PRTJF("i am sending to %u\n",packetbuf_addr(PACKETBUF_ADDR_RECEIVER)->u8[0]);
	
	packetbuf_compact();
	ret_value = RADIO_TX_COLLISION;
	if(NETSTACK_FRAMER.create() >= 0) 
	{
		rts_msg.grade = pri_attribute.grade;
		rts_msg.not_for_data = not_for_data;
	

		memcpy(packetbuf_dataptr(),&rts_msg,sizeof(struct rts_msg_t));
		data_ptr = (struct rts_msg_t*)packetbuf_dataptr();
		now = RTIMER_NOW();
		
		data_ptr->state_dur = now - pri_attribute.state_start;


		if(NETSTACK_RADIO.channel_clear() != 0)
		{
			ret_value = NETSTACK_RADIO.send(packetbuf_hdrptr(), packetbuf_totlen());
		}
		pri_compower_func(TYPE_RTS);		
	}
	return ret_value;
}
/*---------------------------------------------------------------------------*/
static struct pri_neighbor_child * 
find_child_neighbor(const linkaddr_t *address_ptr)
{
	struct pri_neighbor_child *l;
	for(l=list_head(pri_neighbor_child_list); l!=NULL; l=l->next)
	{
		if(linkaddr_cmp(&l->sender,address_ptr))
		{
			return l;
		}
	}
	//printf("no finding.\n");
	return NULL;
}

/*---------------------------------------------------------------------------*/
/*
*  To send CTS message
*/
static int
send_cts()//(const linkaddr_t* dest_addr_ptr) //只在pri_powercycle里调用
{
	struct pri_hdr_t *hdr_ptr;
	struct cts_msg_t cts_msg, *data_ptr;
	rtimer_clock_t now;
	int ret_value;

	/*Set up the control header*/
	packetbuf_clear();
	if(packetbuf_hdralloc(hdr_len))/*set the header of the packetbuf*/
	{
		hdr_ptr = packetbuf_hdrptr();
		hdr_ptr->dispatch = DISPATCH;
		hdr_ptr->type = TYPE_CTS;
	}		
	packetbuf_set_datalen(sizeof(struct cts_msg_t));
			
	packetbuf_set_addr(PACKETBUF_ADDR_SENDER, &linkaddr_node_addr);
	packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER, &pri_control.sender_addr); //if dest_addr_ptr=linkaddr_null, it means that CTS is broadcast
	PRTJF("here 2.2 i am sending to %u\n",pri_control.sender_addr.u8[0]);
	packetbuf_compact();
	ret_value = RADIO_TX_COLLISION;
	if(NETSTACK_FRAMER.create() >= 0) 
	{
		cts_msg.grade = pri_attribute.grade;
	/*	struct pri_neighbor_child * s_tb = NULL;
		s_tb=find_child_neighbor(&pri_control.sender_addr);
		if(s_tb != NULL)
		{
			cts_msg.time_offset = s_tb->time_offset;
			s_tb->time_offset = 0;
		}else
		{
			cts_msg.time_offset = 0;
		}
		cts_msg.sender_num = list_length(pri_neighbor_child_list);
        */
		if(list_length(rdc_pri_buf_list) >= MAX_DATA_QUEUE_BUFS)
		{
			cts_msg.queue_is_full=1;
		}else{
			cts_msg.queue_is_full=0;
		}
		memcpy(packetbuf_dataptr(),&cts_msg,sizeof(struct cts_msg_t));
		data_ptr = (struct cts_msg_t*)packetbuf_dataptr();
		now = RTIMER_NOW();

		data_ptr->state_dur = now - (pri_attribute.state_start + WAKEUP_ADVANCE_TIME);


		if(NETSTACK_RADIO.channel_clear() != 0)
		{
			ret_value = NETSTACK_RADIO.send(packetbuf_hdrptr(), packetbuf_totlen());
		}

		pri_compower_func(TYPE_CTS);	
	}
	return ret_value;
}

/*---------------------------------------------------------------------------*/
static void
pri_schedule_powercycle_fixed(struct rtimer *r_t, rtimer_clock_t fixed_time)
{
	int r;
	if(pdcadc_is_on)
	{
	#if 1
		rtimer_clock_t now = RTIMER_NOW();
    	if(RTIMER_CLOCK_LT(fixed_time, now + RTIMER_GUARD_TIME)) {
      		fixed_time = now + RTIMER_GUARD_TIME;
    	}
	#endif
		r = rtimer_set(r_t, fixed_time, 1,(void (*)(struct rtimer *, void *))pri_powercycle, NULL);
	    if(r != RTIMER_OK) 
		{
    	//	printf("ERROR: schedule_powercycle: could not set rtimer\n");
    	}
	}
	
}
/*---------------------------------------------------------------------------*/
#if DATA_TX_SUPPORT
static int
send_pridata(struct data_msg_t * data_elem, const linkaddr_t * dest_addr)
{
	//struct queuebuf *packet;  
	struct	pri_hdr_t *hdr_ptr;
	PRTJF("send_pridata is called !\n");
	
	int ret_value;

	packetbuf_clear();
	if(packetbuf_hdralloc(hdr_len)) {//分配header空间,实际是将hdrptr减去size
		hdr_ptr = packetbuf_hdrptr();
		hdr_ptr->dispatch = NONDISPATCH;
		hdr_ptr->type = TYPE_DATA;
  	}
	packetbuf_set_datalen(sizeof(struct data_msg_t));
	packetbuf_set_addr(PACKETBUF_ADDR_SENDER, &linkaddr_node_addr);//sender address
	packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER, dest_addr);//receiver address
//	PRTJF("here 3 i am sending to %u\n",dest_addr->u8[0]);
	packetbuf_compact();
	ret_value = RADIO_TX_COLLISION;

	if(NETSTACK_FRAMER.create() >= 0) 
	{
		data_elem->delay += clock_time() - data_elem->gen_or_rcv_time;
		memcpy(packetbuf_dataptr(),data_elem,sizeof(struct data_msg_t));
		ret_value = NETSTACK_RADIO.send(packetbuf_hdrptr(), packetbuf_totlen());

		pri_compower_func(TYPE_DATA);	
	}
	return ret_value;
}
#endif

/*---------------------------------------------------------------------------*/
static void
pri_qsend_packet(mac_callback_t sent, void *ptr) //NETSTACK_RDC.send函数，调用的是该函数
{
	// void *ptr: struct channel *c
#if DATA_TX_SUPPORT && !GEN_DATA_IN_R/*Comment DATA_TX_SUPPORT, because we will use the code in pri_powercycle() to generate packet*/
	if(list_head(pri_neighbor_parent_list)!=NULL && !pri_attribute.to_broadcast_rts && (pri_attribute.total_data_num > 0 || pri_attribute.total_data_num < 0)) /*generate data only when it's in unicast*/
	{
		//store the packet into queue first
		//space allocation
		#if 1 //new
		if(pri_attribute.total_data_num > 0)
		{
			pri_attribute.total_data_num--;
		}
		#endif

		struct rdc_pri_buf *list_elem;
		list_elem = memb_alloc(&rdc_pri_buf_memb);
		if(list_elem != NULL)
		{			
			struct data_msg_t data_msg;
			data_msg.gen_or_rcv_time = clock_time();
			data_msg.src_grade = pri_attribute.grade;
			data_msg.delay = 0;
			data_msg.has_temp = 0;
		#if ZOLERTIA_Z1
			//get_z1_temperature_to_set_data(&data_msg);
		#endif
			pri_attribute.data_gen_num++;
			data_msg.ID = pri_attribute.data_gen_num;

			linkaddr_copy(&data_msg.src_addr,&linkaddr_node_addr);
			
			list_elem->next=NULL; 
			list_elem->data = data_msg;
			list_add(rdc_pri_buf_list, list_elem); // add the item "list_elem" to the start of the list

		}else
		{
			printf("1. drop data, Length:%d\n",list_length(rdc_pri_buf_list));
		}
	}
#endif
}
/*---------------------------------------------------------------------------*/
static void
pri_qsend_list(mac_callback_t sent, void *ptr, struct rdc_buf_list *buf_list)
{
	if(buf_list != NULL) {
    	queuebuf_to_packetbuf(buf_list->buf);
    	pri_qsend_packet(sent, ptr);
  	}
}

/*---------------------------------------------------------------------------*/
static void pri_init_control(void)
{
	PRTJF("GEN_DATA_IN_R is %u\n",GEN_DATA_IN_R);
	int8_t i;
	pri_int_t *ptr;
	ptr = (pri_int_t *)&pri_control;
	for(i=0;i<sizeof(struct pri_control_t)/sizeof(int8_t);i++)
	{
		*ptr=0;
		ptr++;
	}
	pri_control.pri_ret = MAC_TX_ERR_FATAL;
}

/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
static void pri_free_list(struct memb m, list_t list)
{
	while(list_head(list) != NULL)
	{
		memb_free(&m,list_pop(list));
	}
}
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
static void pri_update_parent_neighbor(int16_t cts_time_offset, uint8_t cts_sender_num)//cts_time_offset: means the time_offset embedded in cts
{
	struct pri_neighbor_parent * list_elem;
	list_elem = list_head(pri_neighbor_parent_list);//find_receiver_neighbor(address_ptr);
	if(list_elem == NULL)//no receiver
	{
		list_elem = memb_alloc(&pri_neighbor_parent_memb);
		if(list_elem == NULL){
	   		 PRINTFB("\nWe could not allocate memory!\n");
	   		return;
		}
    	list_elem->next = NULL;
		linkaddr_copy(&list_elem->receiver,&pri_control.sender_addr);
    	list_elem->cycle_counter = 0; 
		list_elem->sender_num = cts_sender_num + 1;
		list_elem->cd_base = pri_control.clock_drift;
		list_add(pri_neighbor_parent_list,list_elem);
	}
	else //the rts might be unicasted
	{
		if(!linkaddr_cmp(&list_elem->receiver,&pri_control.sender_addr))//the sender of CTS is not as a receiver in my receiver_table
		{
			if(list_elem->sender_num - cts_sender_num > 1)//compare sender num
			{
				linkaddr_copy(&list_elem->receiver,&pri_control.sender_addr);
				list_elem->cycle_counter = 0;
				list_elem->cd_base = 0;
				list_elem->sender_num = cts_sender_num + 1;
				PRINTFB("the sender changes its receiver from to %u.%u.\n",pri_control.sender_addr.u8[0],pri_control.sender_addr.u8[1]);
			}else
			{
				return;
			}
		}else
		{
			list_elem->sender_num = cts_sender_num;
		}		
#if PRI_SYNC_SUPPORT
		if(cts_time_offset != 0)//my receiver adjusted its clock
		{
			list_elem->cd_base += cts_time_offset;
		}
#endif
	}
	list_elem->count_no_cts = 0;
	list_elem->clock_drift = pri_control.clock_drift;
}


/*---------------------------------------------------------------------------*/
static void 
pri_update_child_neighbor(void)
{
	struct pri_neighbor_child * list_elem=NULL;
	int8_t first_rts;
	first_rts=0;
	list_elem = find_child_neighbor(&pri_control.sender_addr);
	if(list_elem == NULL)// the first time to receive an rts (more likely an broadcasted RTS)
	{
		first_rts = 1; //the first time of receiving an RTS from this sender
		list_elem = memb_alloc(&pri_neighbor_child_memb);
		if(list_elem == NULL)
		{
		   return;
		}
		list_elem->next = NULL;
		linkaddr_copy(&list_elem->sender,&pri_control.sender_addr);
		list_elem->count_no_rts = 0;
		list_elem->time_offset = 0;
		list_elem->clock_drift_to_me = pri_control.clock_drift;
		list_add(pri_neighbor_child_list,list_elem);
		PRINTFB("Add a new sender to the neighbor_sender:%u.\n",list_elem->sender.u8[0]);
	}
	if(list_elem == NULL)
	{
		return;
	}
	list_elem->clock_drift_to_me = pri_control.clock_drift; //update the clock_drift_to_me
	if(!pri_control.rcvd_bro_rts)// a unicasted RTS
	{
		if(!first_rts)//not the first rts for me
		{
			list_elem->count_no_rts=0;
			
			struct pri_neighbor_child *l,*r;
			int l_length = list_length(pri_neighbor_child_list);
			for(l=list_head(pri_neighbor_child_list); l!=NULL; )
			{
				if(l!=list_elem)
				{
				#if MAX_COUNT_NO_RTS_CTS > 0
					l->count_no_rts++;
					if(l->count_no_rts >= l_length + MAX_COUNT_NO_RTS_CTS)
    				{
    					r=l->next;
						list_remove(pri_neighbor_child_list,l);
    					memb_free(&pri_neighbor_child_memb,l);//free memory
						l=r;
    				}else
    			#endif
    				{
    					l=l->next;
    				}
				}else
				{
					l=l->next;
				}
				
			}
		}
	}

}

static void
pri_powercycle_turn_radio_off(void)
{
#if PRIMAC_CONF_COMPOWER_SUPPORT
  uint8_t was_on = radio_is_on;
#endif /* PRIMAC_CONF_COMPOWER_SUPPORT */
  
  if(we_are_sending == 0) {
    off();
#if PRIMAC_CONF_COMPOWER_SUPPORT
    if(was_on && !radio_is_on) {
      compower_accumulate(&compower_idle_activity);
	}
#endif /* PRIMAC_CONF_COMPOWER_SUPPORT */
  }
}
/*---------------------------------------------------------------------------*/
static void
pri_powercycle_turn_radio_on(void)
{
  if(we_are_sending == 0) {
    on();
  }
}

/*---------------------------------------------------------------------------*/
static int16_t 
find_true_offset(int16_t time_offset)
{
	struct pri_neighbor_child *s_tb;
	int16_t temp_offset, min_offset;
	min_offset = time_offset;
	temp_offset = time_offset;
	for(s_tb=list_head(pri_neighbor_child_list); s_tb!=NULL; s_tb=s_tb->next)
	{
		//printf("to:%d, s_cd:%d.\n",time_offset,s_tb->clock_drift_to_me);
		if((time_offset + s_tb->clock_drift_to_me) >= ((int16_t)MAX_CLOCK_DRIFT))
		{
			//temp_offset = ((int16_t)MAX_CLOCK_DRIFT) + abs(s_tb->cd_difference_to_last) - s_tb->clock_drift_to_me;
			temp_offset = (int16_t)MAX_CLOCK_DRIFT + 5 - s_tb->clock_drift_to_me;
			//printf("1.temp_to:%d, cd: %d.\n",temp_offset, s_tb->clock_drift_to_me);
		}else if((time_offset + s_tb->clock_drift_to_me) <= -((int16_t)MAX_CLOCK_DRIFT))
		{
			//temp_offset = - ((int16_t)MAX_CLOCK_DRIFT) - abs(s_tb->cd_difference_to_last) - s_tb->clock_drift_to_me;
			temp_offset = - (int16_t)MAX_CLOCK_DRIFT - 5 - s_tb->clock_drift_to_me;
			//printf("2.temp_to:%d, cd: %d.\n",temp_offset, s_tb->clock_drift_to_me);
		}
		if(abs(min_offset) > abs(temp_offset))
		{
			min_offset = temp_offset;
		}
		//s_tb->time_offset = time_offset;
	}
	return min_offset; 
}

/*---------------------------------------------------------------------------*/
static char
pri_powercycle(struct rtimer *r_t, void *ptr) //state of a new state
{
//	PRTJF("pri_powercycle everytime \n");
	PT_BEGIN(&pt);
//	PRTJF("pri_powercycle once\n");
	static uint32_t t0;
	static rtimer_clock_t now;
	//static uint8_t rand_cw;
	static int j_for_large_sf; /*have to use static*/
	static int16_t time_offset;
	//static int ret_value;
	static struct pri_neighbor_parent * r_tb;
#if DATA_TX_SUPPORT
	static struct rdc_pri_buf * buf_elem;
#endif
#if PRIMAC_CONF_COMPOWER_SUPPORT
	static uint32_t all_time, all_listen, all_transmit, idle;
#endif

/*Begin: only executed in initialization*/
	if(ptr==&state_left_second_round)//to solve the problem when the cycle duration is larger than the maximum duration the timer can support. 在收到grade message时，设置了该指针
	{
		//printf("\n\nTest\n\n");
		static int i_for_large_sf;
		i_for_large_sf = state_left_second_round; 
		for(;i_for_large_sf>0;i_for_large_sf--)
		{
			now = RTIMER_NOW();
			 pri_schedule_powercycle_fixed(r_t, now+RTMAXTIMER);
             PT_YIELD(&pt);
		}
	}
/*End: only executed in initialization*/ 
	while(1) 
	{

		pri_attribute.state_start= RTIMER_NOW(); //state start time
		time_offset = 0;
		
		pri_init_control();
		/*regular wake, previous state is 'R', currently it is in 'T'*/
		if(pri_attribute.state=='R')
		{
			pri_attribute.state = 'S';
            if(pri_attribute.pri_type == PRI_SENSOR){
                pri_powercycle_turn_radio_off();//pri_show_led();
            }

			leds_off(LEDS_ALL);
			leds_on(LEDS_BLUE);  
					
			/* actually in T state*/
			//Below is to forward grade. If the data queue is empty, we will send RTS instead of GRADE msg.
			PRTJF("grade is %u .forward_grade is %u \n",pri_attribute.grade,pri_attribute.forward_grade);
			if((pri_attribute.grade>=0) && (pri_attribute.forward_grade==1) && (*rdc_pri_buf_list==NULL)
			)
			{
				pri_control.is_sending_data=1;
				t0 = pri_attribute.state_start+WAIT_TO_SEND_GRADE; //wait for DIFS + rand(CW) 
			#if 1
				pri_schedule_powercycle_fixed(r_t, t0);
				pri_control.wait_tosend_timer_over = -1;
				PT_YIELD(&pt);//give up the process occupation
				pri_control.wait_tosend_timer_over = 1;
				
				//pri_cca();
			#endif
				/*
				*	After DIFS+rand(CW) time, we have CCA_COUNT_MAX_TX times of CCA: Clear Channel Assessment;
				*	Use the following module to replace pri_cca() (which has been commented above);
				*	To send/forward grade message
				*/			

				/*
				*	send grade message,there are three situations as follows
				*   1: In MPDC, sink node(1.0) should send grade messgaes totally 「NUM_PATHS/NUM_CHANNELS」 times 
				*		with different settings of order.
				*	2: In MPDC, sensor nodes broadcast grade message with function send_grade_mc()
				*   3: When MPDC is not supported,node (both sink nodes and sensor nodes) 
				*   	broadcast grade message with function send_grade()
				*/
				if(!pri_control.channel_is_busy){
					pri_powercycle_turn_radio_on(); /*trun on radio (the radio might be turned off in 'R' state to save energy*/
				#if MPDC_SUPPORT && !IS_NOT_STAGGER
					if(pri_attribute.pri_type == PRI_SINK){
						rtimer_clock_t tt = RTIMER_NOW() - pri_attribute.state_start;
						int n,nums = num_paths/NUM_CHANNELS;
						PRTJF("ctimer here nums is %u\n",num_paths);
						if(num_paths%NUM_CHANNELS != 0) nums++;
						PRTJF("sigma*dursolt :%u\n",MIN_HOPS_BETWEEN_ADJCENT_NODES*DURSLOT);
						for(n = 1;n <= nums;n++){
							if(send_grade_mc(tt, n) == RADIO_TX_OK){
								PRTJF("send grade message success,and order is:%u\n",n);
							}else{
								PRTJF("send grade messgae error!");
							}
							t0 =  RTIMER_NOW()+ MIN_HOPS_BETWEEN_ADJCENT_NODES*DURSLOT;
							//wait sigma * durslot time
							while(RTIMER_CLOCK_LT(RTIMER_NOW(),t0)) { }
						}
					}else if(pri_attribute.pri_type == PRI_SENSOR){
						send_grade_mc(RTIMER_NOW() - pri_attribute.state_start, pri_attribute.order);
					}
				#else
					send_grade();
				#endif
				
					pri_attribute.forward_grade = 0;
				
				}else{
					pri_attribute.forward_grade=1;
				}
                if(pri_attribute.pri_type == PRI_SENSOR){
                    pri_powercycle_turn_radio_off();//pri_show_led();
                }
			}
			//Below is to send RTS
			//Then receive CTS, send DATA, receive ACK
			else if(
			(!pri_attribute.fail_in_R) && (pri_attribute.grade>=0) && 
				(list_head(rdc_pri_buf_list)!=NULL||pri_attribute.periodic_rts_not_for_data_flag==1 || pri_attribute.clock_drift_rts_not_for_data_flag==1))
			{
				pri_control.is_sending_data=1;
				if(pri_attribute.pri_type == PRI_SENSOR){
                    pri_powercycle_turn_radio_off();//pri_show_led();
                }
				// printf("periodic_rts_not_for_data_flag: %d. clock_drift_rts_not_for_data_flag: %d.\n",pri_attribute.periodic_rts_not_for_data_flag, pri_attribute.clock_drift_rts_not_for_data_flag);
				if(list_head(rdc_pri_buf_list) != NULL)
				{
					pri_attribute.periodic_rts_not_for_data_flag = 0;//data queue is not empty, so the RTS is for data transmission
					pri_attribute.clock_drift_rts_not_for_data_flag = 0;
					//linkaddr_copy(&pri_attribute.dest_addr,&((struct pri_neighbor_parent *)list_head(pri_neighbor_parent_list))->receiver);
				}else 
				{
					printf("EMPTY QUEUE\n");    
				} 
				t0 = pri_attribute.state_start+ WAIT_TO_SEND_RTS; //等待DIFS 和rand(CW)  时间
			#if 1
				pri_schedule_powercycle_fixed(r_t, t0);
				pri_control.wait_tosend_timer_over = -1;
				PT_YIELD(&pt);//进程主动让出执行权
				//printf("***Back. rand_cw=%lu.\n",rand_cw);
				pri_control.wait_tosend_timer_over = 1; 
				
				//pri_cca();
			#endif					 
				/*
				*	After DIFS+rand(CW) time, we have CCA_COUNT_MAX_TX times of CCA: Clear Channel Assessment;
				*	Use the following module to replace pri_cca() (which has been commented above);
				*	To send RTS
				*/
				{
					pri_powercycle_turn_radio_on(); //turn on radio to receive possible CTS	
					
					if(!pri_control.channel_is_busy && send_rts(pri_attribute.periodic_rts_not_for_data_flag|pri_attribute.clock_drift_rts_not_for_data_flag) == RADIO_TX_OK)
					{
						if(pri_attribute.pri_type == PRI_SENSOR)
						{
						
							 //printf("Have sent RTS, dest_addr=%u.%u.\n",pri_attribute.dest_addr.u8[0],pri_attribute.dest_addr.u8[1]);
							pri_attribute.cycle_count_periodic_bcast = 0;
							pri_control.sent_rts = 1;

							//wait CTS
						#if 1
							now = RTIMER_NOW();
							pri_control.wait_cts_timer_over = -1;
							pri_schedule_powercycle_fixed(r_t, now+WAIT_TO_RCV_CTS); //set timer for waiting for CTS
							PT_YIELD(&pt);
							pri_control.wait_cts_timer_over = 1;
						#endif

							if(pri_control.rcvd_cts)// && !pri_control.channel_is_busy)
							{
								
								if(!pri_attribute.periodic_rts_not_for_data_flag && !pri_attribute.clock_drift_rts_not_for_data_flag && !pri_control.queue_is_full)
								{//able to send data now
								//printf("cts\n");/*mar.19,2016*/
								#if DATA_TX_SUPPORT
                                    if(pri_attribute.pri_type == PRI_SENSOR){
                                        pri_powercycle_turn_radio_off();//pri_show_led();
                                    }
									/*After SIFS, send data*/
									now = RTIMER_NOW(); 
									t0 = now+SIFS;
									//while(RTIMER_CLOCK_LT(RTIMER_NOW(), t0)){}
									pri_schedule_powercycle_fixed(r_t, t0);
									PT_YIELD(&pt);
									if(1)//!pri_control.channel_is_busy)
									{
										buf_elem = list_head(rdc_pri_buf_list);
										pri_powercycle_turn_radio_on();
										if(send_pridata(&buf_elem->data,&pri_control.sender_addr) == RADIO_TX_OK)
										{
											pri_attribute.retx_limit = 0;
											list_remove(rdc_pri_buf_list,buf_elem);
											memb_free(&rdc_pri_buf_memb,buf_elem);//free memory
											//printf("sent the data, so can drop it.\n");
										}else
										{
											printf("Warning: data sending process fails.\n");
										}
									}else
									{
										// printf("Sending data but channel is busy.\n");
									}
								#endif
								
								}
                                if(pri_attribute.pri_type == PRI_SENSOR){
                                    pri_powercycle_turn_radio_off();//pri_show_led();
                                }

								pri_attribute.periodic_rts_not_for_data_flag=0;
								pri_attribute.clock_drift_rts_not_for_data_flag = 0;
								
							}else //no rcvd cts
							{
                                if(pri_attribute.pri_type == PRI_SENSOR){
                                    pri_powercycle_turn_radio_off();//pri_show_led();
                                }
								random_init(linkaddr_node_addr.u8[0]); //initialize the random seed /*mar.12,2016*/
								
								if(pri_attribute.pri_type!=PRI_SINK)
								{
									//printf("Didn't Receive CTS, go to sleep.\n");
								}
							}
						}else
						if(pri_attribute.pri_type == PRI_SINK)
						{
                            if(pri_attribute.pri_type == PRI_SENSOR){
                                pri_powercycle_turn_radio_off();//pri_show_led();
                            }
								
								pri_attribute.cycle_count_periodic_bcast = 0;
								pri_attribute.periodic_rts_not_for_data_flag=0;
	     						pri_attribute.clock_drift_rts_not_for_data_flag = 0;
								 //printf("I am sink node, after broadcasting RTS, go to sleep.\n");						
						}
					}
					else //fail in sending rts
					{
                        if(pri_attribute.pri_type == PRI_SENSOR){
                            pri_powercycle_turn_radio_off();//pri_show_led();
                        }

						//PRINTFB("Fail to send RTS: RADIO_TX_COLLISION: Channel is busy!\n");
						pri_control.pri_ret = MAC_TX_COLLISION;
						random_init(linkaddr_node_addr.u8[0]); //initialize the random seed /*mar.12,2016*/
					}
				}	

				//update retransmission limit
				#if DATA_TX_SUPPORT
    				if(list_head(rdc_pri_buf_list)!=NULL)
    				{
       					if(pri_attribute.retx_limit > 0) 
       					{
       						pri_attribute.retx_limit--;
       						if(pri_attribute.retx_limit == 0)
       						{
       							//remove the current data
       							pri_attribute.retx_limit = RETX_LIMIT;
       							buf_elem = list_head(rdc_pri_buf_list);
       							list_remove(rdc_pri_buf_list,buf_elem);
       							memb_free(&rdc_pri_buf_memb,buf_elem);//free memory
       							 printf("Warning: retx limit drop data\n");//,list_length(rdc_pri_buf_list));
       						}
       					}else if(pri_attribute.retx_limit == 0) //retx_limit==0 indicates the head data has been sent
       					{
       						pri_attribute.retx_limit = RETX_LIMIT;
       					}
    					
    				}
					else
    				{
    					pri_attribute.retx_limit = RETX_LIMIT;
    				}
				#endif
			}

			if(pri_attribute.fail_in_R==1)
			{
				pri_attribute.fail_in_R = 0;
			}
			
			if(pri_attribute.grade >= 0) /*update the related varialbes, do synchroization*/
			{
				//printf("data num in buffer:%d\n",list_length(rdc_pri_buf_list));
			#if PRIMAC_CONF_COMPOWER_SUPPORT
					//static uint32_t all_time, all_listen, all_transmit, idle;
					all_time = energest_type_time(ENERGEST_TYPE_CPU) + energest_type_time(ENERGEST_TYPE_LPM);
					all_transmit = energest_type_time(ENERGEST_TYPE_TRANSMIT);
	  				all_listen = energest_type_time(ENERGEST_TYPE_LISTEN);
					idle = all_transmit - (pri_compower.all_ctr_transmit + pri_compower.all_data_transmit) +
								all_listen - (pri_compower.all_ctr_listen + pri_compower.all_data_listen);				

					PRT_OUTPUT_POWER(" P %lu %lu %lu %lu\n",// radio %d.%02d%% %d.%02d%% %d.%02d%%\n",// %d.%03d%%\n", //control, data, idle
						/*clock_time(), pri_attribute.grade, linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1],*/
						(pri_compower.all_ctr_transmit + pri_compower.all_ctr_listen), 
						(pri_compower.all_data_transmit + pri_compower.all_data_listen), 
						idle, all_time);
			#endif
				/***** start: orignial is in R state, now move to T state******************/
				pri_attribute.has_receiver = 1;
				pri_attribute.sender_num_in_receivers = -1; //I have found a relay, now change to -1 indicate it's unicast

				PRTJF("broadcast_rts is %u.rcvd_cts %u.sender_num is %d.has_receiver is %u",
					pri_attribute.to_broadcast_rts,
					pri_attribute.rcvd_cts_after_rts,
					pri_attribute.sender_num_in_receivers,
					pri_attribute.has_receiver);
				if(pri_attribute.to_broadcast_rts && pri_attribute.rcvd_cts_after_rts 
					&& pri_attribute.sender_num_in_receivers == -1 && pri_attribute.has_receiver)//list_head(pri_neighbor_parent_list)!=NULL)
				{// will be canged to unicast from broadcast.
					
					pri_attribute.to_broadcast_rts = 0; // 0: to unicast RTS; 1: to broadcast RTS
					PRTJF("to_broadcast_rts is %u.\n",pri_attribute.to_broadcast_rts);
					}
				//it's broadcast and no neighbor after all tries, keep active; or it's unicast, and no neigbhor (all neighbors are deleted due to no CTS reply), keep active
				//printf("pri_attribute.to_broadcast_rts=%u, pri_attribute.has_receiver=%u.\n",pri_attribute.to_broadcast_rts,pri_attribute.has_receiver);
				if(pri_attribute.pri_type == PRI_SENSOR && 
					((pri_attribute.to_broadcast_rts && pri_attribute.sender_num_in_receivers >= MAX_CHILD_NUM*MAX_TRY_TO_FIND_PARENT && !pri_attribute.has_receiver)//list_head(pri_neighbor_parent_list)==NULL)
					||(!pri_attribute.to_broadcast_rts && !pri_attribute.has_receiver))//list_head(pri_neighbor_parent_list)==NULL)))
				)
				{
					if(pri_attribute.to_broadcast_rts && pri_attribute.sender_num_in_receivers >= MAX_CHILD_NUM && !pri_attribute.has_receiver)
				 	{
						 PRT_FORSYNC("Warning: Couldn't find a receiver. keep active. Free the neighbor receiver list.\n");
					}
					if(!pri_attribute.to_broadcast_rts && !pri_attribute.has_receiver)
					{
						 PRT_FORSYNC("Warning: Unicast, but no receiver. keep active. Free the neighbor receiver list.\n");
					}
					PRTJF("keep activate here !\n");
					//pri_attribute.pre_grade = pri_attribute.grade;
					pri_attribute.durslot = DURSLOT;
					pri_attribute.dursleep = DURSLEEP;
					pri_attribute.durcycle = DURCYCLE;
					pri_attribute.grade = -1;
					pri_attribute.temp_grade = -1;
					pri_attribute.state = 'S';
					pri_attribute.forward_grade = 0;
					//pri_attribute.count_no_cts = 0;
					//pri_attribute.random_active_cycles = random_rand()%MAX_ACTIVE_CYCLES; 
					pri_attribute.random_active_cycles = 2;
					pri_attribute.multiple_for_no_cts ++;
					pri_attribute.try_num_to_find_receiver = 0;
					pri_attribute.cycle_count_periodic_bcast = 0;

					pri_attribute.periodic_rts_not_for_data_flag = 0; //need to reset when reset
					pri_attribute.clock_drift_rts_not_for_data_flag = 0; //need to reset when reset
					pri_attribute.cycle_count_no_rts = 0;
	
					pri_attribute.to_broadcast_rts = 1;// 1: broadcast
					pri_attribute.has_receiver = 0;	//no neighbor
					
					pri_attribute.rcv_grade_num_to_setgrade = RCV_GRADE_NUM_TO_SET;
					pri_attribute.sender_num_in_receivers =0;
					
					//linkaddr_copy(&pri_attribute.dest_addr, &linkaddr_null);
					//pri_attribute.next_receiver_neighbor = NULL; 

					pri_attribute.rcvd_cts_after_rts = 0;
				#if DATA_TX_SUPPORT
					pri_attribute.data_cycles_counter = CYCLES_PER_PACKET;
				#endif
					pri_attribute.fail_in_R = 0;
					pri_attribute.drift_per_cycle = 0;//need to reset when reset
				#if RANDOM_DRIFT_IN_COOJA_SUPPORT && IS_COOJA_SIM
					pri_attribute.drift_speed = random_rand()%MAX_DRIFT_PER_CYCLE; //clock drift speed
					pri_attribute.drift_direct = 1;
					if(random_rand()%2==0)
					{
						pri_attribute.drift_direct = -pri_attribute.drift_direct;
					}
					pri_attribute.cycle_counter_for_cd = pri_attribute.drift_speed;
					// printf("clock drift speed: %u direct: %d \n",pri_attribute.drift_speed, pri_attribute.drift_direct);
				#endif
					//pri_attribute.success_cts_to_brorts = 0;
					//below is to free pri_neighbor_parent_list:
					pri_free_list(pri_neighbor_parent_memb,pri_neighbor_parent_list);
					pri_free_list(pri_neighbor_child_memb,pri_neighbor_child_list);

					pri_init_control();
					pri_powercycle_turn_radio_on();
					//printf("it is here\n");
					break; //exit the schedule
					//printf("it is here now\n");
				}

				
				//After continuous M (M = cycle_count_periodic_bcast) cycles, broadcast the grade message again.
				if(pri_attribute.pri_type==PRI_SINK)
				{
    				if(pri_attribute.cycle_count_periodic_bcast > PERIODIC_BCAST_CYCLES_SINK)
    				{
    					pri_attribute.periodic_rts_not_for_data_flag = 1;
    				}
				}else
				{
    				if(pri_attribute.cycle_count_periodic_bcast > PERIODIC_BCAST_CYCLES_SENSOR)
    				{
    					pri_attribute.periodic_rts_not_for_data_flag = 1;
    				}
				}

				//after receiving a message, wait Q (Q = random_active_cycles) cycles to forward grade
				if(pri_attribute.temp_grade > 0) 
				{
					pri_attribute.random_active_cycles --;
					
					if(pri_attribute.random_active_cycles < 0)
					{
						pri_attribute.temp_grade = -1;
						//pri_attribute.random_active_cycles = pri_attribute.multiple_for_no_cts*MAX_ACTIVE_CYCLES + random_rand()%MAX_ACTIVE_CYCLES;
						pri_attribute.random_active_cycles = 2;

						//printf("random_active_cycles = %d.\n",pri_attribute.random_active_cycles);
						PRTJF("here 2 forward_grade = 1\n");
						pri_attribute.forward_grade=1;
					}
					//on(); //keep active
				}else
				{
					pri_attribute.cycle_count_periodic_bcast ++; //wait M cycles, for periodic broadcast
				}

				/***** end: orignial is in R state, now move to T state******************/
				
				/*will do synchroization*/
			#if PRI_SYNC_SUPPORT
				
				//static struct pri_neighbor_parent *r_tb; //receiver table
				static struct pri_neighbor_child *s_tb;
				if(pri_attribute.pri_type == PRI_SENSOR)/*only sensor will do synchronization*/
				{
					if(pri_control.rcvd_cts)//??? pri_control.rcvd_cts : use this as a switch ? only after receiving a CTS
					{
						 
						r_tb = list_head(pri_neighbor_parent_list);
					    /*******************************************************************************************************************/
						if(r_tb!=NULL && (abs(r_tb->clock_drift) >= (int16_t)MAX_CLOCK_DRIFT))// need to adjust clock: METHOD 1: adjust only when the threshold is reached
						/*******************************************************************************************************************/
						//if(r_tb!=NULL) //METHOD 2: adjust every time
						{ 
							
							// update timeoffset
							//if(r_tb->clock_drift_speed>0) //clock_drift_speed is calculated when receiving a cts and updating the receiver table
							if((r_tb->clock_drift - r_tb->cd_base)>0)//positive drift
							{
								/*single side:*/
								time_offset = r_tb->clock_drift;// + MAX_CLOCK_DRIFT; 
								/*double side:*/
								//time_offset = r_tb->clock_drift + (int16_t)MAX_CLOCK_DRIFT; 
								//printf("1.pos.to: %d.\n",time_offset);
								time_offset = find_true_offset(time_offset);
								//printf("2.pos.to: %d.\n",time_offset);
								/*save time_offset into neighbor sender talbe*/
								for(s_tb=list_head(pri_neighbor_child_list); s_tb!=NULL; s_tb=s_tb->next)
								{
									s_tb->time_offset = time_offset;
									//printf("\n0. s_tb timeoffset=%d.\n",s_tb->time_offset);
								}
								 //PRT_FORSYNC("positive cds, synced, notify my each sender by CTS to=%d.\n",time_offset);

								/********************************************************/

								 //PRT_FORSYNC("SYNC: time_offset=%d . cycle_counter: %ld .\n",time_offset,r_tb->cycle_counter);
								
								//update cds, ccth, cd, and cc

								/*******************************************************/
								/*single side:*/
								r_tb->clock_drift -= time_offset;
								/*double side:*/
								//r_tb->clock_drift = - (int16_t)MAX_CLOCK_DRIFT; //0;//update the clock drift of itself
								r_tb->cd_base = r_tb->clock_drift;
								r_tb->cycle_counter = 0;
							}
							else //if((r_tb->clock_drift - r_tb->cd_base)<0)//negative drift 
							{
								/*single side:*/
								time_offset = r_tb->clock_drift;// - MAX_CLOCK_DRIFT;
								/*double side:*/
								//time_offset = r_tb->clock_drift - (int16_t)MAX_CLOCK_DRIFT;

								//printf("1.neg.to: %d.\n",time_offset);
								time_offset = find_true_offset(time_offset);
								//printf("2.neg.to: %d.\n",time_offset);
								/*save time_offset into neighbor sender talbe*/
								
								for(s_tb=list_head(pri_neighbor_child_list); s_tb!=NULL; s_tb=s_tb->next)
								{
									s_tb->time_offset = time_offset;
								}
								// PRT_FORSYNC("negative cds, synced, notify my each sender by CTS to=%d.\n",time_offset);

								/********************************************************/

								 //PRT_FORSYNC("SYNC: time_offset=%d . cycle_counter: %ld .\n",time_offset,r_tb->cycle_counter);
								
								//l->cycle_counter_threshold = 0;

								/*******************************************************/
								/*single side:*/
								r_tb->clock_drift -= time_offset;
								/*double side:*/
								//r_tb->clock_drift = (int16_t)MAX_CLOCK_DRIFT;//0;//
								
								r_tb->cd_base = r_tb->clock_drift;
								r_tb->cycle_counter = 0;
							}

							//time_offset = r_tb->clock_drift;
							//pri_attribute.time_offset = time_offset; //will embed this in CTS to notify my sender (in higher grade node)
							//self_update_clock_drift(r_tb);//update the clock drift of other receivers
							//r_tb->cycle_counter = 0;
							//r_tb->clock_drift = 0; //update the clock drift of itself
						}
					}
					//update_clock_drift_cycle_counter();
					/************************************************************/
					if(!pri_attribute.to_broadcast_rts) /*update only when it's changed to unicast*/
					{
		 				//struct pri_neighbor_parent *l_need_send_rts; /*find the neighbor receiver which really needs synchronization*/
						r_tb = list_head(pri_neighbor_parent_list);
						if(r_tb != NULL)
						{
							r_tb->cycle_counter ++;
							if((r_tb->clock_drift - r_tb->cd_base) != 0)
							{	
								//MAX_CYCLE_COUNTER/abs(l_need_send_rts->clock_drift - l_need_send_rts->cd_base) is actually the definition of the largest counter that leads to one drift
								//PRT_FORSYNC("MAX cycle counter: %ld .\n",(int32_t)(MAX_CYCLE_COUNTER*MAX_CLOCK_DRIFT)/abs(l_need_send_rts->clock_drift - l_need_send_rts->cd_base));
								if(r_tb->cycle_counter <= (int32_t)(MAX_CYCLE_COUNTER*MAX_CLOCK_DRIFT)/abs(r_tb->clock_drift - r_tb->cd_base))
								{
								 	//PRT_FORSYNC("send RTS for clock drift: cc: %ld , possible ccth: %ld .\n",l_need_send_rts->cycle_counter,
								 	//	(MAX_CLOCK_DRIFT*l_need_send_rts->cycle_counter)/abs(l_need_send_rts->clock_drift- l_need_send_rts->cd_base));
									if(r_tb->cycle_counter >= ((int32_t)MAX_CLOCK_DRIFT*r_tb->cycle_counter)/abs(r_tb->clock_drift- r_tb->cd_base))
										//once the counter is larger than the threshold, send rts every this interval (RTS_FOR_SYNC_INTERVAL) until the counter is smaller than the threshold
									{
										pri_attribute.clock_drift_rts_not_for_data_flag = 1; //needs to send rts for resolve the clock drift. Maybe several RTSs are necessary until the clock drift is eliminated
										r_tb->cycle_counter = 0;
										r_tb->cd_base = r_tb->clock_drift;
										//linkaddr_copy(&pri_attribute.dest_addr,&l_need_send_rts->receiver);
									}
								}
							}
						}
					}
				}
			#endif
			#if RANDOM_DRIFT_IN_COOJA_SUPPORT && IS_COOJA_SIM
				pri_attribute.cycle_counter_for_cd -- ;
				if(pri_attribute.cycle_counter_for_cd <= 0)
				{
					pri_attribute.cycle_counter_for_cd = random_rand()%MAX_DRIFT_PER_CYCLE;//pri_attribute.drift_speed;
					pri_attribute.drift_per_cycle = pri_attribute.drift_direct;
				}
			#endif
			}
            if(pri_attribute.pri_type == PRI_SENSOR) //sink has no receiver but only sender
            {
                pri_print_list(pri_neighbor_parent_list,PARENT_LIST);
            }
		}
		/*regular wakeup and in 'R' state*/
		else if (pri_attribute.state == 'S')
		{
			pri_attribute.state = 'R'; 
			//printf("RS %c\n",pri_attribute.state);//Regular State: RS
			pri_attribute.fail_in_R = 0;
			
			pri_powercycle_turn_radio_on();//pri_show_led(); 
			leds_off(LEDS_ALL);
			leds_on(LEDS_RED);

			if(pri_attribute.grade>=0) //&& list_length(rdc_pri_buf_list) < MAX_DATA_QUEUE_BUFS)
			{
				pri_control.is_receiving_data = 1;
			#if 1
				//wait RTS
				pri_control.channel_is_busy = 0;
				now = RTIMER_NOW();
				//t0 = RTIMER_NOW()+WAIT_TO_RCV_RTS; 
				pri_control.wait_rts_timer_over = -1;
				pri_schedule_powercycle_fixed(r_t, now+WAIT_TO_RCV_RTS); //set timer for waiting for RTS
				PT_YIELD(&pt);
				//printf("rts\n");
				pri_control.wait_rts_timer_over = 1; 
			#endif
			
				if(pri_control.rcvd_rts==1 && pri_attribute.rcvd_cts_after_rts==1 && !pri_control.channel_is_busy)
				{
                    if(pri_attribute.pri_type == PRI_SENSOR){
                        pri_powercycle_turn_radio_off();//pri_show_led();
                    }

					pri_attribute.cycle_count_no_rts = 0;
					//printf("clear cycle_count_no_rts\n");
					//printf("rcv bro rts? %u\n",pri_control.rcvd_bro_rts);
					if(pri_attribute.pri_type == PRI_SENSOR)//pri_control.rcvd_bro_rts)//received broadcasted RTS
					{
						now = RTIMER_NOW();
						t0 = now + SIFS + (random_rand() % CW)*SIGMA;	
					}else // received by the sink
					{
						t0=RTIMER_NOW()+WAIT_TO_UCAST_CTS;
					}
				#if 1	
					pri_control.channel_is_busy = 0;
					pri_schedule_powercycle_fixed(r_t, t0);
					pri_control.wait_tosend_timer_over = -1;
					PT_YIELD(&pt);
					pri_control.wait_tosend_timer_over = 1;
						
					//pri_cca();
				#endif
					if(pri_control.channel_is_busy)
					{
                        if(pri_attribute.pri_type == PRI_SENSOR){
                            pri_powercycle_turn_radio_off();//pri_show_led();
                        }
						// printf("Here: Channel is busy!\n");
					}else
					{
						pri_powercycle_turn_radio_on();
						if(send_cts() == RADIO_TX_OK )
						{
							pri_control.sent_cts = 1;
							if(!pri_control.rcvd_rts_not_for_data && list_length(rdc_pri_buf_list) < MAX_DATA_QUEUE_BUFS) //able to wait data
							{
							#if 1
								now = RTIMER_NOW();
								pri_control.wait_data_timer_over = -1;
								pri_schedule_powercycle_fixed(r_t, now+ WAIT_TO_RCV_DATA);
								PT_YIELD(&pt);
								pri_control.wait_data_timer_over = 1;
							#endif

							
								//printf("2. rcv data.\n");
							}
                            if(pri_attribute.pri_type == PRI_SENSOR){
                                pri_powercycle_turn_radio_off();//pri_show_led();
                            }
						    pri_update_child_neighbor();//(&pri_control.sender_addr,&pri_control.receiver_addr,pri_control.rcvd_rts_not_for_data);

						}else
						{
                            if(pri_attribute.pri_type == PRI_SENSOR){
                                pri_powercycle_turn_radio_off();//pri_show_led();
                            }
							random_init(linkaddr_node_addr.u8[0]); //initialize the random seed /*mar.12,2016*/
							PRINTFB("RADIO_TX_COLLISION (reply with cts): Channel is busy!\n");
						}
					}
				
				}
				else
				{
					/*if no rts received more than 3 x PERIODIC_BCAST_CYCLES_*, free sender list.*/
                    if(pri_attribute.pri_type == PRI_SENSOR){
                        pri_powercycle_turn_radio_off();//pri_show_led();
                    }
					//printf("Didn't Receive RTS, go to sleep.\n"); 
				}
				/*Mar. 26, 2016*/
				pri_attribute.fail_in_R = 0;
				if(pri_control.channel_is_busy && pri_attribute.pri_type == PRI_SENSOR)
				{
					pri_attribute.fail_in_R = 1;
				}
				/*update the related varialbes*/
				/***** start: orignial is in R state, now move to T state******************/
				/***** end: orignial is in R state, now move to T state******************/
				
				#if DATA_TX_SUPPORT && GEN_DATA_IN_R
				/*Comment DATA_TX_SUPPORT, because we will use pri_qsend_packet to generate packet*/
				

				if(
					pri_attribute.state == 'R'/*regular wakeup and in 'R' state*/
				){
					//static struct rdc_pri_buf *data_list_elem;
					/*generate data only when it's in unicast*/
					PRTJF("data tx here . state is %c.\t",pri_attribute.state);
					if(pri_attribute.to_broadcast_rts){
						PRTJF("br is true.\t");
					}else{
						PRTJF("br is false.\t");
					}
					if(pri_attribute.total_data_num > 0){
						PRTJF("total > 0\n");
					}else{
						PRTJF("total <= 0\n");
					}
					if(!pri_attribute.to_broadcast_rts && pri_attribute.total_data_num > 0)
					{
						PRTJF("data tx here1.\n");
						pri_attribute.data_cycles_counter --;
						if(pri_attribute.data_cycles_counter == 0)
						{
							//store the packet into queue first
							//space allocation 
							PRTJF("data tx here2.\n");
							buf_elem = memb_alloc(&rdc_pri_buf_memb);
							if(buf_elem != NULL) 
							{			
								static struct data_msg_t data_msg;
								data_msg.gen_or_rcv_time = clock_time();
								data_msg.src_grade = pri_attribute.grade;
								data_msg.delay = 0;
								linkaddr_copy(&data_msg.src_addr,&linkaddr_node_addr);

								pri_attribute.data_gen_num++;
								data_msg.ID = pri_attribute.data_gen_num;
				
					    		buf_elem->next=NULL; 
					    		//memcpy(list_elem->data,&data_msg,sizeof(struct data_msg_t));
					    		buf_elem->data = data_msg;
					    		//list_elem->sent=sent;
					    		//list_elem->ptr = ptr;
					    		pri_attribute.total_data_num--;
					    		list_add(rdc_pri_buf_list, buf_elem); // add the item "list_elem" to the start of the list
								//GDNum: Generated Data Number
							//	PRT_OUTPUT("%lu G %u.%u %d %lu\n",
                              //      clock_time(),
                               //     linkaddr_node_addr.u8[0],linkaddr_node_addr.u8[1],
                                //    pri_attribute.grade,
                                 //   pri_attribute.data_gen_num);
							}else
							{
								//printf("1. Generation drop data, Length:%d\n",list_length(rdc_pri_buf_list));
							}
							//printf("Store one packet into queue.\n");
							pri_attribute.data_cycles_counter = CYCLES_PER_PACKET;
						}
					}
				}
				#endif

			}
			
		}else
		{
			PRINTFB("ERROR: pri_state has a wrong value.\n");
		}

		/*start: below is to set the following schedule*/
		
		//now = RTIMER_NOW();

				t0 = pri_state_period();
				now = RTIMER_NOW();
				// t0 : the remain time
				t0 = t0 - (now - pri_attribute.state_start);// + time_offset + pri_attribute.drift_per_cycle;
				pri_attribute.drift_per_cycle = 0; 

				if(t0 > RTMAXTIMER)
				{
					j_for_large_sf = t0/RTMAXTIMER;
					t0 = t0 - j_for_large_sf * RTMAXTIMER;
					for(;j_for_large_sf>0;j_for_large_sf--)
					{
						pri_schedule_powercycle_fixed(r_t, now + RTMAXTIMER);
						PT_YIELD(&pt);//进程主动让出执行权
						now = RTIMER_NOW();
					}
				}
				//pri_schedule_powercycle_fixed(r_t, t0+time_offset + pri_attribute.drift_per_cycle); pri_attribute.drift_per_cycle = 0;
			//	pri_schedule_powercycle_fixed(r_t, now + t0 + time_offset + pri_attribute.drift_per_cycle); 
				pri_schedule_powercycle_fixed(r_t, now + t0); 

				pri_attribute.drift_per_cycle = 0;
				PT_YIELD(&pt);//进程主动让出执行权
	}
	PT_END(&pt);
}

static uint32_t
pri_state_period()
{
/*
*  \name:
*		pri_state_period
*  \param:
*		no
*  \return:
*		return state period
*  \brief:
*		Obtain the state period according to the node's state, and store the period in a memory
*			pinted by a unit32_t pointer.
*  \description: 
* 		when considering wakeup advance time, 
* 		R state period needs to be increased by WAKEUP_ADVANCE_TIME;
* 		correspondingly, the S state period needs to be decreased by WAKEUP_ADVANCE_TIME.
*/

	if(pri_attribute.state == 'R' )
	{
		return pri_attribute.durslot;
	}else if (pri_attribute.state == 'S')
	{
		return pri_attribute.dursleep;
	}else
	{
		PRINTFA("\nERROR: in pri_state_period: wrong priState.\n");
		return 0;
	}
}
/*---------------------------------------------------------------------------*/
static void print_definition(void)
{
#if 1
	PRT_DEF("\n************************************************\n");
	PRT_DEF("MPDC_SUPPORT  = %u.\n",MPDC_SUPPORT);
	PRT_DEF("2hhSTART_GRADE = %u\n",START_GRADE);


#if MPDC_SUPPORT
	PRT_DEF("NUM_CHANNELS  = %u.\n",NUM_CHANNELS);
//	PRT_DEF("NUM_PATHS	= %u.\n",NUM_PATHS);
	PRT_DEF("MIN_HOPS_BETWEEN_ADJCENT_NODES  = %u.\n",MIN_HOPS_BETWEEN_ADJCENT_NODES);
	PRT_DEF("IS_NOT_STAGGER  = %u.\n",IS_NOT_STAGGER);
#endif
	PRT_DEF("IS_NOT_STAGGER  = %u.\n",IS_NOT_STAGGER);
	



#if ZOLERTIA_Z1 || TMOTE_SKY || WISEMOTE || EXP5438
	PRT_DEF("RTIMER_SECOND = %u. F_CPU=%lu.\n",RTIMER_SECOND,F_CPU);
	PRT_DEF("CLOCK_SECOND=%u.\n",CLOCK_SECOND);
#else		
	PRT_DEF("RTIMER_SECOND = %u. ",RTIMER_SECOND);
	PRT_DEF("F_CPU=%lu.\n",F_CPU);
	PRT_DEF("RTIMER_ARCH_PRESCALER=%lu.\n",RTIMER_ARCH_PRESCALER);
#endif
	float f = ((float)DURSLOT*1.0)/((float)RTIMER_SECOND*1.0);
	PRT_DEF("DURSLOT = %lu.",pri_attribute.durslot);
	PRT_DEF("(%d.%04u s).\n",(int)f,(unsigned)((f-floor(f))*10000));
	PRT_DEF("DURSLEEP = %lu.\n",pri_attribute.dursleep);
	f = ((float)DURCYCLE*1.0)/((float)RTIMER_SECOND*1.0);
	PRT_DEF("DURCYCLE = %lu .",pri_attribute.durcycle);
	PRT_DEF("(%d.%04u s).\n",(int)f,(unsigned)((f-floor(f))*10000));
	f = ((float)MAX_CLOCK_DRIFT*1.0000)/((float)RTIMER_SECOND*1.0000);
	PRT_DEF("MAX_CLOCK_DRIFT = WAKEUP_ADVANCE_TIME = %d (%d.%04u s).\n",(int16_t)WAKEUP_ADVANCE_TIME,(int)f,(unsigned)((f-floor(f))*10000));
	PRT_DEF("PERIODIC_BCAST_CYCLES_SENSOR = %lu.\n",PERIODIC_BCAST_CYCLES_SENSOR);
	PRT_DEF("PERIODIC_BCAST_CYCLES_SINK = %lu.\n",PERIODIC_BCAST_CYCLES_SINK);
	PRT_DEF("MAX_CYCLE_COUNTER = %ld.\n",MAX_CYCLE_COUNTER);
	PRT_DEF("MAX_COUNT_NO_RTS_CTS = %u.\n",MAX_COUNT_NO_RTS_CTS);
	PRT_DEF("MAX_CHILD_NUM = %u.\n",MAX_CHILD_NUM);
	PRT_DEF("MAX_TRY_TO_FIND_PARENT = %u.\n",MAX_TRY_TO_FIND_PARENT);
	//PRT_OUTPUT("DEFAULT_CYCLE_COUNTER_THRESHOLD = %lu.\n",DEFAULT_CYCLE_COUNTER_THRESHOLD);
	    
	PRT_DEF("\n**********Below is Related Settings**********\n");
	PRT_DEF("PRIMAC_CONF_COMPOWER = %u.\n",PRIMAC_CONF_COMPOWER);
	PRT_DEF("PRIMAC_CONF_COMPOWER_SUPPORT = %u.\n",PRIMAC_CONF_COMPOWER_SUPPORT);
	PRT_DEF("SF = %u.\n",SF);
//	PRT_DEF("PRI_SYNC_SUPPORT = %u.\n",PRI_SYNC_SUPPORT);
//	PRT_DEF("RANDOM_DRIFT_IN_COOJA_SUPPORT = %u.\n",RANDOM_DRIFT_IN_COOJA_SUPPORT);
//	PRT_DEF("ADCSC_SUPPORT = %u.\n",ADCSC_SUPPORT);
	PRT_DEF("DATA_TX_SUPPORT = %u.\n",DATA_TX_SUPPORT);

#if DATA_TX_SUPPORT
	PRT_DEF("CYCLES_PER_PACKET = %u.\n",CYCLES_PER_PACKET); 
	PRT_DEF("DATA_INTERVAL = %u.\n",DATA_INTERVAL);
	PRT_DEF("RETX_LIMIT = %u.\n",RETX_LIMIT);
	PRT_DEF("TOTAL_DATA_NUM = %d.\n",TOTAL_DATA_NUM);
	PRT_DEF("MAX_DATA_QUEUE_BUFS = %d.\n",MAX_DATA_QUEUE_BUFS);
#endif	

//	PRT_DEF("PROC_GRADE_BASED_ON_ADDR_SUPPORT = %u.\n",PROC_GRADE_BASED_ON_ADDR_SUPPORT);
//	PRT_DEF("ADDRESS_FREE_SUPPORT = %u.\n",ADDRESS_FREE_SUPPORT);
	PRT_DEF("IS_COOJA_SIM = %u.\n",IS_COOJA_SIM);
//	PRT_DEF("CROSS_PLATFORM_SUPPORT = %u.\n",CROSS_PLATFORM_SUPPORT);
	PRT_DEF("************************************************\n\n");
#endif 
}

/*---------------------------------------------------------------------------*/
static void
pri_init(void) 
{

	pri_attribute.durslot = DURSLOT;
	pri_attribute.dursleep = DURSLEEP;
	pri_attribute.durcycle = DURCYCLE;
	print_definition();
  	//pdcadc_is_on= 1;
	random_init(linkaddr_node_addr.u8[0]); //initialize the random seed

#if ZOLERTIA_Z1
	tmp102_init();//temperature related.
#endif
#if DATA_TX_SUPPORT
	list_init(rdc_pri_buf_list);
	memb_init(&rdc_pri_buf_memb);
	pri_attribute.total_data_num = TOTAL_DATA_NUM;
#endif

	list_init(pri_neighbor_parent_list);
	memb_init(&pri_neighbor_parent_memb);
	
	list_init(pri_neighbor_child_list);
	memb_init(&pri_neighbor_child_memb);
  	pri_attribute.pri_type = PRI_SENSOR;  
	 
	on(); /*turn on the radio*/
	pri_attribute.grade = -1;//need to reset when reset 
	pri_attribute.temp_grade = -1;//need to reset when reset
	pri_attribute.state = 'S';	 //need to reset when reset. //initialize the node state, either 'S' or 'R' .
	 
	//pri_attribute.retx_time=0; 
	//pri_attribute.cycle_count = 0;//INIT_CYCLE_COUNT_FOR_SYNC;
	pri_attribute.random_active_cycles = random_rand()%MAX_ACTIVE_CYCLES; //need to reset when reset
	pri_attribute.random_active_cycles = 2;
	pri_attribute.multiple_for_no_cts = 0;//need to be increased when reset
	//printf("\n\nrandom cycles=%u\n",pri_attribute.random_active_cycles);
	//pri_attribute.count_no_cts = 0;
	pri_attribute.try_num_to_find_receiver = 0;//need to reset when reset
	pri_attribute.cycle_count_periodic_bcast = 0;//need to reset when reset
	pri_attribute.cycle_count_no_rts = 0;//need to reset when reset

	pri_attribute.periodic_rts_not_for_data_flag = 0; //need to reset when reset
	pri_attribute.clock_drift_rts_not_for_data_flag = 0; //need to reset when reset
	//pri_attribute.rcvd_rts_not_for_data = 0; //set to 1: the rts is not for data transmission
	pri_attribute.rcvd_cts_after_rts = 0; //need to reset when reset //indicate whether a node received a CTS after broadcasting a RTS. A node can only reply a CTS if it previously received a CTS
	pri_attribute.forward_grade = 0; //need to reset when reset //when a node determine its grade, it needs to forward a grade message

	pri_attribute.to_broadcast_rts = 1;//need to reset when reset  // 1: broadcast, 0: unicast
	pri_attribute.has_receiver = 0;	//need to reset when reset  //no neighbor

	pri_attribute.rcv_grade_num_to_setgrade = RCV_GRADE_NUM_TO_SET;//need to reset when reset
	pri_attribute.sender_num_in_receivers = 0; //need to reset when reset
#if DATA_TX_SUPPORT
	pri_attribute.data_gen_num=0;
	pri_attribute.data_rcvd_num_at_sink=0;

	pri_attribute.data_cycles_counter = CYCLES_PER_PACKET;
	pri_attribute.retx_limit = RETX_LIMIT; /*retrasmission limit of the current packet*/
#endif

#if MPDC_SUPPORT
	pri_attribute.rssi = -1;
	pri_attribute.next_hop = linkaddr_null;
	pri_attribute.channel_id = -1;
#endif

#if PRIMAC_CONF_COMPOWER_SUPPORT
	pri_compower.all_ctr_listen = 0;
	pri_compower.all_ctr_transmit = 0;
	pri_compower.all_data_listen = 0;
	pri_compower.all_data_transmit = 0;
#endif
	pri_attribute.fail_in_R = 0;
	pri_attribute.drift_per_cycle = 0;//need to reset when reset
#if RANDOM_DRIFT_IN_COOJA_SUPPORT && IS_COOJA_SIM//need to reset when reset
	pri_attribute.drift_speed = random_rand()%MAX_DRIFT_PER_CYCLE; //clock drift speed
	pri_attribute.drift_direct = 1;
	if(random_rand()%2==0)
	{
		pri_attribute.drift_direct = -pri_attribute.drift_direct;
	}
	pri_attribute.cycle_counter_for_cd = pri_attribute.drift_speed;
#endif


	pri_init_control();
	//printf("init cycle_count=%u\n",pri_attribute.cycle_count);
	pri_show_led();//turn on corresonding led
	if(linkaddr_cmp(&linkaddr_node_addr,&linkaddr_sink))
	{/*\brief:		sink node (with mac address 1.0) will broadcast grade message*/
		PRINTFB("I am sink, I will broadcast grade.\n");
		pri_attribute.grade = 0; //sink node is in grade zero
	  	pri_attribute.pri_type = PRI_SINK;
		pri_attribute.forward_grade=1;
		pri_attribute.rcvd_cts_after_rts = 1;

	/*If multiple channels are supported, probe message should be sent before sending grade message */
	#if MPDC_SUPPORT && !IS_NOT_STAGGER
		pri_attribute.channel_id = 0;
		PRTJF("call the send_probe_message :\t");
		ctimer_set(&probe_start_ctimer, START_PROBE, send_probe_message, NULL);
	#else
		ctimer_set(&grade_start_ctimer, START_GRADE, start_grade, NULL);
	#endif
	}
	
#if MPDC_SUPPORT && !IS_NOT_STAGGER
	int j;
	for(j=2;j<=NUM_CHANNELS;j++){//2.0\3.0
		if(linkaddr_node_addr.u8[0] == j){
			PRTJF("i am sink node too\n");
			pri_attribute.pri_type = PRI_SINK;
			pri_attribute.grade = 0;
            pri_attribute.rcvd_cts_after_rts = 1;
			pri_attribute.channel_id = j-1;
			const radio_value_t channel_t = cc2420_get_channel() - pri_attribute.channel_id;
	  		int rd;
	  		rd = NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, channel_t);
	  		printf("channel_t is %d. ret = %u.\t",channel_t,rd);
	 		printf("now radio channel is %u\n",cc2420_get_channel());	
		}
	}
#endif

#if MPDC_SUPPORT && IS_NOT_STAGGER

	printf("IS_NOT_STAGGER is here\n");
	if(NUM_CHANNELS != 1){
		int j;
		if(linkaddr_node_addr.u8[0] > NUM_CHANNELS){
			int tmp_grade = (linkaddr_node_addr.u8[0]-NUM_CHANNELS)/3;
			pri_attribute.channel_id = (linkaddr_node_addr.u8[0] - (tmp_grade-1)*3 - NUM_CHANNELS)%NUM_CHANNELS;
			const radio_value_t channel_t = cc2420_get_channel() - pri_attribute.channel_id;
			int rd;
			rd = NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, channel_t);
			printf("channel_t is %d. ret = %u.\t",channel_t,rd);
			printf("now radio channel is %u\n",cc2420_get_channel());	
		}
		for(j=2;j<=NUM_CHANNELS;j++){//2.0\3.0
			if(linkaddr_node_addr.u8[0] == j){
				PRINTFB("i am sink node too\n");
				pri_attribute.pri_type = PRI_SINK;
				pri_attribute.grade = 0;
				pri_attribute.rcvd_cts_after_rts = 1;
				pri_attribute.channel_id = j-1;
				pri_attribute.forward_grade=1;
				const radio_value_t channel_t = cc2420_get_channel() - pri_attribute.channel_id;
				int rd;
				rd = NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, channel_t);
				printf("channel_t is %d. ret = %u.\t",channel_t,rd);
				printf("now radio channel is %u\n",cc2420_get_channel());	
				ctimer_set(&grade_start_ctimer, START_GRADE, start_grade, NULL);
			}
		}

	}
#endif
	

	printf("My address is %u.%u.\n",linkaddr_node_addr.u8[0],linkaddr_node_addr.u8[1]);

}

/*---------------------------------------------------------------------------*/
static void
init(void)
{
	radio_is_on = 0;
	PT_INIT(&pt);
	pdcadc_is_on= 1;
	pri_init();
}

/*---------------------------------------------------------------------------*/
/*
*  GRADE-MESSAGE-PROCESSING algorithm
*/
static uint32_t pri_grade_proc(char node_state, uint32_t sender_state_dur)
{
	uint32_t state_dur = 0;
	uint32_t t;
	
	t = sender_state_dur - (sender_state_dur/pri_attribute.durcycle)*pri_attribute.durcycle;
	PRTJF("\n");
	PRTJF("sender_state_dur is %u. \t",sender_state_dur);
	PRTJF("t is %u \t",t);
	if(node_state == 'R')
	{	
		if(t < pri_attribute.dursleep)
		{
			pri_attribute.state = 'S';
			state_dur = t;
			//state_period = DURSLEEP;
		}else
		{
			pri_attribute.state = 'R';
			state_dur = t - pri_attribute.dursleep;
			//state_period = DURSLOT;
		}
	}else if(node_state == 'S')//real S state not T state
	{
		if(t < (pri_attribute.dursleep-pri_attribute.durslot))
		{
			pri_attribute.state = 'S';
			state_dur = t + pri_attribute.durslot; 
			//state_period = DURSLEEP;
		}else if(t < (pri_attribute.dursleep))
		{
			pri_attribute.state = 'R';
			state_dur = t - (pri_attribute.dursleep-2*pri_attribute.durslot); 
			//state_period = DURSLOT;
		}else 
		{
			pri_attribute.state = 'S';
			state_dur = t - pri_attribute.dursleep;
			//state_period = DURSLEEP;
		}
	}else
	{
		PRINTFA("\nERROR: Unknown node state: %c!\n",node_state);
	}
	return state_dur;
}

#if MPDC_SUPPORT
static void
pri_gdps_from_sink(uint32_t receiving_pkt_time, int8_t msg_grade, char msg_state, uint16_t msg_state_dur,linkaddr_t source_addr_ptr,struct grade_msg_t  grade_msg){
		if(pdcadc_is_on == 0)
		{
			return;
		}
		pri_attribute.temp_grade = msg_grade + 1;
		pri_attribute.grade = msg_grade + 1;
		pri_attribute.channel_id = pri_attribute.node_id%NUM_CHANNELS;
		/*
		*	one hop nodes should change their receive node by their channel id.
		*	Because each channel has a corresponding unique sink node
		*/
		PRTJF("order is %u\n",grade_msg.order);
		linkaddr_t addr_temp;
		addr_temp.u8[0] = pri_attribute.channel_id + 1;
		addr_temp.u8[1] = 0;	
		pri_attribute.next_hop = addr_temp;
		/*************************************************/ 
		uint32_t sender_state_dur, state_dur, state_period;
		sender_state_dur = msg_state_dur+PRI_TM;
		rtimer_clock_t now;
		
		//calculate the current state duration according to the GRADE-MESSAGE-PROCESSING algorithm
		state_dur = pri_grade_proc(msg_state,sender_state_dur);
		
		pri_show_led();
		now = RTIMER_NOW();
		state_period = pri_state_period() - (state_dur + (now - receiving_pkt_time));

		//watchdog_stop();
		if(state_period > RTMAXTIMER)
		{
			state_left_second_round=state_period/RTMAXTIMER;
			rtimer_set(&rt, now  + state_period - RTMAXTIMER*state_left_second_round, 1, 
				(void (*)(struct rtimer *, void *))pri_powercycle, &state_left_second_round);
		}else{ 
			rtimer_set(&rt, now + state_period, 1,	
				(void (*)(struct rtimer *, void *))pri_powercycle, NULL); 
		}

}
#endif
/*---------------------------------------------------------------------------*/

#if MPDC_SUPPORT
static void 
pri_gdps_mc(uint32_t receiving_pkt_time, int8_t msg_grade, 
				char msg_state, uint16_t msg_state_dur,struct grade_msg_t  grade_msg)//grade division and pipeline scheduling
{

	PRTJF("DUR:msg_state_dur is %u .from %u. now time is %u\n",msg_state_dur,grade_msg.source.u8[0],RTIMER_NOW());
	packetbuf_attr_t rssi = packetbuf_attr(PACKETBUF_ATTR_RSSI);
	if(pdcadc_is_on == 0)
	{
		return;
	}//至少接收rcv_grade_num_to_setgrade次来确定自己的等级
	if(pri_attribute.rcv_grade_num_to_setgrade > 0 )
	{
		pri_attribute.rcv_grade_num_to_setgrade -- ;
		if(pri_attribute.temp_grade < 0 || pri_attribute.temp_grade > msg_grade + 1
		|| (pri_attribute.temp_grade == msg_grade + 1 && rssi > pri_attribute.rssi)
		)
		{			
			pri_attribute.temp_grade = msg_grade + 1;
			pri_attribute.temp_next_hop = grade_msg.source;
			pri_attribute.temp_msg_state_dur = msg_state_dur;
			pri_attribute.temp_now =  RTIMER_NOW();
			pri_attribute.temp_channel = grade_msg.channel_id;
			pri_attribute.rssi = rssi;
		}
		return;
	}else if(pri_attribute.temp_grade != msg_grade + 1)
	{
		if(pri_attribute.temp_grade < 0 || pri_attribute.temp_grade > msg_grade + 1)
		{
			pri_attribute.temp_grade = msg_grade + 1;
			pri_attribute.temp_next_hop = grade_msg.source;
			pri_attribute.temp_msg_state_dur = msg_state_dur;
			pri_attribute.temp_now =  RTIMER_NOW();
			pri_attribute.temp_channel = grade_msg.channel_id;
			pri_attribute.rssi = rssi;
		}else
		{
			return;
		}
	}else if(pri_attribute.temp_grade == msg_grade + 1){
			if(rssi > pri_attribute.rssi){
				pri_attribute.temp_next_hop = grade_msg.source;
				pri_attribute.temp_msg_state_dur = msg_state_dur;
				pri_attribute.temp_now =  RTIMER_NOW();
				pri_attribute.temp_channel = grade_msg.channel_id;
				pri_attribute.rssi = rssi;
			}
	}
	pri_attribute.rcv_grade_num_to_setgrade = RCV_GRADE_NUM_TO_SET;
	pri_attribute.grade = pri_attribute.temp_grade;//finally determine the grade, and will forward a grade message
	pri_attribute.channel_id = pri_attribute.temp_channel;
	pri_attribute.next_hop = pri_attribute.temp_next_hop;
	PRTJF("channel id %u \t next hop is %u\n",pri_attribute.channel_id,pri_attribute.next_hop);
	
	
	/*************************************************/	
	uint32_t sender_state_dur, state_dur, state_period;
	sender_state_dur = pri_attribute.temp_msg_state_dur + PRI_TM;

	rtimer_clock_t now2 = RTIMER_NOW();
	sender_state_dur += (now2- pri_attribute.temp_now);
	

	
	rtimer_clock_t now;
	//calculate the current state duration according to the GRADE-MESSAGE-PROCESSING algorithm
	state_dur = pri_grade_proc(msg_state,sender_state_dur);
  
	pri_show_led();
	now = RTIMER_NOW();
	state_period = pri_state_period() - (state_dur + (now - receiving_pkt_time));
	
	//watchdog_stop();
	if(state_period > RTMAXTIMER)
	{
		state_left_second_round=state_period/RTMAXTIMER;
		rtimer_set(&rt, now  + state_period - RTMAXTIMER*state_left_second_round, 1, 
			(void (*)(struct rtimer *, void *))pri_powercycle, &state_left_second_round);
	}else{ 
		rtimer_set(&rt, now + state_period, 1,  
			(void (*)(struct rtimer *, void *))pri_powercycle, NULL); 
	}
	/*************************************************/	
}
#endif

static void 
pri_gdps(uint32_t receiving_pkt_time, int8_t msg_grade, char msg_state, uint16_t msg_state_dur)//grade division and pipeline scheduling
{
	if(pdcadc_is_on == 0)
	{
		return;
	}
	if(pri_attribute.rcv_grade_num_to_setgrade > 0 )
	{
		pri_attribute.rcv_grade_num_to_setgrade -- ;
		if(pri_attribute.temp_grade < 0 || pri_attribute.temp_grade > msg_grade + 1)
		{			
			pri_attribute.temp_grade = msg_grade + 1;
		}
		return;
	}else if(pri_attribute.temp_grade != msg_grade + 1)
	{
		if(pri_attribute.temp_grade < 0 || pri_attribute.temp_grade > msg_grade + 1)
		{
			pri_attribute.temp_grade = msg_grade + 1;
		}else
		{
			return;
		}
	}
	pri_attribute.rcv_grade_num_to_setgrade = RCV_GRADE_NUM_TO_SET;
	pri_attribute.grade = pri_attribute.temp_grade;//finally determine the grade, and will forward a grade message

	/*************************************************/	
	uint32_t sender_state_dur, state_dur, state_period;
	sender_state_dur = msg_state_dur+PRI_TM;
	rtimer_clock_t now;
	
	//calculate the current state duration according to the GRADE-MESSAGE-PROCESSING algorithm
	state_dur = pri_grade_proc(msg_state,sender_state_dur);
	
	pri_show_led();
	now = RTIMER_NOW();
	state_period = pri_state_period() - (state_dur + (now - receiving_pkt_time));
	//watchdog_stop();
	if(state_period > RTMAXTIMER)
	{
		state_left_second_round=state_period/RTMAXTIMER;
		rtimer_set(&rt, now  + state_period - RTMAXTIMER*state_left_second_round, 1, 
			(void (*)(struct rtimer *, void *))pri_powercycle, &state_left_second_round);
	}else{ 
		rtimer_set(&rt, now + state_period, 1,  
			(void (*)(struct rtimer *, void *))pri_powercycle, NULL); 
	}
	
}

/*---------------------------------------------------------------------------*/
static void 
pri_input_packet(void)
{
	rtimer_clock_t receiving_pkt_time = RTIMER_NOW();
	
	pri_control.node_state_dur= receiving_pkt_time - pri_attribute.state_start;
	//printf("node_state_dur:%u\n",pri_control.node_state_dur);

	rtimer_clock_t wt;
	uint16_t	state_dur;
	int16_t		offset_or_drift;
//	PRTJF("called me!\n");
	struct pri_hdr_t *hdr_ptr; /*define header pointer*/
	//printf("0. Receive a packet.\n");
	if(NETSTACK_FRAMER.parse() >= 0)
	{		
		
		hdr_ptr = packetbuf_dataptr();
		PRTJF("1. Receive a packet.and type is %u\n",hdr_ptr->type);
		
		if(hdr_ptr->dispatch !=DISPATCH)
		{
			if(hdr_ptr->type == TYPE_DATA)
			{
				
				pri_compower_func(TYPE_DATA);
				
				packetbuf_hdrreduce(hdr_len);/*reduce the header part of the received packet*/

				
				if(pri_control.is_sending_data)//sender in T state
				{
					if(!pri_control.sent_rts||(pri_control.sent_rts && !pri_control.rcvd_cts))//before sending RTS, or have sent RTS but not rcvd CTS
					{
                        if(pri_attribute.pri_type == PRI_SENSOR){
                            pri_powercycle_turn_radio_off();//pri_show_led();
                        }
						pri_control.channel_is_busy = 1;
						if(pri_control.wait_tosend_timer_over == -1 || pri_control.wait_cts_timer_over == -1)//have set timer, and the timer is not over
						{
							rtimer_run_next();
						}
					}
				}else if(pri_control.is_receiving_data || pri_attribute.pri_type == PRI_SINK)// receiver in R state
				{
					if(pri_attribute.pri_type == PRI_SENSOR &&(!pri_control.rcvd_rts || (pri_control.rcvd_rts && !pri_control.sent_cts)))//before receiving RTS, or have rcvd RTS but not replied CTS
					{
                        if(pri_attribute.pri_type == PRI_SENSOR){
                            pri_powercycle_turn_radio_off();//pri_show_led();
                        }
						pri_control.channel_is_busy = 1;
						if(pri_control.wait_tosend_timer_over == -1 || pri_control.wait_rts_timer_over == -1)//have set timer, and the timer is not over
						{
							rtimer_run_next();
						}
					}else if(linkaddr_cmp(packetbuf_addr(PACKETBUF_ADDR_RECEIVER), &linkaddr_node_addr)) //the DATA is for me
					{
	    				//pri_control.rcvd_data = 1;
	    				if(pri_attribute.pri_type == PRI_SENSOR){
                            pri_powercycle_turn_radio_off();//pri_show_led();
                        }
						
					#if DATA_TX_SUPPORT
	                	struct data_msg_t data_msg;
						//packet = queuebuf_new_from_packetbuf();
						memcpy(&data_msg, packetbuf_dataptr(), sizeof(struct data_msg_t));
						PRTJF("i am receiving data from %u\n",packetbuf_addr(PACKETBUF_ADDR_SENDER)->u8[0]);

						if(pri_attribute.pri_type == PRI_SINK)
						{
							
							pri_attribute.data_rcvd_num_at_sink++;
							//Receive data at the sink node, clock_time, src address, src grade, latency, total num of pkts rcved by sink
								
							if(data_msg.has_temp==1)
							{
								//PRT_OUTPUT("%lu R %u.%u RPID: %lu Grade: %d Latency: %ld Num: 1 TotalRcvdNum: %lu\n", 
								PRT_OUTPUT("%lu R %u.%u %lu %d %ld %lu\n",
								clock_time(),
								data_msg.src_addr.u8[0],data_msg.src_addr.u8[1],
								data_msg.ID,
								data_msg.src_grade,
								data_msg.delay,
								pri_attribute.data_rcvd_num_at_sink);
							}else
							{
								//PRT_OUTPUT("%lu R %u.%u RPID: %lu Grade: %d Latency: %ld Num: 1 TotalRcvdNum: %lu\n", 
								PRT_OUTPUT("%lu R %u.%u %lu %d %ld %lu\n", 
								clock_time(), data_msg.src_addr.u8[0],data_msg.src_addr.u8[1],data_msg.ID,data_msg.src_grade,data_msg.delay,pri_attribute.data_rcvd_num_at_sink);
								
							}
							if(pri_control.wait_data_timer_over == -1)//have set timer and the timer is not over
	  						{
	   							rtimer_run_next();
	   						}
						}else 
						{
							struct rdc_pri_buf *list_elem;
		            		list_elem = memb_alloc(&rdc_pri_buf_memb); 
							if(list_elem == NULL)
							{
								// printf("2. input drop data, Length:%d\n",list_length(rdc_pri_buf_list));
								//return; /*mar.19,2016*/
							}else
							{  
								data_msg.gen_or_rcv_time = clock_time();
			            		list_elem->next = NULL; 
			            		list_elem->data = data_msg; 
			            		list_add(rdc_pri_buf_list, list_elem); // add the item "list_elem" to the start of the list
								if(pri_control.wait_data_timer_over == -1)//have set timer and the timer is not over
			  					{ 
			   						rtimer_run_next();
			   					}
			            		//printf("\ninput_packet:Received a packet into buffer with length:%d.\n\n", list_length(rdc_pri_buf_list));
							}
						}
	            	#endif
					
	               	}
				}				
				else				
    			{
    				PRINTFB("rcv DATA: receive DATA but not in \'R\' or \'T\' state: 3.\n");
    				//PRINTFA("\nERROR: Data is not for this node.node addres:%u.%u\n", rcv_addr.u8[0],rcv_addr.u8[1]);
    			}
			}
		}
/* 
control messages: grade/rts/cts/ack
*/  
		else /*hdr_ptr->dispatch==DISPATCH*/
		{
		
		#if MPDC_SUPPORT
			if(hdr_ptr->type == TYPE_GRADE_SINK)
			{
				pri_compower_func(TYPE_GRADE_SINK);
				struct grade_msg_t grade_msg;	
				//packetbuf_set_datalen(sizeof(struct grade_msg_t)); 
				packetbuf_hdrreduce(hdr_len);
				memcpy(&grade_msg, packetbuf_dataptr(),  sizeof(struct grade_msg_t));
				if(pri_attribute.grade<0 )
				{
					state_dur = grade_msg.state_dur;

					uint8_t ord = grade_msg.order;
					if((ord-1)*NUM_CHANNELS + 1 <= pri_attribute.node_id
						&& pri_attribute.node_id <= ord*NUM_CHANNELS){
						PRTJF("state_dur is %u\n",state_dur);
						PRTJF("order is :%u .and node id is %u\n",grade_msg.order,pri_attribute.node_id);
						linkaddr_t linkaddr_t;
						linkaddr_copy(&linkaddr_t,&grade_msg.source);
						pri_gdps_from_sink(receiving_pkt_time, grade_msg.grade, 'S', state_dur,linkaddr_t,grade_msg);
					}
					
					
				}else if(pri_attribute.grade == grade_msg.grade)
				{

				}
			}else 
		#endif
			if(hdr_ptr->type == TYPE_GRADE){
				pri_compower_func(TYPE_GRADE);
				struct grade_msg_t grade_msg;	
				//packetbuf_set_datalen(sizeof(struct grade_msg_t)); 
				packetbuf_hdrreduce(hdr_len);
				memcpy(&grade_msg, packetbuf_dataptr(),  sizeof(struct grade_msg_t));
				
				if(pri_attribute.grade<0)
				{
				//	printf("tf rcvd a grade message.\n");
					state_dur = grade_msg.state_dur;

					
				#if MPDC_SUPPORT && !IS_NOT_STAGGER
					PRTJF("mpdc pri_gdps_mc is called!\n");
					pri_gdps_mc(receiving_pkt_time, grade_msg.grade, 'S', state_dur,grade_msg);
				#else
					PRTJF("mpdc pri_gdps is called\n");
					pri_gdps(receiving_pkt_time, grade_msg.grade, 'S', state_dur);
				#endif
				
				}else if(pri_attribute.grade == grade_msg.grade)
				{

				}				
			}
			else if (hdr_ptr->type == TYPE_RTS)
			{
			//	PRTJF("rcvd rts .receiver is %u.aaand from %u.\t",packetbuf_addr(PACKETBUF_ADDR_RECEIVER)->u8[0],packetbuf_addr(PACKETBUF_ADDR_SENDER)->u8[0]);
				pri_compower_func(TYPE_RTS);
				struct rts_msg_t rts_msg;
				//packetbuf_set_datalen(sizeof(struct rts_msg_t)); 
				packetbuf_hdrreduce(hdr_len);/*before using this fuction, should set "buflen", since it was 0 after parsing the pkt*/
				memcpy(&rts_msg, packetbuf_dataptr(),  sizeof(struct rts_msg_t));

				//printf("rcvd rts state_dur: %u, arch second: %lu, dur_slot: %lu.\n", rts_msg.state_dur, rts_msg.arch_second, rts_msg.durslot);

			#if MPDC_SUPPORT && !IS_NOT_STAGGER
				if(pri_attribute.pri_type == PRI_SINK){
					linkaddr_copy(&pri_control.sender_addr,packetbuf_addr(PACKETBUF_ADDR_SENDER));//rts_msg.src_addr;
					linkaddr_copy(&pri_control.receiver_addr,packetbuf_addr(PACKETBUF_ADDR_RECEIVER));//rts_msg.dest_addr;
					pri_control.rcvd_rts = 1;
					if(send_cts() == RADIO_TX_OK){
						PRTJF("sink node send cts successful! \n");
					}
					return;
				}
			#endif	

				if(pri_attribute.grade<0)
				{
					state_dur = rts_msg.state_dur;
					pri_gdps(receiving_pkt_time, rts_msg.grade, 'S', state_dur);
					/*--------------------------------------------------------------------------*/
				}else if(pri_control.is_receiving_data || pri_attribute.pri_type == PRI_SINK) //in 'R' state
				{
					if(!pri_control.rcvd_rts && rts_msg.grade == pri_attribute.grade+1 && pri_attribute.rcvd_cts_after_rts)//received a RTS in 'R' state from the right grade
					{
						if(pri_attribute.pri_type==PRI_SINK  //it's sink node, or
							//for broadcast:
							|| linkaddr_cmp(packetbuf_addr(PACKETBUF_ADDR_RECEIVER),&linkaddr_null)
							//for unicast:
							||linkaddr_cmp(packetbuf_addr(PACKETBUF_ADDR_RECEIVER),&linkaddr_node_addr)
							)
						{
							linkaddr_copy(&pri_control.sender_addr,packetbuf_addr(PACKETBUF_ADDR_SENDER));//rts_msg.src_addr;
							linkaddr_copy(&pri_control.receiver_addr,packetbuf_addr(PACKETBUF_ADDR_RECEIVER));//rts_msg.dest_addr;
							
							pri_control.rcvd_rts = 1;
							if(linkaddr_cmp(&pri_control.receiver_addr,&linkaddr_null))// a broadcasted RTS
							{
					    		//printf("received a broadcasted RTS: success_cts_to_brorts=%u.\n",list_elem->success_cts_to_brorts);
					    		// printf("rcv bro rts\n");
					    		pri_control.rcvd_bro_rts = 1;
							}

							offset_or_drift = rts_msg.clock_drift;
							
							pri_control.clock_drift = offset_or_drift;//rts_msg.clock_drift; //this clock_drift is obtained in R state, and will be stored in sender table
							if(rts_msg.not_for_data)
							{
								pri_control.rcvd_rts_not_for_data = 1;
							}
							#if 1
							if(pri_control.wait_rts_timer_over == -1) //have set the timer and the timer has not been over
							{
								rtimer_run_next();
							}
							#endif 
							
						}else // the received rts is not for me
						{
							pri_control.channel_is_busy = 1;/*mar.19,2016*/
                            if(pri_attribute.pri_type == PRI_SENSOR){
                                pri_powercycle_turn_radio_off();//pri_show_led();
                            }
							if(pri_control.wait_rts_timer_over == -1) //have set the timer and the timer has not been over
							{
								rtimer_run_next();
							}
						} 
						
					}
					else if (!pri_control.rcvd_rts)// the received rts is not for me
					{
						pri_control.channel_is_busy = 1;/*mar.19,2016*/
                        if(pri_attribute.pri_type == PRI_SENSOR){
                            pri_powercycle_turn_radio_off();//pri_show_led();
                        }
						if(pri_control.wait_rts_timer_over == -1) //have set the timer and the timer has not been over
						{
							rtimer_run_next();
						}
					}
					
				}
				else if(pri_control.is_sending_data) //in 'T' state
				{
					if(!pri_control.sent_rts )//before I send RTS 
					{
						pri_powercycle_turn_radio_off();
						pri_control.channel_is_busy=1;
						if(pri_control.wait_tosend_timer_over == -1)// || pri_control.wait_cts_timer_over == -1)
						{
							pri_attribute.periodic_rts_not_for_data_flag=1;
							rtimer_run_next();
						}
					}
									
				}else
				{
					 PRINTFB("rcv rts: receive RTS but not in \'R\' or \'T\' state: 3.\n");
				}


			}
			else if(hdr_ptr->type == TYPE_CTS)
			{
	//			PRTJF("receive a cts packet .\t");
				pri_compower_func(TYPE_CTS); 
				struct cts_msg_t cts_msg;	
				//packetbuf_set_datalen(sizeof(struct cts_msg_t)); 
				packetbuf_hdrreduce(hdr_len);
				memcpy(&cts_msg, packetbuf_dataptr(),  sizeof(struct cts_msg_t));
				state_dur = cts_msg.state_dur;
				pri_control.msg_state_dur = state_dur+PRI_TM;
				if(pri_attribute.grade<0)
				{
					PRINTFA("Receive an CTS message. The message is from: %u, grade: %u, sender_state_dur:%u.\n",
					pri_control.sender_addr,cts_msg.grade,state_dur);
					pri_gdps(receiving_pkt_time, cts_msg.grade, 'R', state_dur);
					
					/*--------------------------------------------------------------------------*/
				}else if(pri_control.is_sending_data) //received a CTS in 'T' state
				{
					if(!pri_control.sent_rts) //before sending RTS
					{
						pri_control.channel_is_busy = 1;/*mar.19,2016*/
                        if(pri_attribute.pri_type == PRI_SENSOR){
                            pri_powercycle_turn_radio_off();//pri_show_led();
                        }
						if(pri_control.wait_tosend_timer_over == -1)
						{
							rtimer_run_next();
						}
					}else if(!pri_control.rcvd_cts)//after sending an RTS before receiving CTS
					{
						//linkaddr_copy(&pri_control.receiver_addr, &cts_msg.dest_addr);
						//linkaddr_copy(&pri_receiver_addr,packetbuf_addr(PACKETBUF_ADDR_RECEIVER));//rts_msg.dest_addr;
						PRTJF("receiver is %u.and from %u\n",packetbuf_addr(PACKETBUF_ADDR_RECEIVER)->u8[0],packetbuf_addr(PACKETBUF_ADDR_SENDER)->u8[0]);
						if(linkaddr_cmp(packetbuf_addr(PACKETBUF_ADDR_RECEIVER), &linkaddr_node_addr)) //received a CTS in 'T' state from a right node
	   					{
	   						pri_control.clock_drift = pri_control.node_state_dur - pri_control.msg_state_dur; //this clock drift is obtained in T state, and will be stored in receiver table
							 //printf("\nCTS_Time: %lu Error %d\n\n", clock_time(),pri_control.clock_drift);	
	   						pri_control.rcvd_cts = 1;
							pri_control.queue_is_full = cts_msg.queue_is_full;
	   						//pri_attribute.count_no_cts = 0;
	   						pri_attribute.multiple_for_no_cts = 0;
	   						
	   						pri_attribute.rcvd_cts_after_rts = 1;
							//PRTJF("rcvd_cts_after_rts is %u",pri_attribute.rcvd_cts_after_rts);
							linkaddr_copy(&pri_control.receiver_addr, packetbuf_addr(PACKETBUF_ADDR_RECEIVER));
	   						linkaddr_copy(&pri_control.sender_addr, packetbuf_addr(PACKETBUF_ADDR_SENDER));//cts_msg.src_addr;


							offset_or_drift = cts_msg.time_offset;

							pri_update_parent_neighbor(offset_or_drift, cts_msg.sender_num); //here, we will reset count_no_CTS
							//pri_update_parent_neighbor(TRANSFER_RTIMER_SECOND(cts_msg.time_offset, cts_msg.local_rtimer_second), cts_msg.sender_num); //here, we will reset count_no_CTS

							#if 1
	    					if(pri_control.wait_cts_timer_over == -1)//have set timer, and the timer is not over
	    					{
	    						rtimer_run_next();
	    					}
							#endif
	    				}
						
						else //the rcvd CTS is not for me
	    				{
                            if(pri_attribute.pri_type == PRI_SENSOR){
                                pri_powercycle_turn_radio_off();//pri_show_led();
                            }
							pri_control.channel_is_busy = 1;
							if(pri_control.wait_cts_timer_over == -1)//have set timer, and the timer is not over
							{
								rtimer_run_next();
							}
	    				}
						
					}					
				}
				else if(pri_control.is_receiving_data)//received a CTS in 'R' state
				{
					if(!pri_control.rcvd_rts || (pri_control.rcvd_rts && !pri_control.sent_cts)) // is waiting RTS or have rcvd RTS but not sent CTS
					{
                        if(pri_attribute.pri_type == PRI_SENSOR){
                            pri_powercycle_turn_radio_off();//pri_show_led();
                        }
						pri_control.channel_is_busy = 1;
						if(pri_control.wait_rts_timer_over == -1)//have set timer, and the timer is not over
						{
							rtimer_run_next();
						}
					}
				}else
				{
					 PRINTFB("rcv cts: unexptected. 3");
				}

				
			}

		#if MPDC_SUPPORT && !IS_NOT_STAGGER
			else if(hdr_ptr->type == TYPE_PROBE){
				pri_compower_func(TYPE_PROBE); 
				//only sensor node will recept this probe message, other sink nodes will ignore it
				if(pri_attribute.pri_type == PRI_SENSOR){
					linkaddr_t linkaddr_t;
					random_init(linkaddr_node_addr.u8[0]);//wait a random time to send ack of probe message
					ctimer_set(&probe_start_ctimer, random_rand() % CW, send_probe_message_ack, NULL);
				}
			}
			else if(hdr_ptr->type == TYPE_PROBE_R){
				pri_compower_func(TYPE_PROBE_R); 
				if(linkaddr_cmp(packetbuf_addr(PACKETBUF_ADDR_RECEIVER), &linkaddr_node_addr)){
					linkaddr_copy(&pri_control.sender_addr,packetbuf_addr(PACKETBUF_ADDR_SENDER));
					//接收到数据，重置定时器
					ctimer_restart(&grade_start_ctimer);
					send_nodeid_message();// send the order messgae
					num_paths = pri_attribute.cnt;
					printf("ctimer here cnt is %u,now time is %lu\n",
						num_paths
						,RTIMER_NOW());
					if(ctimer_expired(&grade_start_ctimer)){
						printf("ctimer is expired!\n");
					}else{
						printf("ctimer 222 is not expired!\n");
					}

				}
			}
			else if(hdr_ptr->type == TYPE_NODEID){
				pri_compower_func(TYPE_NODEID); 
				if(linkaddr_cmp(packetbuf_addr(PACKETBUF_ADDR_RECEIVER), &linkaddr_node_addr)){
					//PRTJF("has receive my order!\n");
					struct nodeid_msg_t nodeid_msg;	
					//packetbuf_set_datalen(sizeof(struct cts_msg_t)); 
					packetbuf_hdrreduce(hdr_len);
					memcpy(&nodeid_msg, packetbuf_dataptr(),  sizeof(struct nodeid_msg_t));
					pri_attribute.node_id = nodeid_msg.node_id;
					PRTJF("has receive my order:%u!\n",pri_attribute.node_id);
				}
			}
		#endif
			else
			{
				PRINTFA("pdcadc: Cannot determine the type of the received packet.\n");
			}
		}
	}else{
		PRINTFA("pdcadc: failed to parse (%u)\n", packetbuf_totlen());
	}
}


/*---------------------------------------------------------------------------*/
static void
on(void)
{
  if(pdcadc_is_on && radio_is_on == 0) {
    radio_is_on = 1;
    NETSTACK_RADIO.on();
  }
}
/*---------------------------------------------------------------------------*/
static void
off(void)
{
  if(pdcadc_is_on && radio_is_on != 0 && pdcadc_keep_radio_on == 0) {
    radio_is_on = 0;
    NETSTACK_RADIO.off();
  }
}


/*---------------------------------------------------------------------------*/
static int
turn_on(void)
{
  if(pdcadc_is_on == 0) {
    pdcadc_is_on = 1;
    pdcadc_keep_radio_on = 0;
	if(linkaddr_node_addr.u8[0] == 1){
		pdcadc_keep_radio_on = 1;
	}
	pri_init();
  }
  return 1;
}
/*---------------------------------------------------------------------------*/
static int
turn_off(int keep_radio_on)
{
  pdcadc_is_on = 0;
  pdcadc_keep_radio_on = keep_radio_on;
  pri_free_list(pri_neighbor_parent_memb,pri_neighbor_parent_list);
  pri_free_list(pri_neighbor_child_memb,pri_neighbor_child_list);
  pri_free_list(rdc_pri_buf_memb,rdc_pri_buf_list);
  //printf("4. in turn_off, drop all data , length: %d\n",list_length(rdc_pri_buf_list));
  if(keep_radio_on) {
    radio_is_on = 1;
    return NETSTACK_RADIO.on();
  } else {
    radio_is_on = 0;
    return NETSTACK_RADIO.off();
  }
}
/*---------------------------------------------------------------------------*/    
static unsigned short
duty_cycle(void) 
{
  return (1ul * CLOCK_SECOND * CYCLE_TIME) / RTIMER_SECOND;
}
/*---------------------------------------------------------------------------*/
const struct rdc_driver mpdcmac_driver = {
  "MPDCMAC",
  init,
  pri_qsend_packet,
  pri_qsend_list,
  pri_input_packet,
  turn_on,
  turn_off,
  duty_cycle,
};
/*---------------------------------------------------------------------------*/


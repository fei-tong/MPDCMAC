/*
 * Copyright (c) 2007, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met: 
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of c onditions and the following disclaimer.
 * 2. Redistributions in binary form mustc reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote prod ucts derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

/**
 * \file
 *         Example of how the collect primitive works.
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "lib/random.h"
#include "net/rime/rime.h"
#include "net/rime/collect.h"
#include "dev/leds.h"
#include "dev/button-sensor.h"

#include "net/netstack.h"

#include <stdio.h>
#include <stdlib.h>

static struct collect_conn tc;
uint32_t data_rcvd_num_at_sink = 0;
uint32_t gen_num = 0;


struct data_collect_t{
		uint32_t	gen_or_rcv_time; //generated or received time
		uint32_t	delay;
}__attribute__((packed));

#ifndef DATA_INTERVAL
    #define DATA_INTERVAL 30 //Send a packet every 30 seconds.
#endif

/*---------------------------------------------------------------------------*/
PROCESS(example_collect_process, "Test collect process");
AUTOSTART_PROCESSES(&example_collect_process);

/*---------------------------------------------------------------------------*/
static void
recv(const linkaddr_t *originator, uint8_t seqno, uint8_t hops)
{
	if(originator->u8[0] == 1){
	  	return;
	} 	
	struct data_collect_t *msg;
	msg = packetbuf_dataptr();
	//printf("sink1rcv_time is %lu,delay is %lu\n",
	//msg->gen_or_rcv_time,
	//msg->delay);
	if(msg->gen_or_rcv_time == 0){
	        return;
	}
  	data_rcvd_num_at_sink = data_rcvd_num_at_sink +1;
	printf("%lu R %u.%u %d %d %lu %d\n",
		clock_time(), 
		originator->u8[0],
		originator->u8[1],
		seqno,
		hops,
		msg->delay,
		data_rcvd_num_at_sink);
}
/*---------------------------------------------------------------------------*/
static const struct collect_callbacks callbacks = { recv };
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(example_collect_process, ev, data)
{
  static struct etimer periodic;
  static struct etimer et;

  PROCESS_BEGIN();

  printf("DATA_INTERVAL = %u\n",DATA_INTERVAL);

  collect_open(&tc, 130, COLLECT_ROUTER, &callbacks);

  if(linkaddr_node_addr.u8[0] == 1 &&
     linkaddr_node_addr.u8[1] == 0) {
    printf("I am sink\n");
    collect_set_sink(&tc, 1);
	//data_rcvd_num_at_sink = 0;
  }

  /* Allow some time for the network to settle. */
  etimer_set(&et, 120 * CLOCK_SECOND);
  PROCESS_WAIT_UNTIL(etimer_expired(&et));

  while(1) {

    /* Send a packet every 30 seconds. */
    etimer_set(&periodic, CLOCK_SECOND * DATA_INTERVAL);
    etimer_set(&et, random_rand() % (CLOCK_SECOND * DATA_INTERVAL));

    PROCESS_WAIT_UNTIL(etimer_expired(&et));

    {
        printf("lllSending\n");
	  	printf(" P %lu %lu %u %lu\n",
			energest_type_time(ENERGEST_TYPE_TRANSMIT), 
			energest_type_time(ENERGEST_TYPE_LISTEN), 
			0,
			energest_type_time(ENERGEST_TYPE_CPU)+ energest_type_time(ENERGEST_TYPE_LPM));  
	  
      static linkaddr_t oldparent;
      const linkaddr_t *parent;
	  struct data_collect_t *msg;
      packetbuf_clear();
 	  msg = (struct data_collect_t *)packetbuf_dataptr();
	  packetbuf_set_datalen(sizeof(struct data_collect_t));
	  msg->gen_or_rcv_time = clock_time();
	  msg->delay = 0; 
	//printf("size is data(%u)\n",packetbuf_datalen());
      collect_send(&tc, 5);

      parent = collect_parent(&tc);
      if(!linkaddr_cmp(parent, &oldparent)) {
        if(!linkaddr_cmp(&oldparent, &linkaddr_null)) {
          printf("#L %d 0\n", oldparent.u8[0]);
        }
        if(!linkaddr_cmp(parent, &linkaddr_null)) {
          printf("#L %d 1\n", parent->u8[0]);
        }
        linkaddr_copy(&oldparent, parent);
      }
    }

    PROCESS_WAIT_UNTIL(etimer_expired(&periodic));
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

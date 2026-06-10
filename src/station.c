#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "station.h"
#include "station.h"
#include "utils.h"

void station_send(struct station *src, struct station *dst, const char *message, struct scheduler *sched)
{
    struct eth_frame frame;
    memset(&frame, 0, sizeof(frame));

    frame.destination = dst->mac;
    frame.source = src->mac;

    frame.type[0] = 0x08;
    frame.type[1] = 0x00; 

    strncpy((char *)frame.data, message, sizeof(frame.data) - 1);
    
    scheduler_push(sched, &frame, SWITCH, src->p->equipment, src->p->num_voisin);
    while(sched->size > 0)
    {
        scheduler_tick(sched);
    }
}

void receive_frame_st(struct station *st, struct eth_frame *frame)
{
    if (frame->destination != st->mac && frame->destination != 0xFFFFFFFFFFFF) 
    {
        return;
    }

    printf("Station %d reçoit de ", st->id);
    print_mac(frame->source);
    printf(" : \"%s\"\n", (char *)frame->data);
}
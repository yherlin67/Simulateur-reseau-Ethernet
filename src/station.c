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

     struct switch_t *sw = src->p->equipment.switch_t;
    uint8_t in_port = 0;
    for(int k = 0; k < sw->nbPorts; k++)
    {
        if(sw->ports[k] &&
           sw->ports[k]->type == STATION &&
           sw->ports[k]->equipment.station == src)
        {
            in_port = k;
            break;
        }
    }

    scheduler_push(sched, &frame, SWITCH, src->p->equipment, in_port);
    scheduler_tick(sched);
}

void receive_frame_st(struct station *st, struct eth_frame *frame)
{
    if (frame->destination != st->mac && frame->destination != 0xFFFFFFFFFFFF) 
    {
        return;
    }

    printf("Station "); print_mac(st->mac);
    printf(" reçoit de "); print_mac(frame->source);
    printf(" : \"%s\"\n", (char *)frame->data);
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "station.h"
#include "utils.h"

void station_send(struct station *src, struct station *dst, const char *message, struct scheduler *sched, struct network *net)
{
    // Ne pas utiliser (void)net si possible, mais présent pour correspondre à ton prototype.
    (void)net;
    struct eth_frame frame;
    memset(&frame, 0, sizeof(frame));

    frame.destination = dst->mac;
    frame.source = src->mac;

    frame.type[0] = 0x00;
    frame.type[1] = 0x08; 

    strncpy((char *)frame.data, message, sizeof(frame.data) - 1);

    scheduler_push(sched, &frame, SWITCH, src->p->equipment, src->p->num);

    scheduler_tick(sched);
}

void receive_frame_st(struct station *st, struct eth_frame *frame, uint8_t num_port)
{
    // Garde le port pour contrer les avertissements de non utilisation
    (void)num_port;

    if (frame->destination != st->mac && frame->destination != 0xFFFFFFFFFFFF) 
    {
        return;
    }

    printf("Station "); print_mac(st->mac);
    printf(" reçoit de "); print_mac(frame->source);
    printf(" : \"%s\"\n", (char *)frame->data);
}
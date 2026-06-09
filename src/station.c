/*
#include "station.h"

void station_send(struct station *src, struct station *dst, const char *message, struct scheduler *sched, struct network *net)
{
    //La station construit sa trame
    struct eth_frame frame;
    memset(&frame, 0, sizeof(frame));

    frame.destination = dst->mac;
    frame.source = src->mac;

    // Ethernet II : type > 1500 → ici 0x0800 = IPv4 */
    frame.type[0] = 0x00;
    frame.type[1] = 0x08; // Ethernet II ou sinon sizeof(struct BPDU)

    strncpy((char *)frame.data, message, sizeof(frame.data) - 1);

    //Push dans la file vers le switch voisin
    scheduler_push(sched, &frame, SWITCH, src->p->equipment.switch_t, src->p->num);

    //Traiter au tick suivant
    scheduler_tick(sched, net);
}

void receive_frame(struct station *st, struct eth_frame *frame, uint8_t num_port)
{
    // Vérifier que la trame est bien pour cette station
    if (frame->destination != st->mac)
    {
        //Pas pour nous, on ignore
        return;
    }

    //La trame nous est destinée → on affiche le message
    printf("Station "); print_mac(st->mac);
    printf(" reçoit de "); print_mac(frame->source);
    printf(" : \"%s\"\n", (char *)frame->data);
}

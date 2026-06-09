#pragma once

#include <stdint.h>

#include "packet.h"
#include "switch.h"
#include "network.h"
#include "scheduler.h"


struct station {
    uint32_t ip;
    uint64_t mac;
    struct port *p;
    //struct MACaddress voisin;
};


void station_send(struct station *src, struct station *dst, const char *message, struct scheduler *sched);

void receive_frame_st(struct station *st, struct eth_frame *frame);

//void send_frame(struct eth_frame frame);
//Envoie une trame ethernet
//Affiche un message du type : trame envoyée !

//void receive_frame(struct eth_frame frame);
//Reçoit une trame ethernet
//Affiche un message du type : trame reçue !

//fonction envoyer_a()
//Au switch vosin d'office !
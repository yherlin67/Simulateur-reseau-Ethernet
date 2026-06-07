#pragma once

#include <stdint.h>

#include "packet.h"
#include "switch.h"


struct station {
    uint32_t ip;
    uint64_t mac;
    struct port *p;
    //struct MACaddress voisin;
};

void station_send(struct station *src, mac_t dst_mac, const char *message);

void receive_frame(struct station *st, struct eth_frame *frame, uint8_t num_port);

//void send_frame(struct eth_frame frame);
//Envoie une trame ethernet
//Affiche un message du type : trame envoyée !

//void receive_frame(struct eth_frame frame);
//Reçoit une trame ethernet
//Affiche un message du type : trame reçue !

//fonction envoyer_a()
//Au switch vosin d'office !
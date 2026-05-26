#pragma once

#include "adrr_IP.h"
#include "adrr_MAC.h"
#include "switch.h"
#include "packet.h"

struct station {
    struct IPaddress ip;
    struct MACaddress mac;
    //struct MACaddress voisin;
}

//void send_frame(struct eth_frame frame);
//Envoie une trame ethernet
//Affiche un message du type : trame envoyée !

//void receive_frame(struct eth_frame frame);
//Reçoit une trame ethernet
//Affiche un message du type : trame reçue !

//fonction envoyer_a()
//Au switch vosin d'office !
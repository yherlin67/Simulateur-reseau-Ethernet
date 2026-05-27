#pragma once

#include <stdint.h>

#include "packet.h"
#include "station.h"
#include "network.h"
#define MAX_PORTS 32

//#define SIZE_COMMUTATION_TABLE 128

struct commutation {
    uint64_t mac;
    uint8_t port;
};

struct switch_t {
    uint_fast64_t mac;
    uint8_t nbPorts;
    struct port ports[MAX_PORTS];
    uint8_t priority;
    struct commutation **tableCommutation;
    struct BPDU bpdu;
};

//void receive_frame(struct eth_frame frame);
//Recoit une trame et mets à jour la table de commutation, puis transmets
//Affiche un message du type: table mise à jour !

//void receive_BPDU(struct BPDU bpdu)
//Recoit un vecteur BPDU, mets à jour le sien
//Affiche un message du type: vecteur reçu !

//void send_BPDU(struct BPDU bpdu)
//Envoie le vecteur BPDU du switch
//Affiche un message du type: vecteur envoyé !
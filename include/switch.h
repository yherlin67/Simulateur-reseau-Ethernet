#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "packet.h"
#include "station.h"
#include "network.h"
#define MAX_PORTS 32

//#define SIZE_COMMUTATION_TABLE 128

enum type {
    IEEE802_3,
    ETHERNET_II
};

struct commutation {
    uint64_t mac;
    struct port *port;
};

struct switch_t {
    uint_fast64_t mac;
    uint8_t nbPorts;
    struct port *ports[MAX_PORTS];
    uint8_t priority;
    struct commutation *tableCommutation[32];
    struct BPDU bpdu;
};

enum type determine_type(struct eth_frame);
void send_to(int8_t num_port, struct eth_frame frame, struct network net);
void know_station(struct switch_t sw, struct eth_frame frame, struct network net);
void receive_frame_sw(struct switch_t switch_t, struct eth_frame frame);

//void receive_frame(struct eth_frame frame);
//Recoit une trame et mets à jour la table de commutation, puis transmets
//Affiche un message du type: table mise à jour !

//void receive_BPDU(struct BPDU bpdu)
//Recoit un vecteur BPDU, mets à jour le sien
//Affiche un message du type: vecteur reçu !

//void send_BPDU(struct BPDU bpdu)
//Envoie le vecteur BPDU du switch
//Affiche un message du type: vecteur envoye !

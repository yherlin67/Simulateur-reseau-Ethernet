#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "packet.h"
#include "station.h"
#include "network.h"

#define MAX_PORTS 32

//#define SIZE_COMMUTATION_TABLE 128


struct commutation_entry {
    uint64_t mac;
    struct port *port;
};

struct switch {
    uint64_t mac;
    uint8_t nbPorts;
    struct port *ports[MAX_PORTS];
    uint8_t priority;
    struct commutation_entry *tableCommutation[32];
    struct BPDU bpdu;  // c'est ce qu'on envoie.
    struct BPDU received[MAX_PORTS];     // meilleur BPDU reçu sur chaque port (c'est ce qu'on recoit)
};

enum frame_type determine_type(struct eth_frame);
void send_to(struct eth_frame *frame, struct switch_t *sw, int8_t num_port);
void know_station(struct switch *sw, struct eth_frame *frame);
void receive_frame(struct switch *sw, struct eth_frame *frame, uint8_t num_port);
void propagate_bpdu(struct switch *sw, struct BPDU *bpdu, uint8_t num_port);
void disable_stp(struct network *net);

//void receive_frame(struct eth_frame frame);
//Recoit une trame et mets à jour la table de commutation, puis transmets
//Affiche un message du type: table mise à jour !

//void receive_BPDU(struct BPDU bpdu)
//Recoit un vecteur BPDU, mets à jour le sien
//Affiche un message du type: vecteur reçu !

//void send_BPDU(struct BPDU bpdu)
//Envoie le vecteur BPDU du switch
//Affiche un message du type: vecteur envoye !

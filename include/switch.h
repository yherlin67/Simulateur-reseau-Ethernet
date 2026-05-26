#pragma once

#include "adrr_IP.h"
#include "adrr_MAC.h"
#include "station.h"
#include "packet.h"

//#define SIZE_COMMUTATION_TABLE 128

struct commutation {
    struct MACaddress;
    uint8_t port;
}

struct switch {
    struct MACaddress mac;
    uint8_t nbPorts;
    uint8_t priority;
    struct commutation **tableCommutation;
    struct BPDU bpdu;
}

//void receive_frame(struct eth_frame frame);
//Recoit une trame et mets à jour la table de commutation, puis transmets
//Affiche un message du type: table mise à jour !

//void receive_BPDU(struct BPDU bpdu)
//Recoit un vecteur BPDU, mets à jour le sien
//Affiche un message du type: vecteur reçu !

//void send_BPDU(struct BPDU bpdu)
//Envoie le vecteur BPDU du switch
//Affiche un message du type: vecteur envoyé !
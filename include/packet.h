#pragma once

#include "addr_IP.h"
#include "addr_MAC.h"


struct eth_frame {
    uint8_t preambule[7];
    uint8_t sfd;
    struct MACaddress destination;
    struct MACaddress source;
    uint8_t type[2];
    uint8_t data[1500];
    uint8_t bourrage[46];
    uint8_t fcs[4];
};

struct BPDU {
    struct MACaddress root; //addresse MAC du switch racine
    uint8_t cost; //coût pour atteindre la racine
    struct MACaddress bridge_id; //addresse MAC du port qui envoie le BPDU
};

//void verif_type(*t);
//fonction pour vérifier le type de la trame (Ethernet 2 ou IEEE 802.3)
//Cela dépends de la valeur dans le champs type. Si elle est inférieure à 1500, c'est une trame IEEE 802.3.
//Si elle est supérieure à 1500 en revanche, c'est une trame Ethernet 2.
//La diff entre les deux : Aujourd'hui IEEE 802.3 est quasi plus utilisé (sauf pour envoie des BPDU)
//Dans cette trame, le champ qu'on appelle aujourd'hui "type" sur la trame Ethernet2 stockait la longueur 
//des données inclues dans le paquet sur deux octets.
//Le protocole utilisé se trouvait donc sur un autre champs (LLC) sur trois octets.
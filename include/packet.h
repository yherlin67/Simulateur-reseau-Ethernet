#pragma once
#include <stdint.h>

// on peut faire un cast pour remplir automatiquement la structure avec les bons 
struct __attribute__((packed)) eth_frame {
    //uint8_t preambule[7];
    //uint8_t sfd;
    uint64_t destination; // adresse mac destination
    uint64_t source; // adresse mac source
    uint8_t type[2];
    uint8_t data[1500];
    uint8_t bourrage[46];
    //int8_t fcs[4];
};

enum frame_type {
    IEEE802_3,
    ETHERNET_II
};

struct BPDU {
    uint64_t root; //addresse MAC du switch racine
    uint8_t cost; //coût pour atteindre la racine
    uint64_t bridge_id; //addresse MAC du switch qui envoie le BPDU
    uint8_t num_port; // port émetteur qui envoie le BPDU
};

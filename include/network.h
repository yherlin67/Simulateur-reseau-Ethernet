#pragma once

#include "addr_IP.h"
#include "addr_MAC.h"
#include "station.h"
#include "switch.h"
#include "packet.h"

struct port {
    struct switch_t *parent;
    uint8_t num;  
    char status; //R, D, B
    char role; //Learning, Forwarding, Listening
    struct lien *lien;
};

struct lien {
    uint8_t cost;
    struct port *portA;
    struct port *portB;
};

struct reseau {
    size_t nbStations;
    size_t nbSwitchs;
    struct station **stations;
    struct switch_t **switchs;
    size_t nbLiens;
    struct lien **liens;
};

//void read_conf(FILE * configurationFile)
//Lit le fichier de configuration et pratique les échanges dans le réseau

//void run_STP(FILE * configurationFile)
//Lance STP dans le réseau

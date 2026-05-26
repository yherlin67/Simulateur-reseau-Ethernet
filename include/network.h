#pragma once

#include "adrr_IP.h"
#include "adrr_MAC.h"
#include "station.h"
#include "switch.h"
#include "packet.h"

struct port {
    struct switch *parent;
    uint8_t num;  
    char status; //R, D, B
    char role; //Learning, Forwarding, Listening
    struct lien *lien;
}

struct lien {
    uint8_t cost;
    port *portA;
    port *portB;
}

struct reseau {
    size_t nbStations;
    size_t nbSwitchs;
    struct station **stations;
    struct switch **switchs;
    size_t nbLiens;
    struct lien **liens;
}

//void read_conf(FILE * configurationFile)
//Lit le fichier de configuration et pratique les échanges dans le réseau

//void run_STP(FILE * configurationFile)
//Lance STP dans le réseau



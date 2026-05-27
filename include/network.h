#pragma once

#include <stdint.h>
#include <stdio.h>
#include "station.h"
#include "switch.h"
#include "packet.h"

enum status {
    ROOT,
    DESIGNED,
    BLOCKED,
    DEFAULT
};

enum role {
    LISTENING,
    FORWARDING,
    LEARNING,
    MODE_DEFAULT
};

struct port {
    uint8_t num;
    enum status s;  
    enum role r;
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

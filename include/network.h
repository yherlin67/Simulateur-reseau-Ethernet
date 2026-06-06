#pragma once

#include <stdint.h>
#include <stdio.h>
#include "station.h"
#include "switch.h"
#include "packet.h"

enum port_status {
    ROOT,
    DESIGNED,
    BLOCKED,
    DEFAULT
};

enum port_role {
    LISTENING,
    FORWARDING,
    LEARNING,
    MODE_DEFAULT
};

enum device_type {
    STATION,
    SWITCH
};

struct port {
    //Un port dans le tableau ports d'un switch est forcément connecté à ce switch, et un des deux autres appareils dans l'union
    //Un port en attribut d'une station est forcément connecté à cette station et un des deux autres appareils dans l'union
    //Il n'y a plus besoin de struct lien, cost represente le cout pour atteindre le port
    uint8_t num;
    uint8_t cost;
    enum port_status status;
    enum port_role role ;
    enum device_type type;
    // l'équipement connecté sous forme de union = station ou switch
    union {
        struct station *station;
        struct switch_t *switch_t;
    } equipment;  
};

// struct link {
//     uint8_t cost;
//     struct port *portA;
//     struct port *portB;
// };

struct network {
    size_t nbStations;
    size_t nbSwitchs;
    struct station *stations[32];
    struct switch_t *switchs[32];
    size_t nbLiens;
    struct lien **liens;
};

//void read_conf(FILE * configurationFile)
//Lit le fichier de configuration et pratique les échanges dans le réseau

//void run_STP(FILE * configurationFile)
//Lance STP dans le réseau

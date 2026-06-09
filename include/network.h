#pragma once

#include <stdint.h>
#include <stdio.h>
#include "packet.h"

struct station;
struct switch_t;

#define MAX_DEVICES 64

enum port_status {
    ROOT, // chemin le plus court vers la racine
    DESIGNATED, // retransmet les BPDU vers les segments
    BLOCKED, // coupe un cycle
    DEFAULT // état initial
};

// type de voisin connecté sur un port
enum device_type {
    STATION,
    SWITCH
};

// union switch ou station => définie ici avec un nom de type pour pouvoir être réutilisée dans struct port ET dans struct node_frame du scheduler 

union equipment_union {          
    struct station  *station;
    struct switch_t *switch_t;
};



struct port {
    //Un port dans le tableau ports d'un switch est forcément connecté à ce switch, et un des deux autres appareils dans l'union
    //Un port en attribut d'une station est forcément connecté à cette station et un des deux autres appareils dans l'union
    //Il n'y a plus besoin de struct lien, cost represente le cout pour atteindre le port
    uint8_t num;
    uint8_t cost;
    enum port_status status;
   // enum port_role role;
    enum device_type type;
    // l'équipement connecté sous forme de union = station ou switch
    union equipment_union equipment;
};

struct link {
   uint8_t cost;
   struct port *portA;
   struct port *portB;
};

struct network {
    size_t nb_stations;
    size_t nb_switchs;
    struct station *stations[MAX_DEVICES];
    struct switch_t *switchs[MAX_DEVICES];
    size_t nbLiens;
    struct lien **liens;
};

void ReadFile(const char *pathFile, struct network *res);

//void read_conf(FILE * configurationFile)
//Lit le fichier de configuration et pratique les échanges dans le réseau

// Chargement depuis fichier de config
//struct network *load_network(const char *path);
//void free_network(struct network *net);
//void print_network(struct network *net);


//void run_STP(FILE * configurationFile)
//Lance STP dans le réseau

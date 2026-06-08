#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>


//schedule.h ne doit pas inclure switch.h ni station.h directement
struct eth_frame;
struct switch_t;
struct station;

// pour utiliser device_type 
#include "network.h"

// 1 trame en attente dans la file de l'ordonnanceur = 1 noeud.
struct node_frame {
    struct eth_frame frame; // trame complete à envoyer
    // Qui envoie la trame ? 
    enum device_type dst_type; // définit si switch ou station 
    union equipment_union dst;      // même type que port.equipment
    uint8_t in_port;      //numéro du port d'entrée chez le destinataire
    struct node_frame *next; // pointeur pour le noeud (frame) suivante -> file sous forme de liste chainée 
};


// push : ajoute en tail (queue)
// pop : retire en head (tête)
// la file d'attente : 
struct scheduler{
    // liste chainée pour que ce soit dynamique 
    struct node_frame *head;   // premier élément à traiter              
    struct node_frame *tail;   // dernier élément ajouté                    
    int size;   // nombre de trames actuellement dans la file
};

//Initialise la file (vide)
void scheduler_init(struct scheduler *s);

//Ajoute une trame en queue de file + alloue un nouveau nœud dynamiquement
scheduler_push(sched, &frame, pt->type, pt->equipment, pt->num);

//Retire et retourne le nœud en tête de file.
//Retourne NULL si la file est vide.
// IMPORTANT : l'appelant doit free() le nœud après usage !!!!
struct frame_node *scheduler_pop(struct scheduler *s);

//Traite UN tick : envoie toutes les trames présentes AU DÉBUT du tick.
// Les nouvelles trames générées pendant le tick attendent le tick suivant.
// Retourne le nombre de trames traitées
int scheduler_tick(struct scheduler *s);
 
// Vide la file et libère toute la mémoire (noeuds)
void scheduler_clear(struct scheduler *s);
 
//Collecte les trames que les stations veulent envoyer et les ajoute dans la file => A FAIRE DANS LE MENU.
//void scheduler_collect(struct scheduler *s, struct network *net);

 
//Lance la simulation complète jusqu'à ce que la file soit vide => A FAIRE DANS LE MENU
//void scheduler_run(struct scheduler *s, struct network *net);

 
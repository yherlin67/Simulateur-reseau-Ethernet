#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "network.h"
#include "switch.h"    // receive_frame (switch)
#include "station.h"   // receive_frame_st       

// File pour l'ordonnanceur de trames

// initialisation de la file
void init_scheduler(struct schedueler *s){
    s->head = NULL;
    s->tail = NULL;
    s->size = 0;
}

//Ajoute une trame en QUEUE (tail) de la file.
void scheduler_push(struct scheduler *s, struct eth_frame *frame, enum device_type dst_type, union equipment_union dst, uint8_t in_port)
{
    struct node_frame *node = malloc(sizeof(struct node_frame));
    if (!node){
        printf("[SCHEDULER] Erreur malloc !\n"); return; 
    }

    // Remplir le nœud
    node->frame = *frame; //copie de la trame entière
    node->dst_type = dst_type;
    node->dst = dst;      
    node->in_port = in_port;
    node->next = NULL; // sera toujours le dernier !

    // connecter le nouveau noeud à la fin de la file (queue)
    if (s->tail == NULL) {
        // file vide : head et tail pointent sur le seul et meme nœud
        s->head = node;
        s->tail = node;
    } else {
        //file pas vide : on pose après le dernier nœud
        s->tail->next = node; // l'ancien dernier pointe vers le nouveau dernier
        s->tail = node; // tail avance vers le nouveau dernier 
    }
    s->size++;
}

/* ---------- scheduler_pop ----------
 * Retire et retourne le nœud en TÊTE (head).
 * Retourne NULL si la file est vide.
 * L'APPELANT DOIT FAIRE free(node) APRÈS USAGE.
 *
 * Avant :  head → [A] → [B] → NULL
 * Après :  head → [B] → NULL   (retourne [A])
 */
struct node_frame *scheduler_pop(struct scheduler *s)
{
    // si file vide : 
    if (s->head == NULL){
        return NULL;
    }

    struct node_frame *node = s->head;  // on sauvegarde la tête = premier élem
    s->head = s->head->next;            // le head avance au suivant
 
    // si file est vide alors il faut mettre tail à NULL
    if (s->head == NULL)
        s->tail = NULL;
 
    s->size--;
    return node; //celui qui utilise la fonction doit faire free(node) après usage !!!!!!
}


/* ---------- scheduler_tick ----------
 * Traite exactement les trames présentes AU DÉBUT du tick.
 * size_at_start garantit qu'on ne traite pas les nouvelles trames
 * générées pendant ce tick — elles attendent le tick suivant.
 */
int scheduler_tick(struct scheduler *s)
{

    int size_at_start = s->size; // nombre de trames à traiter pour CE tick
    int processed = 0;

    for (int i = 0; i < size_at_start; i++)
    {
        struct node_frame *node = scheduler_pop(s);
        // si scheduler_pop retourne null (on fait ca par précaution)
        if (!node){
            break;
        }

        if (node->dst_type == SWITCH)
        {
            //Le switch peut re-pusher dans la file (flood, relais) mais ces nouvelles trames n'appartiennent pas à ce tick.
            receive_frame(node->dst.sw, &node->frame, node->in_port, s);
        }
        else
        {
            // si le switch doit livrer la trame à une station 
            receive_frame_st(node->dst.st, &node->frame, node->in_port);
        }

        free(node); //libérer le nœud maintenant qu'il a fini d'etre traité.
        processed++;
    }
    return processed;
}

//vide la file et libère tous les nœuds restants
void scheduler_clear(struct scheduler *s)
{
    struct node_frame *node;
    while ((node = scheduler_pop(s)) != NULL){
        free(node);
    }
}
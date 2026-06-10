#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "switch.h"
#include "utils.h"
#include "station.h"


//Variable globale qui indique si le stp doit continuer
static bool stp_running = false;

// fonction pour donner l'id d'une station (+1) pour l'affichage, pour que l'utilisateur puisse voir l'échange de BPDU...
static int get_switch_id(struct network *net, struct switch_t *sw) {
    for (int i = 0; i < (int)net->nb_switchs; i++) {
        if (net->switchs[i] == sw){
            return i + 1;
        }
    }
    return -1;
}

static int get_switch_id_by_mac(struct network *net, uint64_t mac) {
    for (int i = 0; i < (int)net->nb_switchs; i++) {
        if (net->switchs[i]->mac == mac){
            return i + 1;
        }
    }
    return -1;
}

static void print_bpdu_receive(struct switch_t *sw, struct BPDU *bpdu)
{
    printf("[STP] Switch %d reçoit BPDU de ", sw->id);
    print_mac(bpdu->bridge_id);
    printf(" | racine proposée : ");
    print_mac(bpdu->root);
    printf(" | coût : %d\n", bpdu->cost);
}


void disable_stp(struct network *net)
{
    for (int i = 0; i < (int)net->nb_switchs; i++)
    {
        struct switch_t *sw = net->switchs[i];

        sw->visited = false;

        for (int p = 0; p < sw->nbPorts; p++)
            if (sw->ports[p]){
                sw->ports[p]->status = DEFAULT;
            }

        sw->bpdu->root = sw->mac;
        sw->bpdu->cost = 0;
        sw->bpdu->bridge_id = sw->mac;
        sw->bpdu->num_port = 0;
    }

    printf("STP désactivé — tous les ports sont ouverts.\n");
    printf("⚠ Si le réseau a des cycles, une tempête de broadcast est possible.\n");
}

void run_stp(struct network *net, struct scheduler *sched)
{
    init_stp(net);
    propagate_bpdu(net, sched);
}

enum frame_type determine_type(struct eth_frame frame)
{
    uint16_t type = (frame.type[0] << 8) | frame.type[1];

    if (type <= 1500)
    {
        return IEEE802_3;
    }

    return ETHERNET_II;
}

void receive_frame(struct switch_t *sw, struct eth_frame *frame, uint8_t num_port, struct scheduler *sched)
{
    enum frame_type type = determine_type(*frame);

    if(type == IEEE802_3)
    {
        struct BPDU *bpdu = (struct BPDU *)frame->data;

        // Affichage pour l'utilisateur...
        print_bpdu_receive(sw, bpdu);

        sw->received[num_port] = *bpdu;

        struct BPDU candidat;
        candidat.root = bpdu->root;
        candidat.cost = bpdu->cost + sw->ports[num_port]->cost;
        candidat.bridge_id = sw->mac;
        candidat.num_port = num_port;

        if(bpdu_is_better(&candidat, sw->bpdu))
        {
            *sw->bpdu = candidat;
            stp_running = true;
        }
    }
    else if(type == ETHERNET_II)
    {
        know_station(sw, frame, num_port, sched);
    }
}

void know_station(struct switch_t *sw, struct eth_frame *frame, uint8_t num_port, struct scheduler *sched)
{
    int port_dest = -1;
    for(int k = 0; k < MAX_PORTS; k++)
    {
        if(sw->tableCommutation[k] && sw->tableCommutation[k]->mac == frame->destination)
        {
            port_dest = sw->tableCommutation[k]->port->num;
            break;
        }
    }

    if(port_dest != -1)
    {
        send_to(frame, sw, (int8_t)port_dest, num_port, sched);
    }
    else
    {
        update_table(sw, num_port, frame);
        send_to(frame, sw, -1, num_port, sched);
    }
}

void update_table(struct switch_t *sw, uint8_t num_port, struct eth_frame *frame)
{
    // Chercher si la MAC existe déjà sur un autre port
    for(int k = 0; k < MAX_PORTS; k++) {
        if(sw->tableCommutation[k] != NULL && sw->tableCommutation[k]->mac == frame->source) {
            if(k == num_port) {
                return; // déjà connue sur ce port, rien à faire
            } else {
                // MAC connue sur un autre port, on supprime l'ancienne entrée
                free(sw->tableCommutation[k]);
                sw->tableCommutation[k] = NULL;
                break;
            }
        }
    }

    if(sw->tableCommutation[num_port] != NULL && sw->tableCommutation[num_port]->mac == frame->source)
    {
        return; // déjà connue sur ce port, rien à faire
    }

    if(sw->tableCommutation[num_port] == NULL) {
        sw->tableCommutation[num_port] = malloc(sizeof(struct commutation_entry));
    }
    sw->tableCommutation[num_port]->mac = frame->source;
    sw->tableCommutation[num_port]->port = sw->ports[num_port];

    printf("Switch %d apprend ", sw->id);
    print_mac(frame->source);
    printf(" sur port %u\n", num_port);
}

void send_to(struct eth_frame *frame, struct switch_t *sw, int8_t dest, uint8_t src, struct scheduler *sched)
{

    if(dest == -1)
    {
        for(int i = 0; i < sw->nbPorts; i++)
        {
            struct port *p = sw->ports[i];
            if(p == NULL || p->status == BLOCKED || i == src)
            {
                continue;
            }
            if(p->type == STATION)
            {
                receive_frame_st(p->equipment.station, frame);
            }
            else if(p->type == SWITCH)
            {
                if(p->equipment.switch_t == NULL){
                    continue;
                }
                // Vérifier que le port voisin n'est pas BLOCKED
                if(p->equipment.switch_t->ports[p->num_voisin]->status == BLOCKED){
                    continue;
                }

            union equipment_union eq;
            eq.switch_t = p->equipment.switch_t;
            scheduler_push(sched, frame, SWITCH, eq, p->num_voisin);
            }
        }
    }
    else
    {
        struct port *p = sw->ports[dest];
        if(p != NULL)
        {
            if(p->type == STATION)
            {
                receive_frame_st(p->equipment.station, frame);
            }
            else if(p->type == SWITCH)
            {
                union equipment_union eq;
                eq.switch_t = p->equipment.switch_t;
                scheduler_push(sched, frame, SWITCH, eq, p->num_voisin);
            }
        }
    }
}

void propagate_bpdu(struct network *net, struct scheduler *sched)
{
    bool changed = true;
    int tick = 1;
    printf("\n================ ÉCHANGE DE BPDU ================\n");

    while(changed)
    {
        printf("\n---------------- TICK n° %d ----------------\n\n", tick++);
        printf("-> Phase 1 : envoi des BPDU\n\n");
        changed = false;
        for(int i = 0; i < (int)net->nb_switchs; i++)
        {
            struct switch_t *switch_actuel = net->switchs[i];
            for(int j = 0; j < switch_actuel->nbPorts; j++)
            {
                struct port *pt = switch_actuel->ports[j];
                if (pt && pt->type == SWITCH)
                {
                    struct eth_frame new_frame;
                    memset(&new_frame, 0, sizeof(new_frame));

                    struct BPDU bpdu_to_send;
                    bpdu_to_send.root = switch_actuel->bpdu->root;
                    bpdu_to_send.cost = switch_actuel->bpdu->cost;
                    bpdu_to_send.bridge_id = switch_actuel->bpdu->bridge_id;
                    bpdu_to_send.num_port = pt->num;

                    new_frame.destination = pt->equipment.switch_t->mac; 
                    new_frame.source = switch_actuel->mac; 
                    new_frame.type[0] = 0x00; 
                    new_frame.type[1] = sizeof(struct BPDU);
                    memcpy(new_frame.data, &bpdu_to_send, sizeof(struct BPDU));
                    
                    printf("[STP] Switch %d envoie BPDU | racine : Switch %d | coût : %d\n",
                        get_switch_id(net, switch_actuel),
                        get_switch_id_by_mac(net, switch_actuel->bpdu->root),
                        switch_actuel->bpdu->cost);

                    scheduler_push(sched, &new_frame, pt->type, pt->equipment, pt->num_voisin);
                }
            }
        }
        printf("\n-> Phase 2 : réception des BPDU\n\n");
        stp_running = false;
        scheduler_tick(sched);
        printf("\n-> FIN du tick n°%d\n", tick);

        //Si le stp continue après le tick, on continue la propagation de BPDU (on reboucle)
        changed = stp_running;
    }
    //On va gérer chaque switch du réseau pour assigner le bon statut à leurs ports
    manage_switch(net);
    printf("\n================ FIN DE L'ÉCHANGE ================\n");
}

bool bpdu_is_better(struct BPDU *a, struct BPDU *b)
{
    if(a->root < b->root){
        return true;
    }
    if(a->root > b->root){
        return false;
    }

    if(a->cost < b->cost){
        return true;
    }
    if(a->cost > b->cost){
        return false;
    }

    if(a->bridge_id < b->bridge_id){
        return true;
    }
    if(a->bridge_id > b->bridge_id){
        return false;
    }

    return a->num_port < b->num_port;
}

void manage_switch(struct network *net)
{
    for(int i = 0; i < (int)net->nb_switchs; i++)
    {
        set_ports(net->switchs[i]);
    } 
}

void set_ports(struct switch_t * sw)
{
    for(int j = 0; j < sw->nbPorts; j++)
    {
        struct port *p = sw->ports[j];
        if(p == NULL || p->type != SWITCH){
            continue;
        }

        if(sw->mac == sw->bpdu->root)
        {
            // si le switch est la racine alors TOUS SES PORTS SONT DESIGNATED
            p->status = DESIGNATED;
        }
        else if(j == sw->bpdu->num_port)
        {
            p->status = ROOT; // port par lequel on atteint la racine
        }
        else if(bpdu_is_better(sw->bpdu, &sw->received[j]))
        {
            p->status = DESIGNATED; // mon BPDU est meilleur que ce que j'ai reçu ici
        }
        else
        {
            //J'ia recu un meilleur BPDU sur ce port, je bloque le port, il y a un meilleur chemin vers la racine !
            p->status = BLOCKED;
        }
    }
}

void init_stp(struct network *net)
{
    for (int i = 0; i < (int)net->nb_switchs; i++)
    {
        struct switch_t *sw = net->switchs[i];
        sw->bpdu->root = sw->mac;
        sw->bpdu->cost = 0;
        sw->bpdu->bridge_id = sw->mac;
        sw->bpdu->num_port  = 0;

        for (int p = 0; p < MAX_PORTS; p++) {
            sw->received[p].root = UINT64_MAX;
            sw->received[p].cost = UINT8_MAX;
            sw->received[p].bridge_id = UINT64_MAX;
            sw->received[p].num_port = UINT8_MAX;
        }

        for (int p = 0; p < sw->nbPorts; p++)
        {
            if (sw->ports[p]){
                sw->ports[p]->status = DEFAULT;
            }
        }
    }
}

void print_stp(struct network *net)
{
    // Trouver la racine
    int root_id = -1;
    for(int i = 0; i < (int)net->nb_switchs; i++) {
        if(net->switchs[i]->mac == net->switchs[i]->bpdu->root) {
            root_id = i + 1;
            break;
        }
    }

    printf("\n\n================ RÉSULTAT STP ================\n\n");
    printf("Switch racine : Switch %d\n\n", root_id);

    for(int i = 0; i < (int)net->nb_switchs; i++)
    {
        printf("Switch n°%d\n", i+1);
        for(int p = 0; p < net->switchs[i]->nbPorts; p++)
        {
            printf("-> Port n°%d a le status : %s\n",
                net->switchs[i]->ports[p]->num,
                port_status_str(net->switchs[i]->ports[p]->status));
        }
        printf("\n");
    }
    printf("\n=============================================\n");
}

void print_tab_commut(struct network *net, int indexSwitch)
{
    // SECURITÉ 1 : Vérifier que le réseau existe
    if (net == NULL) {
        printf("Erreur : Le réseau n'est pas initialisé.\n");
        return;
    }

    // SECURITÉ 2 : Vérifier que l'index demandé est dans les limites du tableau !
    if (indexSwitch < 0 || indexSwitch >= (int)net->nb_switchs) {
        printf("Erreur : Le switch n°%d n'existe pas (index hors limites).\n", indexSwitch);
        return;
    }

    struct switch_t *sw = net->switchs[indexSwitch];

    // SECURITÉ 3 : Vérifier que la structure du switch n'est pas NULL
    if (sw == NULL) {
        printf("Erreur : Le switch demandé est introuvable en mémoire.\n");
        return;
    }

    bool est_vide = true;

    for(int i = 0; i < MAX_PORTS; i++)
    {
        if(sw->tableCommutation[i] != NULL)
        {
            est_vide = false;
            break; // Dès qu'on trouve une entrée, inutile de continuer, la table n'est pas vide
        }
    }

    if(est_vide)
    {
        printf("La table de commutation du switch n°%d est vide.\n", indexSwitch+1);
    }
    else
    {
        printf("--- Table de commutation du Switch n°%d ---\n", indexSwitch+1);
        for(int i = 0; i < MAX_PORTS; i++)
        {
            if(sw->tableCommutation[i] != NULL)
            {
                int true_station_id = get_station_id_by_mac(net, sw->tableCommutation[i]->mac);

                if (true_station_id != -1) {
                    printf("Port local %d -> Station %d\n", i+1, true_station_id);
                } else {
                    printf("Port local %d -> MAC inconnue : ", i+1);
                    display_mac(sw->tableCommutation[i]->mac);
                    printf("\n");
                }
            }
        }
    }
}
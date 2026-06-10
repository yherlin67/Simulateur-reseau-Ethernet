#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "switch.h"
#include "utils.h"

extern void receive_frame_st(struct station *st, struct eth_frame *frame);

//Variable globale qui indique si le stp doit continuer
static bool stp_running = false;

void disable_stp(struct network *net)
{
    for (int i = 0; i < (int)net->nb_switchs; i++)
    {
        struct switch_t *sw = net->switchs[i];

        sw->visited = false;

        for (int p = 0; p < sw->nbPorts; p++)
            if (sw->ports[p])
                sw->ports[p]->status = DEFAULT;

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
    if(sw->tableCommutation[num_port] != NULL && sw->tableCommutation[num_port]->mac == frame->source)
    {
        return; // déjà connue sur ce port, rien à faire
    }

    if(sw->tableCommutation[num_port] == NULL) {
        sw->tableCommutation[num_port] = malloc(sizeof(struct commutation_entry));
    }
    sw->tableCommutation[num_port]->mac = frame->source;
    sw->tableCommutation[num_port]->port = sw->ports[num_port];

    printf("MISE A JOUR D'UNE TABLE DE COMMUTATION\n"); 
    print_mac(sw->mac);
    printf(" apprend "); print_mac(frame->source);
    printf(" sur port %u\n", num_port);
}

void send_to(struct eth_frame *frame, struct switch_t *sw, int8_t dest, uint8_t src, struct scheduler *sched)
{

    if(dest == -1)
    {
        for(int i = 0; i < sw->nbPorts; i++)
        {
            struct port *p = sw->ports[i];
            if(p == NULL || p->status == BLOCKED || p->status == ROOT || i == src)
            {
                continue;
            }
            if(p->type == STATION)
            {
                receive_frame_st(p->equipment.station, frame);
            }
            else if(p->type == SWITCH)
            {
                if(p->equipment.switch_t == NULL)
                {
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
    while(changed)
    {
        changed = false;
        for(int i = 0; i < (int)net->nb_switchs; i++)
        {
            struct switch_t *switch_actuel = net->switchs[i];
            for(int j = 0; j < switch_actuel->nbPorts; j++)
            {
                struct port *pt = switch_actuel->ports[j];
                if (pt && pt->type == SWITCH)
                {
                    // printf("[DEBUG] push vers switch voisin\n");
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

                    scheduler_push(sched, &new_frame, pt->type, pt->equipment, pt->num_voisin);
                }
            }
        }
        stp_running = false;
        scheduler_tick(sched);
        //Si le stp continue après le tick, on continue la propagation de BPDU (on reboucle)
        changed = stp_running;
    }
    //On va gérer chaque switch du réseau pour assigner le bon statut à leurs ports
    manage_switch(net);
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

//Plus besoin de cette fonction ?
bool update_bpdu(struct switch_t *sw, struct BPDU *bpdu, uint8_t num_port)
{
    sw->received[num_port] = *bpdu;

    sw->bpdu->root = bpdu->root;
    sw->bpdu->cost = bpdu->cost + sw->ports[num_port]->cost;
    sw->bpdu->bridge_id = sw->mac;
    return true;
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
    for(int i = 0; i < (int)net->nb_switchs; i++)
    {
        printf("Switch [ID n°%d]\n",i);
        for(int p = 0; p < net->switchs[i]->nbPorts; p++)
        {
            if(net->switchs[i]->ports[p]->status == 0)
            {
                printf("-> Port n°%d a le status : ROOT\n", net->switchs[i]->ports[p]->num);
            }
            else if(net->switchs[i]->ports[p]->status == 1)
            {
                printf("-> Port n°%d a le status : DESIGNATED\n", net->switchs[i]->ports[p]->num);
            } 
            else if(net->switchs[i]->ports[p]->status == 2)
            {
                printf("-> Port n°%d a le status : BLOCKED\n", net->switchs[i]->ports[p]->num);
            } 
            else if(net->switchs[i]->ports[p]->status == 3)
            {
                printf("-> Port n°%d a le status : DEFAULT\n", net->switchs[i]->ports[p]->num);
            } 
        } 
        printf("\n");
    } 
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

    for(int i = 0; i < 32; i++)
    {
        if(sw->tableCommutation[i] != NULL)
        {
            est_vide = false;
            break; // Dès qu'on trouve une entrée, inutile de continuer, la table n'est pas vide
        }
    }

    if(est_vide)
    {
        printf("La table de commutation du switch n°%d est vide.\n", indexSwitch);
    }
    else
    {
        printf("--- Table de commutation du Switch n°%d ---\n", indexSwitch);
        for(int i = 0; i < 32; i++)
        {
            if(sw->tableCommutation[i] != NULL)
            {
                printf("Port local %d -> MAC: ", i);
                display_mac(sw->tableCommutation[i]->mac); 
                printf("\n");
            }
        }
    }
}

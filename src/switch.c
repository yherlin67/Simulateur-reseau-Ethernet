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

void receive_frame(struct switch_t *sw, struct eth_frame *frame, uint8_t num_port)
{
    enum frame_type type = determine_type(*frame);

    if(type == IEEE802_3)
    {
        struct BPDU *bpdu = (struct BPDU *)frame->data;
        sw->received[num_port] = *bpdu;  
        //on stocke toujours le BPDU recu du port voisin pour le comparer dans set_ports, 
        //parce que si on le le change pas, ca reste à UINT8_t...

        // (Attention : on compare pas direct le bpdu stocké dans received car c'est celui du voisin !)
        // calculer ce que deviendrait NOTRE BPDU si on acceptait celui du voisin
        struct BPDU candidat;
        candidat.root = bpdu->root;
        candidat.cost = bpdu->cost + sw->ports[num_port]->cost;
        candidat.bridge_id = sw->mac;
        candidat.num_port = num_port;

        // l'accepter seulement si c'est vraiment meilleur
        if(bpdu_is_better(&candidat, sw->bpdu))
        {
            *sw->bpdu = candidat;
            stp_running = true;
        }
        //Si on entre jamais dans la condition ci-dessus, 
        //aucun switch ne mets à jour son BPDU stp_running reste à false et on arrête le stp
    }
    else if(type == ETHERNET_II)
    {
        know_station(sw, frame, num_port);
    }
}

void know_station(struct switch_t *sw, struct eth_frame *frame, uint8_t num_port)
{
    if(sw->tableCommutation[num_port] && sw->tableCommutation[num_port]->mac == frame->destination)
    {
        uint8_t num_stat = sw->tableCommutation[num_port]->port->num; 
        send_to(frame, sw, num_stat); 
    }
    else
    {
        update_table(sw, num_port, frame);
        send_to(frame, sw, -1);
    }
}

void update_table(struct switch_t *sw, uint8_t num_port, struct eth_frame *frame)
{
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

void send_to(struct eth_frame *frame, struct switch_t *sw, int8_t num_port)
{
    if(num_port == -1)
    {
        for(int i = 0; i < sw->nbPorts; i++)
        {
            struct port *p = sw->ports[i];
            if(p == NULL || p->status == BLOCKED)
            {
                continue;
            }
            if(p->type == STATION)
            {
                receive_frame_st(p->equipment.station, frame);
            }
            else if(p->type == SWITCH)
            {
                receive_frame(p->equipment.switch_t, frame, p->num);
            }
        }
    }
    else
    {
        struct port *p = sw->ports[num_port];
        if(p != NULL) {
            if(p->type == STATION)
            {
                receive_frame_st(p->equipment.station, frame);
            }
            else if(p->type == SWITCH)
            {
                receive_frame(p->equipment.switch_t, frame, p->num);
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
                    printf("[DEBUG] push vers switch voisin\n");
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

                    // on cherche le port destinataire du switch voisin pour le scheduler :
                    struct switch_t *voisin = pt->equipment.switch_t;
                    uint8_t in_port = 0;
                    for(int k = 0; k < voisin->nbPorts; k++)
                    {
                        if(voisin->ports[k] &&
                        voisin->ports[k]->type == SWITCH &&
                        voisin->ports[k]->equipment.switch_t == switch_actuel)
                        {
                            in_port = k;
                            break;
                        }
                    }
                    scheduler_push(sched, &new_frame, pt->type, pt->equipment, in_port);
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
    if(a->root < b->root) return true;
    if(a->root > b->root) return false;

    if(a->cost < b->cost) return true;
    if(a->cost > b->cost) return false;

    if(a->bridge_id < b->bridge_id) return true;
    if(a->bridge_id > b->bridge_id) return false;

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

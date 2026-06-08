#include "switch.h"

static bool stp_running = true;

enum frame_type determine_type(struct eth_frame frame)
{
    uint16_t type = (frame.type[0] << 8) | frame.type[1];

    if (type <= 1500)
    {
        printf("IEEE 802.3 - longueur: %u octets\n", type);
        return IEEE802_3;
    }

    printf("Ethernet II - protocole: 0x%04x\n", type);
    return ETHERNET_II;

}

void receive_frame(struct switch_t *sw, struct eth_frame *frame, uint8_t num_port)
{
    //num_port représente le numéro du port sur lequel la trame a été reçue
    enum frame_type type = determine_type(*frame);

    if(type == IEEE802_3)
    {
        // Les données contiennent un BPDU
        //J'interprète un tableau d'ocets en mémoire comme une structure
        struct BPDU *bpdu = (struct BPDU *)frame->data;
        if(!update_bpdu(sw,bpdu))
        {
            stp_running = false;
        }
    }
    else if(type == ETHERNET_II)
    {
        know_station(sw, frame);
    }
}

void know_station(struct switch_t *sw, struct eth_frame *frame, uint8_t num_port)
{
    //IMPORTANT : i-ème port su switch = relié à la i-ème station du réseau !
    bool know = false;
    for(int i = 0; i < 32; i++)
    {
        //memcmp ( const void * pointer1, const void * pointer2, size_t size )
        //Compare le contenu de deux blocs mémoires octets par octets sur une taille de size
        //Si ca renvoie autre chose que 0 : les octets diffèrent
        if(memcmp(sw->tableCommutation[i]->mac, frame->destination, 6) == 0)
        {
            //On récup le numéro de port = numéro de la station
            uint8_t num_port = sw->tableCommutation[i]->port; 
            send_to(frame, sw, num_port);  // envoie sur ce port
            know = true;
        }
    }
    if(!know)
    {
        //Le switch ne connaît pas la station de destination, envoyons la trame à tout le monde !;
        {
            update_table(sw, num_port, frame);
            //Broadcast
            send_to(frame, sw, -1);
        }
    }
}

void update_table(struct switch_t *sw, uint8_t num_port, struct eth_frame *frame)
{
    struct commutation_entry *commut = malloc(sizeof(struct commutation_entry));
    commut->mac = frame->source;
    commut->port = sw->ports[num_port];
    sw->tableCommutation[num_port] = commut;
}

void send_to(struct eth_frame *frame, struct switch_t *sw, int8_t num_port)
{
    if(num_port == -1)
    {
        // Broadcast, envoie à toutes les stations
        for(int i = 0; i < sw->nbPorts; i++)
        {
            struct port *p = sw->ports[i];
            if(p->status == BLOCKED)
            {
                //STP a blocké le port, on continue
                continue;
            }
            if(p->type == STATION)
            {
                receive_frame_st(p->equipment.station, frame, num_port);
            }
            else if(p->type == SWITCH)
            {
                receive_frame(p->equipment.switch_t, frame, num_port);
            }
            
        }
    }
    else
    {
        // Envoie uniquement à l'équipement connectée sur ce port
        struct port *p = sw->ports[num_port];
        if(p->type == STATION)
        {
            receive_frame_st(p->equipment.station, frame, num_port);
        }
        else if(p->type == SWITCH)
        {
            receive_frame(p->equipment.switch_t, frame, num_port);
        }
    }
}

void propagate_bpdu(struct network *net)
{
    bool changed = true;
    while(changed)
    {
        changed = false;
        //Pour chaque switch du réseau
        for(int i = 0; i < net->nb_switchs; i++)
        {
            struct switch_t *switch_actuel = net->switchs[i];
            //Pour chaque port du switch
            for(int i = 0; i < switch_actuel->nbPorts; i++)
            {
                struct port *pt = switch_actuel->ports[i];
                //On envoie le BPDU que sur des ports qui correspondent à des switchs
                if (pt->type == SWITCH)
                {
                    //Construction du BPDU à envoyer sur ce port
                    struct BPDU *bpdu_to_send = malloc(sizeof(struct BPDU));
                    bpdu_to_send->root = switch_actuel->bpdu->root;
                    bpdu_to_send->cost = switch_actuel->bpdu->cost;
                    bpdu_to_send->bridge_id = switch_actuel->bpdu->bridge_id;
                    bpdu_to_send->num_port = pt->num;

                    //Encapsulation du BPDU dans une trame Ethernet
                    struct eth_frame *new_frame = malloc(sizeof(struct eth_frame));
                    new_frame->destination = pt->equipment.switch_t->mac; //MAC du switch voisin
                    new_frame->source = switch_actuel->mac; //MAC du switch actuel
                    //Longueur des données
                    new_frame->type[0] = 0x00; 
                    new_frame->type[1] = sizeof(struct BPDU);
                    //On copie le BPDU dans les données de la trame
                    memcpy(new_frame->data, bpdu_to_send, sizeof(struct BPDU));

                    scheduler_push(sced, new_frame,switch_actuel, pt->equipment.switch_t, pt->num);

                }
                else
                {
                    continue;
                }
                
            }
        }
    }
    
}

bool update_bpdu(struct switch_t *sw, struct BPDU *bpdu)
{
    if(bpdu->root < sw->bpdu->root)
    {
        sw->bpdu->root = bpdu->root;          //on met à jour la racine
        sw->bpdu->cost = bpdu->cost + 1;      //on incrémente le coût
        sw->bpdu->bridge_id = sw->mac;        //on est le switch qui propage
        return true;
    }
    return false;
}

void search_root(struct network *net)
{
    uint8_t racine;
    //Parcours des switchs du réseau
    for(int i = 0; i < net->nb_switchs; i++)
    {
        struct switch_t *switch_actuel = net->switchs[i];
        if(switch_actuel->mac == switch_actuel->bpdu->root)
        {
            //On a trouvé la racine !
            racine = i;
            break;
        }
    }
    struct switch_t *switch_racine;
    
    set_roots_ports(switch_racine);   

}

void set_roots_ports(struct switch_t * switch_racine)
{
    for(int j = 0; j < switch_racine->nbPorts; j++)
    {
        if(switch_racine->ports[j]->type == SWITCH)
        {
            struct port *p = switch_racine->ports[j];
            p->status = ROOT;
            struct switch_t *switch_passerelle = p->equipment.switch_t;
            for(int k = 0; k < switch_passerelle->nbPorts; k++)
            {
                if(switch_passerelle->ports[k]->status != ROOT)
                {
                    switch_passerelle->ports[k]->status = DESIGNED;
                    
                }
            }
            set_roots_ports(switch_passerelle);
        }
    }
    
}
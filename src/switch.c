#include "switch.h"

//Variable globale pour savoir si le stp continue ou pas
static bool stp_running = true;

/*
know_station() → UNIQUEMENT apprendre la MAC source → port d'entrée
lookup_cam() → UNIQUEMENT chercher la MAC destination
receive_frame() → orchestre tout : know_station + lookup_cam + scheduler_push
*/

void disable_stp(struct network *net)
{
    for (int i = 0; i < (int)net->nb_switchs; i++)
    {
        struct switch_t *sw = net->switchs[i];

        // Reset visited
        sw->visited = false;

        //tous les ports en DEFAULT
        for (int p = 0; p < sw->nbPorts; p++)
            if (sw->ports[p])
                sw->ports[p]->status = DEFAULT;

        // chaque switch redevient sa propre racine
        sw->bpdu->root = sw->mac;
        sw->bpdu->cost = 0;
        sw->bpdu->bridge_id = sw->mac;
        sw->bpdu->num_port = 0;
    }

    printf("STP désactivé — tous les ports sont ouverts.\n");
    printf("⚠ Si le réseau a des cycles, ");
    printf("une tempête de broadcast est possible.\n");
}

void run_stp(struct network *net, struct scheduler *sched)
{
    init_stp(net);
    propagate_bpdu(net, sched);
}



// PARFAITE 
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
        if(bpdu_is_better(sw->bpdu, bpdu))
            update_bpdu(sw, bpdu,num_port);
        else
            stp_running = false;

    }
    else if(type == ETHERNET_II)
    {
        know_station(sw, frame, num_port);
 
    }
}

/*
// Reçoit une trame sur num_port et push vers le prochain destinataire.
// on passe pt->type et pt->equipment directement à scheduler_push sans avoir besoin de tester SWITCH ou STATION parce que j'ai rajouté UNION
void receive_frame(struct switch_t *sw, struct eth_frame *frame, uint8_t num_port, struct scheduler *sched)
{
    enum frame_type type = determine_type(*frame);
 
    if (type == IEEE802_3)
    {
        // BPDU => mettre à jour la vision du réseau
        // Pas de push ici : propagate_bpdu gère la boucle.
        struct BPDU *bpdu = (struct BPDU *)frame->data;
        update_bpdu(sw, bpdu, num_port);
    }
    else //ETHERNET_II
    {
        // apprendre la MAC source
        know_station(sw, frame, num_port);
 
        // chercher la destination dans notre table de cmomutation
        int out_port = lookup_cam(sw, frame->destination);
 
        if (out_port >= 0)
        {
            // si la destination connue : envoi sur le port lié à cette adresse MAC
            struct port *p = sw->ports[out_port];
            if (p->status == BLOCKED){ // stp bloque
                return;
            }
 
            //On passe p->type et p->equipment directement — plus de test SWITCH/STATION
            scheduler_push(sched, frame, p->type, p->equipment, p->num);
        }
        else
        {
            // destination inconnue : on fait un broadcast sur les ports DESIGNATED uniquement
            for (int i = 0; i < sw->nbPorts; i++)
            {
                if (i == num_port){
                    continue;
                }
                if (!sw->ports[i]){
                    continue;
                }
                if (sw->ports[i]->status != BLOQUED){
                    continue; // DEFAULT et DESIGNATED passent et  ROOT aussi pour le flood données 
                }
                struct port *p = sw->ports[i];
                scheduler_push(sched, frame, p->type, p->equipment, p->num);
            }
        }
    }
}
*/





// mémorise les ports pour les diff adresses mac
void know_station(struct switch_t *sw, struct eth_frame *frame, uint8_t num_port)
{
    //IMPORTANT : i-ème port su switch = relié à la i-ème station du réseau !

    //memcmp ( const void * pointer1, const void * pointer2, size_t size )
    //Compare le contenu de deux blocs mémoires octets par octets sur une taille de size
    //Si ca renvoie autre chose que 0 : les octets diffèrent
    if(memcmp(sw->tableCommutation[num_port]->mac, frame->destination, 6) == 0)
    {
        //On récup le numéro de port = numéro de la station
        uint8_t num_stat = sw->tableCommutation[num_port]->port; 
        send_to(frame, sw, num_stat);  // envoie sur ce port
    }
    else
    {
        //Le switch ne connaît pas la station de destination, envoyons la trame à tout le monde !;
        {
            update_table(sw, num_port, frame);
            //Broadcast
            send_to(frame, sw, -1);
        }
    }
}

/*
void know_station(struct switch_t *sw, struct eth_frame *frame, uint8_t num_port)
{
    //Déjà connue ?
    for (int i = 0; i < 32; i++) {
        if (!sw->tableCommutation[i]) break;
        if (sw->tableCommutation[i]->mac == frame->source) return;
    }
    //Apprendre dans le premier slot libre
    for (int i = 0; i < 32; i++) {
        if (!sw->tableCommutation[i]) {
            sw->tableCommutation[i] = malloc(sizeof(struct commutation_entry));
            sw->tableCommutation[i]->mac = frame->source;
            sw->tableCommutation[i]->port = sw->ports[num_port];
            printf("[CAM] "); print_mac(sw->mac);
            printf(" apprend "); print_mac(frame->source);
            printf(" sur port %u\n", num_port);
            return;
        }
    }
}
*/


/*
//Cherche l'@MAC destination retourne le numéro de port ou -1 
int lookup_cam(struct switch_t *sw, uint64_t mac)
{
    for (int i = 0; i < 32; i++) {
        if (!sw->tableCommutation[i]){
            return -1;
        }
        if (sw->tableCommutation[i]->mac == mac){
            return sw->tableCommutation[i]->port->num;        
        }
    }
    return -1;
}
*/

void update_table(struct switch_t *sw, uint8_t num_port, struct eth_frame *frame)
{
    struct commutation_entry *commut = malloc(sizeof(struct commutation_entry));
    commut->mac = frame->source;
    commut->port = sw->ports[num_port];
    sw->tableCommutation[num_port] = commut;

    printf("MISE A JOUR D'UNE TABLE DE COMMUTATION\n"); 
    print_mac(sw->mac);
    printf(" apprend "); print_mac(frame->source);
    printf(" sur port %u\n", num_port);
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

void propagate_bpdu(struct network *net, struct scheduler *sched)
{
    //Est ce qu'il y a encore des switchs qui mettent à jour leur bpdu ?
    bool changed = true;
    while(changed)
    {
        changed = false;
        //Pour chaque switch du réseau
        for(int i = 0; i < net->nb_switchs; i++)
        {
            struct switch_t *switch_actuel = net->switchs[i];
            //Pour chaque port du switch
            for(int j = 0; j < switch_actuel->nbPorts; j++)
            {
                struct port *pt = switch_actuel->ports[j];
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

                    //On ajoute la trame à la file d'attente
                    scheduler_push(sched, &new_frame, pt->type, pt->equipment, pt->num);

                }
                else
                {
                    continue;
                }
                
            }
        }
        //On traite toutes les trames en un tick
        scheduler_tick(sched);
        changed = stp_running;
    }
    assign_ports(net);
    
}



/* Boucle STP :
 *   1. Tous les switchs pushent leur BPDU dans la file
 *   2. scheduler_tick traite tout le monde en même temps
 *   3. Si quelque chose a changé → recommencer
 *   4. Convergence → assign_ports */
/*
void propagate_bpdu(struct network *net, struct scheduler *sched)
{
    bool changed = true;
 
    while (changed)
    {
        changed = false;
 
        // PHASE 1 : push les BPDU de tous les switchs
        for (int i = 0; i < (int)net->nb_switchs; i++)
        {
            struct switch_t *sw = net->switchs[i];
 
            for (int p = 0; p < sw->nbPorts; p++)
            {
                struct port *pt = sw->ports[p];
                if (!pt || pt->type != SWITCH) continue;
 
                // Construire le BPDU
                struct BPDU bpdu_to_send;
                bpdu_to_send.root = sw->bpdu.root;
                bpdu_to_send.cost = sw->bpdu.cost;
                bpdu_to_send.bridge_id = sw->mac;
                bpdu_to_send.num_port = pt->num;
 
                //Encapsuler dans une trame IEEE 802.3
                struct eth_frame frame;
                memset(&frame, 0, sizeof(frame));
                frame.destination = pt->equipment.switch_t->mac;
                frame.source = sw->mac;
                frame.type[0] = 0x00;
                frame.type[1] = (uint8_t)sizeof(struct BPDU);
                memcpy(frame.data, &bpdu_to_send, sizeof(struct BPDU));
 
                // Push — on passe pt->type et pt->equipment directement
                scheduler_push(sched, &frame, pt->type, pt->equipment, pt->num);
            }
        }
 
        // PHASE 2 : traiter toutes les trames en un tick
        scheduler_tick(sched, net);
 
        // PHASE 3 : vérifier si un switch a changé de racine
        for (int i = 0; i < (int)net->nb_switchs; i++) {
            if (net->switchs[i]->bpdu.root != net->switchs[i]->mac) {
                changed = true;
                break;
            }
        }
    }
 
    // Convergence → attribuer les états de ports
    assign_ports(net);
}
*/


// ON COMPARE LES CHEMINS, c'est TOUT
bool bpdu_is_better(struct BPDU *a, struct BPDU *b)
{
    int cmp_root = memcmp(&a->root, &b->root, 6);
    if(cmp_root < 0) return true;
    if(cmp_root > 0) return false;

    if(a->cost < b->cost) return true;
    if(a->cost > b->cost) return false;

    int cmp_bridge = memcmp(&a->bridge_id, &b->bridge_id, 6);
    if(cmp_bridge < 0) return true;
    if(cmp_bridge > 0) return false;

    return a->num_port < b->num_port;
}

bool update_bpdu(struct switch_t *sw, struct BPDU *bpdu, uint8_t num_port)
{
    // On stocke le BPDU reçu sur ce port
    sw->received[num_port] = *bpdu;

    // On met à jour notre BPDU
    sw->bpdu->root = bpdu->root;
    sw->bpdu->cost = bpdu->cost + 1;
    sw->bpdu->bridge_id = sw->mac;
    return true;
}

/* // construit BPDU candidat à partir de ce qu'elle recoit. UTILISE BPDU_is_better => pour dtermnier si on chnage candidat ou pas (mise à jour ou pas)
// ATTENTION LE BPDU DE CHAQUE PORT CHANGE !!!!
bool update_bpdu(struct switch_t *sw, struct BPDU *bpdu, uint8_t num_port)
{
    // Stocker le BPDU reçu sur ce port
    sw->received[num_port] = *bpdu;

    // construire le cnadidat
    struct BPDU candidat;
    candidat.root = bpdu->root;
    candidat.cost = bpdu->cost + sw->ports[num_port]->cost;
    candidat.bridge_id = sw->mac;
    candidat.num_port = num_port;
 
    //vérif si le bpdu du switch candidat est vraiment différent pour éviter la boucle infinie (=> deux switches avec coût identique)
    if (bpdu_is_better(&candidat, &sw->bpdu) && memcmp(&candidat, &sw->bpdu, sizeof(struct BPDU)) != 0)
    {
        sw->bpdu = candidat;
        return true;
    }
    return false;
}
*/

/*
void assign_ports(struct network *net)
{
    for (int i = 0; i < (int)net->nb_switchs; i++)
    {
        struct switch *sw = net->switchs[i];
        bool is_root = (sw->mac == sw->bpdu.root);
        int root_port = -1;
 
        if (!is_root)
        {
            struct BPDU meilleur;
            meilleur.root = UINT64_MAX;
            meilleur.cost = UINT8_MAX;
            meilleur.bridge_id = UINT64_MAX;
            meilleur.num_port = UINT8_MAX;
 
            for (int p = 0; p < sw->nbPorts; p++)
            {
                struct port *pt = sw->ports[p];
                if (!pt || pt->type != SWITCH){
                    continue;
                }
                if (bpdu_is_better(&sw->received[p], &meilleur)) {
                    meilleur  = sw->received[p];
                    root_port = p;
                }
            }
        }
 
        for (int p = 0; p < sw->nbPorts; p++)
        {
            struct port *pt = sw->ports[p];
            if (!pt) continue;
 
            if (pt->type == STATION) {
                pt->status = DESIGNATED;
            } else if (p == root_port) {
                pt->status = ROOT;
            } else {
                pt->status = bpdu_is_better(&sw->received[p], &sw->bpdu) ? BLOCKED : DESIGNATED;
            }
        }
    }
}
*/



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
    struct switch_t *switch_racine = net->switchs[racine];
    
    set_ports(switch_racine);   

}

void set_ports(struct switch_t * sw)
{
    if(sw->visited) 
        return;
    sw->visited = true;

    for(int j = 0; j < sw->nbPorts; j++)
    {
        struct port *p = sw->ports[j];
        if(p->type != SWITCH) 
            continue;

        

        // Si c'est la racine, tous ses ports sont DESIGNATED
        if(sw->mac == sw->bpdu->root)
        {
            p->status = DESIGNATED;
        }
        else
        {
            // Port racine = celui par lequel on a reçu le meilleur BPDU
            // Les autres sont DESIGNATED ou BLOCKED
            if(p->status != ROOT)
                p->status = bpdu_is_better(&sw->received[j], sw->bpdu) ? BLOCKED : DESIGNATED;
        }

        struct switch_t *voisin = p->equipment.switch_t;
        // Le port du voisin qui pointe vers nous = port ROOT
        for(int k = 0; k < voisin->nbPorts; k++)
        {
            if(voisin->ports[k]->equipment.switch_t->mac == sw->mac)
            {
                voisin->ports[k]->status = ROOT;
                break;
            }
        }

        set_ports(voisin);
    }
    
}

void init_stp(struct network *net)
{
    for (int i = 0; i < net->nb_switchs; i++)
    {
        struct switch_t *sw = net->switchs[i];
        sw->bpdu->root = sw->mac;
        sw->bpdu->cost = 0;
        sw->bpdu->bridge_id = sw->mac;
        sw->bpdu->num_port  = 0;

        // Initialiser received avec pire valeur possible pour que le premier BPDU reçu soit forcément meilleur
        for (int p = 0; p < MAX_PORTS; p++) {
            sw->received[p].root      = UINT64_MAX;
            sw->received[p].cost      = UINT8_MAX;
            sw->received[p].bridge_id = UINT64_MAX;
            sw->received[p].num_port  = UINT8_MAX;
        }

        for (int p = 0; p < sw->nbPorts; p++)
        {
            if (sw->ports[p]){
                sw->ports[p]->status = DEFAULT;
            }
        }
    }
}
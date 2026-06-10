#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h> 

#include "network.h"
#include "utils.h"
#include "switch.h" 
#include "station.h"

struct raw_link {
    int id1;
    int id2;
    int cost;
};

//stock les liens pour plus tard
static struct raw_link *saved_raw_links = NULL;
static int saved_nb_eq = 0;

//Constructeur pour créer un switch
static struct switch_t *make_switch(uint64_t mac, uint16_t priority) {
    //Alloue un switch dans la mémoire et met l'espace mémoire à 0
    struct switch_t *sw = calloc(1, sizeof(struct switch_t));
    sw->mac = mac;
    sw->priority = priority;
    //Pas besoin vu qu'on a déjà calloc à 0 mais bon c'est plus clair
    sw->nbPorts = 0;
    
    // allocation du BPDU pour effectuer le spanning tree pour la première fois.
    sw->bpdu = calloc(1, sizeof(struct BPDU)); 
    sw->bpdu->root = mac;
    sw->bpdu->cost = 0;
    sw->bpdu->bridge_id = mac;
    sw->bpdu->num_port = 0;
    sw->visited = false;
    
    return sw;
}

//Constructeur pour créer une station
static struct station *make_station(uint64_t mac, uint32_t ip) {
    struct station *st = calloc(1, sizeof(struct station));
    st->mac = mac;
    st->ip = ip;
    st->p = NULL;
    return st;
}

static int add_port_to_switch(struct switch_t *sw, int target_id, uint8_t cost, enum device_type ntype, void *neighbor) {
    int idx = sw->nbPorts; 
    struct port *p = calloc(1, sizeof(struct port));
    
    p->num = (uint8_t)idx; // idx = sw->nbPorts avant l'incrémentation = 0, 1, 2... et pour l'affichage on utilise target_id, mais que pour l'affichage...
    p->cost = cost;
    p->status = DEFAULT; 
    p->type = ntype;     
    
    // Correction de l'accès aux champs de l'union
    if (ntype == SWITCH) {
        p->equipment.switch_t = (struct switch_t *)neighbor; 
    } else {
        p->equipment.station = (struct station *)neighbor;
    }
    
    sw->ports[idx] = p;
    sw->nbPorts++;
    return idx;
}

void ReadFile(const char *filepath, struct network *net) {
    FILE *f = fopen(filepath, "r");
    if (!f) {
        printf("Erreur : impossible d'ouvrir '%s'\n", filepath);
        return;
    }

    net->nb_stations = 0;
    net->nb_switchs = 0;
    net->nbLiens = 0;

    char line[256];
    int nb_eq = 0, nb_liens = 0;

    if (fgets(line, sizeof(line), f) != NULL) {
        sscanf(line, "%d %d", &nb_eq, &nb_liens);
        net->nbLiens = nb_liens;
        saved_nb_eq = nb_eq;
    }

    void *eq_ptr[MAX_DEVICES];
    enum device_type eq_type[MAX_DEVICES];
    memset(eq_ptr, 0, sizeof(eq_ptr));

    for (int i = 0; i < nb_eq; i++) {
        if (fgets(line, sizeof(line), f) != NULL) {
            int type = 0;
            sscanf(line, "%d", &type);
            
            char mac_s[32] = {0}, ip_s[32] = {0};

            if (type == 2) { 
                int nb_ports_physiques = 0, prio = 0;
                //"id;mac;nb_ports;priorité" — * ignore l'id, lit au max 31 caractères jusqu'au prochain ; (mac), nb_ports et priorité
                sscanf(line, "%*d;%31[^;];%d;%d", mac_s, &nb_ports_physiques, &prio);
                struct switch_t *sw = make_switch(convert_mac(mac_s), (uint16_t)prio);
                net->switchs[net->nb_switchs++] = sw;
                eq_ptr[i] = sw;
                eq_type[i] = SWITCH;
            } else if (type == 1) { 
                //"id;mac;ip" — ignore l'id, lit la mac (31 caractères max) jusqu'au prochain ;, et l'ip jusqu'à fin de ligne
                sscanf(line, "%*d;%31[^;];%31[^\n]", mac_s, ip_s);
                struct station *st = make_station(convert_mac(mac_s), convert_ip(ip_s));
                net->stations[net->nb_stations++] = st;
                eq_ptr[i] = st;
                eq_type[i] = STATION;
            }
        }
    }

    if (saved_raw_links != NULL) free(saved_raw_links);
    saved_raw_links = malloc(nb_liens * sizeof(struct raw_link));
    
    for (int i = 0; i < nb_liens; i++) {
        if (fgets(line, sizeof(line), f) != NULL) {
            int id1 = 0, id2 = 0, cout = 0;
            sscanf(line, "%d;%d;%d", &id1, &id2, &cout);
            
            saved_raw_links[i].id1 = id1;
            saved_raw_links[i].id2 = id2;
            saved_raw_links[i].cost = cout;

            if (eq_type[id1] == SWITCH) {
                add_port_to_switch((struct switch_t *)eq_ptr[id1], id2, (uint8_t)cout, eq_type[id2], eq_ptr[id2]);
            } else if (eq_type[id1] == STATION) {
                struct port *p_st = calloc(1, sizeof(struct port));
                p_st->num = id2; p_st->cost = cout; p_st->status = DEFAULT; p_st->type = SWITCH;
                p_st->equipment.switch_t = (struct switch_t *)eq_ptr[id2];
                ((struct station *)eq_ptr[id1])->p = p_st;
            }

            if (eq_type[id2] == SWITCH) {
                add_port_to_switch((struct switch_t *)eq_ptr[id2], id1, (uint8_t)cout, eq_type[id1], eq_ptr[id1]);
            } else if (eq_type[id2] == STATION) {
                struct port *p_st = calloc(1, sizeof(struct port));
                p_st->num = id1; p_st->cost = cout; p_st->status = DEFAULT; p_st->type = SWITCH;
                p_st->equipment.switch_t = (struct switch_t *)eq_ptr[id1];
                ((struct station *)eq_ptr[id2])->p = p_st;
            }
        }
    }
    fclose(f);
}

void print_network(struct network *net) {
    if (net == NULL) return;

    printf("================ EN-TÊTE ================\n");
    printf("Nombre d'equipements attendus : %d\n", saved_nb_eq);
    printf("Nombre de liens attendus : %zu\n\n", net->nbLiens);

    printf("============== ÉQUIPEMENTS ==============\n");
    for (size_t i = 0; i < net->nb_switchs; i++) {
        struct switch_t *sw = net->switchs[i];
        printf("[ID %zu] SWITCH  -> MAC : ", i);
        display_mac(sw->mac);
        printf(" | Ports connectés: %d | Priorite: %d\n", sw->nbPorts, sw->priority);
        for(int p = 0; p < sw->nbPorts; p++) {
            if(sw->ports[p] != NULL) {
                printf("             -> Connecté à ID %d (coût: %d)\n", sw->ports[p]->num, sw->ports[p]->cost);
            }
        }
        printf("\n");
    }
    for (size_t i = 0; i < net->nb_stations; i++) {
        struct station *st = net->stations[i];
        printf("STATION -> MAC : ");
        display_mac(st->mac);
        printf("\n        -> IP  : ");
        display_ip(st->ip);
        if(st->p != NULL) {
            printf("\n        -> Connectée à ID %d (coût: %d)", st->p->num, st->p->cost);
        }
        printf("\n\n");
    }

    if (saved_raw_links != NULL) {
        printf("================= LIENS =================\n");
        for (size_t i = 0; i < net->nbLiens; i++) {
            printf("Lien n°%zu : Equipement %d <---> Equipement %d (Cout: %d)\n", i + 1, saved_raw_links[i].id1, saved_raw_links[i].id2, saved_raw_links[i].cost);
        }
    }
}

void free_network(struct network *net) {
    if (net == NULL) return;

    for (size_t i = 0; i < net->nb_switchs; i++) {
        struct switch_t *sw = net->switchs[i];
        for (int p = 0; p < sw->nbPorts; p++) {
            if (sw->ports[p] != NULL) free(sw->ports[p]);
        }
        
        // Libération de la mémoire oubliée
        if (sw->bpdu != NULL) free(sw->bpdu);
        for(int j = 0; j < 32; j++) {
            if(sw->tableCommutation[j] != NULL) free(sw->tableCommutation[j]);
        }
        
        free(sw);
    }
    
    for (size_t i = 0; i < net->nb_stations; i++) {
        struct station *st = net->stations[i];
        if (st->p != NULL) free(st->p);
        free(st);
    }

    if (saved_raw_links != NULL) {
        free(saved_raw_links);
        saved_raw_links = NULL;
    }
}
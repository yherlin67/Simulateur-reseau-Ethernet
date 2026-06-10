#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h> 

#include "network.h"
#include "utils.h"
#include "switch.h" 
#include "station.h"



//stocke les liens pour plus tard
static struct raw_link *saved_raw_links = NULL;
static int saved_nb_eq = 0;

// pour l'affichage dans switch.c (pour avoir le numéro de la station et pas afficher la MAC)
int get_station_id_by_mac(struct network *net, uint64_t mac)
{
    for (size_t s = 0; s < net->nb_stations; s++) {
        if (net->stations[s]->mac == mac) return (int)s + 1;
    }
    return -1;
}

static struct switch_t *make_switch(uint64_t mac, uint16_t priority) {
    struct switch_t *sw = calloc(1, sizeof(struct switch_t));
    if(!sw) {
        perror("calloc switch");
        return NULL;
    }

    sw->mac = mac;
    sw->priority = priority;
    sw->nbPorts = 0;
    
    sw->bpdu = calloc(1, sizeof(struct BPDU)); 
    if(!sw->bpdu) {
        perror("calloc bpdu");
        free(sw);
        return NULL;
    }
    sw->bpdu->root = mac;
    sw->bpdu->cost = 0;
    sw->bpdu->bridge_id = mac;
    sw->bpdu->num_port = 0;
    sw->visited = false;
    
    return sw;
}

static struct station *make_station(uint64_t mac, uint32_t ip) {
    struct station *st = calloc(1, sizeof(struct station));
    if(!st) {
        perror("calloc station");
        return NULL;
    }
    st->mac = mac;
    st->ip = ip;
    st->p = NULL;
    return st;
}

static int add_port_to_switch(struct switch_t *sw, uint8_t cost, enum device_type ntype, void *neighbor) {
    int idx = sw->nbPorts; 
    struct port *p = calloc(1, sizeof(struct port));
    if(!p)
    {
        perror("calloc port");
        return -1;
    }
    
    p->num = (uint8_t)idx; // idx = sw->nbPorts avant l'incrémentation = 0, 1, 2... et pour l'affichage on utilise target_id, mais que pour l'affichage...
    p->num_voisin = 0;
    p->cost = cost;
    p->status = DEFAULT; 
    p->type = ntype;     
    
    if (ntype == SWITCH) {
        p->equipment.switch_t = (struct switch_t *)neighbor; 
    } else {
        p->equipment.station = (struct station *)neighbor;
    }
    
    sw->ports[idx] = p;
    sw->nbPorts++;
    return idx;
}

int ReadFile(const char *filepath, struct network *net) {
    FILE *f = fopen(filepath, "r");
    if (!f) {
        perror(filepath);
        return EXIT_FAILURE;
    }

    net->nb_stations = 0;
    net->nb_switchs = 0;
    net->nbLiens = 0;

    char line[256];
    int nb_eq = 0, nb_liens = 0;

    if (fgets(line, sizeof(line), f) != NULL) {
        if(sscanf(line, "%d %d", &nb_eq, &nb_liens) != 2)
        {
            fprintf(stderr, "Erreur : première ligne mal formatée : %s\n", line);
            fclose(f);
            return EXIT_FAILURE;
        }
        net->nbLiens = nb_liens;
        saved_nb_eq = nb_eq;
    }
    else
    {
        fprintf(stderr, "Erreur : lecture du fichier impossible\n");
        fclose(f);
        return EXIT_FAILURE;
    }

    void *eq_ptr[MAX_DEVICES];
    enum device_type eq_type[MAX_DEVICES];
    int eq_true_id[MAX_DEVICES];
    
    memset(eq_ptr, 0, sizeof(eq_ptr));
    memset(eq_true_id, 0, sizeof(eq_true_id));

    for (int i = 0; i < nb_eq; i++) {
        if (fgets(line, sizeof(line), f) != NULL) {
            int type = 0;
            sscanf(line, "%d", &type);
            
            char mac_s[32] = {0}, ip_s[32] = {0};

            if (type == 2) 
            { 

                int nb_ports_physiques = 0, prio = 0;
                //"id;mac;nb_ports;priorité" — * ignore l'id, lit au max 31 caractères jusqu'au prochain ; (mac), nb_ports et priorité
                int n = sscanf(line, "%*d;%31[^;];%d;%d", mac_s, &nb_ports_physiques, &prio);
                
                 //n représente le nombre de champs correctement lus
                if(n != 3)
                {
                    fprintf(stderr, "Erreur : ligne mal formatée : %s\n", line);
                    fclose(f);
                    return EXIT_FAILURE;
                }
                // Vérif sur le nombre de ports
                if(nb_ports_physiques == 0 || nb_ports_physiques > MAX_PORTS)
                {
                    fprintf(stderr, "Erreur : nombre de ports invalide : %d\n", nb_ports_physiques);
                    fclose(f);
                    return EXIT_FAILURE;
                }
                //Vérif sur l'addresse MAC
                unsigned int m1, m2, m3, m4, m5, m6;
                if(strlen(mac_s) != 17 || 
                sscanf(mac_s, "%x:%x:%x:%x:%x:%x", &m1, &m2, &m3, &m4, &m5, &m6) != 6 ||
                m1 > 255 || m2 > 255 || m3 > 255 || m4 > 255 || m5 > 255 || m6 > 255)
                {
                    fprintf(stderr, "Erreur : MAC invalide : %s\n", mac_s);
                    fclose(f);
                    return EXIT_FAILURE;
                }

                struct switch_t *sw = make_switch(convert_mac(mac_s), (uint16_t)prio);

                //On vérif si le switch a bien été créé
                if(!sw) 
                {
                    free_network(net);
                    fclose(f);
                    return EXIT_FAILURE;
                }
                eq_true_id[i] = net->nb_switchs;
                
                //stock PUIS incrémente : deux opérations en une ligne
                net->switchs[net->nb_switchs++] = sw;

                eq_ptr[i] = sw;
                eq_type[i] = SWITCH;
            } 
            else if (type == 1) 
            { 
                unsigned int i1, i2, i3, i4;
                //"id;mac;ip" — ignore l'id, lit la mac (31 caractères max) jusqu'au prochain ;, et l'ip = uint séparés par des points
                int n = sscanf(line, "%*d;%31[^;];%u.%u.%u.%u", mac_s, &i1, &i2, &i3, &i4);
                
                //n représente le nombre de champs correctement lus
                //On vérifie si aucun champ de l'adresse IP n'est supérieur à la valeur stockable sur un octet
                if(n != 5 || i1 > 255 || i2 > 255 || i3 > 255 || i4 > 255)
                {
                    fprintf(stderr, "Erreur : IP invalide : %s\n", line);
                    fclose(f);
                    return EXIT_FAILURE;
                }

                //Construction de l'adresse IP
                snprintf(ip_s, sizeof(ip_s), "%u.%u.%u.%u", i1, i2, i3, i4);

                //Vérif sur l'addresse MAC
                unsigned int m1, m2, m3, m4, m5, m6;
                if(strlen(mac_s) != 17 || 
                sscanf(mac_s, "%x:%x:%x:%x:%x:%x", &m1, &m2, &m3, &m4, &m5, &m6) != 6 ||
                m1 > 255 || m2 > 255 || m3 > 255 || m4 > 255 || m5 > 255 || m6 > 255)
                {
                    fprintf(stderr, "Erreur : MAC invalide : %s\n", mac_s);
                    fclose(f);
                    return EXIT_FAILURE;
                }

                struct station *st = make_station(convert_mac(mac_s), convert_ip(ip_s));

                //On vérif si la station a bien été créée
                if(!st) 
                {
                    free_network(net);
                    fclose(f);
                    return EXIT_FAILURE;
                }
                eq_true_id[i] = net->nb_stations;

                //stock PUIS incrémente : deux opérations en une ligne
                net->stations[net->nb_stations++] = st;
                eq_ptr[i] = st;
                eq_type[i] = STATION;
            }
            else
            {
                fprintf(stderr, "Erreur : Les équipements ne peuvent être que de type switch (2) ou station (1)\n");
                fclose(f);
                return EXIT_FAILURE;
            }
        }
    }

    if (saved_raw_links != NULL){
        free(saved_raw_links);
    }
    saved_raw_links = malloc(nb_liens * sizeof(struct raw_link));
    if(!saved_raw_links)
    {
        perror("malloc saved_raw_links");
        free_network(net);
        fclose(f);
        return EXIT_FAILURE;
    }
    
    for (int i = 0; i < nb_liens; i++) {
        if (fgets(line, sizeof(line), f) != NULL) {
            int id1 = 0, id2 = 0, cost = 0;

            int n = sscanf(line, "%d;%d;%d", &id1, &id2, &cost);
            if(n != 3)
            {
                fprintf(stderr, "Erreur : ligne mal formatée : %s\n", line);
                fclose(f);
                return EXIT_FAILURE;
            }
            //Vérif sur les id des machines
            if(id1 < 0 || id1 >= nb_eq || id2 < 0 || id2 >= nb_eq)
            {
                fprintf(stderr, "Erreur : ID invalide dans lien : %d %d\n", id1, id2);
                fclose(f);
                return EXIT_FAILURE;
            }
            //Vérif sur le coût
            if(cost != 0 && cost != 4 && cost != 19 && cost !=100)
            {
                fprintf(stderr, "Erreur : coût invalide dans lien : %d %d\n", id1, id2);
                fclose(f);
                return EXIT_FAILURE;
            }
            
            saved_raw_links[i].type1 = eq_type[id1];
            saved_raw_links[i].true_id1 = eq_true_id[id1];
            saved_raw_links[i].type2 = eq_type[id2];
            saved_raw_links[i].true_id2 = eq_true_id[id2];
            saved_raw_links[i].cost = cost;

            // idx1 et idx2 = index des ports créés de chaque côté du lien
            int idx1 = -1, idx2 = -1;

            // === CÔTÉ id1 ===
            if (eq_type[id1] == SWITCH)
            {
                idx1 = add_port_to_switch((struct switch_t *)eq_ptr[id1], (uint8_t)cost, eq_type[id2], eq_ptr[id2]);
                if(idx1 == -1)
                {
                    free_network(net);
                    fclose(f);
                    return EXIT_FAILURE;
                }
            }
            else
            {
                struct port *p_st = calloc(1, sizeof(struct port));
                if(!p_st)
                {
                    perror("calloc port");
                    free_network(net);
                    fclose(f);
                    return EXIT_FAILURE;
                }
                p_st->num = 0;
                p_st->num_voisin = 0; // sera rempli après
                p_st->cost = (uint8_t)cost;
                p_st->status = DEFAULT;
                p_st->type = SWITCH;
                p_st->equipment.switch_t = (struct switch_t *)eq_ptr[id2];
                ((struct station *)eq_ptr[id1])->p = p_st;
                idx1 = 0;
            }

            // CÔTÉ id2
            if (eq_type[id2] == SWITCH)
            {
                idx2 = add_port_to_switch((struct switch_t *)eq_ptr[id2], (uint8_t)cost, eq_type[id1], eq_ptr[id1]);
                if(idx2 == -1)
                {
                    free_network(net);
                    fclose(f);
                    return EXIT_FAILURE;
                }
            }
            else
            {
                struct port *p_st = calloc(1, sizeof(struct port));
                if(!p_st)
                {
                    perror("calloc port");
                    free_network(net);
                    fclose(f);
                    return EXIT_FAILURE;
                }
                p_st->num = 0;
                p_st->num_voisin = 0; // sera rempli après
                p_st->cost = (uint8_t)cost;
                p_st->status = DEFAULT;
                p_st->type = SWITCH;
                p_st->equipment.switch_t = (struct switch_t *)eq_ptr[id1];
                ((struct station *)eq_ptr[id2])->p = p_st;
                idx2 = 0;
            }

            // RELIER LES NUM_VOISIN 
            if (eq_type[id1] == SWITCH)
            {
                // on récupère le switch id1
                struct switch_t *sw1 = (struct switch_t *)eq_ptr[id1];

                // on récupère le port qu'on vient de créer sur ce switch
                struct port *port_de_sw1 = sw1->ports[idx1];

                // on lui dit que son homologue chez le voisin est à l'index idx2
                port_de_sw1->num_voisin = (uint8_t)idx2;
            }
            else
            {
                // id1 est une station, son unique port pointe vers le port idx2 du voisin
                struct station *st1 = (struct station *)eq_ptr[id1];
                st1->p->num_voisin = (uint8_t)idx2;
            }

            if (eq_type[id2] == SWITCH)
            {
                // on récupère le switch id2
                struct switch_t *sw2 = (struct switch_t *)eq_ptr[id2];
                // on récupère le port qu'on vient de créer sur ce switch
                struct port *port_de_sw2 = sw2->ports[idx2];
                // on lui dit que son homologue chez le voisin est à l'index idx1
                port_de_sw2->num_voisin = (uint8_t)idx1;
            }
            else
            {
                // id2 est une station, son unique port pointe vers le port idx1 du voisin
                struct station *st2 = (struct station *)eq_ptr[id2];
                st2->p->num_voisin = (uint8_t)idx1;
            }
        }
    }
    fclose(f);
    return EXIT_SUCCESS;
}

void print_network(struct network *net) {
    if (net == NULL) return;

    printf("================ EN-TÊTE ================\n\n");
    printf("Nombre d'equipements : %d\n", saved_nb_eq);
    printf("Nombre de liens : %zu\n\n", net->nbLiens);

    printf("============== ÉQUIPEMENTS ==============\n\n");
    
    for (size_t i = 0; i < net->nb_switchs; i++) {
        struct switch_t *sw = net->switchs[i];
        printf("SWITCH N°%zu -> MAC : ", i+1);
        display_mac(sw->mac);
        printf(" | Ports connectés: %d | Priorite: %d\n", sw->nbPorts, sw->priority);
        
        for(int p = 0; p < sw->nbPorts; p++) {
            if(sw->ports[p] != NULL) {
                if (sw->ports[p]->type == SWITCH) {
                    int true_switch_id = -1;
                    for (size_t s = 0; s < net->nb_switchs; s++) {
                        if (net->switchs[s] == sw->ports[p]->equipment.switch_t) {
                            true_switch_id = (int)s + 1;
                            break;
                        }
                    }
                    printf("           -> Connecté au switch %d (coût: %d)\n", true_switch_id, sw->ports[p]->cost);
                } 
                else if (sw->ports[p]->type == STATION) {
                    int true_station_id = -1;
                    for (size_t s = 0; s < net->nb_stations; s++) {
                        if (net->stations[s] == sw->ports[p]->equipment.station) {
                            true_station_id = (int)s + 1;
                            break;
                        }
                    }
                    printf("           -> Connecté à la station %d (coût: %d)\n", true_station_id, sw->ports[p]->cost);
                }
            }
        }
        printf("\n");
    }
    
    for (size_t i = 0; i < net->nb_stations; i++) {
        struct station *st = net->stations[i];
        printf("STATION N°%zu -> MAC : ", i+1);
        display_mac(st->mac);
        printf("\n             -> IP  : ");
        display_ip(st->ip);
        
        if(st->p != NULL) {
            if (st->p->type == SWITCH) {
                int true_switch_id = -1;
                for (size_t s = 0; s < net->nb_switchs; s++) {
                    if (net->switchs[s] == st->p->equipment.switch_t) {
                        true_switch_id = (int)s + 1;
                        break;
                    }
                }
                printf("\n             -> Connectée au switch %d (coût: %d)", true_switch_id, st->p->cost);
            } 
            else {
                int true_station_id = -1;
                for (size_t s = 0; s < net->nb_stations; s++) {
                    if (net->stations[s] == st->p->equipment.station) {
                        true_station_id = (int)s + 1;
                        break;
                    }
                }
                printf("\n             -> Connectée à la station %d (coût: %d)", true_station_id, st->p->cost);
            }
        }
        printf("\n\n");
    }

    if (saved_raw_links != NULL) {
        printf("================= LIENS =================\n\n");
        for (size_t i = 0; i < net->nbLiens; i++) {
            printf("Lien n°%zu : ", i + 1);
            
            if (saved_raw_links[i].type1 == SWITCH) {
                printf("Switch %d", saved_raw_links[i].true_id1 + 1);
            } else {
                printf("Station %d", saved_raw_links[i].true_id1 + 1);
            }

            printf(" <---> ");

            if (saved_raw_links[i].type2 == SWITCH) {
                printf("Switch %d", saved_raw_links[i].true_id2 + 1);
            } else {
                printf("Station %d", saved_raw_links[i].true_id2 + 1);
            }

            printf(" (Cout: %d)\n", saved_raw_links[i].cost);
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
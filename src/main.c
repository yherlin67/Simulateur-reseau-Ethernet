#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "network.h"

uint32_t convertirIP(const char *ipStr) {
    unsigned int octet1, octet2, octet3, octet4;
    
    if (sscanf(ipStr, "%u.%u.%u.%u", &octet1, &octet2, &octet3, &octet4) != 4) {
        return 0;
    }

    uint32_t ip_bin = ((uint32_t)octet1 << 24) | 
                      ((uint32_t)octet2 << 16) | 
                      ((uint32_t)octet3 << 8)  | 
                      octet4;
    
    return ip_bin;
}

uint64_t convertirMAC(const char *macStr) {
    unsigned int m1, m2, m3, m4, m5, m6;
    
    if (sscanf(macStr, "%x:%x:%x:%x:%x:%x", &m1, &m2, &m3, &m4, &m5, &m6) != 6) {
        return 0;
    }

    uint64_t mac_bin = ((uint64_t)m1 << 40) | 
                       ((uint64_t)m2 << 32) | 
                       ((uint64_t)m3 << 24) | 
                       ((uint64_t)m4 << 16) | 
                       ((uint64_t)m5 << 8)  | 
                       m6;
    
    return mac_bin;
}

void afficherIPBinaire(uint32_t ip) {
    for (int i = 0; i < 32; i++) {
        uint32_t bit = (ip >> (31 - i)) & 1;
        printf("%u", bit);
    }
}

void afficherMACBinaire(uint64_t mac) {
    for (int i = 0; i < 48; i++) {
        uint64_t bit = (mac >> (47 - i)) & 1;
        printf("%u", (unsigned int)bit);
    }
}


void lireFichierConfiguration(const char *cheminFichier, struct reseau *res) {
    FILE *fichier = fopen(cheminFichier, "r");
    
    if (fichier == NULL) {
        printf("Erreur : Impossible d'ouvrir le fichier '%s'.\n", cheminFichier);
        return;
    }
    char ligne[256];

    int nbEquipements = 0;
    int nbLiens = 0;

    if (fgets(ligne, sizeof(ligne), fichier) != NULL) {
        sscanf(ligne, "%d %d", &nbEquipements, &nbLiens);
    }
    printf("================ EN-TÊTE ================\n");
    printf("Nombre d'equipements attendus : %d\n", nbEquipements);
    printf("Nombre de liens attendus       : %d\n\n", nbLiens);
    res->nbLiens = nbLiens;
    res->liens = malloc(nbLiens * sizeof(struct lien*));

    printf("============== ÉQUIPEMENTS ==============\n");
    for (int i = 0; i < nbEquipements; i++) {
        if (fgets(ligne, sizeof(ligne), fichier) != NULL) {
            int typeEquipement = 0;
            sscanf(ligne, "%d", &typeEquipement);
            
            char macStr[32] = {0};
            char ipStr[32] = {0};

            if (typeEquipement == 2) { // Cas switch
                uint64_t adresseMac = 0;
                int nbPorts = 0;
                int priorite = 0;

                sscanf(ligne, "%*d;%[^;];%d;%d", macStr, &nbPorts, &priorite);
                adresseMac = convertirMAC(macStr);

                printf("[ID %d] SWITCH  -> MAC Brut: %s\n", i, macStr);
                printf("             -> MAC Bin : ");
                afficherMACBinaire(adresseMac);
                printf(" | Ports: %d | Priorite: %d\n\n", nbPorts, priorite);
            } 
            else if (typeEquipement == 1) { // Cas station
                uint64_t adresseMac = 0;
                uint32_t adresseIp = 0;

                sscanf(ligne, "%*d;%[^;];%[^;\n]", macStr, ipStr);
                adresseMac = convertirMAC(macStr);
                adresseIp = convertirIP(ipStr);

                printf("[ID %d] STATION -> MAC Brut: %s | IP Brute: %s\n", i, macStr, ipStr);
                printf("             -> MAC Bin : ");
                afficherMACBinaire(adresseMac);
                printf("\n             -> IP Bin  : ");
                afficherIPBinaire(adresseIp);
                printf("\n\n");
            }
        }
    }

    printf("================= LIENS =================\n");
    for (int i = 0; i < nbLiens; i++) {
        if (fgets(ligne, sizeof(ligne), fichier) != NULL) {
            int idEquipement1 = 0;
            int idEquipement2 = 0;
            int coutLien = 0;

            sscanf(ligne, "%d;%d;%d", &idEquipement1, &idEquipement2, &coutLien);
            printf("Lien n°%d : Equipement %d <---> Equipement %d (Cout: %d)\n", i + 1, idEquipement1, idEquipement2, coutLien);
            res->liens[i] = malloc(sizeof(struct lien));
            if (res->liens[i] == NULL) {
                perror("Erreur allocation structure lien");
                return; 
            }

            res->liens[i]->portA = malloc(sizeof(struct port));
            res->liens[i]->portB = malloc(sizeof(struct port));

            res->liens[i]->cost = (uint8_t)coutLien;
            
            res->liens[i]->portA->num = (uint8_t)idEquipement1;
            res->liens[i]->portA->s = DEFAULT;
            res->liens[i]->portA->r = MODE_DEFAULT;

            res->liens[i]->portB->num = (uint8_t)idEquipement2;
            res->liens[i]->portB->s = DEFAULT;
            res->liens[i]->portB->r = MODE_DEFAULT;
        }
    }

    fclose(fichier);
}

int main() {
    const char *chemin = "mylan_no_cycle.txt";
    struct reseau res;

    res.liens = NULL;
    res.nbLiens = 0;

    lireFichierConfiguration(chemin, &res);

    for(size_t i = 0; i < res.nbLiens; i++) {
        free(res.liens[i]->portA);
        free(res.liens[i]->portB);
        free(res.liens[i]);
    }
    free(res.liens);
    
    return 0;
}
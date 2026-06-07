#include "network.h"

void ReadFile(const char *pathFile, struct network *res) {
    FILE *file = fopen(pathFile, "r");
    
    if (file == NULL) {
        printf("Erreur : Impossible d'ouvrir le fichier '%s'.\n", pathFile);
        return;
    }
    char line[256];

    int equipments = 0;
    int nbLiens = 0;

    if (fgets(line, sizeof(line), file) != NULL) {
        sscanf(line, "%d %d", &equipments, &nbLiens);
    }
    printf("================ EN-TÊTE ================\n");
    printf("Nombre d'equipements attendus : %d\n", equipments);
    printf("Nombre de liens attendus       : %d\n\n", nbLiens);
    res->nbLiens = nbLiens;
    res->liens = malloc(nbLiens * sizeof(struct lien*));

    printf("============== ÉQUIPEMENTS ==============\n");
    for (int i = 0; i < equipments; i++) {
        if (fgets(line, sizeof(line), file) != NULL) {
            int typeEquipement = 0;
            sscanf(line, "%d", &typeEquipement);
            
            char macStr[32] = {0};
            char ipStr[32] = {0};

            if (typeEquipement == 2) { // Cas switch
                uint64_t adresseMac = 0;
                int nbPorts = 0;
                int priorite = 0;

                sscanf(line, "%*d;%[^;];%d;%d", macStr, &nbPorts, &priorite);
                adresseMac = convert_mac(macStr);

                printf("[ID %d] SWITCH  -> MAC Brut: %s\n", i, macStr);
                printf("             -> MAC Bin : ");
                display_binairy_mac(adresseMac);
                printf(" | Ports: %d | Priorite: %d\n\n", nbPorts, priorite);
            } 
            else if (typeEquipement == 1) { // Cas station
                uint64_t adresseMac = 0;
                uint32_t adresseIp = 0;

                sscanf(line, "%*d;%[^;];%[^;\n]", macStr, ipStr);
                adresseMac = convert_mac(macStr);
                adresseIp = convert_ip(ipStr);

                printf("[ID %d] STATION -> MAC Brut: %s | IP Brute: %s\n", i, macStr, ipStr);
                printf("             -> MAC Bin : ");
                display_binary_mac(adresseMac);
                printf("\n             -> IP Bin  : ");
                display_binary_ip(adresseIp);
                printf("\n\n");
            }
        }
    }

    printf("================= LIENS =================\n");
    for (int i = 0; i < nbLiens; i++) {
        if (fgets(line, sizeof(line), file) != NULL) {
            int idEquipement1 = 0;
            int idEquipement2 = 0;
            int coutLien = 0;

            sscanf(line, "%d;%d;%d", &idEquipement1, &idEquipement2, &coutLien);
            printf("Lien n°%d : Equipement %d <---> Equipement %d (Cout: %d)\n", i + 1, idEquipement1, idEquipement2, coutLien);
            res->link[i] = malloc(sizeof(struct link));

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

    fclose(file);
}

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdint.h>

//créer méthode convertir ip en binaire et MAC en binaire (pas de . ou de ; entre)

//uint32_t convertirIP(uint32_t ip)
//{
//    unsigned int octet1, octet2, octet3, octet4;
//    
//    if (sscanf(ip, "%u.%u.%u.%u", &octet1, &octet2, &octet3, &octet4) != 4) {
//        return 0;
//    }
//
//    uint32_t ip_bin = (octet1 << 24) | (octet2 << 16) | (octet3 << 8) | octet4;
//    
//    return ip_bin;
//}
//
//uint64_t convertirMAC(uint64_t MAC)
//{
//    return 0;
//}

void lireFichierConfiguration(const char *cheminFichier) {
    FILE *fichier = fopen(cheminFichier, "r");
    
    if (fichier == NULL) {
        printf("Erreur : Impossible d'ouvrir le fichier '%s'.\n", cheminFichier);
        return;
    }
    char ligne[256];

    //Lecture de la première ligne
    int nbEquipements = 0;
    int nbLiens = 0;

    if (fgets(ligne, sizeof(ligne), fichier) != NULL) {
        //La première ligne sépare les deux nombres par un espace
        sscanf(ligne, "%d %d", &nbEquipements, &nbLiens);
    }
    printf("================ EN-TÊTE ================\n");
    printf("Nombre d'equipements attendus : %d\n", nbEquipements);
    printf("Nombre de liens attendus       : %d\n\n", nbLiens);

    //Lecture des équipements
    printf("============== ÉQUIPEMENTS ==============\n");
    for (int i = 0; i < nbEquipements; i++) {
        if (fgets(ligne, sizeof(ligne), fichier) != NULL) {
            int typeEquipement = 0;
            
            sscanf(ligne, "%d", &typeEquipement);
            
            // Chaînes temporaires pour stocker le texte brut du fichier
            char macStr[32] = {0};
            char ipStr[32] = {0};

            if (typeEquipement == 2) { //Cas switch
                uint64_t adresseMac = 0;
                int nbPorts = 0;
                int priorite = 0;

                // 1. On extrait d'abord sous forme de texte (%[^;]) dans macStr
                sscanf(ligne, "%*d;%[^;];%d;%d", macStr, &nbPorts, &priorite);

                // 2. On convertit ce texte en binaire (pense à ajouter ta fonction convertirMAC au-dessus)
               // adresseMac = convertirMAC(macStr);

                // Affichage (on affiche la chaîne lue, et si tu veux voir le binaire en hexadécimal, utilise %lx)
                printf("[ID %d] SWITCH  -> MAC: %s | Ports: %d | Priorite: %d\n", 
                       i, adresseMac, nbPorts, priorite);
            } 
            else if (typeEquipement == 1) { //Cas station
                uint64_t adresseMac = 0;
                uint32_t adresseIp = 0;

                // 1. On extrait les deux chaînes de caractères
                sscanf(ligne, "%*d;%[^;];%[^;\n]", macStr, ipStr);

                // 2. On convertit les deux en types numériques
               // adresseMac = convertirMAC(macStr);
               // adresseIp = convertirIP(ipStr);

                // Affichage (on affiche l'IP en texte, et si tu veux voir sa valeur numérique : %u)
                printf("[ID %d] STATION -> MAC: %s (0x%lx) | IP: %s\n", 
                       i, adresseMac, ipStr, adresseIp);
            }
        }
    }
    printf("\n");

    //Lecture des liens
    printf("================= LIENS =================\n");
    for (int i = 0; i < nbLiens; i++) {
        if (fgets(ligne, sizeof(ligne), fichier) != NULL) {
            int idEquipement1 = 0;
            int idEquipement2 = 0;
            int coutLien = 0;

            //Format : id1;id2;cout
            sscanf(ligne, "%d;%d;%d", &idEquipement1, &idEquipement2, &coutLien);

            //Stockage temporaire / Affichage
            printf("Lien n°%d : Equipement %d <---> Equipement %d (Cout: %d)\n", i + 1, idEquipement1, idEquipement2, coutLien);
        }
    }

    fclose(fichier);
}

int main() {
    const char *chemin = "mylan_no_cycle.txt";
    lireFichierConfiguration(chemin);
    return 0;
}
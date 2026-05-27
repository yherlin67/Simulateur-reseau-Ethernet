#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

//créer méthode convertir ip en binaire et MAC en binaire (pas de . ou de ; entre)

char convertirIP(char ip)
{
    return 0;
}

char convertirMAC(char MAC)
{
    return 0;
}

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
            
            //On regarde d'abord le premier entier pour savoir si c'est une station (1) ou un switch (2)
            sscanf(ligne, "%d", &typeEquipement);
            if (typeEquipement == 2) { //Cas switch
                char adresseMac[50];
                int nbPorts = 0;
                int priorite = 0;

                //Extraction des données séparées par des points-virgules
                // %*d = lire la valeur mais l'ignorer (ne pas la stocker)   |   %[^;] = lit jusqu'à tomber sur un ;
                sscanf(ligne, "%*d;%[^;];%d;%d", adresseMac, &nbPorts, &priorite);

                //Stockage temporaire / Affichage
                printf("[ID %d] SWITCH  -> MAC: %s | Ports: %d | Priorite: %d\n", i, adresseMac, nbPorts, priorite);
            } 
            else if (typeEquipement == 1) { //Cas station
                char adresseMac[50];
                char adresseIp[50];

                //Extraction des données séparées par des points-virgules
                // %*d = lire la valeur mais l'ignorer (ne pas la stocker)   |   %[^;] = lit jusqu'à tomber sur un ;
                sscanf(ligne, "%*d;%[^;];%[^;\n]", adresseMac, adresseIp);

                //Stockage temporaire / Affichage
                printf("[ID %d] STATION -> MAC: %s | IP: %s\n", i, adresseMac, adresseIp);
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
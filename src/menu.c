#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "network.h"
#include "utils.h"

extern void print_network(struct network *net);

void displayMenu(struct network *net) 
{
    char userCommand = '\0';
    bool stpLance = false;
    int ret; // Pour stocker les retours de system() et annuler les warnings

    while(userCommand != 'q')
    {
        printf("\n--- MENU ---\n");
        printf("\n1 - afficher le réseau");
        
        if(stpLance)
        {
            printf("\n2 - enlever le protocole stp");
        }
        else
        {
            printf("\n2 - lancer le protocole stp");
        }
        printf("\nq - quitter");
        printf("\n\nVotre choix : ");

        if (scanf(" %c", &userCommand) != 1) continue; 
        
        if(userCommand == '1')
        {
            ret = system("clear"); (void)ret;
            printf("-> Affichage du réseau...\n\n");
            
            print_network(net);
            
            char backCommand = '\0';
            printf("\nAppuyez sur 'q' pour quitter l'affichage : ");
            while(backCommand != 'q')
            {
                if (scanf(" %c", &backCommand) != 1) break;
            }
            ret = system("clear"); (void)ret;
            userCommand = '\0';
        }
        else if(userCommand == '2')
        {
            ret = system("clear"); (void)ret;
            if(stpLance)
            {
                printf("-> Arrêt du protocole STP...\n");
                stpLance = false;
            }
            else
            {
                printf("-> Lancement du protocole STP...\n");
                stpLance = true;
            }
        }
        else if(userCommand == 'q')
        {
            printf("-> Fermeture du programme.\n");
        }
        else
        {
            ret = system("clear"); (void)ret;
            printf("-> Commande invalide. Veuillez réessayer.\n");
        }
    }
}
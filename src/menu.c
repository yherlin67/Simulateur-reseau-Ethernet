#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "network.h"
#include "utils.h"
#include "switch.h"
#include "scheduler.h"


extern void print_network(struct network *net);

void displayMenu(struct network *net) 
{
    char userCommand = '\0';
    bool stpIsRunning = false;
    int ret; // Pour stocker les retours de system() et annuler les warnings
    ret = system("clear"); (void)ret;
    while(userCommand != 'q')
    {
        printf("\n--- MENU ---\n");
        printf("\n1 - afficher le réseau");
        
        if(stpIsRunning)
        {
            printf("\n2 - enlever le protocole stp");
        }
        else
        {
            printf("\n2 - lancer le protocole stp");
        }
        printf("\n3 - envoyer une trame");
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
            if(stpIsRunning)
            {
                printf("-> Arrêt du protocole STP...\n");
                stpIsRunning = false;

                disable_stp(net);
            }
            else
            {
                printf("-> Lancement du protocole STP...\n\n");
                stpIsRunning = true;
                
                struct scheduler sched;
                scheduler_init(&sched);
                run_stp(net, &sched);      
                scheduler_clear(&sched);

                print_stp(net);

                char backCommand = '\0';
                printf("\nAppuyez sur 'q' pour quitter l'affichage : ");
                while(backCommand != 'q')
                {
                    if(scanf(" %c", &backCommand) != 1) break;
                }
                ret = system("clear"); (void)ret;
                userCommand = '\0';
            }
        }
        else if(userCommand == '3')
        {
            ret = system("clear"); (void)ret;
            if(net->nb_stations == 0)
            {
                printf("Aucune station présente dans ce réseau.\n\nAppuyez sur 'q' pour quitter : ");
                char backCommand = '\0';
                
                while(backCommand != 'q')
                {
                    if(scanf(" %c", &backCommand) != 1) break;
                }
                ret = system("clear"); (void)ret;
                userCommand = '\0';
            } 
            else
            {
                char backCommand = '\0';
                printf("(Appuyez sur 'q' pour quitter)\nÀ partir de quelle station voulez-vous envoyer ? (ex : 2) : \n");
                
                while(1) 
                {
                    if(scanf(" %c", &backCommand) != 1) break;
                    
                    if (backCommand == 'q') {
                        break; 
                    }
                    
                    int stationID = backCommand - '0'; 
                    
                    if (stationID > 0 && stationID <= net->nb_stations) {
                        break;
                    }
                    printf("Entrée invalide. À partir de quelle station voulez-vous envoyer ? (ex : 2) : \n");
                }
                
                if(backCommand == 'q')
                {
                    ret = system("clear"); (void)ret;
                    userCommand = '\0';
                }
                else
                {
                    printf("(Appuyez sur 'q' pour quitter)\nÀ quelle station voulez-vous envoyer ? (ex : 2) : \n");
                } 
                userCommand = '\0';
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
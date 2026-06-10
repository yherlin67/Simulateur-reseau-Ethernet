#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "menu.h"
#include "network.h"
#include "utils.h"
#include "switch.h"
#include "scheduler.h"

void displayMenu(struct network *net) 
{
    char userCommand = '\0';
    bool stpIsRunning = false;
    int ret;
    
   ret = system("clear"); (void)ret;
    
    while(userCommand != 'q')
    {
        printf("\n================== MENU ==================\n");
        printf("\n1 - Afficher le réseau");
        
        if(stpIsRunning)
        {
            printf("\n2 - Enlever le protocole stp");
            printf("\n3 - Envoyer une trame");
            printf("\n4 - Afficher une table de commutation");
        }
        else
        {
            printf("\n2 - Lancer le protocole stp");
            printf("\n4 - Afficher table de commutation");
        }
        printf("\nq - Quitter");
        printf("\n\nVotre choix : ");

        if (scanf(" %c", &userCommand) != 1) {
            continue;
        }
        
        if(userCommand == '1')
        {
            ret = system("clear"); (void)ret;
            printf("-> Affichage du réseau...\n\n");
            
            print_network(net);
            
            char backCommand = '\0';
            printf("\nAppuyez sur 'q' pour quitter l'affichage : ");
            while(1)
            {
                if(scanf(" %c", &backCommand) != 1) continue;
                if(backCommand == 'q') break;
                printf("Erreur : commande invalide. Appuyez sur 'q' pour quitter : ");
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
            }
            
            char backCommand = '\0';
            printf("\nAppuyez sur 'q' pour quitter l'affichage : ");
            while(1)
            {
                if(scanf(" %c", &backCommand) != 1) continue;
                if(backCommand == 'q') break;
                printf("Erreur : commande invalide. Appuyez sur 'q' pour quitter : ");
            }
            ret = system("clear"); (void)ret;
            userCommand = '\0';
        }
        else if(userCommand == '3' && stpIsRunning)
        {
            char continueAction = 'a';
            
            while (continueAction == 'a') 
            {
                ret = system("clear"); (void)ret;

                bool has_connected_station = false;
                for(size_t k = 0; k < net->nb_stations; k++) {
                    if(net->stations[k]->p != NULL && net->stations[k]->p->type == SWITCH) {
                        has_connected_station = true;
                        break;
                    }
                }

                if(!has_connected_station || net->nb_stations == 0)
                {
                    printf("Aucune station reliée à un switch disponible pour l'envoi.\n");
                    continueAction = 'q'; 
                }
                else
                {
                    printf("Stations disponibles (connectées à un switch) :\n");
                    for(size_t k = 0; k < net->nb_stations; k++)
                    {
                        if(net->stations[k]->p != NULL && net->stations[k]->p->type == SWITCH) {
                            printf("Station n°%zu | MAC : ", k+1);
                            print_mac(net->stations[k]->mac);
                            printf(" | IP : ");
                            print_ip(net->stations[k]->ip);
                            printf("\n");
                        }
                    }

                    int src_idx = -1, dst_idx = -1;
                    
                    printf("\nIndice de la station source : ");
                    if(scanf("%d", &src_idx) != 1) { while(getchar() != '\n'); } 

                    printf("Indice de la station destination : ");
                    if(scanf("%d", &dst_idx) != 1) { while(getchar() != '\n'); } 

                    if(src_idx < 1 || (size_t)src_idx > net->nb_stations || dst_idx < 1 || (size_t)dst_idx > net->nb_stations)
                    {
                        printf("Erreur : Indice de station inexistant.\n");
                    }
                    else if(net->stations[src_idx-1]->p == NULL || net->stations[src_idx-1]->p->type != SWITCH)
                    {
                        printf("Erreur : La station source choisie n'est pas connectée à un switch.\n");
                    }
                    else
                    {
                        char message[256] = {0};
                        printf("Message : ");
                        if(scanf(" %255[^\n]", message) != 1) { message[0] = '\0'; }
                        
                        struct scheduler sched;
                        scheduler_init(&sched);
                        
                        station_send(net->stations[src_idx-1], net->stations[dst_idx-1], message, &sched);
                        
                        scheduler_clear(&sched);
                    }
                }

                if (continueAction != 'q') 
                {
                    printf("\nTapez 'a' pour envoyer une autre trame, ou 'q' pour quitter : ");
                    while(1) {
                        if(scanf(" %c", &continueAction) != 1) continue;
                        if(continueAction == 'a' || continueAction == 'q') break;
                        printf("Erreur : commande non reconnue. Tapez 'a' ou 'q' : ");
                    }
                }
            }
            ret = system("clear"); (void)ret;
            userCommand = '\0';
        }
        else if(userCommand == '4')
        {
            char continueAction = 'a';
            
            while (continueAction == 'a')
            {
                int switchChoice = 0;
                while((size_t)switchChoice > net->nb_switchs || switchChoice < 1)
                {
                    ret = system("clear"); (void)ret;
                    printf("Nombre de switchs présents dans le réseau : %ld\n", net->nb_switchs);
                    printf("Quelle table de quel switch voulez-vous voir ? (1 à %ld) : ", net->nb_switchs);

                    if(scanf("%d", &switchChoice) != 1) {
                        while(getchar() != '\n');
                    }
                } 
                
                ret = system("clear"); (void)ret;
                printf("-> Affichage de la table de commutation...\n\n");

                print_tab_commut(net, switchChoice - 1); 
                
                printf("\nTapez 'a' pour visualiser une autre table, ou 'q' pour quitter : ");
                while(1) 
                {
                    if (scanf(" %c", &continueAction) != 1) continue;
                    if (continueAction == 'a' || continueAction == 'q') break;
                    printf("Erreur : commande non reconnue. Tapez 'a' ou 'q' : ");
                }
            }
            ret = system("clear"); (void)ret;
            userCommand = '\0';
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
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
            printf("\n3 - envoyer une trame");
            printf("\n4 - afficher table de commutation");
        }
        else
        {
            printf("\n2 - lancer le protocole stp");
            printf("\n4 - afficher table de commutation");
        }
        printf("\nq - quitter");
        printf("\n\nVotre choix : ");

        if (scanf(" %c", &userCommand) != 1){
            continue;
        }
        
        if(userCommand == '1')
        {
            ret = system("clear"); (void)ret;
            printf("-> Affichage du réseau...\n\n");
            
            print_network(net);
            
            char backCommand = '\0';
            printf("\nAppuyez sur 'q' pour quitter l'affichage : ");
            while(backCommand != 'q')
            {
                if (scanf(" %c", &backCommand) != 1){
                    break;
                }
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
                printf("Aucune station présente dans ce réseau.\n");
            }
            else
            {
                // afficher les stations disponibles
                printf("Stations disponibles :\n");
                for(size_t k = 0; k < net->nb_stations; k++)
                {
                    printf("  [%zu] ", k);
                    print_mac(net->stations[k]->mac);
                    printf(" | IP : ");
                    print_ip(net->stations[k]->ip);
                    printf("\n");
                }

                // demander la source
                int src_idx = -1;
                printf("\nIndice de la station source : ");
                if(scanf("%d", &src_idx) != 1)
                {
                    src_idx = -1;
                }

                // demander la destination
                int dst_idx = -1;
                printf("Indice de la station destination : ");
                if(scanf("%d", &dst_idx) != 1)
                {
                    dst_idx = -1;
                }

                // vérifier les indices
                if(src_idx < 0 || (size_t)src_idx >= net->nb_stations ||
                dst_idx < 0 || (size_t)dst_idx >= net->nb_stations)
                {
                    printf("Indice invalide.\n");
                }
                else if(net->stations[src_idx]->p == NULL)
                {
                    printf("La station source n'est pas connectée à un switch.\n");
                }
                else
                {
                    // demander le message
                    char message[256] = {0};
                    printf("Message : ");
                    if(scanf(" %255[^\n]", message) != 1)
                    {
                        message[0] = '\0';
                    }
                    // envoyer la trame
                    struct scheduler sched;
                    scheduler_init(&sched);
                    station_send(net->stations[src_idx], net->stations[dst_idx], message, &sched);
                    scheduler_clear(&sched);
                }
            }

            char backCommand = '\0';
            printf("\nAppuyez sur 'q' pour quitter : ");
            while(backCommand != 'q')
            {
                if(scanf(" %c", &backCommand) != 1) break;
            }
            ret = system("clear"); (void)ret;
            userCommand = '\0';
        }
        if(userCommand == '4')
        {
            int backCommand = 0;
            while((size_t)backCommand > net->nb_switchs || (size_t)backCommand < 1)
            {
                ret = system("clear"); (void)ret;
                printf("Quel table de quel switch voulez-vous voir ? (Switch n° ? ) : ");

                scanf("%d", &backCommand);
            } 
            char backcharCommand = '\0';
            ret = system("clear"); (void)ret;
            printf("-> Affichage de la table de commutation...\n\n");

            print_tab_commut(net, backCommand);
            
            printf("\nAppuyez sur 'q' pour quitter l'affichage : ");
            while(backcharCommand != 'q')
            {
                if (scanf(" %c", &backcharCommand) != 1){
                    break;
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
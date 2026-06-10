#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "network.h"
#include "switch.h"

extern void displayMenu(struct network *net);
extern void free_network(struct network *net);

int main(int argc, char *argv[]) {
    const char *chemin;

    if(argc < 2)
    {
        printf("Usage: ./bin/network <fichier>\n");
        return 1;
    }
    
    chemin = argv[1];
    
    struct network res;
    memset(&res, 0, sizeof(res));

    // 1. On charge le réseau
    // n = 1 s'il y a une erreur, n = 0 si tout s'est bien passé
    int n = ReadFile(chemin, &res);
    if(n == 1)
    {
        return 1;
    }


    // 2. On lance le menu en lui donnant accès au réseau chargé
    displayMenu(&res);

    // 3. Libération mémoire à la fermeture du menu
    free_network(&res);

    return 0;
}
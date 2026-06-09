#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "network.h"
#include "switch.h"

extern void displayMenu(struct network *net);
extern void free_network(struct network *net);

int main() {
    const char *chemin = "t_cycle.txt";
    
    struct network res;
    memset(&res, 0, sizeof(res));

    // 1. On charge le réseau
    ReadFile(chemin, &res);

    // 2. On lance le menu en lui donnant accès au réseau chargé
    displayMenu(&res);

    // 3. Libération mémoire à la fermeture du menu
    free_network(&res);

    return 0;
}
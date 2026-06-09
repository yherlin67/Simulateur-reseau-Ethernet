#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "network.h"
#include "switch.h"

void sendFrame(char message) {
    // À implémenter
}

int main() {
    const char *chemin = "mylan_no_cycle.txt";
    
    struct network res;

    // 1. On charge le réseau
    ReadFile(chemin, &res);

    // 2. On lance le menu en lui donnant accès au réseau chargé
    extern void displayMenu(struct network *net);
    displayMenu(&res);

    // 3. Libération mémoire à la fermeture du menu
    free_network(&res);

    return 0;
}
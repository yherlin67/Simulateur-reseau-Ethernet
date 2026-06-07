#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "network.h"


void sendFrame(char message)
{

}

int main() {
    const char *chemin = "mylan_no_cycle.txt";
    struct reseau res;

    res.liens = NULL;
    res.nbLiens = 0;

    ReadFile(chemin, &res);

    for(size_t i = 0; i < res.nbLiens; i++) {
        free(res.liens[i]->portA);
        free(res.liens[i]->portB);
        free(res.liens[i]);
    }
    free(res.liens);

    return 0;
}
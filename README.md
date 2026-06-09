# SAE2.3

## affichage.c
ensemble de fonctions utilitaires qui permettent d'afficher nos composants principaux du LAN (switch, stations, adresses ip...)

## 1 j'ai commenté les diff structures pour qu'on s'y retrouve
## 2 j'ai définit la priorité comme un uint16_t parce que 1024 ca ne passait pas en uint8_t et donc la priorité était de 0 (bits de poids faible uniquement pris...)
## 3 j'ai effacé la structure liens dans network car Yann a déjà fait une méthode (statique) dans network.c (saved_raw_links) qui affiche les liens, parce que le cout et le pointeur vers le voisin (donc un lien) est déjà représenté dans struct PORT.
## 4 j'ai modifié le num dans add_port_to_switch parce que avant ca désignait l'ID de la station liée mais maintenant ca doit juste désigner le numéro du port dans le tableau de port du switch qui possède ce port. 
## 5 IDEE pour l'affichage, ce serait bien d'avoir la possibilité de voir aussi les numéros des stations (parce que dans la saction liens on a des ids mais on ne connait pas l'équipement 14) => donc soit on le fait à la volée dans print_network, soit on ajoute un champ id dans struct station et struct switch_t
## 6 stp_running commence à true et passe à false dès que 1 SEUL SWITCH ne se met pas à jour, donc ca casse tout le protocole parce que c'est pas ca qui est censé arreter la boucle.


## autre soucis dans receive_frame : on met à jour le BPDU du switch quand celui qu'il recoit est moins bon que celui qu'il a lui (bpdu_is_better renvoie true si le BPDU du switch (a) est meilleure que celui recu (b)) : donc il faut changer la logique => j'ai inversé les paramètres.

## dans update BPDU il faut incrémenter du cout du port par lequel on a recu le BPDU, donc on regarde le lien entre les 2 switch, et pas avec un 1 !!!! donc on remplace 1 par sw->ports[num_port]->cost;

## normal que le STP ne fonctionnait pas, on instanciait pas le scheduler et on ne faisait pas run le stp... c'est fait dans menu maintenant.
## rajout de run_stp dans switch.h + include scheduler.h dans menu.c

## après une séance de debugging : au début on passait en index de updatebpdu notre propre port sur lequel on a recu le bpdu d'un switch voisin, hors il faut passer le numéro de port de notre switch voisin nous ayant envoyé le BPDU !! (segfault)

## le scheduler prends en param le port destinataire du BPDU pas le port source... c'était pas fait.

## boucle infinie, on ne vérifie pas si le cout sera meilleur après acceptation du bpdu proposé : par exemple si on compare switch 1 et switch 0 sur des ports hypothétiques : Switch 1 se pose la question : "est-ce que mon chemin actuel vers la racine est meilleur, ou est-ce que le chemin en passant par switch 0 est meilleur ?"
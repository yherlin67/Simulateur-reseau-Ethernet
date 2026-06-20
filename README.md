# Simulateur de réseau Ethernet

Projet réalisé en trinôme, dans le cadre de ma première année de BUT informatique.

## Présentation

Ce projet est un simulateur de réseau Ethernet écrit en langage C. 

Il permet de modéliser un réseau composé de stations et de switchs afin d'y envoyer des trames Ethernet et d'y exécuter le protocole Spanning Tree.

## Fonctionnalités

- Chargement d'un réseau depuis un fichier texte (équipements + liens)
- Apprentissage automatique des adresses MAC stations pour les switchs (table de commutation)
- Diffusion de trames unicast et broadcast
- Ordonnanceur de trames
- Distinction Ethernet II / IEEE 802.3
- Échange de BPDU entre switchs jusqu'à convergence et attribution des rôles des ports (protocole STP)
- Menu interactif

## Captures d'écran

<img width="1694" height="438" alt="image" src="https://github.com/user-attachments/assets/a3265369-2394-4687-8166-8fafa462ebfd" />
<img width="1694" height="1207" alt="image" src="https://github.com/user-attachments/assets/b7392151-bc20-4219-a7d3-1955385d7394" />
<img width="1694" height="1207" alt="image" src="https://github.com/user-attachments/assets/f7a9f9ed-c1dd-4018-a580-532ac2407fa1" />


## Structure du projet

```
.
├── include/        # fichiers d'en-tête (.h)
│   ├── station.h
│   ├── switch.h
│   ├── network.h
│   ├── scheduler.h
│   ├── menu.h
│   └── utils.h
├── src/           # fichiers source (.c)
│   ├── main.c
│   ├── station.c
│   ├── switch.c
│   ├── network.c
│   ├── scheduler.c
│   ├── menu.c
│   └── utils.c
├── bin/            # exécutable (généré)
├── build/          # fichiers objets (générés)
├── Makefile
└── README.md
```

## Format du fichier de configuration

```
<nb_equipements> <nb_liens>
2;<mac>;<nb_ports>;<priorité>   # switch (type 2)
1;<mac>;<ip>                    # station (type 1)
<id_eq1>;<id_eq2>;<coût>        # lien
```

Exemple :

```
4 3
2;01:45:23:a6:f7:ab;8;1024
1;54:d6:a6:82:c5:23;130.79.80.21
1;c8:69:72:5e:43:af;130.79.80.27
1;77:ac:d6:82:12:23;130.79.80.42
0;1;4
0;2;19
0;3;4
```

Poids des liens selon le débit : 10 Mb/s = 100, 100 Mb/s = 19, 1 Gb/s = 4.

## Installation

1. Cloner le dépôt :
```bash
   git clone https://github.com/yherlin67/Simulateur-reseau-Ethernet.git
```
2. Complier :
```bash
make
```

3. Exécuter :
```bash
./bin/network <fichier_config>
```

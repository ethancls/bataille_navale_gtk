# Bataille navale contre ordinateur avec interface graphique (gtk) 

## Compilation et lancement

Pour installer gtk, aller sur https://www.gtk.org/docs/installations/ et suivre les instructions selon son OS.
Après installation, dans le dossier source :
```
make
./bataille.exe
```

## Le jeu

Après avoir lancer le jeu depuis le terminal, il faut appuyer sur le bouton Jouer pour lancer une partie. À chacun de nos coups l’adversaire jouera un coup aussi selon une stratégie probabiliste qui dépend de la difficulté sélectionnée, cette dernière peut être choisie en cliquant sur l’un des boutons « Normal » ou « Difficile ». 

Une probabilité est associée à chaque case de la grille joueur, ces probabilités sont mises à jour de la manière suivante :

	- En mode Normal : si la case jouée ne contient pas de bateau, les probabilités sont augmentées de manière aléatoire sur l’ensemble de la grille, sinon, elles sont augmentées uniquement sur la ligne et la colonne de la case.
 
	- En mode Difficile : la méthode est similaire, mais si la case jouée contient un bateau, les probabilité sont augmenté uniquement sur les cases voisines, l’adversaire joue donc d’une manière un peu plus précise.

## Le code
Ce projet est organisé en 5 fichiers : main.c, vue.c, vue.h, ctrl.c, ctrl.h et struct.h. Le fichier main.c contient la fonction principale du déroulement du jeu et l’initialisation des différentes données, les fichiers vue.c et vue.h contiennent les fonctions d’initialisation et de mise à jour de la vue. Les fichiers ctrl.c et ctrl.h contiennent la fonction d’initialisation de la donnée dans laquelle il y a toutes les informations de la partie en cours, et les fonctions d’initialisation et de mise à jour du tableau de probabilité permettant à l’ordinateur de choisir ses coups. Enfin le fichier struct.h contient les définitions des différentes structures data_t et vue utilisée dans le reste du code.

## Évolutions
L’implémentation d’un mode joueur contre joueur en local a été envisagée, et n’aurais pas été compliquée étant donné que toutes les fonctions joueur ont déjà été créés pour le mode joueur contre ordinateur.

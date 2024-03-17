/* jeu : programme test

	Utilisation : 
		lancer 2 instances,
		demander dans l'une a creer une partie,
		et dans l'autre  a rejoindre une partie

	Attention : les sockets ne sont pas automatiquement supprimes lorsque l'on sort du programme, ce qui peut empecher de rejouer (serveur) !
		=> on capture l'evenement CTRL+C pour supprimer les sockets, avant de finalement effectivement quitter

	TODO :
		- tester toutes les fonctions
		- scenarii de test
*/

#include "jeu.h"

#include <stdlib.h>				/* pour EXIT_SUCCESS						*/
#include <stdio.h>				/* pour affichages						*/
#include <string.h>				/* pour manipulation chaines (buffer)	*/
#include <time.h>				/* pour placement aleatoire				*/
 
#include <stdlib.h>				/* pour EXIT_SUCCESS						*/
#include <stdio.h>				/* pour affichages						*/

#include <signal.h>				/* capture signaux						*/

/* __________ Fonctions	*/

/* Lancement d'une partie cliente ou serveur
	Post-conditions (conformement au modele) :
		- succes total (client ou serveur) : jeu en etat JEU_ETAT_PAS_PARTIE_ENCOURS
		- succes constrution serveur mais echec connexion client : jeu en etat JEU_ETAT_ATTENTE_JOUEUR
		- echec total : jeu en etat JEU_ETAT_NONE
		+ des lors qu'une socket est creee, association de la fonction nettoyer_socket() au signal SIGINT
*/
void connecter(jeu* j, struct sigaction* handler_interruption);

/* Choix de poursuivre ou non
	Pre-conditions	: partie en etat JEU_ETAT_PAS_PARTIE_ENCOURS (jeu donc partage)
	Post-conditions (conformement au modele) :
		- si choix de poursuivre : nouvelle partie lancee (joueur reinitialise, j->num_partie incremente, j->etat devenu JEU_ETAT_PLACEMENT)
		- si choix d'arreter : deconnexion (sockets detruites et champs j->pseudo_adversaire, j->statut_joueur, j->etat reinitialises) 
*/
void do_poursuivre(jeu* j);

/* Fonction principal  */
int main(void);

/* __________ Capture signal interruption (pour supprimer socket(s) en cas d'interruption)

	signal capture  :  	SIGINT, qui est envoye lorsque l'on tappe CTRL+C
	action associee :	fonction nettoyer_socket() qui detruit les sockets eventuellement crees avant de sortir du programme

	Pourquoi le faire : sinon les sockets ne sont pas detruites. Par exemple, on ne peut relancer alors le serveur.
	Contrainte : puisqu'on ne peut passer de parametre aux fonctions de rappel, cela oblige a definir des variables globales dans lesquelles memorise alors les clients et serveurs eventuellement crees
*/

/* sockets a supprimer eventuellement */
client* c = NULL;
serveur* s = NULL;

/* fonction faisant appel aux destructeurs client et serveur sur c et s avant de sortir */
void nettoyer_socket(void) {
	printf("\n\n____ TEST 4 : capture CTRL +C\n");

	printf("%s IN (c == %p, s == %p)\n", __func__, (void*)c, (void*)s);

	if (c != NULL)
		client_delete(c);

	if (s != NULL)
		serveur_delete(s);

	printf("%s OUT (c == %p, s == %p)\n", __func__, (void*)c, (void*)s);

	/* on sort du progamme */
	exit(EXIT_FAILURE);
}

/* __________ Routines	*/

/* Lancement d'une partie cliente ou serveur
	Post-conditions (conformement au modele) :
		- succes total (client ou serveur) : jeu en etat JEU_ETAT_PAS_PARTIE_ENCOURS
		- succes constrution serveur mais echec connexion client : jeu en etat JEU_ETAT_ATTENTE_JOUEUR
		- echec total : jeu en etat JEU_ETAT_NONE
		+ des lors qu'une socket est creee, association de la fonction nettoyer_socket() au signal SIGINT
*/
void connecter(jeu* j, struct sigaction* handler_interruption) {
	int saisie;
	char info_hote_adversaire[(NOM_HOTE_TAILLE_MAX >= IP_TAILLE_MAX ? NOM_HOTE_TAILLE_MAX : IP_TAILLE_MAX) +1] = "";

	/* __ choix partie serveur ou cliente */
	saisie = 0;
	while(saisie < 1 || saisie > 2) {
		printf("Taper 1 pour lancer le client, 2 pour lancer le serveur :\t");
		scanf("%d", &saisie);
		printf("\n");
	}

	/* __ partie serveur */
	if (saisie == 2) {
		/* memorisation serveur creee pour traitement suite a capture du signal CTRL +C */
		s = & (j->p.s);	/* pour le handler */

		/* lancement d'une partie serveur */
		jeu_creer_partie(j);

		/* en cas de succes, affichage informations de connexion (pour communication humaine au joueur adverse) & mise en attente connexion d'un joueur adverse */
		if (j->etat == JEU_ETAT_ATTENTE_JOUEUR) {
			printf("Vous pouvez transmettre le nom '%s' ou l'ip '%s' a votre adversaire pour pouvoir jouer.\n", jeu_get_nom_hote(j), jeu_get_ip(j));

			jeu_etre_rejoint_partie(j);
		}
	}

	/* __ partie cliente */
	else if (saisie == 1) {
		/* memorisation client cree pour traitement suite a capture du signal CTRL +C */
		c = & (j->p.c);

		/* choix info a transmettre pour se connecter */
		saisie = 0;
		while(saisie < 1 || saisie > 2) {
			printf("Identification hote joueur distant : taper 1 pour saisir le nom, 2 pour saisir l'ip :\t");
			scanf("%d", &saisie);
			printf("\n");
		}

		/* Connexion par transmission nom		*/
		if (saisie == 1) {
			printf("Saisir nom hote de votre adversaire :\n");
			scanf("%s", info_hote_adversaire);
			jeu_rejoindre_partie_par_nom_hote(j, info_hote_adversaire);
		}
		/* Connexion par transmission ip		*/
		else {
			printf("Saisir adresse ip de votre adversaire :\n");
			scanf("%s", info_hote_adversaire);
			jeu_rejoindre_partie_par_ip(j, info_hote_adversaire);
		}
	}

	/* en cas de socket creee : associer la fonction nettoyer_socket() au signal SIGINT */
	if ((j->etat == JEU_ETAT_ATTENTE_JOUEUR) || (j->etat == JEU_ETAT_PAS_PARTIE_ENCOURS)) {
		handler_interruption->sa_handler =(void (*)(int)) nettoyer_socket;
		sigaction(SIGINT, handler_interruption, NULL);
	}
}

/* Choix de poursuivre ou non
	Pre-conditions	: partie en etat JEU_ETAT_PAS_PARTIE_ENCOURS (jeu donc partage)
	Post-conditions (conformement au modele) :
		- si choix de poursuivre : nouvelle partie lancee (joueur reinitialise, j->num_partie incremente, j->etat devenu JEU_ETAT_PLACEMENT)
		- si choix d'arreter : deconnexion (sockets detruites et champs j->pseudo_adversaire, j->statut_joueur, j->etat reinitialises) 
*/
void do_poursuivre(jeu* j) {
	int saisie =0;

	while((saisie) < 1 || (saisie > 2)) {
		printf("Taper 1 pour poursuivre, 2 pour quitter :\t");
		scanf("%d", &saisie);
	}

	if (saisie == 2) {
		printf("Vous avez choisi d'arreter.\n");
		jeu_se_deconnecter(j);
	}
	else {
		printf("Une nouvelle partie commence... bonne chance !\n");
		jeu_nouvelle_partie(j);
	}
}

/* __________ Fonction principale	*/
int main(void) {
	/* ____ declarations */

	/* variables techniques */
	int nb_it = 0;
	char pseudo[JOUEUR_PSEUDO_TAILLE +1] = "";

	/* structure de jeu */
	jeu* j = NULL;

	/* variables pour specifier les coups a jouer */
	unsigned int x, y;

	/* structure permettant d'attacher une action a un signal */
	struct sigaction handler_interruption;

	printf("\n\n____ TEST 0 : initialisations\n");

	/* ____ initialisations */

	/* construction de la struture jeu & nommage du joueur */
	j = jeu_construire();

	sprintf(pseudo, "My name is %p", (void*)j);
	jeu_set_pseudo(j, pseudo);

	printf("\tAffichage jeu : \t");
	jeu_afficher_stat(j);

	/* initialisation de la graine aleatoire car utilisation de fonctions de jeu faisant appel a fonction 'rand()' */
	srand(time(NULL));

	printf("\n____ TEST 1 : connexion\n");

	/* ____ nouveau jeu (client ou serveur) */
	connecter(j, & handler_interruption);

	printf("\tAffichage jeu : \t");
	jeu_afficher_stat(j);

	/* ____ boucle parties & tours de jeu */
	printf("\n____ TEST 2 : jeu\n");
	getchar();

	while(jeu_est_partage(j) && nb_it <= 1000) {
		nb_it ++;

		printf("__ tour %d :\t", nb_it);
		jeu_afficher_stat(j);

		if(j->etat == JEU_ETAT_PLACEMENT) {
			printf("\tPlacement aleatoire de la flotte\n");
			jeu_placer_flotte(j);
		}
		else if (j->etat == JEU_ETAT_TOUR_JOUEUR) {
			jeu_afficher_bataille(j);
			printf("\tCoup aleatoire joueur\n");
			jeu_set_coup_a_jouer(j, &x, &y);
			jeu_tour_joueur(j, x, y);
		}
		else if (j->etat == JEU_ETAT_TOUR_ADVERSAIRE) {
			printf("\tTour adversaire\n");
			jeu_tour_adversaire(j);
		}
		else if (j->etat == JEU_ETAT_GAGNEE_JOUEUR) {
			jeu_afficher_bataille(j);
			printf("\tBravo '%s' : partie remportee !\n", jeu_get_pseudo_joueur(j));
			/*
				Pb saisie (a corriger) => continuer toujours => on restrient le nombre total d'iterations
				do_poursuivre(j);
			*/
			printf("Une nouvelle partie commence... bonne chance !\n");
			jeu_nouvelle_partie(j);			
		}
		else if (j->etat == JEU_ETAT_GAGNEE_ADVERSAIRE) {
			jeu_afficher_bataille(j);
			printf("\tFlotte coulee : '%s' remporte la partie\n", jeu_get_pseudo_adversaire(j));
			/*
				Pb saisie (a corriger) => continuer toujours => on restrient le nombre total d'iterations
				do_poursuivre(j);
			*/
			printf("Une nouvelle partie commence... bonne chance !\n");
			jeu_nouvelle_partie(j);			
		}
		else if (j->etat == JEU_ETAT_PAS_PARTIE_ENCOURS) {
			/*
				Pb saisie (a corriger) => continuer toujours => on restrient le nombre total d'iterations
				do_poursuivre(j);
			*/
			printf("Une nouvelle partie commence... bonne chance !\n");
			jeu_nouvelle_partie(j);			
		}
		else {
			printf("\tPartie en etat '%s' : pas de traitement associe, sortie de la boucle principale.\n", jeu_get_etat_string(j));
			break;
		}
	}

	/* ____ sortie */
	printf("__ jeu quitte apres %d iterations :\t", nb_it);
	jeu_afficher_stat(j);

	printf("\n\n____ TEST 3 : destruction jeu\n");

	printf("\tdestruction #1\n");
	jeu_detruire(& j);
	printf("\tdestruction #2\n");
	jeu_detruire(& j);

	return EXIT_SUCCESS;
}


#include "global.h"				/* pour la gestion des erreurs et les constantes symboliques liées au dimensionnement des messages échangés */

#include "jeu.h"

#include <stdio.h>				/* pour les messages d'erreur										*/
#include <stdlib.h>				/* pour allocation memoire & constantes EXIT_SUCCESS, EXIT_FAILURE	*/
#include <string.h>				/* pour la manipulation de fichiers									*/
#include <time.h>				/* pour nommage fichier d'echange									*/

#include <errno.h>				/* pour les erreurs systeme											*/

/* /!\ Gestion erreur pour toutes les fonctions :
		Pour toute fonction necessitant une allocation memoire pour la construction des parametres d'appel de fonctions ou commandes,
		sortie du programme en erreur en cas d'echec allocation memoire (un message d'erreur est alors affiche)	/!\
*/

/* ____________________ Declaration fonctions internes */

/*	Initialise les champs j->statut_joueur, j->etat, j->pseudo_adversaire, j->num_partie, j->nb_parties_gagnees
	Fonction appelee d'abord a la construction du jeu, puis chaque fois qu'un joueur se deconnecte
*/
static void jeu_initialiser_partage(jeu* j);

/*	Pre-conditions 	: j adresse d'un jeu partage
	Post-conditions : en cas de succes, chaine indiques par 'message' envoyee a la socket distante, buffer vide
	Valeur retour : nombre d'octets envoyes en cas de succes, -1 en cas d'echec
*/
static int jeu_message_envoyer(jeu* j, char message[]);

/*	Sous-routine jeu_rejoindre_partie_par_nom_hote() et jeu_rejoindre_partie_par_ip() : apres s'etre connecte a un jeu distant de statut principal,
		- le joueur envoie son pseudo a ce dernier,
		- le joueur recoit le psudo de ce dernier
	Post-conditions : j->statut_joueur = JEU_JOUEUR_STATUT_SECONDAIRE, j->etat = JEU_ETAT_PAS_PARTIE_ENCOURS, j->pseudo_adversaire renseigne
	Valeur retour : -1 en cas d'echec, 0 en cas de succes
	Gestion erreur	: en cas d'erreur survenue, deconnexion & suppression socket client
*/
static int jeu_rejoindre_partie(jeu* j);

/*	Sous-routine de jeu_tour_joueur : reception reponse joueur adverse
	Pre-conditions 	: j adresse d'un jeu partage en etat JEU_ETAT_TOUR_JOUEUR
	Post-conditions : en cas de succes, reponse joueur adverse placee dans (cleType, etat_case), buffer mis a jour
	Valeur retour : 0 en cas de succes, -1 en cas d'echec : deconnexion (erreur technique), 
															ou simplement desynchronisation des parties des joueurs (erreur fonctionnelle)
*/
static int jeu_message_recevoir_tour_joueur(jeu* j, int* cleType, int* etat_case);

/*	Sous-routine de jeu_tour_adversaire : reception reponse joueur adverse
	Pre-conditions 	: j adresse d'un jeu partage en etat JEU_ETAT_TOUR_JOUEUR_ADVERSAIRE
	Post-conditions : en cas de succes, reponse joueur adverse placee dans (x, y), buffer mis a jour
	Valeur retour : 0 en cas de succes, -1 en cas d'echec : deconnexion (erreur technique), 
															ou simplement desynchronisation des parties des joueurs (erreur fonctionnelle)
*/
static int jeu_message_recevoir_tour_adversaire(jeu* j, unsigned int* x, unsigned int* y);

/*	Pre-conditions 	: j adresse d'un jeu partage, buffer de j->p commencant par la chaine indiquee par 'message'
	Post-conditions : decale le contenu du buffer de j->p de strlen(message) cases vers la gauche
	Fonction appelee apres lecture (et traitement) d'un message, dans le cas ou le buffer contient la concatenation de plusieurs messages
*/
static void jeu_message_suivant(jeu* j, char message[]);

/*	Pre-conditions 	: j adresse d'un jeu partage
	Verifications : la partie est en cours (dans le cas contraire : la fonction ne fait rien, un message en informe l'utilisateur)
	Post-conditions : champ j->etat et, le cas echeant (partie remportee par un joueur), champ j->nb_parties_gagnees mis a jour
	Utilisation : fonction appelee par jeu_placer_bateau(), jeu_placer_flotte(), jeu_tour_joueur(), jeu_tour_adversaire() :
					- si placement du dernier bateau : tour joueur ou tour adversaire
					- si flotte coulee a l'issue d'un tour de jeu : partie gagnee par l'un des deux joueurs
*/
static void jeu_tour_suivant(jeu* j);

/* ____________________ Definition fonctions internes */

/*	Initialise les champs j->statut_joueur, j->etat, j->pseudo_adversaire, j->num_partie, j->nb_parties_gagnees
	Fonction appelée d'abord à la construction du jeu, puis chaque fois qu'un joueur se déconnecte
*/
static void jeu_initialiser_partage(jeu* j) {
	#if(JEU_COMPILATION)
	printf("%s:%s(%p) IN\n", __FILE__, __func__, (void*)j);
	#endif

	j->statut_joueur = JEU_JOUEUR_STATUT_INDETERMINE;
	j->etat = JEU_ETAT_NONE;
	strcpy(j->pseudo_adversaire, STR_JOUEUR_PSEUDO_DEFAULT);
	j->num_partie  = 0;

	#if(JEU_COMPILATION)
	printf("%s:%s(%p) OUT\n", __FILE__, __func__, (void*)j);
	#endif
}

/*	Pre-conditions 	: j adresse d'un jeu partage
	Post-conditions : en cas de succes, chaine indiques par 'message' envoyee a la socket distante, buffer vide
	Valeur retour : nombre d'octets envoyes en cas de succes, -1 en cas d'echec
*/
static int jeu_message_envoyer(jeu* j, char message[]) {
	int res = SYSTEME_OK ;
	
	#if(JEU_COMPILATION)
	printf("%s:%s(%p,'%s') IN\n", __FILE__, __func__, (void*)j, message);
	#endif

	if (j->statut_joueur == JEU_JOUEUR_STATUT_PRINCIPAL) {
		serveur_set_buffer(& (j->p.s), message);
		res = serveur_emettre(& (j->p.s));
	}
	else {
		client_set_buffer(& (j->p.c), message);
		res = client_emettre(& (j->p.c));
	}
	
	#if(JEU_COMPILATION)
	printf("%s:%s(%p,'%s') OUT : res == %d\n", __FILE__, __func__, (void*)j, message, res);
	#endif

	return res;
}

/*	Sous-routine de jeu_tour_joueur : reception reponse joueur adverse
	Pre-conditions 	: j adresse d'un jeu partage en etat JEU_ETAT_TOUR_JOUEUR
	Post-conditions : en cas de succes, reponse joueur adverse placee dans (cleType, etat_case), buffer mis a jour
	Valeur retour : 0 en cas de succes, -1 en cas d'echec : deconnexion (erreur technique), 
															ou simplement desynchronisation des parties des joueurs (erreur fonctionnelle)
*/
static int jeu_message_recevoir_tour_joueur(jeu* j, int* cleType, int* etat_case) {
	int res =SYSTEME_OK, nbDonneesLues =0 ;
	char* buf =(j->statut_joueur == JEU_JOUEUR_STATUT_PRINCIPAL ? serveur_get_buffer(&(j->p.s)) : client_get_buffer(&(j->p.c)));
	char action[ACTION_TAILLE_MAX +1] ="";
	char message[ACTION_TAILLE_MAX +2*UINT_MAX_LEN +3] = "";	/* STR_MSG_RESULTAT + (*cleType, *etat_case) + 2 espaces +'\0' */
	
	#if(JEU_COMPILATION)
	printf("%s:%s(%p, ...) IN\n", __FILE__, __func__, (void*)j);
	#endif

	/* __ reception reponse adversaire
		NB : le buffer est initialement vide, le joueur venant d'envoyer un message dans la fonction appelante jeu_tour_joueur() */
	if (j->statut_joueur == JEU_JOUEUR_STATUT_PRINCIPAL) {
		res = serveur_recevoir(&(j->p.s));
	}
	else {
		res = client_recevoir(&(j->p.c));
	}

	if (res == SYSTEME_KO) {
		printf("%s:%s(%p) erreur : %s\n", __FILE__, __func__, (void*)j, STR_STATUT_ERREUR_COMMUNICATION);
	}
	/* __ pas de message recu	*/
	else if (res == 0) {
		printf("%s:%s(%p,...) erreur %s : message action '%s' attendu non recu.\n", __FILE__, __func__, (void*)j, STR_STATUT_ERREUR_SYNCHRONISATION, STR_MSG_RESULTAT);
		res = SYSTEME_KO;
	}
	/* __ message recu : on l'interprete */
	else {
		nbDonneesLues = sscanf(buf, "%s %d %d", action, cleType, etat_case);

		/* message attendu : on met a jour le buffer
				(le message suivant peut deja avoir ete envoye par l'adversaire passe au tour suivant => mette a jour le buffer)	*/
		if ( (nbDonneesLues == 3) && (strcmp (action, STR_MSG_RESULTAT) == 0) ) {
			sprintf(message, "%s %d %d", STR_MSG_RESULTAT, *cleType, *etat_case);
			jeu_message_suivant(j, message);
			res = SYSTEME_OK;
		}
		/* message inattendu : on l'affiche, et l'on quitte la partie  */
		else {
			printf("%s:%s(%p,...) erreur %s : message recu '%s' inattendu.\n", __FILE__, __func__, (void*)j, STR_STATUT_ERREUR_SYNCHRONISATION, action);
			res = SYSTEME_KO;
		}
	}

	#if(JEU_COMPILATION)
	printf("%s:%s(%p,...) OUT : res == %d, (*cleType,*etat_case) == (%d,%d)\n", __FILE__, __func__, (void*)j, res, *cleType, *etat_case);
	#endif

	return res;
}
/*	Sous-routine de jeu_tour_adversaire : reception reponse joueur adverse
	Pre-conditions 	: j adresse d'un jeu partage en etat JEU_ETAT_TOUR_JOUEUR_ADVERSAIRE
	Post-conditions : en cas de succes, reponse joueur adverse placee dans (x, y), buffer mis a jour
	Valeur retour : 0 en cas de succes, -1 en cas d'echec : deconnexion (erreur technique), 
															ou simplement desynchronisation des parties des joueurs (erreur fonctionnelle)
*/
static int jeu_message_recevoir_tour_adversaire(jeu* j, unsigned int* x, unsigned int* y) {
	int res, nbDonneesLues = 0;
	char* buf = (j->statut_joueur == JEU_JOUEUR_STATUT_PRINCIPAL ? serveur_get_buffer(&(j->p.s)) : client_get_buffer(&(j->p.c)));
	char action[ACTION_TAILLE_MAX +1] = "";
	char message[ACTION_TAILLE_MAX +2*UINT_MAX_LEN +3] = "";	/* STR_MSG_PLACER + (*x, *y) + 2 espaces +'\0' */
	
	#if(JEU_COMPILATION)
	printf("%s:%s(%p,...) IN\n", __FILE__, __func__, (void*)j);
	#endif

	/* __ reception donnees jouees par l'adversaire :
		NB : le buffer peut avoir deja recu le message attendu par l'appel a recv() fait dans le tour precedent
	*/
	res = strlen(buf);
	if (res == 0) {
		if (j->statut_joueur == JEU_JOUEUR_STATUT_PRINCIPAL) {
			res = serveur_recevoir(&(j->p.s));
		}
		else if (j->statut_joueur == JEU_JOUEUR_STATUT_SECONDAIRE) {
			res = client_recevoir(&(j->p.c));
		}
	}

	/* __ message recu : on l'interprete */
	if (res > 0) {
		nbDonneesLues = sscanf(buf, "%s %u %u", action, x, y);

		/* message attendu : on met a jour le buffer	*/
		if ( (nbDonneesLues == 3) && (strcmp (action, STR_MSG_PLACER) == 0) ) {
			/* mise a jour du buffer */
			sprintf(message, "%s %u %u", STR_MSG_PLACER, *x, *y);
			jeu_message_suivant(j, message);
			res = SYSTEME_OK;
		}
		/* message inattendu : on l'affiche, et l'on quitte la partie  */
		else {
			printf("%s:%s(%p,...) erreur %s : message recu '%s' inattendu.\n", __FILE__, __func__, (void*)j, STR_STATUT_ERREUR_SYNCHRONISATION, action);
			res = SYSTEME_KO;
		}
	}
	/* pas de message : on l'affiche, et l'on quitte la partie  */
	else if (res == 0) {
		printf("%s:%s(%p,...) erreur %s : message attendu action '%s' non recu.\n", __FILE__, __func__, (void*)j, STR_STATUT_ERREUR_SYNCHRONISATION, STR_MSG_PLACER);
		res = SYSTEME_KO;
	}
	else /* res == SYSTEME_KO */ {
		printf("%s:%s(%p,...) erreur : %s.\n", __FILE__, __func__, (void*)j, STR_STATUT_ERREUR_COMMUNICATION);
	}

	return res;

	#if (JEU_COMPILATION)
	printf("%s:%s(%p,...) OUT : res == %d, (*x,*y) == (%u,%u)\n", __FILE__, __func__, (void*)j, res, *x, *y);
	#endif
}

/*	Pre-conditions 	: j adresse d'un jeu partage, buffer de j->p commencant par la chaine indiquee par 'message'
	Post-conditions : decale le contenu du ffer de j->p de strlen(message) cases vers la gauche
	Fonction appelee apres lecture (et traitement) d'un message, dans le cas ou le buffer contient la concatenation de plusieurs messages
		(typiquement entre fin jeu_tour_joueur() et debut jeu_tour_advesaire(), 2 send() successifs par socket distante)
*/
static void jeu_message_suivant(jeu* j, char message[]) {
	char* buf = (j->statut_joueur == JEU_JOUEUR_STATUT_PRINCIPAL ? serveur_get_buffer(&(j->p.s)) : client_get_buffer(&(j->p.c)));
	char buf_tmp[BUFFER_TAILLE] = "";
	int len;
	
	#if(JEU_COMPILATION)
	printf("%s:%s(%p,'%s') IN :: buffer '%s'\n", __FILE__, __func__, (void*)j, message, buf);
	#endif

	/* mettre a jour le buffer */
	len = strlen(message);
	strcpy(buf_tmp, buf +len);
	strcpy(buf, buf_tmp);

	#if(JEU_COMPILATION)
	printf("%s:%s(%p,'%s') OUT :: buffer '%s'\n", __FILE__, __func__, (void*)j, message, buf);
	#endif
}

/*	Pre-conditions 	: j adresse d'un jeu partage
	Verifications : la partie est en cours (dans le cas contraire : la fonction ne fait rien, un message en informe l'utilisateur)
	Post-conditions : champ j->etat et, le cas echeant (partie remportee par un joueur), champ j->nb_parties_gagnees mis a jour
	Utilisation : fonction appelee par jeu_placer_bateau(), jeu_placer_flotte(), jeu_tour_joueur(), jeu_tour_adversaire() :
					- si placement du dernier bateau : tour joueur ou tour adversaire
					- si flotte coulee a l'issue d'un tour de jeu : partie gagnee par l'un des deux joueurs
*/
static void jeu_tour_suivant(jeu* j) {
	/* __ Mise a jour structure jeu */
	switch(j->etat) {
		case JEU_ETAT_PLACEMENT:
			/* premier tour de la partie : le joueur secondaire commence ssi partie de numero impair */
			if (joueur_get_nb_bateaux_en_mer(j->j) == joueur_get_flotte_nb_bateaux(j->j) ) {
				if ( ((j->num_partie)%2 == 1 && j->statut_joueur == JEU_JOUEUR_STATUT_SECONDAIRE) || ((j->num_partie)%2 == 0 && j->statut_joueur == JEU_JOUEUR_STATUT_PRINCIPAL) ) {
					j->etat = JEU_ETAT_TOUR_JOUEUR;
				}
				else {
					j->etat = JEU_ETAT_TOUR_ADVERSAIRE;
				}
			}
			else {
				printf("Placement des bateaux en cours...\n");
			}

			break;

		case JEU_ETAT_TOUR_JOUEUR:
			/* si la flotte adverse est coulee : le joueur remporte la partie */
			if (joueur_is_flotte_adverse_coulee(j->j)) {
				j->etat = JEU_ETAT_GAGNEE_JOUEUR;
				(j->nb_parties_gagnees)++;
			}
			/* sinon : on passe au tour de l'adversaire */
			else {
				j->etat = JEU_ETAT_TOUR_ADVERSAIRE;
			}

			break;

		case JEU_ETAT_TOUR_ADVERSAIRE:
			/* si la flotte du joueur est coulee : l'adversaire remporte la partie */
			if (joueur_is_flotte_coulee(j->j)) {
				j->etat = JEU_ETAT_GAGNEE_ADVERSAIRE;
			}
			/* sinon : on passe au tour du joueur */
			else {
				j->etat = JEU_ETAT_TOUR_JOUEUR;
			}

			break;

		default:
			printf("%s:%s(%p) etat '%s' incoherent : aucune operation effectuee.\n", __FILE__, __func__, (void*)j, jeu_get_etat_string(j));
			break;
	}
}

/* ____________________ Définition des fonctions de manipulation */

/* __________ Constructeurs / destructeurs  */

/*	Pre-conditions 	: aucune
	Post-conditions : champ joueur du jeu construit avec les dimensions par defaut (flotte & grille), champs atomiques du jeu initialises
	Gestion erreur	: Renvoie NULL si echec allocation memoire (un message d'erreur est alors affiche)
*/
jeu* jeu_construire(void) {
	jeu* j = NULL;
	unsigned int tab_dim[FLOTTE_NB_TYPE_BATEAU] = JOUEUR_FLOTTE_DIM_DEFAULT;

	#if(JEU_COMPILATION)
	printf("%s:%s() IN\n", __FILE__, __func__);
	#endif

	/* Allocation memoire jeu */
	j = malloc(sizeof (jeu));
	if (j != NULL) {
		/* Initialisation champs */
		jeu_initialiser_partage(j);

		/* Construction & initialisation joueur */
		j->j = joueur_construire(STR_JOUEUR_PSEUDO_DEFAULT, tab_dim, JOUEUR_GRILLE_H_DEFAULT, JOUEUR_GRILLE_L_DEFAULT);

		/* Gestion erreur (allocation memoire) */
		if (j->j == NULL) {
			free(j);
			j = NULL;
		}
	}

	/* Gestion erreur memoire */
	if (j == NULL) {
		printf("%s:%s() erreur : %s\n", __FILE__, __func__, STR_STATUT_ERREUR_MEMOIRE);
	}
	
	#if(JEU_COMPILATION)
	printf("%s:%s() OUT : j == %p\n", __FILE__, __func__, (void*)j);
	#endif

	return j;
}

/*	Pre-conditions 	: les donnees qui sont instanciees le sont de facon coherente 
	Post-conditions : si *j est l'adresse d'un jeu instancie, la memoire reservee en l'adresse contenue dans *j est liberee et *j est reinitialise a NULL,
					  la/les eventeulle(s) socket(s) de (*j)->p sont supprimees
*/
void jeu_detruire(jeu** j) {
	if (*j != NULL) {
		/* Destruction de la partie */
		if ((*j)->statut_joueur == JEU_JOUEUR_STATUT_PRINCIPAL) {
			serveur_delete(& ((*j)->p.s) );
		}
		else if ((*j)->statut_joueur == JEU_JOUEUR_STATUT_SECONDAIRE) {
			client_delete(& ((*j)->p.c) );
		}

		/* Destruction du joueur */
		joueur_detruire(& ((*j)->j) );

		/* Destruction du jeu */
		free(*j);
		*j = NULL;
	}
}

/* __________ Accesseurs en lecture  */

/*	Pre-conditions 	: j adresse d'un jeu
	Post-conditions : aucune
	Valeur retour 	: statut du joueur associe au jeu (joueur ayant cree une partie ou joueur ayant rejoint une partie)
*/
int jeu_get_statut_joueur(jeu* j) {
	return j->statut_joueur;
}

/*	Pre-conditions 	: j adresse d'un jeu
	Post-conditions : aucune
	Valeur retour 	: etat de la partie eventuellement en cours
*/
int jeu_get_etat_partie(jeu* j) {
	return j->etat;
}

/*	Pre-conditions 	: j adresse d'un jeu
	Post-conditions : aucune
	Valeur retour : nombre de parties entamees
*/
int jeu_get_num_partie(jeu* j) {
	return j->num_partie;
}

/*	Pre-conditions 	: j adresse d'un jeu dont le joueur est instancie
	Post-conditions : aucune
	Valeur retour 	: pseudo du joueur associe au jeu
*/
const char* jeu_get_pseudo_joueur(jeu* j) {
	return joueur_get_pseudo(j->j);
}

/*	Pre-conditions 	: j adresse d'un jeu
	Post-conditions : aucune
	Valeur retour : pseudo de l'adversaire
*/
const char* jeu_get_pseudo_adversaire(jeu* j) {
	return j->pseudo_adversaire;
}

/*	Pre-conditions 	: j adresse d'un jeu dont le joueur est instancie, (x,y) coordonnees valides
	Post-conditions : *cleType et *etat_case prennent la valeur des champs correspondant de la case (x,y) de la grille du joueur
*/
void jeu_get_case_joueur(jeu* j, int* cleType, case_grille_etat* etat_case, unsigned int x, unsigned int y) {
	joueur_get_case_joueur(j->j, cleType, etat_case, x, y);
}

/*	Pre-conditions 	: j adresse d'un jeu dont le joueur est instancie, (x,y) coordonnees valides
	Post-conditions : *cleType et *etat_case prennent la valeur des champs correspondant de la case (x,y) de la grille de l'adversaire
*/
void jeu_get_case_adversaire(jeu* j, int* cleType, case_grille_etat* etat_case, unsigned int x, unsigned int y) {
	*cleType = joueur_get_grille_adversaire_case_type_bateau(j->j, x, y);
	*etat_case = joueur_get_grille_adversaire_case_etat(j->j, x, y);
}

/*	Pre-conditions 	: j adresse d'un jeu partage en etat JEU_ETAT_TOUR_PLACEMENT dont le joueur est instancie, parametres cleType, x, y techniquement corrects
	Valeur retour	: PLACEMENT_TYPE_NONE si aucune orientation n'est possible depuis la case coordnnees (x,y),
						PLACEMENT_TYPE_H si seule une orientation horizontale est possible depuis la case coordnnees (x,y),
						PLACEMENT_TYPE_V si seule une orientation verticale est possible depuis la case coordnnees (x,y),
						PLACEMENT_TYPE_NB si les 2 orientations sont possibles depuis ces coordonnees
*/
int jeu_get_placement_valide(jeu* j, bateau_type_cle cleType, unsigned int x, unsigned int y) {
	return joueur_get_placement_valide(j->j, cleType, x, y);
}

/*	Pre-conditions 	: j adresse d'un jeu partage en etat JEU_ETAT_TOUR_JOUEUR ou JEU_ETAT_TOUR_ADVERSAIRE dont le joueur est instancie, 
						parametres x, y techniquement corrects
	Valeur retour	: 1 si coup permis, 0 sinon (coup deja joue)
*/
int jeu_est_coup_valide(jeu* j, unsigned int x, unsigned int y) {
	return joueur_is_coup_valide(j->j, x, y);
}

/*	Pre-conditions 	: j adresse d'un jeu
	Post-conditions : aucune
	Valeur retour : 1 si partie en cours, 0 sinon
*/
int jeu_partie_est_en_cours(jeu* j) {
	return ( (j->etat == JEU_ETAT_PLACEMENT) || (j->etat == JEU_ETAT_TOUR_JOUEUR) || (j->etat == JEU_ETAT_TOUR_ADVERSAIRE) );
}

/*	Pre-conditions 	: j adresse d'un jeu
 	Renvoie 1 si 		j->statut_joueur = JEU_JOUEUR_STATUT_PRINCIPAL et j->etat != JEU_ETAT_ATTENTE_JOUEUR,
					ou  j->statut_joueur = JEU_JOUEUR_STATUT_SECONDAIRE,	0 sinon
*/
int jeu_est_partage(jeu* j) {
	if (j->statut_joueur == JEU_JOUEUR_STATUT_SECONDAIRE) {
		return 1;
	}

	if  ((j->statut_joueur == JEU_JOUEUR_STATUT_PRINCIPAL) && (j->etat != JEU_ETAT_ATTENTE_JOUEUR) ) {
		return 1;
	}

	return 0;
}

/*	Pre-conditions 	: j adresse d'un jeu
 	Renvoie le nom de la machine hote si j->statut_joueur == JEU_JOUEUR_STATUT_PRIMAIRE et "" sinon
*/
const char* jeu_get_nom_hote(jeu* j) {
	return (j->statut_joueur == JEU_JOUEUR_STATUT_PRINCIPAL ? serveur_get_nom_hote(&(j->p.s)) : "");
}

/*	Pre-conditions 	: j adresse d'un jeu
 	Renvoie l'ip de la machine hote si j->statut_joueur == JEU_JOUEUR_STATUT_PRIMAIRE et "" sinon
*/
const char* jeu_get_ip(jeu* j) {
	return (j->statut_joueur == JEU_JOUEUR_STATUT_PRINCIPAL ? serveur_get_ip(&(j->p.s)) : "");
}

/* __________ Accesseurs en ecriture  */

/*	Pre-conditions 	: j adresse d'un jeu (partage : contrainte fonctionnelle et non technique) dont le joueur est instancie
	Post-conditions : pseudo du joueur mis a jour
*/
void jeu_set_pseudo(jeu* j, const char* pseudo) {
	joueur_set_pseudo(j->j, pseudo);
}

/*	Pre-conditions 	: j adresse d'un jeu partage, sans partie en cours (etat JEU_ETAT_PAS_PARTIE_EN_COURS, JEU_ETAT_GAGNEE_JOUEUR ou JEU_ETAT_GAGNEE_ADVERSAIRE),
						dont le joueur est instancie
	Post-conditions : joueur reinitialise, champ j->num_partie incremente, j->etat devient JEU_ETAT_PLACEMENT
*/
void jeu_nouvelle_partie(jeu* j) {
	joueur_initialiser(j->j);
	j->num_partie = j->num_partie +1;
	j->etat = JEU_ETAT_PLACEMENT;
}

/*	Pre-conditions 	: j adresse d'un jeu en etat JEU_ETAT_PLACEMENT dont le joueur est instancie,
						valeur des parametres cleType, indBateau, x, y, orientation techniquement correcte
	Post-conditions : Si placement possible, le bateau est place et, le cas echeant, 
							l'etat de la partie est mis a jour (a JEU_ETAT_TOUR_JOUEUR ou JEU_ETAT_TOUR_ADVERSAIRE)
					  Sinon : un message est affiche
	Valeur retour	: STATUT_REUSSITE si placement effectue, STATUT_ERREUR_PARAMETRE sinon
*/
int jeu_placer_bateau(jeu* j, bateau_type_cle cleType, int indBateau, unsigned x, unsigned y, placement_type orientation) {
	int res;

	res = joueur_placer_bateau(j->j, cleType, indBateau, x, y, orientation);
	jeu_tour_suivant(j);

	return res;
}

/* __ Fonctions aleatoires de jeu  */

/*	Placement aleatoire de la flotte	
	Pre-conditions 	: j adresse d'un jeu partage en etat JEU_ETAT_PLACEMENT dont le joueur est instancie
	Post-conditions : tous les bateaux de la flotte du joueur sont places
*/
void jeu_placer_flotte(jeu* j) {
	joueur_placer_flotte(j->j);
	jeu_tour_suivant(j);
}

/*	Choix d'un coup aleatoire
	Pre-conditions 	: j adresse d'un jeu partage en etat JEU_ETAT_TOUR_JOUEUR ou JEU_ETAT_TOUR_ADVERSAIRE dont le joueur est instancie
	Post-conditions : (*x,*y) coordonnees d'un coup valide
*/
void jeu_set_coup_a_jouer(jeu* j, unsigned int* x, unsigned int* y) {
	joueur_set_coup_a_jouer(j->j, x, y);
}

/* __________ Fonctions d'echange  */

/*	Pre-conditions 	: j adresse d'un jeu non partage et non en attente d'etre rejoint (j->statut_joueur = JEU_JOUEUR_STATUT_INDETERMINE, j->etat == JEU_ETAT_NONE)
	Post-conditions : j->p est un serveur instancie, j->statut_joueur = JEU_JOUEUR_STATUT_PRINCIPAL, j->etat = JEU_ETAT_ATTENTE_JOUEUR
	Gestion erreur : en cas d'échec connexion serveur, un message est affiche (statut joueur et etat jeu sont alors inchanges)
	Utilisation : doit etre appelle prealablement a jeu_etre_rejoint_partie
*/
void jeu_creer_partie(jeu* j) {
	/* socket serveur en attente connexion client */
	serveur_initialiser(& (j->p.s));
	serveur_creer(& (j->p.s));

	if (! serveur_est_connecte(&(j->p.s))) {
		printf("%s:%s(%p) erreur : %s\n", __FILE__, __func__, (void*)j, STR_STATUT_ERREUR_SERVEUR);
		j->statut_joueur = JEU_JOUEUR_STATUT_INDETERMINE;
	}
	else {
		/* mise a jour champs jeu */
		j->statut_joueur = JEU_JOUEUR_STATUT_PRINCIPAL;
		j->etat = JEU_ETAT_ATTENTE_JOUEUR;
	}
}

/*	Pre-conditions 	: j adresse d'un jeu en attente d'être rejoint (j->statut_joueur =JEU_JOUEUR_STATUT_PRINCIPAL, j->etat =JEU_ETAT_ATTENTE_JOUEUR)
	Post-conditions : j->p mis à jour (infos client), j->etat =JEU_ETAT_PAS_PARTIE_ENCOURS, j->pseudo_adversaire renseigne
	Autres traitements : le joueur a envoyé son pseudo à la socket distante
	Gestion erreur	: en cas d'erreur survenue, déconnexion & suppression socket client (le statut du joueur et l'état de la partie sont alors inchangés)
	Utilisation : doit être appelée à la suite de jeu_creer_partie
*/
void jeu_etre_rejoint_partie(jeu* j) {
	int res;

	/* connexion client */
	serveur_connecter_client(& (j->p.s));

	if (serveur_est_client_connecte(& (j->p.s))) {
		/* réception pseudo du client
			NB : le buffer est initialement vide */
		res =serveur_recevoir(& (j->p.s));
		if (res != SYSTEME_KO) {
			/* mise à jour champs pseudo_adversaire */
			strcpy(j->pseudo_adversaire, serveur_get_buffer(& (j->p.s)));

			/* envoi pseudo à client */
			serveur_set_buffer(& (j->p.s), joueur_get_pseudo(j->j));
			res =serveur_emettre(& (j->p.s));
			if (res != SYSTEME_KO) {
				/* mise a jour jeu */
				j->etat =JEU_ETAT_PAS_PARTIE_ENCOURS;
			}
		}

		/* Gestion erreur */
		if (res == SYSTEME_KO) {
			printf("%s:%s(%p) erreur : %s\n", __FILE__, __func__, (void*)j, STR_STATUT_ERREUR_COMMUNICATION);
			serveur_deconnecter_client(& (j->p.s));
		}
	}
}

/*	Pre-conditions 	: j adresse d'un jeu non partage et non en attente d'etre rejoint (j->statut_joueur = JEU_JOUEUR_STATUT_INDETERMINE, j->etat == JEU_ETAT_NONE),
						socket en attente d'etre rejointe a l'adresse 'ip:PORT'
	Post-conditions : j->p est un client instancie, j->statut_joueur = JEU_JOUEUR_STATUT_SECONDAIRE, j->etat = JEU_ETAT_PAS_PARTIE_ENCOURS,
						j->pseudo_adversaire renseigne
	Autres traitements : le joueur a envoye son pseudo a la socket distante
	Gestion erreur	: en cas d'erreur survenue, deconnexion & suppression socket client (le statut du joueur et l'etat de la partie sont alors inchanges)
*/
void jeu_rejoindre_partie_par_nom_hote(jeu* j, char nom_hote[NOM_HOTE_TAILLE_MAX +1]) {
	client_connecter_par_nom(& (j->p.c), nom_hote);

	if (client_est_connecte(& (j->p.c))) {
		jeu_rejoindre_partie(j);
	}
}

/*	Pre-conditions 	: j adresse d'un jeu non partage et non en attente d'etre rejoint (j->statut_joueur = JEU_JOUEUR_STATUT_INDETERMINE, j->etat == JEU_ETAT_NONE),
						socket en attente d'etre rejointe a l'adresse 'ip:PORT'
	Post-conditions : j->p est un client instancie, j->statut_joueur = JEU_JOUEUR_STATUT_SECONDAIRE, j->etat = JEU_ETAT_PAS_PARTIE_ENCOURS,
						j->pseudo_adversaire renseigne
	Autres traitements : le joueur a envoye son pseudo a la socket distante
	Gestion erreur	: en cas d'erreur survenue, deconnexion & suppression socket client (le statut du joueur et l'etat de la partie sont alors inchanges)
*/
void jeu_rejoindre_partie_par_ip(jeu* j, char ip[IP_TAILLE_MAX +1]) {
	/* connexion */
	client_connecter_par_ip(& (j->p.c), ip);

	if (client_est_connecte(& (j->p.c))) {
		jeu_rejoindre_partie(j);
	}
}

/*	Sous-routine jeu_rejoindre_partie_par_nom_hote() et jeu_rejoindre_partie_par_ip() : apres s'etre connecte a un jeu distant de statut principal,
		- le joueur envoie son pseudo a ce dernier,
		- le joueur recoit le psudo de ce dernier
	Post-conditions : j->statut_joueur = JEU_JOUEUR_STATUT_SECONDAIRE, j->etat = JEU_ETAT_PAS_PARTIE_ENCOURS, j->pseudo_adversaire renseigne
	Valeur retour : -1 en cas d'echec, 0 en cas de succes
	Gestion erreur	: en cas d'erreur survenue, deconnexion & suppression socket client
*/
static int jeu_rejoindre_partie(jeu* j) {
	int res = SYSTEME_OK;

	/* __ envoi pseudo */
	client_set_buffer(& (j->p.c), joueur_get_pseudo(j->j));
	res = client_emettre(& (j->p.c));
	if (res != SYSTEME_KO) {
		/* __ reception pseudo adversaire
			NB : le buffer est alors vide */
		res = client_recevoir(& (j->p.c));
		if (res != SYSTEME_KO) {
			/* __ mise a jour champs */
			strcpy(j->pseudo_adversaire, client_get_buffer(& (j->p.c)));
			j->statut_joueur = JEU_JOUEUR_STATUT_SECONDAIRE;
			j->etat = JEU_ETAT_PAS_PARTIE_ENCOURS;

			res = SYSTEME_OK;
		}
	}

	/* __ Gestion erreur */
	if (res == SYSTEME_KO) {
		printf("%s:%s(%p) erreur : %s\n", __FILE__, __func__, (void*)j, STR_STATUT_ERREUR_COMMUNICATION);
		client_delete(& (j->p.c));
	}

	return res;
}

/*	Pre-conditions 	: j adresse d'un jeu partage ou en attente d'etre rejoint (j->statut_joueur = JEU_JOUEUR_STATUT_SECONDAIRE ou j->etat = JEU_ETAT_ATTENTE_JOUEUR)
	Post-conditions : - si joueur principal : socket(s) serveur detruite, eventuelle socket cliente detruite
					  - si joueur secondaire : socket cliente detruite
					  - Dans tous les cas : j->pseudo_adversaire devient STR_JOUEUR_PSEUDO_DEFAULT, j->statut_joueur devient JEU_JOUEUR_STATUT_INDETERMINE,
						j->etat devient JEU_ETAT_NONE
	Gestion erreurs : en cas d'erreur survenue (client_delete ou serveur_delete), un message est affiche
*/
void jeu_se_deconnecter(jeu* j) {
	if ( (j->statut_joueur == JEU_JOUEUR_STATUT_PRINCIPAL) && (j->etat == JEU_ETAT_ATTENTE_JOUEUR) ) {
		serveur_delete(& (j->p.s));
	}
	else if (j->statut_joueur == JEU_JOUEUR_STATUT_SECONDAIRE) {
		client_delete(& (j->p.c));
	}

	jeu_initialiser_partage(j);
}

/*	Pre-conditions 	: j adresse d'un jeu partage en etat JEU_ETAT_TOUR_JOUEUR, (x,y) coordonnees valides 
	Post-conditions : coup (x,y) joue dans la grille adversaire, donnees du jeu mises a jour (joueur vainqueur, tour suivant)
	Gestion erreur	: en cas de probleme de communication, ou de synchronisation des deux joueurs, rencontre :
						la/le(s) socket(s) sont supprimee(s), les champs atomiques du jeu sont reinitialises
*/
void jeu_tour_joueur(jeu* j, unsigned int x, unsigned int y) {
	int res = SYSTEME_OK, cleType = BATEAU_TYPE_CLE_NONE;
	case_grille_etat etat_case = CASE_GRILLE_ETAT_NONE;
	char message[ACTION_TAILLE_MAX +2*UINT_MAX_LEN +3] = "";	/* STR_MSG_PLACER + (x, y) + 2 espaces +'\0' */

	#if (JEU_COMPILATION)
	printf("%s:%s(%p, %u, %u) IN\n", __FILE__, __func__, (void*)j, x, y);
	#endif

	/* __ transmission des données jouées à l'adversaire */
	sprintf(message, "%s %u %u", STR_MSG_PLACER, x, y);
	res =jeu_message_envoyer(j, message);

	/* __ réception de la réponse de l'adversaire (résultat du coup joué)	*/
	if (res != SYSTEME_KO) {
		res =jeu_message_recevoir_tour_joueur(j, &cleType, &etat_case);

		/* __ application coup joue dans le modele & tour suivant */
		if (res == SYSTEME_OK) {
			joueur_set_case_adversaire(j->j, cleType, etat_case, x, y);
			jeu_tour_suivant(j);
		}
	}
	else /* res == SYSTEME_OK */ {
		printf("%s:%s(%p, %u, %u) erreur : %s.\n", __FILE__, __func__, (void*)j, x, y, STR_STATUT_ERREUR_COMMUNICATION);
	}

	/* __ rupture communication ou desynchronisation : interruption du jeu */
	if (res == SYSTEME_KO) {
		jeu_afficher_stat(j);		
		jeu_se_deconnecter(j);
	}

	#if (JEU_COMPILATION)
	printf("%s:%s(%p, %u, %u) OUT : res == %d, cleType == %d, etat_case == %d\n", __FILE__, __func__, (void*)j, x, y, res, cleType, etat_case);
	#endif
}

/*	Pre-conditions 	: j adresse d'un jeu partage en etat JEU_ETAT_TOUR_ADVERSAIRE, (x,y) coordonnees valides 
	Post-conditions : coup (x,y) joue dans la grille adversaire, données du jeu mises à jour (adversaire vainqueur, tour suivant)
	Gestion erreur	: en cas de probleme de communication, ou de synchronisation des deux joueurs, rencontre :
						la/le(s) socket(s) sont supprimee(s), les champs atomiques du jeu sont reinitialises
*/
void jeu_tour_adversaire(jeu* j) {
	int res, cleType = BATEAU_TYPE_CLE_NONE;
	unsigned int x = joueur_get_grille_joueur_nbLignes(j->j), y = joueur_get_grille_joueur_nbColonnes(j->j);
	case_grille_etat etat_case = CASE_GRILLE_ETAT_NONE;
	char message[ACTION_TAILLE_MAX +2*UINT_MAX_LEN +3] = "";	/* STR_MSG_RESULTAT + (cleType, etat_case) + 2 espaces +'\0' */

	#if (JEU_COMPILATION)
	printf("%s:%s(%p) IN\n", __FILE__, __func__, (void*)j);
	#endif

	/* __ reception donnees jouees par l'adversaire	*/
	res =jeu_message_recevoir_tour_adversaire(j, &x, &y);
	if (res == SYSTEME_OK) {
		/* __ application coup joue dans le modele	*/
		joueur_recevoir_coup(j->j, x, y);
		joueur_get_case_joueur(j->j, &cleType, &etat_case, x, y);

		/* __ transmission à l'adversaire du résultat de son coup joué */
		sprintf(message, "%s %d %d", STR_MSG_RESULTAT, cleType, etat_case);
		res =jeu_message_envoyer(j, message);

		/* __ tour suivant */
		if (res != SYSTEME_KO) {
			jeu_tour_suivant(j);
		}
		else /* res == SYSTEME_KO */ {
			printf("%s:%s(%p) erreur : %s.\n", __FILE__, __func__, (void*)j, STR_STATUT_ERREUR_COMMUNICATION);
		}
	}

	/* __ erreur communication ou synchronisation joueurs : interruption du jeu */
	if (res == SYSTEME_KO) {
		jeu_afficher_stat(j);		
		jeu_se_deconnecter(j);
	}

	#if (JEU_COMPILATION)
	printf("%s:%s(%p) OUT : res == %d, (x,y) == (%u,%u), cleType = %d, etat_case = %d\n", __FILE__, __func__, (void*)j, res, x, y, cleType, etat_case);
	#endif
}

/* __________ E/S  */

/*	____ Affichage statistiques jeu	*/

void jeu_afficher_stat(jeu* j) {
	printf("Joueur '%s' contre joueur '%s' : %d parties remportees sur %d parties entamees. Jeu en etat '%s'\n", joueur_get_pseudo(j->j), j->pseudo_adversaire, j->nb_parties_gagnees, j->num_partie, jeu_get_etat_string(j));
}

const char* jeu_get_etat_string(jeu* j) {
	switch(j->etat) {
		case JEU_ETAT_ATTENTE_JOUEUR:
			return "joueur en attente d'etre rejoint";

		case JEU_ETAT_PAS_PARTIE_ENCOURS:
			return "pas de partie en cours";

		case JEU_ETAT_PLACEMENT:
			return "placement des bateaux";

		case JEU_ETAT_TOUR_JOUEUR:
			return "tour du joueur";

		case JEU_ETAT_TOUR_ADVERSAIRE:
			return "tour de l'adversaire";

		case JEU_ETAT_GAGNEE_JOUEUR:
			return "partie remportee par le joueur";

		case JEU_ETAT_GAGNEE_ADVERSAIRE:
			return "remportee par l'adversaire";

		case JEU_ETAT_NONE:
			return "non connectee";

		default:
			return STR_STATUT_ERREUR_PARAMETRE;
	}
}

/*	____ Affichage bataille en cours	*/

/*	Pre-conditions 	: j adresse d'un jeu partage, j->j joueur dont les 2 grilles sont instanciees et de memes dimensions
*/
void jeu_afficher_bataille(jeu* j) {
	joueur_afficher_grilles(j->j);
}

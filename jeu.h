/*	Une partie est caractérisée par :
	- 1 joueur
	- 1 connexion (serveur ou client) à un autre joueur
	- le role du joueur dans la connexion : 
		- secondaire si connexion cliente (par connexion alors a un serveur cree par le joueur distant)
		- primaire si connexion serveur (cree par le joueur et auquel le joueur distant s'est alors connecte)
	- l'etat du jeu
	- le pseudo du joueur adverse dans le cadre de la connexion en cours
	- le nombre de parties entamees dans le cadre de la connexion en cours
	- le nombre de parties remportees par le joueur dans le cadre de la connexion en cours

	Fonctionnement :
	- un jeu est partage ssi
			j->statut_joueur = JEU_JOUEUR_STATUT_SECONDAIRE
		 ou j->statut_joueur = JEU_JOUEUR_STATUT_PRINCIPAL et j->etat <> JEU_ETAT_ATTENTE_JOUEUR

	- les donnes du jeu partage (pseudo joueur adverse, nombre de parties entamees, nombre de parties remportees par le joueur)
		sont reinitiliasees a chaque nouvelle connexion

	Cycle de vie client :
		C.1 Le client se connecte a un serveur (ici : echange des pseudo)
		C.2 Si client souhaite continuer, il attend la notification serveur (c'est la qu'il apprend les dimensions du jeu)
			Sinon, aller en C.6
		C.3 Deroulement partie
		C.4 Aller en C.2
		C.5 Deconnexion client

	Cycle de vie serveur :
		S.1 Le serveur est cree et peut accueillir un client (l'ip de l'hote du serveur est alors disponible)
		S.2 Le serveur attend d'etre rejoint par client (ici : echange des pseudo)
		S.3 Si le serveur souhaite continuer, il notifie le lancement de la partie au client (il transmet alors les dimensions du jeu)
			Sinon, aller en S.7
		S.4 Deroulement partie
		S.5 Aller en S.3
		S.6 Deconnexion serveur (en supprimant bien aussi sa socket cliente)

		Déroulement partie :
			P.1 Tant que tous les bateaux ne sont pas places
					Placer un bateau : choisir coordonnees de placement +orientation pour un bateau donne, ou a choisir aussi (selon choix mode interaction joueur)
			P.2 Tant que la partie n'est remportee par aucune des deux joueurs
					Si tour joueur : choisir coordonnees, envoyer coordonnees, recevoir reponse 
					Si tour adversaire : recevoir coordonnees, envoyer reponse

	Client et serveur devraient pouvoir :
		- À tout moment, modifier leur pseudo; toutefois :
			/!\ le joueur adverse ne connait le nouveau pseudo que s'il est modifié avant connexion (C.1 pour le client, S.2 pour le serveur)
		- À tout moment, quitter le jeu partage (ssi se deconnecter) :
			/!\ le jeu adverse s'en rend compte dÈs lors qu'il fait une action impliquant communication. Il est alors deconnecté.
			/!\ gestion de l'interruption par CTRL +C : ne peut etre realisée au niveau du module, seulement dans le programme principal

	Le serveur doit pouvoir :
		- Avant S.2, modifier les dimensions du jeu

	Synchronisation :
		- Les envois / réception sont synchronisés par la partie jouée, dont l'etat d'avancement est commun aux deux joueurs; toutefois:
			/!\

	TODO :
		- transmettre et gerer dimensions jeu
			=> accesseurs joueur a ecrire, fonction void jeu_lancer_partie(jeu* j); a completer
*/

#ifndef JEU_H
#define JEU_H

#include "joueur.h"
#include "client_serveur.h"

/* ____________________ Parametres */

/* messages echanges */
#define STR_MSG_PLACER "Placer"			/* entete attendu dans un tour de jeu : le joueur dont c'est le tour indique a l'autre le coup joue							*/
#define STR_MSG_RESULTAT "Resultat"		/* entete attendu dans un tour de jeu : le joueur dont ce n'est pas le tour indique a son adversaire le resultat de son coup	*/

#define C_MSG_SEPARATEUR "."			/* separateurs de messages 																									*/
	/* /!\ NB : chaines & separateur non utilise pour echange des pseudo, qui survient juste apres la connexion /!\  */

/* ____________________ Structures de donnees */

/* __ Types enumeres */

/* Type enumere jouer coup */
enum e_jeu_etat {
	JEU_ETAT_NONE,					/* jeu non connecte, ou connecte mais pas de partie en cours ni finie 		*/
	JEU_ETAT_ATTENTE_JOUEUR,		/* jeu serveur sans client connecte 										*/
	JEU_ETAT_PAS_PARTIE_ENCOURS,	/* jeu connecte, pas de partie en cours (suite 1ere connexion uniquement)	*/
	JEU_ETAT_PLACEMENT,				/* jeu connecte, partie en cours, etape de placement  						*/
	JEU_ETAT_TOUR_JOUEUR,			/* jeu connecte, partie en cours, etape de bataille, tour du joueur  		*/
	JEU_ETAT_TOUR_ADVERSAIRE,		/* jeu connecte, partie en cours, etape de bataille, tour de l'adversaire  	*/
	JEU_ETAT_GAGNEE_JOUEUR,			/* jeu connecte, partie finie, joueur vainqueur  							*/
	JEU_ETAT_GAGNEE_ADVERSAIRE		/* jeu connecte, partie finie, adversaire vainqueur  						*/
};

/* Type enumere statut joueur */
enum e_jeu_joueur_statut {
	JEU_JOUEUR_STATUT_INDETERMINE,
	JEU_JOUEUR_STATUT_PRINCIPAL,
	JEU_JOUEUR_STATUT_SECONDAIRE
};

/* Type structure partie	*/
union u_partie {
	client c;
	serveur s;
};

/* Type structure decrivant un jeu	*/
struct s_jeu {
	joueur* j;										/* joueur																									*/
	union u_partie p;								/* partie : NULL ou serveur instancie ou client instancie													*/
	enum e_jeu_joueur_statut statut_joueur;			/* statut joueur : INDETERMINE si p == NULL, PRINCIPAL si p est un serveur, SECONDAIRE si p est un client	*/
	enum e_jeu_etat etat;							/* etat jeu, notamment : ATTENTE_JOUEUR si p est un serveur sans client, NONE si p == NULL					*/
	char pseudo_adversaire[JOUEUR_PSEUDO_TAILLE +1];/* pseudo de l'adversaire																					*/
	int num_partie;									/* numero de la partie dans le cadre du partage en cours													*/
	int nb_parties_gagnees;							/* nombre de parties remportees par le joueur dans le cadre du partage en cours								*/
};

/* __ Alias de type */
typedef enum e_jeu_etat jeu_etat;
typedef enum e_jeu_joueur_statut jeu_joueur_statut;
typedef union u_partie partie;
typedef struct s_jeu jeu;

/* ____________________ Fonctions de manipulation */

/* __________ Constructeurs / destructeurs  */

/*	Pre-conditions 	: aucune
	Post-conditions : champ joueur du jeu construit avec les dimensions par defaut (flotte & grille), champs atomiques du jeu initialisés
	Gestion erreur	: Renvoie NULL si echec allocation memoire (un message d'erreur est alors affiché)
*/
jeu* jeu_construire(void);

/*	Pre-conditions 	: les donnees qui sont instanciees le sont de facon coherente 
	Post-conditions : si *j est l'adresse d'un jeu instancie, la memoire reservee en l'adresse contenue dans *j est liberee et *j est reinitialise a NULL,
					  la/les eventeulle(s) socket(s) de (*j)->p sont supprimees
*/
void jeu_detruire(jeu** j);

/* __________ Accesseurs en lecture  */

/*	Pre-conditions 	: j adresse d'un jeu
	Post-conditions : aucune
	Valeur retour : statut du joueur associe au jeu (joueur ayant cree une partie ou joueur ayant rejoint une partie)
*/
int jeu_get_statut_joueur(jeu* j);

/*	Pre-conditions 	: j adresse d'un jeu
	Post-conditions : aucune
	Valeur retour : etat de la partie eventuellement en cours
*/
int jeu_get_etat_partie(jeu* j);

/*	Pre-conditions 	: j adresse d'un jeu
	Post-conditions : aucune
	Valeur retour : nombre de parties entamees dans le partage en cours
*/
int jeu_get_num_partie(jeu* j);

/*	Pre-conditions 	: j adresse d'un jeu dont le joueur est instancie
	Post-conditions : aucune
	Valeur retour : pseudo du joueur associe au jeu
*/
const char* jeu_get_pseudo_joueur(jeu* j);

/*	Pre-conditions 	: j adresse d'un jeu
	Post-conditions : aucune
	Valeur retour : pseudo de l'adversaire
*/
const char* jeu_get_pseudo_adversaire(jeu* j);

/*	Pre-conditions 	: j adresse d'un jeu dont le joueur est instancie, (x,y) coordonnées valides
	Post-conditions : *cleType et *etat_case prennent la valeur des champs correspondant de la case (x,y) de la grille du joueur
*/
void jeu_get_case_joueur(jeu* j, int* cleType, case_grille_etat* etat_case, unsigned int x, unsigned int y);

/*	Pre-conditions 	: j adresse d'un jeu dont le joueur est instancie, (x,y) coordonnées valides
	Post-conditions : *cleType et *etat_case prennent la valeur des champs correspondant de la case (x,y) de la grille de l'adversaire
*/
void jeu_get_case_adversaire(jeu* j, int* cleType, case_grille_etat* etat_case, unsigned int x, unsigned int y);

/*	Pre-conditions 	: j adresse d'un jeu partage en etat JEU_ETAT_TOUR_PLACEMENT dont le joueur est instancie, parametres cleType, x, y techniquement corrects
	Valeur retour	: PLACEMENT_TYPE_NONE si aucune orientation n'est possible depuis la case coordonnées (x,y),
						PLACEMENT_TYPE_H si seule une orientation horizontale est possible depuis la case coordonnées (x,y),
						PLACEMENT_TYPE_V si seule une orientation verticale est possible depuis la case coordonnées (x,y),
						PLACEMENT_TYPE_NB si les 2 orientations sont possibles depuis ces coordonnees
*/
int jeu_get_placement_valide(jeu* j, bateau_type_cle cleType, unsigned int x, unsigned int y);

/*	Pre-conditions 	: j adresse d'un jeu partage en etat JEU_ETAT_TOUR_JOUEUR ou JEU_ETAT_TOUR_ADVERSAIRE dont le joueur est instancie, 
						parametres x, y techniquement corrects
	Valeur retour	: 1 si coup permis, 0 sinon (coup deja joue)
*/
int jeu_est_coup_valide(jeu* j, unsigned int x, unsigned int y);

/*	Pre-conditions 	: j adresse d'un jeu
	Post-conditions : aucune
	Valeur retour : 1 si partie en cours, 0 sinon
*/
int jeu_partie_est_en_cours(jeu* j);

/*	Pre-conditions 	: j adresse d'un jeu
 	Renvoie 1 si 		j->statut_joueur = JEU_JOUEUR_STATUT_PRIMAIRE et j->etat != JEU_ETAT_ATTENTE_JOUEUR,
					ou  j->statut_joueur = JEU_JOUEUR_STATUT_SECONDAIRE,	0 sinon
*/
int jeu_est_partage(jeu* j);

/*	Pre-conditions 	: j adresse d'un jeu
 	Renvoie le nom de la machine hote si j->statut_joueur == JEU_JOUEUR_STATUT_PRIMAIRE et "" sinon
*/
const char* jeu_get_nom_hote(jeu* j);

/*	Pre-conditions 	: j adresse d'un jeu
 	Renvoie l'ip de la machine hote si j->statut_joueur == JEU_JOUEUR_STATUT_PRIMAIRE et "" sinon
*/
const char* jeu_get_ip(jeu* j);

/* __________ Accesseurs en ecriture  */

/*	Pre-conditions 	: j adresse d'un jeu (partage : contrainte fonctionnelle et non technique) dont le joueur est instancie
	Post-conditions : pseudo du joueur mis a jour
*/
void jeu_set_pseudo(jeu* j, const char* pseudo);

/*	Pre-conditions 	: j adresse d'un jeu partage, sans partie en cours (ie etat <> JEU_ETAT_PAS_PARTIE_EN_COURS, JEU_ETAT_GAGNEE_JOUEUR ou JEU_ETAT_GAGNEE_ADVERSAIRE), dont le joueur est instancie
	Post-conditions : joueur reinitialise, champ j->num_partie incremente, j->etat devient JEU_ETAT_PLACEMENT
*/
void jeu_nouvelle_partie(jeu* j);

/*	Pre-conditions 	: j adresse d'un jeu en etat JEU_ETAT_PLACEMENT dont le joueur est instancie,
						valeur des parametres cleType, indBateau, x, y, orientation techniquement correcte
	Post-conditions : Si placement possible, le bateau est place et, le cas echeant, 
							l'etat de la partie est mis a jour (a JEU_ETAT_TOUR_JOUEUR ou JEU_ETAT_TOUR_ADVERSAIRE)
					  Sinon : un message est affiche
	Valeur retour	: STATUT_REUSSITE si placement effectue, STATUT_ERREUR_PARAMETRE sinon
*/
int jeu_placer_bateau(jeu* j, bateau_type_cle cleType, int indBateau, unsigned x, unsigned y, placement_type orientation);

/* __ Fonctions aleatoires de jeu  */

/*	Placement aleatoire de la flotte	
	Pre-conditions 	: j adresse d'un jeu partage en etat JEU_ETAT_PLACEMENT dont le joueur est instancie
	Post-conditions : tous les bateaux de la flotte du joueur sont places
*/
void jeu_placer_flotte(jeu* j);

/*	Fonction de jeu aleatoire : choix d'un coup aleatoire
	Pre-conditions 	: j adresse d'un jeu partage en etat JEU_ETAT_TOUR_JOUEUR ou JEU_ETAT_TOUR_ADVERSAIRE dont le joueur est instancie
	Post-conditions : (*x,*y) coordonnees d'un coup valide
*/
void jeu_set_coup_a_jouer(jeu* j, unsigned int* x, unsigned int* y);

/* __________ Fonctions d'echange  */

/*	Pre-conditions 	: j adresse d'un jeu non partage et non en attente d'etre rejoint (j->statut_joueur = JEU_JOUEUR_STATUT_INDETERMINE, j->etat == JEU_ETAT_NONE)
	Post-conditions : j->p est un serveur instancie, j->statut_joueur = JEU_JOUEUR_STATUT_PRINCIPAL, j->etat = JEU_ETAT_ATTENTE_JOUEUR
	Gestion erreur : en cas d'échec connexion serveur, un message est affiche (statut joueur et etat jeu sont alors inchanges)
	Utilisation : doit etre appelle prealablement a jeu_etre_rejoint_partie
*/
void jeu_creer_partie(jeu* j);

/*	Pre-conditions 	: j adresse d'un jeu en attente d'etre rejoint (j->statut_joueur = JEU_JOUEUR_STATUT_PRINCIPAL, j->etat = JEU_ETAT_ATTENTE_JOUEUR)
	Post-conditions : j->p mis a jour (infos client), j->etat = JEU_ETAT_PAS_PARTIE_ENCOURS, j->pseudo_adversaire renseigne
	Autres traitements : le joueur a envoye son pseudo a la socket distante
	Gestion erreur	: en cas d'erreur survenue, deconnexion & suppression socket client (le statut du joueur et l'état de la partie sont alors inchangés)
	Utilisation : doit etre appellee a la suite de jeu_creer_partie
*/
void jeu_etre_rejoint_partie(jeu* j);

/*	Pre-conditions 	: j adresse d'un jeu non partage et non en attente d'etre rejoint (j->statut_joueur = JEU_JOUEUR_STATUT_INDETERMINE, j->etat == JEU_ETAT_NONE),
						socket en attente d'etre rejointe a l'adresse 'ip:PORT'
	Post-conditions : j->p est un client instancie, j->statut_joueur = JEU_JOUEUR_STATUT_SECONDAIRE, j->etat = JEU_ETAT_PAS_PARTIE_ENCOURS,
						j->pseudo_adversaire renseigne
	Autres traitements : le joueur a envoye son pseudo a la socket distante
	Gestion erreur	: en cas d'erreur survenue, deconnexion & suppression socket client (le statut du joueur et l'état de la partie sont alors inchangés)
*/
void jeu_rejoindre_partie_par_nom_hote(jeu* j, char nom_hote[NOM_HOTE_TAILLE_MAX +1]);

/*	Pre-conditions 	: j adresse d'un jeu non partage et non en attente d'etre rejoint (j->statut_joueur = JEU_JOUEUR_STATUT_INDETERMINE, j->etat == JEU_ETAT_NONE),
						socket en attente d'etre rejointe a l'adresse 'ip:PORT'
	Post-conditions : j->p est un client instancie, j->statut_joueur = JEU_JOUEUR_STATUT_SECONDAIRE, j->etat = JEU_ETAT_PAS_PARTIE_ENCOURS,
						j->pseudo_adversaire renseigne
	Autres traitements : le joueur a envoye son pseudo a la socket distante
	Gestion erreur	: en cas d'erreur survenue, deconnexion & suppression socket client (le statut du joueur et l'état de la partie sont alors inchangés)
*/
void jeu_rejoindre_partie_par_ip(jeu* j, char ip[IP_TAILLE_MAX +1]);

/*	Pre-conditions 	: j adresse d'un jeu partage ou en attente d'etre rejoint (j->statut_joueur = JEU_JOUEUR_STATUT_SECONDAIRE ou j->etat = JEU_ETAT_ATTENTE_JOUEUR)
	Post-conditions : - si joueur principal : socket(s) serveur detruite, eventuelle socket cliente detruite
					  - si joueur secondaire : socket cliente detruite
					  - Dans tous les cas : j->pseudo_adversaire devient STR_JOUEUR_PSEUDO_DEFAULT, j->statut_joueur devient JEU_JOUEUR_STATUT_INDETERMINE,
						j->etat devient JEU_ETAT_NONE
	Gestion erreurs : en cas d'erreur survenue (client_delete ou serveur_delete), un message est affiche
*/
void jeu_se_deconnecter(jeu* j);

/*	Pre-conditions 	: j adresse d'un jeu partage en etat JEU_ETAT_TOUR_JOUEUR, (x,y) coordonnees valides 
	Post-conditions : coup (x,y) joue dans la grille adversaire, donnees du jeu mises a jour (joueur vainqueur, tour suivant)
	Gestion erreur	: en cas de probleme de communication, ou de synchronisation des deux joueurs, rencontre :
						la/le(s) socket(s) sont supprimee(s), les champs atomiques du jeu sont reinitialises
*/
void jeu_tour_joueur(jeu* j, unsigned int x, unsigned int y);

/*	Pre-conditions 	: j adresse d'un jeu partage en etat JEU_ETAT_TOUR_ADVERSAIRE, (x,y) coordonnees valides 
	Post-conditions : coup (x,y) joue dans la grille adversaire, donnees du jeu mises a jour (adversaire vainqueur, tour suivant)
	Gestion erreur	: en cas de probleme de communication, ou de synchronisation des deux joueurs, rencontre :
						la/le(s) socket(s) sont supprimee(s), les champs atomiques du jeu sont reinitialises
*/
void jeu_tour_adversaire(jeu* j);

/* __________ Entrées / Sorties  */

/*	____ Affichage statistiques jeu	*/

void jeu_afficher_stat(jeu* j);

const char* jeu_get_etat_string(jeu* j);

/*	____ Affichage bataille en cours	*/

/*	Pre-conditions 	: j adresse d'un jeu partage, j->j joueur dont les 2 grilles sont instanciees et de memes dimensions
*/
void jeu_afficher_bataille(jeu* j);

#endif


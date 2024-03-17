/* Un joueur est caracterise par :
	- 1 pseudo
	- sa flotte
	- sa grille de jeu
	- la vue qu'il a de la grille de jeu l'adversaire

	Mode de gestion :
	- C'est le joueur qui construit sa flotte => qui en determine les dimensions
	- C'est le joueur qui construit sa grille et celle de son adversaire (la vue qu'il en a) => qui determine les dimensions des grilles de jeu

	Coherence (fonctionnelle) :
	- Les dimensions (nombres de lignes et de colonnes) des deux grilles doivent toujours etre identiques
	- La taille de la flotte et le nombre de cases a couler dans les deux grille doivent toujours etre identiques
	- Les dimensions de la flotte d'une part, des grilles d'autre part, doivent etre coherentes (ie. il existe un placement possible des bateaux)

	Le joueur peut :
	- modifier son pseudo
	- modifier les dimensions de la flotte 
	- modifier les dimensions de la grille de jeu

	TODO :
	- gestion coherence dimensions flotte et grille
	- fonction 'joueur_placer_flotte' : nombre d'essais selon dimensions relatives grille et flotte ou maintenir grille realisable apres chaque placement
*/

#ifndef JOUEUR_H
#define JOUEUR_H

#include "flotte.h"
#include "grille.h"

/* ____________________ Parametres */

#define STR_JOUEUR_PSEUDO_DEFAULT "Joueur anononyme" 	/* Attention : doit etre de taille <= JOUEUR_PSEUDO_TAILLE */
#define NB_ESSAIS_MAX 1000

/* Dimensions par defaut du jeu */

#define JOUEUR_GRILLE_H_DEFAULT 10				/* hauteur (nombre de lignes)					*/
#define JOUEUR_GRILLE_L_DEFAULT 10				/* largeur (nombre de colonnes)					*/
#define JOUEUR_FLOTTE_DIM_DEFAULT {1,1,1,1,1}	/* nombre par defaut de bateaux de chaque type	*/

/* ____________________ Structures de donnees */

#include "global.h"				/* pour JOUEUR_PSEUDO_TAILLE					*/

/* Type structure decrivant un joueur	*/
struct s_joueur {
	char pseudo[JOUEUR_PSEUDO_TAILLE +1];			/* pseudo du joueur			*/
	flotte* flotte_joueur;							/* flotte du joueur			*/
	grille* grille_joueur;							/* grille du joueur			*/
	grille* grille_adversaire;						/* grille de l'adversaire	*/
};

/* Alias de type */

typedef struct s_joueur joueur;

/* ____________________ Fonctions de manipulation */

/* __________ Constructeurs / destructeurs / Initialisation / Dimensionnement */

/*	Pre-conditions 	: aucune
	Post-conditions : joueur instancie (dont sa flotte, sa grille, celle de son advsersaire) en une adresse memoire qui est renvoyee
					  Le joueur, la flotte et les grilles sont instancées selon les parametres transmis
					  Si la chaine pseudo excede JOUEUR_PSEUDO_TAILLE caracteres (significatifs), seuls les JOUEUR_PSEUDO_TAILLE premiers sont retenus 
	Gestion erreur	: Renvoie NULL si echec allocation memoire (un message d'erreur est alors affiche)
*/
joueur* joueur_construire(const char* pseudo, unsigned int tab_nb_par_type[FLOTTE_NB_TYPE_BATEAU], unsigned int nbL, unsigned int nbC);

/*	Pre-conditions 	: les champs  (*j)->flotte,  (*j)->grille_joueur et (*j)->grille_adversaire
						qui sont (totalement ou partiellement) instancies le sont de facon coherente 
	Post-conditions : si *j est l'adresse d'un joueur instancie, la memoire reservee en l'adresse contenue dans *j est liberee et *j est reinitialise a NULL
*/
void joueur_detruire(joueur** j);

/*	Pre-conditions 	: j adresse d'un joueur dont la flotte est instanciee
	Post-conditions : flotte du joueur redimensionnee et reinitialisee
	Gestion erreur	: Renvoie NULL si echec allocation memoire (un message d'erreur est alors affiche)
*/
joueur* joueur_redimensionner_flotte(joueur* j, unsigned int tab_nb_par_type[FLOTTE_NB_TYPE_BATEAU]);

/*	Pre-conditions 	: j adresse d'un joueur dont les grilles sont instanciees
	Post-conditions : grilles (joueur et adversaire) redimensionnees et reinitialisees
	Gestion erreur	: En cas d'erreur (une message d'erreur est alors affiche), les dimensions initiales sont preservees
*/
void joueur_redimensionner_grilles(joueur* j, unsigned int nbL, unsigned int nbC);

/*	Pre-conditions 	: j adresse d'un joueur dont la flotte et les grilles sont instanciees
	Post-conditions : flotte et grilles  (joueur et adversaire) reinitialisees (structure flotte et dimension grilles inchangees)
*/
void joueur_initialiser(joueur* j);

/* __________ Accesseurs en ecriture */

/*	Pre-conditions 	: j adresse d'un joueur
	Post-conditions : j->pseudo mis a jour
*/
void joueur_set_pseudo(joueur* j, const char* pseudo);

/* __________ Accesseurs en lecture */

/*	Pre-conditions 	: j adresse d'un joueur
	Post-conditions : aucune
*/
const char* joueur_get_pseudo(joueur* j);

/*	Pre-conditions 	: j adresse d'un joueur dont la flotte est instanciee
	Post-conditions : aucune
	Valeur retour	: nombre de bateaux de la flotte
*/
unsigned int joueur_get_flotte_nb_bateaux(joueur* j);

/*	Pre-conditions 	: j adresse d'un joueur dont la flotte est instanciee
	Post-conditions : aucune
	Valeur retour	: somme des tailles des bateaux de la flotte
*/
unsigned int joueur_get_flotte_taille(joueur* j);

/*	Pre-conditions 	: j adresse d'un joueur dont la flotte est instanciee
	Post-conditions : aucune
*/
unsigned int joueur_get_flotte_type_nb_bateaux(joueur* j, bateau_type_cle cleType);

/*	Pre-conditions 	: j adresse d'un joueur dont la grille joueur est instanciee
	Post-conditions : aucune
*/
unsigned int joueur_get_grille_joueur_nbLignes(joueur* j);

/*	Pre-conditions 	: j adresse d'un joueur dont la grille joueur est instanciee
	Post-conditions : aucune
*/
unsigned int joueur_get_grille_joueur_nbColonnes(joueur* j);

/*	Pre-conditions 	: j adresse d'un joueur dont la grille advsersaire est instanciee
						x et y a valeur dans respectivement [0,j->grille_joueur->nbLignes -1] et [0,j->grille_joueur->nbColonnes -1]
	Post-conditions : aucune
*/
case_grille_etat joueur_get_grille_joueur_case_etat(joueur* j, unsigned int x, unsigned int y);

/*	Pre-conditions 	: j adresse d'un joueur dont la grille joueur est instanciee
						x et y a valeur dans respectivement [0,j->grille_joueur->nbLignes -1] et [0,j->grille_joueur->nbColonnes -1]
	Post-conditions : aucune
*/
bateau_type_cle joueur_get_grille_joueur_case_type_bateau(joueur* j, unsigned int x, unsigned int y);

/*	Pre-conditions 	: j adresse d'un joueur dont la grille joueur est instanciee
						x et y a valeur dans respectivement [0,j->grille_joueur->nbLignes -1] et [0,j->grille_joueur->nbColonnes -1]
	Post-conditions : aucune
*/
int joueur_get_grille_joueur_case_id_bateau_type(joueur* j, unsigned int x, unsigned int y);

/*	Pre-conditions 	: j adresse d'un joueur dont la grille joueur est instanciee
	Post-conditions : aucune
*/
unsigned int joueur_get_grille_adversaire_nbLignes(joueur* j);

/*	Pre-conditions 	: j adresse d'un joueur dont la grille advsersaire est instanciee
	Post-conditions : aucune
*/
unsigned int joueur_get_grille_adversaire_nbColonnes(joueur* j);

/*	Pre-conditions 	: j adresse d'un joueur dont la grille adversaire est instanciee
						x et y a valeur dans respectivement [0,j->grille_adversaire->nbLignes -1] et [0,j->grille_adversaire->nbColonnes -1]
	Post-conditions : aucune
*/
case_grille_etat joueur_get_grille_adversaire_case_etat(joueur* j, unsigned int x, unsigned int y);

/*	Pre-conditions 	: j adresse d'un joueur dont la grille adversaire est instanciee
						x et y a valeur dans respectivement [0,j->grille_adversaire->nbLignes -1] et [0,j->grille_adversaire->nbColonnes -1]
	Post-conditions : aucune
*/
bateau_type_cle joueur_get_grille_adversaire_case_type_bateau(joueur* j, unsigned int x, unsigned int y);

/* __________ Fonctions de jeu */

/* _____ Etat de la flotte du joueur */

/*	Pre-conditions 	: j adresse d'un joueur dont la flotte est instanciee
	Post-conditions : aucune
*/
unsigned int joueur_get_nb_bateaux_en_mer(joueur* j);

/*	Pre-conditions 	: j adresse d'un joueur dont la flotte est instanciee
						cleType a valeur dans [BATEAU_TYPE_CLE_NONE +1, BATEAU_TYPE_CLE_NB -1],
						indBateau a valeur dans [0,j->flotte_joueur->tab_nb_par_type[cleType -BATEAU_TYPE_CLE_NONE -1] -1]
	Valeur retour	: 1 si le bateau est place, 0 sinon
*/
int joueur_is_bateau_en_mer(joueur* j, bateau_type_cle cleType, unsigned int indBateau);

/*	Pre-conditions 	: j adresse d'un joueur dont la flotte est instanciee
						cleType a valeur dans [BATEAU_TYPE_CLE_NONE +1, BATEAU_TYPE_CLE_NB -1],
						indBateau a valeur dans [0,j->flotte_joueur->tab_nb_par_type[cleType -BATEAU_TYPE_CLE_NONE -1] -1],
						xD, yD, xF, yF adresses d'un entier
	Post-conditions : Si bateau non place : *x_1 == *y_1 == *x_2 == *y_2 == PLACEMENT_COORD_NONE
					  Sinon, si L designe la longueur des bateaux du type identifie par cleType :
						(*xD,*yD) == coordonnees du placement du bateau de la flotte designe par cleType et indBateau,
						(*xF,*yF) == (xD+L,yD) ou (xD,yD+L) selon l'orientation du bateau
*/
void joueur_get_bateau_position(joueur* j, bateau_type_cle cleType, unsigned int indBateau, int* xD, int* yD, int* xF, int* yF);

/*	Pre-conditions 	: j adresse d'un joueur dont la flotte et la grille joueur sont instanciees
						cleType a valeur dans [BATEAU_TYPE_CLE_NONE +1, BATEAU_TYPE_CLE_NB -1],
						indBateau a valeur dans [0,j->flotte_joueur->tab_nb_par_type[cleType -BATEAU_TYPE_CLE_NONE -1] -1]
	Valeur retour	: 1 si le bateau est coule, 0 sinon
*/
int joueur_is_bateau_coule(joueur* j, bateau_type_cle cleType, unsigned int indBateau);

/*	Pre-conditions 	: j adresse d'un joueur dont la grille joueur est instanciee
	Valeur retour	: le nombre de coups portes par l'adversaires ayant fait mouche
*/
int joueur_get_nb_coups_joues_adversaire(joueur* j);

/*	Pre-conditions 	: j adresse d'un joueur dont la grille adversaire est instanciee
	Valeur retour	: le nombre de coups portes par le joueur ayant fait mouche
*/
int joueur_get_nb_coups_joues_joueur(joueur* j);

/*	Pre-conditions 	: j adresse d'un joueur dont la grille joueur est instanciee
	Valeur retour	: le nombre de coups portes par l'adversaires ayant fait mouche
*/
int joueur_get_nb_coups_recus_touche(joueur* j);

/*	Pre-conditions 	: j adresse d'un joueur dont la grille adversaire est instanciee
	Valeur retour	: le nombre de coups portes par le joueur ayant fait mouche
*/
int joueur_get_nb_coups_donnes_touche(joueur* j);

/*	Pre-conditions 	: j adresse d'un joueur dont la flotte et la grille joueur sont instanciees
	Valeur retour	: 1 si la flotte est coulee, 0 sinon
*/
int joueur_is_flotte_coulee(joueur* j);

/*	Pre-conditions 	: j adresse d'un joueur dont la flotte et la grille joueur sont instanciees
	Valeur retour	: 1 si la flotte est coulee, 0 sinon
*/
int joueur_is_flotte_adverse_coulee(joueur* j);

/* _____ Tests validite actions de jeu */

/*	Pre-conditions 	: j adresse d'un joueur dont la grille joueur et la flotte sont instanciees,
						x a valeur dans [0,j->grille_joueur->nbLignes -1], y a valeur dans [0,j->grille_joueur->nbColonnes -1],
						cleType a valeur dans [BATEAU_TYPE_CLE_NONE +1, BATEAU_TYPE_CLE_NB -1]
	Valeur retour	: PLACEMENT_TYPE_NONE si aucune orientation n'est possible depuis la case coordnnees (x,y),
						PLACEMENT_TYPE_H si seule une orientation horizontale est possible depuis la case coordnnees (x,y),
						PLACEMENT_TYPE_V si seule une orientation verticale est possible depuis la case coordnnees (x,y),
						PLACEMENT_TYPE_NB si les 2 orientations sont possibles depuis ces coordonnees
*/
int joueur_get_placement_valide(joueur* j, bateau_type_cle cleType, unsigned int x, unsigned int y);

/*	Pre-conditions 	: j adresse d'un joueur dont la grille adversaire est instanciee,
						x a valeur dans [0,j->grille_adversaire->nbLignes -1], y a valeur dans [0,j->grille_adversaire->nbColonnes -1]
	Valeur retour	: 1 si coup permis, 0 sinon (coup deja joue)
*/
int joueur_is_coup_valide(joueur* j, unsigned int x, unsigned int y);

/* _____ Actions de jeu */

/*	Pre-conditions 	: j adresse d'un joueur dont la grille joueur et la flotte sont instanciees,
						x a valeur dans [0,j->grille_joueur->nbLignes -1], y a valeur dans [0,j->grille_joueur->nbColonnes -1],
						orientation a valeur dans {PLACEMENT_TYPE_H,PLACEMENT_TYPE_V},
						cleType a valeur dans [BATEAU_TYPE_CLE_NONE +1, BATEAU_TYPE_CLE_NB -1],
						indBateau a valeur dans [0,j->flotte_joueur->tab_nb_par_type[cleType -BATEAU_TYPE_CLE_NONE -1] -1]
	Post-conditions : Si placement possible : 
					  - les cases de la grille du joueur sont mises a jour conformement au placement du bateau,
					  - la position du bateau dans la flotte est mise a jour
					  Sinon : un message est affiche
	Valeur retour	: STATUT_REUSSITE si placement effectue, STATUT_ERREUR_PARAMETRE sinon
*/
int joueur_placer_bateau(joueur* j, bateau_type_cle cleType, unsigned int indBateau, unsigned int x, unsigned int y, placement_type orientation);

/*	Pre-conditions 	: j adresse d'un joueur instancie dont la grille joueur et la flotte sont instanciees, 
						(x,y) coordonnees valides non encore jouees par l'adversaire
	Post-conditions : l'etat de la case de cordonnees (x,y) dans la grille du joueur est mis a jour en fonction des informations de la grille :
						- si la case n'est pas occupee par un bateau, l'etat devient CASE_GRILLE_ETAT_JOUEE_EAU
						- si la case est occupee par un bateau qui n'est pas coule a l'issue de ce coup, l'etat devient CASE_GRILLE_ETAT_JOUEE_TOUCHE
						- si la case est occupee par un bateau qui est coule a l'issue de ce coup, l'etat devient CASE_GRILLE_ETAT_JOUEE_COULE
*/
void joueur_recevoir_coup(joueur* j, unsigned int x, unsigned int y);

/*	Pre-conditions 	: j adresse d'un joueur instancie dont la grille adverse et la flotte sont instanciees, (x,y) coordonnees valides
						cleType a valeur dans [BATEAU_TYPE_CLE_NONE,BATEAU_TYPE_CLE_NB -1] et
					  	etat_case a valeur dans [CASE_GRILLE_ETAT_NONE +1,CASE_GRILLE_ETAT_NB -1] verifiant
							cleType == BATEAU_TYPE_CLE_NONE ssi etat == CASE_GRILLE_ETAT_JOUEE_EAU
	Post-conditions : L'etat et le type de bateau de la case (x,y) dans la grille de l'adversaire sont mises a jour aux valeurs passees en parametre
*/
void joueur_set_case_adversaire(joueur* j, bateau_type_cle cleType, case_grille_etat etat_case, unsigned int x, unsigned int y);

/*	Pre-conditions 	: j adresse d'un joueur instancie dont la grille joueur et la flotte sont instanciees, (x,y) coordonnees valides
						cleType et etat_case adresses d'un entier
							cleType == BATEAU_TYPE_CLE_NONE ssi etat == CASE_GRILLE_ETAT_JOUEE_EAU
	Post-conditions : *cleType et *etat_case prennent la valeur des champs correspondant de la case (x,y) de la grille du joueur
*/
void joueur_get_case_joueur(joueur* j, bateau_type_cle* cleType, case_grille_etat* etat_case, unsigned int x, unsigned int y);

/* _____ Actions de jeu aleatoires */

/*	Pre-conditions 	: j adresse d'un joueur instancie, il a ete prealablement fait appel a la fonction srand()
	Post-conditions : (*x,*y) designe une case non joue dans la grille adverse
	Tirage aleatoire : 1 case est tiree au hasard parmi les cases non encores jouees dans la grille adversaire
*/
void joueur_set_coup_a_jouer(joueur* j, unsigned int* x, unsigned int* y);

/*	Pre-conditions 	: j adresse d'un joueur instancie, dont les dimensions de la grille et de la flotte sont compatibles, 
						et dont les bateaux ne sont pas places, , il a ete prealablement fait appel a la fonction srand()
	Post-conditions : tous les bateaux du joueur sont places en cas de success, aucun bateau place en ces d'echec
	Tirage aleatoire : 
			- l'ordre dans lequel les bateaux sont consideres est fixe (type par type, de BATEAU_TYPE_CLE_NONE +1 a BATEAU_TYPE_CLE_NB -1)
			- pour un bateau d'un type donne, 1 case est tiree au hasard parmi les cases a partir desquelles le bateau peut etre place
			- si 2 orientations sont possibles, l'orientation horizontale ou verticale est tiree au hasard
	Valeur retour : STATUT_REUSSITE si tous les bateaux places, STR_STATUT_ERREUR_NB_ESSAIS sinon.
		La fonction retourne STR_STATUT_ERREUR_NB_ESSAIS si au bout de NB_ESSAIS_MAX tentatives, elle n'est pas parvenue a placer les bateaux.
		Ce case peut survenir car :
			- sur joueur : aucune verification de coherence n'est faite entre les dimensions de la grille et la dimension de la flotte
			- sur joueur_placer_flotte : lorsqu'un bateau est place, la fonction ne verifie pas que la grille resultante est realisable
					(ie., que les bateaux restant a placer peuvent etre places sur la grille partiellement remplie)

	TODO :	nombre d'essais selon dimensions relatives grille et flotte ou maintenir grille realisable apres chaque placement
*/
int joueur_placer_flotte(joueur* j);

/* __________ Entrees / Sorties */

/*	____ Affichage jeu bataille	*/

/*	Pre-conditions 	: j adresse d'un joueur dont les 2 grilles sont instanciees et de memes dimensions
*/
void joueur_afficher_grilles(joueur* j);

#endif


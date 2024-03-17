/* La case d'une grille est cacaracterisee par :
	- sa grille d'appartenance
	- un etat joue : non joue, joue dans l'eau, joue et touche, joue et coule
	- un type de bateau qui la recouvre
	- l'identifiant du bateau du type de bateau qui la recouvre : CASE_GRILLE_BATEAU_NONE (-1) si pas de bateau, entier >= 0 sinon

	Mode de gestion :
	- la reference sur la grille d'appartenance est non NULL et constante (une case d'existe pas sans grille)

	Dans une partie :
	- l'etat est initialement CASE_GRILLE_ETAT_NONE
	- l'etat CASE_GRILLE_ETAT_NONE peut devenir CASE_GRILLE_ETAT_JOUEE_EAU, CASE_GRILLE_ETAT_JOUEE_TOUCHEE ou CASE_GRILLE_ETAT_JOUEE_COULE
		(au plus 1 tel changement eu cours de partie, phase de jeu)
	- l'etat CASE_GRILLE_ETAT_JOUEE_TOUCHEE peut devenir CASE_GRILLE_ETAT_JOUEE_COULE
		(au plus 1 tel changement eu cours de partie, phase de jeu)
	- le type de bateau est initialement BATEAU_TYPE_CLE_NONE, puis peut devenir une constante de [BATEAU_TYPE_CLE_NONE +1,BATEAU_TYPE_CLE_NB -1]
		(au plus 1 changement eu cours de partie, phase de placement pour le joueur, phase de jeu pour son adversaire) 
	- l'identifiant du bateau est initialement CASE_GRILLE_BATEAU_NONE, puis peut devenir une valeur entiere >= 0
		(au plus 1 changement eu cours de partie, phase de placement pour le joueur)

	Coherence (fonctionnelle) :
	- si une case a un type de bateau <> BATEAU_TYPE_CLE_NONE, son etat doit etre CASE_GRILLE_ETAT_NONE, CASE_GRILLE_ETAT_TOUCHE ou CASE_GRILLE_ETAT_COULE 
	- si une case a un type de bateau == BATEAU_TYPE_CLE_NONE, son etat doit etre CASE_GRILLE_ETAT_NONE ou CASE_GRILLE_ETAT_JOUEE_EAU
	- si une case a un identifiant de bateau <> CASE_GRILLE_BATEAU_NONE, elle doit avoir un type a valeur dans [BATEAU_TYPE_CLE_NONE +1,BATEAU_TYPE_CLE_NB -1]

	/!\ 
		Grille joueur :
		- A l'issue de la phase de placement : le type de bateau et son identifiants sont connus
				=> accesseur case_grille_set_bateau
		- Phase de jeu : lorsqu'un coup est porte, le joueur sait en quel etat faire evoluer la case (+ cases adjacentes si bateau coule)
				=> accesseur case_grille_set_etat

		Grille adverdaire :
		- Phase de jeu : lorsqu'un coup est porte, le joueur prend connaissance de l'etat + type de bateau
				=> accesseurs case_grille_set_etat et case_grille_set_type_bateau

		La coherence fonctionnelle est assuree par les fonctions de joueur.h
	/!\
*/

#ifndef CASE_GRILLE_H
#define CASE_GRILLE_H

#include "bateau_type.h"

/* ____________________ Structures de donnees */

/* Type enumere indiquant l'etat d'une case	*/
enum e_case_grille_etat {
	CASE_GRILLE_ETAT_NONE = -1,
	CASE_GRILLE_ETAT_JOUEE_EAU,
	CASE_GRILLE_ETAT_JOUEE_TOUCHE,
	CASE_GRILLE_ETAT_JOUEE_COULE,
	CASE_GRILLE_ETAT_NB
};

/* Constante indiquant l'absence de bateau sur la case */
#define CASE_GRILLE_BATEAU_NONE -1

/* Type structure decrivant une case du jeu	*/

struct s_grille;						/* Structure parente definie dans un autre module	*/

struct s_case {
	const struct s_grille* parent;	/* reference de la grille a laquelle la case appartient									*/
	enum e_case_grille_etat etat;	/* etat joue ou non de la case															*/
	bateau_type_cle type_bateau;	/* type de bateau place sur une case (BATEAU_TYPE_CLE_NONE si pas de bateau sur la case)	*/
	int id_bateau_type;				/* identifiant du bateau du type (si plusieurs bateaux du meme type)						*/
};

/* Alias de type */

typedef enum e_case_grille_etat case_grille_etat;
typedef struct s_case case_grille;

/* ____________________ Fonctions de manipulation */

/*	Pre-conditions 	: aucune
	Post-conditions : case instanciee en une adresse memoire qui est renvoyee :
						 - l'etat de la case est initialise a CASE_GRILLE_ETAT_NONE
						 - le type du bateau place sur la case est initialise a BATEAU_TYPE_CLE_NONE
						 - l'identifiant du bateau du type place sur la case est initialise a CASE_GRILLE_BATEAU_NONE
	Gestion erreur	: Renvoie NULL si parent == NULL, ou parent != NULL mais echec allocation memoire (dans les 2 cas, un message d'erreur idoine est affiche)
*/
case_grille* case_grille_construire(const struct s_grille* parent);

/*	Pre-conditions 	: aucune
	Post-conditions : si *c est l'adresse d'une case instanciee, la memoire reservee en l'adresse indiquee dans *c est liberee et *c est affectee a NULL
*/
void case_grille_detruire(case_grille** c);

/*	Pre-conditions 	: c adresse d'une case
	Post-conditions : - l'etat de la case est initialise a CASE_GRILLE_ETAT_NONE
					  - le type du bateau place sur la case est initialise a BATEAU_TYPE_CLE_NONE
					  - l'identifiant du bateau du type place sur la case est initialise a CASE_GRILLE_BATEAU_NONE
*/
void case_grille_initialiser(case_grille* c);

/*	Pre-conditions 	: c adresse d'une case, etat_case a valeur dans [CASE_GRILLE_ETAT_NONE +1,CASE_GRILLE_ETAT_NB -1]
	Post-conditions : l'etat de la case est mis a jour a la valeur passee en parametre
*/
void case_grille_set_etat(case_grille* c, case_grille_etat etat_case);

/*	Pre-conditions 	: c adresse d'une case, cleType a valeur dans [BATEAU_TYPE_CLE_NONE +1,BATEAU_TYPE_CLE_NB -1]
	Post-conditions : type de bateau mis a jour a la valeur passee en parametre
*/
void case_grille_set_type_bateau(case_grille* c, bateau_type_cle cleType);

/*	Pre-conditions 	: c adresse d'une case, cleType a valeur dans [BATEAU_TYPE_CLE_NONE +1,BATEAU_TYPE_CLE_NB -1]
	Post-conditions : type de bateau et identifiant du bateau dans le type mis a jour aux valeurs passees en parametre
*/
void case_grille_set_bateau(case_grille* c, bateau_type_cle cleType, unsigned int indBateau);

/*	Pre-conditions 	: c adresse d'une case
	Post-conditions : aucune
*/
case_grille_etat case_grille_get_etat(case_grille* c);

/*	Pre-conditions 	: c adresse d'une case
	Post-conditions : aucune
*/
bateau_type_cle case_grille_get_type_bateau(case_grille* c);

/*	Pre-conditions 	: c adresse d'une case
	Post-conditions : aucune
*/
int case_grille_get_id_bateau_type(case_grille* c);

#endif

/* Une grille est caracterisee par :
	- ses dimensions, un nombre nbL de lignes et un nombre nbC de colonnes
	- le nombre de cases a trouver pour couler la flotte adverse 
	- nbL x nbC cases identifiees par leurs coordonnees (x,y) indiquant :
		- l'etat de la case qui peut etre : non jouee, jouee dans l'eau, jouee et touche, jouee et coule
			(la case est jouee par le joueur s'il s'agit de la grille adverse, par l'adversaire s'il s'agit de la grille du joueur)
		- si l'on sait qu'un bateau passe sur cette case : information donnee par le type (obligatoire) et l'identifiant du bateau dans le type (optionnel)

	Mode de gestion :
	- la grille connait toujours ses dimensions (mais la grille peut être redimensionnee)
	- la grille connait toujours le nombre de cases necessaires pour gagner (mais ce nombre peut etre mise a jour)
	- la grille construit (et initialise) ses cases a sa construction
	- l'etat de ses cases est toujours a valeur dans [CASE_GRILLE_ETAT_NONE, CASE_GRILLE_ETAT_NB -1]
	- le type de bateau de ses cases est toujours a valeur dans [BATEAU_TYPE_CLE_NONE,BATEAU_TYPE_CLE_NB -1]

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

	/!\ 
		Grille joueur :
		- A l'issue de la phase de placement : le type de bateau et son identifiant sont connus
			=> accesseur grille_joueur_set_case
		- Phase de jeu : lorsqu'un coup est porte, le joueur sait en quel etat faire evoluer la case (+ cases adjacentes si bateau coule)
			=> accesseur grille_joueur_jouer_coup

		Grille adverdaire :
		- Phase de jeu : lorsqu'il porte un coup, le joueur prend connaissance de l'etat + type de bateau de la case visee
			=> accesseur grille_adversaire_set_case

		La coherence fonctionnelle est assuree par les fonctions de joueur.h
	/!\
*/

#ifndef GRILLE_H
#define GRILLE_H

#include "case_grille.h"

/* ____________________ Structures de donnees */

/* ____ Grille __ */

/* Type structure decrivant une grille de jeu	*/
struct s_grille {
	unsigned int nbLignes;			/* nombre de lignes de la grille										*/
	unsigned int nbColonnes;			/* nombre de colonnes de la grille									*/
	unsigned int nbCasesBateau;		/* nombre de cases a toucher pour couler la flotte de l'adversaire	*/
	case_grille** tab_cases;			/* tableau de cases 													*/
};

/* Alias de type */

typedef struct s_grille grille;

/* ____________________ Fonctions de manipulation */

/*	Pre-conditions 	: nbCasesBateau <= nbL * nbC
	Post-conditions : grille instanciee en une adresse memoire qui est renvoyee
					  - le nombre de lignes, de colonnes, de cases a toucher de la grille sont affectes a nbL, nbC et nbCasesBateau
					  - le tableau des cases contient nbL x nbC cases initialisees
	Gestion erreur	: Renvoie NULL si echec allocation memoire (un message d'erreur est alors affiche)
*/
grille* grille_construire(unsigned int nbL, unsigned int nbC, unsigned int nbCasesBateau);

/*	Pre-conditions 	: si *g != NULL et si *g->tab_cases !=NULL, alors 
						(*g)->nbLignes * (*g)->nbColonnes est bien egal au nombre de cases construites dans (*g)->tab_cases adresse
	Post-conditions : si *g est l'adresse d'une grille instanciee, la memoire reservee en l'adresse indiquee dans *g est liberee et *g est affectee a NULL
*/
void grille_detruire(grille** g);

/*	Pre-conditions 	: g adresse d'une grille dont les dimensions sont connues et le tableau de cases cree
	Post-conditions : cases reinitialisees a etat CASE_GRILLE_ETAT_NONJOUEE et type_bateau BATEAU_TYPE_CLE_NONE (dimensions grille inchangee)
*/
void grille_initialiser(grille* g);

/*	Pre-conditions 	: g adresse d'une grille verifiant
						  g->nbLignes * g->nbColonnes == 0 et g->tab_cases == NULL,
						  ou g->nbLignes * g->nbColonnes > 0 et g->nbLignes * g->nbColonnes == nombre de cases construites dans g->tab_cases
	Post-conditions : dimensions et tableau des cases de la grille mises a jour
					  les cases (initiales comme novuelles) sont toutes sinitialisees
	Gestion erreur	: Si echec allocation memoire (un message d'erreur est alors affiche), les dimensions intiales sont maintenues
*/
void grille_dimensionner(grille* g, unsigned int nbL, unsigned int nbC);

/*	Pre-conditions 	: g adresse d'une grille, nbCasesBateau <= g->nbLignes * g->nbColonnes
	Post-conditions : nombre de cases a toucher mis a jour a la valeur passee en parametre
*/
void grille_set_dimensionJeu(grille* g, unsigned int nbCasesBateau);

/*	Pre-conditions 	: g adresse d'une grille
	Post-conditions : aucune
*/
unsigned int grille_get_nbLignes(grille* g);

/*	Pre-conditions 	: g adresse d'une grille
	Post-conditions : aucune
*/
unsigned int grille_get_nbColonnes(grille* g);

/*	Pre-conditions 	: g adresse d'une grille
	Post-conditions : aucune
*/
unsigned int grille_get_dimensionJeu(grille* g);

/*	Pre-conditions 	: g adresse d'une grille totalement instanciee,
						x et y a valeur dans respectivement [0,g->nbLignes -1] et [0,g->nbColonnes -1]
	Post-conditions : aucune
*/
case_grille_etat grille_get_case_etat(grille* g, unsigned int x, unsigned int y);

/*	Pre-conditions 	: g adresse d'une grille totalement instanciee,
						x et y a valeur dans respectivement [0,g->nbLignes -1] et [0,g->nbColonnes -1]
	Post-conditions : aucune
*/
bateau_type_cle grille_get_case_type_bateau(grille* g, unsigned int x, unsigned int y);

/*	Pre-conditions 	: g adresse d'une grille totalement instanciee,
						x et y a valeur dans respectivement [0,g->nbLignes -1] et [0,g->nbColonnes -1]
	Post-conditions : aucune
*/
int grille_get_case_id_bateau_type(grille* g, unsigned int x, unsigned int y);

/*	Pre-conditions 	: g adresse d'une grille totalement instanciee
	Post-conditions : aucune
	Valeur retour	: le nombre de cases jouees
*/
unsigned int grille_get_nbCoupsJoues(grille* g);

/*	Pre-conditions 	: g adresse d'une grille totalement instanciee
	Post-conditions : aucune
	Valeur retour	: le nombre de cases touchees ou coulees, soit le nombre de case conjointement jouees et occupees par un bateau
*/
unsigned int grille_get_nbTouche(grille* g);

/*	Pre-conditions 	: g adresse d'une grille totalement instanciee
	Post-conditions : aucune
	Valeur retour	: 1 si le nombre de cases touchees ou coulees == g->nbCasesBateau, 0 sinon
*/
unsigned int grille_is_coule(grille* g);

/*	Pre-conditions 	: g adresse d'une grille totalement instanciee,
						x a valeur dans [0,j->grille_joueur->nbLignes -1], y a valeur dans [0,j->grille_joueur->nbColonnes -1],
						cleType a valeur dans [BATEAU_TYPE_CLE_NONE +1, BATEAU_TYPE_CLE_NB -1]
	Valeur retour	: PLACEMENT_TYPE_NONE si aucune orientation n'est possible depuis la case coordnnees (x,y),
						PLACEMENT_TYPE_H si seule une orientation horizontale est possible depuis la case coordnnees (x,y),
						PLACEMENT_TYPE_V si seule une orientation verticale est possible depuis la case coordnnees (x,y),
						PLACEMENT_TYPE_NB si les 2 orientations sont possibles depuis ces coordonnees
	Utilisation dans le jeu : fonction a appeler sur la grille du joueur exclusivement, phase de placement
*/
int grille_joueur_get_placement_valide(grille* g, bateau_type_cle cleType, unsigned int x, unsigned int y);

/*	Pre-conditions 	: g adresse d'une grille totalement instanciee,
						x et y a valeur dans respectivement [0,g->nbLignes -1] et [0,g->nbColonnes -1],
					  	cleType a valeur dans [BATEAU_TYPE_CLE_NONE,BATEAU_TYPE_CLE_NB -1]
	Post-conditions : le type de bateau de la case de cordonnees (x,y) est mis a jour a la valeur passee en parametre
	Utilisation dans le jeu : fonction a appeler sur la grille du joueur exclusivement, phase de placement
*/
void grille_joueur_set_case(grille* g, bateau_type_cle cleType, unsigned int indBateau, unsigned int x, unsigned int y);

/*	Pre-conditions 	: g adresse d'une grille du joueur totalement instanciee,
						x et y a valeur dans respectivement [0,g->nbLignes -1] et [0,g->nbColonnes -1]
	Post-conditions : l'etat de la case de cordonnees (x,y) est mis a jour en fonction des informations de la grille :
						- si la case n'est pas occupee par un bateau, l'etat devient CASE_GRILLE_ETAT_JOUEE_EAU
						- si la case est occupee par un bateau qui n'est pas coule a l'issue de ce coup, l'etat devient CASE_GRILLE_ETAT_JOUEE_TOUCHE
						- si la case est occupee par un bateau qui est coule a l'issue de ce coup, l'etat devient CASE_GRILLE_ETAT_JOUEE_COULE
	Utilisation dans le jeu : fonction a appeler sur la grille du joueur exclusivement, phase de jeu
*/
void grille_joueur_jouer_coup(grille* g, unsigned int x, unsigned int y);

/*	Pre-conditions 	: g adresse d'une grille totalement instanciee, x et y a valeur dans respectivement [0,g->nbLignes -1] et [0,g->nbColonnes -1],
						cleType a valeur dans [BATEAU_TYPE_CLE_NONE,BATEAU_TYPE_CLE_NB -1] et
					  	etat_case a valeur dans [CASE_GRILLE_ETAT_NONE +1,CASE_GRILLE_ETAT_NB -1] verifiant
							cleType == BATEAU_TYPE_CLE_NONE ssi etat == CASE_GRILLE_ETAT_JOUEE_EAU
	Post-conditions : l'etat et le type de bateau de la case de cordonnees (x,y) sont mises a jour aux valeurs passees en parametre
	Utilisation dans le jeu : fonction a appeler sur la grille de l'adversaire exclusivement, phase de jeu
*/
void grille_adversaire_set_case(grille* g, bateau_type_cle cleType, case_grille_etat etat_case, unsigned int x, unsigned int y);

#endif

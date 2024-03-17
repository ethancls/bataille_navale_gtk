/*
	Une flotte est caracterisee par :
	- ses dimensions ssi un nombre de bateaux pour chaque type de bateaux
	- le placement de ces bateaux

	Mode de gestion :
	- la flotte connait toujours ses dimensions (mais la flotte peut être redimensionnee)
	- la flotte construit (et initialise) les placements de ses bateaux a sa construction

	/!\ Cohérence (technique) :
		si f designe l'adresse d'une flotte, pour tout indice indType dans [0,FLOTTE_NB_TYPE_BATEAU],
			f->tab_nb_par_type[indType] et le nombre de placements construits dans f->tab_placements[indType] coincident toujours
	/!\

	/!\ Cohérence (fonctionnelle) : 
		- La coherence des placements des bateaux relativement a une grille de jeur est de la responsabilite de joueur.h
		- Les chevauchements entre bateaux (independants d'une grille) n'est pas non plus geree par flotte.h
	/!\
*/

#ifndef FLOTTE_H
#define FLOTTE_H

#include "bateau_type.h"
#include "placement.h"

/* nombre de types de bateaux dans la flotte */
#define FLOTTE_NB_TYPE_BATEAU (BATEAU_TYPE_CLE_NB -BATEAU_TYPE_CLE_NONE -1)

/* ____________________ Structures de donnees */

/* Constante symbolique indiquant l'usage des parametres par defaut	*/
#define FLOTTE_MODE_DEFAULT 0

/* Type structure decrivant une flotte de bateaux	*/
struct s_flotte {
	unsigned int tab_nb_par_type[FLOTTE_NB_TYPE_BATEAU];	/* Dimension de la flotte (nombre de bateaux de chaque type) 	*/
	placement** tab_placements[FLOTTE_NB_TYPE_BATEAU];		/* Placements des bateaux par type de bateaux					*/
};

/* Alias de type */

typedef struct s_flotte flotte;

/* ____________________ Fonctions de manipulation */

/*	Pre-conditions 	: aucune
	Post-conditions : flotte instanciee en une adresse memoire qui est renvoyee
	Gestion erreur	: Renvoie NULL si echec allocation memoire (un message d'erreur est alors affiche)
*/
flotte* flotte_construire(unsigned int tab_nb_par_type[FLOTTE_NB_TYPE_BATEAU]);

/*	Pre-conditions 	: si *f != NULL, *f instanciee de facon coherente
	Post-conditions : si *f est l'adresse d'une flotte instanciee, la memoire reservee en l'adresse contenue dans *f est liberee et *f est reinitalisee a NULL
*/
void flotte_detruire(flotte** f);

/*	Pre-conditions 	: f adresse d'une flotte totalement instanciee
	Post-conditions : flotte redimensionnee et reinitialisee
	Gestion erreur	: Renvoie NULL si echec allocation memoire (un message d'erreur est alors affiche)
*/
flotte* flotte_dimensionner(flotte* f, unsigned int tab_nb_par_type[FLOTTE_NB_TYPE_BATEAU]);

/*	Pre-conditions 	: f adresse d'une flotte totalement instanciee
	Post-conditions : initialise les placements des bateaux de la flotte (placements indefinis)
*/
void flotte_initialiser(flotte* f);

/*	Pre-conditions 	: f adresse d'une flotte dont les dimensions dont connues
	Post-conditions : aucune
	Valeur retour	: nombre de bateaux de la flotte
*/
unsigned int flotte_get_nb_bateaux(flotte* f);

/*	Pre-conditions 	: f adresse d'une flotte dont les dimensions dont connues
	Post-conditions : aucune
	Valeur retour	: somme des tailles des bateaux de la flotte
*/
unsigned int flotte_get_taille(flotte* f);

/*	Pre-conditions 	: f adresse d'une flotte dont les dimensions dont connues
	Post-conditions : aucune
*/
unsigned int flotte_get_type_nb_bateaux(flotte* f, bateau_type_cle cleType);

/*	Pre-conditions 	: f adresse d'une flotte totalement instanciee,
						cleType a valeur dans [BATEAU_TYPE_CLE_NONE +1, BATEAU_TYPE_CLE_NB -1],
						indBateau a valeur dans [0,f->tab_nb_par_type[cleType -BATEAU_TYPE_CLE_NONE -1] -1],
						orientation a valeur dans {PLACEMENT_TYPE_H,PLACEMENT_TYPE_V}
	Post-conditions : le placement du bateau de type cleType d'indice indBateau a ete mis a jour aux valeurs passees en parametre
*/
void flotte_set_bateau_position(flotte* f, bateau_type_cle cleType, unsigned int indBateau, unsigned int x, unsigned int y, placement_type orientation);

/*	Pre-conditions 	: f adresse d'une flotte totalement instanciee,
						cleType a valeur dans [BATEAU_TYPE_CLE_NONE +1, BATEAU_TYPE_CLE_NB -1],
						indBateau a valeur dans [0,f->tab_nb_par_type[cleType -BATEAU_TYPE_CLE_NONE -1] -1],
						xD, yD, xF, yF adresses d'un entier
	Post-conditions : Si bateau non place : *x_1 == *y_1 == *x_2 == *y_2 == PLACEMENT_COORD_NONE
					  Sinon, si L designe la longueur des bateaux du type identifie par cleType :
						(*xD,*yD) == coordonnees du placement du bateau de la flotte designe par cleType et indBateau,
						(*xF,*yF) == (xD+L,yD) ou (xD,yD+L) selon l'orientation du bateau
*/
void flotte_get_bateau_position(flotte* f, bateau_type_cle cleType, unsigned int indBateau, int* xD, int* yD, int* xF, int* yF);

/*	Pre-conditions 	: f adresse d'une flotte totalement instanciee,
						cleType a valeur dans [BATEAU_TYPE_CLE_NONE +1, BATEAU_TYPE_CLE_NB -1],
						indBateau a valeur dans [0,f->tab_nb_par_type[cleType -BATEAU_TYPE_CLE_NONE -1] -1]
	Valeur retour	: 1 si le bateau est place, 0 sinon
*/
int flotte_is_bateau_en_mer(flotte* f, bateau_type_cle cleType, unsigned int indBateau);

/*	Pre-conditions 	: f adresse d'une flotte totalement instanciee
	Post-conditions : aucune
*/
unsigned int flotte_get_nb_bateaux_en_mer(flotte* f);

#endif

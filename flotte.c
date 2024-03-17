#include "global.h"				/* pour la gestion des erreurs				*/

#include "flotte.h"

#include <stdio.h>				/* pour les messages d'erreur				*/
#include <stdlib.h>				/* pour allocation memoire					*/

/* ____________________ Declaration des fonctions statiques (sous-routines) */

/*	Pre-conditions 	: f adresse d'une flotte instanciee de facon coherente, indType a valeur dans [0,FLOTTE_NB_TYPE_BATEAU -1]
	Post-conditions : (re)construction et (re)initialisation de f->tab_placements[indType] selon nombre nbBateauType passe en parametre,
					  	f->tab_nb_par_type[indType] mis a jour a nbBateauType
	Retour			: STATUT_REUSSITE si reussite, STATUT_ERREUR_MEMOIRE si echec -- allocation memoire
*/
static int flotte_tab_placement_type_dimensionner(flotte* f, unsigned int indType, unsigned int nbBateauType);

/*	Pre-conditions 	: f adresse d'une flotte instanciee de facon coherente, indType a valeur dans [0,FLOTTE_NB_TYPE_BATEAU -1], nbBateauPlus > 0
	Post-conditions : tableau f->tab_placements[indType] mis a jour,
					  les nbBateauPlus derniers placements de ce f->tab_placements[indType] viennent sont construits et initialises,
					  f->tab_nb_par_type[indType] mis a jour
*/
static int flotte_tab_placement_type_ajouter(flotte* f, unsigned int indType, unsigned int nbBateauPlus);

/*	Pre-conditions 	: f adresse d'une flotte instanciee de facon coherente, indType a valeur dans [0,FLOTTE_NB_TYPE_BATEAU -1], 
						0< nbBateauMoins <= f->tab_nb_par_type[indType]
	Post-conditions : les nbBateauMoins derniers placements de f->tab_placements[indType] sont detruits,
					  tableau f->tab_placements[indType] mis a jour, 
					  f->tab_nb_par_type[indType] mis a jour
*/
static void flotte_tab_placement_type_supprimer(flotte* f, unsigned int indType, unsigned int nbBateauMoins);

/*	Pre-conditions 	: aucune
	Post-conditions 	: aucune
	Retour 			: (cleType -BATEAU_TYPE_CLE_NONE -1) si cleType a valeur dans [BATEAU_TYPE_CLE_NONE +1, BATEAU_TYPE_CLE_NB -1],
					  FLOTTE_NB_TYPE_BATEAU (un message s'affiche alors) sinon
*/
static unsigned int flotte_get_indice_type_from_cle(bateau_type_cle cleType);

/* ____________________ Definition des fonctions de manipulation */

/*	Pre-conditions 	: aucune
	Post-conditions : flotte instanciee en une adresse memoire qui est renvoyee
	Gestion erreur	: Renvoie NULL si echec allocation memoire (un message d'erreur est alors affiche)
*/
flotte* flotte_construire(unsigned int tab_nb_par_type[FLOTTE_NB_TYPE_BATEAU]) {
	flotte* f = NULL;
	unsigned int indType;

	/* __ Allocation flotte */
	f=(flotte*)malloc(sizeof(flotte));

	if (f != NULL) {
		/* __ Initialisation des champs de la flotte */
		for (indType =0 ; indType <FLOTTE_NB_TYPE_BATEAU ; indType ++) {
			f->tab_placements[indType] = NULL;
			f->tab_nb_par_type[indType] = 0;
		}

		/* __ Construction & initialisation des bateaux (leurs placements) de la flotte */
		f =flotte_dimensionner(f, tab_nb_par_type);
	}

	/* message en cas de probleme allocation memoire rencontre */	
	if (f == NULL) {
		printf("%s::%s\n", __func__, STR_STATUT_ERREUR_MEMOIRE);
	}

	return f;
}

/*	Pre-conditions 	: si *f != NULL, *f instanciee de facon coherente
	Post-conditions : si *f est l'adresse d'une flotte instanciee, la memoire reservee en l'adresse contenue dans *f est liberee et *f est reinitalisee a NULL
*/
void flotte_detruire(flotte** f) {
	unsigned int indType;

	if (*f != NULL) {
		/* destruction des tableaux de placements */
		for (indType =0 ; indType <FLOTTE_NB_TYPE_BATEAU ; indType ++) {
			flotte_tab_placement_type_dimensionner((*f), indType, 0);
		}

		/* destruction de la flotte */
		free(*f);
		*f=NULL;
	}
}

/*	Pre-conditions 	: f adresse d'une flotte instanciee de facon coherente, indType a valeur dans [0,FLOTTE_NB_TYPE_BATEAU -1]
	Post-conditions : (re)construction et (re)initialisation de f->tab_placements[indType] selon nombre nbBateauType passe en parametre,
					  	f->tab_nb_par_type[indType] mis a jour a nbBateauType
	Retour			: STATUT_REUSSITE si reussite, STATUT_ERREUR_MEMOIRE si echec -- allocation memoire
*/
static int flotte_tab_placement_type_dimensionner(flotte* f, unsigned int indType, unsigned int nbBateauType) {
	int statut = STATUT_REUSSITE;
	unsigned int indBateau;

	#if (FLOTTE_COMPILATION == 1)
	printf("%s(%p,%u,%u)::IN -- dimension intiale f->tab_nb_par_type[%u] == %u\n", __func__, (void*)f, indType, nbBateauType, indType, f->tab_nb_par_type[indType]);
	#endif

	/* __ Reinitialisation des placements des bateaux qui restent dans la flotte */
	for (indBateau =0 ; indBateau <f->tab_nb_par_type[indType] && indBateau <nbBateauType ; indBateau ++) {
		placement_initialiser(f->tab_placements[indType][indBateau]);
	}

	/* __ Si nbBateauType == f->tab_nb_par_type[indType] : rien a faire de plus */

	/* __ Sinon Si nbBateauType < f->tab_nb_par_type[indType] : des placements sont a detruire */
	if (nbBateauType < f->tab_nb_par_type[indType]) {
		flotte_tab_placement_type_supprimer(f, indType, f->tab_nb_par_type[indType] -nbBateauType);
	}
	/* __ Sinon Si nbBateauType < f->tab_nb_par_type[indType] : des placements sont a ajouter */
	else if (nbBateauType > f->tab_nb_par_type[indType]) {
		statut = flotte_tab_placement_type_ajouter(f, indType, nbBateauType -f->tab_nb_par_type[indType]);
	}

	#if (FLOTTE_COMPILATION == 1)
	printf("%s(%p,%u,%u)::OUT: statut = %d -- dimension finale f->tab_nb_par_type[%u] == %u\n", __func__, (void*)f, indType, nbBateauType, statut, indType, f->tab_nb_par_type[indType]);
	#endif

	return statut;
}

/*	Pre-conditions 	: f adresse d'une flotte instanciee de facon coherente, indType a valeur dans [0,FLOTTE_NB_TYPE_BATEAU -1], nbBateauPlus > 0
	Post-conditions : tableau f->tab_placements[indType] mis a jour,
					  les nbBateauPlus derniers placements de ce f->tab_placements[indType] viennent sont construits et initialises,
					  f->tab_nb_par_type[indType] mis a jour
*/
static int flotte_tab_placement_type_ajouter(flotte* f, unsigned int indType, unsigned int nbBateauPlus) {
	int statut = STATUT_REUSSITE;
	unsigned int i;
	placement** old = f->tab_placements[indType];

	#if (FLOTTE_COMPILATION == 1)
	printf("%s(%p,%u,%u)::IN\n", __func__, (void*)f, indType, nbBateauPlus);
	#endif

	/* __ (re)allocation du tableau de placements */
	f->tab_placements[indType] = (placement**) realloc (f->tab_placements[indType], (f->tab_nb_par_type[indType] +nbBateauPlus) * sizeof(placement*));

	/* gestion erreur (re)alloc */
	if (f->tab_placements[indType] == NULL) {
		statut = STATUT_ERREUR_MEMOIRE;
		f->tab_placements[indType] =old;
	}
	else {
		/* __ construction & initialisation des placements en plus */

		/* construction & initialisation des placements des bateaux */
		for (i = 1 ; i <= nbBateauPlus && statut == STATUT_REUSSITE ; i ++) {
			f->tab_placements[indType][f->tab_nb_par_type[indType]] = placement_construire();

			if (f->tab_placements[indType][f->tab_nb_par_type[indType]] == NULL) {
				statut = STATUT_ERREUR_MEMOIRE;
			}
			else {
				f->tab_nb_par_type[indType] ++;
			}
		}
	}

	#if (FLOTTE_COMPILATION == 1)
	printf("%s(%p,%u,%u)::OUT: statut = %d\n", __func__, (void*)f, indType, nbBateauPlus, statut);
	#endif

	return statut;
}

/*	Pre-conditions 	: f adresse d'une flotte instanciee de facon coherente, indType a valeur dans [0,FLOTTE_NB_TYPE_BATEAU -1], 
						0< nbBateauMoins <= f->tab_nb_par_type[indType]
	Post-conditions : les nbBateauMoins derniers placements de f->tab_placements[indType] sont detruits,
					  tableau f->tab_placements[indType] mis a jour, 
					  f->tab_nb_par_type[indType] mis a jour
*/
static void flotte_tab_placement_type_supprimer(flotte* f, unsigned int indType, unsigned int nbBateauMoins) {
	unsigned int i;
	placement** old = f->tab_placements[indType];

	#if (FLOTTE_COMPILATION == 1)
	printf("%s(%p,%u,%u)::IN\n", __func__, (void*)f, indType, nbBateauMoins);
	#endif

	/* __ suppression des placements des bateaux en trop sont detruits */
	for (i = 1 ; i <= nbBateauMoins ; i ++) {
		placement_detruire(& f->tab_placements[indType][f->tab_nb_par_type[indType] -1]);
		f->tab_nb_par_type[indType] --;
	}

	/* le tableau de placements pour le type de bateaux est redimensionne */
	f->tab_placements[indType] = (placement**) realloc (f->tab_placements[indType], f->tab_nb_par_type[indType] * sizeof(placement*));

	/* gestion erreur realloc */
	if (f->tab_nb_par_type[indType] >0 && f->tab_placements[indType] == NULL) {
		f->tab_placements[indType] = old;
	}

	#if (FLOTTE_COMPILATION == 1)
	printf("%s(%p,%u,%u)::OUT\n", __func__, (void*)f, indType, nbBateauMoins);
	#endif
}

/*	Pre-conditions 	: f adresse d'une flotte totalement instanciee
	Post-conditions : flotte redimensionnee et reinitialisee
	Gestion erreur	: Renvoie NULL si echec allocation memoire (un message d'erreur est alors affiche)
*/
flotte* flotte_dimensionner(flotte* f, unsigned int tab_nb_par_type[FLOTTE_NB_TYPE_BATEAU]) {
	int statut = STATUT_REUSSITE;
	unsigned int indType;

	#if (FLOTTE_COMPILATION == 1)
	printf("%s(%p,%p)::IN\n", __func__, (void*)f, (void*)tab_nb_par_type);
	#endif

	/* Construction & initialisation des placements de la flotte */
	for (indType =0 ; indType < FLOTTE_NB_TYPE_BATEAU && statut == STATUT_REUSSITE ; indType ++) {
		statut = flotte_tab_placement_type_dimensionner(f,indType,tab_nb_par_type[indType]);
	}

	/* gestion pb memoire */
	if (statut != STATUT_REUSSITE) {
		flotte_detruire(& f);
		printf("%s::%s\n", __func__, STR_STATUT_ERREUR_MEMOIRE);
	}

	#if (FLOTTE_COMPILATION == 1)
	printf("%s(%p,%p)::OUT\n", __func__, (void*)f, (void*)tab_nb_par_type);
	#endif

	return f;
}

/*	Pre-conditions 	: f adresse d'une flotte totalement instanciee
	Post-conditions : initialise les placements des bateaux de la flotte (placements indefinis)
*/
void flotte_initialiser(flotte* f) {
	unsigned int indType, indBateau;

	/* Parcours de la flotte */
	for (indType = 0 ; indType < FLOTTE_NB_TYPE_BATEAU ; indType ++) {
		for (indBateau = 0 ; indBateau < f->tab_nb_par_type[indType] ; indBateau ++) {
			placement_initialiser(f->tab_placements[indType][indBateau]);
		}
	}
}

/*	Pre-conditions 	: aucune
	Post-conditions 	: aucune
	Retour 			: (cleType -BATEAU_TYPE_CLE_NONE -1) si cleType a valeur dans [BATEAU_TYPE_CLE_NONE +1, BATEAU_TYPE_CLE_NB -1],
					  FLOTTE_NB_TYPE_BATEAU (un message s'affiche alors) sinon
*/
static unsigned int flotte_get_indice_type_from_cle(bateau_type_cle cleType) {
	if (cleType >= BATEAU_TYPE_CLE_NONE +1 && cleType <= BATEAU_TYPE_CLE_NB -1)
		return (unsigned int) (cleType -BATEAU_TYPE_CLE_NONE -1);

	printf("%s::%s: cleType=%d d'intervalle requis [%d,%d]\n", __FILE__, STR_STATUT_ERREUR_DEPASSEMENT_INDICE, cleType, BATEAU_TYPE_CLE_NONE +1, BATEAU_TYPE_CLE_NB -1);
	return FLOTTE_NB_TYPE_BATEAU;
}

/*	Pre-conditions 	: f adresse d'une flotte dont les dimensions dont connues
	Post-conditions : aucune
	Valeur retour	: nombre de bateaux de la flotte
*/
unsigned int flotte_get_nb_bateaux(flotte* f) {
	unsigned int nb=0;
	bateau_type_cle cleType;

	for (cleType =BATEAU_TYPE_CLE_NONE +1 ; cleType <= BATEAU_TYPE_CLE_NB -1 ; cleType ++) {
		nb = nb +flotte_get_type_nb_bateaux(f, cleType);
	}
	
	return nb;
}

/*	Pre-conditions 	: f adresse d'une flotte dont les dimensions dont connues
	Post-conditions : aucune
	Valeur retour	: somme des tailles des bateaux de la flotte
*/
unsigned int flotte_get_taille(flotte* f) {
	unsigned int indType, taille=0;
	bateau_type_cle cleType;

	for (cleType =BATEAU_TYPE_CLE_NONE +1 ; cleType <= BATEAU_TYPE_CLE_NB -1 ; cleType ++) {
		indType = flotte_get_indice_type_from_cle (cleType);
		taille = taille +(f->tab_nb_par_type[indType] * bateau_type_get_longueur(cleType)) ;
	}
	
	return taille;
}

/*	Pre-conditions 	: f adresse d'une flotte, cleType a valeur dans [BATEAU_TYPE_CLE_NONE +1, BATEAU_TYPE_CLE_NB -1],
	Post-conditions : aucune
*/
unsigned int flotte_get_type_nb_bateaux(flotte* f, bateau_type_cle cleType) {
	unsigned int indType = flotte_get_indice_type_from_cle(cleType);

	return f->tab_nb_par_type[indType];
}

/*	Pre-conditions 	: f adresse d'une flotte totalement instanciee,
						cleType a valeur dans [BATEAU_TYPE_CLE_NONE +1, BATEAU_TYPE_CLE_NB -1],
						indBateau a valeur dans [0,f->tab_nb_par_type[cleType -BATEAU_TYPE_CLE_NONE -1] -1],
						orientation a valeur dans {PLACEMENT_TYPE_H,PLACEMENT_TYPE_V}
	Post-conditions : le placement du bateau de type cleType d'indice indBateau a ete mis a jour aux valeurs passees en parametre
*/
void flotte_set_bateau_position(flotte* f, bateau_type_cle cleType, unsigned int indBateau, unsigned int x, unsigned int y, placement_type orientation) {
	unsigned int indType = flotte_get_indice_type_from_cle(cleType);

	placement_set(f->tab_placements[indType][indBateau], x, y, orientation);
}

/*	Pre-conditions 	: f adresse d'une flotte totalement instanciee,
						cleType a valeur dans [BATEAU_TYPE_CLE_NONE +1, BATEAU_TYPE_CLE_NB -1],
						indBateau a valeur dans [0,f->tab_nb_par_type[cleType -BATEAU_TYPE_CLE_NONE -1] -1],
						xD, yD, xF, yF adresses d'un entier
	Post-conditions : Si bateau non place : *x_1 == *y_1 == *x_2 == *y_2 == PLACEMENT_COORD_NONE
					  Sinon, si L designe la longueur des bateaux du type identifie par cleType :
						(*xD,*yD) == coordonnees du placement du bateau de la flotte designe par cleType et indBateau,
						(*xF,*yF) == (xD+L,yD) ou (xD,yD+L) selon l'orientation du bateau
*/
void flotte_get_bateau_position(flotte* f, bateau_type_cle cleType, unsigned int indBateau, int* xD, int* yD, int* xF, int* yF) {
	unsigned int indType = flotte_get_indice_type_from_cle(cleType);

	/* Case (*xD,*yD) debut placement bateau : */
	*xD = placement_get_x(f->tab_placements[indType][indBateau]);
	*yD = placement_get_y(f->tab_placements[indType][indBateau]);

	/* Case (*xF,*yF) fin placement bateau : */
	placement_get_fin(f->tab_placements[indType][indBateau], bateau_type_get_longueur(cleType), xF, yF);
}

/*	Pre-conditions 	: f adresse d'une flotte totalement instanciee,
						cleType a valeur dans [BATEAU_TYPE_CLE_NONE +1, BATEAU_TYPE_CLE_NB -1],
						indBateau a valeur dans [0,f->tab_nb_par_type[cleType -BATEAU_TYPE_CLE_NONE -1] -1]
	Valeur retour	: 1 si le bateau est place, 0 sinon
*/
int flotte_is_bateau_en_mer(flotte* f, bateau_type_cle cleType, unsigned int indBateau) {
	unsigned int indType = flotte_get_indice_type_from_cle(cleType);

	return (placement_get_x(f->tab_placements[indType][indBateau]) != PLACEMENT_COORD_NONE);
}

/*	Pre-conditions 	: f adresse d'une flotte totalement instanciee
	Post-conditions : aucune
*/
unsigned int flotte_get_nb_bateaux_en_mer(flotte* f) {
	unsigned int indType, indBateau, nb=0;
	bateau_type_cle cleType;

	for (cleType =BATEAU_TYPE_CLE_NONE +1 ; cleType <= BATEAU_TYPE_CLE_NB -1 ; cleType ++) {
		indType = flotte_get_indice_type_from_cle (cleType);
		for (indBateau = 0 ; indBateau < f->tab_nb_par_type[indType] ; indBateau++) {
			nb = nb +flotte_is_bateau_en_mer(f, cleType, indBateau);
		}
	}
	
	return nb;
}

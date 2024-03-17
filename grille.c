#include "global.h"				/* pour la gestion des erreurs					*/

#include "grille.h"
#include "placement.h"			/* pour fonction grille_get_placement_valide		*/

#include <stdio.h>				/* pour les messages d'erreur					*/
#include <stdlib.h>				/* pour allocation memoire						*/

/* ____________________ Declaration des fonctions internes */

/*	Pre-conditions 	: g adresse d'une grille verifiant
						- g->nbLignes * g->nbColonnes == 0 et g->tab_cases == NULL,
						  ou g->nbLignes * g->nbColonnes > 0 et g->nbLignes * g->nbColonnes == nombre de cases construites dans g->tab_cases,
					  	- nbL*nbC > g->nbLignes * g->nbColonnes
	Post-conditions : dimensions et tableau des cases de la grille mises a jour selon les nouvelles dimensions nbL x nbC
						(les cases nouvellement crees de la grille sont alors initialisees)
	Gestion erreur	: Si echec allocation memoire, la configuration initiale est preservee (un message d'erreur est alors affiche)
*/
static void grille_augmenter(grille* g, unsigned int nbL, unsigned int nbC);

/*	Pre-conditions 	: g adresse d'une grille verifiant
						- g->nbLignes * g->nbColonnes == 0 et g->tab_cases == NULL,
						  ou g->nbLignes * g->nbColonnes > 0 et g->nbLignes * g->nbColonnes == nombre de cases construites dans g->tab_cases,
					  	- nbL*nbC < g->nbLignes * g->nbColonnes
	Post-conditions : dimensions et tableau des cases de la grille mises a jour selon les nouvelles dimensions nbL x nbC
						(les cases en trop dans la grille initiale sont detruites)
*/
static void grille_diminuer(grille* g, unsigned int nbL, unsigned int nbC);

/*	Pre-conditions 	: g adresse d'une grille dont les dimensions sont connues
	Post-conditions 	: aucune
	Retour 			: (x*g->nbColonnes)+y si les coordonnees (x,y) sont correctes ssi x,y a valeur dans resp. [0,g->nbLignes -1] et [0,g->nbColonnes -1],
					  -1 (un message s'affiche alors) sinon
*/
static int grille_get_indice_from_coordonnees(grille* g, unsigned int x, unsigned int y);

/*	Pre-conditions 	: g adresse d'une grille totalement instanciée, xD, yD, xF, yF adresses d'un entier,
						x a valeur dans [0, g->nbLignes -1], y a valeur dans [0, g->nbColonnes -1],
						la case de coordonnées (x, y) a un type de bateau dans [BATEAU_TYPE_CLE_NONE +1, BATEAU_TYPE_CLE_NB -1]
						et un identifiant de bateau <> CASE_GRILLE_BATEAU_NONE
	Post-conditions : (*xD,*yD) et (*xF,*yF) donnent les coordonnées de début et fin du placement du bateau situé sur la case (x, y)

	Utilisation dans le jeu : grille du joueur
*/
static void grille_get_bateau_position_from_case(grille* g, unsigned int x, unsigned int y, unsigned int *xD, unsigned int *yD, unsigned int *xF, unsigned int *yF);

/* ____________________ Definition des fonctions de manipulation */

/*	Pre-conditions 	: nbCasesBateau <= nbL * nbC
	Post-conditions : grille instanciee en une adresse memoire qui est renvoyee
					  - le nombre de lignes, de colonnes, de cases a toucher de la grille sont affectes a nbL, nbC et nbCasesBateau
					  - le tableau des cases contient nbL x nbC cases initialisees
	Gestion erreur	: Renvoie NULL si echec allocation memoire (un message d'erreur est alors affiche)
*/
grille* grille_construire(unsigned int nbL, unsigned int nbC, unsigned int nbCasesBateau) {
	grille* g = NULL;

	/* __ Allocation grille */
	g = (grille*) malloc (sizeof(grille));

	/* __ Initialisation des champs */
	if (g != NULL) {
		g->nbCasesBateau = nbCasesBateau;
		g->nbLignes = 0;
		g->nbColonnes = 0;
		g->tab_cases = NULL;

		/* __ Tableau des cases */
		grille_dimensionner(g, nbL, nbC);

		/* __ En cas d'echec, la grille est detruite */
		if (g->nbLignes * g->nbColonnes < nbL * nbC)
			grille_detruire( & g);
	}

	/* message en cas de probleme allocation memoire rencontre */	
	if (g == NULL)
		printf("%s::%s\n", __func__, STR_STATUT_ERREUR_MEMOIRE);

	return g;
}

/*	Pre-conditions 	: si *g != NULL et si *g->tab_cases !=NULL, alors 
						(*g)->nbLignes * (*g)->nbColonnes est bien egal au nombre de cases construites dans (*g)->tab_cases adresse
	Post-conditions : si *g est l'adresse d'une grille instanciee, la memoire reservee en l'adresse indiquee dans *g est liberee et *g est affectee a NULL
*/
void grille_detruire(grille** g) {
	unsigned int ind;

	if (*g != NULL) {
		if ((*g)->tab_cases != NULL) {
			/* Destruction des cases 1 a 1 */
			for (ind =0 ; ind < ((*g)->nbLignes) * ((*g)->nbColonnes) ; ind ++)
				case_grille_detruire( & ((*g)->tab_cases[ind]) );

			/* Destruction du tableau */
			free((*g)->tab_cases);
		}

		/* Destruction de la grille */
		free(*g);
		*g = NULL;
	}
}

/*	Pre-conditions 	: g adresse d'une grille verifiant
						- g->nbLignes * g->nbColonnes == 0 et g->tab_cases == NULL,
						  ou g->nbLignes * g->nbColonnes > 0 et g->nbLignes * g->nbColonnes == nombre de cases construites dans g->tab_cases,
					  	- nbL*nbC > g->nbLignes * g->nbColonnes
	Post-conditions : dimensions et tableau des cases de la grille mises a jour selon les nouvelles dimensions nbL x nbC
						(les cases nouvellement crees de la grille sont alors initialisees)
	Gestion erreur	: Si echec allocation memoire, la configuration initiale est preservee (un message d'erreur est alors affiche)
*/
static void grille_augmenter(grille* g, unsigned int nbL, unsigned int nbC) {
	int statut = STATUT_REUSSITE;
	unsigned int ind, nb;
	case_grille** old = g->tab_cases;

	#if (GRILLE_COMPILATION == 1)
	printf("%s(%p, %u, %u)::IN : (g->nbLignes,g->nbColonnes) == (%u, %u)\n", __func__, (void*)g, nbL, nbC, g->nbLignes, g->nbColonnes);
	#endif

	/* __ Il faut d'abord (re)dimensionner le tableau */
	g->tab_cases = (case_grille**) realloc (g->tab_cases, nbL*nbC*sizeof(case_grille*));

	/* En cas d'echec : les dimensions initiales sont preservees */
	if (g->tab_cases == NULL) {
		printf("%s::%s -- maintien des dimensions initiales %u x %u\n", __func__, STR_STATUT_ERREUR_MEMOIRE, g->nbLignes, g->nbColonnes);
		g->tab_cases = old;
	}
	else {
		/* __ Puis creer les nouvelles cases (ces cases sont alors initialisees) */
		for (ind =g->nbLignes * g->nbColonnes ; ind < nbL*nbC && statut == STATUT_REUSSITE; ind ++) {
			g->tab_cases[ind] = case_grille_construire(g);
			if (g->tab_cases[ind] == NULL)
				statut = STATUT_ERREUR_MEMOIRE;
		}

		/* En cas d'echec : les dimensions initiales sont preservees */
		if (statut == STATUT_ERREUR_MEMOIRE) {
			printf("%s::%s -- maintien des dimensions initiales %u x %u\n", __func__, STR_STATUT_ERREUR_MEMOIRE, g->nbLignes, g->nbColonnes);

			/* destruction des cases nouvellement construites */
			nb =ind;
			for (ind =g->nbLignes * g->nbColonnes ; ind <nb ; ind ++)
				case_grille_detruire( & (g->tab_cases[ind]) );

			/* reallocation du tableau a sa dimension initiale */
			g->tab_cases = (case_grille**) realloc (g->tab_cases, g->nbLignes*g->nbColonnes*sizeof(case_grille*));
			if (g->tab_cases == NULL)
				g->tab_cases = old;
		}
		else {
			/* __ Enfin mettre a jour les dimensions de la grille */
			g->nbLignes = nbL;
			g->nbColonnes = nbC;
		}
	}

	#if (GRILLE_COMPILATION == 1)
	printf("%s(%p, %u, %u)::OUT : (g->nbLignes,g->nbColonnes) == (%u, %u)\n", __func__, (void*)g, nbL, nbC, g->nbLignes, g->nbColonnes);
	#endif
}

/*	Pre-conditions 	: g adresse d'une grille verifiant
						- g->nbLignes * g->nbColonnes == 0 et g->tab_cases == NULL,
						  ou g->nbLignes * g->nbColonnes > 0 et g->nbLignes * g->nbColonnes == nombre de cases construites dans g->tab_cases,
					  	- nbL*nbC < g->nbLignes * g->nbColonnes
	Post-conditions : dimensions et tableau des cases de la grille mises a jour selon les nouvelles dimensions nbL x nbC
						(les cases en trop dans la grille initiale sont detruites)
*/
static void grille_diminuer(grille* g, unsigned int nbL, unsigned int nbC) {
	unsigned int ind;
	case_grille** old = g->tab_cases;

	#if (GRILLE_COMPILATION == 1)
	printf("%s(%p, %u, %u)::IN : (g->nbLignes,g->nbColonnes) == (%u, %u)\n", __func__, (void*)g, nbL, nbC, g->nbLignes, g->nbColonnes);
	#endif

	/* Destruction des cases en trop */
	for (ind = nbL*nbC ; ind < g->nbLignes * g->nbColonnes ; ind ++)
		case_grille_detruire(& (g->tab_cases[ind]));

	/* Dimensionnement du tableau et de la grille	*/
	g->tab_cases = (case_grille**) realloc (g->tab_cases, nbL*nbC*sizeof(case_grille*));
	if (g->tab_cases == NULL)
		g->tab_cases = old;

	g->nbLignes = nbL;
	g->nbColonnes = nbC;

	#if (GRILLE_COMPILATION == 1)
	printf("%s(%p, %u, %u)::OUT : (g->nbLignes,g->nbColonnes) == (%u, %u)\n", __func__, (void*)g, nbL, nbC, g->nbLignes, g->nbColonnes);
	#endif
}

/*	Pre-conditions 	: g adresse d'une grille verifiant
						  g->nbLignes * g->nbColonnes == 0 et g->tab_cases == NULL,
						  ou g->nbLignes * g->nbColonnes > 0 et g->nbLignes * g->nbColonnes == nombre de cases construites dans g->tab_cases
	Post-conditions : dimensions et tableau des cases de la grille mises a jour
					  les cases (initiales comme novuelles) sont toutes initialisees
	Gestion erreur	: Si echec allocation memoire (un message d'erreur est alors affiche), les dimensions intiales sont maintenues
*/
void grille_dimensionner(grille* g, unsigned int nbL, unsigned int nbC) {
	unsigned int ind;

	#if (GRILLE_COMPILATION == 1)
	printf("%s(%p, %u, %u)::IN : (g->nbLignes, g->nbColonnes) == (%u, %u)\n", __func__, (void*)g, nbL, nbC, g->nbLignes, g->nbColonnes);
	#endif

	/* __ Reinitialisation des cases persistantes de la grille */
	for (ind =0 ; ind < (g->nbLignes) * (g->nbColonnes) && ind < nbL * nbC ; ind ++)
		case_grille_initialiser(g->tab_cases[ind]);

	/* __ Le cas echeant (nouvelle grille et grille initiale de taille differente), redimensionnement du tableau des cases */
	if (g->nbLignes * g->nbColonnes < nbL * nbC)
		grille_augmenter(g, nbL, nbC);
	else if (g->nbLignes * g->nbColonnes > nbL * nbC)
		grille_diminuer(g, nbL, nbC);
	else {
		g->nbLignes =nbL;
		g->nbColonnes =nbC;
	}

	#if (GRILLE_COMPILATION == 1)
	printf("%s(%p, %u, %u)::OUT : (g->nbLignes, g->nbColonnes) == (%u, %u)\n", __func__, (void*)g, nbL, nbC, g->nbLignes, g->nbColonnes);
	#endif
}

/*	Pre-conditions 	: g adresse d'une grille, nbCasesBateau <= g->nbLignes * g->nbColonnes
	Post-conditions : nombre de cases a toucher mis a jour a la valeur passee en parametre
*/
void grille_set_dimensionJeu(grille* g, unsigned int nbCasesBateau) {
	g->nbCasesBateau = nbCasesBateau;
}

/*	Pre-conditions 	: g adresse d'une grille dont les dimensions sont connues et le tableau de cases cree 
	Post-conditions : cases reinitialisees a etat CASE_GRILLE_ETAT_NONJOUEE et type_bateau BATEAU_TYPE_CLE_NONE (dimensions grille inchangee)
*/
void grille_initialiser(grille* g) {
	unsigned int ind;

	/* Parcours des cases de la grille */
	for (ind =0 ; ind < (g->nbLignes) * (g->nbColonnes) ; ind ++)
		case_grille_initialiser(g->tab_cases[ind]);
}

/*	Pre-conditions 	: g adresse d'une grille
	Post-conditions : aucune
*/
unsigned int grille_get_nbLignes(grille* g) {
	return g->nbLignes;
}

/*	Pre-conditions 	: g adresse d'une grille
	Post-conditions : aucune
*/
unsigned int grille_get_nbColonnes(grille* g) {
	return g->nbColonnes;
}

/*	Pre-conditions 	: g adresse d'une grille
	Post-conditions : aucune
*/
unsigned int grille_get_dimensionJeu(grille* g) {
	return g->nbCasesBateau;
}

/*	Pre-conditions 	: g adresse d'une grille dont les dimensions sont connues
	Post-conditions 	: aucune
	Retour 			: (x*g->nbColonnes)+y si les coordonnees (x,y) sont correctes ssi x,y a valeur dans resp. [0,g->nbLignes -1] et [0,g->nbColonnes) -1],
					  -1 (un message s'affiche alors) sinon
*/
static int grille_get_indice_from_coordonnees(grille* g, unsigned int x, unsigned int y) {
	if (x <g->nbLignes && y <g->nbColonnes)
		return ( (x * g->nbColonnes) +y );

	printf("%s::%s: x =%u d'intervalle requis [0, %u[ et y =%u d'intervalle requis [0, %u[\n", __FILE__, STR_STATUT_ERREUR_DEPASSEMENT_INDICE, x, g->nbLignes, y, g->nbColonnes);
	return -1;
}

/*	Pre-conditions 	: g adresse d'une grille totalement instanciee,
						x et y a valeur dans respectivement [0,g->nbLignes -1] et [0,g->nbColonnes -1]
	Post-conditions : aucune
*/
case_grille_etat grille_get_case_etat(grille* g, unsigned int x, unsigned int y) {
	unsigned int ind =grille_get_indice_from_coordonnees(g, x, y);

	return case_grille_get_etat(g->tab_cases[ind]);
}

/*	Pre-conditions 	: g adresse d'une grille totalement instanciee,
						x et y a valeur dans respectivement [0,g->nbLignes -1] et [0,g->nbColonnes -1]
	Post-conditions : aucune
*/
bateau_type_cle grille_get_case_type_bateau(grille* g, unsigned int x, unsigned int y) {
	unsigned int ind =grille_get_indice_from_coordonnees(g, x, y);

	return case_grille_get_type_bateau(g->tab_cases[ind]);
}

/*	Pre-conditions 	: g adresse d'une grille totalement instanciee,
						x et y a valeur dans respectivement [0,g->nbLignes -1] et [0,g->nbColonnes -1]
	Post-conditions : aucune
*/
int grille_get_case_id_bateau_type(grille* g, unsigned int x, unsigned int y) {
	unsigned int ind =grille_get_indice_from_coordonnees(g, x, y);

	return case_grille_get_id_bateau_type(g->tab_cases[ind]);
}

/*	Pre-conditions 	: g adresse d'une grille totalement instanciee
	Post-conditions : aucune
	Valeur retour	: le nombre de cases jouees
*/
unsigned int grille_get_nbCoupsJoues(grille* g) {
	unsigned int ind, nb = 0;

	for (ind =0 ; ind < (g->nbLignes) * (g->nbColonnes) ; ind ++)
		nb = nb + ( case_grille_get_etat(g->tab_cases[ind]) != CASE_GRILLE_ETAT_NONE );

	return nb;
}

/*	Pre-conditions 	: g adresse d'une grille totalement instanciee
	Post-conditions : aucune
	Valeur retour	: le nombre de cases touchees ou coulees, soit le nombre de case conjointement jouees et occupees par un bateau
*/
unsigned int grille_get_nbTouche(grille* g) {
	unsigned int ind, nb = 0;

	for (ind =0 ; ind < (g->nbLignes) * (g->nbColonnes) ; ind ++)
		nb =nb +( (case_grille_get_type_bateau(g->tab_cases[ind]) >= BATEAU_TYPE_CLE_NONE +1)
					 && (case_grille_get_type_bateau(g->tab_cases[ind]) <= BATEAU_TYPE_CLE_NB -1)
					 && (case_grille_get_etat(g->tab_cases[ind]) != CASE_GRILLE_ETAT_NONE) );

	return nb;
}

/*	Pre-conditions 	: g adresse d'une grille totalement instanciee
	Post-conditions : aucune
	Valeur retour	: 1 si le nombre de cases touchees ou coulees == g->nbCasesBateau, 0 sinon
*/
unsigned int grille_is_coule(grille* g) {
	return (grille_get_nbTouche(g) == g->nbCasesBateau );
}

/*	Pre-conditions 	: g adresse d'une grille totalement instanciee,
						x a valeur dans [0,j->grille_joueur->nbLignes -1], y a valeur dans [0,j->grille_joueur->nbColonnes -1],
						cleType a valeur dans [BATEAU_TYPE_CLE_NONE +1, BATEAU_TYPE_CLE_NB -1]
	Valeur retour	: PLACEMENT_TYPE_NONE si aucune orientation n'est possible depuis la case coordnnees (x,y),
						PLACEMENT_TYPE_H si seule une orientation horizontale est possible depuis la case coordnnees (x,y),
						PLACEMENT_TYPE_V si seule une orientation verticale est possible depuis la case coordnnees (x,y),
						PLACEMENT_TYPE_NB si les 2 orientations sont possibles depuis ces coordonnees
	Utilisation dans le jeu : fonction a appeler sur la grille du joueur exclusivement, phase de placement
*/
int grille_joueur_get_placement_valide(grille* g, bateau_type_cle cleType, unsigned int x, unsigned int y) {
	int reponse =PLACEMENT_TYPE_NB, ok_H =1, ok_V =1;
	unsigned int xD=x, yD=y, xF, yF, longueur =bateau_type_get_longueur(cleType);

	/* test placement horizontal */
	xF =xD;	
	yF =yD +longueur -1;

	if (yF >= g->nbColonnes)
		ok_H =0;
	else
		for (y =yD ; (y <= yF) && ok_H ; y++)
			ok_H =(grille_get_case_type_bateau(g,xD,y) == BATEAU_TYPE_CLE_NONE);

	/* test placement vertical */
	xF = xD +longueur -1;
	yF = yD;

	if (xF >= g->nbLignes)
		ok_V =0;
	else
		for (x =xD ; (x <= xF) && ok_V ; x++)
			ok_V =(grille_get_case_type_bateau(g,x,yD) == BATEAU_TYPE_CLE_NONE);

	/* reponse */
	if (ok_H && ok_V)
		reponse = PLACEMENT_TYPE_NB;
	else if (ok_H && ! ok_V)
		reponse = PLACEMENT_TYPE_H;
	else if (! ok_H && ok_V)
		reponse = PLACEMENT_TYPE_V;
	else
		reponse = PLACEMENT_TYPE_NONE;

	return reponse;
}

/*	Pre-conditions 	: g adresse d'une grille totalement instanciee,
						x et y a valeur dans respectivement [0,g->nbLignes -1] et [0,g->nbColonnes -1],
					  	cleType a valeur dans [BATEAU_TYPE_CLE_NONE,BATEAU_TYPE_CLE_NB -1]
	Post-conditions : le type de bateau de la case de cordonnees (x,y) est mis a jour a la valeur passee en parametre
	Utilisation dans le jeu : fonction a appeler sur la grille du joueur exclusivement, phase de placement
*/
void grille_joueur_set_case(grille* g, bateau_type_cle cleType, unsigned int indBateau, unsigned int x, unsigned int y) {
	unsigned int ind = grille_get_indice_from_coordonnees(g,x,y);

	case_grille_set_bateau(g->tab_cases[ind], cleType, indBateau);
}

/*	Pre-conditions 	: g adresse d'une grille totalement instanciée, xD, yD, xF, yF adresses d'un entier,
						x a valeur dans [0, g->nbLignes -1], y a valeur dans [0, g->nbColonnes -1],
						la case de coordonnées (x, y) a un type de bateau dans [BATEAU_TYPE_CLE_NONE +1, BATEAU_TYPE_CLE_NB -1]
						et un identifiant de bateau <> CASE_GRILLE_BATEAU_NONE
	Post-conditions : (*xD,*yD) et (*xF,*yF) donnent les coordonnées de début et fin du placement du bateau situé sur la case (x, y)

	Utilisation dans le jeu : grille du joueur
*/
static void grille_get_bateau_position_from_case(grille* g, unsigned int x, unsigned int y, unsigned int *xD, unsigned int *yD, unsigned int *xF, unsigned int *yF) {
	unsigned int ind =grille_get_indice_from_coordonnees(g, x, y);
	bateau_type_cle cleType =case_grille_get_type_bateau(g->tab_cases[ind]);
	int idBateauType =case_grille_get_id_bateau_type(g->tab_cases[ind]);

	#if (GRILLE_COMPILATION != 0)
	printf("%s sur grille %p IN : (x, y)=(%u, %u)\n", __func__, (void*)g, x, y);
	#endif

	*xD = (unsigned int) x;
	*xF = (unsigned int) x;
	*yD = (unsigned int) y;
	*yF = (unsigned int) y;

	/* recherche en haut */
	for (x =*xD -1 ; x <g->nbLignes ; x --) {
		ind = grille_get_indice_from_coordonnees(g,x,*yD);
	 	if ( case_grille_get_type_bateau(g->tab_cases[ind]) != cleType || case_grille_get_id_bateau_type(g->tab_cases[ind]) != idBateauType )
			break;

		*xD = *xD -1;
	}

	/* recherche en bas */
	for (x =*xF +1 ; x <g->nbLignes ; x ++) {
		ind = grille_get_indice_from_coordonnees(g,x,*yD);
	 	if ( case_grille_get_type_bateau(g->tab_cases[ind]) != cleType || case_grille_get_id_bateau_type(g->tab_cases[ind]) != idBateauType )
			break;

		*xF = *xF +1;
	}

	/* si *xD<*xF, on est sur que le placement n'est pas horizontal (la reciproque n'est pas vraie, la longueur pouvant etre 1) */
	if (*xD == *xF)	{ 
		/* recherche a gauche */
		for (y =*yD -1 ; y <g->nbColonnes ; y --) {
			ind = grille_get_indice_from_coordonnees(g,*xD,y);
		 	if ( case_grille_get_type_bateau(g->tab_cases[ind]) != cleType || case_grille_get_id_bateau_type(g->tab_cases[ind]) != idBateauType )
				break;

			*yD = *yD -1;
		}

		/* recherche a droite */
		for (y =*yF +1 ; y <g->nbColonnes ; y ++) {
			ind = grille_get_indice_from_coordonnees(g,*xD,y);
		 	if ( case_grille_get_type_bateau(g->tab_cases[ind]) != cleType || case_grille_get_id_bateau_type(g->tab_cases[ind]) != idBateauType )
				break;

			*yF = *yF +1;
		}
	}

	#if (GRILLE_COMPILATION != 0)
	printf("%s sur grille %p OUT : (*xD, *yD)=(%u, %u) (*xF, *yF)=(%u, %u)\n", __func__, (void*)g, *xD, *yD, *xF, *yF);
	#endif
}

/*	Pre-conditions 	: g adresse d'une grille du joueur totalement instanciee,
						x et y a valeur dans respectivement [0,g->nbLignes -1] et [0,g->nbColonnes -1]
	Post-conditions : l'etat de la case de cordonnees (x,y) est mis a jour en fonction des informations de la grille :
						- si la case n'est pas occupee par un bateau, l'etat devient CASE_GRILLE_ETAT_JOUEE_EAU
						- si la case est occupee par un bateau qui n'est pas coule a l'issue de ce coup, l'etat devient CASE_GRILLE_ETAT_JOUEE_TOUCHE
						- si la case est occupee par un bateau qui est coule a l'issue de ce coup, l'etat devient CASE_GRILLE_ETAT_JOUEE_COULE
	Utilisation dans le jeu : fonction a appeler sur la grille du joueur exclusivement, phase de jeu
*/
void grille_joueur_jouer_coup(grille* g, unsigned int x, unsigned int y) {
	unsigned int ind = grille_get_indice_from_coordonnees(g,x,y);
	bateau_type_cle cleType = case_grille_get_type_bateau(g->tab_cases[ind]), idBateauType;
	unsigned int xD, yD, xF, yF, coule =1;

	if (cleType == BATEAU_TYPE_CLE_NONE)
		case_grille_set_etat(g->tab_cases[ind], CASE_GRILLE_ETAT_JOUEE_EAU);
	else {
		case_grille_set_etat(g->tab_cases[ind], CASE_GRILLE_ETAT_JOUEE_TOUCHE);

		/* __ test si bateau coule */
		idBateauType = case_grille_get_id_bateau_type(g->tab_cases[ind]);

		/* /!\	grille joueur 		: type renseigne ssi id renseigne
				grille advsersaire 	: l'id n'est jamais renseigne	/!\	*/
		if (idBateauType != CASE_GRILLE_BATEAU_NONE) {
			/* recuperation du placement du bateau */
			grille_get_bateau_position_from_case(g, x, y, &xD, &yD, &xF, &yF);

			/* verification cases jouees */
			for (x = xD ; (x <= xF) && coule ; x ++)
				for (y = yD ; (y <= yF) && coule ; y ++) {
					ind = grille_get_indice_from_coordonnees(g,x,y);
					coule = (case_grille_get_etat(g->tab_cases[ind]) != CASE_GRILLE_ETAT_NONE);
				}

			/* mise a jour des infos bateau */		
			if (coule)
				for (x = xD ; x <= xF ; x ++)
					for (y = yD ; y <= yF ; y ++) {
						ind = grille_get_indice_from_coordonnees(g,x,y);
						case_grille_set_etat(g->tab_cases[ind], CASE_GRILLE_ETAT_JOUEE_COULE);
					}
		}
	}
}

/*	Pre-conditions 	: g adresse d'une grille totalement instanciee, x et y a valeur dans respectivement [0,g->nbLignes -1] et [0,g->nbColonnes -1],
						cleType a valeur dans [BATEAU_TYPE_CLE_NONE,BATEAU_TYPE_CLE_NB -1] et
					  	etat_case a valeur dans [CASE_GRILLE_ETAT_NONE +1,CASE_GRILLE_ETAT_NB -1] verifiant
							cleType == BATEAU_TYPE_CLE_NONE ssi etat == CASE_GRILLE_ETAT_JOUEE_EAU
	Post-conditions : l'etat et le type de bateau de la case de cordonnees (x,y) sont mises a jour aux valeurs passees en parametre
	Utilisation dans le jeu : fonction a appeler sur la grille de l'adversaire exclusivement, phase de jeu
*/
void grille_adversaire_set_case(grille* g, bateau_type_cle cleType, case_grille_etat etat_case, unsigned int x, unsigned int y) {
	unsigned int ind = grille_get_indice_from_coordonnees(g,x,y);

	case_grille_set_etat(g->tab_cases[ind], etat_case);
	case_grille_set_type_bateau(g->tab_cases[ind], cleType);
}

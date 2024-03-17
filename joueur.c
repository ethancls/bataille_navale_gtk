#include "global.h"				/* pour la gestion des erreurs							*/

#include "joueur.h"

#include <stdio.h>				/* pour les messages d'erreur							*/
#include <stdlib.h>				/* pour allocation memoire								*/
#include <string.h>				/* pour pseudo joueur									*/
#include <time.h>				/* pour fonctions de jeu aleatoires						*/

/* ____________________ Declaration fonctions internes */

/*	Pre-conditions 	: j adresse d'un joueur dont la grille joueur est instanciee, (x,y) coordonnes valides
*/
static void joueur_case_joueur_get_string(joueur* j, unsigned int x, unsigned int y, char res[4]);

/*	Pre-conditions 	: j adresse d'un joueur dont la grille adversaire est instanciee, (x,y) coordonnes valides
*/
static void joueur_case_adversaire_get_string(joueur* j, unsigned int x, unsigned int y, char res[4]);

/* ____________________ Definition des fonctions de manipulation */

/* __________ Constructeurs / destructeurs / Initialisation / Dimensionnement */

/*	Pre-conditions 	: aucune
	Post-conditions : joueur instancie (dont sa flotte, sa grille, celle de son advsersaire) en une adresse memoire qui est renvoyee
					  Le joueur, la flotte et les grilles sont instancées selon les parametres transmis
					  Si la chaine pseudo excede JOUEUR_PSEUDO_TAILLE caracteres (significatifs), seuls les JOUEUR_PSEUDO_TAILLE premiers sont retenus 
	Gestion erreur	: Renvoie NULL si echec allocation memoire (un message d'erreur est alors affiche)
*/
joueur* joueur_construire(const char* pseudo, unsigned int tab_nb_par_type[FLOTTE_NB_TYPE_BATEAU], unsigned int nbL, unsigned int nbC) {
	joueur* j = NULL;
	int statut = STATUT_REUSSITE;
	int dimJeu;

	/* Allocation joueur */
	j = (joueur*) malloc (sizeof(joueur));

	if (j != NULL) {
		/* Initialisation des champs du joueur */
		joueur_set_pseudo(j, pseudo);

		/* Construction & initialisation flotte joueur */
		j->flotte_joueur = flotte_construire(tab_nb_par_type);

		if (j->flotte_joueur != NULL) {
			/* Construction & initialisation grille joueur */
			dimJeu = flotte_get_taille(j->flotte_joueur);

			j->grille_joueur = grille_construire(nbL, nbC, dimJeu);

			if (j->grille_joueur != NULL) {
				/* Construction & initialisation grille joueur */
				j->grille_adversaire = grille_construire(nbL, nbC, dimJeu);

				if(j->grille_adversaire == NULL)
					statut = STATUT_ERREUR_MEMOIRE;	
			}
			else	
				statut = STATUT_ERREUR_MEMOIRE;	
		}
		else	
			statut = STATUT_ERREUR_MEMOIRE;	
	}
	else	
		statut = STATUT_ERREUR_MEMOIRE;	

	/* Gestion erreur memoire */
	if (statut == STATUT_ERREUR_MEMOIRE) {
		joueur_detruire(& j);
		printf("%s::%s\n", __func__, STR_STATUT_ERREUR_MEMOIRE);
	}

	return j;
}

/*	Pre-conditions 	: les champs  (*j)->flotte,  (*j)->grille_joueur et (*j)->grille_adversaire
						qui sont (totalement ou partiellement) instancies le sont de facon coherente 
	Post-conditions : si *j est l'adresse d'un joueur instancie, la memoire reservee en l'adresse contenue dans *j est liberee et *j est reinitialise a NULL
*/
void joueur_detruire(joueur** j) {
	if (*j != NULL) {
		/* Destruction de la flotte */
		flotte_detruire(& ((*j)->flotte_joueur) );

		/* Destruction des grilles */
		grille_detruire(& ((*j)->grille_joueur) );
		grille_detruire(& ((*j)->grille_adversaire) );

		/* Destruction du joueur */
		free(*j);
		*j = NULL;
	}
}

/*	Pre-conditions 	: j adresse d'un joueur dont la flotte est instanciee
	Post-conditions : flotte du joueur redimensionnee et reinitialisee
	Gestion erreur	: Renvoie NULL si echec allocation memoire (un message d'erreur est alors affiche), auquel le joueur est detruit
*/
joueur* joueur_redimensionner_flotte(joueur* j, unsigned int tab_nb_par_type[FLOTTE_NB_TYPE_BATEAU]) {
	/* __ dans la flotte */
	j->flotte_joueur = flotte_dimensionner(j->flotte_joueur, tab_nb_par_type);

	/* Gestion erreur memoire */
	if (j->flotte_joueur == NULL) {
		joueur_detruire(& j);
		printf("%s::%s\n", __func__, STR_STATUT_ERREUR_MEMOIRE);
	}
	/* __ dans les grilles */
	else {
		grille_set_dimensionJeu(j->grille_joueur, flotte_get_taille(j->flotte_joueur));
		grille_set_dimensionJeu(j->grille_adversaire, flotte_get_taille(j->flotte_joueur));
	}

	return j;
}

/*	Pre-conditions 	: j adresse d'un joueur dont les grilles sont instanciees
	Post-conditions : grilles (joueur et adversaire) redimensionnees et reinitialisees
	Gestion erreur	: En cas d'erreur (une message d'erreur est alors affiche), les dimensions initiales sont preservees
*/
void joueur_redimensionner_grilles(joueur* j, unsigned int nbL, unsigned int nbC) {
	grille_dimensionner(j->grille_joueur, nbL, nbC);

	/* Si taille cible nbL*nbC > taille initiale de j->grille_joueur, l'allocation a pu poser probleme
		=> on ne redimensionne j->grille_adversaire que si j->grille_joueur a bien ete redimensionnee	*/
	if (grille_get_nbLignes(j->grille_joueur)*grille_get_nbColonnes(j->grille_joueur) == nbL*nbC) {
		grille_dimensionner(j->grille_adversaire, nbL, nbC);

		/* Si echec redimensionnement grille adversaire, on remet la grille joueur aux dimensions initiales */
		if (grille_get_nbLignes(j->grille_adversaire)*grille_get_nbColonnes(j->grille_adversaire) < nbL*nbC)
			grille_dimensionner(j->grille_joueur, grille_get_nbLignes(j->grille_adversaire), grille_get_nbColonnes(j->grille_adversaire));
	}
}

/*	Pre-conditions 	: j adresse d'un joueur dont la flotte et les grilles sont instanciees
	Post-conditions : flotte et grilles  (joueur et adversaire) reinitialisees (structure flotte et dimension grilles inchangees)
*/
void joueur_initialiser(joueur* j) {
	flotte_initialiser(j->flotte_joueur);
	grille_initialiser(j->grille_joueur);
	grille_initialiser(j->grille_adversaire);
}

/* __________ Accesseurs en ecriture */

/*	Pre-conditions 	: j adresse d'un joueur
	Post-conditions : j->pseudo mis a jour
*/
void joueur_set_pseudo(joueur* j, const char* pseudo) {
	strncpy(j->pseudo, pseudo, JOUEUR_PSEUDO_TAILLE);

	if (strlen(pseudo) <= JOUEUR_PSEUDO_TAILLE)
		j->pseudo[strlen(pseudo)] = '\0';
	else {
		j->pseudo[JOUEUR_PSEUDO_TAILLE] = '\0';
		printf("%s:%s :: le pseudo '%s' est ecourte en '%s'.\n", __FILE__, __func__, pseudo, j->pseudo);
	}
}

/* __________ Accesseurs en lecture */

/*	Pre-conditions 	: j adresse d'un joueur
	Post-conditions : aucune
*/
const char* joueur_get_pseudo(joueur* j) {
	return j->pseudo;
}

/*	Pre-conditions 	: j adresse d'un joueur dont la flotte est instanciee
	Post-conditions : aucune
	Valeur retour	: nombre de bateaux de la flotte
*/
unsigned int joueur_get_flotte_nb_bateaux(joueur* j) {
	return flotte_get_nb_bateaux(j->flotte_joueur);
}

/*	Pre-conditions 	: j adresse d'un joueur dont la flotte est instanciee
	Post-conditions : aucune
	Valeur retour	: somme des tailles des bateaux de la flotte
*/
unsigned int joueur_get_flotte_taille(joueur* j) {
	return flotte_get_taille(j->flotte_joueur);
}

/*	Pre-conditions 	: j adresse d'un joueur dont la flotte est instanciee
	Post-conditions : aucune
*/
unsigned int joueur_get_flotte_type_nb_bateaux(joueur* j, bateau_type_cle cleType) {
	return flotte_get_type_nb_bateaux(j->flotte_joueur, cleType);
}

/*	Pre-conditions 	: j adresse d'un joueur dont la grille joueur est instanciee
	Post-conditions : aucune
*/
unsigned int joueur_get_grille_joueur_nbLignes(joueur* j) {
	return grille_get_nbLignes(j->grille_joueur);
}

/*	Pre-conditions 	: j adresse d'un joueur dont la grille joueur est instanciee
	Post-conditions : aucune
*/
unsigned int joueur_get_grille_joueur_nbColonnes(joueur* j) {
	return grille_get_nbColonnes(j->grille_joueur);
}

/*	Pre-conditions 	: j adresse d'un joueur dont la grille joueur est instanciee
						x et y a valeur dans respectivement [0,j->grille_joueur->nbLignes -1] et [0,j->grille_joueur->nbColonnes -1]
	Post-conditions : aucune
*/
case_grille_etat joueur_get_grille_joueur_case_etat(joueur* j, unsigned int x, unsigned int y) {
	return grille_get_case_etat(j->grille_joueur, x, y);
}

/*	Pre-conditions 	: j adresse d'un joueur dont la grille joueur est instanciee
						x et y a valeur dans respectivement [0,j->grille_joueur->nbLignes -1] et [0,j->grille_joueur->nbColonnes -1]
	Post-conditions : aucune
*/
bateau_type_cle joueur_get_grille_joueur_case_type_bateau(joueur* j, unsigned int x, unsigned int y) {
	return grille_get_case_type_bateau(j->grille_joueur, x, y);
}

/*	Pre-conditions 	: j adresse d'un joueur dont la grille joueur est instanciee
						x et y a valeur dans respectivement [0,j->grille_joueur->nbLignes -1] et [0,j->grille_joueur->nbColonnes -1]
	Post-conditions : aucune
*/
int joueur_get_grille_joueur_case_id_bateau_type(joueur* j, unsigned int x, unsigned int y) {
	return grille_get_case_id_bateau_type(j->grille_joueur, x, y);
}

/*	Pre-conditions 	: j adresse d'un joueur dont la grille adversaire est instanciee
	Post-conditions : aucune
*/
unsigned int joueur_get_grille_adversaire_nbLignes(joueur* j) {
	return grille_get_nbLignes(j->grille_adversaire);
}

/*	Pre-conditions 	: j adresse d'un joueur dont la grille adversaire est instanciee
	Post-conditions : aucune
*/
unsigned int joueur_get_grille_adversaire_nbColonnes(joueur* j) {
	return grille_get_nbColonnes(j->grille_adversaire);
}

/*	Pre-conditions 	: j adresse d'un joueur dont la grille adversaire est instanciee
						x et y a valeur dans respectivement [0,j->grille_adversaire->nbLignes -1] et [0,j->grille_adversaire->nbColonnes -1]
	Post-conditions : aucune
*/
case_grille_etat joueur_get_grille_adversaire_case_etat(joueur* j, unsigned int x, unsigned int y) {
	return grille_get_case_etat(j->grille_adversaire, x, y);
}

/*	Pre-conditions 	: j adresse d'un joueur dont la grille adversaire est instanciee
						x et y a valeur dans respectivement [0,j->grille_adversaire->nbLignes -1] et [0,j->grille_adversaire->nbColonnes -1]
	Post-conditions : aucune
*/
bateau_type_cle joueur_get_grille_adversaire_case_type_bateau(joueur* j, unsigned int x, unsigned int y) {
	return grille_get_case_type_bateau(j->grille_adversaire, x, y);
}

/* __________ Fonctions de jeu */

/* _____ Etat de la flotte */

/*	Pre-conditions 	: j adresse d'un joueur dont la flotte est instanciee
	Post-conditions : aucune
*/
unsigned int joueur_get_nb_bateaux_en_mer(joueur* j) {
	return flotte_get_nb_bateaux_en_mer(j->flotte_joueur);
}

/*	Pre-conditions 	: j adresse d'un joueur dont la flotte est instanciee
						cleType a valeur dans [BATEAU_TYPE_CLE_NONE +1, BATEAU_TYPE_CLE_NB -1],
						indBateau a valeur dans [0,j->flotte_joueur->tab_nb_par_type[cleType -BATEAU_TYPE_CLE_NONE -1] -1]
	Valeur retour	: 1 si le bateau est place, 0 sinon
*/
int joueur_is_bateau_en_mer(joueur* j, bateau_type_cle cleType, unsigned int indBateau) {
	return flotte_is_bateau_en_mer(j->flotte_joueur, cleType, indBateau);
}

/*	Pre-conditions 	: j adresse d'un joueur dont la flotte est instanciee
						cleType a valeur dans [BATEAU_TYPE_CLE_NONE +1, BATEAU_TYPE_CLE_NB -1],
						indBateau a valeur dans [0,j->flotte_joueur->tab_nb_par_type[cleType -BATEAU_TYPE_CLE_NONE -1] -1],
						xD, yD, xF, yF adresses d'un entier
	Post-conditions : Si bateau non place : *x_1 == *y_1 == *x_2 == *y_2 == PLACEMENT_COORD_NONE
					  Sinon, si L designe la longueur des bateaux du type identifie par cleType :
						(*xD,*yD) == coordonnees du placement du bateau de la flotte designe par cleType et indBateau,
						(*xF,*yF) == (xD+L,yD) ou (xD,yD+L) selon l'orientation du bateau
*/
void joueur_get_bateau_position(joueur* j, bateau_type_cle cleType, unsigned int indBateau, int* xD, int* yD, int* xF, int* yF) {
	flotte_get_bateau_position(j->flotte_joueur, cleType, indBateau, xD, yD, xF, yF);
}

/*	Pre-conditions 	: j adresse d'un joueur dont la flotte et la grille joueur sont instanciees
						cleType a valeur dans [BATEAU_TYPE_CLE_NONE +1, BATEAU_TYPE_CLE_NB -1],
						indBateau a valeur dans [0,j->flotte_joueur->tab_nb_par_type[cleType -BATEAU_TYPE_CLE_NONE -1] -1]
	Valeur retour	: 1 si le bateau est coule, 0 sinon
*/
int joueur_is_bateau_coule(joueur* j, bateau_type_cle cleType, unsigned int indBateau) {
	int xD, yD, xF, yF, x, y, coule = 1;

	flotte_get_bateau_position(j->flotte_joueur, cleType, indBateau, &xD, &yD, &xF, &yF);

	for (x = xD ; (x <= xF) && coule ; x++)
		for (y = yD ; (y <= yF) && coule ; y++)
			coule = ( grille_get_case_etat (j->grille_joueur, x, y) != CASE_GRILLE_ETAT_NONE );

	return coule;
}

/*	Pre-conditions 	: j adresse d'un joueur dont la grille joueur est instanciee
	Valeur retour	: le nombre de coups portes par l'adversaires ayant fait mouche
*/
int joueur_get_nb_coups_joues_adversaire(joueur* j) {
	return grille_get_nbCoupsJoues(j->grille_joueur);
}

/*	Pre-conditions 	: j adresse d'un joueur dont la grille adversaire est instanciee
	Valeur retour	: le nombre de coups portes par le joueur ayant fait mouche
*/
int joueur_get_nb_coups_joues_joueur(joueur* j) {
	return grille_get_nbCoupsJoues(j->grille_adversaire);
}

/*	Pre-conditions 	: j adresse d'un joueur dont la grille joueur est instanciee
	Valeur retour	: le nombre de coups portes par l'adversaires ayant fait mouche
*/
int joueur_get_nb_coups_recus_touche(joueur* j) {
	return grille_get_nbTouche(j->grille_joueur);
}

/*	Pre-conditions 	: j adresse d'un joueur dont la grille adversaire est instanciee
	Valeur retour	: le nombre de coups portes par le joueur ayant fait mouche
*/
int joueur_get_nb_coups_donnes_touche(joueur* j) {
	return grille_get_nbTouche(j->grille_adversaire);
}

/*	Pre-conditions 	: j adresse d'un joueur dont la flotte et la grille joueur sont instanciees
	Valeur retour	: 1 si la flotte est coulee, 0 sinon
*/
int joueur_is_flotte_coulee(joueur* j) {
	return grille_is_coule(j->grille_joueur);
}

/*	Pre-conditions 	: j adresse d'un joueur dont la flotte et la grille joueur sont instanciees
	Valeur retour	: 1 si la flotte est coulee, 0 sinon
*/
int joueur_is_flotte_adverse_coulee(joueur* j) {
	return grille_is_coule(j->grille_adversaire);
}

/* _____ Tests validite actions de jeu */

/*	Pre-conditions 	: j adresse d'un joueur dont la grille joueur et la flotte sont instanciees,
						x a valeur dans [0,j->grille_joueur->nbLignes -1], y a valeur dans [0,j->grille_joueur->nbColonnes -1],
						cleType a valeur dans [BATEAU_TYPE_CLE_NONE +1, BATEAU_TYPE_CLE_NB -1]
	Valeur retour	: PLACEMENT_TYPE_NONE si aucune orientation n'est possible depuis la case coordnnees (x,y),
						PLACEMENT_TYPE_H si seule une orientation horizontale est possible depuis la case coordnnees (x,y),
						PLACEMENT_TYPE_V si seule une orientation verticale est possible depuis la case coordnnees (x,y),
						PLACEMENT_TYPE_NB si les 2 orientations sont possibles depuis ces coordonnees
*/
int joueur_get_placement_valide(joueur* j, bateau_type_cle cleType, unsigned int x, unsigned int y) {
	return grille_joueur_get_placement_valide(j->grille_joueur, cleType, x, y);
}

/*	Pre-conditions 	: j adresse d'un joueur dont la grille adversaire est instanciee,
						x a valeur dans [0,j->grille_adversaire->nbLignes -1], y a valeur dans [0,j->grille_adversaire->nbColonnes -1]
	Valeur retour	: 1 si coup permis, 0 sinon (coup deja joue)
*/
int joueur_is_coup_valide(joueur* j, unsigned int x, unsigned int y) {
	return ( grille_get_case_etat(j->grille_adversaire, x, y) != CASE_GRILLE_ETAT_NONE );
}

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
int joueur_placer_bateau(joueur* j, bateau_type_cle cleType, unsigned int indBateau, unsigned int x, unsigned int y, placement_type orientation) {
	int res = STATUT_ERREUR_PARAMETRE, placementOK, xD=PLACEMENT_COORD_NONE, yD=PLACEMENT_COORD_NONE, xF=PLACEMENT_COORD_NONE, yF=PLACEMENT_COORD_NONE;

	#if (JOUEUR_COMPILATION)
	printf("%s sur joueur %p IN : (x,y)=(%d,%d) bateau de longueur %u = \n", __func__, (void*)j, x, y, bateau_type_get_longueur(cleType));
	#endif

	/* __ test bateau non deja place */
	if ( flotte_is_bateau_en_mer(j->flotte_joueur, cleType, indBateau) )
		printf("%s:%s:: bateau deja place\n", __func__, STR_STATUT_ERREUR_PARAMETRE);
	else { 
		/* __ test validite coup */
		placementOK = grille_joueur_get_placement_valide(j->grille_joueur, cleType, x, y);

		if (orientation == PLACEMENT_TYPE_H && placementOK != PLACEMENT_TYPE_H && placementOK != PLACEMENT_TYPE_NB)
			printf("%s:%s:: placement impossible\n", __func__, STR_STATUT_ERREUR_PARAMETRE);
		else if (orientation == PLACEMENT_TYPE_V && placementOK != PLACEMENT_TYPE_V && placementOK != PLACEMENT_TYPE_NB)
			printf("%s:%s:: placement impossible\n", __func__, STR_STATUT_ERREUR_PARAMETRE);
		else {
			/* __ Placement du bateau */
			res = STATUT_REUSSITE;

			/* dans la flotte */
			flotte_set_bateau_position(j->flotte_joueur, cleType, indBateau, x, y, orientation);

			/* sur la grille */
			flotte_get_bateau_position(j->flotte_joueur, cleType, indBateau, &xD, &yD, &xF, &yF);
			for (x = (unsigned int)xD ; x <= (unsigned int)xF ; x ++)
				for (y = (unsigned int)yD ; y <= (unsigned int)yF ; y ++)
					grille_joueur_set_case(j->grille_joueur, cleType, indBateau, x, y);
		}
	}

	#if (JOUEUR_COMPILATION)
	printf("%s sur joueur %p OUT : (xD,yD)=(%d,%d) (xF,yF)=(%d,%d)\n", __func__, (void*)j, xD, yD, xF, yF);
	#endif

	return res;
}

/*	Pre-conditions 	: j adresse d'un joueur instancie dont la grille joueur et la flotte sont instanciees, 
						(x,y) coordonnees valides non encore jouees par l'adversaire
	Post-conditions : l'etat de la case de cordonnees (x,y) dans la grille du joueur est mis a jour en fonction des informations de la grille :
						- si la case n'est pas occupee par un bateau, l'etat devient CASE_GRILLE_ETAT_JOUEE_EAU
						- si la case est occupee par un bateau qui n'est pas coule a l'issue de ce coup, l'etat devient CASE_GRILLE_ETAT_JOUEE_TOUCHE
						- si la case est occupee par un bateau qui est coule a l'issue de ce coup, l'etat devient CASE_GRILLE_ETAT_JOUEE_COULE
*/
void joueur_recevoir_coup(joueur* j, unsigned int x, unsigned int y) {
	grille_joueur_jouer_coup(j->grille_joueur, x, y);
}

/*	Pre-conditions 	: j adresse d'un joueur instancie dont la grille adverse et la flotte sont instanciees, (x,y) coordonnees valides
						cleType a valeur dans [BATEAU_TYPE_CLE_NONE,BATEAU_TYPE_CLE_NB -1] et
					  	etat_case a valeur dans [CASE_GRILLE_ETAT_NONE +1,CASE_GRILLE_ETAT_NB -1] verifiant
							cleType == BATEAU_TYPE_CLE_NONE ssi etat == CASE_GRILLE_ETAT_JOUEE_EAU
	Post-conditions : L'etat et le type de bateau de la case (x,y) dans la grille de l'adversaire sont mises a jour aux valeurs passees en parametre
*/
void joueur_set_case_adversaire(joueur* j, bateau_type_cle cleType, case_grille_etat etat_case, unsigned int x, unsigned int y) {
	grille_adversaire_set_case(j->grille_adversaire, cleType, etat_case, x, y);
}

/*	Pre-conditions 	: j adresse d'un joueur instancie dont la grille joueur et la flotte sont instanciees, (x,y) coordonnees valides
						cleType et etat_case adresses d'un entier
							cleType == BATEAU_TYPE_CLE_NONE ssi etat == CASE_GRILLE_ETAT_JOUEE_EAU
	Post-conditions : *cleType et *etat_case prennent la valeur des champs correspondant de la case (x,y) de la grille du joueur
*/
void joueur_get_case_joueur(joueur* j, bateau_type_cle* cleType, case_grille_etat* etat_case, unsigned int x, unsigned int y) {
	*cleType = grille_get_case_type_bateau(j->grille_joueur, x, y);
	*etat_case = grille_get_case_etat(j->grille_joueur, x, y);
}

/* _____ Actions de jeu aleatoires */

/*	Pre-conditions 	: j adresse d'un joueur instancie, il a ete prealablement fait appel a la fonction srand()
	Post-conditions : (*x,*y) designe une case non joue dans la grille adverse
	Tirage aleatoire : 1 case est tiree au hasard parmi les cases non encores jouees dans la grille adversaire
*/
void joueur_set_coup_a_jouer(joueur* j, unsigned int* x, unsigned int* y) {
	unsigned int nbL = grille_get_nbLignes(j->grille_adversaire), nbC = grille_get_nbColonnes(j->grille_adversaire);
	int nbCaseNonJouees, indCase, cptCaseNonJouee, indL, indC;

	/* choix au hasard de la case a jouer */
	nbCaseNonJouees = (int)(nbL*nbC - grille_get_nbCoupsJoues(j->grille_adversaire));
	indCase = rand() % nbCaseNonJouees;

	/* localisation sur la grille de la case a jouer */
	cptCaseNonJouee = -1;
	indL = -1;
	while(indL < (int)(nbL -1) && cptCaseNonJouee < indCase) {
		indL++;
		indC = -1;
		while (indC < (int)(nbC -1) && cptCaseNonJouee < indCase) {
			indC++;
			if (grille_get_case_etat(j->grille_adversaire, (unsigned int)indL, (unsigned int)indC) == CASE_GRILLE_ETAT_NONE)
				cptCaseNonJouee ++;
		}
	}

	*x = (unsigned int) indL;
	*y = (unsigned int) indC;
}

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
int joueur_placer_flotte(joueur* j) {
	bateau_type_cle cleType;
	int placementOk, nbEssais = 0, nbCasesValides, indCase, cptCases, x, y;
	unsigned int nbBateaux = flotte_get_nb_bateaux(j->flotte_joueur), indBateau;
	unsigned int nbL = grille_get_nbLignes(j->grille_joueur), nbC = grille_get_nbColonnes(j->grille_joueur);

	while(flotte_get_nb_bateaux_en_mer(j->flotte_joueur) < nbBateaux && nbEssais < NB_ESSAIS_MAX) {
		grille_initialiser(j->grille_joueur);

		nbCasesValides = 1; /* valeur arbitraire <> 0 */
		while( flotte_get_nb_bateaux_en_mer(j->flotte_joueur) <nbBateaux && (nbCasesValides>0) )
			/* __ parcours des bateaux de la flotte */
			for (cleType = BATEAU_TYPE_CLE_NONE +1 ; cleType <= BATEAU_TYPE_CLE_NB -1 && (nbCasesValides>0) ; cleType ++)
				for (indBateau = 0 ; indBateau < flotte_get_type_nb_bateaux(j->flotte_joueur, cleType) && (nbCasesValides>0) ; indBateau ++) {
					/* dénombrement des cases à partir desquelles on peut placer un bateau du type */
					nbCasesValides = 0;
					for(x = 0 ; x < (int)nbL ; x ++)
						for(y = 0 ; y < (int)nbC ; y ++)
							if (grille_joueur_get_placement_valide(j->grille_joueur, cleType, (unsigned int)x, (unsigned int)y) != PLACEMENT_TYPE_NONE)
								nbCasesValides ++;

					if (nbCasesValides > 0) {
						/* tirage au hasard de la case à partir de laquelle placer le bateau */
						indCase = rand() % nbCasesValides;

						/* localisation de la case sur la grille */
						cptCases = -1;
						x = -1;
						while(x < (int)(nbL -1) && cptCases < indCase) {
							x++;
							y = -1;
							while (y < (int)(nbC -1) && cptCases < indCase) {
								y++;
								placementOk = grille_joueur_get_placement_valide(j->grille_joueur, cleType, (unsigned int)x, (unsigned int)y);
								if (placementOk != PLACEMENT_TYPE_NONE)
									cptCases ++;
							}
						}

						/* placement du bateau : si placementOk == PLACEMENT_TYPE_NB, on tire au sort PLACEMENT_TYPE_H ou PLACEMENT_TYPE_V */
						if (placementOk == PLACEMENT_TYPE_NB)
							placementOk = ((rand()%2 == 0) ? PLACEMENT_TYPE_H : PLACEMENT_TYPE_V);

						joueur_placer_bateau(j, cleType, indBateau, (unsigned int)x, (unsigned int)y, placementOk);
					}
				}

		nbEssais ++;
	}

	/* En cas d'echec : la grille est reinitialisee */
	if (flotte_get_nb_bateaux_en_mer(j->flotte_joueur) < nbBateaux)
		grille_initialiser(j->grille_joueur);

	#if (JOUEUR_COMPILATION)
	printf("%s(%p): %s : placement des %u bateaux a placer sur une grille %ux%u apres %d essais\n", __func__, (void*)j, (joueur_get_nb_bateaux_en_mer(j) == joueur_get_flotte_nb_bateaux(j) ? "SUCCES" : "ECHEC"), joueur_get_flotte_nb_bateaux(j), joueur_get_grille_joueur_nbLignes(j), joueur_get_grille_joueur_nbColonnes(j), nbEssais);
	#endif

	return (flotte_get_nb_bateaux_en_mer(j->flotte_joueur) < nbBateaux ? STATUT_ERREUR_NB_ESSAIS : STATUT_REUSSITE);
}

/* __________ Entrees / Sorties */

/*	____ Affichage jeu bataille	*/

/*	Pre-conditions 	: j adresse d'un joueur dont les 2 grilles sont instanciees et de memes dimensions
*/
void joueur_afficher_grilles(joueur* j) {
	unsigned int x, y, nbL, nbC;
	char str_case[4];

	nbL = joueur_get_grille_joueur_nbLignes(j);
	nbC = joueur_get_grille_joueur_nbColonnes(j);

	/* lignes entetes */
	printf("\t |");
	for (y = 0 ; y < nbC ; y ++)
		printf(" %u |",y);
	printf("\t |");
	for (y = 0 ; y < nbC ; y ++)
		printf(" %u |",y);
	printf("\n\t_|");
	for (y = 0 ; y < nbC ; y ++)
		printf("___|");
	printf("\t_|");
	for (y = 0 ; y < nbC ; y ++)
		printf("___|");
	printf("\n");

	/* cases ligne par ligne */
	for (x = 0 ; x < nbL ; x ++) {
		printf("\t%u|",x);
		for (y = 0 ; y < nbC ; y ++) {
			joueur_case_joueur_get_string(j, x, y, str_case);
			printf("%s|", str_case);
		}
		printf("\t%u|",x);
		for (y = 0 ; y < nbC ; y ++) {
			joueur_case_adversaire_get_string(j, x, y, str_case);
			printf("%s|", str_case);
		}
		printf("\n");
	}
}

/*	Pre-conditions 	: j adresse d'un joueur dont la grille joueur est instanciee, (x,y) coordonnes valides
*/
static void joueur_case_joueur_get_string(joueur* j, unsigned int x, unsigned int y, char res[4]) {
	unsigned int id;
	int etat, type;

	strcpy(res, " ? ");

	etat = joueur_get_grille_joueur_case_etat(j, x, y);
	type = joueur_get_grille_joueur_case_type_bateau(j, x, y);
	id = joueur_get_grille_joueur_case_id_bateau_type(j, x, y);

	/* pas jouee, pas de bateau */
	if (etat == CASE_GRILLE_ETAT_NONE && type == BATEAU_TYPE_CLE_NONE)
		strcpy(res, "   ");

	/* pas jouee, un bateau */
	else if (etat == CASE_GRILLE_ETAT_NONE && type > BATEAU_TYPE_CLE_NONE && type < BATEAU_TYPE_CLE_NB) {
		strcpy(res, "");
		sprintf(res, "%d-%d", type, id);
	}
	
	/* jouee, pas de bateau */
	else if (etat == CASE_GRILLE_ETAT_JOUEE_EAU && type == BATEAU_TYPE_CLE_NONE) {
		strcpy(res, "");
		strcpy(res, " . ");
	}
	
	/* jouee, un bateau touche */
	else if (etat == CASE_GRILLE_ETAT_JOUEE_TOUCHE && type > BATEAU_TYPE_CLE_NONE && type < BATEAU_TYPE_CLE_NB) {
		strcpy(res, "");
		sprintf(res, "%dx%d", type, id);
	}
	
	/* jouee, un bateau coule */
	else if (etat == CASE_GRILLE_ETAT_JOUEE_COULE && type > BATEAU_TYPE_CLE_NONE && type < BATEAU_TYPE_CLE_NB) {
		strcpy(res, "");
		sprintf(res, "%d!%d", type, id);
	}
}

/*	Pre-conditions 	: j adresse d'un joueur dont la grille adversaire est instanciee, (x,y) coordonnes valides
*/
static void joueur_case_adversaire_get_string(joueur* j, unsigned int x, unsigned int y, char res[4]) {
	case_grille_etat etat;
	bateau_type_cle type;

	strcpy(res, " ? ");

	etat =joueur_get_grille_adversaire_case_etat(j, x, y);
	type =joueur_get_grille_adversaire_case_type_bateau(j, x, y);

	/* pas jouee */
	if (etat == CASE_GRILLE_ETAT_NONE)
		strcpy(res, "   ");
	
	/* jouee, pas de bateau */
	else if (etat == CASE_GRILLE_ETAT_JOUEE_EAU && type == BATEAU_TYPE_CLE_NONE)
		strcpy(res, " . ");
	
	/* jouee, un bateau touche */
	else if (etat == CASE_GRILLE_ETAT_JOUEE_TOUCHE && type > BATEAU_TYPE_CLE_NONE && type < BATEAU_TYPE_CLE_NB) {
		strcpy(res, "");
		sprintf(res, "x%dx", type);
	}
	
	/* jouee, un bateau coule */
	else if (etat == CASE_GRILLE_ETAT_JOUEE_COULE && type > BATEAU_TYPE_CLE_NONE && type < BATEAU_TYPE_CLE_NB) {
		strcpy(res, "");
		sprintf(res, "!%d!", type);
	}
}

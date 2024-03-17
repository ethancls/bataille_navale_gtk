#include "global.h"					/* pour la gestion des erreurs			*/

#include "case_grille.h"

#include <stdio.h>					/* pour les messages d'erreur			*/
#include <stdlib.h>					/* pour allocation memoire				*/

/* ____________________ Definition des fonctions de manipulation */

/*	Pre-conditions 	: aucune
	Post-conditions : case instanciee en une adresse memoire qui est renvoyee :
						 - l'etat de la case est initialise a CASE_GRILLE_ETAT_NONE
						 - le type du bateau place sur la case est initialise a BATEAU_TYPE_CLE_NONE
						 - l'identifiant du bateau du type place sur la case est initialise a CASE_GRILLE_BATEAU_NONE
	Gestion erreur	: Renvoie NULL si parent == NULL, ou parent != NULL mais echec allocation memoire (dans les 2 cas, un message d'erreur idoine est affiche)
*/
case_grille* case_grille_construire(const struct s_grille* parent) {
	case_grille* c = NULL;

	/* Verification des parametres */
	if (parent == NULL)
		printf("%s::%s: champ parent de valeur %p tandis que ce champ est obligatoire\n", __func__, STR_STATUT_ERREUR_PARAMETRE, NULL);
	else {
		/* Allocation memoire */
		c = (case_grille*) malloc (sizeof (case_grille));

		/* Verification allocation reussie */
		if (c == NULL)
			printf("%s::%s\n", __func__, STR_STATUT_ERREUR_MEMOIRE);
		/* Initialisation */
		else {
			c->parent = parent;
			case_grille_initialiser(c);
		}
	}

	return c;	
}

/*	Pre-conditions 	: aucune
	Post-conditions : si *c est l'adresse d'une case instanciee, la memoire reservee en l'adresse indiquee dans *c est liberee et *c est affectee a NULL
*/
void case_grille_detruire(case_grille** c) {
	if (*c != NULL) {
		free(*c);
		*c = NULL;
	}
}

/*	Pre-conditions 	: c adresse d'une case
	Post-conditions : - l'etat de la case est initialise a CASE_GRILLE_ETAT_NONE
					  - le type du bateau place sur la case est initialise a BATEAU_TYPE_CLE_NONE
					  - l'identifiant du bateau du type place sur la case est initialise a CASE_GRILLE_BATEAU_NONE
*/
void case_grille_initialiser(case_grille* c) {
	c->etat = CASE_GRILLE_ETAT_NONE;
	c->type_bateau = BATEAU_TYPE_CLE_NONE;
	c->id_bateau_type = CASE_GRILLE_BATEAU_NONE;
}

/*	Pre-conditions 	: c adresse d'une case, etat_case a valeur dans [CASE_GRILLE_ETAT_NONE +1,CASE_GRILLE_ETAT_NB -1]
	Post-conditions : l'etat de la case est mis a jour a la valeur passee en parametre
*/
void case_grille_set_etat(case_grille* c, case_grille_etat etat_case) {
	c->etat = etat_case;
}

/*	Pre-conditions 	: c adresse d'une case, cleType a valeur dans [BATEAU_TYPE_CLE_NONE +1,BATEAU_TYPE_CLE_NB -1]
	Post-conditions : type de bateau mis a jour a la valeur passee en parametre
*/
void case_grille_set_type_bateau(case_grille* c, bateau_type_cle cleType) {
	c->type_bateau = cleType;
}

/*	Pre-conditions 	: c adresse d'une case, cleType a valeur dans [BATEAU_TYPE_CLE_NONE +1,BATEAU_TYPE_CLE_NB -1]
	Post-conditions : type de bateau et identifiant du bateau dans le type mis a jour aux valeurs passees en parametre
*/
void case_grille_set_bateau(case_grille* c, bateau_type_cle cleType, unsigned int indBateau) {
	c->type_bateau =cleType;
	c->id_bateau_type =(int)indBateau;
}

/*	Pre-conditions 	: c adresse d'une case
	Post-conditions : aucune
*/
case_grille_etat case_grille_get_etat(case_grille* c) {
	return c->etat;
}

/*	Pre-conditions 	: c adresse d'une case
	Post-conditions : aucune
*/
bateau_type_cle case_grille_get_type_bateau(case_grille* c) {
	return c->type_bateau;
}

/*	Pre-conditions 	: c adresse d'une case
	Post-conditions : aucune
*/
int case_grille_get_id_bateau_type(case_grille* c) {
	return c->id_bateau_type;
}

#include "global.h"					/* pour la gestion des erreurs			*/

#include "bateau_type.h"

#include <stdio.h>					/* pour affichage messages d'erreur		*/


/* ____________________ Fonctions statiques */

/*	Pre-conditions 		: aucune
	Post-conditions 	: aucune
	Retour 				: 1 si cle correcte ssi cle a valeur dans {BATEAU_TYPE_CLE_NONE +1,...,BATEAU_TYPE_CLE_NB -1},
						  0 (un message s'affiche alors) sinon
*/
static int bateau_type_is_cle(int cle);

/*	Pre-conditions 		: aucune
	Post-conditions 	: aucune
	Retour 				: 1 si cle correcte ssi cle a valeur dans {BATEAU_TYPE_CLE_NONE +1,...,BATEAU_TYPE_CLE_NB -1},
						  0 (un message s'affiche alors) sinon
*/
static int bateau_type_is_cle(int cle) {
	if (cle >= BATEAU_TYPE_CLE_NONE +1 && cle <= BATEAU_TYPE_CLE_NB -1)
		return 1;

	printf("%s::%s: cle=%d, intervalle requis [%d,%d]\n", __FILE__, STR_STATUT_ERREUR_DEPASSEMENT_INDICE, cle, BATEAU_TYPE_CLE_NONE +1, BATEAU_TYPE_CLE_NB -1);

	return 0;
}

/* ____________________ Definition des fonctions de manipulation */


/*	Pre-conditions 		: aucune
	Post-conditions 	: au premier appel de la fonction, tableau statique contenant les noms des types de bateaux instancie
	Retour				: le nom du type si cle correcte, "" sinon
*/
const char* bateau_type_get_nom(bateau_type_cle cle) {
	static char* tab_nom[BATEAU_TYPE_CLE_NB]={STR_BATEAU_TYPE_NAME_PORTE_AVIONS, STR_BATEAU_TYPE_NAME_CROISEUR, STR_BATEAU_TYPE_NAME_CONTRE_TORPILLEUR, STR_BATEAU_TYPE_NAME_SOUS_MARIN, STR_BATEAU_TYPE_NAME_TORPILLEUR};

	if (bateau_type_is_cle(cle))
		return tab_nom[cle];

	return "";
}

/*	Pre-conditions 		: aucune
	Post-conditions 	: au premier appel de la fonction, tableau statique contenant les noms courts des types de bateaux instancie
	Retour				: l'alias nom du type si cle correcte, "" sinon
*/
const char* bateau_type_get_alias(bateau_type_cle cle) {
	static char tab_alias[BATEAU_TYPE_CLE_NB][BATEAU_TYPE_TAILLE_ALIAS +1] = {STR_BATEAU_TYPE_ALIAS_PORTE_AVIONS, STR_BATEAU_TYPE_ALIAS_CROISEUR, STR_BATEAU_TYPE_ALIAS_CONTRE_TORPILLEUR, STR_BATEAU_TYPE_ALIAS_SOUS_MARIN, STR_BATEAU_TYPE_ALIAS_TORPILLEUR};

	if (bateau_type_is_cle(cle))
		return tab_alias[cle];

	return "";
}

/*	Pre-conditions 		: aucune
	Post-conditions 	: au premier appel de la fonction, tableau statique contenant les longueurs des types de bateaux instancie
	Retour				: la longueur du type si cle correcte, 0 sinon
*/
unsigned int bateau_type_get_longueur(bateau_type_cle cle) {
	static unsigned int tab_longueur[BATEAU_TYPE_CLE_NB]={INT_BATEAU_TYPE_LONGUEUR_PORTE_AVIONS, INT_BATEAU_TYPE_LONGUEUR_CROISEUR, INT_BATEAU_TYPE_LONGUEUR_CONTRE_TORPILLEUR, INT_BATEAU_TYPE_LONGUEUR_SOUS_MARIN, INT_BATEAU_TYPE_LONGUEUR_TORPILLEUR};

	if (bateau_type_is_cle(cle))
		return tab_longueur[cle];

	return 0;
}

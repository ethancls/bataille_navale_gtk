/* Un type de bateau est caractérisé par une longueur et identifié par un nom. 

	On lui attribue en outre un identifiant court de deux caractères significatifs.

	Ces informations étant constantes, on fait le choix de les encoder par 3 tableaux de taille le nombre (BATEAU_TYPE_CLE_NB) de types.
	Chaque tableau est accessible par une fonction dédiée, en utilisant alors une classe de mémorisation statique. 
*/

#ifndef BATEAU_TYPE_H
#define BATEAU_TYPE_H

/* ____________________ Structures de donnees */

/* Type enumere associant une cle (constante entiere) a chaque type de bateau	*/
enum e_bateau_type_cle {
	BATEAU_TYPE_CLE_NONE =-1,
	BATEAU_TYPE_CLE_PORTE_AVIONS,
	BATEAU_TYPE_CLE_CROISEUR,
	BATEAU_TYPE_CLE_CONTRE_TORPILLEUR,
	BATEAU_TYPE_CLE_SOUS_MARIN,
	BATEAU_TYPE_CLE_TORPILLEUR,
	BATEAU_TYPE_CLE_NB
};

/* Alias de type */
typedef enum e_bateau_type_cle bateau_type_cle;

/* Traduction en chaines de caracteres (noms longs)	*/
#define STR_BATEAU_TYPE_NAME_PORTE_AVIONS "porte-avions"
#define STR_BATEAU_TYPE_NAME_CROISEUR "croiseur"
#define STR_BATEAU_TYPE_NAME_CONTRE_TORPILLEUR "contre-torpilleur"
#define STR_BATEAU_TYPE_NAME_SOUS_MARIN "sous-marin"
#define STR_BATEAU_TYPE_NAME_TORPILLEUR "torpilleur"
#define STR_BATEAU_TYPE_NAME_INCONNU "inconnu"

/* Traduction en chaines de caracteres (noms courts)	*/
#define BATEAU_TYPE_TAILLE_ALIAS 2

#define STR_BATEAU_TYPE_ALIAS_PORTE_AVIONS "PA"
#define STR_BATEAU_TYPE_ALIAS_CROISEUR "CR"
#define STR_BATEAU_TYPE_ALIAS_CONTRE_TORPILLEUR "CT"
#define STR_BATEAU_TYPE_ALIAS_SOUS_MARIN "SM"
#define STR_BATEAU_TYPE_ALIAS_TORPILLEUR "TO"
#define STR_BATEAU_TYPE_ALIAS_INCONNU "??"

/* Longueur des types de bateau	*/
#define INT_BATEAU_TYPE_LONGUEUR_PORTE_AVIONS 5
#define INT_BATEAU_TYPE_LONGUEUR_CROISEUR 4
#define INT_BATEAU_TYPE_LONGUEUR_CONTRE_TORPILLEUR 3
#define INT_BATEAU_TYPE_LONGUEUR_SOUS_MARIN 3
#define INT_BATEAU_TYPE_LONGUEUR_TORPILLEUR 2

/* ____________________ Fonctions de manipulation */

/*	Pre-conditions 		: aucune
	Post-conditions 	: au premier appel de la fonction, tableau statique contenant les noms des types de bateaux instancie
	Retour				: le nom du type si cle correcte, "" sinon
*/
const char* bateau_type_get_nom(bateau_type_cle cle);

/*	Pre-conditions 		: aucune
	Post-conditions 	: au premier appel de la fonction, tableau statique contenant les noms courts des types de bateaux instancie
	Retour				: l'alias du type si cle correcte, "" sinon
*/
const char* bateau_type_get_alias(bateau_type_cle cle);

/*	Pre-conditions 		: aucune
	Post-conditions 	: au premier appel de la fonction, tableau statique contenant les longueurs des types de bateaux instancie
	Retour				: la longueur du type si cle correcte, 0 sinon
*/
unsigned int bateau_type_get_longueur(bateau_type_cle cle);

#endif

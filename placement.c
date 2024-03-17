#include "global.h"					/* pour la gestion des erreurs			*/

#include "placement.h"

#include <stdlib.h>					/* pour allocation memoire				*/

/* ____________________ Definition des fonctions de manipulation */

/*	Pre-conditions 	: aucune
	Post-conditions : cree un placement dont l'adresse est retournee
					  les champs sont affectes aux valeurs x=PLACEMENT_COORD_NONE, y=PLACEMENT_COORD_NONE, orientation=PLACEMENT_TYPE_NONE
	Gestion erreur	: renvoie NULL si echec allocation memoire
*/
placement* placement_construire(void) {
	placement* p = NULL;

	/* allocation memoire */
	p = (placement*) malloc (sizeof(placement));

	/* initialisation des champs */
	if (p != NULL)
		placement_initialiser(p);

	return p;
}

/*	Pre-conditions 	: aucune
	Post-conditions : si *p != NULL, la memoire reservee en l'adresse indiquee en *p est liberee et la variable *p est mise a NULL
*/
void placement_detruire(placement** p) {
	if (*p != NULL) {
		free(*p);
		*p = NULL;
	}
}

/*	Pre-conditions 	: la memoire en l'adresse p est reservee pour stocker un placement
	Post-conditions : les champs sont affectes aux valeurs x=PLACEMENT_COORD_NONE, y=PLACEMENT_COORD_NONE, orientation=PLACEMENT_TYPE_NONE
*/
void placement_initialiser(placement* p) {
	p->x = PLACEMENT_COORD_NONE;
	p->y = PLACEMENT_COORD_NONE;
	p->orientation = PLACEMENT_TYPE_NONE;
}

/*	Pre-conditions 	: la memoire en l'adresse p est reservee pour stocker un placement, 
						orientation a valeur dans [PLACEMENT_TYPE_NONE +1,PLACEMENT_TYPE_NB -1]
	Post-conditions : les champs du placement ont ete mis a jour aux valeurs passees en parametre
*/
void placement_set(placement* p, unsigned int x, unsigned int y, placement_type orientation) {
	p->x = (int)x;
	p->y = (int)y;
	p->orientation = orientation;
}

/*	Pre-conditions 	: p est l'adresse d'un placement instancie
	Post-conditions : aucune
*/
int placement_get_x(placement* p) {
	return p->x;
}

/*	Pre-conditions 	: p est l'adresse d'un placement instancie
	Post-conditions : aucune
*/
int placement_get_y(placement* p) {
	return p->y;
}

/*	Pre-conditions 	: p est l'adresse d'un placement instancie
	Post-conditions : aucune
*/
placement_type placement_get_orientation(placement* p) {
	return p->orientation;
}

/*	Pre-conditions 	: p est l'adresse d'un placement instancie, xF, yF adresses d'un entier
	Post-conditions : (*xF,*yF) vaut (p->x +longueur, p->y), (p->x, p->y +longueur) ou (PLACEMENT_COORD_NONE,PLACEMENT_COORD_NONE) selon p->orientation
*/
void placement_get_fin(placement* p, unsigned int longueur, int* xF, int* yF) {
	*xF = p->x;
	*yF = p->y;

	if (p->orientation == PLACEMENT_TYPE_H)
		*yF = *yF +(int)longueur -1;
	else if (p->orientation == PLACEMENT_TYPE_V)
		*xF = *xF +(int)longueur -1;

	/* sinon ssi placement indetermine alors *xF == *yF == p->x == p-y == PLACEMENT_COORD_NONE */
}

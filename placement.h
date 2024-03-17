/* Un placement est specifie par :
	- un point (x, y) de depart ou x indique un indice de ligne et y indique un indice de colonne
	- une orientation, horizontale ou verticale

	Mode de gestion :
		Un placement (x, y, orientation) verifie toujours
			(x, y, orientation) = (PLACEMENT_COORD_NONE, PLACEMENT_COORD_NONE, PLACEMENT_TYPE_NONE)
			ou	orientation in {PLACEMENT_TYPE_H, PLACEMENT_TYPE_V}, x >= 0 et y>=0  
*/

#ifndef PLACEMENT_H
#define PLACEMENT_H

/* ____________________ Structures de donnees */

/* Type enumere indiquant l'orientation du placement d'un bateau	*/
enum e_placement_type {
	PLACEMENT_TYPE_NONE = -1,
	PLACEMENT_TYPE_H,
	PLACEMENT_TYPE_V,
	PLACEMENT_TYPE_NB
};

/* Valeur par defaut des coordonnees */
#define PLACEMENT_COORD_NONE -1

/* Type structure decrivant le placement d'un bateau	*/
struct s_placement {
	int x;								/* indice ligne 1e case occupee						*/
	int y;								/* indice colonne 1e case occupee					*/
	enum e_placement_type orientation;	/* orientation (horizontale ou verticale) du bateau	*/
};

/* Alias de type */

typedef enum e_placement_type placement_type;
typedef struct s_placement placement;

/* ____________________ Fonctions de manipulation */

/*	Pre-conditions 	: aucune
	Post-conditions : cree un placement dont l'adresse est retournee
					  les champs sont affectes aux valeurs x=PLACEMENT_COORD_NONE, y=PLACEMENT_COORD_NONE, orientation=PLACEMENT_TYPE_NONE
	Gestion erreur	: renvoie NULL si echec allocation memoire
*/
placement* placement_construire(void);

/*	Pre-conditions 	: aucune
	Post-conditions : si *p != NULL, la memoire reservee en l'adresse indiquee en *p est liberee et la variable *p est mise a NULL
*/
void placement_detruire(placement** p);

/*	Pre-conditions 	: la memoire en l'adresse p est reservee pour stocker un placement
	Post-conditions : les champs sont affectes aux valeurs x=PLACEMENT_COORD_NONE, y=PLACEMENT_COORD_NONE, orientation=PLACEMENT_TYPE_NONE
*/
void placement_initialiser(placement* p);

/*	Pre-conditions 	: la memoire en l'adresse p est reservee pour stocker un placement, 
						orientation a valeur dans [PLACEMENT_TYPE_NONE +1,PLACEMENT_TYPE_NB -1]
	Post-conditions : les champs du placement ont ete mis a jour aux valeurs passees en parametre
*/
void placement_set(placement* p, unsigned int x, unsigned int y, placement_type orientation);

/*	Pre-conditions 	: p est l'adresse d'un placement instancie
	Post-conditions : aucune
*/
int placement_get_x(placement* p);

/*	Pre-conditions 	: p est l'adresse d'un placement instancie
	Post-conditions : aucune
*/
int placement_get_y(placement* p);

/*	Pre-conditions 	: p est l'adresse d'un placement instancie
	Post-conditions : aucune
*/
placement_type placement_get_orientation(placement* p);

/*	Pre-conditions 	: p est l'adresse d'un placement instancie, xF, yF adresses d'un entier
	Post-conditions : (*xF,*yF) vaut (p->x +longueur, p->y), (p->x, p->y +longueur) ou (PLACEMENT_COORD_NONE,PLACEMENT_COORD_NONE) selon p->orientation
*/
void placement_get_fin(placement* p, unsigned int longueur, int* xF, int* yF);

#endif

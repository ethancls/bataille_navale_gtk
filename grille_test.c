/* grille : programme test

	Fonctions testees :

		main :
			grille* grille_construire(unsigned int nbL, unsigned int nbC, unsigned int nbCasesBateau);
			void grille_detruire(grille** g);
			void grille_initialiser(grille* g);
			void grille_dimensionner(grille* g, unsigned int nbL, unsigned int nbC);
			void grille_set_dimensionJeu(grille* g, unsigned int nbCasesBateau);

		grille_test_afficher :
			unsigned int grille_get_nbLignes(grille* g);
			unsigned int grille_get_nbColonnes(grille* g);
			unsigned int grille_get_dimensionJeu(grille* g);
			int grille_get_case_etat(grille* g, unsigned int x, unsigned int y);
			int grille_get_case_type_bateau(grille* g, unsigned int x, unsigned int y);
			int grille_get_case_id_bateau_type(grille* g, unsigned int x, unsigned int y);
			unsigned int grille_get_nbCoupsJoues(grille* g);
			unsigned int grille_get_nbTouche(grille* g);
			unsigned int grille_is_coule(grille* g);

		grille_test_placement_valide :
			int grille_joueur_get_placement_valide(grille* g, int cleType, unsigned int x, unsigned int y);

		grille_joueur_test_set :
			void grille_joueur_set_case(grille* g, int cleType, unsigned int indBateau, unsigned int x, unsigned int y);

		grille_test_jouer_coup :
			void grille_joueur_jouer_coup(grille* g, unsigned int x, unsigned int y);

		grille_adversaire_test_set :
			void grille_adversaire_set_case(grille* g, int cleType, case_grille_etat etat_case, unsigned int x, unsigned int y);

	TODO (tests manquant) :

		Scenarios déterministes pour :
			void grille_joueur_jouer_coup(grille* g, unsigned int x, unsigned int y);
*/

#include "grille.h"

#include "placement.h"			/* pour type enumere placement_type				 	*/

#include <stdlib.h>				/* pour EXIT_SUCCESS									*/
#include <stdio.h>				/* pour affichages									*/
#include <time.h>				/* pour test coups joues (choix des coups aleatoire)	*/

/* ____________________ liste des fonctions */

/* Fonction d'affichage grille faisant appel aux accesseurs */
void grille_test_afficher(grille *g);

/* grille joueur : place des bateaux :
	- de type 1 sur (x,y)(x,y+1) pour x=0[4] et y=0[4] (sur (x,y) si y == g->nbColonnes -1)
	- de type 2 sur (x,y)(x+1,y) pour x=1[4] et y=0[3] (sur (x,y) si x == g->nbLignes -1)
*/
void grille_joueur_test_set(grille *g, int cleType1, int cleType2);

/* grille adversaire place coups :
	- dans l'eau sur les cases d'indice de valeur 2[8]
	- touche bateau type 1 sur les cases d'indice de valeur 4[8]
	- coule bateau de type 2 sur les cases d'indice de valeur 0[8]
*/
void grille_adversaire_test_set(grille *g, int cleType1, int cleType2);

/* grille_test_fill mets les cases :
	- d'indice de valeur 0[2] dans l'etat CASE_GRILLE_ETAT_JOUEE_EAU
	- d'indice de valeur 0[3] de type de bateau CASE_GRILLE_ETAT_JOUEE_EAU
	- d'indice de valeur 0[5] d'id de bateau 3
*/
void grille_test_fill(grille *g);

/* Sous-routine placement valide */
void grille_test_placement_valide(grille *g);

/* Sous-routine test jouer coup */
void grille_test_jouer_coup(grille* g);

/* Fonction principale */
int main(void);

/* ____________________ definition des fonctions */

/* ____ Affichages :: */

/* Fonction d'affichage grille faisant appel aux accesseurs */
void grille_test_afficher(grille *g) {
	unsigned int x,y;

	printf("Affichage grille %p :\t", (void*)g);

	if (g == NULL)
		printf("grille non instanciee.\n");
	else {
		/* __ champs atomiques grille */
		printf("%u lignes x %u colonnes, %u cases touchees apres %u coups joues sur %u cases a toucher : %s.\n", grille_get_nbLignes(g), grille_get_nbColonnes(g), grille_get_nbTouche(g), grille_get_nbCoupsJoues(g), grille_get_dimensionJeu(g), (grille_is_coule(g) ? "flotte coulee": "flotte non encore coulee"));
		printf("\tDetail des cases (etat|type|idBateau) ou etat dans [%d,%d] si case jouee et %d sinon, type dans [%d,%d] si case occupee et %d sinon :\n", CASE_GRILLE_ETAT_NONE+1, CASE_GRILLE_ETAT_NB -1, CASE_GRILLE_ETAT_NONE, BATEAU_TYPE_CLE_NONE +1, BATEAU_TYPE_CLE_NB -1, BATEAU_TYPE_CLE_NONE);

		/* __ cases grille */

		/* lignes entetes */
		printf("\t   ");
		for (y = 0 ; y < g->nbColonnes ; y ++)
			printf("     %u     ",y);
		printf("\n\t___");
		for (y = 0 ; y < g->nbColonnes ; y ++)
			printf("___________");
		printf("\n");

		/* cases ligne par ligne */
		for (x = 0 ; x < g->nbLignes ; x ++) {
			printf("\t%u| ",x);
			for (y = 0 ; y < g->nbColonnes ; y ++){
				if (grille_get_case_etat(g,x,y) < 0)
					printf("(%d|", grille_get_case_etat(g,x,y));
				else
					printf("(+%d|", grille_get_case_etat(g,x,y));

				if (grille_get_case_type_bateau(g,x,y) < 0)
					printf("%d|", grille_get_case_type_bateau(g,x,y));
				else
					printf("+%d|", grille_get_case_type_bateau(g,x,y));

				if (grille_get_case_id_bateau_type(g,x,y) < 0)
					printf("%d) ", grille_get_case_id_bateau_type(g,x,y));
				else
					printf("+%d) ", grille_get_case_id_bateau_type(g,x,y));
			}
			printf("\n");
		}
	}
}

/* ____ Fonction principale :: */

/* __ Sous-routines affectation :: */

/* grille joueur : place des bateaux :
	- de type 1 sur (x,y)(x,y+1) pour x=0[4] et y=0[4] (sur (x,y) si y == g->nbColonnes -1)
	- de type 2 sur (x,y)(x+1,y) pour x=1[4] et y=0[3] (sur (x,y) si x == g->nbLignes -1)
*/
void grille_joueur_test_set(grille *g, int cleType1, int cleType2) {
	unsigned int indBateau1 =0, indBateau2 =0;
	unsigned int x, y;

	grille_initialiser(g);
	g->nbCasesBateau = 0;

	/* placements horizontaux */
	for (x =0; x <g->nbLignes; x ++)
		for (y =0; y <g->nbColonnes; y ++)
			if ( x %4 == 0 && (y %4 == 0 || (y -1) %4 == 0) ) {
				grille_joueur_set_case(g, cleType1, indBateau1, (unsigned int)x, (unsigned int)y);
				g->nbCasesBateau ++;

				if ((y -1) %4 == 0 || y == g->nbColonnes -1)
					indBateau1 ++;
			}

	/* placements verticaux */
	for (y =0; y <g->nbColonnes; y ++)
		for (x =0; x <g->nbLignes; x ++)
			if ( ( (x -1) %4 == 0 || (x -2) %4 == 0) && y %3 == 0 ) {
				grille_joueur_set_case(g, cleType2, indBateau2, (unsigned int)x, (unsigned int)y);
				g->nbCasesBateau ++;

				if ((x -2) %4 == 0 || x == g->nbLignes -1)
					indBateau2 ++;
			}
}

/* grille adversaire place coups :
	- dans l'eau sur les cases d'indice de valeur 2[8]
	- touche bateau type 1 sur les cases d'indice de valeur 4[8]
	- coule bateau de type 2 sur les cases d'indice de valeur 0[8]
*/
void grille_adversaire_test_set(grille *g, int cleType1, int cleType2) {
	int ind;
	unsigned int x, y;

	grille_initialiser(g);

	for (x=0; x < g->nbLignes; x ++)
		for (y=0; y < g->nbColonnes; y ++) {
			ind = (x * g->nbColonnes) +y;

			if (ind % 2 == 0 && ind % 4 != 0 && ind % 8 != 0)
				grille_adversaire_set_case(g,BATEAU_TYPE_CLE_NONE,CASE_GRILLE_ETAT_JOUEE_EAU,x,y);

			if (ind % 4 == 0 && ind % 8 != 0)
				grille_adversaire_set_case(g,cleType1,CASE_GRILLE_ETAT_JOUEE_TOUCHE,x,y);

			if (ind % 8 == 0)
				grille_adversaire_set_case(g,cleType2,CASE_GRILLE_ETAT_JOUEE_COULE,x,y);
		}
}

/* grille_test_fill met les cases :
	- d'indice de valeur 0[2] dans l'etat CASE_GRILLE_ETAT_JOUEE_EAU
	- d'indice de valeur 0[3] de type de bateau CASE_GRILLE_ETAT_JOUEE_EAU
	- d'indice de valeur 0[5] d'id de bateau 3
*/
void grille_test_fill(grille *g) {
	int ind;
	unsigned int x, y;

	grille_initialiser(g);

	for (x=0; x < g->nbLignes; x ++)
		for (y=0; y < g->nbColonnes; y ++) {
			ind = (x * g->nbColonnes) +y;

			if (ind % 2 == 0)
				case_grille_set_etat(g->tab_cases[ind], CASE_GRILLE_ETAT_JOUEE_EAU);

			if (ind % 3 == 0 && ind % 5 != 0)
				case_grille_set_type_bateau(g->tab_cases[ind], BATEAU_TYPE_CLE_PORTE_AVIONS);

			if (ind % 5 == 0)
				case_grille_set_bateau(g->tab_cases[ind], BATEAU_TYPE_CLE_TORPILLEUR, 3);
		}
}

/* Sous-routine placement valide */
void grille_test_placement_valide(grille *g) {
	unsigned int x,y;
	int res, cleType;

	printf("%s(%p) ::\n", __func__, (void*)g);

	if (g == NULL) {
		printf("\tgrille non instanciee.\n");
		return;
	}

	/* ____ on place des bateaux dans la grille pour verifier que la fonction fonctionne */
	grille_joueur_test_set(g, BATEAU_TYPE_CLE_PORTE_AVIONS, BATEAU_TYPE_CLE_TORPILLEUR);

	/* ____ on essaie les pacements de tout type de bateau */
	for (cleType = BATEAU_TYPE_CLE_NONE +1 ; cleType <= BATEAU_TYPE_CLE_NB -1 ; cleType ++) {
		/* __ affichage occupation grille */
		printf("Cases occupees ssi a valeur dans [%d,%d])\t", BATEAU_TYPE_CLE_NONE +1, BATEAU_TYPE_CLE_NB -1);

		/* __ affichage placements autorises */
		printf("Placements possibles pour bateau %d--%s de longueur %d -- %d:AUCUN, %d:HORIZONTAL, %d:VERTICAL, %d:HORIZONTAL & VERTICAL\n", cleType, bateau_type_get_nom(cleType), bateau_type_get_longueur(cleType), PLACEMENT_TYPE_NONE, PLACEMENT_TYPE_H, PLACEMENT_TYPE_V, PLACEMENT_TYPE_NB);

		/* lignes entetes */
		printf("\t   ");
		for (y = 0 ; y < g->nbColonnes ; y ++)
			printf(" %u ",y);
		printf("\t   ");
		for (y = 0 ; y < g->nbColonnes ; y ++)
			printf(" %u ",y);
		printf("\n\t__");
		for (y = 0 ; y < g->nbColonnes ; y ++)
			printf("___");
		printf("\t__");
		for (y = 0 ; y < g->nbColonnes ; y ++)
			printf("___");
		printf("\n");

		/* cases ligne par ligne */
		for (x = 0 ; x < g->nbLignes ; x ++) {
			printf("\t%u| ",x);
			for (y = 0 ; y < g->nbColonnes ; y ++) {
				res = grille_get_case_type_bateau(g, x, y);

				if (res < 0)
					printf("%d ", res);
				else
					printf("+%d ", res);
			}
			printf("\t%u| ",x);
			for (y = 0 ; y < g->nbColonnes ; y ++) {
				res = grille_joueur_get_placement_valide(g, cleType, x, y);

				if (res < 0)
					printf("%d ", res);
				else
					printf("+%d ", res);
			}
			printf("\n");
		}

		getchar();
	}
}

/* Sous-routine test jouer coup */
void grille_test_jouer_coup(grille* g) {
	int indCaseNonJouee, cptCaseNonJouee;
	unsigned int x, y;

	printf("%s(%p) IN\n", __func__, (void*)g);

	srand( time (NULL) );

	/* on met quelque chose dans la grille */
	grille_joueur_test_set(g, BATEAU_TYPE_CLE_PORTE_AVIONS, BATEAU_TYPE_CLE_TORPILLEUR);

	/* on joue tant que pas coule */
	while(! grille_is_coule(g)) {
		grille_test_afficher(g);

		indCaseNonJouee = rand() % ( g->nbLignes*g->nbColonnes -grille_get_nbCoupsJoues(g) );
		cptCaseNonJouee = -1;
		for (x = 0 ; x < g->nbLignes ; x++) {
			for (y = 0 ; y < g->nbColonnes ; y++) {
				if (grille_get_case_etat(g, x, y) == CASE_GRILLE_ETAT_NONE)
					cptCaseNonJouee ++;

				if (cptCaseNonJouee == indCaseNonJouee)
					break;
			}

			if (cptCaseNonJouee == indCaseNonJouee)
				break;
		}

		printf("\n%s(%p): indCaseNonJouee == %d -- Appel grille_joueur_jouer_coup(%p,%u,%u)\n", __func__, (void*)g, indCaseNonJouee, (void*)g, x, y);
		grille_joueur_jouer_coup(g,x,y);

		getchar();
	}

	grille_test_afficher(g);
	printf("%s(%p) OUT -- nombre de cases jouees == %u\n", __func__, (void*)g, grille_get_nbCoupsJoues((void*)g));
}

/* Fonction principale */
int main(void) {
	grille* g = NULL;
	unsigned int nbL=5, nbC=4, nbCasesJeu = 10;
	int cleType1 = BATEAU_TYPE_CLE_CONTRE_TORPILLEUR, cleType2 = BATEAU_TYPE_CLE_TORPILLEUR;

	/* ____ construction */
	printf("________ TEST 1 : construction :\n");

	g = grille_construire(nbL,nbC,nbCasesJeu);
	printf("\nAppel %p=grille_construire(%u,%u,%u) :\n", (void*)g, nbL, nbC, nbCasesJeu);
	grille_test_afficher(g);
	getchar();

	/* ____ affectation */

	/* affectation #1 */
	printf("________ TEST 1 : accesseur set grille joueur :\n");

	grille_joueur_test_set(g, cleType1, cleType2);
	printf("\nSur grille %p reinitialisee, via grille_joueur_set_case, placement de bateaux :\n", (void*)g);
	printf("\t- de type %d sur (x,y)(x,y+1) pour x=0[4] et y=0[4] (sur (x,y) si y == %u)\n", cleType1, g->nbColonnes -1);
	printf("\t- de type %d sur (x,y)(x+1,y) pour x=1[4] et y=0[3] (sur (x,y) si x == %u)\n", cleType2, g->nbLignes -1);
	grille_test_afficher(g);

	getchar();

	/* affectation #2 */
	printf("________ TEST 2 : accesseur set grille advsersaire :\n");

	grille_adversaire_test_set(g, cleType1, cleType2);
	printf("\nSur grille %p reinitialisee, appel de grille_adversaire_set_case:\n", (void*)g);
	printf("\t- pour etat %d et type bateau %d sur les cases d'indice de valeur 2[8]\n", CASE_GRILLE_ETAT_JOUEE_EAU, BATEAU_TYPE_CLE_NONE);
	printf("\t- pour etat %d et type bateau %d sur les cases d'indice de valeur 4[8]\n", CASE_GRILLE_ETAT_JOUEE_TOUCHE, cleType1);
	printf("\t- pour etat %d et type bateau %d sur les cases d'indice de valeur 0[8]\n", CASE_GRILLE_ETAT_JOUEE_COULE, cleType2);
	grille_test_afficher(g);

	getchar();

	/* affectation #3 */
	printf("________ TEST 3 : initialisation :\n");

	/* on met des choses dans la grille pour verifier que la reinitialisation se fait bien */
	grille_test_fill(g);
	printf("\nGrille initiale %ux%u, %u cases a toucher :\n", g->nbLignes, g->nbColonnes, g->nbCasesBateau);

	grille_initialiser(g);
	printf("\nAppel de grille_initialiser(%p) :\n", (void*)g);
	grille_test_afficher(g);

	getchar();

	/* ____ redimensionnement */

	/* redimensionnement #0 */
	printf("________ TEST 4 : redimensionnement jeu (nombre de cases a toucher, dimensions de la grille inchangees) :\n");

	/* on met des choses dans la grille pour verifier que la reinitialisation se fait bien */
	grille_test_fill(g);

	printf("\nGrille initiale %ux%u, %u cases a toucher :\n", g->nbLignes, g->nbColonnes, g->nbCasesBateau);
	nbCasesJeu ++;
	grille_set_dimensionJeu(g, nbCasesJeu);
	printf("\nAppel de grille_set_dimensionJeu(%p, %u) :\n", (void*)g, nbCasesJeu);
	grille_test_afficher(g);

	getchar();

	/* redimensionnement #1 */
	printf("________ TEST 5 : redimensionnement (nouvelle grille plus petite) :\n");

	printf("\nGrille initiale %ux%u, %u cases a toucher :\n", g->nbLignes, g->nbColonnes, g->nbCasesBateau);
	grille_test_afficher(g);

	nbL --;
	nbC --;
	grille_dimensionner(g, nbL, nbC);
	printf("\nAppel de grille_dimensionner(%p, %u, %u) :\n", (void*)g, nbL, nbC);
	grille_test_afficher(g);

	getchar();

	/* redimensionnement #2 */
	printf("________ TEST 6 : redimensionnement (nouvelle grille plus grande) :\n");

	/* on met des choses dans la grille pour verifier que la reinitialisation se fait bien */
	grille_test_fill(g);

	printf("\nGrille initiale %ux%u, %u cases a toucher :\n", g->nbLignes, g->nbColonnes, g->nbCasesBateau);
	grille_test_afficher(g);

	nbL ++;
	nbC = nbC +2;
	grille_dimensionner(g, nbL, nbC);
	printf("\nAppel de grille_dimensionner(%p, %u, %u) :\n", (void*)g, nbL, nbC);
	grille_test_afficher(g);

	getchar();

	/* placement */
	printf("________ TEST 7 : placement valide :\n");
	grille_test_placement_valide(g);

	/* jouer coup */
	printf("________ TEST 8 : jouer coup :\n");

	grille_test_jouer_coup(g);

	/* ____ destruction */
	printf("________ TEST 9 : destruction :\n");

	/* destruction #1 */
	grille_detruire(&g);
	printf("\nAppel grille_detruire(%p) : g == %p\n", (void*) &g, (void*)g);
	grille_test_afficher(g);

	/* destruction #2 */
	grille_detruire(&g);
	printf("\nAppel grille_detruire(%p) : g == %p\n", (void*) &g, (void*)g);
	grille_test_afficher(g);

	return EXIT_SUCCESS;
}

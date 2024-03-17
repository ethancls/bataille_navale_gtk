#include "struct.h"

void initialiser_vue(data_t *donnees);
void initialiser_grilles(data_t *donnees, GtkWidget *grid, GtkWidget *lab);

void affichage(data_t *donnees);
void afficher_bateau(data_t *data, char *b1, char *b2);

void on_play_button_clicked(GtkWidget *widget, gpointer data);
void on_regles_button_clicked(gpointer data);
void on_button_clicked(GtkWidget *widget, gpointer data);

void rejouer(data_t *data);
void reinitialiser_jeu(gpointer data);
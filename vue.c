#include "vue.h"
#include "ctrl.h"

void on_regles_button_clicked(gpointer data)
{
    GtkWidget *dialog;
    GtkWidget *content;
    GtkWidget *label;

    dialog = gtk_dialog_new_with_buttons("Règles", GTK_WINDOW(((data_t *)data)->v.window), GTK_DIALOG_DESTROY_WITH_PARENT, ("RETOUR"), GTK_RESPONSE_NONE, NULL);
    content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    label = gtk_label_new("La bataille navale oppose deux joueurs.\nChaque joueur dispose de deux grilles carrées de côté 10, dont les colonnes sont numérotées de 1 à 10 et les lignes de A à J, ainsi que d'une flotte composée de cinq bateaux de deux à cinq cases de long. \nL'une des grilles représente la zone contenant sa propre flotte (Joueur). \nAu début du jeu, chaque joueur place ses bateaux sur sa grille (aléatoire). \nL'autre grille représente la zone adverse (Ennemi), où il cherchera à couler les bateaux de son adversaire. \nChaque joueur, à son tour, joue une case, et voit sur la grille si le tir tombe à l'eau ou au contraire s'il touche un bateau.\nLe jeu continue jusqu'à ce qu'une des flottes soit coulée.");
    gtk_container_add(GTK_CONTAINER(content), label);
    g_signal_connect_swapped(dialog, "response", G_CALLBACK(gtk_widget_destroy), dialog);
    gtk_widget_show_all(dialog);
}

void reinitialiser_jeu(gpointer data)
{
    data_t *donnees = (data_t *)data;

    // Réinitialiser les grilles et les flottes des joueurs
    free(donnees->j);
    donnees->j = jeu_construire();
    free(donnees->adv);
    donnees->adv = jeu_construire();

    // Réinitialiser l'affichage des grilles
    for (int row = 1; row < 11; row++)
    {
        for (int col = 1; col < 11; col++)
        {
            GtkWidget *buttonJoueur = gtk_grid_get_child_at(GTK_GRID(donnees->v.gridJoueur), col, row);
            GtkWidget *buttonEnnemi = gtk_grid_get_child_at(GTK_GRID(donnees->v.gridEnnemi), col, row);

            gtk_button_set_label(GTK_BUTTON(buttonJoueur), " ");
            gtk_button_set_label(GTK_BUTTON(buttonEnnemi), " ");

            gtk_widget_set_sensitive(buttonJoueur, FALSE);
            gtk_widget_set_sensitive(buttonEnnemi, FALSE);
        }
    }

    // Réactiver le bouton "Jouer"
    gtk_widget_set_sensitive(GTK_WIDGET(donnees->v.jouer), TRUE);
    gtk_widget_set_sensitive(GTK_WIDGET(donnees->v.normal), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(donnees->v.difficile), TRUE);
    donnees->difficulte = 0;
}

void afficher_bateau(data_t *data, char *b1, char *b2)
{
    for (unsigned int type = 0; type < FLOTTE_NB_TYPE_BATEAU; type++)
    {
        for (unsigned int index = 0; index < ((data_t *)data)->j->j->flotte_joueur->tab_nb_par_type[type]; index++)
        {
            int x = ((data_t *)data)->j->j->flotte_joueur->tab_placements[type][index]->x + 1;
            int y = ((data_t *)data)->j->j->flotte_joueur->tab_placements[type][index]->y + 1;
            int orientation = ((data_t *)data)->j->j->flotte_joueur->tab_placements[type][index]->orientation;
            for (unsigned int length = 0; length < bateau_type_get_longueur(type); length++)
            {
                if (orientation == PLACEMENT_TYPE_H)
                {
                    GtkWidget *button = gtk_grid_get_child_at(GTK_GRID(((data_t *)data)->v.gridJoueur), x, (y + length));
                    gtk_button_set_label(GTK_BUTTON(button), b1);
                }
                else
                {
                    GtkWidget *button = gtk_grid_get_child_at(GTK_GRID(((data_t *)data)->v.gridJoueur), (x + length), y);
                    gtk_button_set_label(GTK_BUTTON(button), b2);
                }
            }
        }
    }
}

void on_play_button_clicked(GtkWidget *widget, gpointer data)

{
    gtk_widget_set_sensitive(widget, FALSE);
    jeu_placer_flotte(((data_t *)data)->j);
    jeu_placer_flotte(((data_t *)data)->adv);
    initialiser_tableau_proba(((data_t *)data));
    afficher_bateau((data_t *)data, "o", "o");

    for (int row = 1; row < 11; row++)
    {
        for (int col = 1; col < 11; col++)
        {
            GtkWidget *button = gtk_grid_get_child_at(GTK_GRID(((data_t *)data)->v.gridEnnemi), col, row);
            gtk_widget_set_sensitive(button, TRUE);
        }
    }
}

void on_normal_button_clicked(GtkWidget *widget, gpointer data)
{
    gtk_widget_set_sensitive(widget, FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(((data_t *)data)->v.difficile), TRUE);
    ((data_t *)data)->difficulte = 0;
}

void on_difficile_button_clicked(GtkWidget *widget, gpointer data)
{
    gtk_widget_set_sensitive(widget, FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(((data_t *)data)->v.normal), TRUE);
    ((data_t *)data)->difficulte = 1;
}

void initialiser_vue(data_t *donnees)
{
    // DONNEES
    donnees->v.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    donnees->v.container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    donnees->v.gridsContainer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    donnees->v.gridJoueur = gtk_grid_new();
    donnees->v.gridEnnemi = gtk_grid_new();
    donnees->v.labelJoueur = gtk_label_new("Joueur");
    donnees->v.labelEnnemi = gtk_label_new("Ennemi");
    donnees->v.toolbar = gtk_toolbar_new();
    donnees->v.jouer = gtk_tool_button_new(NULL, "Jouer");
    donnees->v.regles = gtk_tool_button_new(NULL, "Règles");
    donnees->v.normal = gtk_tool_button_new(NULL, "Normal");
    donnees->v.difficile = gtk_tool_button_new(NULL, "Difficile");

    // VUE

    gtk_window_set_title(GTK_WINDOW(donnees->v.window), "Bataille Navale");
    gtk_container_set_border_width(GTK_CONTAINER(donnees->v.window), 10);

    // TOOLBAR

    g_signal_connect(donnees->v.jouer, "clicked", G_CALLBACK(on_play_button_clicked), donnees);
    g_signal_connect_swapped(donnees->v.regles, "clicked", G_CALLBACK(on_regles_button_clicked), donnees);
    g_signal_connect(donnees->v.difficile, "clicked", G_CALLBACK(on_difficile_button_clicked), donnees);
    g_signal_connect(donnees->v.normal, "clicked", G_CALLBACK(on_normal_button_clicked), donnees);
    gtk_toolbar_insert(GTK_TOOLBAR(donnees->v.toolbar), donnees->v.jouer, 0);
    gtk_toolbar_insert(GTK_TOOLBAR(donnees->v.toolbar), donnees->v.regles, 1);
    gtk_toolbar_insert(GTK_TOOLBAR(donnees->v.toolbar), donnees->v.normal, 2);
    gtk_widget_set_sensitive(GTK_WIDGET(donnees->v.normal), FALSE);
    gtk_toolbar_insert(GTK_TOOLBAR(donnees->v.toolbar), donnees->v.difficile, 3);
}

void rejouer(data_t *data)
{
    // Vérifier si le joueur a perdu
    if (((data_t *)data)->j->j->grille_joueur->nbCasesBateau == 0)
    {
        // Afficher une boîte de dialogue pour demander au joueur de rejouer ou quitter
        GtkWidget *dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO,
                                                   "Vous avez perdu ! Souhaitez-vous rejouer ?");

        // Obtenir la réponse du joueur
        gint response = gtk_dialog_run(GTK_DIALOG(dialog));

        // Fermer la boîte de dialogue
        gtk_widget_destroy(dialog);

        // Vérifier la réponse du joueur
        if (response == GTK_RESPONSE_YES)
        {
            // Le joueur souhaite rejouer, réinitialiser le jeu
            reinitialiser_jeu(data);
        }
        else
        {
            // Le joueur souhaite quitter
            gtk_main_quit();
        }

        return;
    }

    // Vérifier si le joueur a gagné
    if (((data_t *)data)->adv->j->grille_joueur->nbCasesBateau == 0)
    {
        // Afficher une boîte de dialogue pour demander au joueur de rejouer ou quitter
        GtkWidget *dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO,
                                                   "Vous avez gagné ! Souhaitez-vous rejouer ?");

        // Obtenir la réponse du joueur
        gint response = gtk_dialog_run(GTK_DIALOG(dialog));

        // Fermer la boîte de dialogue
        gtk_widget_destroy(dialog);

        // Vérifier la réponse du joueur
        if (response == GTK_RESPONSE_YES)
        {
            // Le joueur souhaite rejouer, réinitialiser le jeu
            reinitialiser_jeu(data);
        }
        else
        {
            // Le joueur souhaite quitter
            gtk_main_quit();
        }

        return;
    }
}

void initialiser_grilles(data_t *donnees, GtkWidget *grid, GtkWidget *lab)
{
    gchar alphabet[] = "ABCDEFGHIJ";

    // TABLEAU JOUEUR

    gtk_grid_set_row_homogeneous(GTK_GRID(grid), TRUE);
    gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);
    gtk_grid_attach(GTK_GRID(grid), lab, 0, 0, 1, 1);

    // Ajout des numéros de colonne
    for (int col = 1; col < 11; col++)
    {
        gchar *text = g_strdup_printf("%d", col);
        GtkWidget *label = gtk_label_new(text);
        gtk_grid_attach(GTK_GRID(grid), label, col, 0, 1, 1);
        g_free(text);
    }

    // Ajout des numéros de ligne et des boutons
    for (int row = 1; row < 11; row++)
    {
        gchar *text = g_strdup_printf("%c", alphabet[row - 1]);
        GtkWidget *label = gtk_label_new(text);
        gtk_grid_attach(GTK_GRID(grid), label, 0, row, 1, 1);
        g_free(text);

        for (int col = 1; col < 11; col++)
        {
            GtkWidget *button = gtk_button_new_with_label(" ");
            gtk_button_set_relief(GTK_BUTTON(button), GTK_RELIEF_NONE);
            gtk_widget_set_sensitive(button, FALSE);
            gtk_grid_attach(GTK_GRID(grid), button, col, row, 1, 1);
            g_object_set_data(G_OBJECT(button), "row", GINT_TO_POINTER(row));
            g_object_set_data(G_OBJECT(button), "col", GINT_TO_POINTER(col));
            g_signal_connect(button, "clicked", G_CALLBACK(on_button_clicked), donnees);
        }
    }
}

void on_button_clicked(GtkWidget *widget, gpointer data)
{
    int row = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget), "row"));
    int col = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget), "col"));
    gtk_widget_set_sensitive(widget, FALSE);

    int bateau_ennemi_present = 0;
    for (unsigned int type = 0; type < FLOTTE_NB_TYPE_BATEAU; type++)
    {
        for (unsigned int index = 0; index < ((data_t *)data)->adv->j->flotte_joueur->tab_nb_par_type[type]; index++)
        {
            int x = ((data_t *)data)->adv->j->flotte_joueur->tab_placements[type][index]->x + 1;
            int y = ((data_t *)data)->adv->j->flotte_joueur->tab_placements[type][index]->y + 1;
            int orientation = ((data_t *)data)->adv->j->flotte_joueur->tab_placements[type][index]->orientation;
            unsigned int length = bateau_type_get_longueur(type);

            if (orientation == PLACEMENT_TYPE_H)
            {
                if (y <= row && row < y + (int)length && x == col)
                {
                    bateau_ennemi_present = 1;
                    break;
                }
            }
            else
            {
                if (x <= col && col < x + (int)length && y == row)
                {
                    bateau_ennemi_present = 1;
                    break;
                }
            }
        }
        if (bateau_ennemi_present == 1)
        {
            break;
        }
    }
    if (bateau_ennemi_present == 1)
    {
        ((data_t *)data)->adv->j->grille_joueur->nbCasesBateau--;
        gtk_button_set_label(GTK_BUTTON(widget), "X");
    }
    else
    {
        gtk_button_set_label(GTK_BUTTON(widget), "-");
    }

    // COUP ORDINATEUR

    int bateau_present = 0;
    int c_row = 0;
    int c_col = 0;
    int max_proba = 0;
    for (int i = 0; i < 10; i++)
    {
        for (int j = 0; j < 10; j++)
        {
            if ((((data_t *)data)->proba[i][j] > max_proba) && (((data_t *)data)->proba[i][j] > 0))
            {
                max_proba = ((data_t *)data)->proba[i][j];
                c_col = i + 1;
                c_row = j + 1;
            }
        }
    }

    for (unsigned int type = 0; type < FLOTTE_NB_TYPE_BATEAU; type++)
    {
        for (unsigned int index = 0; index < ((data_t *)data)->j->j->flotte_joueur->tab_nb_par_type[type]; index++)
        {
            int x = ((data_t *)data)->j->j->flotte_joueur->tab_placements[type][index]->x + 1;
            int y = ((data_t *)data)->j->j->flotte_joueur->tab_placements[type][index]->y + 1;
            int orientation = ((data_t *)data)->j->j->flotte_joueur->tab_placements[type][index]->orientation;
            unsigned int length = bateau_type_get_longueur(type);

            if (orientation == PLACEMENT_TYPE_H)
            {
                if (y <= c_row && c_row < y + (int)length && x == c_col)
                {
                    bateau_present = 1;
                    break;
                }
            }
            else
            {
                if (x <= c_col && c_col < x + (int)length && y == c_row)
                {
                    bateau_present = 1;
                    break;
                }
            }
        }
        if (bateau_present == 1)
        {
            break;
        }
    }

    GtkWidget *button = gtk_grid_get_child_at(GTK_GRID(((data_t *)data)->v.gridJoueur), c_col, c_row);

    mise_a_jour_proba_apres_coup(((data_t *)data), c_col - 1, c_row - 1, bateau_present);

    if (bateau_present == 1)
    {
        ((data_t *)data)->j->j->grille_joueur->nbCasesBateau--;
        gtk_button_set_label(GTK_BUTTON(button), "X");
    }
    else
    {
        gtk_button_set_label(GTK_BUTTON(button), "-");
    }

    rejouer(((data_t *)data));
}

void affichage(data_t *donnees)
{
    // CONTAINERS
    gtk_box_pack_start(GTK_BOX(donnees->v.container), donnees->v.toolbar, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(donnees->v.gridsContainer), donnees->v.gridJoueur, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(donnees->v.gridsContainer), donnees->v.gridEnnemi, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(donnees->v.container), donnees->v.gridsContainer, TRUE, TRUE, 5);
    gtk_container_add(GTK_CONTAINER(donnees->v.window), donnees->v.container);

    // AFFICHAGE
    gtk_widget_show_all(donnees->v.window);

    // QUITTER FENETRE
    g_signal_connect(G_OBJECT(donnees->v.window), "delete-event", G_CALLBACK(gtk_main_quit), NULL);
}

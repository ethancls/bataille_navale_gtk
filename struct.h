#pragma once
#include "global.h"
#include "jeu.h"
#include "joueur.h"
#include <gtk/gtk.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <stdbool.h>
#define SIZE 10

struct vue_s
{
    GtkWidget *window;
    GtkWidget *vboxJoueur;
    GtkWidget *vboxEnnemi;
    GtkWidget *container;
    GtkWidget *gridsContainer;
    GtkWidget *gridJoueur;
    GtkWidget *gridEnnemi;
    GtkWidget *labelJoueur;
    GtkWidget *labelEnnemi;
    GtkWidget *IDEnnemi;
    GtkWidget *toolbar;
    GtkWidget *etatPartie;
    GtkToolItem *jouer;
    GtkToolItem *regles;
    GtkToolItem *normal;
    GtkToolItem *difficile;
};

typedef struct vue_s vue;

struct data_s
{
    vue v;
    jeu *j;
    jeu *adv;
    int proba[SIZE][SIZE];
    int difficulte;
};

typedef struct data_s data_t;
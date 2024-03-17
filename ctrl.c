#include "ctrl.h"

int max(int a, int b)
{
    return (a > b) ? a : b;
}

int min(int a, int b)
{
    return (a < b) ? a : b;
}

void initialiser_tableau_proba(data_t *donnees)
{
    // Initialiser toutes les cases avec la même probabilité
    for (int i = 0; i < 10; i++)
    {
        for (int j = 0; j < 10; j++)
        {
            donnees->proba[i][j] = 1;
        }
    }
}

void mise_a_jour_proba_apres_coup(data_t *donnees, int x, int y, int touche)
{
    // Si le coup a touché un navire, augmenter la probabilité des cases alignées
    if (touche)
    {
        donnees->proba[x][y] = 0;
        if (donnees->difficulte >= 1)
        {
            // Augmenter la probabilité des cases à gauche et à droite
            for (int i = max(0, x - 1); i <= min(9, x + 1); i++)
            {
                if (donnees->proba[i][y] != 0)
                    donnees->proba[i][y] += 5;
            }
            // Augmenter la probabilité des cases en haut et en bas
            for (int j = max(0, y - 1); j <= min(9, y + 1); j++)
            {
                if (donnees->proba[x][j] != 0)
                    donnees->proba[x][j] += 5;
            }
        }
        else
        {
            for (int j = 0; j < 10; j++)
            {
                if (donnees->proba[x][j] != 0 && donnees->proba[j][y] != 0)
                {
                    donnees->proba[x][j] += 3;
                    donnees->proba[j][y] += 3;
                }
            }
        }
    // Si le coup n'a pas touché de navire, diminuer la probabilité de cette case
    }
    else
    {
        donnees->proba[x][y] = 0;
        if (donnees->difficulte >= 1)
        {
            int xrand, yrand;
            do
            {
                xrand = x + rand() % 5 + 1;
                yrand = y + rand() % 5 + 1;
            }while ( donnees->proba[xrand][yrand] == 0 );
            donnees->proba[xrand][yrand] += 1;
        }
        else
        {
            for (int i = 0; i < 10; i++)
            {
                for (int j = 0; j < 10; j++)
                {
                    if (donnees->proba[i][j] != 0)
                    {
                        int res = rand() % 4;
                        donnees->proba[i][j] += res;
                    }
                }
            }
        }
    }
}

void initialiser_data(data_t *donnees)
{
    donnees->j = jeu_construire();
    donnees->adv = jeu_construire();
    donnees->difficulte = 0;
}
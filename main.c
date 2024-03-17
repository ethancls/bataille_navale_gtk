#include "vue.h"
#include "ctrl.h"

int main(int argc, char *argv[])
{
    srand(time(NULL));
    
    // STRUCTURE DE DONNEES
    data_t donnees;

    // INITIALISATION STRUCTURE
    gtk_init(&argc, &argv);
    initialiser_data(&donnees);
    initialiser_vue(&donnees);
    initialiser_grilles(&donnees, donnees.v.gridJoueur, donnees.v.labelJoueur);
    initialiser_grilles(&donnees, donnees.v.gridEnnemi, donnees.v.labelEnnemi);
    // AFFICHER VUE
    affichage(&donnees);

    // MAIN GTK
    gtk_main();

    return 0;
}
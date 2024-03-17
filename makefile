# ____ general
CFLAGS = -Wall -pedantic -Wextra `pkg-config --cflags gtk+-3.0`	# options de compilation
LDFLAGS = `pkg-config --libs gtk+-3.0`			# options de liaison

# regles generiques
.SUFFIXES: .c
.c.o:
	gcc $(CFLAGS) -c $<

# ____ liste des executables
all: bataille.exe

bataille.exe: bateau_type.o placement.o flotte.o case_grille.o grille.o joueur.o client_serveur.o jeu.o vue.o ctrl.o main.o 
	gcc $^ -o $@ $(LDFLAGS)

# ____ dependances pour la mise a jour des objets
bateau_type.o: bateau_type.c bateau_type.h global.h
placement.o: placement.c placement.h global.h
flotte.o: flotte.c flotte.h bateau_type.h placement.h global.h
case_grille.o: case_grille.c case_grille.h bateau_type.h global.h
grille.o: grille.c grille.h case_grille.h placement.h bateau_type.h global.h
joueur.o: joueur.c joueur.h bateau_type.h placement.h flotte.h case_grille.h grille.h global.h
client_serveur.o: client_serveur.c client_serveur.h global.h
jeu.o: jeu.c jeu.h client_serveur.h joueur.h bateau_type.h placement.h flotte.h case_grille.h grille.h global.h
ctrl.o: ctrl.c ctrl.h struct.h
vue.o: vue.c vue.h
main.o: main.c ctrl.h vue.h

# ____ cibles speciales : nettoyage
clean:
	rm *.o *.exe
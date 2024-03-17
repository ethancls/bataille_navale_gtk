#ifndef BT_GLOBAL_H
#define BT_GLOBAL_H

/* ________________ Mode de compilation */
#define FLOTTE_COMPILATION 0		/* 0 => RELEASE,		1 => DEBUG (traces)		*/
#define GRILLE_COMPILATION 0		/* 0 => RELEASE,		1 => DEBUG (traces)		*/
#define JOUEUR_COMPILATION 0		/* 0 => RELEASE,		1 => DEBUG (traces)		*/
#define CLIENT_SERVEUR_COMPILE 0	/* 0 => RELEASE,		1 => DEBUG (traces) 	*/
#define JEU_COMPILATION 0			/* 0 => RELEASE,		1 => DEBUG (traces) 	*/

/* ________________ Statuts & messages de réussite et d'erreur */

/* Constantes symboliques indiquant le statut de reussite / d'echec des fonctions	*/
enum e_statut {
	STATUT_REUSSITE,
	STATUT_ERREUR_PARAMETRE,
	STATUT_ERREUR_DEPASSEMENT_INDICE,
	STATUT_ERREUR_MEMOIRE,
	STATUT_ERREUR_NB_ESSAIS,
	STATUT_ERREUR_COMMUNICATION,
	STATUT_ERREUR_SYNCHRONISATION,
	STATUT_ERREUR_SERVEUR
};

/* Traductions chaines de caracteres */
#define STR_STATUT_REUSSITE "Operation reussie"
#define STR_STATUT_ERREUR_PARAMETRE "Echec: parametre(s) incorrect(s)"
#define STR_STATUT_ERREUR_DEPASSEMENT_INDICE "Erreur de depassement d'indice"
#define STR_STATUT_ERREUR_MEMOIRE "Echec allocation memoire"
#define STR_STATUT_ERREUR_NB_ESSAIS "Echec: nombre maximum de tentatives infructueuses atteint"
#define STR_STATUT_ERREUR_COMMUNICATION "Probleme rencontre dans la communication joueur distant"
#define STR_STATUT_ERREUR_SYNCHRONISATION "Probleme rencontre en cours de jeu : joueurs desynchronises"
#define STR_STATUT_ERREUR_SERVEUR "le lancement du serveur a echoue"

/* ________________ Tailles pour l'échange de messages entre le client et le serveur */

#define JOUEUR_PSEUDO_TAILLE 200	/* taille maximale de description du nom d'un joueur
		/!\ doit être >= STR_JOUEUR_PSEUDO_DEFAULT /!\ 
*/

#define UINT_MAX_LEN 10	/* taille (en nombre de chiffres) de représentation en base décimale d'un entier non signé 
	Cette taille intervention dans l'élaboration des messages entre le client et le serveur
	Ici on suppose UINT_MAX =4294967295 =2^(8 x 4) -1 où 8 == #bits dans 1 octet et 4 == #octets pour décrire 1 unsigned int == sizeof(unsigned int)
		/!\ à mettre à jour selon l'architecture /!\ 
*/

#define ACTION_TAILLE_MAX 50		/* taille maximale (en nombre de caractères) de représentation des actions de jeu 
		/!\ doit être >= strlen(action) pour action in {STR_MSG_PLACER, STR_MSG_RESULTAT} /!\ 
*/

#define MESSAGE_TAILLE_MAX 200		/* taille maximale des messages échangés
		/!\ doit permettre de formuler des messages jeu de la forme 
			nomjoueur où strlen(nomjoueur) <= JOUEUR_PSEUDO_TAILLE
			STR_MSG_PLACER x y où x et y sont des valeurs uint
			STR_MSG_RESULTAT clé etat où clé et état sont des valeurs int
		/!\ 
*/

#endif

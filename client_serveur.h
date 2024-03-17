/* client_serveur.h

	Communication par sockets connectés TCP/IPv4	

	Principes :
	- Une socket est un flux de données permettant la communication entre 2 processus, identifié par 1 numéro (identifiant entier)
	- La communication implique une partie cliente et une partie serveur
	- Le client nécessite 1 seule socket. Il faut :
		1) créer une socket de type SOCK_STREAM (socket connectée) et de domaine/famille d'adresse AF_INET (TCP/IPv4) par socket()
		2) connecter cette socket à la socket distante en transmettant l'adresse de celle-ci par connect(), cette adresse étant spécifiée par: 
			- le domaine/famille d'adresse AF_INET,
			- l'adresse de la machine hôte (que l'on connaît par son nom ou par son ip),
			- le port PORT (qui est 1024 en l'état du code mais pourrait être tout port à partir de 1024)
	- Le serveur nécessite 2 sockets. Il faut :
		1) créer une socket de type SOCK_STREAM (socket connectée) et de domaine/famille d'adresse AF_INET (TCP/IPv4) par socket()
		2) attacher cette socket à une adresse par bind() en spécifiant:
			- le domaine/famille d'adresse AF_INET,
			- le port PORT (qui est 1024 en l'état du code mais pourrait être tout port a partir de 1024),
			- les adresses des machines pouvant s'y connecter -> INADDR_ANY pour accepter toute machine 
		3) se mettre en écoute d'un nombre maximal de connexions par listen() -> dans notre cas, 1 connexion au maximum
		4) se mettre en attente d'une demande de connexion par accept()
			-> une connexion génère 1 nouvelle socket, précisément celle avec laquelle la socket distante communique 
	- 2 sockets connectées, qu'elles soient du côté client ou du côté serveur, peuvent ensuite s'échanger des messages par recv() et send()
	- Pour supprimer 1 socket client, il suffit de faire appel a close() sur son identifiant 
	- Pour supprimer 1 socket serveur, il faut d'abord supprimer l'éventuelle socket créée par accept(), puis supprimer la socket serveur.
		Dans les 2 cas, il suffit de faire appel a close() sur l'identifiant de la socket à supprimer
	- Le serveur peut accepter une nouvelle connexion dès lors qu'aucune connexion n'est en cours

	Structures de données:
	- Un client est caractérisé par un identifiant et un buffer de communication
	- Un serveur est caractérisé par un identifiant, le nom et l'ip de sa machine hôte, et un client (~socket locale qui communique avec le client distant)

	Mode de gestion :
	- Un champ 'id' d'une structure client ou serveur est le descripteur d'1 socket ssi id <> SYSTEME_KO
		/!\ Attention à la gestion de ce champ. Il est notamment recommandé de faire suivre une instruction de déclaration d'une variable
			client ou serveur d'un appel à la fonction d'initialisation respectivement client_initialiser() et serveur_initialiser()
		/!\
	- La taille des buffers est définie par
				'taille maximale d'un message' x 'nombre maximal de messages pouvant être conjointement contenus dans le buffer'
			(X send() successifs par la socket distante => le buffer de la socket locale peut contenir la concatenation de ces X messages)
		/!\ Attention : on considère ici au +2 messages conjointement dans le buffer, de par l'utilisation qui est faite des structures 
			client et serveur dans le jeu (au pire: succession jeu_tour_joueur() et jeu_tour_adversaire())
			=> à réviser selon utilisation faite du modèle
		/!\

	Compatibilité : Linux/Unix/MacOS POSIX

	TODO :
	- compatibilité systèmes:
		- pour les fonctions systèmes, préférer getaddrinfo/getnameinfo a gethostbyaddr/gethostbyname
		- prise en compte d'autres systèmes (linux non POSIX, Windows, ...)
	- utilités structures:
		- gestion plus fine gestion buffer (notamment avec 1 separateur de messages, verifier buffer vide avant 1 send(), capture evenements, ...)
		- gestion plus fine des erreurs système
		- gestion plus fine des échanges par utilisation des options des sockets (notamment: timeout)
		- gestion du port (choix dynamique du premier port disponible à partir du port 1024, auquel cas le port aussi doit être communiqué au client)
*/

#ifndef CLIENT_SERVEUR_H
#define CLIENT_SERVEUR_H

#include <netinet/in.h>	/*	struct sockaddr_in		*/
#include <limits.h>		/*	constante HOST_NAME_MAX	*/

/* ______________________________ Structures de donnees	*/

/* Parametres */

#define SYSTEME_KO -1											/* -1 : valeur (le plus souvent) retournée par les fonctions système en cas d'erreur	*/
#define SYSTEME_OK 0											/* 0  : valeur (le plus souvent) retournée par les fonctions système en cas de succès	*/
#define INET_ATON_KO 0											/* 0  : valeur retournee par la fonction inet_aton en cas d'echec						*/

#define PORT 1024												/* port de connexion pour les socket (client comme serveur)
																	TODO : plutôt fonction cherchant 1 port disponible (à partir du port 1024)			*/

#define NB_CLIENTS_MAX 1										/* tout au plus 1 client a la fois					 									*/

/* Client */
#include "global.h"				/* pour NB_MESSAGE_MAX					*/
#define NB_MESSAGE_MAX 2										/* nombre maximal de messages concaténes dans le buffer									*/
#define BUFFER_TAILLE ( (NB_MESSAGE_MAX * MESSAGE_TAILLE_MAX) +1)	/* taille maximale des messages échangés sur la socket									*/

struct s_client {
	int id;									/* 	nom (id) socket											*/
	char buffer[BUFFER_TAILLE];				/*	buffer de communication									*/
};

/* Serveur */
#define IP_TAILLE_MAX 15										/* taille IPv4 : xxx.xxx.xxx.xxx														*/
#define NOM_HOTE_TAILLE_MAX _POSIX_HOST_NAME_MAX				/* taille maximale nom hote																*/
struct s_serveur {
	int id;									/* 	nom (id) socket serveur									*/
	struct s_client client;					/*	socket communiquant avec la socket cliente distante		*/
	char nom_hote[NOM_HOTE_TAILLE_MAX +1];	/*	nom hôte serveur										*/
	char ip[IP_TAILLE_MAX +1];				/*	ip hôte serveur											*/
};

/* Alias de type */
typedef struct s_client client;
typedef struct s_serveur serveur;

/* ______________________________ Fonctions client	*/

/* Initialise une structure client
	Pré-conditions : c adresse d'un client tel que c->id n'est pas le descripteur d'1 socket
	Post-conditions : champ c->id == -1, buffer vide
*/
void client_initialiser(client* c);

/* Connexion d'un client a un serveur par transmission du nom de la machine hote du serveur
	Pre-conditions : c adresse d'un client, serveur en attente d'une connexion client
	Post-conditions :
	- l'eventuelle socket c->id initiale est supprimee
	- c->id devient le descripteur d'une nouvelle socket connectée à une socket de domaine AF_INET d'adresse 'nom_hote:PORT'
	La connexion ne peut reussir que si un serveur est en attente de connexion client a l'adresse 'nom_hote:PORT' pour le domaine AF_INET
	Gestion erreurs : 
	- si échec suppression socket : message idoine
	- si échec nouvelle socket, lecture informations serveur, connexion socket distante : message idoine & c->id == -1
*/
void client_connecter_par_nom(client* c, const char* nom_hote);

/* Connexion d'un client a un serveur par transmission de l'IP de la machine hote du serveur
	Pre-conditions : c adresse d'un client, serveur en attente d'une connexion client
	Post-conditions :
	- l'eventuelle socket c->id initiale est supprimee
	- c->id devient le descripteur d'une nouvelle socket connectee a une socket de domaine AF_INET d'adresse 'ip:PORT'
	La connexion ne peut reussir que si un serveur est en attente de connexion client a l'adresse 'ip:PORT' pour le domaine AF_INET
	Gestion erreurs : 
	- si échec suppression socket : message idoine
	- si échec nouvelle socket, lecture informations serveur, connexion socket distante : message idoine & c->id == -1
*/
void client_connecter_par_ip(client* c, char ip[IP_TAILLE_MAX +1]);

/* Supprime la socket du client dont l'adresse est passee en parametre
	Pre-conditions : c adresse d'un client tel que c->id == -1 ou c->id descripteur d'une socket
	Post-conditions : eventuelle socket c->id supprimee, c->id == -1, buffer vide
	Gestion des erreurs : en cas d'echec (la variable errno est alors mise a jour), un message d'erreur est affiche
*/
void client_delete(client* c);

/* Renvoie l'adresse du buffer du client
	Pre-conditions : c adresse d'un client
*/
char* client_get_buffer(client* c);

/* Ecrit la chaine placee en parametre dans le buffer du client
	Pre-conditions : c adresse d'un client
	Post-conditions : c->buffer contient la chaine 'texte'
	Gestion erreur : si chaine trop longue pour le buffer, chaine seulement partiellement copiee (un message est alors affiche)
*/
void client_set_buffer(client* c, const char* texte);

/* Renvoie 1 si le client est connecte, 0 sinon
	Pre-conditions : s adresse d'un serveur
*/
int client_est_connecte(client* c);

/* Reception d'un message sur la socket passee en parametre
	Pre-conditions : c adresse d'un client connecte
	Post-conditions : message ecrit dans c->buffer
	Valeur retour : nombre d'octets recus en cas de succes, -1 en cas d'echec (la variable errno est alors mise a jour)
	Gestion des erreurs : en cas d'echec, un message d'erreur est affiche
	/!\ NB1 : l'eventuel contenu initial du buffer est perdu
		NB2 : le buffer peut contenir la concatenation de plusieurs messages resultant de send() successifs par la socket distante
		NB3 : recv() peut rester en attente. Cela peut etre gere par l'utilisation de l'option MSG_DONTWAIT et la gestion de l'erreur EAGAIN ou EWOULDBLOCK
	/!\
*/
int client_recevoir(client* c);

/* Emission d'un message sur la socket passee en parametre
	Pre-conditions : c adresse d'un client connecte
	Post-conditions : en cas de succes, message envoye et buffer vide
	Valeur retour : nombre d'octets envoyes en cas de succes, -1 en cas d'echec (la variable errno est alors mise a jour)
	Gestion des erreurs : en cas d'echec, un message d'erreur est affiche et le contenu du buffer n'est pas modifie
*/
int client_emettre(client* c);

/* ______________________________ Fonctions serveur	*/

/* Initialise le serveur dont l'adresse est passe en parametre
	Pre-conditions : s adresse d'un serveur dont le champ s->id n'est pas le descripteur d'une socket
	Post-conditions : s->id == -1, structure s->client initialisee, champs s->nom_hote et s->ip renseignes
	Gestion des erreurs : en cas d'echec de lectrure des informations de la machine hote, un message d'erreur est affiche
*/
void serveur_initialiser(serveur* s);

/* Cree une socket dont le descripteur est place dans le champ id du serveur dont l'adresse est passee en parametre
	Pre-conditions : s adresse d'un serveur initialise dont le champ s->id n'est pas le descripteur d'une socket
	Post-conditions : s->id socket liee a une adresse & mise a l'ecoute d'au plus 1 connexion client sur cette adresse
	Valeur retour : -1 en cas d'echec connexion
	Gestion des erreurs : en cas d'echec (construction socket, options socket, attachement a une adresse, mise en ecoute connexion client),
							un message d'erreur idoine est affiche et l'eventuelle socket construite est supprimee (s->id == -1)
*/
void serveur_creer(serveur* s);

/* Accepte une connexion d'un client
	Pre-conditions : s adresse d'un serveur initialise, lie a une adresse par la fonction bind(), a l'ecoute d'au plus un client a cette adresse
	Post-conditions : si client initial, il est reinitialise ; dans tous les cas : nouvelle connexion client
	La connexion ne peut reussir que si un client demande la connexion a l'adresse 's->ip:PORT' pour le domaine AF_INET	
	Gestion erreur : en cas d'echec, un message est affiche
*/
void serveur_connecter_client(serveur* s);

/* Deconnecte le client eventuellement connecte
	Pre-conditions : s adresse d'un serveur
	Post-conditions : si client initial, il est reinitialise
*/
void serveur_deconnecter_client(serveur* s);

/* Supprime la socket serveur apres avoir supprime l'eventuelle socket cliente
	Pre-conditions : s adresse d'un serveur
	Post-conditions : s et s->client reinitialises
*/
void serveur_delete(serveur* s);

/* Renvoie le nom de l'hote du serveur
	Pre-conditions : s adresse d'un serveur
*/
const char* serveur_get_nom_hote(serveur* s);

/* Renvoie l'adresse ip (notation avec '.') de l'hote du serveur
	Pre-conditions : s adresse d'un serveur
*/
const char* serveur_get_ip(serveur* s);

/* Renvoie l'adresse du buffer de la connexion au client
	Pre-conditions : s adresse d'un serveur
*/
char* serveur_get_buffer(serveur* s);

/* Ecrit la chaine placee en parametre dans le buffer du clent connecte a s
	Pre-conditions : s adresse d'un serveur
	Post-conditions : s->client->buffer contient la chaine 'texte'
*/
void serveur_set_buffer(serveur* s, const char* texte);

/* Renvoie 1 si le serveur est connecte, 0 sinon
	Pre-conditions : s adresse d'un serveur
*/
int serveur_est_connecte(serveur* s);

/* Renvoie 1 si un client est connecte, 0 sinon
	Pre-conditions : s adresse d'un serveur
*/
int serveur_est_client_connecte(serveur* s);

/* Reception d'un message sur la socket
	Pre-conditions : s adresse d'un serveur auquel un client est connecte
	Post-conditions : message ecrit dans s->client.buffer
	Valeur retour : nombre d'octets recus en cas de succes, -1 en cas d'echec (la variable errno est alors mise a jour)
	Gestion des erreurs : en cas d'echec, un message d'erreur est affiche
	/!\ NB1 : l'eventuel contenu initial du buffer est perdu
		NB2 : le buffer peut contenir la concatenation de plusieurs messages resultant de send() successifs par la socket distante
		NB3 : recv() peut rester en attente. Cela peut etre gere par l'utilisation de l'option MSG_DONTWAIT et la gestion de l'erreur EAGAIN ou EWOULDBLOCK
	/!\
*/
int serveur_recevoir(serveur* s);

/* Emission d'un message sur la socket passee en parametre
	Pre-conditions : s adresse d'un serveur auquel un client est connecte
	Post-conditions : en cas de succes, message envoye et buffer vide
	Valeur retour : nombre d'octets envoyes en cas de succes, -1 en cas d'echec (la variable errno est alors mise a jour)
	Gestion des erreurs : en cas d'echec, un message d'erreur est affiche et le contenu du buffer n'est pas modifie
*/
int serveur_emettre(serveur* s);

#endif

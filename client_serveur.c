#include <stdio.h>					/* traces d'exécution (erreurs)											*/
#include <errno.h>					/* pour erreurs système													*/
#include <string.h>					/* pour la manipulation des chaînes de caractères (notamment buffer)	*/

#include "global.h"					/* messages d'erreur													*/

#include "client_serveur.h"

#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

/* variable à laquelle lier la valeur (que l'on souhaite vraie) des options SO_REUSEADDR et SO_REUSEPORT	*/
int optServeurYes = 1;

/* ______________________________ Declaration sous-routines	*/

/* Création d'1 socket
	Valeur retour: id de la socket créée en cas de succès, -1 (et variable errno mise à jour) sinon
	Gestion erreurs: en cas d'échec, un message d'erreur est affiché
*/
static int socket_new(void);

/* Destruction d'1 socket
	Valeur retour: 0 en cas de succès, -1 (et variable errno mise à jour) sinon (un message est alors affiché)
*/
static void socket_delete(int id);

/* Initialise c->id à 1 socket nouvellement créé
	Pré-conditions: c adresse d'un client
	Post-conditions: éventuelle socket c->id initiale, c->id descripteur d'1 nouvlle socket
	Gestion des erreurs: 
	- si échec suppression socket: message idoine
	- si échec nouvelle socket: message idoine & c->id == -1
*/
static void client_new_socket(client* c);

/*	Sous-routine de client_connecter_par_nom() et client_connecter_par_ip(): connexion d'un client à un serveur par tansmission adresse hôte serveur
	Pré-conditions:	 c adresse d'un client
	Post-conditions:
	- l'éventuelle socket c->id initiale est supprimée
	- c->id devient le descripteur d'1 nouvelle socket connectée à 1 socket de domaine AF_INET d'adresse 'adr_serveur->sin_addr.s_addr:PORT'
	La connexion ne peut réussir que si un serveur est en attente de connexion client à l'adresse 'adr_serveur->sin_addr.s_addr:PORT' pour le domaine AF_INET
	Gestion des erreurs: 
	- si échec suppression socket: message idoine
	- si échec nouvelle socket, connexion socket distante: message idoine & c->id == -1
	Valeur retour: -1 en cas d'échec, 0 sinon
*/
static void client_connecter(client* c, struct sockaddr_in* adr_serveur);

/* Accesseur client pour l'accès du serveur à son client local 
	Renvoie l'id du client
	Pré-conditions: c adresse d'un client
*/
static int client_get_id(client* c);

/* Accesseur client pour l'accès du serveur à son client local 
	Attribue au client l'id passé en paramètre
	Pré-conditions: c adresse d'un client, id descripteur d'1 socket, c->id == -1
*/
static void client_set_id(client* c, int id);

/* ______________________________ Definition des fonctions	*/
	
/* __________ Sockets */

/* Création d'1 socket
	Valeur retour: id de la socket créée en cas de succès, -1 (et variable errno mise à jour) sinon (un message est alors affiché)
*/
static int socket_new(void) {
	int id_socket;

	/* Création d'1 socket connectée
		domaine d'adresse AF_INET (IPv4)
		type SOCK_STREAM (TCP)
		protocole 0 (le système choisit le protocole)
	*/
	id_socket =socket(AF_INET, SOCK_STREAM, 0);
	if (id_socket == SYSTEME_KO) {
		perror("socket()");
	}

	return id_socket;
}

/* Destruction d'1 socket
	Valeur retour: 0 en cas de succès, -1 (et variable errno mise à jour) sinon (un message est alors affiché)
*/
static void socket_delete(int id) {
	int res =SYSTEME_OK;

	if (id != SYSTEME_KO) {
		res =close(id);
		if (res == SYSTEME_KO) {
			perror("close()");
		}
	}
}

/* __________ Client */

/* Initialise une structure client
	Pré-conditions: c adresse d'un client tel que c->id n'est pas le descripteur d'1 socket
	Post-conditions: champ c->id == -1, buffer vide
*/
void client_initialiser(client* c) {
	c->id =SYSTEME_KO;
	c->buffer[0] ='\0';
}

/* Connexion d'un client à un serveur par transmission du nom de la machine hôte du serveur
	Pré-conditions: c adresse d'un client, serveur en attente d'une connexion client
	Post-conditions:
	- l'éventuelle socket c->id initiale est supprimée
	- c->id devient le descripteur d'1 nouvelle socket connectée à 1 socket de domaine AF_INET d'adresse 'nom_hote:PORT'
	La connexion ne peut réussir que si un serveur est en attente de connexion client à l'adresse 'nom_hote:PORT' pour le domaine AF_INET
	Gestion des erreurs: 
	- si échec suppression socket: message idoine
	- si échec nouvelle socket, lecture informations serveur, connexion socket distant: message idoine & c->id == -1
*/
void client_connecter_par_nom(client* c, const char* nom_hote) {
	struct hostent* info_hote = NULL;
	struct sockaddr_in adr_serveur;

	#if (CLIENT_SERVEUR_COMPILE)
	printf("%s:%s(%p,'%s') IN:: c->id entree %d\n", __FILE__, __func__, (void*)c, nom_hote, c->id);
	#endif

	/* __ Récupération adresse de la socket distante */

	/* gethostbyname() renvoie un pointeur vers la structure hostent, ou un pointeur NULL si une erreur se produit, auquel cas h_errno contient le code d'erreur */
	info_hote =gethostbyname(nom_hote);
	if (info_hote == NULL) {
		printf("gethostbyname(): h_errno == %d\n", h_errno);
	}
	else {
		adr_serveur.sin_addr.s_addr =( (*(struct in_addr*) info_hote->h_addr).s_addr );

		/* __ Nouvelle socket connectée à la socket distante  */
		client_connecter(c, &(adr_serveur));
	}

	#if (CLIENT_SERVEUR_COMPILE == 0)
	printf("%s:%s(%p,'%s') OUT:: c->id sortie %d\n", __FILE__, __func__, (void*)c, nom_hote, c->id);
	#endif
}

/* Connexion d'un client à un serveur par transmission de l'IP de la machine hôte du serveur
	Pré-conditions: c adresse d'un client, serveur en attente d'une connexion client
	Post-conditions:
	- l'éventuelle socket c->id initiale est supprimée
	- c->id devient le descripteur d'1 nouvelle socket connectée à 1 socket de domaine AF_INET d'adresse 'nom_hote:PORT'
	La connexion ne peut réussir que si un serveur est en attente de connexion client à l'adresse 'nom_hote:PORT' pour le domaine AF_INET
	Gestion des erreurs: 
	- si échec suppression socket: message idoine
	- si échec nouvelle socket, lecture informations serveur, connexion socket distant: message idoine & c->id == -1
*/
void client_connecter_par_ip(client* c, char ip[IP_TAILLE_MAX +1]) {
	struct sockaddr_in adr_serveur;

	#if (CLIENT_SERVEUR_COMPILE)
	printf("%s:%s(%p,'%s') IN:: c->id entree %d\n", __FILE__, __func__, (void*)c, ip, c->id);
	#endif

	/* __ Recuperation adresse de la socket distant */

	/* inet_aton() retourne une valeur <> 0 si ip interpretee correctement, 0 sinon */
	if ( inet_aton(ip, &(adr_serveur.sin_addr)) == INET_ATON_KO) {
		printf("%s:%s:: échec inet_aton() pour l'ip %s\n", __FILE__, __func__, ip);
	}
	else {
		/* __ Nouvelle socket connectée à socket distante  */
		client_connecter(c, &(adr_serveur));
	}

	#if (CLIENT_SERVEUR_COMPILE)
	printf("%s:%s(%p,'%s') OUT:: c->id sortie %d\n", __FILE__, __func__, (void*)c, ip, c->id);
	#endif
}

/*	Sous-routine de client_connecter_par_nom() et client_connecter_par_ip(): connexion d'un client à un serveur par tansmission adresse hôte serveur
	Pré-conditions:	 c adresse d'un client
	Post-conditions:
	- l'eventuelle socket c->id initiale est supprimée
	- c->id devient le descripteur d'une nouvelle socket connectée à une socket de domaine AF_INET d'adresse 'adr_serveur->sin_addr.s_addr:PORT'
	La connexion ne peut réussir que si un serveur est en attente de connexion client à l'adresse 'adr_serveur->sin_addr.s_addr:PORT' pour le domaine AF_INET
	Gestion erreurs: 
	- si échec suppression socket: message idoine
	- si échec nouvelle socket, connexion socket distante: message idoine & c->id == -1
*/
static void client_connecter(client* c, struct sockaddr_in* adr_serveur) {
	int res;

	#if (CLIENT_SERVEUR_COMPILE)
	printf("%s:%s(%p,'%u') IN:: c->id entree %d", __FILE__, __func__, (void*)c, (adr_serveur->sin_addr).s_addr, c->id);
	#endif

	/* __ Creation nouvelle socket */
	client_new_socket(c);

	/* __ Connexion serveur	*/
	if (c->id != SYSTEME_KO) {
		/* Finalisation information socket distante (port & domaine)	*/
		adr_serveur->sin_port = htons(PORT);
		adr_serveur->sin_family = AF_INET;

		/* Connexion
			connect() retourne 0 en cas de succès et -1 (errno est alors mise à jour) en cas d'erreur */
		res = connect(c->id, (struct sockaddr*) (adr_serveur), sizeof(struct sockaddr));
		if (res == SYSTEME_KO) {
			perror("connect()");
			client_delete(c);
		}
	}

	#if (CLIENT_SERVEUR_COMPILE)
	printf("%s:%s(%p,'%u') OUT:: c->id sortie %d", __FILE__, __func__, (void*)c, (adr_serveur->sin_addr).s_addr, c->id);
	#endif
}

/* Initialise c->id à une socket nouvellement creee
	Pré-conditions: c adresse d'un client
	Post-conditions: eventuelle socket c->id initiale, c->id descripteur d'une nouvelle socket
	Gestion erreurs: 
	- si échec suppression socket: message idoine
	- si échec nouvelle socket: message idoine & c->id == -1
*/
static void client_new_socket(client* c) {
	/* Destruction eventuelle socket initiale */
	client_delete(c);

	/* Creation nouvelle socket */
	c->id = socket_new();
}

/* Supprime la socket du client dont l'adresse est passee en paramètre
	Pré-conditions: c adresse d'un client tel que c->id == -1 ou c->id descripteur d'une socket
	Post-conditions: eventuelle socket c->id supprimee, c->id == -1, buffer vide
	Gestion des erreurs: en cas d'échec (la variable errno est alors mise à jour), un message d'erreur est affiche
*/
void client_delete(client* c) {
	if (c->id != SYSTEME_KO) {
		socket_delete(c->id);
	}

	client_initialiser(c);
}

/* Renvoie l'adresse du buffer du client
	Pré-conditions: c adresse d'un client
*/
char* client_get_buffer(client* c) {
	return c->buffer;
}

/* Ecrit la chaine placee en paramètre dans le buffer du client
	Pré-conditions: c adresse d'un client
	Post-conditions: c->buffer contient la chaine 'texte'
	Gestion erreur: si chaine trop longue pour le buffer, chaine seulement partiellement copiee (un message est alors affiché)
*/
void client_set_buffer(client* c, const char* texte) {
	strncpy(c->buffer, texte, BUFFER_TAILLE -1);

	if (strlen(texte) <= BUFFER_TAILLE -1) {
		c->buffer[strlen(texte)] = '\0';
	}
	else {
		c->buffer[BUFFER_TAILLE -1] = '\0';
		printf("%s:%s:: la chaine '%s', trop longue pour le buffer de capacite %d, à seulement ete partiellement copiee.\n", __FILE__, __func__, texte, BUFFER_TAILLE -1);
	}
}

/* Accesseur client pour l'accès du serveur à son client local 
	Renvoie l'id du client
	Pré-conditions: c adresse d'un client
*/
static int client_get_id(client* c) {
	return c->id;
}

/* Accesseur client pour l'accès du serveur à son client local 
	Attribue au client l'id passe en paramètre
	Pré-conditions: c adresse d'un client, id descripteur d'une socket, c->id == -1
*/
static void client_set_id(client* c, int id) {
	c->id = id;
}

/* Renvoie 1 si le client est connecte, 0 sinon
	Pré-conditions: s adresse d'un serveur
*/
int client_est_connecte(client* c) {
	return (c->id != SYSTEME_KO);
}

/* Reception d'un message sur la socket passee en paramètre
	Pré-conditions: c adresse d'un client connecte
	Post-conditions: message ecrit dans c->buffer
	Valeur retour: nombre d'octets recus en cas de succès, -1 en cas d'échec (la variable errno est alors mise à jour)
	Gestion des erreurs: en cas d'échec, un message d'erreur est affiche
	/!\ NB1: l'eventuel contenu initial du buffer est perdu
		NB2: le buffer peut contenir la concatenation de plusieurs messages resultant de send() succèssifs par la socket distante
		NB3: recv() peut rester en attente. Cela peut etre gere par l'utilisation de l'option MSG_DONTWAIT et la gestion de l'erreur EAGAIN ou EWOULDBLOCK
	/!\
*/
int client_recevoir(client* c) {
	int res = SYSTEME_KO;
	
	#if (CLIENT_SERVEUR_COMPILE)
	printf("%s:%s(%p) IN:: c->buffer entree '%s'\n", __FILE__, __func__, (void*)c, c->buffer);
	#endif

	/* recv reste en attente d'un message tant qu'aucun message n'est disponible, message qu'il ecrit dans le buffer
		Valeur renvoyee: nombre d'octets recus en cas de succès, -1 en cas d'échec
	*/
	res = recv(c->id, c->buffer, BUFFER_TAILLE -1, 0);

	if (res == SYSTEME_KO) {
		perror("recv()");
	}
	else if (res < BUFFER_TAILLE) {
		c->buffer[res] = '\0';
	}
	else {
		c->buffer[BUFFER_TAILLE -1] = '\0';
	}

	#if (CLIENT_SERVEUR_COMPILE)
	printf("%s:%s(%p) OUT:: c->buffer sortie '%s', %d octets lus pour taille max contenu buffer %d\n", __FILE__, __func__, (void*)c, c->buffer, res, BUFFER_TAILLE -1);
	#endif

	return res;
}

/* Emission d'un message sur la socket passee en paramètre
	Pré-conditions: c adresse d'un client connecte
	Post-conditions: en cas de succès, message envoye et buffer vide
	Valeur retour: nombre d'octets envoyes en cas de succès, -1 en cas d'échec (la variable errno est alors mise à jour)
	Gestion des erreurs: en cas d'échec, un message d'erreur est affiche et le contenu du buffer n'est pas modifie
*/
int client_emettre(client* c) {
	int res = 0;
	
	#if (CLIENT_SERVEUR_COMPILE)
	printf("%s:%s(%p) IN:: c->buffer entree '%s' de taille strlen(c->buffer) %lu\n", __FILE__, __func__, (void*)c, c->buffer, strlen(c->buffer));
	#endif

	/* le contenu du buffer est envoye
		Valeur renvoyee: nombre d'octets envoyes en cas de succès, -1 en cas d'échec
	*/
	res = send(c->id, c->buffer, strlen(c->buffer), 0);

	if (res <0) {
		perror("send()");
	}
	else if (res <(int)strlen(c->buffer)) {
		printf("%s:%s(%p) erreur (non geree): seulement %d octets du message '%s' ont pu etre envoyes\n", __FILE__, __func__, (void*)c, res, c->buffer);
	}
	else {
		strcpy(c->buffer, "");
	}
	
	#if (CLIENT_SERVEUR_COMPILE)
	printf("%s:%s(%p) OUT:: c->buffer sortie '%s', %d octets envoyes\n", __FILE__, __func__, (void*)c, c->buffer, res);
	#endif

	return res;
}

/* ______________________________ Fonctions serveur	*/

/* Initialise le serveur dont l'adresse est passe en paramètre
	Pré-conditions: s adresse d'un serveur dont le champ s->id n'est pas le descripteur d'une socket
	Post-conditions: s->id == -1, structure s->client initialisee, champs s->nom_hôte et s->ip renseignes
	Gestion des erreurs: en cas d'échec de lectrure des informations de la machine hote, un message d'erreur est affiche
*/
void serveur_initialiser(serveur* s) {
	int res;
	struct hostent* info_hote = NULL;
	
	#if (CLIENT_SERVEUR_COMPILE)
	printf("%s:%s(%p) IN\n", __FILE__, __func__, (void*)s);
	#endif

	/* __ socket serveur */
	s->id = SYSTEME_KO;

	/* __ socket cliente */
	client_initialiser(& (s->client));

	/* __ informations hôte serveur */

	strcpy(s->ip,"");

	/* nom hôte */
	res = gethostname(s->nom_hote, sizeof(s->nom_hote) -1);
	if (res == SYSTEME_KO) {
		perror("gethostname()");
		strcpy(s->nom_hote,"");
	}
	else {
		s->nom_hote[sizeof(s->nom_hote) -1] = '\0'; /* securite en cas de depassement */

		/* ip */
		info_hote = gethostbyname(s->nom_hote);
		if (info_hote == NULL) {
			printf("gethostbyname(): h_errno == %d\n", h_errno);
		}
		else {
			strcpy(s->ip, inet_ntoa((*(struct in_addr*) info_hote->h_addr)));
		}
	}
	
	#if (CLIENT_SERVEUR_COMPILE)
	printf("%s:%s(%p) OUT:: s->id %d, s->nom_hôte '%s', s->ip '%s'\n", __FILE__, __func__, (void*)s, s->id, s->nom_hote, s->ip);
	#endif
}

/* Cree une socket dont le descripteur est place dans le champ id du serveur dont l'adresse est passee en paramètre
	Pré-conditions: s adresse d'un serveur initialise dont le champ s->id n'est pas le descripteur d'une socket
	Post-conditions: s->id socket liee à une adresse & mise à l'ecoute d'au plus 1 connexion client sur cette adresse
	Valeur retour: -1 en cas d'échec connexion
	Gestion des erreurs: en cas d'échec (construction socket, options socket, attachement à une adresse, mise en ecoute connexion client),
							un message d'erreur idoine est affiche et l'eventuelle socket construite est supprimee (s->id == -1)
*/
void serveur_creer(serveur* s) {
	int res;
	struct sockaddr_in sin;
	
	#if (CLIENT_SERVEUR_COMPILE)
	printf("%s:%s(%p) IN:: s->id %d, s->nom_hôte '%s', s->ip '%s'\n", __FILE__, __func__, (void*)s, s->id, s->nom_hote, s->ip);
	#endif

	/* __ Creation socket */

	/* nouvelle socket */
	res = socket_new();
	if (res != SYSTEME_KO) {
		s->id = res;

		/* option socket: option 'SO_REUSEADDR' permet d'attacher une socket à 1 port par bind() des lors qu'aucune socket active n'ecoute sur ce port
			-> cela permet d'utiliser de nouveau un meme port pour le serveur apres une sortie brutale du programme	*/
		res = setsockopt(s->id, SOL_SOCKET, SO_REUSEADDR, &optServeurYes, sizeof(int));
		if (res == SYSTEME_KO) {
			perror("setsockopt()");
		}
		else {
			/* specification adresse pour attachement */
			sin.sin_port = htons(PORT);
			sin.sin_family = AF_INET;
			sin.sin_addr.s_addr = htonl(INADDR_ANY);	/* adresse pour accepter toute connexion */

			/* attachement de la socket s->id à l'adresse specifiee */
			res = bind(s->id, (struct sockaddr*) &(sin), sizeof(struct sockaddr));
			if (res == SYSTEME_KO) {
				perror("bind()");
			}
			else {
				/* mise en ecoute d'au plus 1 connexion client */
				res = listen(s->id, NB_CLIENTS_MAX);
				if (res == SYSTEME_KO) {
					perror("listen()");
				}
			}
		}

		/* __ Gestion erreur */
		if (res == SYSTEME_KO) {
			socket_delete(s->id);
			s->id = SYSTEME_KO;
		}
	}
	
	#if (CLIENT_SERVEUR_COMPILE)
	printf("%s:%s(%p) OUT:: s->id %d\n", __FILE__, __func__, (void*)s, s->id);
	#endif
}

/* Accepte une connexion d'un client
	Pré-conditions: s adresse d'un serveur initialise, lie à une adresse par la fonction bind(), à l'ecoute d'au plus un client à cette adresse
	Post-conditions: si client initial, il est reinitialise ; dans tous les cas: nouvelle connexion client
	La connexion ne peut reussir que si un client demande la connexion à l'adresse 's->ip:PORT' pour le domaine AF_INET	
	Gestion erreur: en cas d'échec, un message est affiche
*/
void serveur_connecter_client(serveur* s) {
	int res;
	socklen_t len;
	struct sockaddr_in adr_serveur;
	
	#if (CLIENT_SERVEUR_COMPILE)
	printf("%s:%s(%p) IN: s->id %d, s->client.id initial %d\n", __FILE__, __func__, (void*)s, s->id, s->client.id);
	#endif

	/* __ (Re)Initialisation client */
	client_delete(& (s->client));

	/* __ mise à jour serveur */

	/* adresse de connexion */
	adr_serveur.sin_port =htons(PORT);
	adr_serveur.sin_family =AF_INET;
	res =(inet_aton(s->ip, &(adr_serveur.sin_addr)) == INET_ATON_KO ? SYSTEME_KO: SYSTEME_OK);
	if (res == SYSTEME_KO) {
		printf("%s:%s(%p) erreur inet_aton() pour l'ip '%s'\n", __FILE__, __func__, (void*)s, s->ip);
	}
	/* attente de connexion */
	else {
		res =accept(s->id, (struct sockaddr*) &adr_serveur, &len);
		if (res == SYSTEME_KO) {
			perror("accept()");
		}
		else {
			client_set_id(& (s->client), res);
		}
	}
	
	#if (CLIENT_SERVEUR_COMPILE)
	printf("%s:%s(%p) OUT: s->client.id sortie %d\n", __FILE__, __func__, (void*)s, s->client.id);
	#endif
}

/* Deconnecte le client eventuellement connecte
	Pré-conditions: s adresse d'un serveur
	Post-conditions: si client initial, il est reinitialise
*/
void serveur_deconnecter_client(serveur* s) {
	client_delete(& (s->client));
}

/* Supprime la socket serveur apres avoir supprime l'eventuelle socket cliente
	Pré-conditions: s adresse d'un serveur
	Post-conditions: s et s->client reinitialises
*/
void serveur_delete(serveur* s) {
	/* suppression socket client (le cas echeant) */
	client_delete(& (s->client));

	/* suppression socket serveur */
	socket_delete(s->id);
}

/* Renvoie le nom de l'hôte du serveur
	Pré-conditions: s adresse d'un serveur
*/
const char* serveur_get_nom_hote(serveur* s) {
	return s->nom_hote;
}

/* Renvoie l'adresse de l'hôte du serveur
	Pré-conditions: s adresse d'un serveur
*/
const char* serveur_get_ip(serveur* s) {
	return s->ip;
}

/* Renvoie l'adresse du buffer de la connexion au client
	Pré-conditions: s adresse d'un serveur
*/
char* serveur_get_buffer(serveur* s) {
	return client_get_buffer(& (s->client));
}

/* Ecrit la chaine placee en paramètre dans le buffer de la socket cliente de s
	Pré-conditions: s adresse d'un serveur
	Post-conditions: s->client->buffer contient la chaine 'texte'
	Gestion erreur: si chaine trop longue pour le buffer, chaine seulement partiellement copiee (un message est alors affiché)
*/
void serveur_set_buffer(serveur* s, const char* texte) {
	client_set_buffer(& (s->client), texte);
}

/* Renvoie 1 si le serveur est connecte, 0 sinon
	Pré-conditions: s adresse d'un serveur
*/
int serveur_est_connecte(serveur* s) {
	return (s->id != SYSTEME_KO);
}

/* Renvoie 1 si un client est connecte, 0 sinon
	Pré-conditions: s adresse d'un serveur
*/
int serveur_est_client_connecte(serveur* s) {
	return ( (s->id != SYSTEME_KO) && (client_get_id(& (s->client)) != SYSTEME_KO) );
}

/* Reception d'un message sur la socket
	Pré-conditions: s adresse d'un serveur auquel un client est connecte
	Post-conditions: message ecrit dans s->client.buffer
	Valeur retour: nombre d'octets recus en cas de succès, -1 en cas d'échec (la variable errno est alors mise à jour)
	Gestion des erreurs: en cas d'échec, un message d'erreur est affiche
	/!\ NB1: l'eventuel contenu initial du buffer est perdu
		NB2: le buffer peut contenir la concatenation de plusieurs messages resultant de send() succèssifs par la socket distante
		NB3: recv() peut rester en attente. Cela peut etre gere par l'utilisation de l'option MSG_DONTWAIT et la gestion de l'erreur EAGAIN ou EWOULDBLOCK
	/!\
*/
int serveur_recevoir(serveur* s) {
	return client_recevoir(& (s->client));
}

/* Emission d'un message sur la socket passee en paramètre
	Pré-conditions: s adresse d'un serveur auquel un client est connecte
	Post-conditions: en cas de succès, message envoye et buffer vide
	Valeur retour: nombre d'octets envoyes en cas de succès, -1 en cas d'échec (la variable errno est alors mise à jour)
	Gestion des erreurs: en cas d'échec, un message d'erreur est affiche et le contenu du buffer n'est pas modifie
*/
int serveur_emettre(serveur* s) {
	return client_emettre(& (s->client));
}

 /* Youssef EL OTMANI 12208301
    Je déclare qu'il s'agit de mon propre travail.
    Ce travail a été réalisé intégralement par un être humain. */


/* fichiers de la bibliothèque standard */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

/* bibliothèque standard unix */
#include <unistd.h>		/* close, read, write */
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <dirent.h>
#include <errno.h>
/* spécifique à internet */
#include <arpa/inet.h>		/* inet_pton */
/* spécifique aux comptines */
#include "comptine_utils.h"

#define PORT_WCP 4321

#include <pthread.h> 

void usage (char *nom_prog)
{
  fprintf (stderr, "Usage: %s repertoire_comptines\n" "serveur pour WCP (Wikicomptine Protocol)\n" "Exemple: %s comptines\n", nom_prog, nom_prog);
}

/** Retourne en cas de succès le descripteur de fichier d'une socket d'écoute
 *  attachée au port port et à toutes les adresses locales. */
int creer_configurer_sock_ecoute (uint16_t port);

/** Écrit dans le fichier de desripteur fd la liste des comptines présents dans
 *  le catalogue c comme spécifié par le protocole WCP, c'est-à-dire sous la
 *  forme de plusieurs lignes terminées par '\n' :
 *  chaque ligne commence par le numéro de la comptine (son indice dans le
 *  catalogue) commençant à 0, écrit en décimal, sur 6 caractères
 *  suivi d'un espace
 *  puis du titre de la comptine
 *  une ligne vide termine le message */
void envoyer_liste (int fd, struct catalogue *c);

/** Lit dans fd un entier sur 2 octets écrit en network byte order
 *  retourne : cet entier en boutisme machine. */
uint16_t recevoir_num_comptine (int fd);

/** Écrit dans fd la comptine numéro ic du catalogue c dont le fichier est situé
 *  dans le répertoire dirname comme spécifié par le protocole WCP, c'est-à-dire :
 *  chaque ligne du fichier de comptine est écrite avec son '\n' final, y
 *  compris son titre, deux lignes vides terminent le message */
void envoyer_comptine (int fd, const char *dirname, struct catalogue *c, uint16_t ic);

void *f (void *arg); /*Pour les p_thread */
struct thread /*Pour les p_thread */
{
  int s; /* socket de connexion */
  char dirname[256];
  struct catalogue *cata;/* il est plus facile de mettre un pointeur pour pouvoir directement transmettre le résultat de creer_catalogue */
};/* on attribue à chaque client une structure avec les infos nécessaires */


int main (int argc, char *argv[])
{
  if (argc != 2)
    {
      usage (argv[0]);
      return 1;
    }
    
  int s_l = creer_configurer_sock_ecoute (PORT_WCP);
  
  struct sockaddr_in sa_clt;
  socklen_t sl = sizeof (sa_clt);
  
  struct catalogue *cata = creer_catalogue (argv[1]);
  
  struct thread *t;

  int i = 0; /* Compteur de client, on commence avec le client numéro 0 */

  pthread_t * tab;

  char debug[256];
  strcpy(debug,argv[1]); /* afin de remplir le champs dirname de la structure thread */
  	
  int s;
  
  while (1)
    {
      s = accept (s_l, (struct sockaddr *) &sa_clt, &sl);
      if (s < 0)
	{
	  perror ("accept failed");
	  exit (1);
	}
	puts ("connexion établie");

      t = malloc (sizeof (struct thread));
      t->s = s;
      strcpy(t->dirname,debug);
      t->cata = cata;

      pthread_t tab[i];/* Creéation d'un nouveau thread pour le nouveau client */
      
      if (pthread_create(&tab[i], NULL, f, t) < 0)
	{
	  perror ("pthread failed");
	  exit (1);
	}
	
      i++;/* Prochain client */
    }

  close (s_l);

  int j;

  for (j = 0; j < i; j++)
    {
      pthread_join (tab[j], NULL);
    }

  return 0;
}

void * f (void *arg)
{
  /* définie */
  uint16_t m;
  struct thread *t = (struct thread * ) arg; /* cast */
  
  struct sockaddr_in sa_clt;
  socklen_t sl = sizeof (sa_clt);
  getpeername(t->s,(struct sockaddr *) &sa_clt, &sl);
  
  
  char ipp[256];
  if(inet_ntop(AF_INET,&(sa_clt.sin_addr),ipp, 256)== NULL){
  exit(1);
  }
  
  char buf[2048];
  
  int fd = open("log_clients.txt", O_WRONLY | O_CREAT | O_APPEND, 0666);
  if (fd < 0) {
 perror("open failed");
 exit(1);
 }
  sprintf(buf, "[%s] connexion établie avec le client\n",ipp );
  write(fd,buf,strlen(buf));
  
  envoyer_liste (t->s, t->cata);
  m = recevoir_num_comptine (t->s);

  sprintf(buf, "[%s] comptine demandé numéro %d correspondante à %s",ipp, m, t->cata->tab[m]->titre);
  write(fd,buf,strlen(buf));
  
  envoyer_comptine (t->s,t->dirname, t->cata, m);
  sprintf(buf, "[%s] comptine envoyé\n",ipp);
  write(fd,buf,strlen(buf));

  close (t->s);
  sprintf(buf, "[%s] connexion rompue avec le client\n",ipp);
  write(fd,buf,strlen(buf));
  free (t);/*nous avons précédemment allouer de l'espace sur le tas dans le main */
  
  close(fd);
  
  return NULL;
}



int creer_configurer_sock_ecoute (uint16_t port)
{
  /* définie */
  int s_l = socket (AF_INET, SOCK_STREAM, 0);
  if (s_l < 0)
    {
      perror ("socket failed");
      exit (1);
    }
    
  struct sockaddr_in sa = {.sin_family = AF_INET, .sin_port = htons (port), .sin_addr.s_addr = htonl (INADDR_ANY) };
  socklen_t sl = sizeof (sa);

  int opt = 1;
  setsockopt (s_l, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof (int));
  
  if (bind (s_l, (struct sockaddr *) &sa, sl) < 0)
    {
      perror ("bind failed");
      exit (3);
    }
    
  if (listen (s_l, 128) < 0)
    {
      perror ("listen failed");
      exit (2);
    }
    
  puts ("en attente d'une première connexion");
  
  return s_l;
}



void envoyer_liste (int fd, struct catalogue *c)
{
  /* définie */
  for (int i = 0; i < c->nb && i < UINT16_MAX; i++)
    {
      dprintf (fd, "%6d %s", i, c->tab[i]->titre);
    }
  dprintf (fd, "\n");
  
  return;
}

uint16_t recevoir_num_comptine (int fd)
{
  /* définie */
  uint16_t m;
  read (fd, &m, 2);
  m = ntohs (m);
  return m;
}

void envoyer_comptine (int fd, const char *dirname, struct catalogue *c, uint16_t ic)
{
  /* définie */
  char buffer[256];
  char *chemin;
  ssize_t m;
  
  chemin = malloc (256 * sizeof (char));
  strcat (strcat (strcpy (chemin, dirname), "/"), c->tab[ic]->nom_fichier);
  
  int fd_2 = open (chemin, O_RDONLY);
  if (fd_2 < 0)
    {
      perror ("open failed");
    }
    
    free(chemin);
    
  do
    {
      m = read (fd_2, buffer, 256);
      buffer[m] = '\0';
      dprintf (fd, "%s", buffer);
    }
  while (m > 0);
  
  dprintf (fd, "\n\n");
  
  close (fd_2);
  return; 
}

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
#include <sys/types.h>
#include <sys/socket.h>
/* spécifique à internet */
#include <arpa/inet.h>		/* inet_pton */
/* spécifique aux comptines */
#include "comptine_utils.h"

#define PORT_WCP 4321


void usage (char *nom_prog)
{
  fprintf (stderr, "Usage: %s addr_ipv4\n" "client pour WCP (Wikicomptine Protocol)\n" "Exemple: %s 208.97.177.124\n", nom_prog, nom_prog);
}

/** Retourne (en cas de succès) le descripteur de fichier d'une socket
 *  TCP/IPv4 connectée au processus écoutant sur port sur la machine d'adresse
 *  addr_ipv4 */
int creer_connecter_sock (char *addr_ipv4, uint16_t port);

/** Lit la liste numérotée des comptines dans le descripteur fd et les affiche
 *  sur le terminal.
 *  retourne : le nombre de comptines disponibles */
uint16_t recevoir_liste_comptines (int fd);

/** Demande à l'utilisateur un nombre entre 0 (compris) et nc (non compris)
 *  et retourne la valeur saisie. */
uint16_t saisir_num_comptine (uint16_t nb_comptines);
void viderBuffer(void);

/** Écrit l'entier ic dans le fichier de descripteur fd en network byte order */
void envoyer_num_comptine (int fd, uint16_t nc);

/** Affiche la comptine arrivant dans fd sur le terminal */
void afficher_comptine (int fd);

int main (int argc, char *argv[])
{
  if (argc != 2)
    {
      usage (argv[0]);
      return 1;
    }

  int s = creer_connecter_sock (argv[1], PORT_WCP);
  uint16_t nb_compt = recevoir_liste_comptines (s);
  uint16_t n = saisir_num_comptine (nb_compt);
  
  envoyer_num_comptine (s, n);
  afficher_comptine (s);
  
  close (s);
  return EXIT_SUCCESS;
}

int creer_connecter_sock (char *addr_ipv4, uint16_t port)
{
  /* définie */
  int s = socket (AF_INET, SOCK_STREAM, 0);
  if (s < 0)
    {
      perror ("socket failed");
      exit (1);
    }
    
  struct sockaddr_in sa = {.sin_family = AF_INET,.sin_port = htons (port) };
  if (inet_pton (AF_INET, addr_ipv4, &sa.sin_addr) != 1)
    {
      fprintf (stderr, "adresse ipv4 non valable \n");
      exit (1);
    }
    
  socklen_t sl = sizeof (sa);
  
  if (connect (s, (struct sockaddr *) &sa, sl) < 0)
    {
      perror ("connect failed");
      exit (3);
    }
    
  return s;
}

uint16_t recevoir_liste_comptines (int fd)
{
  /* définie */
  char buffer[256];
  int n = 0;
  uint16_t nb_compt = 0;
  
  do
    {
      n = read_until_nl (fd, buffer);
      if (n < 1)
	{
	  break; // while normal
	}
	
      printf ("%s", buffer);
      
      nb_compt ++;
    }
  while (n > 0);
  
  return nb_compt;
}

uint16_t saisir_num_comptine (uint16_t nb_comptines)
{
  /* définie */
  uint16_t nb;
  int i ; /* Variables nécessaire pour bloquer les entrées de lettre et caractères spéciaux */
  
  do
    {
      printf ("Quelle comptine voulez-vous ? (Entrer un entier entre 0 et %"PRIu16 ") : ", nb_comptines);
      
      i = scanf ("%" SCNu16, &nb);
      
      if (nb >= nb_comptines || i < 1)
	{
	  viderBuffer();
	  printf ("Votre Saisie est incorrecte\n");
	}

    }
  while (nb >= nb_comptines || i < 1);
  
  return nb;
}

void viderBuffer(void)
{
  int c;
  while((c=getchar()) != EOF && c != '\n');/* nous vidons le buffer de scanf */
 
}

void envoyer_num_comptine (int fd, uint16_t nc)
{
  /* définie */
  nc = htons (nc);
  write (fd, &nc, 2);
}

void afficher_comptine (int fd)
{
  /* définie */
  ssize_t n;
  char buffer[256];
  
  do
    {
      n = read (fd, buffer, 256);
      if (buffer[n - 2] == '\n' && buffer[n - 1] == '\n')
	{
	  buffer[n - 2] = '\0';
	  printf ("%s", buffer);
	  break;
	}
      else
	{
	  buffer[n] = '\0';
	  printf ("%s", buffer);
	}
    }
  while (n > 0);
  
  return;
}

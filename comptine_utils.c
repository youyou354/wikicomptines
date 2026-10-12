 /* Youssef EL OTMANI 12208301
    Je déclare qu'il s'agit de mon propre travail.
    Ce travail a été réalisé intégralement par un être humain. */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include "comptine_utils.h"


int read_until_nl (int fd, char *buf)
{
  /* définie */
  int i = 0;
  char cara;
  while (read (fd, &cara, 1) == 1)
    {
      if (cara == '\n')
	{
	  break;
	}
      buf[i] = cara;
      i++;
    }
  buf[i] = '\n';
  buf[i + 1] = '\0';
  return i;
}

int est_nom_fichier_comptine (char *nom_fich)
{
  /* définie */
  if (strlen (nom_fich) > 4)
    {
      while ((*nom_fich) != '\0')
	{
	  nom_fich++;
	}
      nom_fich--;
      if (*nom_fich == 't' && *(nom_fich - 1) == 'p' && *(nom_fich - 2) == 'c' && *(nom_fich - 3) == '.')
	{
	  return 1;
	}
    }
  return 0;
}

struct comptine * init_cpt_depuis_fichier (const char *dir_name, const char *base_name)
{
  /* définie */
  struct comptine * compt = malloc (sizeof (struct comptine));
  if (compt == NULL)
    {
      perror ("malloc failed");
      return NULL;
    }
  compt->titre = malloc (256 * sizeof (char));
  if (compt->titre == NULL)
    {
      perror ("malloc failed");
      return NULL;
    }
  compt->nom_fichier = malloc (256 * sizeof (char));
  if (compt->nom_fichier == NULL)
    {
      perror ("malloc failed");
      return NULL;
    }

  strcpy (compt->nom_fichier, base_name);

  if (est_nom_fichier_comptine (compt->nom_fichier) == 0)
    {
      return NULL;
    }

  int fd = open (dir_name, O_RDONLY);
  if (fd < 0)
    {
      perror ("open failed");
      return NULL;
    }

  read_until_nl (fd, compt->titre);

  close (fd);

  return compt;
}

void liberer_comptine (struct comptine *cpt)
{
  /* définie */
  free (cpt->nom_fichier);
  free (cpt->titre);
  free (cpt);
}

struct catalogue * creer_catalogue (const char *dir_name)
{
  /* définie */
  DIR *dir = opendir (dir_name);
  if (dir == NULL)
    {
      perror ("opendir failed");
      return NULL;
    }
    
  struct dirent * ent;/*vu dans man readdir*/

  struct catalogue *ctg = malloc (sizeof (struct comptine));
  if (ctg == NULL)
    {
      perror ("malloc failed");
      return NULL;
    }
    
  while ((ent = readdir (dir)) != NULL)
    {
      if ( (ent->d_name[0]) != '.' && est_nom_fichier_comptine (ent->d_name) ) /*on considère que .cpt n'est pas un nom valable*/
	{
	  ctg->nb+=1;
	}
    }
    
  rewinddir (dir);
  
  ctg->tab = malloc (sizeof (struct comptine) * ctg->nb);
  if (ctg->tab == NULL)
    {
      perror ("malloc failed");
      return NULL;
    }

  int i = 0;
  char * chemin;

  while ((ent = readdir (dir)) != NULL)
    {
      if ((ent->d_name[0]) != '.' && est_nom_fichier_comptine (ent->d_name))
	{
	  chemin = malloc (256 * sizeof (char));
	  if (chemin == NULL)
	    {
	      perror ("malloc failed");
	      return NULL;
	    }
	    
	  strcat (strcat (strcpy (chemin, dir_name), "/"), ent->d_name);

	  ctg->tab[i] = init_cpt_depuis_fichier (chemin, ent->d_name);
	  
	  free (chemin);
	  
	  i++;
	}
    }
  closedir (dir);
  return ctg;
}

void liberer_catalogue (struct catalogue *c)
{
  /* définie */
  int j = 0;
  
  while (j <= c->nb)
    {
      liberer_comptine (c->tab[j]);
      j++;
    }
    
  free (c->tab);
  free (c);
}

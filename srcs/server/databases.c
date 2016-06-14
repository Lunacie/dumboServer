/*                               -*- Mode: C -*- 
 * 
 * Filename: databases.c
 * Description: 
 * Author: Lunacie
 * Created: Wed Sep  2 13:12:54 2015 (+0200)
 * Last-Updated: 
 *           By: 
 *     Update #: 0
 * 
 */

/* Change Log:
 * 
 * 
 * 
 */


#include		"server.h"
#include		"databases.h"
#include		"handle_error.h"
#include		"list.h"


static char		*database = NULL;
static char		*host = NULL;
static char		*script = NULL;
static char		*result = NULL;

static int		dbSystem = MYSQL;


char			*setDatabase(char *new)
{
  if (new)
    database = new;
  return database;
}

int			setDatabaseSystem(int new)
{
  if (new)
    dbSystem = new;
  return dbSystem;
}

char			*setDBHost(char *new)
{
  host = new;
  return host;
}



char			*setScript(char *new)
{
  if (new)
    script = new;
  return script;
}


void			setRequestResult(char *new)
{
  result = new;
}


char			*getNextResultValue(char **ptr)
{
  char			*start;

  if (result)
    {
      start = result;
      if (!*result)
	{
	  *ptr = NULL;
	  return NULL;
	}
      while (*result && *result != '\t')
	result++;
      if (*result)
	{
	  *result = '\0';
	  result++;
	}
      *ptr = strdup(start);
      if (!*result)
	result = NULL;
      if (*ptr && (*ptr)[strlen(*ptr) - 1] == '\n')
	(*ptr)[strlen(*ptr) - 1] = '\0';
      return *ptr;
    }
  return NULL;
}

char			*execRequestGetValue(void)
{
  t_list		*ret;
  t_list		*tmp;
  char			*result;

  result = NULL;
  if ((ret = execRequest()) && (tmp = ret) && (ret = ret->next) && ret->content)
    {
      result = strdup(ret->content);
      if (result && result[strlen(result) - 1] == '\n')
	result[strlen(result) - 1] = '\0';
      destroyList(tmp, TRUE);
    }
  return result;
}

t_list			*execRequest(void)
{
  t_list		*list;
  char			cmd[4096 + 1024];
  char			buffer[1024];
  FILE			*fp;

  if (!script || !database)
    return NULL;

  /* if (dbSystem == MYSQL) */
  /*   { */
      if (access("/home/lunacie/server/.prod", F_OK) != FAILURE)
	{
	  if (!host)
	    snprintf(cmd, 4096 + 1024, 
		     "mysql --user=root --password=\"insertPasswordHereOnNonGithubVersion\" %s --execute \"%s\"",
		     database , script);
	  else
	    snprintf(cmd, 4096 + 1024, 
		     "mysql %s %s --execute \"%s\"",
		     host, database , script);
	    
	}
      else
	snprintf(cmd, 4096 + 1024, 
		 "mysql --user=root %s --execute \"%s\"",
		 database , script);
  /*   } */
  /* else if (dbSystem == POSTGRESQL) */
  /*   { */
  /*     /\* if (access("./.prod", F_OK) != FAILURE) *\/ */
  /*     /\* 	snprintf(cmd, 4096 + 1024, *\/ */
  /*     /\* 		 "mysql --user=root --password=\"PASSWORDFORNONGITHUB\" %s --execute \"%s\"", *\/ */
  /*     /\* 		 database , script); *\/ */
  /*     if (access("./.prod", F_OK) != FAILURE) */
  /*     	snprintf(cmd, 4096 + 1024, */
  /*     		 "sudo -u postgres psql -c  \"%s\"", */
  /*     		 script); */
  /*     else */
  /* 	snprintf(cmd, 4096 + 1024,  */
  /* 		 "mysql --user=root %s --execute \"%s\"", */
  /* 		 database , script); */
  /*   } */

  printf("\t\tExecuting SQL command : '%s'\n", cmd);
  if (!(fp = popen(cmd, "r")))
    return (t_list *)handle_error("popen", (long int)NULL);
  list = NULL;
  while (fgets(buffer, 1024, fp))
    { 
      char *str;
      char *new;
      size_t step;
      char *tmp;
      int value;

      /* printf("buffer : '%s'\n", buffer); */
      if (buffer[strlen(buffer - 1)] == '\n' || buffer[strlen(buffer - 1)] == '\r')
	buffer[strlen(buffer - 1)] = '\0';
      str = buffer;

      if (dbSystem == POSTGRESQL)
	{
	  if (strlen(str) > 2 && str[0] == '-' && str[1] == '-');
	  else if (sscanf(str, "(%d row", &value) == 1);
	  else
	    {
	      step = 0;
	      while (*str && (*str == ' '|| *str == '\t') && (step = 1))
		str++;
	      /* while ((step >= 1) && *str && (*str >= '0'& *str <= '9') && (step = 2)) */
	      /* 	str++; */
	      /* while ((step >= 2) && *str && (*str == ' '|| *str == '\t') && (step = 3)) */
	      /* 	str++; */
	      while ((step >= 1) && *str && *str == '|' && (step = 2))
		str++;
	      while ((step >= 2) && *str && (*str == ' '|| *str == '\t') && (step = 3))
		str++;

	      if (!(new = malloc(strlen(str) + 1)))
		{
		  perror("malloc");
		  exit(FAILURE);
		}
	      *new = '\0';
	      step = 0;
	      while (*str)
		{
		  tmp = str;
		  while (*str && *str == ' ')
		    str++;
		  if (*str && *str == '|')
		    {
		      strncat(new, "\t", 1);
		      str++;
		      while (*str  && (*str == ' ' || *str == '\t'))
			str++;
		    }
		  /* if (*str && (*str == '\n' || *str == '\r')) */
		  /*   { */
		  /*     str++; */
		  /*   } */
		  else
		    {
		      str = tmp;
		      strncat(new, str, 1);
		      str++;
		      while (*str && strlen(str) > 2 && (str[0] == ' ' && str[1] == ' '))
			str++;
		    }
		}
	      strncat(new, "\0", 1);

	      if (strlen(new))
		{
		  if (strlen(new) == 1 && (*new == '\n' || *new == '\r'));
		  else
		    {
		      tmp = new;
		      while (*tmp && *tmp == ' ')
			tmp++;
		      if (!*tmp)
			{
			  /* free(new); */
			  new = NULL;
			}
		      if (new)
			{
			  /* printf("Edited version = '%s'\n", new); */
			  list = addToList(list, /* strdup(str) */new);
			}
		    }
		}
	    }
	}
      else
	list = addToList(list, strdup(buffer));
    }
  pclose(fp);
  return list;
}

/*                               -*- Mode: C -*- 
 * 
 * Filename: accountValidation.c
 * Description: 
 * Author: Lunacie
 * Created: Mon Jan 11 21:17:32 2016 (+0100)
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

#include			"server.h"
#include			"program.h"
#include			"handle_error.h"
#include			"list.h"
#include			"strings.h"
#include			"response.h"
#include			"execution.h"
#include			"session.h"
#include			"projects.h"
#include			"http/http.h"
#include			"http/html/html.h"
#include			"databases.h"
#include			"parameters.h"
#include			"crypto.h"

#include			"projects/lapothicaire.h"

static char			*id;
static char			*username;

static t_list		*accountValidation(t_list **script, t_socket *client)
{
  t_list		*list;
  t_list		*res;
  char			cmd[1024];

  list = NULL;
  if (!(id = getParameter(client, "id")))
    {
      list = addToList(list, "Une erreur est subvenue\n");
      printf("Missing id parameter\n");
      return commitResult(list, script);
    }
  if (!(username = getParameter(client, "user")))
    {
      list = addToList(list, "Une erreur est subvenue\n");
      printf("Missing username parameter\n");
      return commitResult(list, script);
    }
  printf("Id = '%s'\n", id);
  snprintf(cmd, 1024,
	   "SELECT validationKey FROM users WHERE validationKey='%s' AND username='%s';",
	   id, username);
  setDatabase("lapothicaire");
  setScript(cmd);
  if (res = execRequest())
    {
      destroyList(res, TRUE);
      
      snprintf(cmd, 1024,
	       "UPDATE users SET validationKey='0' WHERE validationKey='%s' AND username='%s';"\
	       "SELECT ROW_COUNT();",
	       id, username);
      setScript(cmd); 
      if ((res = execRequest()) && (res = res->next) &&
	  res->content && *((char *)(res->content)) == '1')	
	list = addToList(list, "Votre adresse mail a été validée avec succès\n");
      else
	{
	  list = addToList(list, "Une erreur est subvenue\n");
	  printf("No '%s' key was found \n", id);
	  return commitResult(list, script);
	}	
    }
  else
    {
      list = addToList(list, "Une erreur est subvenue\n");
      printf("No '%s' key was found \n", id);
      return commitResult(list, script);
    }
    
  
  return commitResult(list, script); 
}

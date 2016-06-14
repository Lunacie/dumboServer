/*                               -*- Mode: C -*- 
 * 
 * Filename: exec.c
 * Description: 
 * Author: Lunacie
 * Created: Tue Sep  1 13:45:07 2015 (+0200)
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
#include			"projects.h"
#include			"parameters.h"
#include			"databases.h"
#include			"session.h"
#include			"http/http.h"
#include			"http/html/html.h"

#include			"projects/wunderbar.h"

#define				NB_FUNCTIONS			2

static t_function		functionsArray[NB_FUNCTIONS] = 
  {
    {"auth", &_auth},
    {"exec", &_execRequest}
  };

t_list				*_exec(t_list **script, t_socket *client)
{
  size_t			x;
  char				*name;
  t_list			*new;
  t_list			*list;

  list = *script;
  if (!(name = getFunctionName((char *)(list->content))))
    {
      list->content = (void *)INVALID_SCRIPT;
      return list;
    }
  for (x = 0; x < NB_FUNCTIONS; x++)
    {
      if (stringsAreTheSame(functionsArray[x].name, name))
	{
	  if (functionsArray[x]._ptr != NULL)
	    {
	      return functionsArray[x]._ptr(script, client);
	    }
	  else
	    {
	      printf("Script `%s' : null pointer.\n", name);
	      list->content = (void *)NULL_FUNCTION_SCRIPT;
	      return list;
	    }
	}
    }
  printf("Script `%s' was not found.\n", name);
  list->content = (void *)(NOT_FOUND_SCRIPT);
  return list;
}



#define			COOKIE_SESSIONID		"DUMBOSESSIONID"
#define			COOKIE_SESSIONusername		"DUMBOSESSIONusername"

static t_list		*_auth(t_list **script, t_socket *client)
{
  t_list		*list;
  char			*result;
  char			*host;
  char			*tmp;
  char			cmd[1024];
  char			buffer[1024];
  FILE			*fp;
  char			request[1024];
  int			session;
  
  char			*sessionUsername;
  char			*username;
  char			*password;

  printf("\t\tAuth() -> Starting Execution\n");
  
  if (!client)
    return *script;
  // Is session already running ?
  if (checkForExistingSession(client->request, COOKIE_SESSIONID, COOKIE_SESSIONusername))
    {
      list = addToList(NULL, HTTP_OK);
      list = addToList(list, "\r\n");
      printf("\t\tAthentification success; Session already running\n");
    }
  // Session not running but user is trying to auth
  else if ((username = getParameter(client, "username")) &&
	   (password = getParameter(client, "password")))
    {  
      // Getting Parameter
      printf("\t\tParamaters recieved :\n");
      printf("\t\t\tUsername = '%s' \n", username);
      printf("\t\t\tPassword = '%s' \n", password);

      setDatabase("wunderbar");
      setDBHost("--user=USERNAME --password=password123 -h HOST");
      snprintf(request, 1024,
	       "SELECT * FROM users WHERE username='%s'AND password='%s';",
	       username, password);
      setScript(request);
      if (!(execRequest()))
	{
	  setDBHost(NULL);
	  printf("\t\tAthentification failed; Returning 403\n");
	  (*script)->content = HTTP_403;
	  return *script;
	}
      else
	{
	  setDBHost(NULL);
	  if (!(tmp = createNewSession(username, client->request, COOKIE_SESSIONID, COOKIE_SESSIONusername)))
	    {
	      (*script)->content = HTTP_403;
	      printf("\t\tCould not create session; Returning 403\n");
	      return *script;
	    }
	  else
	    {
	      list = addToList(NULL, HTTP_OK);
	      list = addToList(list, tmp);
	      printf("\t\tAthentification success; New Session created\n");
	    }
	}
    }
  // session not running and missing password and or username...
  else 
    {
      	  printf("\t\tAthentification failed; Returning 403\n");
	  (*script)->content = HTTP_403;
	  return *script;
    }
  setDBHost(NULL);

  // Saving results to return
  if (list)
    {
      result = listToString(list);
      if (result)
	{
	  destroyList(list, FALSE);
	  (*script)->content = result;
	}
    }

  if (!*script)
    printf("\t\tExecution failed; Returning null\n");
  else
    printf("\t\tExecution done; Returning\n");

  return *script;
}

static t_list		*_execRequest(t_list **script, t_socket *client)
{
  t_list		*list;
  char			*result;
  char			*host;
  t_list		*tmp;
  char			cmd[1024];
  char			buffer[1024];
  FILE			*fp;
  char			request[1024];
  char			*command;

  list = NULL;
  printf("\t\tExec() -> Starting Execution\n");
  
  if (!client)
    return *script;
  // Is session already running ?
      setDatabaseSystem(POSTGRESQL);
  if (!checkForExistingSession(client->request, COOKIE_SESSIONID, COOKIE_SESSIONusername))
    {
      (*script)->content = HTTP_403;
      printf("\t\tAuthentification Needed; Returning 403\n");
      return *script;
    }
        // Getting Parameter
  if ((command = getParameter(client, "cmd")))
    {
      command = replaceCharactersInStr('+', ' ', command);
      printf("\t\tParamater recieved :\n");
      printf("\t\t\tCmd = '%s' \n", command);
    }
  else
    {
      (*script)->content = HTTP_417;
      printf("\t\tMissing Parameter; Returning 417\n");
      return *script;
    }
      setDatabase("wunderbar");
      setDatabaseSystem(POSTGRESQL);
	
      /* snprintf(request, 1024, */
      /* 	       "SELECT * FROM users WHERE username='%s'AND password='%s';", */
      /* 	       username, password); */
      setScript(command);
      setDBHost("--user=USERNAME --password=password123 -h HOST");
      setDatabaseSystem(MYSQL);
      if (!(tmp = execRequest()))
  	{
	  setDBHost(NULL);
  	  printf("\t\tCould Not Execute request; Returning 417\n");
  	  (*script)->content = HTTP_417;
  	  return *script;
  	}
      else
	{
	  setDBHost(NULL);
	  list = addToList(NULL, HTTP_OK);
	  list = addToList(list, "\r\n");
	  list = addListToList(list, tmp);
	  printf("\t\tExecution successful; Returning 200\n");
	}
      setDBHost(NULL);

  // Saving results to return
  if (list)
    {
      result = listToString(list);
      if (result)
  	{
  	  destroyList(list, FALSE);
  	  (*script)->content = result;
  	}
    }

  if (!*script)
    printf("\t\tExecution failed; Returning null\n");
  else
    printf("\t\tExecution done; Returning\n");

  return *script;

}


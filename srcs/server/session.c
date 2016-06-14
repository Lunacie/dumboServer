/*                               -*- Mode: C -*- 
 * 
 * Filename: session.c
 * Description: 
 * Author: Lunacie
 * Created: Wed Sep  2 15:11:47 2015 (+0200)
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
#include		"projects.h"
#include		"parameters.h"
#include		"session.h"
#include		"databases.h"
#include		"execution.h"
#include		"http/http.h"
#include		"list.h"
#include		"handle_error.h"

#include		<uuid/uuid.h>

char			*createNewSession(char *username, t_request *clientRequest,
					  char*cookieName, char *cookieUser, t_socket *client)
{
  char			request[1024];
  char			ret[512];
  char			*key;
  t_list		*list;
  t_projects		project;
  char			*host;
  t_parameters		*cookieJar;
  char			*cookieValue;

  if (!clientRequest)
    return NULL;
  project = clientRequest->_project;
  /* if (access("./dev", F_OK) != FAILURE) */
    host = clientRequest->_host;
  /* else */
  /*   host = "localhost"; */
  cookieJar = clientRequest->_cookieJar;

  /* if (checkForExistingSession(username, clientRequest, cookieName) == TRUE) */
  /*   { */
  /*     printf("\tWorking session existing. Authentification success.\n"); */
  /*     return "\r\n"; */
  /*   } */
  if (!(key = getUniqueId()))
    return NULL;

  printf("\t\tCreating new session for username='%s', project=%d, host='%s'\nKey : %s\n",
	 username, project, host, key);

  setDatabase("dumbo");


  deleteOldSessions(clientRequest, username);

  snprintf(request, 1024, "INSERT INTO sessions (username, sessionID, project, created, expires)" \
	   "VALUES ('%s', '%s', '%d', %s, %s); SELECT ROW_COUNT();", 
	   username,
	   key, 
	   project, 
	   "NOW()",
	   "NOW()");
  setScript(request);

  // Checking request result, row_count() must be 1, the inserted row
  if ((list = execRequest()) && 
      (list && list->content && list->next && list->next->content) &&
      (!strncmp(list->content, "ROW_COUNT()\n", strlen(list->content)) &&
       !strncmp(list->next->content, "1\n", strlen(list->next->content))))
    {
      destroyList(list, TRUE);
      
      addParameter(clientRequest, cookieName, key, COOKIE);
      addParameter(clientRequest, cookieUser, username, COOKIE);

      // Sending cookies
      snprintf(ret, 512, "Set-cookie:%s=%s; domain=%s; path=/\r\n"\
	       "Set-cookie:%s=%s; domain=%s; path=/\r\n\r\n",
	       cookieName, key, host,
	       cookieUser, username, host);
      if (!(cookieValue = strdup(ret)))
	return NULL;
      addCookie(client, cookieName, key);
      addCookie(client, cookieUser, username);

       printf("\t\tGenerated Cookies : \n%s\n", cookieValue);
      return cookieValue;
    }
  else
    printf("\tCould not add session to database\n");
  return NULL;
}


void			deleteOldSessions(t_request *clientRequest, char *username)
{
  char			request[1024];
  char			ret[512];
  char			*key;
  t_list		*list;
  t_projects		project;
 
  if (!clientRequest)
    return;
  project = clientRequest->_project;
  printf("\tChecking for old Sessions : \n");

  setDatabase("dumbo");
  snprintf(request, 1024,
	   "DELETE FROM sessions WHERE username='%s' AND project='%d';",
	   username, clientRequest->_project);
  /* snprintf(request, 1024, */
  /* 	   "DELETE FROM sessions WHERE username='%s' AND project='%d'; SELECT ROW_COUNT();", */
  /* 	   username, clientRequest->_project); */
  setScript(request);
  if ((list = execRequest()))
    {
      if (list->content && list->next && list->next->content)
	{
	  printf("\t\tNumber of rows deleted : %s", list->next->content);
	  return;
	}
    }
  printf("\tNo old Session found, returning\n");
}

t_bool			checkForExistingSession(t_request *clientRequest, char*cookieName, char*cookieUser)
{
  t_projects		project;
  t_parameters		*cookieJar;
  t_list		*list;
  char			*sessionID;
  char			*username;
  char			request[1024];

  project = clientRequest->_project;
  cookieJar = clientRequest->_cookieJar;

  printf("\tChecking for existing Session :\n");
  if (!(sessionID = getCookie(clientRequest, cookieName)))
    return FALSE;
  if (!(username = getCookie(clientRequest, cookieUser)))
    return FALSE;
  
  printf("\t\tCookie found '%s' = '%s'\n", cookieName, sessionID);
  printf("\t\tCookie found '%s' = '%s'\n", cookieUser, username);

  setDatabase("dumbo");
  snprintf(request, 1024,
	   "SELECT sessionID FROM sessions WHERE "\
	   "username='%s' AND project='%d' AND sessionID='%s';",
	   username, clientRequest->_project, sessionID);
  setScript(request);
  printf("\t\tGoing to check into database for session :\n");
  if ((list = execRequest()))
    {
      destroyList(list, TRUE);
      printf("\tSession Found.\n");
      return TRUE;
    }
  printf("\tSession NOT Found.\n");
  return FALSE;
}


t_bool			destroySession(t_request *clientRequest, char*cookieName, char*cookieUser)
{
  t_projects		project;
  t_parameters		*cookieJar;
  t_list		*list;
  char			*sessionID;
  char			*username;
  char			request[1024];

  project = clientRequest->_project;
  cookieJar = clientRequest->_cookieJar;

  printf("\tDeleting existing Session :\n");
  if (!(sessionID = getCookie(clientRequest, cookieName)))
    return FALSE;
  if (!(username = getCookie(clientRequest, cookieUser)))
    return FALSE;
  
  printf("\t\tCookie found '%s' = '%s'\n", cookieName, sessionID);
  printf("\t\tCookie found '%s' = '%s'\n", cookieUser, username);

  setDatabase("dumbo");
  snprintf(request, 1024,
	   "DELETE FROM sessions WHERE "\
	   "username='%s' AND project='%d' AND sessionID='%s'; SELECT ROW_COUNT()",
	   username, clientRequest->_project, sessionID);
  setScript(request);
  printf("\t\tGoing delete existing session :\n");
  if ((list = execRequest()))
    {
      list = list->next;
      if (list->content && strncmp(list->content, "0", 1))
	{
	  printf("\tSession Found and deleted.\n");
	  return TRUE;
	}
      destroyList(list, TRUE);
    }
  printf("\tSession NOT Found. No Deletion\n");
  return FALSE;
}


char			*getUniqueId(void)
{
  char			*key;
  uuid_t		id;
  size_t		i;
  char			*tmp;


  if ((uuid_generate_time_safe(id)) == 0)
    printf("\t\tuuid WAS successfully generated safely\n");
  else
    printf("\t\tuuid was NOT generated safely\n");
  
  if (!(key = malloc(sizeof(uuid_t) * 2 + 1)))
    return NULL;
  tmp = key;
  for (i = 0; i < sizeof(uuid_t); i ++)
    {
      sprintf(tmp,  "%02x", id[i]);
      tmp += 2;
    }
  tmp -= 1;
  *tmp = '\0';
  printf("\t\t\tReturning unique ID : %s\n", key);
  return key;
}


static char	*cookie_sessionID = "DUMBOSESSIONID";
static char	*cookie_sessionUsername = "DUMBOSESSIONusername";

void		setSessionCookiesNames(char *id, char *username)
{
  cookie_sessionID = id;
  cookie_sessionUsername = username;
}


t_bool		authentification(t_list **script, t_socket *client)
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
  

  char			*database;
  char			*sessionUsername;
  char			*username;
  char			*password;

  printf("\t\tAuth() -> Starting Execution\n");

  t_list		*tmpList;
  t_list		*httpHeader;

  httpHeader = NULL;
  tmpList = *script;
  while (tmpList && !httpHeader)
    {
      /* printf("ALL :: %s\n", tmpList->content); */
      if (tmpList->content && !strncmp(tmpList->content, "HTTP/1.1 ", strlen("HTTP/1.1 ")))
	httpHeader = tmpList;
      tmpList = tmpList->prev;
    }

  /* if (!httpHeader) */
  /*   { */
  /*     printf("\t\tCould not find httpHeader; Returning\n"); */
  /*     return FALSE; */
  /*   } */
  database = setDatabase(NULL);
  if (!client)
    {
      if (httpHeader)
	httpHeader->content = HTTP_403;
      return FALSE;
    }
  // Is session already running ?
  if (checkForExistingSession(client->request, cookie_sessionID, cookie_sessionUsername))
    {
      /* httpHeader->content = HTTP_OK; */
      /* list = addToList(NULL, HTTP_OK); */
      /* list = addToList(list, "\r\n"); */
      printf("\t\tAthentification success; Session already running\n");
      return TRUE;
    }
  // Session not running but user is trying to auth
  else if ((username = getParameter(client, "username")) &&
	   (password = getParameter(client, "password")))
    {  
      // Getting Parameter
      printf("\t\tParamaters recieved :\n");
      printf("\t\t\tUsername = '%s' \n", username);
      printf("\t\t\tPassword = '%s' \n", password);

      setDatabase(database);
      snprintf(request, 1024,
	       "SELECT * FROM users WHERE username='%s'AND password='%s';",
	       username, password);
      setScript(request);
      if (!(execRequest()))
	{
	  printf("\t\tAthentification failed; Returning 403\n");
	  /* (*script)->content = NULL; */
	  if (httpHeader)
	    httpHeader->content = HTTP_403;
	  getAuthErrorCode(403);
	  return FALSE;
	}
      else
	{
	  if (!(tmp = createNewSession(username,
				       client->request,
				       cookie_sessionID, cookie_sessionUsername, client)))
	    {
	      /* (*script)->content = NULL; */
	      /* (*script)->content = HTTP_403; */
	      if (httpHeader)
		httpHeader->content = HTTP_403;
	      printf("\t\tCould not create session; Returning 403\n");
	      return FALSE;
	    }
	  else 
	    {
	      /* t_list	*next; */
	      /* char	buffer[1024]; */
	      
	      /* *buffer = '\0'; */
	      /* if (httpHeader) */
	      /* 	httpHeader->content = HTTP_OK; */
	      
	      /* if (!(tmpList = addToList(NULL, tmp))) */
	      /* 	{ */
	      /* 	  perror("malloc"); */
	      /* 	  exit(FAILURE); */
	      /* 	} */

	      /* if (httpHeader) */
	      /* 	{ */
	      /* 	  next = httpHeader->next; */
	      /* 	  printf("next : %s\n", next->content); */
	      /* 	  tmpList->prev = httpHeader; */
	      /* 	  tmpList->next = next; */
	      /* 	  httpHeader->next = tmpList; */
	      /* 	} */
	      /* else */
	      /* 	{ */
	      /* 	  printf("DOESN'T HAVE A HTTP HEADER\n"); */
	      /* 	  insertCookie(tmp); */
	      /* 	} */
	      
	      /* list = addToList(NULL, HTTP_OK); */
	      /* list = addToList(list, tmp); */

	      free(tmp);
	      printf("\t\tAthentification success; New Session created\n");
	      return TRUE;
	    }
	}
    }
  // session not running and missing password and or username...
  else
    {
      	  printf("\t\tAthentification failed : No session running + No auth parameters; Returning 403\n");
	  /* (*script)->content = NULL; */
	  /* (*script)->content = HTTP_403; */
	  if (httpHeader)
	    httpHeader->content = HTTP_403;
	  return FALSE;
    }
  return FALSE;
  /* return commitResult(list, script); */
}



t_bool		isAuthenticated(t_list **script, t_socket *client)
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
  

  char			*database;
  char			*sessionUsername;
  char			*username;
  char			*password;

  printf("\t\tisAuthenticated() -> Starting Execution\n");

  database = setDatabase(NULL);
  if (!client)
      return FALSE;
  // Is session already running ?
  if (checkForExistingSession(client->request, cookie_sessionID, cookie_sessionUsername))
    {
      /* httpHeader->content = HTTP_OK; */
      /* list = addToList(NULL, HTTP_OK); */
      /* list = addToList(list, "\r\n"); */
      printf("\t\tAthentification success; Session already running\n");
      return TRUE;
    }
  return FALSE;
}




t_bool			disconnect(t_list **script, t_socket *client)
{
  printf("\t\tDisconnect() -> Starting Execution\n");

  if (!client)
    return FALSE;
  if (destroySession(client->request, cookie_sessionID, cookie_sessionUsername))
    return TRUE;
  return FALSE;
}


int			getAuthErrorCode(int value)
{
  static int		code = 0;
  int			tmp;

  if (value)
    code = value;
  else if (!value)
    {
      tmp = code;
      code = 0;
      return tmp;
    }
  return code;
}

/*                               -*- Mode: C -*- 
 * 
 * Filename: writeResponse.c
 * Description: 
 * Author: Lunacie
 * Created: Sat Jun 20 12:27:47 2015 (+0200)
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
#include			"projects.h"
#include			"response.h"
#include			"http/http.h"
#include			"http/html/html.h"

#include			"loadProjects.h"
#include			"projects/lapothicaire.h"
#include			"projects/fetchfood.h"
#include			"projects/wunderbar.h"
#include			"projects/homegps.h"
#include			"projects/den.h"


#define	CURRENT_PROJECT		LAPOTHICAIRE

t_hostPointer		hostArray[NB_HOSTS] = 
  {
    {FETCHFOOD, SRCS_PATH_FETCHFOOD, "srcs/projects/fetchfood/bin/fetchFood.so",
     NULL, NULL, NULL, 
     NULL},
    
    {WUNDERBAR, SRCS_PATH_WUNDERBAR, "srcs/projects/wunderbar/bin/wunderbar.so",
     NULL, NULL, NULL, 
     "app-wunderbar.com"},
    
    {LAPOTHICAIRE, SRCS_PATH_LAPOTHICAIRE, "srcs/projects/lapothicaire/bin/lapothicaire.so",
     NULL, NULL,  NULL, 
     "lapothicaire-vapeshop.com:www.lapothicaire-vapeshop.com"},
    
    {HOMEGPS, SRCS_PATH_HOMEGPS, "srcs/projects/homegps/bin/homegps.so",
     NULL, NULL,  NULL,
     "homegps.com:www.homegps.com"},
    
    {DEN, SRCS_PATH_DEN, "srcs/projects/den/bin/den.so",
     NULL, NULL,  NULL, 
     "lunacies-den.com:www.lunacies-den.com"},

  };

extern char		*pwd;

t_list			*getResponse(t_socket *client)
{
  t_list		*response = NULL;
  size_t		i;

  /* char			path[1024]; */

  /* *path = '\0'; */
  /* snprintf(path, 1024, "./.dns", pwd);   */
  i = 0;
  while (i < NB_HOSTS)
    {
      if (access("/home/lunacie/server/.dns", F_OK) != FAILURE &&
	  (hostArray[i].domains && strstr(hostArray[i].domains, client->request->_host)) 
	  ||
	  (access("/home/lunacie/server/.dns", F_OK) == FAILURE &&
	   CURRENT_PROJECT == hostArray[i].id))
	{
	  set_handle(NULL);
	  if (loadProjects(client, hostArray[i].id, hostArray[i].lib) == FAILURE)
	    return NULL;
	  else
	    {
	      if (hostArray[i]._getResponse)
		{
		  printf("Asking project for response : \n");
		  return hostArray[i]._getResponse(client);
		}
	      printf("Error : Could not retrieve _getResponse from lib\n");
	      return NULL;
	    }
	}
      i++;
    }
  return response;
}

int				writeResponse(t_socket *client)
{
  t_list			*response;
  char				*preview;
  size_t			index;
  
  if (!client)
    return FAILURE;
  response = getResponse(client);
  if (!response)
    {
      printf("WriteResponse:: Could not get response\n");
      printf("aborted\n");
      return FAILURE;
    }
  printf("WriteResponse:: Initiating writing\n");
  client->response = (void *)response;



  /* if (response->content) */
  /*   write(client->fd, response->content, strlen(response->content)); */
  /* if (putStrFd((char *)response->content, client->fd, client->request->_size) <= 0) */
  /*   break; */
  /* response = response->next; */


  index = 0;
  while (response)
    {
      if (response->content)
	{
	  if (index == 0) // HEADER
	    {
	      /* char	*headerSuffix; */
	      /* if (!client->request->_isBinary) */
	      /* 	printf("RESPONSE : %s\n", previewString(response->content, 100)); */
	      if (putStrFd(response->content, client->fd,
			   client->request->_size, FALSE) < 0)
		break;
	      /* if (headerSuffix = insertAfterHeader(NULL)) */
	      /* 	{ */
	      /* 	  if (putStrFd(headerSuffix, client->fd, */
	      /* 		       strlen(headerSuffix), FALSE) < 0) */
	      /* 	    break; */
	      /* 	} */
	    }
	  else	// CONTENT
	    {
	      if (client->request->_isBinary)
		printf("| | Is Binary\n");
	      /* else */
	      /* 	printf("RESPONSE : %s\n", previewString(response->content, 100)); */
	      if (putStrFd(response->content, client->fd,
			   client->request->_size, client->request->_isBinary) < 0)
		break;
	    }
	  
	  /* if (!client->request->_isBinary) */
	  /*   printf("-\n"); */
	}
      index++;
      response = response->next;
    }
  write(client->fd, "\r\n", 2);
  printf("WriteResponse:: Writing ended; Returning\n");
  set_handle(NULL);
  return 0;
}


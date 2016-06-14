/*                               -*- Mode: C -*- 
 * 
 * Filename: getResponse.c
 * Description: 
 * Author: Lunacie
 * Created: Wed Jul  1 16:43:07 2015 (+0200)
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
#include			"http/http.h"
#include			"http/html/html.h"

#include			"projects/wunderbar.h"


t_list				*_getAllowedFileList()
{
  t_list			*list;

  list = NULL;

  list = addToList(list, (void *)"/index.html");
  return list;
}

t_list				*_getResponse(t_socket *client)
{
  t_list		*list = NULL;


  client->request->_project = WUNDERBAR;
  if (client->request->_POST)
    {
      client->request->_GET = client->request->_POST;
    }

  if (client->request->_GET)
    {
      // **
      // Url redirections
      // **
      if (!strncmp(client->request->_GET, "/auth", strlen("/auth")))
	{
	  printf("WunderBar :: Requested function : auth();\n");
	  free(client->request->_GET);
	  client->request->_GET = strdup("auth();");
	}
      else if (!strncmp(client->request->_GET, "/exec", strlen("/exec")))
	{
	  printf("WunderBar :: Requested function : exec();\n");
	  free(client->request->_GET);
	  client->request->_GET = strdup("exec();");
	}
      /* list = addToList(NULL, ""); */
      list = execScript(client->request->_GET, client, &list);
    }
  
  /* t_list *copy; */
  /* char *preview; */
  /* copy = list; */
  /* while (copy) */
  /*   { */
  /*     if (copy->content) */
  /* 	{ */
  /* 	  preview = strndup(copy->content, 100); */
  /* 	  /\* printf("\t Response Preview : %s[...]\n", preview); *\/ */
  /* 	  free(preview); */
  /* 	} */
  /*     copy = copy->next; */
  /*   } */
  printf("WunderBar:: Response was prepared; Returning\n");
  return list;
}


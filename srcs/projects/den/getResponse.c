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
#include			"session.h"
#include			"projects.h"
#include			"http/http.h"
#include			"http/html/html.h"
#include			"databases.h"
#include			"parameters.h"

#include			"projects/den.h"


t_list				*_getAllowedFileList()
{
  t_list			*list;

  list = NULL;

  //style
  list = addToList(list, (void *)"/style.css");
  list = addToList(list, (void *)"/fonts/fonts.html");
  list = addToList(list, (void *)"/css/footer.css");
  list = addToList(list, (void *)"/css/header.css");
  list = addToList(list, (void *)"/css/page.css");
  list = addToList(list, (void *)"/css/solarized.css");


  // pages
  //	main pages
  list = addToList(list, (void *)"/index.html");
  //	include pages
  list = addToList(list, (void *)"/header.html");
  list = addToList(list, (void *)"/footer.html");

  // pictures
  //	logos
  /* list = addToList(list, (void *)"/images/logo.png"); */
  //	style
  list = addToList(list, (void *)"/images/background.jpg");


  // scripts 
  list = addToList(list, (void *)"/scripts/highlight.pack.js");
  return list;
}


#define				NB_REDIRECTIONS 2
t_urlRewrite			redirections[NB_REDIRECTIONS] = 
  {
    {NULL, "/index.html", 0},
    {"/\0", "/index.html", 2}
  };


t_list				*_getResponse(t_socket *client)
{
  t_list		*list = NULL;
  size_t		x;

  client->request->_project = DEN;

  if (client->request->_POST)
    client->request->_GET = client->request->_POST;


  // Url rewriting
  for (x = 0; x < NB_REDIRECTIONS; x++)
    {
      if (redirections[x].original == client->request->_GET ||
	  (
	    redirections[x].original &&
	    !strncmp(redirections[x].original,
		     client->request->_GET, redirections[x].len)
	  ))
	{
	  printf("Redirecting '%s' -> '%s'\n",
		 redirections[x].original, redirections[x].redirect);
	  free(client->request->_GET);
	  client->request->_GET = strdup(redirections[x].redirect);
	}
    }

  if (!(list = getPage(client, client->request->_GET, TRUE)))
    {
      printf("page was null\n");
      return NULL;
    }
  printf("page retrieved\n");

  if (client->request->_isBinary == FALSE)
    {
      if (!(list = cutPage(client, list)))
	return NULL;
    }
  return list;
}

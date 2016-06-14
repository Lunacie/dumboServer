/*                               -*- Mode: C -*- 
 * 
 * Filename: destroyCient.c
 * Description: 
 * Author: Lunacie
 * Created: Wed Jul  1 15:40:00 2015 (+0200)
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

int				destroyClient(t_socket *client)
{
  t_list			*list;
  int				id;

  id = client->fdId;
  close(client->fd);


  FD_CLR(client->fd, &(client->server->active_fd_set));
  client->fd = FAILURE;

  if (client->request)
    {
      if (client->request->_GET)
	free(client->request->_GET);
      client->request->_GET = NULL;
      
      if (client->request->_host)
	free(client->request->_host);
      client->request->_host = NULL;
      
      if (client->request->_parameters)
	{
	  if (client->request->_parameters->list)
	    destroyList(client->request->_parameters->list, FALSE);
	  client->request->_parameters->list = NULL;
	  
	  free(client->request->_parameters);
	  client->request->_parameters = NULL;
	}
      
      if (client->request->_cookieJar)
	{
	  if (client->request->_cookieJar->list)
	    destroyList(client->request->_cookieJar->list, FALSE);
	  client->request->_cookieJar->list = NULL;
	  
	  free(client->request->_cookieJar);
	  client->request->_cookieJar = NULL;
	}
      
      if (client->request)
	free(client->request);
      client->request = NULL;
    }

  list = storeClients(NULL);
  while (list)
    {
      if (list->content && ((t_socket *)(list->content))->fdId == id)
	{
	  list->content = NULL;
	  return 0;
	}
      list = list->next;
    }

  free(client);
  client = NULL;

  return 0;
}

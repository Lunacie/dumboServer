/*                               -*- Mode: C -*- 
 * 
 * Filename: getClient.c
 * Description: 
 * Author: Lunacie
 * Created: Sat Jun 20 10:49:02 2015 (+0200)
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

t_socket			*getClient(t_socket *server)
{
  int				fd;
  struct sockaddr_in		peer_addr;
  char				buffer[1024];
  size_t			i;
  int				rc;
  t_socket			*client;
  t_list			*list;

  server->read_fd_set = server->active_fd_set;
  if ((rc = select(FD_SETSIZE, &(server->read_fd_set), NULL, NULL, NULL)) < 0)
    {
      perror("select");
      exit(FAILURE);
    }

  for (i = 0; i < FD_SETSIZE; ++i)
    {
      if (FD_ISSET(i, &(server->read_fd_set)))
	{
	  if (i == server->fd)
	    {
	      if ((fd = accept(server->fd,
			       (struct sockaddr *) &peer_addr,
			       &server->addr_size)) < 0)
		{
		  perror ("accept");
		  return(NULL);
		}
	      FD_SET(fd, &(server->active_fd_set));
	      if (!(client = malloc(sizeof(t_socket))))
		{
		  perror("malloc");
		  exit(FAILURE);
		}
	      client->fd = fd;
	      client->addr = peer_addr;
	      client->addr_size = sizeof(peer_addr);
	      client->fdId = fd;
	      client->server = server;
	      storeClients(client);
	    }
	  else
	    {
	      if ((list = storeClients(NULL)))
		{
		  while (list)
		    {
		      if (list->content && ((t_socket *)(list->content))->fdId == i)
			  return list->content;
		      list = list->next;
		    }
		}
	    }
	}
    }

  return NULL;
}


t_list				*storeClients(t_socket *new)
{
  static t_list			*clients = NULL;
  t_list			*list;

  if (new)
    {
      if (clients)
	{
	  list = clients;
	  while (list)
	    {
	      if (list->content == NULL)
		{
		  printf("\nReplacing client in list,  fdId : %d\n", new->fdId);
		  list->content = new;
		  return clients;
		}
	      list = list->next;
	    }
	}
      if (!(clients = addToList(clients, new)))
	exit(FAILURE);
      printf("\nAdded client list fdId : %d\n", ((t_socket *)(clients->content))->fdId);
    }
  return clients;
}

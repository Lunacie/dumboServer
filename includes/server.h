/*                               -*- Mode: C -*- 
 * 
 * Filename: server.h
 * Description: 
 * Author: Lunacie
 * Created: Sat May  9 20:20:39 2015 (+0200)
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


#ifndef				SERVER_H
#define				SERVER_H

#include			<sys/types.h>
#include			<sys/socket.h>
#include			<netdb.h>
#include			<netinet/in.h>

#include			"program.h"
#include			"projects.h"
#include			"list.h"

#define				HOST				"127.0.0.1"
#define				MAX_FILE_SIZE_DIGITS		16

#define				QUEUE_SIZE			1

typedef struct			s_parameter
{
  char				*variable;
  char				*_value;
}				t_parameter;



typedef struct			s_parameters
{
  size_t			_size;
  char				*_original;
  t_list			*list;
}				t_parameters;


typedef struct			s_request
{
  char				*original;
  char				*_GET;
  char				*_POST;
  char				*_host;
  t_parameters			*_cookieJar;
  t_parameters			*_parameters;
  

  size_t			_size;
  t_projects			_project;
  t_bool			_isBinary;
}				t_request;

typedef struct			s_socket
{
  int				fd;
  socklen_t			addr_size;
  struct sockaddr_in		addr;
  t_list			*response;
  t_list			*header;
  t_request			*request;

  fd_set			active_fd_set;
  fd_set			read_fd_set;

  int				fdId;
  struct s_socket		*server;
}				t_socket;



int				start_server(t_socket *server);
t_socket			*getClient(t_socket *server);
t_list				*storeClients(t_socket *client);

int				destroyClient(t_socket *client);

#endif				/* SERVER_H */

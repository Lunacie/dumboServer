/*                               -*- Mode: C -*- 
 * 
 * Filename: start_server.c
 * Description: 
 * Author: Lunacie
 * Created: Tue May 19 12:26:14 2015 (+0200)
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

#include			<signal.h>

#include			"server.h"
#include			"program.h"
#include			"handle_error.h"

#define				DEV_PORT		8081

extern char			*pwd;

int				start_server(t_socket *server)
{
  int				fd_socket; 
  struct sockaddr_in		addr;
  socklen_t			addr_size;
  int				port;
  fd_set			active_fd_set;
  fd_set			read_fd_set;

  /* char				path[1024]; */

  /* *path = '\0'; */
  /* snprintf(path, 1024, "%s/.dev", pwd); */
  if (access("./.dev", F_OK) != FAILURE)
    port = DEV_PORT;
  else
    port = 80;

  if ((fd_socket = socket(PF_INET, SOCK_STREAM, 0)) == FAILURE)
    return handle_error("socket", FAILURE);
  addr_size = sizeof(addr); 
  memset((void *)&addr, addr_size, 0);
  addr.sin_family = AF_INET;
  addr.sin_port = htons((unsigned short)port);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  /* if ((addr.sin_addr.s_addr = inet_addr(HOST)) == FAILURE) */
  /*   return handle_error("inet_addr", FAILURE); */
  if (bind(fd_socket, (const struct sockaddr *)&addr, addr_size) == FAILURE)
    return handle_error("bind", FAILURE); 
  if (listen(fd_socket, QUEUE_SIZE) == FAILURE)
    return handle_error("listen", FAILURE);

  FD_ZERO (&active_fd_set);
  FD_SET (fd_socket, &active_fd_set);

  server->addr_size = addr_size;
  server->addr = addr;
  server->fd = fd_socket;
  server->active_fd_set = active_fd_set;
  server->read_fd_set = read_fd_set;

  if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
    {
      perror("Signal error : ");
      exit(1);
    }

  printf("Server was successfully started\nRunning on port : %d\n", port);
  return 0;
}

/*                               -*- Mode: C -*- 
 * 
 * Filename: main.c
 * Description: 
 * Author: Lunacie
 * Created: Thu Jan 16 02:45:28 2014 (+0100)
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

#include		<unistd.h>
#include		<stdlib.h>

#include		"program.h"
#include		"server.h"

char			*pwd;

int			main(int argc, char **argv, char **env)
{
  t_socket		server;
  t_socket		*client;

  while (*env)
    {
      if (!strncmp(*env, "PWD=", 4))
	{
	  *env += 4;
	  pwd = *env;
	}
      env++;
    }

  if (start_server(&server) == FAILURE)
    return FAILURE;
  printf("$PWD : '%s'\n", pwd);
  while (42)
    {
      printf("\n\033[33m1.  Waiting for client : --\033[0m\n");
      if (client = getClient(&server))
	{
	  printf("\n\033[33m2.  Reading Request -- \033[0m\n");
	  if (getRequest(client) != FAILURE)
	    {
	      printf("\n\033[33m3.  Writing Reponses -- \033[0m\n");
	      writeResponse(client);
	      printf("\n\033[33m4.  Closing Client --\033[0m\n");
	      destroyClient(client);
	    }
	  else
	    {
	      printf("Destroying client\n");
	      destroyClient(client);
	      printf("\n\033[33m3.  Failed Reading Request, Closing Client --\033[0m\n");
	    }
	}
    }
  close(server.fd);
  return 0;
}

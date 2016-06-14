/*                               -*- Mode: C -*- 
 * 
 * Filename: response.h
 * Description: 
 * Author: Lunacie
 * Created: Sat Jun 20 12:33:48 2015 (+0200)
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


#ifndef			RESPONSE_H
#define			RESPONSE_H

#include		<fcntl.h>
#include		<string.h>

#include		"server.h"
#include		"list.h"

#define			SRCS_PATH		"srcs/projects"

typedef struct		s_message
{
  char			*str;
}			t_message;

typedef struct		s_hostPointer
{
  t_projects		id;
  char			*directory;
  char			*lib;
  t_list		*(*_getResponse)(t_socket *client);
  t_list		*(*_getFileList)(void);
  t_list		*(*_exec)(t_list **script, t_socket *client);
  char			*domains;
}			t_hostPointer;

int			writeResponse(t_socket *client);
t_list			*getResponse(t_socket *client);
t_list			*getPage(t_socket *client, char *name, t_bool getHeader);
t_list			*cutPage(t_socket *client, t_list *list);
char			*removeComments(char *str);

static char		*getFilePath(t_socket *client, char *name);
static t_bool		accessToFileIsAllowed(char *path, t_request *request);



#endif			/* RESPONSE_H */

/*                               -*- Mode: C -*- 
 * 
 * Filename: request.h
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


#ifndef			REQUEST_H
#define			REQUEST_H

#include		"server.h"
#include		"list.h"

#define			REQUEST_BUFFER_SIZE		4096

int			getRequest(t_socket *client);
char			*readRequest(t_socket *client);
char			*readParameters(t_socket *client, size_t size);
int			parseRequest(t_socket *client);
t_list			*parseParameters(char *buffer, t_parameters *parameters,
					 char *linePattern, char *elementPattern);
char			*urlDecode(char *str);


#endif			/* REQUEST_H */

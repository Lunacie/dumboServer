/*                               -*- Mode: C -*- 
 * 
 * Filename: session.h
 * Description: 
 * Author: Lunacie
 * Created: Wed Sep  2 15:10:38 2015 (+0200)
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


#ifndef			SESSION_H
#define			SESSION_H

#include		"server.h"
#include		"projects.h"

char			*createNewSession(char *username, t_request *request, char*cookieName,
					  char *cookieUser, t_socket *client);
t_bool			checkForExistingSession(t_request *request, char*cookieName, char*cookieUser);
char			*getUniqueId(void);
void			deleteOldSessions(t_request *clientRequest, char *username);
t_bool			authentification(t_list **script, t_socket *client);
void			setSessionCookiesNames(char *id, char *username);
int			getAuthErrorCode(int value);

t_bool			destroySession(t_request *clientRequest, char*cookieName, char*cookieUser);
t_bool			disconnect(t_list **script, t_socket *client);
t_bool			isAuthenticated(t_list **script, t_socket *client);

#endif			/* SESSION_H */

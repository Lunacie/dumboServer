/*                               -*- Mode: C -*- 
 * 
 * Filename: functions.h
 * Description: 
 * Author: Lunacie
 * Created: Mon Aug 17 17:10:33 2015 (+0200)
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


#ifndef			FUNCTIONS_H
#define			FUNCTIONS_H

#include		"server.h"
#include		"execution.h"
#include		"list.h"

#define			NB_FUNCTIONS		2

t_list			*includeFile(t_socket *client, char *name);
t_list			*include(t_list **script, t_socket *client);
t_list			*printVariable(t_list **script, t_socket *client);

#endif			/* FUNCTIONS_H */

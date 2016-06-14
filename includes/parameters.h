/*                               -*- Mode: C -*- 
 * 
 * Filename: parameters.h
 * Description: 
 * Author: Lunacie
 * Created: Tue Aug 11 14:51:45 2015 (+0200)
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


#ifndef			PARAMETERS_H
#define			PARAMETERS_H

#include		"server.h"
#include		"list.h"

typedef	enum
  {
    PARAMETER = 0,
    COOKIE
  }			t_parameterType;

char			*getParameter(t_socket *client, char *name);
char			*getCookie(t_request *request, char *name);

int			addParameter(t_request *request, char *variale, char *value, t_parameterType type);
char			*setParameterValue(t_list *parameters, char *name, char *value);
t_list			*generateEmptyParameterListFromNames(char *names, char seperator);
t_list			*fillParameterList(t_socket *client, t_list *names);
char			*getParameterValueFromList(t_list *parameters, char *name);
t_bool			parameterIsDefined(t_list *list, char granularity);

#endif			/* PARAMETERS_H */

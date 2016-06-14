/*                               -*- Mode: C -*- 
 * 
 * Filename: parameters.c
 * Description: 
 * Author: Lunacie
 * Created: Tue Aug 11 14:52:56 2015 (+0200)
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

#include		"program.h"
#include		"server.h"
#include		"list.h"
#include		"parameters.h"

char			*getParameter(t_socket *client, char *name)
{
  t_list		*parameters;
  t_parameter		*parameter;

  if (!client || !client->request || !name)
    return NULL;
  if (client->request->_parameters &&
      ((parameters = client->request->_parameters->list)))
    {
      while (parameters /* && parameters->next */)
	{
	  if ((parameter = parameters->content) &&
	      parameter->_value &&
	      (parameter->variable && stringsAreTheSame(name, parameter->variable)))
	    {
	      printf("\tParameter found: '%s'='%s'\n", parameter->variable, parameter->_value);
	      return (parameter->_value);
	    }
	  /* printf("\tnot returning '%s'\n", parameter->variable); */
	  parameters = parameters->next;
	}
    }
  printf("\tParameter not found '%s'; returning null\n", name);
  return NULL;
}


t_bool			parameterIsDefined(t_list *list, char granularity)
{
  t_parameter		*current;

  while (list)
    {
      if (current = list->content)
	{
	  if (current->_value && granularity == ANY)
	    return TRUE;
	  else if (!current->_value && granularity == ALL)
	    return FALSE;
	}
      list = list->next;
    }
  if (granularity == ALL)
    return TRUE;
  return FALSE;
}

char			*getParameterValueFromList(t_list *parameters,
						   char *name)
{
  if (!parameters)
    return NULL;
  while (parameters)
    {
      if (parameters->content &&
	  ((t_parameter *)parameters->content)->variable &&
	  stringsAreTheSame(name,
			    ((t_parameter *)parameters->content)->variable))
	return ((t_parameter *)parameters->content)->_value;
      parameters = parameters->next;
    }
  return NULL;
}

t_list			*fillParameterList(t_socket *client, t_list *names)
{
  t_list		*tmp;
  t_parameter		*current;
  t_list		*start;

  start = names;
  printf("Starting filling of parameter list\n");
  if (!client || !client->request || !client->request->_parameters || !client->request->_parameters->list)
    {
      printf("\tReturning; Missing parameters list\n");
      return names;
    }

  while (names)
    { 
      tmp = client->request->_parameters->list;
      if (names && names->content && ((t_parameter *)(names->content))->variable)
	{
	  /* printf("\t%s\n", ((t_parameter *)(names->content))->variable); */
	  while (tmp)
	    {
	      if (tmp->content && ((t_parameter *)(tmp->content))->variable)
		{
		  /* printf("going to compare : %s %s\n", ((t_parameter *)(tmp->content))->variable, ((t_parameter *)(names->content))->variable); */
		  if (stringsAreTheSame(((t_parameter *)(tmp->content))->variable, ((t_parameter *)(names->content))->variable))
		    {
		      ((t_parameter *)(names->content))->_value = ((t_parameter *)(tmp->content))->_value;
		      /* printf("\t\tFound '%s' = '%s'\n", ((t_parameter *)(tmp->content))->variable, ((t_parameter *)(tmp->content))->_value); */
		      tmp = NULL;
		    }
		}
	      if (tmp)
		tmp = tmp->next;
	    }
	}
      names = names->next;
    }
  return start;
}


t_list			*generateEmptyParameterListFromNames(char *names, char separator)
{
  t_parameter		*currentParameter;
  t_list		*list;
  char			*start;
  size_t		len;

  list = NULL;
  while (*names)
    {
      len = 0;
      start = names;
      while (*names && *names != separator)
	{
	  names += 1;
	  len += 1;
	}
      if (*names == separator || !*names)
	{
	  if (!(currentParameter = malloc(sizeof(t_parameter))))
	    {
	      perror("malloc");
	      exit(FAILURE);
	    }
	  if (strlen(start))
	    {
	      currentParameter->variable = strndup(start, len);
	      currentParameter->_value = NULL;
	      list = addToList(list, currentParameter);
	    }
	  if (*names == separator)
	    names += 1;
	}
    }
  return list;
}

char			*setParameterValue(t_list *parameters, char *valuePtr, char *value)
{
  while (parameters)
    {
      if (parameters->content &&
	  ((t_parameter *)parameters->content)->variable &&
	  ((t_parameter *)parameters->content)->_value == valuePtr)
	{
	  ((t_parameter *)parameters->content)->_value = value;
	  return((t_parameter *)parameters->content)->_value;
	}
      parameters = parameters->next;
    }
  return NULL;
}

char			*getCookie(t_request *request, char *name)
{
  t_list		*cookies;

  if (!request)
    return NULL;
  if (request->_cookieJar &&
      (request->_cookieJar->list))
    {
      cookies = request->_cookieJar->list;
      while (cookies)
	{
	  if (cookies->content &&
	      ((t_parameter *)cookies->content)->variable &&
	      !strncmp(name, ((t_parameter *)cookies->content)->variable,
		       strlen(((t_parameter *)cookies->content)->variable)) && 
	      ((t_parameter *)cookies->content)->_value)
	    return ((t_parameter *)cookies->content)->_value;
	  cookies = cookies->next;
	}
    }
  return NULL;
}



int			addParameter(t_request *request, char *variable, char *value, t_parameterType type)
{
  t_parameter		*new;

  if (!request)
    return FAILURE;
  if (!(new = malloc(sizeof(t_parameter))))
    return FAILURE;
  new->variable = variable;
  new->_value = value;

  if (type == PARAMETER)
    {
      if (!request->_parameters)
	{
	  if (!(request->_parameters = malloc(sizeof(t_parameters *))))
	    return FAILURE;
	  request->_parameters->list = NULL;
	}
      if (!request->_parameters->list)
	{
	  if ((request->_parameters->list = addToList(NULL, new)))
	    return 0;
	}
    }
  else if (type == COOKIE)
    {
      if (!request->_cookieJar)
	{
	  if (!(request->_cookieJar = malloc(sizeof(t_parameters *))))
	    return FAILURE;
	  request->_cookieJar->list = NULL;
	}
      if (!request->_cookieJar->list)
	{
	  if ((request->_cookieJar->list = addToList(NULL, new)))
	    return 0;
	}
    }
  return FAILURE;
}


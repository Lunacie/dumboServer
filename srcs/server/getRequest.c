/*                               -*- Mode: C -*- 
 * 
 * Filename: getRequest.c
 * Description: 
 * Author: Lunacie
 * Created: Tue Jun 23 18:16:47 2015 (+0200)
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
#include			"hours.h"
#include			"program.h"
#include			"handle_error.h"
#include			"list.h"
#include			"request.h"


int			getRequest(t_socket *client)
{
  char			*request;

  displayTime();
  if (!(request = readRequest(client)))
    {
      printf("Get request returning failure\n");
      client->request = NULL;
      return FAILURE;
    }
  printf("%s\n", request);
  client->request->original = request;
 return parseRequest(client);
}

int			parseRequest(t_socket *client)
{
  char			*buffer;
  char			*tmp;
  char			*content;
  char			*end;
  char			*copy;
  char			*start;
  size_t		offset;

  buffer = client->request->original;
  start = buffer;
  copy = strdup(buffer);

  printf("\n\nOriginal Request : %s\n", buffer);

  content = NULL;
  if ((client->request->_GET = getPattern("GET ", " ", &buffer)))
    content = client->request->_GET;
  if ((client->request->_POST = getPattern("POST ", " ", &buffer)))
    content = client->request->_POST;
  client->request->_host = getPattern("Host: ", "\r", &buffer);
  client->request->_size = 0;
  client->request->_isBinary = FALSE;
  client->request->_parameters = NULL;
  client->request->_cookieJar = NULL;


  
  // ** TESTING
  printf("#\nGET : \033[36m%s\033[0m\n", client->request->_GET);
  printf("POST : %s\n", client->request->_POST);
  printf("Host : %s\n",client->request->_host);
  // **

  if ((tmp = getPattern("Cookie: ", "\r", &start)))
    {
      if (!(client->request->_cookieJar = malloc(sizeof(t_parameters))))
	return (long int)handle_error("malloc", FAILURE);
      client->request->_cookieJar->list = NULL;
      client->request->_cookieJar->_original = tmp;
      if (!(parseParameters(client->request->_cookieJar->_original,
			    client->request->_cookieJar,
			    "%m[^';']", "%m[^'=']=%ms")))
	return FAILURE;
    }

  if (content && 
      (tmp = getMatchingPattern(content, "?<?>", FALSE)) && 
      (client->request->_GET || client->request->_POST))
    {
      printf("Request with URL parameter(s) : %s\n", tmp);
      if (!(client->request->_parameters = malloc(sizeof(t_parameters))))
	return (long int)handle_error("malloc", FAILURE);
      client->request->_parameters->list = NULL;
      client->request->_parameters->_original = tmp;
      if (!(parseParameters(client->request->_parameters->_original,
			    client->request->_parameters,
			    "%m[^'&']", "%m[^'=']=%ms")))
	return FAILURE;
      if (end = strstr(content, "?"))
	*end = '\0';
    }

  if (client->request && client->request->_POST)
    {
      char *tmpStr;
      if (tmp = strstr(copy, "Content-Length:"))
      {
	tmp += strlen( "Content-Length") + 1;
	for (offset = 0; tmp[offset] && tmp[offset] != '\r'; offset++);
	tmp = strndup(tmp, offset);
	printf("len : '%s'\n", tmp);
	if (!client->request->_parameters)
	  {
	    if (!(client->request->_parameters = malloc(sizeof(t_parameters))))
	      return (long int)handle_error("malloc", FAILURE);
	    client->request->_parameters->list = NULL;
	  }
	client->request->_parameters->_size = atoi(tmp);
	if (!(tmp = readParameters(client, client->request->_parameters->_size)))
	  return FAILURE;
	client->request->_parameters->_original = tmp;
	if (!(parseParameters(client->request->_parameters->_original,
			      client->request->_parameters,
			      "%m[^'&']", "%m[^'=']=%ms")))
	  return FAILURE;
      }
      else 
	{
	  printf("Error : No 'Content-Lenght : ' found\n");
	  return FAILURE;
	} 
      free(copy);
    }

  if ((!client->request->_GET && !client->request->_POST) || !client->request->_host)
    return FAILURE;
 

  /* //PARAM DEBUG FULL LIST UNCOMMENT HERE */
  /* // ** */
  /* if (client && client->request && client->request->_parameters) */
  /*   { */
  /*     t_list *test = client->request->_parameters->list; */
  /*     while (test) */
  /* 	{ */
  /* 	  t_parameter *current = test->content; */
  /* 	  if (test->content) */
  /* 	    printf("CURRENT VARIABLES FULL ARRAY : '%s'\n", current->variable); */
  /* 	  test = test->next; */
  /* 	} */
  /*   } */
  /* // ** */
  /* //END DEBUG */

  return 0;
}

char			*readParameters(t_socket *client, size_t size)
{
  char			*buffer;
  size_t		rc;

  /* printf("Lenght : %d\n", size); */
  if (!(buffer = malloc(size + 1)))
    return (char *)handle_error("malloc", (long int)NULL);
  if ((rc = read(client->fd, buffer, size)) <= 0)
    return (char *)handle_error("read", (long int)NULL);
  buffer[rc] = '\0';
  /* printf("BUFFER : '%s'\n", buffer); */
  return buffer;
}


t_list			*parseParameters(char *buffer, t_parameters *parameters,
					 char *linePattern, char *elementPattern)
{
  char			*end;
  char			*start;
  t_bool		run = TRUE;
  t_parameter		*tmp;
  t_parameter		*parameter;
  char			*variable;
  char			*value;
  t_list		*list;
  t_list		*first;
  size_t		len;
  size_t		x;

  parameter = NULL;
  tmp = NULL;
  list = NULL;
  end = NULL;
  len = strlen(buffer);

  if (parameters->list)
    list = parameters->list;
  x = 0;
  while (*buffer &&
	 (sscanf(buffer, linePattern, &start) ||
	  sscanf(buffer, "%ms", &start)))
    {
      /* printf("Buffer : '%s'\n", start); */
      variable = NULL;
      value = NULL;
      if (sscanf(start, elementPattern, &variable, &value))
      	{
	  if (variable /* && value */)
	    {
	      if (!(tmp = malloc(sizeof(t_parameter))))
		return (t_list *)handle_error("malloc", (long int)NULL);
	      parameter = tmp;
	      if (value)
		{
		  /* printf("Value pre decode : '%s'\n", value); */
		  value = urlDecode(value);
		  /* printf("Value post decode : '%s'\n", value); */
		}
	      parameter->variable = variable;
	      parameter->_value = value;
	      /* printf("Result : '%s' = '%s'\n", variable, value); */
	      list = addToList(list, parameter);
	    }
	  else
	      break;
      	}

      buffer += strlen(start);
      x += strlen(start);
      if (*buffer)
	{
	  buffer += 1;
	  x += 1;
	}
    }
  parameters->list = list;
  first = list;

  while (list)
    {
      if (list->content &&
	  ((t_parameter *)(list->content))->variable/*  &&  */
	  /* ((t_parameter *)(list->content))->_value */)
	{
	  char *str;

	  str = ((t_parameter *)(list->content))->variable;
	  while (*str && *str == ' ')
	    str++;
	  ((t_parameter *)(list->content))->variable = str;
	  printf("\t'%s' = '%s'\n", ((t_parameter *)(list->content))->variable,
		   ((t_parameter *)(list->content))->_value);
	}
      list = list->next;
    }
  return first;
}




char			*readRequest(t_socket *client)
{
  char			*buffer;
  FILE			*input;
  size_t		x;
  t_bool		run;
  int			rc;

  if (!(buffer = malloc(REQUEST_BUFFER_SIZE + 1)))
    return (char *)handle_error("malloc", (long int)NULL);
  x = 0;
  run = TRUE;
  while (run && x < REQUEST_BUFFER_SIZE)
    {
      if ((rc = read(client->fd, &buffer[x], 1)) < 0)
	{
	  printf("Read retuned -1\n");
	  return (char *)handle_error("read", (long int)NULL);
	}
      if (rc == 0)
	{
	  printf("Could not read next character; Returning.\n");
	  buffer[x] = '\0';
	  
	  /* if (client && !client->request) */
	  /*   { */
	      if (!(client->request = malloc(sizeof(t_request))))
		return (char *)handle_error("malloc", (long int)NULL);
	    /* } */
	  return buffer;
	}
      else
	{	  
	  if (x > 2)
	    {
	      if (buffer[x - 2] == '\n' &&
		  buffer[x - 1] == '\r' &&
		  buffer[x] == '\n')
		run = FALSE;
	    }
	  x++;
	}
    }
  buffer[x - 4] = '\0';
  /* if (client && !client->request) */
  /*   { */
      if (!(client->request = malloc(sizeof(t_request))))
	return (char *)handle_error("malloc", (long int)NULL);
    /* } */
	  /* printf("READING REQUEST ================================== \n"); */
	  /* printf("'%s'\n", buffer); */
	  /* printf("================================== READING REQUEST\n"); */
	  write(1, &buffer[x], 1);
  return buffer;
}


char			*urlDecode(char *str)
{
  char			*start;
  char			*tmp;
  char			*old;
  char			hexValue;
  char			*hex;

  old = str;
  if (!(tmp = malloc(strlen(str) + 1)))
    return NULL;
  start = tmp;
  while (*str)
    {
      if (*str == '%')
	{
	  str += 1;
	  if (*str && strlen(str) >= 2)
	    {
	      if ((hex = strndup(str, 2)))
		{
		  hex[2] = '\0';
		  if (hex[1] >= 'A' && hex[1] <= 'Z')
		    sscanf(hex, "%X", &hexValue);
		  else
		    sscanf(hex, "%x", &hexValue);
		  if (hex)
		    {
		      *tmp = hexValue;
		      /* free(hex); */
		      str += 1;
		    }
		  else
		      return start;
		}
	    }
	}
      else if (*str == '+')
      	*tmp = ' ';
      else
	*tmp = *str;
      tmp++;
      if (str)
	str++;
    }
  *tmp = '\0';
  return start;
}

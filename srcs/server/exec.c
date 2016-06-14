#include			"server.h"
#include			"program.h"
#include			"handle_error.h"
#include			"list.h"
#include			"strings.h"
#include			"projects.h"
#include			"response.h"
#include			"http/http.h"
#include			"http/html/html.h"
#include			"execution.h"
#include			"functions.h"

t_hostPointer		hostArray[NB_HOSTS];

t_function		functions[NB_FUNCTIONS] = 
  {
    {"include", &include},
    {"variable", &printVariable}
  };

t_list			*execScript(char *script, t_socket *client, t_list **full)
{
  t_list		*list;
  t_list		**original;
  size_t		i;
  t_bool		func = FALSE;
  t_list		*tmp;

  original = full;
  list = NULL;

  if (!(list = cutScript(script)))
    {
      printf("\tReturning null, Invalid Script : '%s'\n", script);
      return addToList(list, (void*)HTTP_444);
    }
  if (*full == NULL)
    {
      *full = list;
      *original = *full;
    }
  else
    addListToList(*full, list);

  while (list)
    {
      func = FALSE;
      // Removing comments
      if (list->content)
	{
	  list->content = removeComments(list->content);
	  printf("\n\t Starting the Execution of Script : %s\n", list->content);
	}

      // Checking for server functions
      i = 0;
      while (i < NB_FUNCTIONS)
	{
	  if (!strncmp(list->content, functions[i].name, strlen(functions[i].name)))
	    {
	      if (tmp = functions[i]._ptr(&list, client))
		list = tmp;
	      func = TRUE;
	      break;
	    }
	  i++;
	}

      // Execution
      if (func == FALSE && list && list->content && *((char *)(list->content)))
	list = hostArray[client->request->_project]._exec(&list, client);
      /* if (list->content) */
      /* 	printf("\tResult preview : '%s\n'", previewString(list->content, 100)); */
      if (list)
	list = list->next;
    }


  return *original;
}


t_list			*cutScript(char *script)
{
  t_list		*list;
  char			*tmp;
  char			*last;
  t_bool		first = TRUE;

  if (!(tmp = script))
    return NULL;
  list = NULL;
  last = tmp;
  while (*tmp)
    {
      if (*tmp == END_COMMAND_CHAR)
	{
	  /* printf("New instruction\n"); */
	  *tmp = '\0';
	  if (first == TRUE)
	    list = addToList(list, (void*)last);
	  else
	    addToList(list, (void*)last);
	  last = tmp;
	  last++;
	}
      tmp++;
    }
  return list;
}


char			*getFunctionName(char *str)
{
  char			*end;
  char			*new;

  new = strdup(str);
  while (*new && (*new == '\t' || *new == ' ' || 
		  *new == '\r' || *new == '\n'))
    new++;
  if (!(end = strstr(new, "(")))
    return NULL;
  *end = '\0';
  return new;
}


t_list			*commitResult(t_list *list, t_list **script)
{
  char			*result;
  
  result = NULL;
  if (list)
    {      
      result = listToString(list);
      if (result)
	{
	  destroyList(list, FALSE);
	  (*script)->content = result;
	}
    }    
                                                                                                 
  printf("\t\tResult Preview : %s\n", previewString(result, 100));
  printf("\t\tExecution done; Returning\n");
  return *script;
}

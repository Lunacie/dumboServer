/*                               -*- Mode: C -*- 
 * 
 * Filename: cutPage.c
 * Description: 
 * Author: Lunacie
 * Created: Fri Jul 10 18:36:25 2015 (+0200)
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
#include			"program.h"
#include			"handle_error.h"
#include			"list.h"
#include			"strings.h"
#include			"projects.h"
#include			"response.h"
#include			"http/http.h"
#include			"http/html/html.h"
#include			"execution.h"

t_hostPointer		hostArray[NB_HOSTS];

t_list			*cutPage(t_socket *client, t_list *list)
{
  char			*current;
  char			*start = NULL;
  char			*end = NULL;
  t_bool		first = TRUE;
  t_list		*original;
  t_list		*new;

  original = list;
  list = getLastElement(list);
  if (list)
    current = (char *)list->content;
  if (!list || !current)
    return list;
  while (strlen(current) > strlen(START_EXEC) &&
	 (start = strstr(current, START_EXEC)))
    {
      start[0] = '\0';
      start += 3; //strlen START_EXEC
      if (!(end = strstr(start, END_EXEC)))
	break; // syntax error in src file
      end[0] = '\0';
      end += 2; // strlen END_EXEC
      if (first == TRUE)
	{
	  list->content = (void *)current;
	  first = FALSE;
	}
      else
	  addToList(list, (void *)current);

      
      list = getLastElement(list);
      if ((list = execScript(start, client, &list)))
      	{
      	  /* printf("post exec : [%s]\n", new->content); */
      	  /* list = addListToList(list, new); */
      	}
      /* else */
      /* 	printf("No content\n"); */


      
	  /* list = addToList(list, INVALID_SCRIPT); */
	
      /* printf("CURENT LINE : [[[%s]]]\n", list->content); */
      current = end;
    }
  if (first == FALSE)
    addToList(list, current);
  

  // for testing purpose

  /* while (original) */
  /*   { */
  /*     printf("CUT : --------[%s]----------\n", original->content); */
  /*     original = original->next; */
  /*   } */


  /* ** end test */

  return original;
}

char			*removeComments(char *str)
{
  char			*original;
  int			offset;

  original = str;
  offset = 0;
  while (*str)
    {
      if (!strncmp(str, "/*", 2))
	{
	  offset += 1;
	  str[0] = ' ';
	  str[1] = ' ';
	  str += 2;
	}
      else if (!strncmp(str, "*/", 2))
	{
	  offset -= 1;
	  str += 2;
	}

      if (offset == 0);
      else if (offset > 0 && *str)
	*str = ' ';
      
      if (*str)
	str++;
    }
  str= original;
  while (*str && *str == ' ')
    str++;
  return str;
}

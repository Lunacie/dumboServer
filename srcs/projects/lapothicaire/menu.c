/*                               -*- Mode: C -*- 
 * 
 * Filename: menu.c
 * Description: 
 * Author: Lunacie
 * Created: Wed Dec  9 15:46:54 2015 (+0100)
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
#include			"databases.h"
#include			"html.h"

#include			"projects/lapothicaire.h"

static t_list		*displayMenu(t_list **script, t_socket *client)
{
  t_list		*list;
  char			request[1024];
  t_list		*ret;

  char			*tmp;
  size_t		len;
  t_list		*subMenu;

  list = NULL;
  setDatabase("lapothicaire");

  list = getSubMenu(1, 0, NULL);
  return commitResult(list, script);
}


t_list			*getSubMenu(unsigned int id, size_t level, t_list *list)
{
  char			request[1024];
  t_list		*ret;
  t_list		*subMenu;
  char			*tmp;
  size_t		count;


  snprintf(request, 1024,
	   "SELECT CategorieId, CategorieNom FROM categories WHERE CategorieIdParent='%d';", id);
  setScript(request);
  if ((ret = execRequest()))
    { 
      if (level == 0)
	list = addToList(list, "<ul class=\"main\">\n");
      else
	{
	  if (!(tmp = malloc(50)))
	    {
	      perror("malloc : ");
	      exit(FAILURE);
	    }
	  snprintf(tmp, 50, "<ul class=\"sub level_%d\">\n", level);
	  list = addToList(list, tmp);
	}
      ret = ret->next;

      // FOr each Category Element
      count = 0;
      while (ret && ret->content)
      	{
	  char		*str;
	  size_t	id;
	  size_t       	len;

	  /* printf("PARSE : '%s'\n", ret->content); */
	  // parse id name\n
	  /* if (sscanf(ret->content, "%d %ms[^'\r]\r", &id, &str) == 2) */
	  if ((sscanf(ret->content, "%d ", &id) == 1))
	    {
	      str = ret->content;
	      while (*str && *str != '\t' && *str != ' ')
		str++;
	      if (*str)
		if (!(str = strdup(str)))
		  return list;
		  
		
	      free(ret->content);
	      ret->content = NULL;

	      /* size_t x; */
	      /* for (x = 0; x < level; x++) */
	      /* 	printf("\t"); */
	      /* printf("-%s\n", tmp); */

	      if (!(tmp = malloc(1024)))
		{
		  perror("malloc : ");
		  exit(FAILURE);
		}
	      snprintf(tmp, 1024, 
		       "<li class=\"id_%d level_%d\"><a class=\"level_%d\" "\
		       "href=\"./categories?id=%d\">%s</a>\n",
		       id, level, level, id, str);
	      free(str);
	      
	      list = addToList(list, tmp);

	      // get sub menu recursively
	      list = getSubMenu(id, level + 1, list);
	      list = addToList(list, "</li>");
	    }
      	  ret = ret->next;
	  count++;
      	}
      list = addToList(list, "</ul>\n");
    }
  return list;
}

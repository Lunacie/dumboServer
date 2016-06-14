/*                               -*- Mode: C -*- 
 * 
 * Filename: parsingCart.c
 * Description: 
 * Author: Lunacie
 * Created: Thu Jun  2 00:49:39 2016 (+0200)
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


static t_list		*parseItems(t_list *items)
{
  char			*str;
  int			step;
  char			*start;
  t_product		*current;
  t_list		*ret;

  ret = items;
  while (items && items->content)
    {
      if (!(current = malloc(sizeof(t_product))))
	{
	  perror("malloc");
	  exit(FAILURE);
	}
      str = items->content;
      start = items->content;
      step = 0;
      // Name
      while(*str && *str != ':')
	str++;
      if (*str == ':' && (step = 1))
	{
	  *str = '\0';
	  current->id = strdup(start);
	  str++;
	  start = str;
	}
      // Amount
      while((step == 1) && *str && *str != ':')
	str++;
      if (step == 1 && (step = 2))
	{
	  if (*str == ':')
	    {
	      *str = '\0';
	      current->amount = strdup(start);
	      str++;
	      start = str;
	    }
	  else
	    {
	      current->amount = strdup(start);
	      current->reference = NULL;
	    }
	}
      // Reference
      while((step == 2) && *str)
	str++;
      if ((step == 2) && (step = 3))
	current->reference = strdup(start);
      
      items->content = current;
      items = items->next;
    }
  return ret;
}

static t_list		*parseCart(char *cookie)
{
  size_t		max;
  size_t		i;
  int			step;
  char			*start;
  t_list		*items;

  items = NULL;
  start = cookie;
  while (*cookie)
    {
      step = 0;
      while (*cookie && *cookie != CART_SEPARATOR)
	cookie++;
      if (*cookie == CART_SEPARATOR && (step = 1))
	{
	  *cookie = '\0';
	  items = addToList(items, strdup(start));
	  start = cookie + 1;
	}
      cookie++;
    }
  
  if (step == 0)
    items = addToList(items, strdup(start));
  /* free(cookie); */
  return items;
}

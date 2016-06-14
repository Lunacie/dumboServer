/*                               -*- Mode: C -*- 
 * 
 * Filename: list.h
 * Description: 
 * Author: Lunacie
 * Created: Sat Jun 20 12:49:49 2015 (+0200)
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


#ifndef			LIST_H
#define			LIST_H

typedef struct		s_list
{
  void			*content;
  struct s_list		*prev;
  struct s_list		*next;
}			t_list;

t_list				*addToList(t_list *list, void *content);
t_list				*addListToList(t_list *list1, t_list *list2);
t_list				*getLastElement(t_list *list);
t_list				*addListAfterElement(t_list *element, t_list *list);
t_list				*addListInPlaceOfElement(t_list *element, t_list *list);
t_list				*getFirstElement(t_list *list);
t_list				*addBeforeList(t_list *list, void *content);

char				*listToString(t_list *list/* , char separator */);
t_bool				stringExistsInList(char *str, t_list *list);

void				destroyList(t_list *list, t_bool allocated);

#endif			/* LIST_H */

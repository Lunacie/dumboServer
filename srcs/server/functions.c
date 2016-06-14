/*                               -*- Mode: C -*- 
 * 
 * Filename: functions.c
 * Description: 
 * Author: Lunacie
 * Created: Mon Aug 17 17:14:54 2015 (+0200)
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

#include		"server.h"
#include		"response.h"
#include		"execution.h"
#include		"list.h"

t_list			*include(t_list **script, t_socket *client)
{
  t_list		*list;
  t_list		*new;
  char			*name;
  char			*ret;

  if (!script)
    return NULL;
  list = *script;
  if (!list || !list->content)
    return NULL;
  if (!(name = getMatchingPattern(list->content, "include('<?>')", FALSE)))
    {
      list->content = "[?c Script include() : Missing Parameter?]\n";
      return *script;
    }
  if (!(name = strAlCat("/", name, strlen(name) + 1 + 1)))
    return NULL;
  if (!(new = getPage(client, name, FALSE)))
    return NULL;
  if (!(new = cutPage(client, new)))
    return NULL;
  if (!(ret = listToString(new)))
    return NULL;
  (*script)->content = ret;
  return *script;
}

t_list			*includeFile(t_socket *client, char *name)
{
  t_list		*list;
  t_list		*new;
  char			*ret;

  if (!(new = getPage(client, name, FALSE)))
    return NULL;
  if (!(new = cutPage(client, new)))
    return NULL;
  if (!(ret = listToString(new)))
    return NULL;
  return new;
}

t_list			*printVariable(t_list **script, t_socket *client)
{
  t_list		*list;
  t_list		*new;
  char			*name;
  char			*ret;
  t_list		*parameters;
  

  if (!script)
    return NULL;
  list = *script;
  if (!list || !list->content)
    return NULL;
  ret = "";
  if (!(name = getMatchingPattern(list->content, "variable('<?>');", FALSE)))
    {
      list->content = "[?c Script variable() : Missing Parameter ?]\n";
      return *script;
    }
  if (!(parameters = client->request->_parameters->list))
    {
      list->content = "[?c Script variable() : No Variables for current request ?]\n";
      return *script;
    }
  while (parameters)
    {
      if (parameters->content && 
	  ((t_parameter *)(parameters->content))->variable &&
	  ((t_parameter *)(parameters->content))->_value &&
	  !strncmp(((t_parameter *)(parameters->content))->variable, name,
		   strlen(((t_parameter *)(parameters->content))->variable)))
	{
	  list->content = ((t_parameter *)(parameters->content))->_value;
	  return *script;
	}
      parameters = parameters->next;
    }
  list->content = "[?c Script variable() : No such variable ?]\n";
  return *script;
}

/*                               -*- Mode: C -*- 
 * 
 * Filename: balise.c
 * Description: 
 * Author: Lunacie
 * Created: Tue Jun 23 14:05:53 2015 (+0200)
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

#include			"program.h"
#include			"server.h"
#include			"list.h"
#include			"handle_error.h"
#include			"http/html/html.h"

char				*closeTag(char *pattern)
{
  char				*tmp;
  char				*ret;
  t_bool			first;

  if (!(tmp = malloc(strlen(pattern) + 2)))
    return (char *)handle_error("malloc", (long int)NULL);
  ret = tmp; 
  first = TRUE;
  while (*pattern)
    {
      *tmp = *pattern;
      if (first == TRUE)
	{
	  tmp++;
	  *tmp = '/';
	  first = FALSE;
	}
      pattern++;
      tmp++;
    }
  *tmp = '\0';
  return ret;
}

char			*title_tag(short nb)
{
  char			*str;
  char			*ret;
  char			*pattern = TITLE_TAG;

  if (!(str = malloc(strlen(TITLE_TAG) + 1)))
    return (char *)handle_error("malloc", (long int)NULL);
  ret = str;
  while(*pattern)
    {
      *str = *pattern;
      if (*pattern == 'x')
	*str = (char)nb + ASCII_DIFF_NB;
      str++;
      pattern++;
    }
  *str = '\0';
  return ret;
}


char			*tagsArray[_NB_TAGTYPES] = 
{
  "<li>",
  "<a>"
    };

char			*htmlTag(tagType type, char *element)
{
  size_t		len;
  char			*open;
  char			*close;
  char			*str;

  open = tagsArray[type];
  close = closeTag(open);
  len = strlen(element) + strlen(open) + strlen(close);
  if (!(str = malloc(len + 1)))
    {
      perror("Malloc : ");
      exit(FAILURE);
    }
  snprintf(str, len, "%s%s%s", open, element, close);
  return str;
}

char			*htmlLinkImage(char *url, char *src, char *alt)
{
  char			*buffer;
  size_t		len;

  if (!url || !src || !alt)
    return NULL;
  len = strlen(url) + strlen(src) + strlen(alt) + (strlen(LINKIMAGE_PATTERN) - 6);
  if (!(buffer = malloc(len + 1)))
    {
      perror("malloc");
      exit(FAILURE);
    }
  snprintf(buffer, len, LINKIMAGE_PATTERN, url, src, alt);
  return buffer;
}

t_list			*htmlInput(t_list *list, char *name, char *label, char *value, char *type, char *suffix)
{
  if (!name)
    return list;
  if (label)
    {
      list = addToList(list, "<label for=\"");
      list = addToList(list, name);
      list = addToList(list, "\">");
      list = addToList(list, label);
      list = addToList(list, "</label>");
    }
  list = addToList(list, "<input type=\"");
  list = addToList(list, type);
  list = addToList(list, "\" name=\"");
  list = addToList(list, name);
  if (value)
    {
      list = addToList(list, "\" value=\"");
      list = addToList(list, value);
    }
  list = addToList(list, "\"/>");
  list = addToList(list, suffix);
  return list;
}

t_list			*htmlTextArea(t_list *list, char *name, char *label, char *value, char *suffix)
{
  if (!name)
    return list;
  if (label)
    {
      list = addToList(list, "<label for=\"");
      list = addToList(list, name);
      list = addToList(list, "\">");
      list = addToList(list, label);
      list = addToList(list, "</label>");
    }
  list = addToList(list, "<textarea name=\"");
  list = addToList(list, name);
  list = addToList(list, "\">");
  if (value)
    list = addToList(list, value);
  list = addToList(list, "</textarea>");
  list = addToList(list, suffix);
  return list;
}

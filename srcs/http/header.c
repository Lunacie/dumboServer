/*                               -*- Mode: C -*- 
 * 
 * Filename: header.c
 * Description: 
 * Author: Lunacie
 * Created: Mon Jun 22 16:30:03 2015 (+0200)
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
#include			"strings.h"
#include			"handle_error.h"
#include			"http.h"

char				*getHttpHeader(t_socket *client, char *fileName)
{
  char				*header;
  char				*tmp;
  char				*suffix;

  header = NULL;
  
  if (stringsAreTheSame(fileName, "404"))
    header = HTTP_404;
  else if (stringsAreTheSame(fileName, "403"))
    header = HTTP_403;
  else if (fileExtensionIs(fileName, ".html") == TRUE)
    header = HTTP_HTML_HEADER;
  else if (fileExtensionIs(fileName, ".css") == TRUE)
    header = HTTP_CSS_HEADER;
  else if ((fileExtensionIs(fileName, ".jpg") == TRUE) ||
	   (fileExtensionIs(fileName, ".jpeg") == TRUE))
    {
      if (!(header = image_header(HTTP_JPG_HEADER, client->request->_size)))
	header = HTTP_417;
      client->request->_isBinary = TRUE;
      return header;
    }
  else if (fileExtensionIs(fileName, ".png") == TRUE)
    {
      if (!(header = image_header(HTTP_PNG_HEADER, client->request->_size)))
	header = HTTP_417;
      client->request->_isBinary = TRUE;
      return header;
    }
  else if (fileExtensionIs(fileName, ".ttf") == TRUE || fileExtensionIs(fileName, ".otf") == TRUE )
    {
      if (!(header = image_header(HTTP_FONT_HEADER, client->request->_size)))
	header = HTTP_417;
      client->request->_isBinary = TRUE;
      return header;
    }
  else 
    header = HTTP_HTML_HEADER;

  tmp = strdup(header);
  if ((suffix = insertCookie(NULL)))
    {
      size_t len;
      
      len = strlen(header) + strlen(suffix);
      if (!(header = malloc(len + 1)))
	{
	  perror("malloc");
	  exit(FAILURE);
	}
      tmp[strlen(tmp) - 2] = '\0';
      snprintf(header, len, "%s%s\r\n\r\n", tmp, suffix);
      free(tmp);
      printf("NEW HEADER : %s\n", header);
      return header;
    }
  return strdup(header);
}


int			addCookie(t_socket *client, char *name, char *value)
{
  char			*old;
  char			*new;
  size_t		len;
  size_t		i;
  char			*cookie;
  char			*tmp;

  printf("Adding Cookie ---- \n");
 if (client && client->header && client->header->content)
    {
      // prep cookie
      len = strlen(name) + strlen(value) +
	strlen(HTTP_COOKIE_PREFIX) + /* strlen(HTTP_COOKIE_SUFFIX) + strlen(HTTP_RETURN) */6 + 1 + 1;
      if (!(cookie = malloc(len + 1)))
	{
	  perror("malloc");
	  exit(FAILURE);
	}
      tmp = cookie;
      *cookie = '\0';
      snprintf(cookie, len,
      	       HTTP_COOKIE_PREFIX"%s=%s;\r\n\r\n",
	       name, value/* , */
      	       /* HTTP_COOKIE_SUFFIX , HTTP_RETURN */);

      // prep old
      old = client->header->content;
      while (old && strlen(old) > 4 && old[strlen(old) - 2] == '\r' && old[strlen(old) - 4] == '\r')
	{
	  printf("\tRemoved\\r\\n");
	  old[strlen(old) - 2] = '\0';
	}

      // prep old + new
      if (old)
	len = strlen(old) + strlen(cookie);
      else
	len = strlen(cookie);
      if (!(new = malloc(len + 1)))
	{
	  perror("malloc");
	  exit(FAILURE);
	}
      for (i = 0; i < len && (*old || *cookie); i++)
	{
	  if (*old)
	    {
	      new[i] = *old;
	      old += 1;
	    }
	  else
	    {
	      new[i] = *cookie;
	      cookie += 1;
	    }
	  /* printf("Char : '%c' -> %d\n", new[i], new[i]); */
	}
      new[i] = '\0';
      free(client->header->content);
      free(tmp);
      client->header->content = new;
      printf("\n\tAdded Cookie - NEW HEADER : \n'%s'", client->header->content);
      return 0;
    }
  return FAILURE;
}

char			*insertCookie(char *new)
{
  static char		*str = NULL;
  char			*tmp;

  if (new)
    str = new;
  else if (!new && str)
    {
      tmp = strdup(str);
      str = NULL;
      return tmp;
    }
  return str;
}


static char		*image_header(char *header, size_t size)
{
  char			*str;

  if (!(str = malloc(MAX_FILE_SIZE_DIGITS + 
		     strlen(HTTP_SIZE) +
		     2 + strlen(HTTP_CACHE) +
		     strlen(header) + 3)))
    return (char *)handle_error("malloc", (long int)NULL);
  sprintf(str, "%s%s%s%d\n\r\n", header, HTTP_CACHE, HTTP_SIZE, size);
  return str;
}

t_bool			fileExtensionIs(char *name, char *extension)
{
  size_t		x;
  size_t		y;

  x = strlen(extension) - 1;
  y = strlen(name) - 1;
  while (x > 0 && extension[x] == name[y])
    {
      x--;
      y--;
    }
  if (x == 0)
    return TRUE;
  return FALSE;
}

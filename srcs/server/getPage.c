/*                               -*- Mode: C -*- 
 * 
 * Filename: getPage.c
 * Description: 
 * Author: Lunacie
 * Created: Fri Jul 10 18:35:16 2015 (+0200)
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

#include			<sys/stat.h>

t_hostPointer			hostArray[NB_HOSTS];


t_list			*getPage(t_socket *client, char *name, t_bool getHeader)
{
  t_list		*list;
  t_list		*header;
  int			fd;
  char			*path;
  char			*buffer;
  int			ret;
  size_t		len;
  size_t		count;
  void			*str;
  
  char			fullPath[1024];
  

  list = NULL;
  if (!(path = getFilePath(client, name)))
    return NULL;
    *fullPath = '\0';
  
  if (access("/home/lunacie/server/.dns", F_OK) != FAILURE)
    {
      snprintf(fullPath, 1024, "/home/lunacie/server/%s", path); 
      printf("Path to file : '%s'\n", fullPath);
      if (access(fullPath, F_OK) == FAILURE)
  	return (list = addToList(list, (void *)getHttpHeader(client, "404")));
    }
  else 
    {
      printf("Path to file (dev): '%s'\n", path);
      if (access(path, F_OK) == FAILURE)
	{
	  printf("Returning 404\n");
	  return (list = addToList(list, (void *)getHttpHeader(client, "404")));
	}
    }
  if(!accessToFileIsAllowed(name, client->request))
    return (list = addToList(list, (void *)getHttpHeader(client, "403")));
  if (*fullPath)
    {
      if ((fd = open(fullPath, O_RDONLY)) == FAILURE)
	return (t_list *)handle_error("open", (long int)NULL);
    }
  else
    {
      
      if ((fd = open(path, O_RDONLY)) == FAILURE)
	return (t_list *)handle_error("open", (long int)NULL);
    }
  len = 0;
  count = 0;
  client->request->_size = lseek(fd, 0, SEEK_END);
  lseek(fd, 0, SEEK_SET);

  if (!(str = malloc(client->request->_size + 1)))
    return (t_list *)handle_error("malloc", (long int)NULL);
  memset(str, 0, client->request->_size);

  ret = read(fd, str, client->request->_size);

  ((char *)(str))[ret] = '\0';

  /* printf("Size requested : %d\nvs read : %d\nvs len : %d\n", client->request->_size, ret, str - ret); */
  /* printf("CONTENt : '%s'\n" ,str); */

  if (getHeader ==  TRUE)
    {
      char *headerBuffer;

      headerBuffer = getHttpHeader(client, name);
     
      /* header = addToList(NULL, headerBuffer); */
      if (!(list = addToList(list, headerBuffer)))
	return NULL;
      client->header = list;
    }

  if (!(list = addToList(list, str)))
    return NULL;


  close(fd);
  free(path);
  return list;
}

static t_bool			accessToFileIsAllowed(char *path, t_request *request)
{
  t_list			*list;

  if (!(list = hostArray[request->_project]._getFileList()))
    return FALSE;
  while (list)
    {
      if (stringsAreTheSame((char *)list->content, path))
	{
	  printf("%s : Found the requested file\n", path);
	  return TRUE;
	}
      list = list->next;
    }
  return FALSE;
}

static char			*getFilePath(t_socket *client, char *name)
{
  char				*tmp;
  char				*path;
  
  
  path = SRCS_PATH;
  
  if (!name)
    return NULL;
  /* printf("path : %s\n hostArray : %s\n name : %s\n", path, hostArray[client->request->_project].directory, */
  /* 	name); */

  if ((path = strAlCat(path, hostArray[client->request->_project].directory,
		       strlen(path) + strlen(hostArray[client->request->_project].directory) + 1)))
    {
      tmp = path;
      if ((path = strAlCat(path, name, strlen(path) + strlen(name) + 1)))
	free(tmp);
    }
  return path;
}


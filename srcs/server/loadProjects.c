/*                               -*- Mode: C -*- 
 * 
 * Filename: loadProjects.c
 * Description: 
 * Author: Lunacie
 * Created: Tue Aug 11 16:30:11 2015 (+0200)
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


#include		<dlfcn.h>

#include		"program.h"
#include		"server.h"
#include		"projects.h"
#include		"response.h"
#include		"loadProjects.h"
#include		"handle_error.h"

t_hostPointer		hostArray[NB_HOSTS]; 

extern char		*pwd;

int			loadProjects(t_socket *client, t_projects id, char *lib)
{
  void			*handle;
  char			*error;
  char			path[1024];


  printf("Lib path : %s\n", lib);

  if (access("/home/lunacie/server/.dns", F_OK) != FAILURE)
    {
      *path = '\0';
      snprintf(path, 1024, "/home/lunacie/server/%s", lib);
      handle = dlopen(path, RTLD_LAZY);
    }
  else
    handle = dlopen(lib, RTLD_LAZY);

  if (!handle)
    return handle_error("dlopen", FAILURE);
  set_handle(handle);

  if (!(hostArray[id]._getResponse = dlsym(handle, "_getResponse")))
    return FAILURE;
  if (!(hostArray[id]._exec = dlsym(handle, "_exec")))
    return FAILURE;
  if (!(hostArray[id]._getFileList = dlsym(handle, "_getAllowedFileList")))
    return FAILURE;

  dlerror();
  if ((error = dlerror()))
    return handle_error(error, FAILURE);
  printf("\tProject was succesfully loaded from dynamic lib\n");
  return 0;
}

void			set_handle(void *new)
{
  static void		*handle = NULL;
  
  if (!new && handle)
    dlclose(handle);
  handle = new;
}

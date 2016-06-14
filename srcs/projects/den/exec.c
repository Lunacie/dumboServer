

#include			 <dirent.h> 

#include			"server.h"
#include			"program.h"
#include			"handle_error.h"
#include			"list.h"
#include			"strings.h"
#include			"response.h"
#include			"execution.h"
#include			"databases.h"
#include			"projects.h"
#include			"parameters.h"
#include			"http/http.h"
#include			"http/html/html.h"

#include			"projects/den.h"

#define				NB_FUNCTIONS			1


static t_function		functionsArray[NB_FUNCTIONS] = 
  {
    {"printPosts", &printPosts}
  };

t_list				*_exec(t_list **script, t_socket *client)
{
  size_t			x;
  char				*name;
  t_list			*new;
  t_list			*list;

  list = *script;
  if (!(name = getFunctionName((char *)(list->content))))
    {
      list->content = (void *)INVALID_SCRIPT;
      return list;
    }
  for (x = 0; x < NB_FUNCTIONS; x++)
    {
      if (stringsAreTheSame(functionsArray[x].name, name))
	{
	  if (functionsArray[x]._ptr != NULL)
	    {
	      return functionsArray[x]._ptr(script, client);
	    }
	  else
	    {
	      printf("Script `%s' : null pointer.\n", name);
	      list->content = (void *)NULL_FUNCTION_SCRIPT;
	      return list;
	    }
	}
    }
  printf("Script `%s' was not found.\n", name);
  list->content = (void *)(NOT_FOUND_SCRIPT);
  return list;
}



#include		"parsePost.c"


static t_list		*printPosts(t_list **script, t_socket *client)
{
  t_list		*list;
  t_list		*ret;
  t_list		*result;
  char			*parameter;
  DIR			*d;
  struct dirent		*dir;
  char			*path;
  char			fullPath[1024];

  list = NULL;
  path = "projects/den/srcs/posts/";
  setDatabase("den");
  setDatabaseSystem(MYSQL);
  setScript("SELECT file FROM posts ORDER BY id;");
  if ((ret = execRequest()) && (ret = ret->next))
    {
      while (ret && ret->content)
	{
	  parameter = ret->content;
	  snprintf(fullPath, 1024, "%s%s", path, parameter);
	  
	  /* list = addToList(list, fullPath); */
	    if (!list)
	      list = parsePost(fullPath);
	    else if (result = parsePost(fullPath))
	      list = addListToList(list, result);
	    ret = ret->next;
	}
    }
  /* d = opendir(path); */
  /* if (d) */
  /*   { */
  /*     while ((dir = readdir(d))) */
  /* 	{ */
  /* 	  printf("%s\n", dir->d_name); */
  /* 	  if ((!strncmp(dir->d_name, ".", 1) && strlen(dir->d_name) == 1) || */
  /* 	      (!strncmp(dir->d_name, "..", 2) && strlen(dir->d_name) == 2)); */
  /* 	  else */
  /* 	    { */
  /* 	      if (dir->d_name[strlen(dir->d_name) - 1] != '~') */
  /* 		{ */
  /* 		  snprintf(fullPath, 1024, "%s%s", path, dir->d_name); */
  /* 		  if (!list) */
  /* 		    list = parsePost(fullPath); */
  /* 		  else if (ret = parsePost(fullPath)) */
  /* 		    list = addListToList(list, ret); */
  /* 		} */
  /* 	    } */
	  
  /* 	} */
  /*     closedir(d); */
  /*   } */
  return commitResult(list, script);
}

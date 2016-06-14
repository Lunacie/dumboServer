#include			"server.h"
#include			"program.h"
#include			"handle_error.h"
#include			"list.h"
#include			"strings.h"
#include			"response.h"
#include			"execution.h"
#include			"session.h"
#include			"projects.h"
#include			"http/http.h"
#include			"http/html/html.h"
#include			"databases.h"
#include			"parameters.h"

#include			"projects/lapothicaire.h"


#define				NB_FUNCTIONS			11

static t_function		functionsArray[NB_FUNCTIONS] = 
  {
    {"displayMenu", displayMenu},
    {"searchProduct", searchProduct},
    {"membersArea", membersArea},
    {"createAccount", createAccount},
    {"accountValidation", accountValidation},
    {"printCategoryName", printCategoryName},
    {"printBrandName", printBrandName},
    {"printProductName", printProductName},
    {"productPage", productPage},
    {"cartPage", cartPage},
    {"orderPage", orderPage},
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
	      free(name);
	      return functionsArray[x]._ptr(script, client);
	    }
	  else
	    {
	      printf("Script `%s' : null pointer.\n", name);
	      list->content = (void *)NULL_FUNCTION_SCRIPT;
	      free(name);
	      return list;
	    }
	}
    }
  printf("Script `%s' was not found.\n", name);
  list->content = (void *)(NOT_FOUND_SCRIPT);
  return list;
}




#include			"membersArea.c"
#include			"productSearch.c"
#include			"menu.c"
#include			"createAccount.c"
#include			"accountValidation.c"
#include			"handleAccount.c"
#include			"product.c"
#include			"panier.c"
#include			"order.c"



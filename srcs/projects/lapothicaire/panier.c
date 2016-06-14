/*                               -*- Mode: C -*- 
 * 
 * Filename: panier.c
 * Description: 
 * Author: Lunacie
 * Created: Thu Jan 21 09:42:00 2016 (+0100)
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
#include			"response.h"
#include			"execution.h"
#include			"session.h"
#include			"projects.h"
#include			"http/http.h"
#include			"http/html/html.h"
#include			"databases.h"
#include			"parameters.h"

#include			"projects/lapothicaire.h"


#include			"checkCartDisplay.c"
#include			"parsingCart.c"

static t_list		*cartPage(t_list **script, t_socket *client)
{
  t_list		*list;
  t_list		*items;
  char			*cookie;
  t_bool		edit;
  char			*tmp;

  edit = FALSE;
  list = NULL;
  if (!script || !*script || !client || !client->request)
    return NULL;
  
  printf("\tCartPage :: \n");
  
  if ((getParameter(client, "sentCart") && (edit = TRUE)) || 
       (cookie = getCookie(client->request, CART_COOKIE)))
    {
      if ((edit == TRUE) ||
	  (items = parseCart(cookie)))
	{
	  if (edit == TRUE ||
	      (items = parseItems(items)))
	    {
	      if ((edit == TRUE && (items = parseItemsEdit(client))) || 
		  (items = requestItemsCart(list, items)))
		{
		  checkItems(items);
		  if (edit == TRUE)
		    {
		      commitItems(client, items);
		      list = checkCartDisplay(list, items);
		    }
		  else
		    {
		      list = includeFile(client, "/pages/cartHead.html");
		      list = displayItemsCart(list, items);
		      list = addListToList(list, includeFile(client, "/pages/cartFoot.html"));
		    }
		}
	      else
		list = addToList(list, strdup("<b>Item(s) not in database</b><br/>\n"));
	    }
	  else
	    list = addToList(list, strdup("<b>Parsing error (items)</b><br/>\n"));
	  	
	}
      else
	list = addToList(list, strdup("<b>Parsing error (cart)</b><br/>\n"));
    }
  else
    list = addToList(list, "<p>Pas d'articles dans le panier.</p><br/>\n");
  
  return commitResult(list, script);
}

static void		commitItems(t_socket *client, t_list *items)
{
  size_t		i;
  size_t		len;
  t_list		*start;
  t_product		*item;
  char			*buffer;
  char			tmp[1024];

  start = items;
  len = 0;
  while (items && (item = items->content))
    {
      if (item->id)
	len += strlen(item->id) + 1;
      if (item->amount)
	len += strlen(item->amount) + 1;
      if (item->variant)
	len += strlen(item->variant);

      if (items->next)
	len += 1;
      items = items->next;
    }

  items = start;
  if (!(buffer = malloc(len + 1)))
    {
      perror("malloc");
      exit(FAILURE);
    }

  *buffer = '\0';
  while (items && (item = items->content))
    {
      if (item->id)
	{
	  buffer = strncat(buffer, item->id, strlen(item->id));
	  buffer = strncat(buffer, ":", 1);
	}
      if (item->amount)
	{
	  buffer = strncat(buffer, item->amount, strlen(item->amount));
	}
      if (item->variant)
	{
	  buffer = strncat(buffer, ":", 1);
	  buffer = strncat(buffer, item->variant, strlen(item->variant));
	}
      printf("buffer : '%s'\n");
      if (items->next)
	buffer = strncat(buffer, ",", 1);
      items = items->next;
    }
  addCookie(client, CART_COOKIE, buffer);
}

static void		checkItems(t_list *items)
{
  t_product		*current;
  t_product		*original;
  t_list		*start;
  t_list		*tmp;
  char			buffer[10];

  size_t		i;
  size_t		j;
  int			amount;
  int			originalAmount;

  start = items;
  i = 0;
  while (items && (original = items->content) && original->amount)
    {
      originalAmount = atoi(original->amount);
      tmp = start;
      j = 0;
      while (tmp && (current = tmp->content) && current->amount)
	{
	  if (i != j &&
	      stringsAreTheSame(original->id, current->id) &&
	      (*current->amount != '0' && *original->amount != '0') &&
			((!current->variant) ||
			 (stringsAreTheSame(original->variant, current->variant))))
	    {
	      printf("DOUBLE : Will not be displayed -- \n");
	      amount = atoi(current->amount);
	      free(original->amount);
	      if (snprintf(buffer, 10, "%d", originalAmount + amount))
		original->amount = strdup(buffer);
	      else
		original->amount = NULL;
	      if (snprintf(buffer, 10, "%d", 0))
		current->amount = strdup(buffer);
	    }
	  tmp = tmp->next;
	  j++;
	}
      items = items->next;
      i++;
    }
}

static t_list		*displayItemsCart(t_list *list, t_list *items)
{
  t_list		*start;
  t_product		*current;
  char			buffer[4096];
  size_t		count;


  start = items;
  list = addToList(list, strdup("<script type=\"text/javascript\">\n"));
  list = addToList(list, strdup("var amounts = new Array();\n"));
  list = addToList(list, strdup("var prices = new Array();\n"));
  list = addToList(list, strdup("var pricesHT = new Array();\n"));
  list = addToList(list, strdup("</script>\n"));
  
  *buffer = '\0';
  count = 0;
  while (items && (current = items->content) &&
	 current->amount)
    {
      if (*current->amount != '0')
	{
	  snprintf(buffer, 4096, "<ul id=\"item_%d\">\n" ,count);
	  list = addToList(list, strdup(buffer));

	  // Name
	  if (current->ProduitNom)
	    {
	      if (current->variant)
		{
		  snprintf(buffer, 4096, "<li class=\"article\">%s<br/><i>%s</i></li>\n"\
			   "<input type=\"hidden\" name=\"item_%d\" value=\"%s\"/>\n"\
			   "<input type=\"hidden\" name=\"variant_%d\" value=\"%s\"/>\n",
			   current->ProduitNom, current->variant,
			   count, current->id,
			   count, current->reference);
		}
	      else
		snprintf(buffer, 4096, "<li class=\"article\">%s<br/></li>\n"\
			 "<input type=\"hidden\" name=\"item_%d\" value=\"%s\"/>\n",
			 current->ProduitNom, count, current->id);
	      list = addToList(list, strdup(buffer));
	    }

	  // Stock
	  if (current->ProduitStockStatut)
	    {
	      *buffer = '\0';
	      if (atoi(current->ProduitStockStatut) > 0)
		snprintf(buffer, 4096,
			 "<li class='dispo'><img src=\"images/available.png\" alt='Disponible'/></li>");
	      else
		snprintf(buffer, 4096,
			 "<li class='dispo'><img src=\"images/notAvailable.png\" alt='Indisponible'/></li>");
	      list = addToList(list, strdup(buffer));
	    }

      
	  // Amount
	  if (current->amount)
	    {
	      list = addToList(list, strdup("<li class='quantite'>\n"));
	      *buffer = '\0';
	      snprintf(buffer, 4096,
		       "<input type='number' value='%s' "\
		       "onchange=\"updateAmount(this, this.value, %d)\" name=\"amount_%d\"/>\n",
		       current->amount, count, count);
	      list = addToList(list, strdup(buffer));
	      list = addToList(list, strdup("</li>\n"));
	    }
      
	  // Price
	  if (current->ProduitPrixTTC && current->ProduitPrixHT)
	    {
	      *buffer = '\0';
	      snprintf(buffer, 4096, "<li class='prix'><b>%s&#8364;</b><br/>(%s&#8364; HT)</li>\n",
		       current->ProduitPrixTTC, current->ProduitPrixHT);
	      list = addToList(list, strdup(buffer));
	    }

	  // Remove
	  *buffer = '\0';
	  snprintf(buffer, 4096,
		   "<li class=\"remove\">"\
		   "<a  href=\"javascript:;\" onclick=\"removeItemId(event, %d); return false;\">" \
		   "<img src=\"images/delete.png\"/></a>"			\
		   "</li>\n",
		   count);
	  list = addToList(list, strdup(buffer));
	  list = addToList(list, "<hr/>\n");   
      
	  list = addToList(list, "</ul>\n");   


	  snprintf(buffer, 4096, "<script type=\"text/javascript\">\n"	\
		   "amounts[%d] = %s;\n"				\
		   "prices[%d] = %s;\n"				\
		   "pricesHT[%d] = %s;\n"				\
		   "</script>\n",
		   count, current->amount,
		   count, replaceCharactersInStr(',', '.', current->ProduitPrixTTC), 
		   count, replaceCharactersInStr(',', '.', current->ProduitPrixHT));
	  list = addToList(list, strdup(buffer));
	  count += 1;
	}
      items = items->next;
    }

  snprintf(buffer, 4096, "<input type=\"hidden\" name=\"sentCart\" value=\"1\"/>\n");
  list = addToList(list, strdup(buffer));


  destroyList(start, FALSE);
  return list;
}



static t_list		*parseItemsEdit(t_socket *client)
{
  t_list		*items;
  char			request[4096];
  t_product		*current;
  t_list		*ret;
  t_list		*tmp;
  t_list		*start;
  char			*item;
  char			*amount;
  char			*reference;
  size_t		count;

  start = NULL;
  items = NULL;
  *request = '\0';
  count = 0;
  while (snprintf(request, 4096, "item_%d", count) &&
	 (item = getParameter(client, request)))
    {
      setDatabaseSystem(MYSQL);
      setDatabase("lapothicaire");

      // Basic data
      *request = '\0';
      snprintf(request, 4096,
      	       "SELECT "\
      	       "ProduitNom, ProduitPrixHT, ProduitPrixTTC, ProduitStockStatut "\
      	       "FROM produits WHERE produitReference='%s'", item);
      setScript(request);

      if ((ret = execRequest()) && (tmp = ret) &&
      	  (ret = ret->next) && ret->content)
      	{
	  if (!(current = malloc(sizeof(t_product))))
	    {
	      perror("malloc");
	      exit(FAILURE);
	    }
      	  printf("\n=====================\n");
	  snprintf(request, 4096, "amount_%d", count);
	  if (!(amount = getParameter(client, request)))
	    amount = "0";
	  snprintf(request, 4096, "variant_%d", count);
	  if ((reference = getParameter(client, request)))
	    current->reference = strdup(reference);
	  else
	  current->reference = NULL;
	  

	  current->variant = NULL;
	  current->id = strdup(item);
	  current->amount = strdup(amount);

      	  setRequestResult(ret->content);
      	  getNextResultValue(&current->ProduitNom);
      	  getNextResultValue(&current->ProduitPrixHT);
      	  getNextResultValue(&current->ProduitPrixTTC);
      	  getNextResultValue(&current->ProduitStockStatut);
      	  setRequestResult(NULL);

      	  printf("ID : '%s'\n", current->id);
      	  printf("Amount : '%s'\n", current->amount);
      	  printf("Nom : '%s'\n", current->ProduitNom);

      	  destroyList(tmp, FALSE);
	  if (items == NULL)
	    {
	      items = addToList(items, current);
	      start = items;
	    }
	  else
	    items = addToList(items, current);
      	}
      else
      	return NULL;

      // Variant
      if (current->reference)
      	{
      	  current->variant = NULL;
      	  snprintf(request, 4096,
      		   "SELECT "						\
      		   "Nom, DeclinaisonPrixHT, DeclinaisonPrixTTC, DeclinaisonStock "		\
      		   "FROM declinaisons WHERE DeclinaisonReference='%s'", current->reference);
      	  setScript(request);
      	  if ((ret = execRequest()) && (tmp = ret) &&
      	      (ret = ret->next) && ret->content)
      	    {
      	      setRequestResult(ret->content);
      	      getNextResultValue(&current->variant);
      	      getNextResultValue(&current->ProduitPrixHT);
      	      getNextResultValue(&current->ProduitPrixTTC);
      	      getNextResultValue(&current->ProduitStockStatut);
      	      setRequestResult(NULL);

      	      printf("Déclinaison : '%s'\n", current->variant);
      	      destroyList(tmp, FALSE);
      	    }
      	  /* else */
      	  /*   return NULL; */
      	}


      printf("Price : '%s'\n", current->ProduitPrixTTC);
      printf("(price) : '%s'\n", current->ProduitPrixHT);
      printf("Stock : '%s'\n", current->ProduitStockStatut);
      printf("--------------------\n");
      count += 1;
    }
  return start;

}


static t_list		*requestItemsCart(t_list *list, t_list *items)
{
  char			request[4096];
  t_product		*current;
  t_list		*ret;
  t_list		*tmp;
  t_list		*start;


  start = items;
  *request = '\0';
  while (items && items->content)
    {
      current = items->content;
      setDatabaseSystem(MYSQL);
      setDatabase("lapothicaire");

      // Basic data
      snprintf(request, 4096,
  	       "SELECT "\
  	       "ProduitNom, ProduitPrixHT, ProduitPrixTTC, ProduitStockStatut "\
	       "FROM produits WHERE produitReference='%s'", current->id);
      setScript(request);
      if ((ret = execRequest()) && (tmp = ret) &&
	  (ret = ret->next) && ret->content)
	{
	  setRequestResult(ret->content);
	  getNextResultValue(&current->ProduitNom);
	  getNextResultValue(&current->ProduitPrixHT);
	  getNextResultValue(&current->ProduitPrixTTC);
	  getNextResultValue(&current->ProduitStockStatut);
	  setRequestResult(NULL);

	  printf("\n=====================\n");
	  printf("ID : '%s'\n", current->id);
	  printf("Amount : '%s'\n", current->amount);
	  printf("Nom : '%s'\n", current->ProduitNom);

	  destroyList(tmp, FALSE); 
	}
      /* else */
      /* 	return NULL; */

      // Variant
      if (current->reference)
	{
	  current->variant = NULL;
	  snprintf(request, 4096,
		   "SELECT "						\
		   "Nom, DeclinaisonPrixHT, DeclinaisonPrixTTC, DeclinaisonStock "		\
		   "FROM declinaisons WHERE DeclinaisonReference='%s'", current->reference);
	  setScript(request);
	  if ((ret = execRequest()) && (tmp = ret) &&
	      (ret = ret->next) && ret->content)
	    {
	      setRequestResult(ret->content);
	      getNextResultValue(&current->variant);
	      getNextResultValue(&current->ProduitPrixHT);
	      getNextResultValue(&current->ProduitPrixTTC);
	      getNextResultValue(&current->ProduitStockStatut);
	      setRequestResult(NULL);

	      printf("Déclinaison : '%s'\n", current->variant);
	      destroyList(tmp, FALSE); 
	    }
	  /* else */
	  /*   return NULL; */
	}


      printf("Price : '%s'\n", current->ProduitPrixTTC);
      printf("(price) : '%s'\n", current->ProduitPrixHT);
      printf("Stock : '%s'\n", current->ProduitStockStatut);
      items = items->next;
    }
  return start;
}


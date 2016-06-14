/*                               -*- Mode: C -*- 
 * 
 * Filename: order.c
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


static t_list		*orderPage(t_list **script, t_socket *client)
{
  t_list		*list;
  t_list		*items;
  char			*cookie;
  t_bool		edit;
  char			*tmp;
  char			*username;
  char			*id;

  edit = FALSE;
  list = NULL;
  if (!script || !*script || !client || !client->request)
    return NULL;
  
  printf("\tOrderPage :: \n");
  
  // 1. IS AUTH ?
  if (isAuthenticated(script, client))
    {
      username = getCookie(client->request, "lapothicaireSESSIONusername");

      // 2a. admin ? Order was treated
      if ((id = getParameter(client, "id")) &&
	  stringsAreTheSame(username, "admin"))
	list = setOrderWasTreated(list, id);

      // 2. Handle new order
      if ((getParameter(client, "add") &&
	   (cookie = getCookie(client->request, CART_COOKIE))))
	{ 
	  if (items = parseCart(cookie))
	    {
	      if (items = parseItems(items))
		{
		  if (items = requestItemsCart(list, items))
		    {
		      checkItems(items);
		      list = addNewOrder(list, items, username);
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

      // 3. Display all orders;
      if (username)
	list = displayOrders(list, username, client);
    }
  else
    {
      list = addToList(list, 
		       strdup("<b>Finalisez votre inscription pour valider la commande "\
			      "OU <a href=\"./member\">Se connecter</a>.</b><br/>\n"));
      list = addListToList(list, includeFile(client, "/pages/inscription.html"));
    }

  return commitResult(list, script);
}


static t_list		*setOrderWasTreated(t_list *list, char *id)
{
  char			request[1024];
  t_list		*ret;
  t_list		*tmp;

  snprintf(request, 1024,
	   "UPDATE orders SET "				\
	   "status='Commande traitée' " \
	   "WHERE orderId='%s'; ROW_COUNT();", id);
  setDatabase("lapothicaire");
  setScript(request);
 if ((ret = execRequest()) && (tmp = ret) && (ret = ret->next) && ret->content)
   {
     list = addToList(list, "<b>Modification effectuées avec succès.</b>");
     destroyList(ret, TRUE);
   }
  return list;
}

static t_list		*displayOrders(t_list *list, char *username, t_socket *client)
{
  t_list		*start;
  t_product		*current;
  char			buffer[4096];
  char			request[1024];
  t_list		*ret;
  t_list		*tmp;
  size_t		count;

  char			*orderId;
  char			*orderDate;
  char			*priceHT;
  char			*priceTTC;
  char			*status;
  char			*description;
  char			*clientUsername;
  char			*lastname;
  char			*firstname;
  char			*address;
  t_bool		admin;

  admin = FALSE;
  list = addListToList(list, includeFile(client, "/pages/ordersDisplay.html"));
  
  if (!stringsAreTheSame("admin", username))
    snprintf(request, 1024,
	     "SELECT orderId, orderDate, "				\
	     "priceHT, priceTTC, status FROM orders WHERE username='%s'", 
	     username);
  else
    {
      snprintf(request, 1024,
	       "SELECT orderId, orderDate, "				\
	       "priceHT, priceTTC, status, textContent, username, lastname, "\
	       "firstname, address FROM orders ORDER BY orderDate");
      admin = TRUE;
    }
  setDatabase("lapothicaire");
  setScript(request);
  if ((ret = execRequest()) && (tmp = ret) && (ret = ret->next) && ret->content)
    {
      do
	{
	  setRequestResult(ret->content);
	  getNextResultValue(&orderId);
	  getNextResultValue(&orderDate);
	  getNextResultValue(&priceHT);
	  getNextResultValue(&priceTTC);
	  getNextResultValue(&status);
	  if (admin)
	    {
	      getNextResultValue(&description);
	      getNextResultValue(&clientUsername);
	      getNextResultValue(&lastname);
	      getNextResultValue(&firstname);
	      getNextResultValue(&address);
	    }
	    
	  setRequestResult(NULL);


	  /* snprintf(buffer, 4096, */
	  /* 	   "<ul><li>%s</li>", orderId); */
	  /* list = addToList(list, strdup(buffer)); */

	  snprintf(buffer, 4096,
		   "<ul><li>%s</li>", orderDate);
	  list = addToList(list, strdup(buffer));

	  snprintf(buffer, 4096,
		   "<li><b>%s&#8364;</b> (%s&#8364;HT)</li>", priceTTC, priceHT);
	  list = addToList(list, strdup(buffer));

	  snprintf(buffer, 4096,
		   "<li>%s</li>", status);
	  list = addToList(list, strdup(buffer));

	  if (stringsAreTheSame(status, "En attente de paiement") && !admin)
	    {
	      list = addToList(list, strdup("<li>"));
	      list = paypalButton(list, priceTTC);
	      list = addToList(list, strdup("</li>"));
	    }

	  
	  if (admin)
	    {
	      snprintf(buffer, 4096,
		       "<b>%s %s (%s) %s</b> <li>%s</li>",
		       lastname, firstname, clientUsername, address, parseOrder(description));
	      list = addToList(list, strdup(buffer));
	      snprintf(buffer, 4096,
		       "<a href=\"/commandes?id=%s\">Valider</a>", orderId);
	      list = addToList(list, strdup(buffer));
	    }

	  *buffer = '\0';
	  snprintf(buffer, 4096,
		   "</ul>", status);
	  list = addToList(list, strdup(buffer));

	  /* if (admin) */
	  /*   list = addToList(list, strdup("<hr/>")); */



	} while ((ret = ret->next) && ret->content);
      /* list = addToList(list, strdup(ret->content)); */
      destroyList(tmp, TRUE);
    }
  else
    list = addToList(list, "<p>Pas de commandes en cours.</p>");
  return list;
}


static char		*parseOrder(char *str)
{
  char			*found;
  char			*start;
  const char		*pattern = "<br/>";
  size_t		x;
  
  start = str;
  while ((found = strstr(str, "     Article #")) ||
	 (found = strstr(str, "     Total")))
    {
      for (x = 0; x < strlen(pattern); x++)
	found[x] = pattern[x];
      str = found;
      str++;
    }
  return start;
}

static t_list		*paypalButton(t_list *list, char *price)
{
  char			buffer[4096];

  snprintf(buffer, 4096, 
	   "<form action=\"https://www.paypal.com/cgi-bin/webscr\" method=\"post\" target=\"_top\">" \
	   "<input type=\"hidden\" name=\"cmd\" value=\"_xclick\">"	\
	   "<input type=\"hidden\" name=\"business\" value=\"apothicaire.ohm@gmail.com\">" \
	   "<input type=\"hidden\" name=\"lc\" value=\"FR\">"		\
	   "<input type=\"hidden\" name=\"item_name\" value=\"L'apothicaire\">"	\
	   "<input type=\"hidden\" name=\"amount\" value=\"%s\">"	\
	   "<input type=\"hidden\" name=\"currency_code\" value=\"EUR\">" \
	   "<input type=\"hidden\" name=\"button_subtype\" value=\"services\">"	\
	   "<input type=\"hidden\" name=\"no_note\" value=\"0\">"	\
	   "<input type=\"hidden\" name=\"cn\" value=\"Commentaires particuliers ? :\">" \
	   "<input type=\"hidden\" name=\"no_shipping\" value=\"2\">"	\
	   "<input type=\"hidden\" name=\"rm\" value=\"1\">"		\
	   /* "<input type=\"hidden\" name=\"return\" value=\"http://lapothicaire-vapeshop.com/callback\">" \ */
	   "<input type=\"hidden\" name=\"tax_rate\" value=\"0\">"	\
	   "<input type=\"hidden\" name=\"shipping\" value=\"0\">"	\
	   "<input type=\"hidden\" name=\"bn\" value=\"PP-BuyNowBF:btn_buynowCC_LG.gif:NonHosted\">" \
	   "<input type=\"image\" src=\"https://www.paypalobjects.com/fr_FR/FR/i/btn/btn_buynowCC_LG.gif\" " \
	   "border=\"0\" name=\"submit\" alt=\"PayPal, le réflexe sécurité pour payer en ligne\">" \
	   "<img alt=\"\" border=\"0\" src=\"https://www.paypalobjects.com/fr_FR/i/scr/pixel.gif\"" \
	   "width=\"1\" height=\"1\">"					\
	   "</form>", price);


  list = addToList(list, strdup(buffer));
  return list;
}


static t_list		*addNewOrder(t_list *list, t_list *items, char *username)
{
  char			*order;
  float			total;
  float			totalHT;
  char			*str;

  char			request[4096];
  char			buffer[4096];
  t_list		*ret;
  t_list		*tmp;

  char			*key;

  // 1. Format Order meta
  if ((order = formatOrder(items, &total, &totalHT)));
  /* list = addToList(list, order); */
  else
    {
      list = addToList(list, strdup("<b>Could not format order</b><br/>\n"));
      return list;
    }


  // 2. Get ID for order
  if (!(key = getUniqueId()))
    {
      list = addToList(list, "<b>Une erreur est survenue à la "	\
		       "création de votre clé de validation. "	\
		       "Veuillez réitérer ultérieurement.</b><br/>");
      return list;
    }


  snprintf(request, 1024,
	   "SELECT nom, prenom, adresse FROM users WHERE username='%s';", username);
  setDatabase("lapothicaire");
  setScript(request);
  if ((ret = execRequest()) && (tmp = ret) && (ret = ret->next) && ret->content)
    {
      // 3. Request

      char	*name;
      char	*lastname;
      char	*adress;

      setRequestResult(ret->content);
      getNextResultValue(&name);
      getNextResultValue(&lastname);
      getNextResultValue(&adress);
      setRequestResult(NULL);


      snprintf(request, 4094, "INSERT INTO orders "	\
	       "(orderId, "\
	       "textContent, "\
	       "username, firstname, lastname, address, "\
	       "orderDate, priceHT, priceTTC, status) "						\

	       "VALUES ('%s', "\
	       "'%s', "\
	       "'%s', '%s', '%s', '%s', "\
	       "%s, '%.2f', '%.2f', '%s'); SELECT ROW_COUNT();",

	       key,
	       order,
	       "Lunacie", name, lastname, adress,
	       "NOW()",totalHT , total, "En attente de paiement");
    }  
  /* list = addToList(list, request); */


  setScript(request);
  if ((ret = execRequest()))
    {
      tmp = ret;
      if ((ret = ret->next) && ret->content &&
  	  !strncmp(ret->content, "1", 1))
  	{
  	  destroyList(tmp, TRUE);
	  list = addToList(list, "<b>Votre commande a été passée avec succès.</b><br/>");

	  // 4. Adress
	  // ----------
	  snprintf(buffer, 4096, "<h3>Adresse de livraison : </h3>\n");
	  list = addToList(list, strdup(buffer));

	  snprintf(request, 1024,
		   "SELECT adresse, nom, prenom FROM users WHERE username='%s' LIMIT 1;", username);
	  setDatabase("lapothicaire");
	  setScript(request);
	  if ((ret = execRequest()) && (tmp = ret) && (ret = ret->next) && ret->content)
	    {
	      char	*name;
	      char	*lastname;
	      char	*adress;
	      setRequestResult(ret->content);
	      getNextResultValue(&adress);
	      getNextResultValue(&lastname);
	      getNextResultValue(&name);
	      setRequestResult(NULL);

	      list = addToList(list, "<b>");

	      /* str = ret->content; */
	      /* if (str[strlen(ret->content) - 1] == '\\') */
	      /* 	str[strlen(ret->content) - 1] = '\0'; */
	      /* list = addToList(list, strdup(ret->content)); */
	      snprintf(buffer, 4096, "<p>%s %s<br/>\n%s</p>\n", lastname, name, adress);
	      list = addToList(list, strdup(buffer));
	      list = addToList(list, "</b>");
	      destroyList(tmp, TRUE);
	    }
	  else
	    list = addToList(list, "Adresse de livraison manquante.<br/>\n");
	  list = addToList(list, 
			   "<p>Pour modifier l'adresse de livraison, "\
			   "cliquez <a href=\"/member?action=account\">ici</a>.</p>\n");

  	  return list;
  	}
      destroyList(tmp, TRUE);
    }
  else
    list = addToList(list, "<b>Une erreur est survenue à la "	\
		     "création de votre commande. "		\
		     "Veuillez réitérer ultérieurement.</b><br/>");
  return list; 
}




static char		*formatOrder(t_list *items, float *total, float *totalHT)
{
  t_list		*start;
  t_product		*current;
  char			buffer[4096];
  size_t		count;
  t_list		*order;

  float			price;
  char			*result;


  order = NULL;
  start = items;
  *buffer = '\0';
  count = 0;
  *total = 0;
  *totalHT = 0;
  while (items && (current = items->content) &&
	 current->amount)
    {

      snprintf(buffer, 4096, "     Article #%d : " ,count);
      order = addToList(order, strdup(buffer));

      if (*current->amount != '0')
	{
	  // Name
	  if (current->ProduitNom)
	    {
	      if (current->variant)
		{
		  snprintf(buffer, 4096, "%s %s (%s, %s) ", 
			   current->ProduitNom, current->variant,
			   current->id, current->reference);
		}
	      else
		snprintf(buffer, 4096, "%s (%s) ",
			 current->ProduitNom, current->id);
	      order = addToList(order, strdup(buffer));
	    }

	  /* // Stock */
	  /* if (current->ProduitStockStatut) */
	  /*   { */
	  /*     *buffer = '\0'; */
	  /*     if (atoi(current->ProduitStockStatut) > 0) */
	  /* 	snprintf(buffer, 4096, */
	  /* 		 "En stock"); */
	  /*     else */
	  /* 	snprintf(buffer, 4096, */
	  /* 		 "Hors stock"); */
	  /*     order = addToList(order, strdup(buffer)); */
	  /*   } */

      
	  // Amount
	  if (current->amount)
	    {
	      *buffer = '\0';
	      snprintf(buffer, 4096,
		       "Nombre : x%s ",
		       current->amount);
	      order = addToList(order, strdup(buffer));
	    }
      
	  // Price
	  if (current->ProduitPrixTTC && current->ProduitPrixHT)
	    {
	      *buffer = '\0';
	      snprintf(buffer, 4096, "Prix a la commande "\
		       "%s&#8364;    (%s&#8364; HT)            ",
		       current->ProduitPrixTTC, current->ProduitPrixHT);
	      order = addToList(order, strdup(buffer));

	      if (price = atof(toFloat(current->ProduitPrixTTC)))
		*total += price;
	      if (price = atof(toFloat(current->ProduitPrixHT)))
		*totalHT += price; 
	      
	    }

	  count += 1;
	  /* order = addToList(order, "</ul>"); */

	}
      items = items->next;
    }

     
  snprintf(buffer, 4096, "     Total : %.2f"\
	   "     Total HT :  %.2f",
	   *total, *totalHT);
  order = addToList(order, strdup(buffer));

  result = listToString(order);
  destroyList(order, FALSE);
  destroyList(start, FALSE);
    
  return result;
}



/*                               -*- Mode: C -*- 
 * 
 * Filename: product.c
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

static t_list		*printProductName(t_list **script, t_socket *client)
{
  t_list		*list;
  char			request[1024];
  t_list		*ret;
  t_list		*tmp;

  char			*id;

  list = NULL;
  if (!script || !*script)
    return NULL;
  
  printf("\tprintProductName :: \n");
  
  if (!(id = getParameter(client, "id")))
    {
      printf("Missing id parameter\n");
      list = addToList(list, "");
      return commitResult(list, script);
    }
  snprintf(request, 1024,
	   "SELECT ProduitNom FROM produits WHERE ProduitReference='%s' LIMIT 1;", id);
  setDatabase("lapothicaire");
  setScript(request);
  if ((ret = execRequest()) && (tmp = ret) && (ret = ret->next) && ret->content)
    {
      printf("Name was found: %s\n", ret->content);
      list = addToList(list, strdup(ret->content));
      destroyList(tmp, TRUE);
    }
  else
    list = addToList(list, "");
    
  return commitResult(list, script);
}


static t_list		*addProductToCart(t_list *list, t_socket *client, char *id, char *amount)
{
  char			*old;
  char			*buffer;
  size_t		len;
  size_t		offset;
  char			*declinaison;

  if (!id || !*id || !amount || !*amount)
    return NULL;

  len = strlen(id) + 1 + strlen(amount);

  if (old = getCookie(client->request, CART_COOKIE))
    len += (strlen(old) + 1);

  if ((declinaison = getParameter(client, "colorSelect")))
    len += (strlen(declinaison) + 1);

  if (!(buffer = malloc(len + 1)))
    {
      perror("malloc");
      exit(FAILURE);
    }

  *buffer = '\0';
  offset = 0;
  if (old && *old)
    {
      if (old[strlen(old) - 1] == ' ' || old[strlen(old) - 1] == '\t')
	old[strlen(old) - 1] = '\0';
      snprintf(buffer + offset, strlen(old) + 1 + 1, "%s%c", old, CART_SEPARATOR);
      offset += (strlen(old) + 1);
      printf("1 - Buffer : '%s'\n", buffer);
    }

  snprintf(buffer + offset, strlen(id) + strlen(amount) + 1 + 1, "%s:%s", id, amount);
  offset += (strlen(id) + strlen(amount) + 1);
  printf("2 - Buffer : '%s'\n", buffer);

  if (declinaison && *declinaison)
    {
      snprintf(buffer + offset, strlen(declinaison) + 1 + 1, ":%s", declinaison);
      offset += (strlen(declinaison) + 1);
      printf("3 - Buffer : '%s'\n", buffer);
    }

  buffer[len] = '\0';
  addCookie(client, CART_COOKIE, buffer);
  list = addToList(list, "<b>Article ajouté au panier</b>\n");
  return list;
}

static t_list		*productPage(t_list **script, t_socket *client)
{
  t_list		*list;
  char			request[1024];
  t_list		*ret;
  t_list		*tmp;

  char			*id;
  char			*amount;

  list = NULL;
  if (!script || !*script || !client || !client->request)
    return NULL;
  
  printf("\tProductPage :: \n");  
  if (!(id = getParameter(client, "id")))
    {
      printf("Missing id parameter\n");
      list = addToList(list, "");
      return commitResult(list, script);
    }


  // PARAMS WERE FILLED
  if ((amount = getParameter(client, "hiddenAmount")))
    list = addProductToCart(list, client, id, amount);

  /* DISPLAY */
  snprintf(request, 1024,
	   "SELECT "\
	   "MarqueId, CategorieId, TVAId, ProduitStockStatut, "\
	   "ProduitNom, ProduitDescription, ProduitPrixHT, ProduitPrixTTC "\
	   "FROM produits WHERE ProduitReference='%s' LIMIT 1;", id);
  setDatabase("lapothicaire");
  setScript(request);
  if ((ret = execRequest()) && (tmp = ret) && (ret = ret->next) && ret->content)
    {
      char	*content;
      char	*marqueId;
      char	*categorieId;
      char	*TVAId;

      t_product product;
      t_list	*tmp2;

      product.id = id;

      setRequestResult(ret->content);
      getNextResultValue(&marqueId);
      getNextResultValue(&categorieId);
      getNextResultValue(&TVAId);
      
      getNextResultValue(&product.ProduitStockStatut);
      getNextResultValue(&product.ProduitNom);
      getNextResultValue(&product.ProduitDescription);
      getNextResultValue(&product.ProduitPrixHT);
      getNextResultValue(&product.ProduitPrixTTC);
      setRequestResult(NULL);
      

      snprintf(request, 1024,
	       "SELECT CategorieNom FROM categories WHERE CategorieId='%s' LIMIT 1;",
	       categorieId);
      product.categorie = execRequestGetValue();
      snprintf(request, 1024,
	       "SELECT MarqueNom FROM marques WHERE MarqueId='%s' LIMIT 1;",
	       marqueId);
      product.marque = execRequestGetValue();
      snprintf(request, 1024,
	       "SELECT TVAValeur FROM tva WHERE TVAId='%s' LIMIT 1;",
	       TVAId);
      product.TVA = execRequestGetValue();      
      destroyList(tmp, FALSE);

      getDeclinaisons(&product);

      printf("\tCatégorie : '%s'\n", product.categorie);
      printf("\tMarque : '%s'\n", product.marque);
      printf("\tTVA : '%s'\n", product.TVA);
      printf("\tNom : '%s'\n", product.ProduitNom);
      printf("\tDescription : '%s'\n", product.ProduitDescription);
      printf("\tHT : '%s'\n", product.ProduitPrixHT);
      printf("\tTTC : '%s'\n", product.ProduitPrixTTC);


      list = displayProduct(list, &product);
    }
  else
    list = addToList(list, "<p>Pas de résultat pour ce produit.</p>");
  return commitResult(list, script);
}

static int			getDeclinaisons(t_product *product)
{
  char			request[1024];
  t_list		*ret;
  t_list		*tmp;
  t_list		*start;

  start = NULL;
  product->declinaisons = NULL;
  snprintf(request, 1024,
  	   "SELECT "\
	   "Nom, Attribut, Attribut2, CouleurPrimaire, DeclinaisonStock, DeclinaisonPrixTTC, " \
	   "DeclinaisonPrixHT, DeclinaisonReference "\
	   "FROM declinaisons WHERE LOWER(DeclinaisonReference) LIKE '%s%%';", product->id);
  setScript(request);
  if ((ret = execRequest()) && (tmp = ret) && (ret = ret->next) && ret->content)
    {
      while (ret && ret->content)
	{
	  t_declinaison *current;

	  printf("RET : '%s'\n", ret->content);
	  
	  if (!(current = malloc(sizeof(t_declinaison))))
	    {
	      perror("malloc");
	      exit(FAILURE);
	    }
	  setRequestResult(ret->content);
	  getNextResultValue(&current->Nom);
	  getNextResultValue(&current->Attribut);
	  getNextResultValue(&current->Attribut2);
	  getNextResultValue(&current->CouleurPrimaire);
	  getNextResultValue(&current->DeclinaisonStock);
	  getNextResultValue(&current->DeclinaisonPrixTTC);
	  getNextResultValue(&current->DeclinaisonPrixHT);
	  getNextResultValue(&current->DeclinaisonReference);
	  setRequestResult(NULL);
	  
	  product->declinaisons = addToList(product->declinaisons, current);
	  if (!start)
	    start = product->declinaisons;

	  ret = ret->next;
	}
      product->declinaisons = start;
      destroyList(tmp, TRUE);
      return 0;
    }
  return FAILURE;
}

static t_list		*displayProduct(t_list *list, t_product *product)
{
  char			tmp[1024];
  t_declinaison		*current;
  t_list		*declinaisons;
  t_list		*start;
  size_t		 count;

  list = addToList(list, "<span class=\"productPage\">");
  
  *tmp = '\0';
  snprintf(tmp, 1024, "<a href=\"./produit?id=%s\"><img src=\"%s\"/></a>",
  	   id,  "images/no-image.jpg");
  list = addToList(list, strdup(tmp));

  *tmp = '\0';
  snprintf(tmp, 1024, "<h3>%s</h3>",
  	   product->ProduitNom);
  list = addToList(list, strdup(tmp));

  // FORM
  list = addToList(list, strdup("<form method='post' action=\"#\">\n"));

  // hidden values
  //	id (ProduitReference)
  *tmp = '\0';
  if (id)
    {
      snprintf(tmp, 1024, "<input type='hidden' value=\"%s\" name=\"hiddenId\"/>\n",
	       id);
      list = addToList(list, strdup(tmp));
    }

  // Displaying product description
  *tmp = '\0';
  if (strstr(product->ProduitDescription, "<p></p>") == product->ProduitDescription)
    snprintf(tmp, 1024, "<p>Pas de description disponible.</p>\n",
	     product->ProduitDescription);
  else
    snprintf(tmp, 1024, "%s",
	     product->ProduitDescription);
  list = addToList(list, strdup(tmp));

  if ((declinaisons = product->declinaisons))
    {
      list = addToList(list, "<select id=\"colorSelect\" name=\"colorSelect\">\n");
      start = declinaisons;

      //  Displaying options
      while (declinaisons && declinaisons->content)
	{
	  current = declinaisons->content;
	  *tmp = '\0';
	  if (atoi(current->DeclinaisonStock) > 0)
	    {
	      snprintf(tmp, 1024, "<option name=\"declinaison\" onclick=\"setSelectedOption(this);\" "\
		       "value=\"%s\">%s - (%s, %s)</option>\n",
		       current->DeclinaisonReference, current->Nom,
		       current->Attribut, current->Attribut2);
	    }
	  else
	    {
	      snprintf(tmp, 1024, "<option class=\"disabled\"  onclick=\"setSelectedOption(this);\" "\
		       "value=\"%s\">%s - Stock épuisé</option>\n",
		       current->DeclinaisonReference, current->Nom);
	    }
	  list = addToList(list, strdup(tmp));
	  declinaisons = declinaisons->next;
	}
      list = addToList(list, "</select>\n");
      
      count = 0;
      declinaisons = start;
      // displaying color buttons for options
      while (declinaisons && declinaisons->content)
	{
	  current = declinaisons->content;
	  *tmp = '\0';
	  if (atoi(current->DeclinaisonStock) > 0)
	    {
	      if (count == 0 /* or selected */)
		snprintf(tmp, 1024, "<input type=\"button\" "\
			 "name=\"buttonColor%d\" "		     \
			 "onclick=\"setSelected(this);\" "\
			 "class=\"colorSquare\" value=\"&#9899;\" style=\"background-color:%s;\"/>\n",
			 count, current->CouleurPrimaire);
	      else
		snprintf(tmp, 1024, "<input type=\"button\" "\
			 "name=\"buttonColor%d\" "		     \
			 "onclick=\"setSelected(this);\" "\
			 "class=\"colorSquare\" value=\"&#9898;\" style=\"background-color:%s;\"/>\n",
			 count, current->CouleurPrimaire);
	      count += 1;
	    }
	  else
	    {
	      snprintf(tmp, 1024, "<input type=\"button\" "\
			 "name=\"buttonColor%d\" "		     \
		       "onclick=\"setSelected(this);\" "\
		       "class=\"colorSquare\" value=\"&#9747;\" style=\"background-color:%s;\"/>\n",
		       count, current->CouleurPrimaire);
	    }
	  list = addToList(list, strdup(tmp));
	  declinaisons = declinaisons->next;
	}

      /* // hidden values */
      /* //	id (ProduitReference) */
      /* *tmp = '\0'; */
      /* snprintf(tmp, 1024, "<input type='hidden' value=\"%s\" name=\"hiddenId\"/>\n"\ */
      /* 	       "<input type=\"hidden\" value=\"%s\" name=\"hiddenReference\" id=\"hiddenDeclinaison\"/>\n"\ */
      /* 	       "<input type=\"hidden\" value=\"%s\" name=\"hiddenAmount\" id=\"hiddenAmount\"/>\n", */
      /* 	       id, "", ""); */
      /* list = addToList(list, strdup(tmp)); */
      declinaisons = start;
    }

  list = getComposition(list, product);


      

  // Quantité
  *tmp = '\0';
  snprintf(tmp, 1024, "<br/><br/><label for='amount'>Quantité : <br/></label>\n" \
	   "<input type='number' name='hiddenAmount' value='1' onchange=\"updateAmount(this.value)\"/>"\
"<br/><br/>\n");
  list = addToList(list, strdup(tmp));
  

  // Prix
  //	javacript price array generation
  list = addToList(list, "\n<script type=\"text/javascript\">\n");
  list = addToList(list, "//This portion of code is being generated\n");
  if (declinaisons)
    {
      declinaisons = start;
      count = 0;
      list = addToList(list, "var prices = new Array();\n");
      list = addToList(list, "var pricesHT = new Array();\n");
      list = addToList(list, "var references = new Array();\n");
      while (declinaisons && declinaisons->content)
	{
	  char		buffer[4096];

	  current = declinaisons->content;
	  *buffer = '\0';
	  snprintf(buffer, 4096, "prices[%d] = parseFloat(\"%s\");\n"\
		   "pricesHT[%d] = parseFloat(\"%s\");\n"\
		   "references[%d] = \"%s\";\n",
	  	   count, toFloat(current->DeclinaisonPrixTTC),
		   count, toFloat(current->DeclinaisonPrixHT),
		   count, current->DeclinaisonReference);
	  /* snprintf(tmp, 1024, "prices[%d] = \"%d\";\npricesHT[%d] = \"%d\";\n", */
	  /* 	   count, count, count, count * 2); */
	  count += 1;
	  list = addToList(list, strdup(buffer));
	  declinaisons = declinaisons->next;
	}
    }
  else
    {
      list = addToList(list, "var price = 0.0;\n");
      list = addToList(list, "var priceHT = 0.0;\n");
      
	  char		buffer[4096];

	  *buffer = '\0';

	  snprintf(buffer, 4096, "price = parseFloat(\"%s\");\n"\
		   "priceHT = parseFloat(\"%s\");\n",
		   toFloat(product->ProduitPrixTTC),
		   toFloat(product->ProduitPrixHT));
	  list = addToList(list, strdup(buffer));
    }
  list = addToList(list, "</script>\n");


  //	ttc
  *tmp = '\0';
  if (!declinaisons)
    snprintf(tmp, 1024, "<b class=\"price\"><span id='priceSpan'>%s</span>&#x20AC;</b> TTC<br/> " \
	     "(<span id='priceSpanHT'>%s</span>&#x20AC; HT)<br/>\n",
	     product->ProduitPrixTTC, product->ProduitPrixHT);
  else
    {
      declinaisons = start;
      if (current = declinaisons->content)
	snprintf(tmp, 1024, "<b class=\"price\"><span id='priceSpan'>%s</span>&#x20AC;</b> TTC <br/>"\
		 "(<span id='priceSpanHT'>%s</span>&#x20AC; HT)<br/>\n",
		 current->DeclinaisonPrixTTC,  current->DeclinaisonPrixHT);
    }
  list = addToList(list, strdup(tmp));



  list = addToList(list,
		   "<input type='image' name='cartButton' src='images/addToCart.png' "\
		   "class='cart' alt='ajouter au panier'/>");
  list = addToList(list,
		   "<input type='submit' class='cart' value=\"Ajouter au panier\"/>\n");

  list = addToList(list, "</form>\n");
  list = addToList(list, "</span>\n");
  return list;
}



static t_list		*getComposition(t_list *list, t_product *product)
{
  char			request[1024];
  char			buffer[1024];
  t_list		*ret;
  t_list		*tmp;
  t_list		*start;
  t_composition		*current;

  start = NULL;
  product->composition = NULL;
  snprintf(request, 1024,
  	   "SELECT "\
	   "ComposeQuantite, ComposeReference " \
	   "FROM compositions WHERE ProduitReference='%s';", product->id);
  setScript(request);
  if ((ret = execRequest()) && (tmp = ret) && (ret = ret->next) && ret->content)
    {
      while (ret && ret->content)
	{
	  /* printf("RET : '%s'\n", ret->content); */
	  
	  if (!(current = malloc(sizeof(t_composition))))
	    {
	      perror("malloc");
	      exit(FAILURE);
	    }
	  setRequestResult(ret->content);
	  getNextResultValue(&current->ComposeQuantite);
	  getNextResultValue(&current->ComposeReference);
	  setRequestResult(NULL);
	  
	  product->composition = addToList(product->composition, current);
	  ret = ret->next;
	}
      
      list = addToList(list, "<h3>Composition : </h3><br/>\n");
  
      start = product->composition;
      list = addToList(list, "<ul class=\"composition\">\n");
      while (start && start->content)
	{
	  current = start->content;
	  if (current->ComposeReference)
	    {
	      char	*name;

	      *request = '\0';
	      snprintf(request, 1024, "SELECT ProduitNom FROM produits WHERE ProduitReference='%s';",
		       current->ComposeReference);
	      setScript(request);
	      name = execRequestGetValue();
	      *buffer = '\0';
	      snprintf(buffer, 1024, "<li><a href='/produit?id=%s'>%s</a> (x%s)</li>\n",
		       current->ComposeReference, name, current->ComposeQuantite);
	      list = addToList(list, strdup(buffer));
	    }
	  start = start->next;
	}
      list = addToList(list, "</ul>\n");
      destroyList(tmp, TRUE);
    }
  return list;
}

static char		*toFloat(char *str)
{
  char			*start;

  start = str;
  while (str && *str)
    {
      if (*str == ',')
	*str = '.';
      str++;
    }
  return start;
}

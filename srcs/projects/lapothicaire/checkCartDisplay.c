/*                               -*- Mode: C -*- 
 * 
 * Filename: checkCartDisplay.c
 * Description: 
 * Author: Lunacie
 * Created: Tue Apr 12 13:38:37 2016 (+0200)
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



static t_list		*checkCartDisplay(t_list *list, t_list *items)
{
  char			buffer[4096];
  t_product		*current;
  t_list		*start;
  size_t		count;
  float			price;
  float			priceHT;
  size_t		amount;


  *buffer = '\0';

  // cart
  snprintf(buffer, 4096, "<h3>Récapitulatif du panier : </h3>");
  list = addToList(list, strdup(buffer));

  price = 0;
  priceHT = 0;
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
		  snprintf(buffer, 4096, "<li class=\"article\">%s<br/><i>%s</i></li>\n",
			   current->ProduitNom, current->variant);
		}
	      else
		snprintf(buffer, 4096, "<li class=\"article\">%s<br/></li>\n",
			 current->ProduitNom);
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
	      *buffer = '\0';
	      snprintf(buffer, 4096,
		       "<li class='quantite'>x<b> %s</b></li>\n",
		       current->amount);
	      list = addToList(list, strdup(buffer));
	      amount = atoi(current->amount);
	    }
      
	  // Price
	  if (current->ProduitPrixTTC && current->ProduitPrixHT)
	    {
	      *buffer = '\0';
	      snprintf(buffer, 4096, "<li class='prix'><b>%s&#8364;</b><br/>(%s&#8364; HT)</li>\n",
		       current->ProduitPrixTTC, current->ProduitPrixHT);
	      list = addToList(list, strdup(buffer));
	      price += atof(current->ProduitPrixTTC) * amount;
	      priceHT += atof(current->ProduitPrixHT) * amount;
	    }

	  list = addToList(list, "<hr/>\n");   
      
	  list = addToList(list, "</ul>\n");   

	  count += 1;
	}
      items = items->next;
    }

  destroyList(start, FALSE);

  
  list = deliveryDisplay(list, items, &price);


  *buffer = '\0';
  snprintf(buffer, 4096, "<a href=\"/panier\" >Retour</a>\n");
  list = addToList(list, strdup(buffer));


  *buffer = '\0';
  snprintf(buffer, 4096,
	   "<div id=\"total\">"					\
	   "<div id=\"totalBox\">"				\
	   "<b>Total : <span id=\"priceSpan\">%.2f</span>&#8364;</b><br/>" \
	   "Total HT : <span id=\"priceSpanHT\">%.2f</span>&#8364;"	\
	   "</div>"							\
	   "<br/>", price, priceHT);
	   
    list = addToList(list, strdup(buffer));

    list = orderButton(list);
  


  return list;
}

static t_list		*deliveryDisplay(t_list *list, t_list *items, float *price)
{
  char			buffer[4096];

  snprintf(buffer, 4096, "<ul id=\"item_delivery\">\n");
  list = addToList(list, strdup(buffer));

  snprintf(buffer, 4096, "<li class=\"article\"><b>Livraison Colissimo</b><br/>48h</li>\n");
  list = addToList(list, strdup(buffer));
  
     
  *buffer = '\0';
  snprintf(buffer, 4096, "<li class='prix'><b>4,90 € TTC</b><br/></li>\n");
  list = addToList(list, strdup(buffer));
  *price += 4.90;
 
  list = addToList(list, "</ul>\n");   


  return list;
}


static t_list		*orderButton(t_list *list)
{
  char			buffer[4096];

  /* Direct Valid order */
  snprintf(buffer, 4096, 
	   "<form action=\"/commandes.html\" method=\"post\">"				\
	   "<input type=\"hidden\" name=\"add\" value=\"1\"/>" \
	   "<input  type=\"submit\" value=\"Commander\"/>" \
	   "</form>\n");





  /* ============== PAYPAL ============== */

  /* snprintf(buffer, 4096,  */
  /* 	   "<form action=\"https://www.paypal.com/cgi-bin/webscr\" "	\ */
  /* 	   "method=\"post\" target=\"_top\">"				\ */
  /* 	   "<input type=\"hidden\" name=\"cmd\" value=\"_s-xclick\">"	\ */
  /* 	   "<input type=\"hidden\" name=\"hosted_button_id\" value=\"B2ZJEJ3GLBTRY\">" \ */
  /* 	   "<input class=\"paypal\" type=\"image\" "					\ */
  /* 	   "src=\"https://www.paypalobjects.com/fr_FR/FR/i/btn/btn_buynowCC_LG.gif\" " \ */
  /* 	   "border=\"0\" name=\"submit\" alt=\"PayPal, le réflexe sécurité pour payer en ligne\">" \ */
  /* 	   "<img alt=\"\" border=\"0\" src=\"https://www.paypalobjects.com/fr_FR/i/scr/pixel.gif\" "\ */
  /* 	   "width=\"1\" height=\"1\">"					\ */
  /* 	   "</form>\n"); */


  // kit graphique area Button
  /* snprintf(buffer, 4096,  */
  /* 	   "<form action=\"https://www.paypal.com/cgi-bin/webscr\" "	\ */
  /* 	   "method=\"post\" target=\"_top\">"				\ */
  /* 	   "<input type=\"hidden\" name=\"cmd\" value=\"_s-xclick\">"	\ */
  /* 	   "<input type=\"hidden\" name=\"hosted_button_id\" value=\"B2ZJEJ3GLBTRY\">" \ */
  /* 	   "<input class=\"paypal\" type=\"image\" "					\ */
  /* 	   "src=\"https://www.paypalobjects.com/fr_FR/FR/i/btn/btn_buynowCC_LG.gif\" " \ */
  /* 	   "border=\"0\" name=\"submit\" alt=\"PayPal, le réflexe sécurité pour payer en ligne\">" \ */
  /* 	   "<img alt=\"\" border=\"0\" src=\"https://www.paypalobjects.com/fr_FR/i/scr/pixel.gif\" "\ */
  /* 	   "width=\"1\" height=\"1\">"					\ */
  /* 	   "</form>\n"); */


  list = addToList(list, strdup(buffer));
  return list;
}

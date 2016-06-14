/*                               -*- Mode: C -*- 
 * 
 * Filename: getResponse.c
 * Description: 
 * Author: Lunacie
 * Created: Wed Jul  1 16:43:07 2015 (+0200)
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
#include			"projects.h"
#include			"execution.h"
#include			"http/http.h"
#include			"http/html/html.h"

#include			"projects/lapothicaire.h"

t_list				*_getAllowedFileList()
{
  t_list			*list;

  list = NULL;
  //style
  list = addToList(list, (void *)"/style.css");
  list = addToList(list, (void *)"/styles/top-menu.css");
  list = addToList(list, (void *)"/styles/header.css");
  list = addToList(list, (void *)"/styles/menu.css");
  list = addToList(list, (void *)"/styles/member.css");
  list = addToList(list, (void *)"/styles/products.css");
  list = addToList(list, (void *)"/styles/order.css");
  list = addToList(list, (void *)"/styles/panier.css");
  list = addToList(list, (void *)"/styles/product.css");
  list = addToList(list, (void *)"/styles/slideshow.css");
  list = addToList(list, (void *)"/styles/footer.css");

  // scripts
  list = addToList(list, (void *)"/scripts/produits.js");
  list = addToList(list, (void *)"/scripts/panier.js");


  // main
  list = addToList(list, (void *)"/index.html");
  list = addToList(list, (void *)"/member.html");
  list = addToList(list, (void *)"/inscription.html");
  list = addToList(list, (void *)"/validation.html");
  list = addToList(list, (void *)"/categorie.html");
  list = addToList(list, (void *)"/marque.html");
  list = addToList(list, (void *)"/produit.html");
  list = addToList(list, (void *)"/panier.html");
  list = addToList(list, (void *)"/commandes.html");
  //	pages
  list = addToList(list, (void *)"/pages/menu.html");
  list = addToList(list, (void *)"/pages/home.html");
  list = addToList(list, (void *)"/pages/member.html");
  list = addToList(list, (void *)"/pages/inscription.html");
  list = addToList(list, (void *)"/pages/validation.html");
  list = addToList(list, (void *)"/pages/categorie.html");
  list = addToList(list, (void *)"/pages/marque.html");
  list = addToList(list, (void *)"/pages/produit.html");
  list = addToList(list, (void *)"/pages/panier.html");
  list = addToList(list, (void *)"/pages/commandes.html");
  list = addToList(list, (void *)"/pages/ordersDisplay.html");
  //	includes
  list = addToList(list, (void *)"/header.html");
  list = addToList(list, (void *)"/menu.html");
  list = addToList(list, (void *)"/slideshow.html");
  list = addToList(list, (void *)"/fonts.html");
  list = addToList(list, (void *)"/footer.html");

  list = addToList(list, (void *)"/pages/cartHead.html");
  list = addToList(list, (void *)"/pages/cartFoot.html");



  // pictures
  list = addToList(list, (void *)"/images/background.jpg");
  list = addToList(list, (void *)"/images/body-top.jpg");
  list = addToList(list, (void *)"/images/logo.png");
  list = addToList(list, (void *)"/images/addToCart.png");
  list = addToList(list, (void *)"/images/order.png");
  list = addToList(list, (void *)"/images/account.png");
  list = addToList(list, (void *)"/images/disconnect.png");
  list = addToList(list, (void *)"/images/search.png");
  list = addToList(list, (void *)"/images/search_icon.png");
  list = addToList(list, (void *)"/images/buy.png");
  list = addToList(list, (void *)"/images/delete.png");
  list = addToList(list, (void *)"/images/member.png");
  list = addToList(list, (void *)"/images/overlay.png");
  list = addToList(list, (void *)"/images/menu_background.png");
  list = addToList(list, (void *)"/images/menu_open-bottom.png");
  list = addToList(list, (void *)"/images/menu_open-repeat.png");
  list = addToList(list, (void *)"/images/menu_open-top.png");
  list = addToList(list, (void *)"/images/next.png");
  list = addToList(list, (void *)"/images/no-image.png");
  list = addToList(list, (void *)"/images/no-image.jpg");
  list = addToList(list, (void *)"/images/prev.png");
  list = addToList(list, (void *)"/images/select.png");
  list = addToList(list, (void *)"/images/separator.png");
  list = addToList(list, (void *)"/images/slider_overlay.png");
  list = addToList(list, (void *)"/images/available.png");
  list = addToList(list, (void *)"/images/notAvailable.png");
  list = addToList(list, (void *)"/images/buyNext.png");

  list = addToList(list, (void *)"/images/slideshow/01.jpg");
  return list;
}

#define				NB_REDIRECTIONS 10
t_urlRewrite			redirections[NB_REDIRECTIONS] = 
  {
    {NULL, "/index.html", 0},
    {"/\0", "/index.html", 2},
    {"/member\0", "/member.html", 9 + 1},
    {"/inscription\0", "/inscription.html", 14 + 1},
    {"/validation\0", "/validation.html", 13 + 1},
    {"/categories\0", "/categorie.html", 12 + 1},
    {"/marques\0", "/marque.html", 10 + 1},
    {"/produit\0", "/produit.html", 10 + 1},
    {"/panier\0", "/panier.html", 9 + 1},
    {"/commandes\0", "/commandes.html", 14 + 1},
  };

t_list				*_getResponse(t_socket *client)
{
  t_list		*list = NULL;
  size_t		x;

  client->request->_project = LAPOTHICAIRE;

  if (client->request->_POST)
    client->request->_GET = client->request->_POST;


  // Url rewriting
  for (x = 0; x < NB_REDIRECTIONS; x++)
    {
      if (redirections[x].original == client->request->_GET ||
	  (
	    redirections[x].original &&
	    !strncmp(redirections[x].original,
		     client->request->_GET, redirections[x].len)
	  ))
	{
	  printf("Redirecting '%s' -> '%s'\n",
		 redirections[x].original, redirections[x].redirect);
	  free(client->request->_GET);
	  client->request->_GET = strdup(redirections[x].redirect);
	}
    }

  if (!(list = getPage(client, client->request->_GET, TRUE)))
    return NULL;
  if (client->request->_isBinary == FALSE)
    {
      if (!(list = cutPage(client, list)))
	return NULL;
    }
  return list;
}

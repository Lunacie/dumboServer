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
#include			"crypto.h"

#include			"projects/lapothicaire.h"

static t_list		*membersArea(t_list **script, t_socket *client)
{
  t_list		*list;
  t_list		*ret;

  char			*tmp;
  size_t		len;
  t_list		*subMenu;
  char			*action;
  char			*password;
  char			*encodedPassword;
  int			errorCode;
  t_list		*parameters;

  action = NULL;
  list = NULL;  
  setDatabase("lapothicaire");
  setSessionCookiesNames("lapothicaireSESSIONID",
			 "lapothicaireSESSIONusername");

  parameters = NULL;
  action = getParameter(client, "action");
  if (password = getParameter(client, "password"))
    parameters = client->request->_parameters->list;
  if (password && parameters)
    {
      encodedPassword = stringToSHA1(password);
      printf("\t\tEncoding password to SHA1 =>\n");
      password = setParameterValue(parameters, password, encodedPassword);
    }
  

  printf("WHICH ACTION ? action='%s'\n", action);
  // connected -> disconnect
  if (action && !strncmp(action, "disconnect", strlen("disconnect")))
    {
      printf("Action == disconnect\n");
      disconnect(script, client);
      list = addToList(list, "<p>Déconnecté avec succès.</p>");
    }
  // connected -> action account (gestion)
  else if (action && !strncmp(action, "account", strlen("account")))
    {
      printf("Action == Gestion compte\n");
      /* disconnect(script, client); */
      list = handleAccount(list, client);
    }

  // connected DISPLAY MEMBERS AREA
  else if (authentification(script, client))
	{
	  printf("NO ACTION ? \n");
	  list = addToList(NULL, 
			   htmlLinkImage("/commandes",
					 "images/order.png", "Vos commandes"));
	  list = addToList(list, "<a href=\"commandes\">Vos commandes</a><br/>");

	  list = addToList(list, 
			   htmlLinkImage("/member?action=account", "images/account.png",
					 "Gestion de votre compte"));
	  list = addToList(list, "<a href=\"/member?action=account\">Gestion de votre compte</a><br/>");

	  list = addToList(list, 
			   htmlLinkImage("/member?action=disconnect", "images/disconnect.png",
					 "Se déconnecter"));
	  list = addToList(list, "<a href=\"/member?action=disconnect\">Se déconnecter</a><br/>");
	}

  // Not connected -> form
  else
    {
      errorCode = getAuthErrorCode(0);
      if (errorCode == 403)
      list = addToList(list, "<b>Erreur lors de l'authentification : </b>Mauvais mot de passe ou adresse mail.<br/>");
      
      list = addToList(list, "<form action=\"#\" method=\"post\">");
      list = addToList(list, "<label for=\"username\"/>Adresse mail : </label>");
      list = addToList(list, "<input type=\"text\" name=\"username\"/><br/>");
      list = addToList(list, "<label for=\"password\"/>Mot de passe : </label>");
      list = addToList(list, "<input type=\"password\" name=\"password\"/>");
      list = addToList(list, "<input type=\"submit\" value=\"Connexion\"/>");
      list = addToList(list, "</form>");

      list = addToList(list, "<p>Pas de compte ? <a href='./inscription'>Inscrivez-vous !</a></p>");

    }
  return commitResult(list, script);
}



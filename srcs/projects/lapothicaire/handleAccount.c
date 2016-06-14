/*                               -*- Mode: C -*- 
 * 
 * Filename: handleAccount.c
 * Description: 
 * Author: Lunacie
 * Created: Mon Jan 11 23:06:12 2016 (+0100)
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
#include			"crypto.h"

#include			"projects/lapothicaire.h"


static char			*confirmPassword;
static char			*currentUsername;
static char			*pending;
static char			*deleteAccount;
static char			*confirmDeletion;

t_list				*handleAccount(t_list *list, t_socket *client)
{
  t_list		*ret;

  char			*tmp;
  size_t		len;
  char			*action;

  t_list		*parameters;
  char			*paramNames;

  t_bool		ready;
  char			*message;

  /* if (!(username = getCookie(client->request, "lapothicaireSESSIONusername"))) */
  /*   { */
  /*     printf("Missing username\n"); */
  /*     return list; */
  /*   } */
  confirmDeletion = NULL;
  if (!(currentUsername = getCookie(client->request, "lapothicaireSESSIONusername")))
    {
      printf("Missing username\n");
      return list;
    }

  ready = FALSE;
  action = NULL;

  message = strdup("<p>Pour modifier les informations de votre  compte, " \
		   "remplissez le formulaire ci-dessous : </p>");
  list = addToList(list, message);
      
  paramNames = "newUsername:newPassword:newPassword2:nom:prenom"\
    ":adresse:adresse2:confirmPassword:deleteAccount:confirmDeletion";
  parameters = generateEmptyParameterListFromNames(paramNames, ':');
  parameters = fillParameterList(client, parameters);
  
  if (parameterIsDefined(parameters, ANY))
    list = checkHandleAccountForm(list, parameters, &ready);
  if (ready)
    {
      /* list = popList(list, FALSE); */
      *message = '\0';
      list = editAccount(list, parameters, &ready);
      if (ready)
	disconnect(&list, client);
      /* 	list = sendValidationMail(list, &ready); */
    }
  else
    list = displayHandleAccountForm(list, parameters);
  destroyList(parameters, TRUE);
  /* return commitResult(list, script); */

  return list;
}

static t_list		*checkHandleAccountForm(t_list *list,
					  t_list *parameters, t_bool *ready)
{
  t_bool		match;

  match = FALSE;
  newUsername = getParameterValueFromList(parameters, "newUsername");
  nom = getParameterValueFromList(parameters, "nom");
  prenom = getParameterValueFromList(parameters, "prenom");
  adresse = getParameterValueFromList(parameters, "adresse");
  password = getParameterValueFromList(parameters, "newPassword");
  password2 = getParameterValueFromList(parameters, "newPassword2");
  confirmPassword = getParameterValueFromList(parameters, "confirmPassword");
  deleteAccount = getParameterValueFromList(parameters, "deleteAccount");

  if (((password && password2) || deleteAccount || newUsername || nom || prenom || adresse) && confirmPassword)
    {
      encodedPassword = stringToSHA1(confirmPassword);
      printf("\t\tEncoding password to SHA1 =>\n");
      confirmPassword = setParameterValue(parameters, confirmPassword, encodedPassword);

      if ((password && password2) && stringsAreTheSame(password, password2))
	{
	  match = TRUE;
	  encodedPassword = stringToSHA1(password);
	  printf("\t\tEncoding password to SHA1 =>\n");
	  password = setParameterValue(parameters, password, encodedPassword);
	}
      else if (!password || (password && strlen(password) == 0))
	match = TRUE;
      
      if (newUsername && mailOK(newUsername))
	*ready = TRUE;
      else if (deleteAccount)
	*ready = TRUE;
      else if (newUsername)
	list = addToList(list, "<b>Cette adresse mail est déjà utilisée."\
			 "<br/></b>");
    }
  else
    list = addToList(list, "<b>Merci de remplir tous les champs obligatoires marqués d'une '*'.<br/></b>");
  if (match == FALSE)
    list = addToList(list,
		     "<b>Les mots de passe ne correspondent pas.<br/></b>"); 
  return list;
}


static t_list		*editAccount(t_list *list, t_list *parameters,
				     t_bool *ready)
{
  char			request[4096];
  char			buffer[1024];
  char			*strValues;
  t_list		*values;
  t_list		*ret;
  t_list		*tmp;


  if (!currentUsername)
    {
      list = addToList(list, "<b>Vous n'êtes pas authentifié.</b><br/>");
      printf("Missing current username cookie; returning\n");
      return list;
    }


  setDatabase("lapothicaire");


  // -- delete account
  if (deleteAccount && strlen(deleteAccount))
    {
      confirmDeletion = getParameterValueFromList(parameters, "confirmDeletion");
      printf("CONFIRM : %s\n", confirmDeletion);
      if (confirmDeletion && strlen(confirmDeletion))
	{
	  snprintf(request, 4096,
		   "DELETE FROM users WHERE username='%s' AND password='%s' LIMIT 1; SELECT ROW_COUNT();",
		   currentUsername, confirmPassword);
	  setScript(request);

	  /* printf("CMD = %s\n", request); */
	  if ((ret = execRequest()))
	    {
	      tmp = ret;
	      if ((ret = ret->next) && ret->content &&
	  	  !strncmp(ret->content, "1", 1))
	  	{
	  	  destroyList(tmp, TRUE);
	  	  *ready = TRUE;
	  	  list = addToList(list, "<p>Votre compte a été supprimé avec succès.</p>");
	  	  printf("Account deleted\n");
	  	  return list;
	  	}
	      destroyList(tmp, TRUE);
	    }
	  list = addToList(list, "<b>Une erreur est survenue à la supression "	\
	  		   "de votre compte. Veuillez réessayer ultérieurement." \
	  		   "<br/></b>");
	}
      else
	list = displayDeleteAccountForm(list, parameters);
      return list;
    }
  // delete account --
  

  *ready = FALSE;
  if (!(key = getUniqueId()))
    {
      list = addToList(list, "<b>Une erreur est survenue à la "	\
		       "génération de votre clé de validation. "	\
		       "Veuillez réitérer ultérieurement.</b><br/>");
      return list;
    }

  values = NULL;
  if (newUsername && strlen(newUsername))
    {
      snprintf(buffer, 1024, " username='%s'", newUsername);
      values = addToList(values, strdup(buffer));
    }
  if (password && strlen(password))
    {
      if (values)
	values = addToList(values, strdup(","));
      snprintf(buffer, 1024, " password='%s'", password);
      values = addToList(values, strdup(buffer));
    }
  if (nom && strlen(nom))
    {
      if (values)
	values = addToList(values, strdup(","));
      snprintf(buffer, 1024, " nom='%s'", nom);
      values = addToList(values, strdup(buffer));
    }
  if (prenom && strlen(prenom))
    {
      if (values)
	values = addToList(values, strdup(","));
      snprintf(buffer, 1024, " prenom='%s'", prenom);
      values = addToList(values, strdup(buffer));
    }
  if (adresse && strlen(adresse))
    {
      if (values)
	values = addToList(values, strdup(","));
      snprintf(buffer, 1024, " adresse='%s'", adresse);
      values = addToList(values, strdup(buffer));
    }
  if (values)
    {
      if (!(strValues = listToString(values)))
	{
	  destroyList(values, TRUE);
	  list = addToList(list, "<b>Echec de la modification</b><br/>");
	  printf("Missing Could not turn list to string; returning\n");
	  return list;
	}
    }
  snprintf(request, 4096,
	   "UPDATE users SET %s WHERE username='%s' AND password='%s'; SELECT ROW_COUNT();",
	   strValues, currentUsername, confirmPassword);

  destroyList(values, TRUE);
  free(strValues);

  setScript(request);

  /* printf("CMD = %s\n", request); */
  if ((ret = execRequest()))
    {
      tmp = ret;
      if ((ret = ret->next) && ret->content &&
  	  !strncmp(ret->content, "1", 1))
  	{
  	  destroyList(tmp, TRUE);
  	  *ready = TRUE;
	  list = addToList(list, "<p>Votre compte a été modifié avec succès.</p>");
	  printf("Modifications apportées\n");
  	  return list;
  	}
      destroyList(tmp, TRUE);
    }
  list = addToList(list, "<b>Une erreur est survenue à la modification "	\
  		   "de votre compte. Veuillez réessayer ultérieurement." \
  		   "<br/></b>");

  return list;
}


static t_list		*displayHandleAccountForm(t_list *list, t_list *parameters)
{
  list = addToList(list, "<form action=\"/member?action=account\" method=\"post\">");
  list = htmlInput(list, "newUsername", "Adresse e-mail : ",
		   getParameterValueFromList(parameters, "newUsername"),
		   "text", "<br/>");

  list = htmlInput(list, "newPassword", "Nouveau mot de passe : ",
		   NULL,
		   "password", "<br/>");

  list = htmlInput(list, "newPassword2", "Confirmation nouveau mot de passe : ",
		   NULL,
		   "password", "<br/>");
 
  list = htmlInput(list, "nom", "Nom : ",
		   getParameterValueFromList(parameters, "nom"),
		   "text", "<br/>");

  list = htmlInput(list, "prenom", "Prénom : ",
		   getParameterValueFromList(parameters, "prenom"),
		   "text", "<br/>");
      
  list = htmlTextArea(list, "adresse", "Adresse : ",
		   getParameterValueFromList(parameters, "adresse"),
		   "<br/>");
      
  list = htmlInput(list, "deleteAccount", "<b>Supprimer mon compte : </b>",
		   NULL,
		   "checkbox", "<br/>");
  list = htmlInput(list, "confirmPassword", "Mot de passe actuel pour vérification *: ",
		   NULL,
		   "password", "<br/>");
  list = addToList(list, "<input type=\"submit\" value=\"Valider\"/>");
  list = addToList(list, "</form>");

  return list;
}


static t_list		*displayDeleteAccountForm(t_list *list, t_list *parameters)
{
  list = addToList(list, "<p>Vous avez demandé la supression de votre compte. "	\
		   "Est ce bien ce que vous souhaitez faire ?</p>");

  list = addToList(list, "<form action=\"/member?action=account\" method=\"post\">");

  list = htmlInput(list, "deleteAccount", NULL,
		   "DELETE",
		   "hidden", NULL);
  list = htmlInput(list, "confirmDeletion", NULL,
		   "DELETE",
		   "hidden", NULL);
  list = htmlInput(list, "confirmPassword", "Confirmez votre mot de passe : ",
		   NULL,
		   "password", "<br/>");
  list = addToList(list, "<input type=\"submit\" value=\"Valider\"/>");
  list = addToList(list, "</form>");

  return list;
}

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

static char			*key;
static char			*password;
static char			*password2;
static char			*encodedPassword;
static char			*newUsername;
static char			*nom;
static char			*prenom;
static char			*adresse;

static t_list		*createAccount(t_list **script, t_socket *client)
{
  t_list		*list;
  t_list		*ret;

  char			*tmp;
  size_t		len;
  t_list		*subMenu;
  char			*action;

  t_list		*parameters;
  char			*paramNames;

  t_bool		ready;
  char			*message;


  list = NULL;  
  ready = FALSE;
  action = NULL;

  message = strdup("<p>Pour procéder à la création d'un compte, " \
		   "remplissez le formulaire ci-dessous : </p>");
  list = addToList(list, message);
      
  paramNames = "newUsername:newPassword:newPassword2:nom:prenom"\
    ":adresse:adresse2";
  parameters = generateEmptyParameterListFromNames(paramNames, ':');
  parameters = fillParameterList(client, parameters);
  
  if (parameterIsDefined(parameters, ANY))
    list = checkAccountForm(list, parameters, &ready);
  if (ready)
    {
      /* list = popList(list, FALSE); */
      *message = '\0';
      list = addNewAccount(list, parameters, &ready);
      if (ready)
	list = sendValidationMail(list, &ready);
    }
  else
    list = displayAccountForm(list, parameters);
  destroyList(parameters, TRUE);
  return commitResult(list, script);
}


static t_list		*sendValidationMail(t_list *list, t_bool *ready)
{
  char			mail[1024];
  char			link[512];
  char			cmd[4096];
  char			tmp[1024 + 100];
  FILE			*fp;
  char			letter;
  char			subject[1024];

  snprintf(link, 512, 
	   "http://lapothicaire-vapeshop.com/validation?id=%s&user=%s",key, newUsername);

  snprintf(mail, 1024,
	   "Bonjour,\n\n"\
	   "Votre création de compte sur l'Apothicaire Vapeshop s'est déroulée avec succès.\n"\
	   "Veuillez visiter ce lien pour procéder à la validation de votre adresse mail.\n"\
	   "%s\n"\
	   "Cordialement,\n"\
	   "L'apothicaire, Cigarette éléctroniques, Accessoires et e-liquides.\n", link);

  snprintf(subject, 1024, "Vérification de votre adresse mail");

  snprintf(cmd, 4096, "./bin/mail.sh \"%s\" \"%s\" \"%s\"", newUsername, mail, subject);
  /* if (list) */
  /*     popList(list, FALSE); */
  printf("Mail cmd : %s", cmd);
  if (access(".dev", F_OK) == FAILURE)
    {
      // ONLINE VERSION
      if (!(fp = popen(cmd, "r")))
	return (t_list *)handle_error("popen", (long int)NULL);
      letter = '\0';
      if (fgets(&letter, 1, fp) && letter == '\0')
	snprintf(tmp, 1124, "<p>Un mail de vérification a été envoyé à votre adresse : <b>%s<b></p>",
		 newUsername);
      else
	snprintf(tmp, 1124, "<p>Le mail de vérification n'a pas pu être envoyé.</p>");
      pclose(fp);
    }
  else
    {
      // DEV VERSION
      snprintf(tmp, 1124,  "<p>On the Online version, the following link would have had been sent to you : \n"\
	      "%s</p>", link);
    }
  list = addToList(list, strdup(tmp));
  return list;
}

static t_list		*addNewAccount(t_list *list, t_list *parameters,
				       t_bool *ready)
{
  char			request[4096];
  t_list		*ret;
  t_list		*tmp;

  *ready = FALSE;
  if (!(key = getUniqueId()))
    {
      list = addToList(list, "<b>Une erreur est survenue à la "	\
		       "création de votre clé de validation. "	\
		       "Veuillez réitérer ultérieurement.</b><br/>");
      return list;
    }
  setDatabase("lapothicaire");
  snprintf(request, 4094, "INSERT INTO users "	\
  	   "(username, password, validationKey, nom, prenom, adresse) "\
	   "VALUES ('%s', '%s', '%s', '%s', '%s', '%s'); SELECT ROW_COUNT();",
	   newUsername, password, key, nom, prenom, adresse);
  setScript(request);
  if ((ret = execRequest()))
    {
      tmp = ret;
      if ((ret = ret->next) && ret->content &&
	  !strncmp(ret->content, "1", 1))
	{
	  destroyList(tmp, TRUE);
	  *ready = TRUE;
	  return list;
	}
      destroyList(tmp, TRUE);
    }
  list = addToList(list, "<b>Une erreur est survenue à la création "	\
		   "de votre compte. Veuillez réessayer ultérieurement." \
		   "<br/></b>");
  return list;
}


static t_list		*checkAccountForm(t_list *list,
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

    if (password && password2 && newUsername && nom && prenom && adresse)
    {
      if (stringsAreTheSame(password, password2))
	match = TRUE;
      encodedPassword = stringToSHA1(password);
      printf("\t\tEncoding password to SHA1 =>\n");
      password = setParameterValue(parameters, password, encodedPassword);
      if (mailOK(newUsername))
	*ready = TRUE;
      else
	list = addToList(list, "<b>Cette adresse mail est déjà utilisée."\
			 "<br/></b>");
    }
  else
    list = addToList(list, "<b>Merci de remplir tous les champs.<br/></b>");
  if (match == FALSE)
    list = addToList(list,
		     "<b>Les mots de passe ne correspondent pas.<br/></b>"); 
  return list;
}

static t_bool		mailOK(char *mail)
{
  char			request[1024];

  setDatabase("lapothicaire");
  snprintf(request, 1024, "SELECT id FROM users WHERE username='%s';", mail);
  setScript(request);
  if (execRequest())
    return FALSE;
  return TRUE;
}


static t_list		*displayAccountForm(t_list *list, t_list *parameters)
{
  list = addToList(list, "<form action=\"#\" method=\"post\">");
  list = htmlInput(list, "newUsername", "Adresse e-mail : ",
		   getParameterValueFromList(parameters, "newUsername"),
		   "text", "<br/>");

  list = htmlInput(list, "newPassword", "Mot de passe : ",
		   NULL,
		   "password", "<br/>");

  list = htmlInput(list, "newPassword2", "Confirmation mot de passe : ",
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
  
      
  list = addToList(list, "<input type=\"submit\" value=\"Valider\"/>");
  list = addToList(list, "</form>");

  return list;
}

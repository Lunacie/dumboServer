/*                               -*- Mode: C -*- 
 * 
 * Filename: productSearch.c
 * Description: 
 * Author: Lunacie
 * Created: Thu Dec 17 16:25:39 2015 (+0100)
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
#include			"projects.h"
#include			"http/http.h"
#include			"http/html/html.h"
#include			"databases.h"

#include			"projects/lapothicaire.h"

static t_list			*categories;
static t_list			*marques;


static t_list		*printCategoryName(t_list **script, t_socket *client)
{
  t_list		*list;
  char			request[1024];
  t_list		*ret;
  t_list		*tmp;

  char			*id;

  list = NULL;
  if (!script || !*script)
    return NULL;
  
  printf("\tprintCategoryName :: \n");
  
  if (!(id = getParameter(client, "id")))
    {
      printf("Missing id parameter\n");
      list = addToList(list, "");
      return commitResult(list, script);
    }
  snprintf(request, 1024,
	   "SELECT CategorieNom FROM categories WHERE CategorieId='%s' LIMIT 1;", id);
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


static t_list		*printBrandName(t_list **script, t_socket *client)
{
  t_list		*list;
  char			request[1024];
  t_list		*ret;
  t_list		*tmp;

  char			*id;

  list = NULL;
  if (!script || !*script)
    return NULL;
  
  printf("\tprintBrandName :: \n");
  
  if (!(id = getParameter(client, "id")))
    {
      printf("Missing id parameter\n");
      list = addToList(list, "");
      return commitResult(list, script);
    }
  snprintf(request, 1024,
	   "SELECT MarqueNom FROM  marques WHERE MarqueId='%s' LIMIT 1;", id);
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



/*
 **
 */ 

static t_list		*searchProduct(t_list **script, t_socket *client)
{
  t_list		*list;
  char			request[1024];
  t_list		*ret;

  char			*tmp;
  char			*func;

  char			*categorie;
  char			*marque;
  char			*id_str;
  char			*amount_str;
  char			*sort_str;

  t_list		*tmpList;
  int			rc;

  list = NULL;
  if (!script || !*script)
    return NULL;
  
  tmpList = *script;
  func = (*script)->content;
  printf("\tSearchProduct :: '%s'\n", func);

  categorie = NULL;
  id_str = NULL;
  amount_str= NULL;
  sort_str= NULL;
  rc = sscanf(func, "searchProduct(%m[^,], %m[^,], %m[^,], %m[^,], %m[^)])",
	      &categorie, &marque, &id_str, &amount_str, &sort_str);
  printf("\tFound parameters :\n");
  printf("\t\t categorie = '%s'\n", categorie);
  printf("\t\t marque = '%s'\n", marque);
  printf("\t\t id = '%s'\n", id_str);
  printf("\t\t amount = '%s'\n", amount_str);
  printf("\t\t sort = '%s'\n", sort_str);
  if (rc == 5 && categorie && marque && id_str && amount_str && sort_str)
    {
      printf("passed sscanf\n");

      setDatabaseSystem(MYSQL);
      setDatabase("lapothicaire");
      list = startProductSearch(categorie, marque, id_str, amount_str, sort_str, client);
      return commitResult(list, script);
    }
  else
    {
      list = addToList(list, "[?c Script ");
      list = addToList(list, USAGE_searchProduct);
      list = addToList(list, ": Missing Parameter(s) ?]\n");
      return commitResult(list, script);
    }
  
  /* setDatabase("lapothicaire"); */

  

  /* list = getSubMenu(1, 0, NULL); */
  return commitResult(list, script);
}




t_list			*startProductSearch(char * categorie, char *marque, 
					    char *id_str, char *amount_str, char *sort_str, 
					    t_socket *client)
{
  char			request[1024];
  char			tmp[512];
  t_list		*list;
  t_list		*res;
  t_list		*treeRes;

  char			*categoryId;
  char			*brandId;

  categories = getElementListId("categories", "CategorieNom", "CategorieId");

  printf("startProductSearch #1\n");
  marques = getElementListId("marques", "MarqueNom", "MarqueId");
  *request = '\0';
  printf("startProductSearch #2\n");
  strcat(request, "SELECT ProduitReference FROM produits WHERE ");
  if (categorie)
    {
      // Category :: all
      if (!(strncmp(categorie, "ALL", 3)) || !(strncmp(categorie, "ANY", 3)))
	strcat(request, "1 AND ");
      // Categorie :: id
      else if (categoryId = getParameter(client, categorie))
	{
	  t_list		*element;
	  ListId		*current;
	  size_t		count;
	  t_list		*first;

	  // categorie has children
	  printf("Parrent tree : \n");

	  element = categories;
	  while (element)
	    {
	      if (element->content && (current = element->content) &&
		  current->id == atoi(categoryId))
		{
		  snprintf(tmp, 64, "CategorieId='%d' ", current->id);
		  strcat(request, tmp);
		  *tmp = '\0';


		  treeRes = categoriesChildren(current->child, NULL);
		  first = treeRes;
		  while (treeRes && treeRes->content)
		    {
		      /* printf("RESULT : '%s'\n", treeRes->content); */
		      
		      strcat(request, treeRes->content);
		      treeRes = treeRes->next;
		    }
		  destroyList(first, TRUE);
		  strcat(request, " AND ");
		  break;
		}
	      element = element->next;
	    }
	}
      // Categorie :: name
      else
	{
	  snprintf(tmp, 64, "CategorieNom='%s' AND ", categorie);
	  strcat(request, tmp);
	  *tmp = '\0';
	}
    }


  printf("startProductSearch #3\n");



  if (marque)
    {
      // Category :: all
      if (!(strncmp(marque, "ANY", 3)))
	strcat(request, "1 AND ");
      // Marque :: id
      else if (brandId = getParameter(client, marque))
	{
	  snprintf(tmp, 64, "MarqueId='%s' AND ", brandId);
	  strcat(request, tmp);
	  *tmp = '\0';
	}
      // Marque :: name
      else
	{
	  snprintf(tmp, 64, "MarqueNom='%s' AND ", marque);
	  strcat(request, tmp);
	  *tmp = '\0';
	}
    }



  printf("startProductSearch #4\n");



  if (id_str)
    {
      // id :: Any/All
      if (!(strncmp(id_str, "ANY", 3)) || !(strncmp(id_str, "ALL", 3)))
	strcat(request, "1 ");
      // id :: specific nb
      else
	{
	  snprintf(tmp, 64, "ProduitReference=%s ", id_str);
	  strcat(request, tmp);
	  *tmp = '\0';
	}
    }
  if (sort_str)
    {
      if (!(strncmp(sort_str, "NONE", 4)));
      else
	{
	  snprintf(tmp, 64, " ORDER BY %s ", sort_str);
	  strcat(request, tmp);
	  *tmp = '\0';
	}
    }
  if (amount_str)
    {
      if (!(strncmp(amount_str, "ALL", 3)));
      else
	{
	  snprintf(tmp, 64, " LIMIT %s ", amount_str);
	  strcat(request, tmp);
	  *tmp = '\0';
	}
    }


  printf("startProductSearch #5\n");

  strcat(request, ";");
  printf("\t Request :: '%s'\n", request);

  setScript(request);
  if ((list = execRequest()) && (list = list->next));
  /* printf("HERE : %s\n", list->content); */
  else
    /* if (!list || !list->content) */
    {
      printf("no result\n");
      if (list)
	destroyList(list, TRUE);
      list = NULL;
      list = addToList(list, "<p>Aucun résultat pour cette rubrique.</p>");
      return list;
    }


  printf("startProductSearch #6\n");
  return displayProducts(list);
}

t_list			*categoriesChildren(t_list *list, t_list *result)
{
  ListId		*current;
  char			tmp[64];

  // child list elements
  while (list && (current = list->content))
    {
	  /* if (id == 0) */
	  /*   id = current->id; */
	  /* /\* printf("sibbling level : %s\n", current->content); *\/ */
	  /* if (list->content && (current->id == id || current->parentId == id )) */
	  /*   { */
      snprintf(tmp, 64, "OR CategorieId='%d' ", current->id);
      result = addToList(result, strdup(tmp));
      
      /* printf("\ttree level : %s\n", current->content); */
      
      /*     // explore child for child; */
      if (current->child)
	result = categoriesChildren(current->child, result);
	  /*   } */
	  // looking next sibbling
      list = list->next;
    }
  return result;
}

t_list			*displayProducts(t_list *idList)
{
  t_list		*list;
  t_list		*res;
  t_list		*tmpList;
  char			request[1024];
  char			*id;


  if (!marques || !categories)
    return NULL;

  tmpList = idList;
  list = NULL;
  while (idList)
    {
      if (id = ((char *)(idList->content)))
	{
	  char		*name = NULL;
	  char		*price = NULL;
	  char		*image = NULL;
	  char		*marque;
	  char		*category;

	  char		*categorieId = NULL;
	  char		*marqueId = NULL;
	  
	  if (id[strlen(id) - 1] == '\n')
	    id[strlen(id) - 1] = '\0';
	  *request = '\0';
	  snprintf(request, 1024,
		   "SELECT ProduitNom, ProduitPrixTTC, MarqueId, CategorieId, ProduitPrixTTC " \
		   "FROM produits "\
		   "WHERE ProduitReference='%s';", id);
	  setScript(request);
	  if (res = execRequest())
	    {
	      res = res->next;
	      if (res->content)
		{
		  /* printf("PARSED : '%s'\n", res->content); */
		  if  (sscanf(res->content, "%m[^\t]\t%m[^\t]\t%m[^\t]\t%m[^\t]\t%m[^\n]\n",
		  		   &name, &price, &marqueId, &categorieId, &price) > 0)
		    {
		      if (marqueId && marques)
			marque = getNameAtIdFromList(marqueId, marques);
		      if (categorieId && categories)
			category = getNameAtIdFromList(categorieId, categories);
		      
		      if (marque && category)
			{
			  char		tmp[1024];

			  printf("\t\tnom='%s'\n", name);
			  /* printf("\t\tdescription='%s'\n", description); */
			  printf("\t\tprice='%s'\n", price);
			  printf("\t\tmarqueId='%s' (%s)\n", marque, marqueId);
			  printf("\t\tcategorieId='%s' (%s)\n", category, categorieId);
			  printf("\n");

			  list = addToList(list, "<span class=\"product\">");


			  *tmp = '\0';
			  snprintf(tmp, 1024, "<a href=\"./produit?id=%s\"><img src=\"%s\"/>",
				   id,  "images/no-image.png");
			  list = addToList(list, strdup(tmp));

			  *tmp = '\0';
			  snprintf(tmp, 1024, "<strong>%s</strong>",
				   name);
			  list = addToList(list, strdup(tmp));
			  
			  *tmp = '\0';
			  snprintf(tmp, 1024,"<p>%s&#8364;</p></a>", price/* description */);
			  list = addToList(list, strdup(tmp));
			  
			  
			  *tmp = '\0';
			  snprintf(tmp, 1024, "<a href=\"./categories?id=%s\" class=\"category\">%s</a>",
				   categorieId, category);
			  list = addToList(list, strdup(tmp));
			  
			  
			  *tmp = '\0';
			  snprintf(tmp, 1024, "<a href=\"./marques?id=%s\" class=\"brand\">%s</a>",
				   marqueId, marque);
			  list = addToList(list, strdup(tmp));
			  

			  list = addToList(list, "</span>");
			}
		    }
		}
	      destroyList(res, TRUE);
	    }
	  else
	    {
	      destroyList(categories, TRUE);
	      destroyList(marques, TRUE);
	      return list;
	    }
	}
      idList = idList->next;
    }
  if (tmpList)
    destroyList(tmpList, TRUE);
  destroyList(categories, TRUE);
  destroyList(marques, TRUE);
  return list;
}


char			*getNameAtIdFromList(char *idStr, t_list *list)
{
  int			id;
  ListId		*element;

  id = atoi(idStr);
  while (list)
    {
      if (element = ((ListId*)(list->content)))
	{
	  /* printf("COMPARING %d to %d\n", id, element->id); */
	  if (id == element->id)
	    {
	      /* printf("element : %s\n", element->content); */
	      return element->content;
	    }
	}
      list = list->next;
    }
  return NULL;
}


t_list			*getElementListId(char *table, char *nameVariable, char *idVariable)
{
  t_list		*list;
  t_list		*ret;
  char			request[1024];
  t_list		*tmp;
  ListId		*element;
  t_bool		isCategory;

  list = NULL;
  *request = '\0';
  if (stringsAreTheSame(table, "categories"))
    {
      snprintf(request, 1024, "SELECT %s, CategorieIdParent, %s FROM %s;", nameVariable, idVariable, table);
      isCategory = TRUE;
    }
  else
    {
      snprintf(request, 1024, "SELECT %s, %s FROM %s;", nameVariable, idVariable, table);
      isCategory = FALSE;
    }
  setScript(request);
  if (ret = execRequest())
    {
      tmp = ret;
      ret = ret->next;
      while (ret)
	{
	  if (ret->content)
	    {
	      /* printf("RET : '%s'\n", ret->content); */
	      if (!(element = malloc(sizeof(ListId))))
		{
		  perror("malloc");
		  exit(FAILURE);
		}
	      element->parent = NULL;
	      element->child = NULL;

	      char	*current;
	      char	*temp;
	      size_t	count;
	      
	      current = ret->content;
	      count = strlen(current) - 1;
	      while (count >= 0)
		{
		  if (current[count] == '\n')
		    current[count] = '\0';
		  else if (current[count] == '\t' || current[count] == ' ')
		    {
		      if (temp = strdup(&(current[count + 1])))
			{
			  /* printf("temp : '%s'\n", temp); */
			  element->id = atoi(temp);
			  free(temp);
			  /* current[count] = '\0'; */
			  /* if (element->content = strdup(current)) */
			  /*   { */
			      
			  /*   } */
			  if (!isCategory)
			    break;
			  else
			    {
			      while (count >= 0 && (current[count] == ' ' || current[count] == '\t'))
				count--;
			      while (count >= 0 && current[count] != ' ' && current[count] != '\t')
				count--;
			      if (count > 0 && (temp = strdup(&(current[count + 1]))))
				{
				  /* printf("temp : '%s'\n", temp); */
				  element->parentId = atoi(temp);
				  free(temp);
				  break;
				}
			    }
			}
		    }
		  count--;
		}
	      char *tmp;

	      if (tmp = strstr(ret->content, "\t"))
		*tmp = '\0';
	      element->content = strdup(ret->content);
	      /* sscanf(ret->content, "%ms[^'\t']\t", &element->content); */
	      if (element->content)
	      	{
	      	  /* printf("\t\t\tname = '%s'\n", element->content); */
	      	  /* printf("\t\t\tid = %d\n", element->id); */
		  /* if (element->parentId) */
		  /*   printf("\t\t\tparentId = '%d'\n", element->parentId); */
		  /* printf("\n"); */
	      	  list = addToList(list, element);
	      	}
	    }
	  ret = ret->next;
	}
      if (tmp)
	destroyList(tmp, TRUE);
    }

  buildCategoriesTree();
  return list;
}

void		buildCategoriesTree(void)
{
  t_list	*full;
  t_list	*list;
  t_list	*subList;
 
  ListId	*current;
  ListId	*subCurrent;

  full = categories;
  list = categories;
  while (list)
    {
      if (list->content && (current = list->content))
	{
	  subList = full;
	  while (subList)
	    {
	      if (subList->content && (subCurrent = subList->content))
		{
		  if (current->id == subCurrent->parentId)
		    {
		      /* if (!current->child) */
		      	/* current->child = subList; */
		      /* else */
		      current->child = addToList(current->child, subCurrent);
		      subCurrent->parent = list;
		      /* printf("%s is %s's parent\n", current->content, subCurrent->content); */
		    }
		}
	      subList = subList->next;
	    }
	}
      list = list->next;
    }
}

/*                               -*- Mode: C -*- 
 * 
 * Filename: databases.h
 * Description: 
 * Author: Lunacie
 * Created: Wed Sep  2 13:12:20 2015 (+0200)
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

#ifndef			DATABASES_H
#define			DATABASES_H

#include		"list.h"

enum
  {
    MYSQL = 1,
    POSTGRESQL = 2
  };

char			*setDatabase(char *new);
char			*setScript(char *new);
int			setDatabaseSystem(int new);
void			setRequestResult(char *new);
char			*getNextResultValue(char **ptr);

t_list			*execRequest(void);
char			*execRequestGetValue(void);

/*
INSERT INTO table (nom_colonne_1, nom_colonne_2, ...
		   VALUES ('valeur 1', 'valeur 2', ...)
*/

#endif			/* DATABASES_H */

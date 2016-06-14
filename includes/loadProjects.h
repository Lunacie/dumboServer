/*                               -*- Mode: C -*- 
 * 
 * Filename: loadProjects.h
 * Description: 
 * Author: Lunacie
 * Created: Tue Aug 11 16:33:17 2015 (+0200)
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


#ifndef			LOAD_PROJECTS_H
#define			LOAD_PROJECTS_H

#include		"server.h"
#include		"projects.h"

int			loadProjects(t_socket *client, t_projects id, char *lib);
void			set_handle(void *handle);


#endif			/* LOAD_PROJECTS_H */

/*                               -*- Mode: C -*- 
 * 
 * Filename: hours.c
 * Description: 
 * Author: Lunacie
 * Created: Fri Sep  4 13:16:34 2015 (+0200)
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


#include		"server.h"
#include		"hours.h"

void			displayTime()
{
  char			*cmd;
  FILE			*fp;
  char			buffer[1024];

  cmd = "date";
  if (!(fp = popen(cmd, "r")))
    return;
  while (fgets(buffer, 1024, fp))
    { 
      buffer[strlen(buffer) - 1] = '\0';
      printf("\n\033[32m[%s] CLIENT REQUEST ============================\033[0m\n", buffer);
      /* list = addToList(list, strdup(buffer)); */
    }
  fclose(fp);

}

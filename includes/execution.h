/*                               -*- Mode: C -*- 
 * 
 * Filename: execution.c
 * Description: 
 * Author: Lunacie
 * Created: Fri Jul 10 18:45:37 2015 (+0200)
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


#ifndef			EXECUTION_H
#define			EXECUTION_H

#define			START_EXEC		"<?c"
#define			END_EXEC		"?>"

#define			INVALID_SCRIPT		"[?c Invalid Script ?]\n"
#define			NOT_FOUND_SCRIPT	"[?c Script Not Found ?]\n"
#define			NULL_FUNCTION_SCRIPT	"[?c Script error : Null fuction ?]\n"
#define			ERROR_SCRIPT		"[?c Script error : Abort ?]\n"
#define			MISSING_PAR_SCRIPT	"[?c Script error : Missing Parameters ?]\n"

#define			END_COMMAND_CHAR	';'

t_list			*execScript(char *script, t_socket *client, t_list **full);
t_list			*cutScript(char *script);
char			*getFunctionName(char *pattern);

t_list			*commitResult(t_list *list, t_list **script);

typedef struct		s_function
{
  char			*name;
  t_list		*(*_ptr)(t_list **script, t_socket *client);
}			t_function;


typedef struct		s_urlRewrite
{
  char			*original;
  char			*redirect;
  size_t		len;
}			t_urlRewrite;



#endif			/* EXECUTION_H */

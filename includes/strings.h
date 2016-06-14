/*                               -*- Mode: C -*- 
 * 
 * Filename: strings.h
 * Description: 
 * Author: Lunacie
 * Created: Mon Jun 22 16:53:03 2015 (+0200)
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


#ifndef			STRINGS_H
#define			STRINGS_H

#include		"list.h"

char			*getAllocatedStringSet(char *str, size_t len);
char			*strAlCat(char *str1, char *str2, size_t len);
int			putStrFd(void *str, int fd, size_t len, t_bool isBinary);
t_bool			stringsAreTheSame(char *str1, char *str2);

char			*replaceStrInStr(char *needle, char *haystack, char *new);
char			*replaceCharactersInStr(char old, char new, char *str);

char			*getPattern(char *start, char *end, char **str);
char			*startsByPattern(char *pattern, char *str);
char			*previewString(char *str, size_t max);
char			*removeDoubleChar(char *str, char c, t_bool alloc);

void			printStringUTF8(char *str);

/*			Will return anything matching the pattern
**			* means any character
**			PATTERN means, the string we are looking for
*/
#define			PATTERN			"<?>"
char			*getMatchingPattern(char *str, char *pattern, t_bool debug);
t_list			*getWordList(char *pattern, t_bool debug);

char			*reformatString(char *str, char *_original, char *_new, t_bool debug);
char			*foundWord(t_list **list, char *_original, char **str, t_bool debug);
char			*foundAnyCharacters(t_list **, t_list *, char *,
					    char **, char **, t_bool debug, t_bool save);
char			*foundPattern(t_list **, t_list *, char *, char **, char **, t_bool debug);
t_bool			strIsContainedBetween(char *str, char min, char max);

#define			specialCharacters	",-:éàè ()"

#endif			/* STRINGS_H */

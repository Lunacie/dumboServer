/*                               -*- Mode: C -*- 
 * 
 * Filename: crypto.c
 * Description: 
 * Author: Lunacie
 * Created: Fri Jan  1 13:28:47 2016 (+0100)
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


#include			<openssl/sha.h>

#include			"program.h"
#include			"crypto.h"

char				*stringToSHA1(char *original)
{
  char				*ret;
  char				buffer[SHA_DIGEST_LENGTH];
  size_t			i;
  char				cbyte;

  if (!(ret = malloc(SHA_DIGEST_LENGTH * 2 + 1)))
    {
      perror("malloc");
      exit(FAILURE);
    }
  memset(ret, 0x0, SHA_DIGEST_LENGTH * 2 + 1);
  memset(buffer, 0x0, SHA_DIGEST_LENGTH + 1);
  SHA1(original, strlen(original), buffer);
  for (i = 0; i < SHA_DIGEST_LENGTH; i++)
    {
      // snprintf gives out a signed result. 
      // Store into char and use binary mask
      // 0xff to keep only what we're looking for without trailing ffff...
      cbyte = buffer[i];
      sprintf(((char *)&(ret[i * 2])), "%02x", cbyte & 0xff);
      /* printf("%02x\n", cbyte & 0xff); */
    }
  return ret;
}

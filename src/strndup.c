#include <string.h>
#include <stdlib.h>
#include <assert.h>

/**
 * Only added because apple is a piece of shit company that insists on changing
 * the GNU C compiler and doesn't have an strndup function.
 *
 * Only difference is this one doesn't try to do an strlen on the given string.
 * It's assumed that you want to copy the whole string.
 */

const char *strndup(char *str, size_t len)
{
	char *dest = NULL;
    assert(str && "You must give a valid input string, not NULL.");

    dest = malloc(len + 1);
    assert(dest && "Memory failure in strndup.");

	/* Copy and truncate */
	memcpy(dest, str, len);
	dest[len] = '\0';

	return dest;
}

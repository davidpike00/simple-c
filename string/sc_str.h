/*
 * MIT License
 *
 * Copyright (c) 2020 Ozan Tezcan
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef SC_STR_H
#define SC_STR_H

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/**
 * Length prefixed C strings, length is at the start of the allocated memory
 *  e.g :
 *  -----------------------------------------------
 *  | 0 | 0 | 0 | 4 | 'T' | 'E' | 'S' | 'T' | '\0'|
 *  -----------------------------------------------
 *                    ^
 *                  return
 *  User can keep pointer to first character, so it's like C style strings with
 *  additional functionality when it's used with these functions here.
 */

/**
 * Plug your allocators.
 */
#define sc_str_malloc  malloc
#define sc_str_realloc realloc
#define sc_str_free    free

/**
* If you want to log or abort on errors like out of memory,
* put your error function here. It will be called with printf like error msg.
*
* my_on_error(const char* fmt, ...);
*/
#define sc_str_on_error(...)

/**
 * @param str '\0' terminated C string, must not be NULL.
 * @return    Length prefixed string. NULL on out of memory.
 */
char *sc_str_create(const char *str);

/**
 * @param str C string, no need for '\0' termination, must not be NULL.
 * @param len Length of the 'str'.
 * @return    Length prefixed string. NULL on out of memory.
 */
char *sc_str_create_len(const char *str, uint32_t len);

/**
 * printf-style string creation.
 *
 * @param fmt Format
 * @param ... Arguments
 * @return    Length prefixed string. NULL on out of memory.
 */
char *sc_str_create_fmt(const char *fmt, ...);

/**
 * vprintf-style string creation.
 *
 * @param fmt Format
 * @param va  va_list
 * @return    Length prefixed string. NULL on out of memory.
 */
char *sc_str_create_va(const char *fmt, va_list va);

/**
 * Deallocate length prefixed string.
 *
 * @param str Length prefixed string. NULL values are accepted.
 */
void sc_str_destroy(char *str);

/**
 * @param str Length prefixed string. NULL values are accepted.
 * @return    Length of the string. If NULL, '-1' will be returned.
 */
int64_t sc_str_len(const char *str);

/**
 * @param str Length prefixed string. NULL values are accepted.
 * @return    Length of the duplicate. NULL on out of memory.
 */
char *sc_str_dup(const char *str);

/**
 * @param str    Pointer to length prefixed string.
 * @param param  New value to set.
 * @return       'false' on out of memory.
 *               'true' on success, '*str' may change.
 */
bool sc_str_set(char **str, const char *param);

/**
 * @param str Pointer to length prefixed string.
 * @param fmt Format
 * @param ... Arguments
 * @return    'false' on out of memory, previous value will remain intact.
 *            'true' on success, '*str' may change.
 */
bool sc_str_set_fmt(char **str, const char *fmt, ...);

/**
 * @param str  Pointer to length prefixed string.
 * @param text Text to append.
 * @return     'false' on out of memory, previous value will remain intact.
 *             'true' on success, '*str' may change.
 */
bool sc_str_append(char **str, const char *text);

/**
 * @param str Pointer to length prefixed string. (char**).
 * @param fmt Format
 * @param ... Arguments
 * @return    'false' on out of memory, previous value will remain intact.
 *            'true' on success, '*str' may change.
 */
#define sc_str_append_fmt(str, fmt, ...)                                       \
    sc_str_set_fmt(str, "%s" fmt, *str, __VA_ARGS__)

/**
 * Compare two length prefixed string. If you want to compare with regular
 * C string, use strcmp().
 *
 * @param str   Length prefixed string, must not be NULL.
 * @param other Length prefixed string, must not be NULL.
 * @return      'true' if equals.
 */
bool sc_str_cmp(const char *str, const char *other);

/**
 * Tokenization is zero-copy but a bit tricky. This function will mutate 'str',
 * but it is temporary. On each 'sc_str_token_begin' call, this function will
 * place '\0' character at the end of a token and put delimiter at the end of
 * the 'str'.
 * e.g user1,user2\0 after first iteration will be user1\0user2,
 *
 * sc_str_token_end() will fix original string if necessary.
 *
 * usage:
 *
 * char *str = sc_str_create("user1,user2,user3");
 * char *save = NULL; // Must be initialized with NULL.
 * const char *token;
 *
 * while ((token = sc_str_token_begin(str, &save, ",") != NULL) {
 *      printf("token : %s \n", token);
 * }
 *
 * sc_str_token_end(str, &save);
 *
 *
 * @param str   Length prefixed string, must not be NULL.
 * @param save  Helper variable for tokenizer code.
 * @param delim Delimiter list.
 * @return      Token.
 */
const char *sc_str_token_begin(char *str, char **save, const char *delim);
void sc_str_token_end(char *str, char **save);

/**
 * @param str  Length prefixed string, must not be NULL.
 * @param list Character list to trim.
 * @return     'false' on out of memory, previous value will remain intact.
 *             'true' on success, '*str' may change.
 */
bool sc_str_trim(char **str, char *list);

/**
 * @param str   Length prefixed string, must not be NULL.
 * @param start Start index.
 * @param end   End index.
 * @return      'false' on out of range.
 *              'false' on out of memory, previous value will remain intact.
 *              'true' on success, '*str' may change.
 */
bool sc_str_substring(char **str, uint32_t start, uint32_t end);

bool sc_str_replace(char **str, const char *rep, const char *with);


#endif

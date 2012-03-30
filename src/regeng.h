/*
 * regeng.h - APme: Aion Automatic Abyss Point Tracker
 *
 * Copyright (C) 2012 Mitja Horvat <pinkfluid@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */

/**
 * @file 
 *
 * The Regular Expression Engine
 *
 * @author Mitja Horvat <pinkfluid@gmail.com>
 */
#ifndef REGENG_H_INCLUDED
#define REGENG_H_INCLUDED

#include <pcreposix.h>

/**
 * @ingroup regeng
 *
 * @{
 */

/**
 * This macro checks if a regmatch_t @p x was matched
 *
 * @retval      true        If @p x was matched
 * @retval      false       If @p x was not matched
 */
#define RE_MATCH(x)  (((x).rm_so != -1) && ((x).rm_eo != -1))

/** Marker for end of regeng */
#define RE_INVALID_ID       0xffffffff
/** Maximum number of regmatch_t structures */
#define RE_REMATCH_MAX      32

/** Check if the regeng is valid */
#define RE_REGENG_VALID(x)  (((x)->re_id  != RE_INVALID_ID) && \
                             ((x)->re_exp != NULL))

/** The last element of the regeng array must be this macro */
#define RE_REGENG_END       { .re_id = RE_INVALID_ID, .re_exp = NULL }

/**
 * Main regeng structure
 * 
 * The regeng functions accept an array of these structures as input parameters
 */
struct regeng
{
    uint32_t    re_id;          /**< Regular expression ID, this is mainly useful for the
                                  * callback function                                                   */
    regex_t     re_comp;        /**< Compiled regular expression, this is initialized by
                                  * re_init() from @p re_exp                                            */
    char        *re_exp;        /**< Regular expression string                                          */
};

/**
 * The regeng callback
 *
 * @note That rematch may contain fewer matches than rematch_max. @ref RE_MATCH should be used
 * to check if a regmatch_t element is valid or not.
 *
 * @param[in]       re_id       This is the @p re_id from @ref regeng
 * @param[in]       str         This is the full string that had a match
 * @param[in]       rematch     Array of matched parameters
 * @param[in]       rematch_max Number of matches in @p rematch, maximum @ref RE_REMATCH_MAX
 */
typedef void  re_callback_t(uint32_t re_id, const char *str, regmatch_t *rematch, size_t rematch_max);

extern bool   re_init(struct regeng *re_array);
extern bool   re_parse(re_callback_t re_callback, struct regeng *re_array, char *str);

extern void   re_strlcpy(char *outstr, const char *instr, size_t outsz, regmatch_t rem);
extern size_t re_strlen(regmatch_t rem);

/**
 * @}
 */
#endif /* REGENG_H_INCLUDE */

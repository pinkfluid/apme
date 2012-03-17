/*
 * aion_trans.c - APme: Aion Automatic Abyss Point Tracker
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
 * @file aion_trans.c Aion Language Translator
 *
 * @author Mitja Horvat <pinkfluid@gmail.com>
 */
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>

#include "util.h"
#include "console.h"
#include "aion.h"

/**
 * 
 * @defgroup aion_trans Aion Translator
 * @brief Aion Translator between Asmodian <-> Elyos languages
 *
 * In game this module is used by 4 commands:
 *      - ?elyos: This is whta an Asmodian uses to talk to Elys
 *      - ?asmo: This is what Elyos use to talk to Asmodians
 *      - ?relyos: The reverse of ?elyos (used by Elyos to decode
 *          what other Elyos said to Asmodians)
 *      - ?rasom: Reverse of ?asmo (used by Asmodians to decode 
 *          what other Asmodians said to elyos)
 *
 * @note The tables and algorithm there was not inveted by me, but
 * was ripped off on a forum thread on aionsource.com
 * @todo Update credits for the translator.
 *
 * @section aion_trans_howto How does it work?
 *
 * - There are 4 translation tables; start with Table 1
 * - Translate the character in the <I>In</I> row to the
 *   character in the <I>Out</I> row
 * - Non-alphanumeric characters are not translated, but they
 *   do reset the table index to Table1
 * - Move to the table with index <I>NextTbl</I>
 * - Repeat until done
 *
 * @dot
 * digraph translate
 * {
 *      TABLE1
 *      [
 *          shape="none"
 *          label=
 *          <
 *              <TABLE border="0" cellborder="1" cellspacing="0">
 *                  <TR>
 *                      <TD border="0" port="blank"></TD>
 *                      <TD colspan="5" port="table">Table 1</TD>
 *                  </TR>
 *                  <TR>
 *                      <TD port="in"><FONT point-size="8.0">In</FONT></TD>
 *                      <TD>a</TD>
 *                      <TD>b</TD>
 *                      <TD>c</TD>
 *                      <TD>d</TD>
 *                      <TD port="e">e</TD>
 *                  </TR>
 *                  <TR>
 *                      <TD><FONT point-size="8.0">Out</FONT></TD>
 *                      <TD>q</TD>
 *                      <TD>w</TD>
 *                      <TD>e</TD>
 *                      <TD>r</TD>
 *                      <TD>t</TD>
 *                  </TR>
 *                  <TR>
 *                      <TD><FONT point-size="8.0">NextTbl</FONT></TD>
 *                      <TD port="1">1</TD>
 *                      <TD port="2">2</TD>
 *                      <TD port="3">3</TD>
 *                      <TD port="4">4</TD>
 *                      <TD port="5">1</TD>
 *                  </TR>
 *              </TABLE>
 *          >
 *      ]
 *      TABLE2
 *      [
 *          shape="none"
 *          label=
 *          <
 *              <TABLE border="0" cellborder="1" cellspacing="0">
 *                  <TR>
 *                      <TD colspan="5" port="table">Table 2</TD>
 *                  </TR>
 *                  <TR>
 *                      <TD port="a">a</TD>
 *                      <TD>b</TD>
 *                      <TD>c</TD>
 *                      <TD>d</TD>
 *                      <TD>e</TD>
 *                  </TR>
 *                  <TR>
 *                      <TD>q</TD>
 *                      <TD>w</TD>
 *                      <TD>e</TD>
 *                      <TD>r</TD>
 *                      <TD>t</TD>
 *                  </TR>
 *                  <TR>
 *                      <TD port="2">2</TD>
 *                      <TD port="3">3</TD>
 *                      <TD port="4">4</TD>
 *                      <TD port="3">3</TD>
 *                      <TD port="5">1</TD>
 *                  </TR>
 *              </TABLE>
 *          >
 *      ]
 *
 *      TABLE3 [ shape="box" color="lightgray" fontcolor="lightgray"]
 *      TABLE4 [ shape="box" color="lightgray" fontcolor="lightgray"]
 *
 *      TABLE1:1 -> TABLE1:in
 *      TABLE2:2 -> TABLE2:a
 *      TABLE1:2 -> TABLE2
 *      TABLE2:5 -> TABLE1:e
 *
 *      TABLE1:3 -> TABLE3 [color="lightgray"]
 *      TABLE1:4 -> TABLE4 [color="lightgray"]
 *      TABLE2:3 -> TABLE3 [color="lightgray"]
 *      TABLE2:4 -> TABLE4 [color="lightgray"]
 * }
 * @enddot
 *
 * @{
 */

/** Translation structure */
struct aion_table
{
    char *at_alpha;         /**< Translation table              */
    char *at_next;          /**< Index of next table to be used */
};

/** Language table definition           */
struct aion_language
{
    struct aion_table al_table[4];
};

/** Elyos -> Asmodian language translation table */
struct aion_language aion_lang_asmodian =
{
    .al_table =
    {
        {
            .at_alpha = "ihkjmlonqpsrutwvyxazcbedgf",
            .at_next  = "11111111111111111121222222",
        },
        {
            .at_alpha = "dcfehgjilknmporqtsvuxwzyba",
            .at_next  = "11111111111111111111111122",
        },
        {
            .at_alpha = "edgfihkjmlonqpsrutwvyxazcb",
            .at_next  = "11111111111111111111112122",
        },
        {
            .at_alpha = "@@@@@@@@@@@@@@@@@@@@@@@@@@",
            .at_next  = "33333333333333333333333333",
        }
    }
};

/** Asmodian -> Elyos language translation table */
struct aion_language aion_lang_elyos =
{
    .al_table =
    {
        {
            .at_alpha = "jkhinolmrspqvwtuzGbcJafgde",
            .at_next  = "11111111111111111322322222",
        },
        {
            .at_alpha = "efcdijghmnklqropuvstyzabIJ",
            .at_next  = "11111111111111111111112233",
        },
        {
            .at_alpha = "fgdejkhinolmrspqvwtuzGbcJa",
            .at_next  = "11111111111111111111132232",
        },
        {
            .at_alpha = "ghefklijopmnstqrwxuvGHcdab",
            .at_next  = "11111111111111111111332222",
        }
    }
};

#ifndef USE_NEW_TRANSLATE
/**
 * Translate text to a language. Currently only
 * the Asmodian and Elyos language are supported.
 * If somebody wants to add support for Mau or
 * Krall, feel free to send patches :P
 *
 * Supported language IDs:
 *  - LANG_ASMODIAN
 *  - LANG_ELYOS
 *
 * @note This is used by the ?relyos and ?rasmo commands
 *
 * @param[in,out]   txt     Input/Output text
 * @param[in]       langid  Language ID
 *
 * @see aion_rtranslate()
 */
void aion_translate(char *txt, uint32_t langid)
{
    int index;
    struct aion_language *lang = NULL;

    switch (langid)
    {
        case LANG_ASMODIAN:
            lang = &aion_lang_asmodian;
            break;

        case LANG_ELYOS:
            lang = &aion_lang_elyos;
            break;

        default:
            con_printf("Unknown language\n");
            return;
    }

    index = 0;

    while (*txt != '\0')
    {
        int c = tolower((int)*txt);

        if (isalpha(c))
        {
            int offset = c - 'a';
            *txt  = lang->al_table[index].at_alpha[offset];
            index = lang->al_table[index].at_next[offset] - '0';
        }
        else
        {
            index = 0;
        }

        txt++;
    }
}

#else /* USE_NEW_TRANSLATE */

/**
 * @cond
 * This translator produces lots of capital text, so it's not that good
 * and has several other issues. It's much better to use the table.
 */
void aion_translate(char *txt, uint32_t langid)
{
    int carry = 0;

    while (*txt != '\0')
    {
        int input = tolower(*txt);
        if (isalpha(input))
        {
            int output;
            int base = 'a';

            input = input - base;
            while (input < 128)
            {
                output = (input ^ langid);
                output -= carry;

                if (isalpha(output))
                {
                    *txt = output;
                    carry = (((output + carry) ^ langid) / 26) + 1;
                    break;
                }

                input+=26;
            }
        }
        else
        {
            carry = 0;
        }

        txt++;
    }

    return;
}
/**
 * @endcond
 */
#endif /* USE_NEW_TRNASLATE */

/**
 * Reverse translator. This uses the translation formula
 * to reverse a translated a text back to normal language.
 *
 * This function does the opposite of aion_translate().
 * 
 * @note This is used by the ?relyos and ?rasmo commands
 *
 * @param[in,out]   txt     Input/Output text to translate
 * @param[in]       langid  Language ID. 
 *                          Either LANG_ASMODIAN or LANG_ELYOS
 *
 * @see aion_translate()
 */
void aion_rtranslate(char *txt, uint32_t langid)
{
    char carry = 0;

    while (*txt != '\0')
    {
        int input = *txt;

        if (isalpha(input))
        {
            int output;

            output = (((input + carry) ^ langid) % 26) + 'a';
            carry = (((input + carry) ^ langid) / 26) + 1;

            *txt = output;
        }
        else
        {
            carry = 0;
        }

        txt++;
    }
}

/**
 * @}
 */

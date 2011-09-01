#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>

#include "util.h"
#include "console.h"
#include "aion.h"

/*
 * Aion translation tables
 */
struct aion_table
{
    char *at_alpha;
    char *at_next;
};

struct aion_language
{
    struct aion_table al_table[4];
};

/* Asmodian -> elyos language translation table */
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

/* Elyos -> Asmodian language translation table */
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
            .at_next  = "11111111111111111111112222",
        },
        {
            .at_alpha = "fgdejkhinolmrspqvwtuzGbcJa",
            .at_next  = "11111111111111111111132222",
        },
        {
            .at_alpha = "ghefklijopmnstqrwxuvGHcdab",
            .at_next  = "11111111111111111111332222",
        }
    }
};

#ifndef USE_NEW_TRANSLATE
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

/* This translator produces lots of capital text, so it's not that good */
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
#endif /* USE_NEW_TRNASLATE */

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


/* ----------------------------------------------------------------- */
/*           The Japanese TTS System "Open JTalk"                    */
/*           developed by HTS Working Group                          */
/*           http://open-jtalk.sourceforge.net/                      */
/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2008-2016  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                                                                   */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/* - Redistributions of source code must retain the above copyright  */
/*   notice, this list of conditions and the following disclaimer.   */
/* - Redistributions in binary form must reproduce the above         */
/*   copyright notice, this list of conditions and the following     */
/*   disclaimer in the documentation and/or other materials provided */
/*   with the distribution.                                          */
/* - Neither the name of the HTS working group nor the names of its  */
/*   contributors may be used to endorse or promote products derived */
/*   from this software without specific prior written permission.   */
/*                                                                   */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            */
/* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       */
/* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          */
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          */
/* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS */
/* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          */
/* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON */
/* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   */
/* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    */
/* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           */
/* POSSIBILITY OF SUCH DAMAGE.                                       */
/* ----------------------------------------------------------------- */

#ifndef NJD_SET_DIGIT_C
#define NJD_SET_DIGIT_C

#ifdef __cplusplus
#define NJD_SET_DIGIT_C_START extern "C" {
#define NJD_SET_DIGIT_C_END   }
#else
#define NJD_SET_DIGIT_C_START
#define NJD_SET_DIGIT_C_END
#endif                          /* __CPLUSPLUS */

NJD_SET_DIGIT_C_START;

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "njd.h"
#include "njd_set_digit.h"

#ifdef ASCII_HEADER
#if defined(CHARSET_EUC_JP)
#include "njd_set_digit_rule_ascii_for_euc_jp.h"
#elif defined(CHARSET_SHIFT_JIS)
#include "njd_set_digit_rule_ascii_for_shift_jis.h"
#elif defined(CHARSET_UTF_8)
#include "njd_set_digit_rule_ascii_for_utf_8.h"
#else
#error CHARSET is not specified
#endif
#else
#if defined(CHARSET_EUC_JP)
#include "njd_set_digit_rule_euc_jp.h"
#elif defined(CHARSET_SHIFT_JIS)
#include "njd_set_digit_rule_shift_jis.h"
#elif defined(CHARSET_UTF_8)
#include "njd_set_digit_rule_utf_8.h"
#else
#error CHARSET is not specified
#endif
#endif

#define MAXBUFLEN 1024

static int strtopcmp(const char *str, const char *pattern)
{
   int i;

   for (i = 0;; i++) {
      if (pattern[i] == '\0')
         return i;
      if (str[i] == '\0')
         return -1;
      if (str[i] != pattern[i])
         return -1;
   }
}

static int get_digit(NJDNode * node, int convert_flag)
{
   int i;

   if (strcmp(NJDNode_get_string(node), "*") == 0)
      return -1;

   if (strcmp(NJDNode_get_pos_group1(node), NJD_SET_DIGIT_KAZU) == 0)
      for (i = 0; njd_set_digit_rule_numeral_list1[i] != NULL; i += 3)
         if (strcmp(njd_set_digit_rule_numeral_list1[i], NJDNode_get_string(node)) == 0) {
            if (convert_flag == 1) {
               NJDNode_set_string(node, (char *) njd_set_digit_rule_numeral_list1[i + 2]);
               NJDNode_set_orig(node, (char *) njd_set_digit_rule_numeral_list1[i + 2]);
            }
            return atoi(njd_set_digit_rule_numeral_list1[i + 1]);
         }

   return -1;
}

static int is_period(const char *str)
{
   if (str != NULL &&
       (strcmp(str, NJD_SET_DIGIT_TEN1) == 0 || strcmp(str, NJD_SET_DIGIT_TEN2) == 0)) {
      return 1;
   } else {
      return 0;
   }
}

static int is_comma(const char *str)
{
   if (str != NULL && strcmp(str, NJD_SET_DIGIT_COMMA) == 0) {
      return 1;
   } else {
      return 0;
   }
}

static int get_digit_sequence_score(NJDNode * start, NJDNode * end)
{
   const char *buff_pos_group1 = NULL;
   const char *buff_pos_group2 = NULL;
   const char *buff_string = NULL;
   int score = 0;

   if (start->prev) {
      buff_pos_group1 = NJDNode_get_pos_group1(start->prev);
      buff_pos_group2 = NJDNode_get_pos_group2(start->prev);
      buff_string = NJDNode_get_string(start->prev);
      if (strcmp(buff_pos_group1, NJD_SET_DIGIT_SUUSETSUZOKU) == 0)     /* prev pos_group1 */
         score += 2;
      if (strcmp(buff_pos_group2, NJD_SET_DIGIT_JOSUUSHI) == 0 || strcmp(buff_pos_group1, NJD_SET_DIGIT_FUKUSHIKANOU) == 0)     /* prev pos_group1 and pos_group2 */
         score += 1;
      if (buff_string != NULL) {
         if (is_period(buff_string) == 1) {
            if (!start->prev->prev
                || strcmp(NJDNode_get_pos_group1(start->prev->prev), NJD_SET_DIGIT_KAZU) != 0)
               score += 0;
            else
               score -= 5;
         } else if (strcmp(buff_string, NJD_SET_DIGIT_HAIHUN1) == 0)
            score -= 2;
         else if (strcmp(buff_string, NJD_SET_DIGIT_HAIHUN2) == 0)
            score -= 2;
         else if (strcmp(buff_string, NJD_SET_DIGIT_HAIHUN3) == 0)
            score -= 2;
         else if (strcmp(buff_string, NJD_SET_DIGIT_HAIHUN4) == 0)
            score -= 2;
         else if (strcmp(buff_string, NJD_SET_DIGIT_HAIHUN5) == 0)
            score -= 2;
         else if (strcmp(buff_string, NJD_SET_DIGIT_KAKKO1) == 0) {
            if (!start->prev->prev
                || strcmp(NJDNode_get_pos_group1(start->prev->prev), NJD_SET_DIGIT_KAZU) != 0)
               score += 0;
            else
               score -= 2;
         } else if (strcmp(buff_string, NJD_SET_DIGIT_KAKKO2) == 0)
            score -= 2;
         else if (strcmp(buff_string, NJD_SET_DIGIT_BANGOU) == 0)
            score -= 2;
      }
      if (start->prev->prev) {
         buff_string = NJDNode_get_string(start->prev->prev);   /* prev prev string */
         if (strcmp(buff_string, NJD_SET_DIGIT_BANGOU) == 0)
            score -= 2;
      }
   }
   if (end->next) {
      buff_pos_group1 = NJDNode_get_pos_group1(end->next);
      buff_pos_group2 = NJDNode_get_pos_group2(end->next);      /* next pos_group2 */
      buff_string = NJDNode_get_string(end->next);      /* next string */
      if (strcmp(buff_pos_group2, NJD_SET_DIGIT_JOSUUSHI) == 0
          || strcmp(buff_pos_group1, NJD_SET_DIGIT_FUKUSHIKANOU) == 0)
         score += 2;
      if (buff_string != NULL) {
         if (strcmp(buff_string, NJD_SET_DIGIT_HAIHUN1) == 0)
            score -= 2;
         else if (strcmp(buff_string, NJD_SET_DIGIT_HAIHUN2) == 0)
            score -= 2;
         else if (strcmp(buff_string, NJD_SET_DIGIT_HAIHUN3) == 0)
            score -= 2;
         else if (strcmp(buff_string, NJD_SET_DIGIT_HAIHUN4) == 0)
            score -= 2;
         else if (strcmp(buff_string, NJD_SET_DIGIT_HAIHUN5) == 0)
            score -= 2;
         else if (strcmp(buff_string, NJD_SET_DIGIT_KAKKO1) == 0)
            score -= 2;
         else if (strcmp(buff_string, NJD_SET_DIGIT_KAKKO2) == 0) {
            if (!end->next->next
                || strcmp(NJDNode_get_pos_group1(end->next->next), NJD_SET_DIGIT_KAZU) != 0)
               score += 0;
            else
               score -= 2;
         } else if (strcmp(buff_string, NJD_SET_DIGIT_BANGOU) == 0)
            score -= 2;
         else if (is_period(buff_string) == 1)
            score += 4;
      }
   }

   return score;
}

static void convert_digit_sequence_for_non_numerical_reading(NJDNode * start, NJDNode * end)
{
   NJDNode *node;
   int size = 0;

   for (node = start; node != end->next; node = node->next)
      size++;
   if (size <= 1)
      return;

   for (node = start, size = 0; node != end->next; node = node->next) {
      if (strcmp(NJDNode_get_string(node), NJD_SET_DIGIT_ZERO1) == 0
          || strcmp(NJDNode_get_string(node), NJD_SET_DIGIT_ZERO2) == 0) {
         NJDNode_set_pron(node, NJD_SET_DIGIT_ZERO_AFTER_DP);
         NJDNode_set_mora_size(node, 2);
      } else if (strcmp(NJDNode_get_string(node), NJD_SET_DIGIT_TWO) == 0) {
         NJDNode_set_pron(node, NJD_SET_DIGIT_TWO_AFTER_DP);
         NJDNode_set_mora_size(node, 2);
      } else if (strcmp(NJDNode_get_string(node), NJD_SET_DIGIT_FIVE) == 0) {
         NJDNode_set_pron(node, NJD_SET_DIGIT_FIVE_AFTER_DP);
         NJDNode_set_mora_size(node, 2);
      }
      NJDNode_set_chain_rule(node, NULL);
      if (size % 2 == 0) {
         NJDNode_set_chain_flag(node, 0);
      } else {
         NJDNode_set_chain_flag(node, 1);
         NJDNode_set_acc(node->prev, 3);
      }
      size++;
   }
}

static void convert_digit_sequence_for_numerical_reading(NJDNode * start, NJDNode * end)
{
   NJDNode *node;
   NJDNode *newnode;
   int digit;
   int place = 0;
   int index;
   int size = 0;
   int have = 0;

   for (node = start; node != end->next; node = node->next)
      size++;
   if (size <= 1)
      return;

   index = size % 4;
   if (index == 0)
      index = 4;
   if (size > index)
      place = (size - index) / 4;
   index--;
   if (place > 17)
      return;

   for (node = start; node != end->next; node = node->next) {
      digit = get_digit(node, 0);
      if (index == 0) {
         if (digit == 0) {
            NJDNode_set_pron(node, NULL);
            NJDNode_set_acc(node, 0);
            NJDNode_set_mora_size(node, 0);
         } else {
            have = 1;
         }
         if (have == 1) {
            if (place > 0) {
               newnode = (NJDNode *) calloc(1, sizeof(NJDNode));
               if (newnode == NULL) {
                  fprintf(stderr, "WARNING: convert_digit_sequence_for_numerical_reading() in njd_set_digit.c: Failed to allocate NJDNode.\n");
                  return;
               }
               NJDNode_initialize(newnode);
               NJDNode_load(newnode, (char *) njd_set_digit_rule_numeral_list3[place]);
               if (node->next == NULL) {
                  node->next = newnode;
                  newnode->prev = node;
                  node = newnode;
               } else {
                  node = NJDNode_insert(node, node->next, newnode);
               }
            }
            have = 0;
         }
         place--;
      } else {
         if (digit <= 0) {
            NJDNode_set_pron(node, NULL);
            NJDNode_set_acc(node, 0);
            NJDNode_set_mora_size(node, 0);
         } else if (digit == 1) {
            NJDNode_load(node, (char *) njd_set_digit_rule_numeral_list2[index]);
            have = 1;
         } else {
            newnode = (NJDNode *) calloc(1, sizeof(NJDNode));
            if (newnode == NULL) {
               fprintf(stderr, "WARNING: convert_digit_sequence_for_numerical_reading() in njd_set_digit.c: Failed to allocate NJDNode.\n");
               return;
            }
            NJDNode_initialize(newnode);
            NJDNode_load(newnode, (char *) njd_set_digit_rule_numeral_list2[index]);
            if (node->next == NULL) {
               node->next = newnode;
               newnode->prev = node;
               node = newnode;
            } else {
               node = NJDNode_insert(node, node->next, newnode);
            }
            have = 1;
         }
      }
      index--;
      if (index < 0)
         index = 4 - 1;
   }
}

static int search_numerative_class(const char *list[], NJDNode * node)
{
   int i;
   const char *str = NJDNode_get_string(node);

   if (strcmp(str, "*") == 0)
      return 0;
   for (i = 0; list[i] != NULL; i++) {
      if (strcmp(list[i], str) == 0)
         return 1;
   }
   return 0;
}

static void convert_digit_pron(const char *list[], NJDNode * node)
{
   int i;
   const char *str = NJDNode_get_string(node);

   if (strcmp(str, "*") == 0)
      return;
   for (i = 0; list[i] != NULL; i += 4) {
      if (strcmp(list[i], str) == 0) {
         NJDNode_set_pron(node, (char *) list[i + 1]);
         NJDNode_set_acc(node, atoi(list[i + 2]));
         NJDNode_set_mora_size(node, atoi(list[i + 3]));
         return;
      }
   }
}

static void convert_numerative_pron(const char *list[], NJDNode * node1, NJDNode * node2)
{
   int i, j;
   int type = 0;
   const char *str = NJDNode_get_string(node1);
   char buff[MAXBUFLEN];

   if (strcmp(str, "*") == 0)
      return;
   for (i = 0; list[i] != NULL; i += 2) {
      if (strcmp(list[i], str) == 0) {
         type = atoi(list[i + 1]);
         break;
      }
   }
   if (type == 1) {
      for (i = 0; njd_set_digit_rule_voiced_sound_symbol_list[i] != NULL; i += 2) {
         str = NJDNode_get_pron(node2);
         j = strtopcmp(str, njd_set_digit_rule_voiced_sound_symbol_list[i]);
         if (j >= 0) {
            if (strlen(njd_set_digit_rule_voiced_sound_symbol_list[i + 1]) + strlen(&str[j]) >= MAXBUFLEN) {
               fprintf(stderr, "ERROR: %s() in %s:%d: Buffer overflow prevented.\n", __func__, __FILE__, __LINE__);
               return;
            }
            strcpy(buff, njd_set_digit_rule_voiced_sound_symbol_list[i + 1]);
            strcat(buff, &str[j]);
            NJDNode_set_pron(node2, buff);
            break;
         }
      }
   } else if (type == 2) {
      for (i = 0; njd_set_digit_rule_semivoiced_sound_symbol_list[i] != NULL; i += 2) {
         str = NJDNode_get_pron(node2);
         j = strtopcmp(str, njd_set_digit_rule_semivoiced_sound_symbol_list[i]);
         if (j >= 0) {
            if (strlen(njd_set_digit_rule_semivoiced_sound_symbol_list[i + 1]) + strlen(&str[j]) >= MAXBUFLEN) {
               fprintf(stderr, "ERROR: %s() in %s:%d: Buffer overflow prevented.\n", __func__, __FILE__, __LINE__);
               return;
            }
            strcpy(buff, njd_set_digit_rule_semivoiced_sound_symbol_list[i + 1]);
            strcat(buff, &str[j]);
            NJDNode_set_pron(node2, buff);
            break;
         }
      }
   }
}

static void convert_digit_sequence(NJD * njd, NJDNode * s, NJDNode * e)
{
   NJDNode *node;
   NJDNode *final_digit_before_period = s;
   int numerical_reading = 1;   /* 1: numerical 0: unknown -1: non-numerical */
   int num_comma = 0;
   NJDNode *first_comma_before_period = NULL;

   /* skip head marks */
   if (s == NULL || e == NULL) {
      return;
   }
   if (is_comma(NJDNode_get_string(s)) == 1 || is_period(NJDNode_get_string(s)) == 1) {
      if (s != e)
         convert_digit_sequence(njd, s->next, e);
      return;
   }

   /* find final digit before period */
   while (final_digit_before_period->next != NULL && final_digit_before_period != e &&
          is_period(NJDNode_get_string(final_digit_before_period->next)) != 1) {
      final_digit_before_period = final_digit_before_period->next;      /* forward search */
   }
   while (final_digit_before_period->prev != NULL && final_digit_before_period != s &&
          is_comma(NJDNode_get_string(final_digit_before_period)) == 1) {
      final_digit_before_period = final_digit_before_period->prev;      /* backward search */
   }

   /* check commas */
   {
      int rindex = 0;
      for (rindex = 0, node = final_digit_before_period;; node = node->prev, rindex++) {
         if (is_comma(NJDNode_get_string(node)) == 1) {
            first_comma_before_period = node;
            num_comma++;
            if (numerical_reading == 1 && rindex % 4 != 3) {
               numerical_reading = 0;
            }
         } else if (numerical_reading == 1 && rindex % 4 == 3) {
            numerical_reading = 0;
         }
         if (node == s) {
            break;
         }
      }
   }

   /* check zero-start */
   if (s != final_digit_before_period && get_digit(s, 0) == 0)
      numerical_reading = -1;

   /* if no info, set unknown flag */
   if (numerical_reading == 1 && num_comma == 0)
      numerical_reading = 0;

   if (numerical_reading == 1) {
      /* numerical reading until period */
      if (num_comma > 0) {
         /* remove all commas before period */
         for (node = s; node != final_digit_before_period;) {
            if (is_comma(NJDNode_get_string(node)) == 1)
               node = NJD_remove_node(njd, node);
            else
               node = node->next;
         }
      }
      convert_digit_sequence_for_numerical_reading(s, final_digit_before_period);
      // NOTE: NJDNode_insert でリスト末尾にノードが追加された場合、njd->tail が古くなるため修復する
      while (njd->tail->next != NULL)
         njd->tail = njd->tail->next;
      if (final_digit_before_period != e)
         convert_digit_sequence(njd, final_digit_before_period->next, e);
   } else {
      NJDNode *final_digit;
      if (first_comma_before_period == NULL)
         final_digit = final_digit_before_period;
      else
         final_digit = first_comma_before_period->prev;

      if (numerical_reading == 0) {
         if (get_digit_sequence_score(s, final_digit) >= 0)
            numerical_reading = 1;
         else
            numerical_reading = -1;
      }

      if (numerical_reading == 1) {
         /* numerical reading until comma */
         convert_digit_sequence_for_numerical_reading(s, final_digit);
         // NOTE: NJDNode_insert でリスト末尾にノードが追加された場合、njd->tail が古くなるため修復する
         while (njd->tail->next != NULL)
            njd->tail = njd->tail->next;
      } else {
         /* non-numerical reading */
         convert_digit_sequence_for_non_numerical_reading(s, final_digit);
      }
      if (final_digit != e)
         convert_digit_sequence(njd, final_digit->next, e);
   }
}

static void set_digit_accent_rules(NJD * njd)
{
   NJDNode *node;
   NJDNode *digit_node;
   NJDNode *counter;
   NJDNode *tail;
   int digit;
   int counter_mora_size;
   int is_compound;
   int is_short_counter;

   /* 数詞の末尾に合わせて、助数詞との結合後のアクセント核を決める */
   for (counter = njd->head->next; counter != NULL; counter = counter->next) {
      node = counter->prev;
      if (strcmp(NJDNode_get_pos_group1(node), NJD_SET_DIGIT_KAZU) != 0 ||
          (strcmp(NJDNode_get_pos_group2(counter), NJD_SET_DIGIT_JOSUUSHI) != 0 &&
           strcmp(NJDNode_get_pos_group1(counter), NJD_SET_DIGIT_FUKUSHIKANOU) != 0))
         continue;
      digit = get_digit(node, 0);
      is_compound = node->prev != NULL &&
         strcmp(NJDNode_get_pos_group1(node->prev), NJD_SET_DIGIT_KAZU) == 0;

      /* 「個」は数詞の末尾にアクセント核を置く前部末型で結合する */
      if (strcmp(NJDNode_get_string(counter), "個") == 0)
         NJDNode_set_chain_rule(counter, "C3");

      /* 「十二人」は前部末型、「四人」「五人」「九人」の短い読みは後部のアクセント核を保つ */
      if (strcmp(NJDNode_get_string(counter), NJD_SET_DIGIT_NIN) == 0 &&
          ((is_compound && !(NJDNode_get_mora_size(node) == 1 &&
                            (digit == 4 || digit == 5 || digit == 9))) ||
           (!is_compound && (digit == 6 || digit == 7 || digit == 8 ||
                            (digit == 9 && NJDNode_get_mora_size(node) == 2)))))
         NJDNode_set_chain_rule(counter, "C3");

      /* 前部末型のアクセント核が撥音・長音・促音に来る数詞では、1拍前に置く */
      if (strcmp(NJDNode_get_chain_rule(counter), "C3") == 0 &&
          (digit == 3 || ((digit == 4 || digit == 9) && NJDNode_get_mora_size(node) == 2) ||
           strcmp(NJDNode_get_string(node), NJD_SET_DIGIT_TEN) == 0 ||
           strcmp(NJDNode_get_string(node), njd_set_digit_rule_numeral_list5[2]) == 0))
         NJDNode_set_chain_rule(counter, "F4@-1");

      /* 「十一日」「十二日」などは尾高型で結合する */
      if (strcmp(NJDNode_get_string(counter), NJD_SET_DIGIT_NICHI) == 0 && is_compound &&
          (digit == 1 || digit == 2 || digit == 6 || digit == 7 || digit == 8))
         NJDNode_set_chain_rule(counter, "F4@2");

      /* 「円」との結合や、「万」に短い助数詞が続く場合に、表の平板型を適用する */
      if ((strcmp(NJDNode_get_string(counter), "円") == 0 &&
           ((!is_compound && (digit == 1 || digit == 2 || digit == 3 || digit == 6 || digit == 8 ||
                              strcmp(NJDNode_get_string(node), NJD_SET_DIGIT_TEN) == 0)) ||
            strcmp(NJDNode_get_string(node), njd_set_digit_rule_numeral_list5[1]) == 0 ||
            strcmp(NJDNode_get_string(node), njd_set_digit_rule_numeral_list5[2]) == 0)) ||
          (strcmp(NJDNode_get_string(node), njd_set_digit_rule_numeral_list5[3]) == 0 &&
           NJDNode_get_mora_size(counter) <= 2 &&
           strcmp(NJDNode_get_string(counter), "ウォン") != 0 &&
           strcmp(NJDNode_get_string(counter), "ギガ") != 0))
         NJDNode_set_chain_rule(counter, "F5");

      /* 「五」に続く「本」「枚」「台」「年」などは、数詞の桁にかかわらず後部を平板型にする */
      if (digit == 5 && (strcmp(NJDNode_get_string(counter), "本") == 0 ||
                        strcmp(NJDNode_get_string(counter), "枚") == 0 ||
                        strcmp(NJDNode_get_string(counter), "台") == 0 ||
                        strcmp(NJDNode_get_string(counter), "代") == 0 ||
                        strcmp(NJDNode_get_string(counter), "番") == 0 ||
                        strcmp(NJDNode_get_string(counter), "年") == 0))
         NJDNode_set_chain_rule(counter, "F5");
      if (strcmp(NJDNode_get_string(counter), "年") == 0 &&
          ((NJDNode_get_mora_size(node) == 1 && (digit == 4 || digit == 9)) ||
           (!is_compound && digit == 3)))
         NJDNode_set_chain_rule(counter, "F5");

      /* 「二十回」などの10の倍数と「千回」は、助数詞より前にアクセント核を置く */
      if (strcmp(NJDNode_get_string(counter), "回") == 0 &&
          (strcmp(NJDNode_get_string(node), njd_set_digit_rule_numeral_list5[2]) == 0 ||
           (is_compound && strcmp(NJDNode_get_string(node), NJD_SET_DIGIT_TEN) == 0)))
         NJDNode_set_chain_rule(counter, "F4@-1");

      /* 「日目」は尾高型、「人前」は平板型で、分かれた接尾辞まで含めて結合する */
      if (counter->next != NULL && NJDNode_get_chain_flag(counter->next) != 0 &&
          strcmp(NJDNode_get_pos_group1(counter->next), "接尾") == 0) {
         if (strcmp(NJDNode_get_string(counter), NJD_SET_DIGIT_NICHI) == 0 &&
             strcmp(NJDNode_get_string(counter->next), "目") == 0)
            NJDNode_set_chain_rule(counter->next, "F4@1");
         if (strcmp(NJDNode_get_string(counter), NJD_SET_DIGIT_NIN) == 0 &&
             strcmp(NJDNode_get_string(counter->next), "前") == 0)
            NJDNode_set_chain_rule(counter->next, "F5");
      }
   }

   /* NHK アクセント辞典の付録の数詞と助数詞の表では、11〜19の「十」と一の位は通常1つのアクセント句になる */
   for (node = njd->head; node != NULL && node->next != NULL; node = node->next) {
      if (strcmp(NJDNode_get_string(node), NJD_SET_DIGIT_TEN) != 0 ||
          (node->prev != NULL &&
           strcmp(NJDNode_get_pos_group1(node->prev), NJD_SET_DIGIT_KAZU) == 0))
         continue;
      digit_node = node->next;
      digit = get_digit(digit_node, 0);
      if (digit < 1 || digit > 9)
         continue;
      counter = digit_node->next;
      if (counter != NULL &&
          (strcmp(NJDNode_get_pos(counter), NJD_SET_DIGIT_MEISHI) != 0 ||
           (strcmp(NJDNode_get_pos_group1(counter), NJD_SET_DIGIT_KAZU) == 0 &&
            search_numerative_class(njd_set_digit_rule_numeral_list5, counter) == 0)))
         counter = NULL;

      /* 「時半」「時間」のように助数詞が複数ノードに分かれる場合も拍数を合計する */
      counter_mora_size = 0;
      for (tail = counter; tail != NULL; tail = tail->next) {
         if (tail != counter &&
             (NJDNode_get_chain_flag(tail) == 0 ||
              strcmp(NJDNode_get_pos_group1(tail), "接尾") != 0))
            break;
         counter_mora_size += NJDNode_get_mora_size(tail);
      }
      is_short_counter = counter != NULL && counter_mora_size <= 2;

      /* 「球」「週」などの11〜19は平板型、「機種」「地区」は助数詞のアクセント核を保つ */
      if (counter != NULL) {
         if (strcmp(NJDNode_get_string(counter), "球") == 0 ||
             strcmp(NJDNode_get_string(counter), "周") == 0 ||
             strcmp(NJDNode_get_string(counter), "週") == 0 ||
             strcmp(NJDNode_get_string(counter), "戦") == 0 ||
             strcmp(NJDNode_get_string(counter), "層") == 0 ||
             strcmp(NJDNode_get_string(counter), "倍") == 0 ||
             strcmp(NJDNode_get_string(counter), "場所") == 0)
            NJDNode_set_chain_rule(counter, "C4");
         if (strcmp(NJDNode_get_string(counter), "機種") == 0 ||
             strcmp(NJDNode_get_string(counter), "地区") == 0)
            NJDNode_set_chain_rule(counter, "C1");
      }

      /* 同じ拍数でも語ごとに結合型が異なる短い助数詞は、表にある1つのアクセント句の型を使う */
      if (is_short_counter &&
          (strcmp(NJDNode_get_string(counter), "階") == 0 ||
           strcmp(NJDNode_get_string(counter), "級") == 0 ||
           strcmp(NJDNode_get_string(counter), "型") == 0 ||
           strcmp(NJDNode_get_string(counter), "巡") == 0 ||
           strcmp(NJDNode_get_string(counter), "勝") == 0 ||
           strcmp(NJDNode_get_string(counter), "乗") == 0 ||
           strcmp(NJDNode_get_string(counter), "敗") == 0 ||
           strcmp(NJDNode_get_string(counter), "ウォン") == 0 ||
           strcmp(NJDNode_get_string(counter), "ギガ") == 0 ||
           strcmp(NJDNode_get_string(counter), "か所") == 0 ||
           strcmp(NJDNode_get_string(counter), "機種") == 0 ||
           strcmp(NJDNode_get_string(counter), "地区") == 0 ||
           strcmp(NJDNode_get_string(counter), "球") == 0 ||
           strcmp(NJDNode_get_string(counter), "周") == 0 ||
           strcmp(NJDNode_get_string(counter), "週") == 0 ||
           strcmp(NJDNode_get_string(counter), "戦") == 0 ||
           strcmp(NJDNode_get_string(counter), "層") == 0 ||
           strcmp(NJDNode_get_string(counter), "倍") == 0 ||
           strcmp(NJDNode_get_string(counter), "場所") == 0)) {
         is_short_counter = 0;
      }

      /* 一の位を「ゴ」「ヨ」「ク」と読む数に短い助数詞が続くときは、NHK アクセント辞典で最初に掲載されている2つのアクセント句の形を使う */
      if (is_short_counter && NJDNode_get_mora_size(digit_node) == 1 &&
          (digit == 4 || digit == 5 || digit == 9)) {
         NJDNode_set_chain_flag(digit_node, 0);
         NJDNode_set_acc(node, 1);
      } else {
         NJDNode_set_chain_flag(digit_node, 1);
         if (counter == NULL && (digit == 3 || digit == 5))
            NJDNode_set_chain_rule(digit_node, "F4@-1");
         else
            NJDNode_set_chain_rule(digit_node, "C1");
      }
   }
}

void njd_set_digit(NJD * njd)
{
   int i, j;
   NJDNode *s = NULL;
   NJDNode *e = NULL;
   NJDNode *node;
   int find = 0;

   /* convert digit sequence */
   for (node = njd->head; node != NULL; node = node->next) {
      if (find == 0 && strcmp(NJDNode_get_pos_group1(node), NJD_SET_DIGIT_KAZU) == 0)
         find = 1;
      if (get_digit(node, 1) >= 0 ||
          (strcmp(NJDNode_get_pos_group1(node), NJD_SET_DIGIT_KAZU) == 0 &&
           (is_period(NJDNode_get_string(node)) == 1 || is_comma(NJDNode_get_string(node)) == 1)
          )) {
         if (s == NULL)
            s = node;
         if (node == njd->tail)
            e = node;
      } else {
         if (s != NULL)
            e = node->prev;
      }
      if (s != NULL && e != NULL) {
         convert_digit_sequence(njd, s, e);
         s = e = NULL;
      }
   }
   if (find == 0)
      return;
   NJD_remove_silent_node(njd);
   if (njd->head == NULL)
      return;

   for (node = njd->head->next; node != NULL && node->next != NULL;) {
      if (strcmp(NJDNode_get_string(node), "*") != 0
          && strcmp(NJDNode_get_string(node->prev), "*") != 0
          && is_period(NJDNode_get_string(node)) == 1
          && strcmp(NJDNode_get_pos_group1(node->prev), NJD_SET_DIGIT_KAZU) == 0
          && strcmp(NJDNode_get_pos_group1(node->next), NJD_SET_DIGIT_KAZU) == 0) {
         NJDNode_load(node, NJD_SET_DIGIT_TEN_FEATURE);
         NJDNode_set_chain_flag(node, 1);
         if (strcmp(NJDNode_get_string(node->prev), NJD_SET_DIGIT_ZERO1) == 0
             || strcmp(NJDNode_get_string(node->prev), NJD_SET_DIGIT_ZERO2) == 0) {
            NJDNode_set_pron(node->prev, NJD_SET_DIGIT_ZERO_BEFORE_DP);
            NJDNode_set_mora_size(node->prev, 2);
         } else if (strcmp(NJDNode_get_string(node->prev), NJD_SET_DIGIT_TWO) == 0) {
            NJDNode_set_pron(node->prev, NJD_SET_DIGIT_TWO_BEFORE_DP);
            NJDNode_set_mora_size(node->prev, 2);
         } else if (strcmp(NJDNode_get_string(node->prev), NJD_SET_DIGIT_FIVE) == 0) {
            NJDNode_set_pron(node->prev, NJD_SET_DIGIT_FIVE_BEFORE_DP);
            NJDNode_set_mora_size(node->prev, 2);
         } else if (strcmp(NJDNode_get_string(node->prev), NJD_SET_DIGIT_SIX) == 0) {
            NJDNode_set_acc(node->prev, 1);
         }
         /* skip digit sequence */
         node = node->next;
         while (node && strcmp(NJDNode_get_pos(node), NJD_SET_DIGIT_MEISHI) == 0)
            node = node->next;
         if (node)
            node = node->next;
      } else {
         node = node->next;
      }
   }

   for (node = njd->head->next; node != NULL; node = node->next) {
      if (strcmp(NJDNode_get_pos_group1(node->prev), NJD_SET_DIGIT_KAZU) == 0) {
         if (strcmp(NJDNode_get_pos_group2(node), NJD_SET_DIGIT_JOSUUSHI) == 0
             || strcmp(NJDNode_get_pos_group1(node), NJD_SET_DIGIT_FUKUSHIKANOU) == 0) {
            /* convert digit pron */
            if (search_numerative_class(njd_set_digit_rule_numerative_class1b, node) == 1)
               convert_digit_pron(njd_set_digit_rule_conv_table1b, node->prev);
            else if (search_numerative_class(njd_set_digit_rule_numerative_class1c1, node) == 1)
               convert_digit_pron(njd_set_digit_rule_conv_table1c1, node->prev);
            else if (search_numerative_class(njd_set_digit_rule_numerative_class1c2, node) == 1)
               convert_digit_pron(njd_set_digit_rule_conv_table1c2, node->prev);
            else if (search_numerative_class(njd_set_digit_rule_numerative_class1d, node) == 1)
               convert_digit_pron(njd_set_digit_rule_conv_table1d, node->prev);
            else if (search_numerative_class(njd_set_digit_rule_numerative_class1e, node) == 1)
               convert_digit_pron(njd_set_digit_rule_conv_table1e, node->prev);
            else if (search_numerative_class(njd_set_digit_rule_numerative_class1f, node) == 1)
               convert_digit_pron(njd_set_digit_rule_conv_table1f, node->prev);
            else if (search_numerative_class(njd_set_digit_rule_numerative_class1g, node) == 1)
               convert_digit_pron(njd_set_digit_rule_conv_table1g, node->prev);
            else if (search_numerative_class(njd_set_digit_rule_numerative_class1h, node) == 1)
               convert_digit_pron(njd_set_digit_rule_conv_table1h, node->prev);
            else if (search_numerative_class(njd_set_digit_rule_numerative_class1i, node) == 1)
               convert_digit_pron(njd_set_digit_rule_conv_table1i, node->prev);
            else if (search_numerative_class(njd_set_digit_rule_numerative_class1j, node) == 1)
               convert_digit_pron(njd_set_digit_rule_conv_table1j, node->prev);
            else if (search_numerative_class(njd_set_digit_rule_numerative_class1k, node) == 1)
               convert_digit_pron(njd_set_digit_rule_conv_table1k, node->prev);
            /* convert numerative pron */
            if (search_numerative_class(njd_set_digit_rule_numerative_class2b, node) == 1)
               convert_numerative_pron(njd_set_digit_rule_conv_table2b, node->prev, node);
            else if (search_numerative_class(njd_set_digit_rule_numerative_class2c, node) == 1)
               convert_numerative_pron(njd_set_digit_rule_conv_table2c, node->prev, node);
            else if (search_numerative_class(njd_set_digit_rule_numerative_class2d, node) == 1)
               convert_numerative_pron(njd_set_digit_rule_conv_table2d, node->prev, node);
            else if (search_numerative_class(njd_set_digit_rule_numerative_class2e, node) == 1)
               convert_numerative_pron(njd_set_digit_rule_conv_table2e, node->prev, node);
            else if (search_numerative_class(njd_set_digit_rule_numerative_class2f, node) == 1)
               convert_numerative_pron(njd_set_digit_rule_conv_table2f, node->prev, node);
            /* modify accent phrase */
            NJDNode_set_chain_flag(node->prev, 0);
            NJDNode_set_chain_flag(node, 1);
         }
      }
   }

   for (node = njd->head->next; node != NULL; node = node->next) {
      if (strcmp(NJDNode_get_pos_group1(node->prev), NJD_SET_DIGIT_KAZU) == 0) {
         if (strcmp(NJDNode_get_pos_group1(node), NJD_SET_DIGIT_KAZU) == 0
             && NJDNode_get_string(node->prev) != NULL && NJDNode_get_string(node) != NULL) {
            /* modify accent phrase */
            find = 0;
            for (i = 0; njd_set_digit_rule_numeral_list4[i] != NULL; i++) {
               if (strcmp(NJDNode_get_string(node->prev), njd_set_digit_rule_numeral_list4[i]) == 0) {
                  for (j = 0; njd_set_digit_rule_numeral_list5[j] != NULL; j++) {
                     if (strcmp(NJDNode_get_string(node), njd_set_digit_rule_numeral_list5[j]) == 0) {
                        NJDNode_set_chain_flag(node->prev, 0);
                        NJDNode_set_chain_flag(node, 1);
                        find = 1;
                        break;
                     }
                  }
                  break;
               }
            }
            if (find == 0) {
               for (i = 0; njd_set_digit_rule_numeral_list5[i] != NULL; i++) {
                  if (strcmp(NJDNode_get_string(node->prev), njd_set_digit_rule_numeral_list5[i]) ==
                      0) {
                     for (j = 0; njd_set_digit_rule_numeral_list4[j] != NULL; j++) {
                        if (strcmp(NJDNode_get_string(node), njd_set_digit_rule_numeral_list4[j]) ==
                            0) {
                           NJDNode_set_chain_flag(node, 0);
                           break;
                        }
                     }
                     break;
                  }
               }
            }
         }
         if (search_numerative_class(njd_set_digit_rule_numeral_list8, node) == 1)
            convert_digit_pron(njd_set_digit_rule_numeral_list9, node->prev);
         if (search_numerative_class(njd_set_digit_rule_numeral_list10, node) == 1)
            convert_digit_pron(njd_set_digit_rule_numeral_list11, node->prev);
         if (search_numerative_class(njd_set_digit_rule_numeral_list6, node) == 1)
            convert_numerative_pron(njd_set_digit_rule_numeral_list7, node->prev, node);
      }
   }

   for (node = njd->head; node != NULL; node = node->next) {
      if (node->next != NULL &&
          strcmp(NJDNode_get_string(node->next), "*") != 0 &&
          strcmp(NJDNode_get_pos_group1(node), NJD_SET_DIGIT_KAZU) == 0 &&
          (node->prev == NULL
           || strcmp(NJDNode_get_pos(node->prev), NJD_SET_DIGIT_KIGOU) == 0
           || strcmp(NJDNode_get_pos_group1(node->prev), NJD_SET_DIGIT_KAZU) != 0)
          && (strcmp(NJDNode_get_pos_group2(node->next), NJD_SET_DIGIT_JOSUUSHI) == 0
              || strcmp(NJDNode_get_pos_group1(node->next), NJD_SET_DIGIT_FUKUSHIKANOU) == 0)) {
         /* convert class3 */
         for (i = 0; njd_set_digit_rule_numerative_class3[i] != NULL; i += 2) {
            if (strcmp(NJDNode_get_string(node->next), njd_set_digit_rule_numerative_class3[i]) == 0
                && strcmp(NJDNode_get_read(node->next),
                          njd_set_digit_rule_numerative_class3[i + 1]) == 0) {
               for (j = 0; njd_set_digit_rule_conv_table3[j] != NULL; j += 4) {
                  if (strcmp(NJDNode_get_string(node), njd_set_digit_rule_conv_table3[j]) == 0) {
                     NJDNode_set_read(node, (char *) njd_set_digit_rule_conv_table3[j + 1]);
                     NJDNode_set_pron(node, (char *) njd_set_digit_rule_conv_table3[j + 1]);
                     NJDNode_set_acc(node, atoi(njd_set_digit_rule_conv_table3[j + 2]));
                     NJDNode_set_mora_size(node, atoi(njd_set_digit_rule_conv_table3[j + 3]));
                     break;
                  }
               }
               break;
            }
         }
         /* person */
         if (strcmp(NJDNode_get_string(node->next), NJD_SET_DIGIT_NIN) == 0) {
            for (i = 0; njd_set_digit_rule_conv_table4[i] != NULL; i += 2) {
               if (strcmp(NJDNode_get_string(node), njd_set_digit_rule_conv_table4[i]) == 0) {
                  NJDNode_load(node, (char *) njd_set_digit_rule_conv_table4[i + 1]);
                  NJDNode_set_pron(node->next, NULL);
                  break;
               }
            }
         }
         /* the day of month */
         if (strcmp(NJDNode_get_string(node->next), NJD_SET_DIGIT_NICHI) == 0
             && strcmp(NJDNode_get_string(node), "*") != 0) {
            if (node->prev != NULL
                && strstr(NJDNode_get_string(node->prev), NJD_SET_DIGIT_GATSU) != NULL
                && strcmp(NJDNode_get_string(node), NJD_SET_DIGIT_ONE) == 0) {
               NJDNode_load(node, NJD_SET_DIGIT_TSUITACHI);
               NJDNode_set_pron(node->next, NULL);
            } else {
               for (i = 0; njd_set_digit_rule_conv_table5[i] != NULL; i += 2) {
                  if (strcmp(NJDNode_get_string(node), njd_set_digit_rule_conv_table5[i]) == 0) {
                     NJDNode_load(node, (char *) njd_set_digit_rule_conv_table5[i + 1]);
                     NJDNode_set_pron(node->next, NULL);
                     break;
                  }
               }
            }
         } else if (strcmp(NJDNode_get_string(node->next), NJD_SET_DIGIT_NICHIKAN) == 0) {
            for (i = 0; njd_set_digit_rule_conv_table6[i] != NULL; i += 2) {
               if (strcmp(NJDNode_get_string(node), njd_set_digit_rule_conv_table6[i]) == 0) {
                  NJDNode_load(node, (char *) njd_set_digit_rule_conv_table6[i + 1]);
                  NJDNode_set_pron(node->next, NULL);
                  break;
               }
            }
         }
      }
   }

   for (node = njd->head; node != NULL; node = node->next) {
      if ((node->prev == NULL
           || strcmp(NJDNode_get_pos_group1(node->prev), NJD_SET_DIGIT_KAZU) != 0)
          && node->next != NULL && node->next->next != NULL) {
         if (strcmp(NJDNode_get_string(node), NJD_SET_DIGIT_TEN) == 0
             && strcmp(NJDNode_get_string(node->next), NJD_SET_DIGIT_FOUR) == 0) {
            if (strcmp(NJDNode_get_string(node->next->next), NJD_SET_DIGIT_NICHI) == 0) {
               /* 「十四日」は「ジュ＼ー・ヨッカ」の2つのアクセント句、「十四日目」は尾高型の1つのアクセント句にする */
               if (node->next->next->next != NULL &&
                   strcmp(NJDNode_get_string(node->next->next->next), "目") == 0) {
                  NJDNode_load(node, NJD_SET_DIGIT_JUYOKKA);
                  NJDNode_set_pron(node->next, NULL);
                  NJDNode_set_chain_rule(node->next->next->next, "F4@1");
               } else {
                  NJDNode_load(node->next, NJD_SET_DITIT_YOKKA);
               }
               NJDNode_set_pron(node->next->next, NULL);
            } else if (strcmp(NJDNode_get_string(node->next->next), NJD_SET_DIGIT_NICHIKAN) == 0) {
               NJDNode_load(node, NJD_SET_DIGIT_JUYOKKAKAN);
               NJDNode_set_pron(node->next, NULL);
               NJDNode_set_pron(node->next->next, NULL);
            }
         } else if (strcmp(NJDNode_get_string(node), NJD_SET_DIGIT_TWO) == 0
                    && strcmp(NJDNode_get_string(node->next), NJD_SET_DIGIT_TEN) == 0) {
            if (strcmp(NJDNode_get_string(node->next->next), NJD_SET_DIGIT_NICHI) == 0) {
               NJDNode_load(node, NJD_SET_DITIT_HATSUKA);
               NJDNode_set_pron(node->next, NULL);
               NJDNode_set_pron(node->next->next, NULL);
            } else if (strcmp(NJDNode_get_string(node->next->next), NJD_SET_DIGIT_NICHIKAN) == 0) {
               NJDNode_load(node, NJD_SET_DIGIT_HATSUKAKAN);
               NJDNode_set_pron(node->next, NULL);
               NJDNode_set_pron(node->next->next, NULL);
            } else if (strcmp(NJDNode_get_string(node->next->next), NJD_SET_DIGIT_FOUR) == 0
                       && node->next->next->next != NULL) {
               if (strcmp(NJDNode_get_string(node->next->next->next), NJD_SET_DIGIT_NICHI) == 0) {
                  NJDNode_load(node, NJD_SET_DIGIT_NIJU);
                  NJDNode_load(node->next, NJD_SET_DITIT_YOKKA);
                  NJDNode_set_pron(node->next->next, NULL);
                  NJDNode_set_pron(node->next->next->next, NULL);
               } else if (strcmp(NJDNode_get_string(node->next->next->next), NJD_SET_DIGIT_NICHIKAN)
                          == 0) {
                  NJDNode_load(node, NJD_SET_DIGIT_NIJU);
                  NJDNode_load(node->next, NJD_SET_DIGIT_YOKKAKAN);
                  NJDNode_set_pron(node->next->next, NULL);
                  NJDNode_set_pron(node->next->next->next, NULL);
               }
            }
         }
      }
   }

   NJD_remove_silent_node(njd);
   if (njd->head == NULL)
      return;

   set_digit_accent_rules(njd);
}

NJD_SET_DIGIT_C_END;

#endif                          /* !NJD_SET_DIGIT_C */

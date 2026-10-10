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
   /* 小数点は「．」だけにし、「1・2年生」「3・4月」のように数を並べる中黒は小数点として読まない */
   if (str != NULL && strcmp(str, NJD_SET_DIGIT_TEN1) == 0) {
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

static int number_digit(NJDNode *node);

/* 「1.5」の小数点は後ろに数字が続き、「03-1234-5678.」「一〇一.」の文末の「.」は句点として読む */
static int is_decimal_point(NJDNode *node)
{
   return node != NULL && is_period(NJDNode_get_string(node)) && number_digit(node->next) >= 0;
}

/* 「1,050」の桁区切りは後ろに数字が続き、「一〇一,」「四〇五,電話」の句の終わりの「,」は読点として読む */
static int is_digit_group_comma(NJDNode *node)
{
   return node != NULL && is_comma(NJDNode_get_string(node)) && number_digit(node->next) >= 0;
}

static int is_kanji_digit_string(NJDNode *node)
{
   const char *str;
   if (node == NULL)
      return 0;
   str = NJDNode_get_string(node);
   return strlen(str) == 3 && strstr("〇一二三四五六七八九", str) != NULL;
}

/* 「三〇九,三〇八」のように「〇」を書いた漢数字は桁読みの表記なので、後ろの「,」を3桁区切りとみなさない */
static int has_written_zero(NJDNode *start, NJDNode *end)
{
   NJDNode *node;
   for (node = start; node != end->next; node = node->next)
      if (strcmp(NJDNode_get_string(node), "〇") == 0)
         return 1;
   return 0;
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
         else if (is_decimal_point(end->next) == 1)
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
      /* 「〇」「０」に加えて「零」も、小数点以下の桁では「ゼロ」と読む */
      if (get_digit(node, 0) == 0) {
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

#if defined(CHARSET_UTF_8) && !defined(ASCII_HEADER)
static int calendar_month_number(NJDNode *counter)
{
   NJDNode *node, *start = counter->prev;
   int value = 0, digit, count = 0;

   /* 「1 月」「０５ 月」「十一 月」は暦の月とし、「20 月」のような期間は「ツキ」の読みを保つ */
   while (start->prev != NULL &&
          strcmp(NJDNode_get_pos_group1(start->prev), NJD_SET_DIGIT_KAZU) == 0)
      start = start->prev;
   if (strcmp(NJDNode_get_string(start), NJD_SET_DIGIT_TEN) == 0) {
      if (start->next == counter)
         return 10;
      digit = get_digit(start->next, 0);
      if (start->next->next == counter && digit >= 1 && digit <= 2)
         return 10 + digit;
      return 0;
   }
   for (node = start; node != counter; node = node->next) {
      digit = get_digit(node, 0);
      if (digit < 0 || ++count > 2)
         return 0;
      value = value * 10 + digit;
   }
   return value >= 1 && value <= 12 ? value : 0;
}

static void restore_counter_features(NJD *njd)
{
   NJDNode *node, *start, *next;
   int i;

   for (node = njd->head; node != NULL; node = node->next) {
      /* ユーザー辞書で読みを保護した「2 人」の名詞「ヒト」は、助数詞「ニン」への変換対象から外して登録された読みを保つ */
      if (strcmp(NJDNode_get_pos_group3(node), "読み保護") == 0) {
         NJDNode_set_pos_group3(node, "*");
         continue;
      }
      /* 「2024 年」「10 時」の一般名詞や非自立名詞を対象にし、既に助数詞である「2024年」は辞書の値を保つ */
      if (node->prev == NULL ||
          strcmp(NJDNode_get_pos_group1(node->prev), NJD_SET_DIGIT_KAZU) != 0 ||
          strcmp(NJDNode_get_pos_group2(node), NJD_SET_DIGIT_JOSUUSHI) == 0 ||
          !((strcmp(NJDNode_get_pos(node), NJD_SET_DIGIT_MEISHI) == 0 &&
             (strcmp(NJDNode_get_pos_group1(node), "一般") == 0 ||
              strcmp(NJDNode_get_pos_group1(node), "非自立") == 0 ||
              strcmp(NJDNode_get_pos_group1(node), "接尾") == 0)) ||
            (strcmp(NJDNode_get_pos(node), "接頭詞") == 0 &&
             strcmp(NJDNode_get_pos_group1(node), NJD_SET_DIGIT_SUUSETSUZOKU) == 0)))
         continue;
      /* 「場面6 人前で話す」の6は場面番号とし、後ろの「人」は「ヒト」の読みを保つ */
      start = node->prev;
      while (start->prev != NULL &&
             strcmp(NJDNode_get_pos_group1(start->prev), NJD_SET_DIGIT_KAZU) == 0)
         start = start->prev;
      if (start->prev != NULL &&
          (strcmp(NJDNode_get_string(start->prev), "場面") == 0 ||
           strcmp(NJDNode_get_string(start->prev), "発言") == 0 ||
           strcmp(NJDNode_get_string(start->prev), "図表") == 0 ||
           strcmp(NJDNode_get_string(start->prev), "図") == 0 ||
           strcmp(NJDNode_get_string(start->prev), "表") == 0))
         continue;
      /* 「２ 人づくりの基盤」は見出し番号と人材育成の名詞なので、「人づくり」の「ヒト」を保つ */
      if (strcmp(NJDNode_get_string(node), "人") == 0 && node->next != NULL &&
          strcmp(NJDNode_get_string(node->next), "づくり") == 0)
         continue;
      /* 割合を表す「3 分の1」の「分」は「ブン」のまま、時間量の「3 分」は「フン」へ戻す */
      if (strcmp(NJDNode_get_string(node), "分") == 0 && node->next != NULL &&
          strcmp(NJDNode_get_string(node->next), "の") == 0 && node->next->next != NULL &&
          strcmp(NJDNode_get_pos_group1(node->next->next), NJD_SET_DIGIT_KAZU) == 0)
         continue;
      /* 「1 月」「1 月に会う」「2008 年 05 月 05 日」は日付として「ガツ」に戻し、「1 月ほど待った」「一 月が過ぎた」は解析済みの「ツキ」を保つ */
      if (strcmp(NJDNode_get_string(node), NJD_SET_DIGIT_GATSU) == 0) {
         if (calendar_month_number(node) == 0)
            continue;
         next = node->next;
         while (next != NULL && strcmp(NJDNode_get_pos_group1(next), NJD_SET_DIGIT_KAZU) == 0)
            next = next->next;
         if (node->next != NULL &&
             (start->prev == NULL || strcmp(NJDNode_get_string(start->prev), "年") != 0) &&
             (next == NULL || strcmp(NJDNode_get_string(next), "日") != 0) &&
             !(strcmp(NJDNode_get_pos_group1(node->next), "格助詞") == 0 &&
               strcmp(NJDNode_get_string(node->next), "に") == 0))
            continue;
      }
      for (i = 0; njd_set_digit_rule_counter_features[i][0] != NULL; i++) {
         /* 「１ 日本文化」「５ 本書」「図表３ 年齢」は「日」「本」「年」と表層全体が一致しないので、その語の読みを保つ */
         if (strcmp(NJDNode_get_string(node), njd_set_digit_rule_counter_features[i][0]) != 0)
            continue;
         NJDNode_set_pos(node, NJD_SET_DIGIT_MEISHI);
         NJDNode_set_pos_group1(node, "接尾");
         NJDNode_set_pos_group2(node, NJD_SET_DIGIT_JOSUUSHI);
         NJDNode_set_read(node, njd_set_digit_rule_counter_features[i][1]);
         NJDNode_set_pron(node, njd_set_digit_rule_counter_features[i][2]);
         NJDNode_set_acc(node, atoi(njd_set_digit_rule_counter_features[i][3]));
         NJDNode_set_mora_size(node, atoi(njd_set_digit_rule_counter_features[i][4]));
         NJDNode_set_chain_rule(node, njd_set_digit_rule_counter_features[i][5]);
         NJDNode_set_chain_flag(node, -1);
         /* 「1 月」の暦の読みは、番号の保護を解除した後で「1月」と同じアクセントにする */
         if (strcmp(NJDNode_get_string(node), NJD_SET_DIGIT_GATSU) == 0)
            NJDNode_set_pos_group3(node, "暦月");
         break;
      }
   }
}

static void mark_written_group_quantities(NJD *njd)
{
   NJDNode *node, *previous;
   int combined, separate;
   for (node = njd->head; node != NULL; node = node->next) {
      combined = strcmp(NJDNode_get_string(node), "一組") == 0 &&
                 strcmp(NJDNode_get_read(node), "イチクミ") == 0;
      separate = strcmp(NJDNode_get_pos_group1(node), NJD_SET_DIGIT_KAZU) == 0 &&
                 (strcmp(NJDNode_get_string(node), "一") == 0 ||
                  strcmp(NJDNode_get_string(node), "二") == 0) && node->next != NULL &&
                 strcmp(NJDNode_get_string(node->next), "組") == 0 &&
                 strcmp(NJDNode_get_read(node->next), "クミ") == 0;
      if ((!combined && !separate) || strcmp(NJDNode_get_pos_group3(node), "読み保護") == 0 ||
          (separate && strcmp(NJDNode_get_pos_group3(node->next), "読み保護") == 0))
         continue;
      previous = node->prev;
      /* 算用数字が漢数字へ変わる前に数量だけを記録し、複合数詞・小数・序数・学年直後の組番号を除く */
      if (previous != NULL &&
          (strcmp(NJDNode_get_pos_group1(previous), NJD_SET_DIGIT_KAZU) == 0 ||
           strcmp(NJDNode_get_string(previous), "第") == 0 ||
           strcmp(NJDNode_get_string(previous), "．") == 0 ||
           strcmp(NJDNode_get_string(previous), "・") == 0 ||
           (strcmp(NJDNode_get_string(previous), "年") == 0 && previous->prev != NULL &&
            strcmp(NJDNode_get_pos_group1(previous->prev), NJD_SET_DIGIT_KAZU) == 0) ||
           strcmp(NJDNode_get_string(previous), "一年") == 0))
         continue;
      if (combined) {
         NJDNode_set_read(node, "ヒトクミ");
         NJDNode_set_pron(node, "ヒトクミ");
         NJDNode_set_mora_size(node, 4);
         NJDNode_set_acc(node, 2);
      } else
         NJDNode_set_pos_group3(node, "数量組");
   }
}

static void set_written_group_readings(NJD *njd)
{
   NJDNode *node;
   const char *reading;
   for (node = njd->head; node != NULL; node = node->next) {
      if (strcmp(NJDNode_get_pos_group3(node), "数量組") != 0)
         continue;
      reading = strcmp(NJDNode_get_string(node), "一") == 0 ? "ヒト" : "フタ";
      NJDNode_set_read(node, reading);
      NJDNode_set_pron(node, reading);
      NJDNode_set_mora_size(node, 2);
      NJDNode_set_acc(node, 2);
      NJDNode_set_pos_group3(node, "*");
      /* 「二」の1モーラが「フタ」の2モーラになるので、組の C3 に渡す数詞末尾の核も2へ動かす */
      if (node->next != NULL)
         NJDNode_set_chain_rule(node->next, "C3");
   }
}

static void set_restored_month_accents(NJD *njd)
{
   NJDNode *node;
   int month;
   for (node = njd->head; node != NULL; node = node->next) {
      if (strcmp(NJDNode_get_pos_group3(node), "暦月") != 0)
         continue;
      month = calendar_month_number(node);
      /* 空白付きの「０４ 月」「０７ 月」「０９ 月」も「シ」「シチ」「ク」と読み、ゼロを含むアクセント句を保つ */
      get_digit(node->prev, 1);
      convert_digit_pron(njd_set_digit_rule_conv_table1e, node->prev);
      /* NHK アクセント辞典に従い、「1 月」「2 月」は尾高型にし、「3 月」「5 月」「9 月」は「サ＼ンガツ」「ゴ＼ガツ」「ク＼ガツ」と読む */
      /* 「01 月」「09 月」はアクセント核を保ち、空白なしと同じ「ゼロイ＼チガツ」「ゼロク＼ガツ」と読む */
      if (node->prev->prev != NULL && get_digit(node->prev->prev, 0) == 0)
         NJDNode_set_chain_rule(node, "C5");
      else
         NJDNode_set_chain_rule(node, month == 3 ? "F4@-1" :
                               (month == 5 || month == 9 ? "F4@0" : "F4@2"));
      NJDNode_set_pos_group3(node, "*");
   }
}

static int is_decimal_digit(NJDNode * digit)
{
   NJDNode *integer;
   /* 数詞を前へたどって小数点に行き着く桁は、小数の一部とみなす */
   while (digit != NULL && strcmp(NJDNode_get_pos_group1(digit), NJD_SET_DIGIT_KAZU) == 0)
      digit = digit->prev;
   if (digit == NULL)
      return 0;
   /* 「・・一日」の文の区切りは小数点とせず、「1.5日」のように整数部の数詞に続く点だけを小数点とする */
   if (is_period(NJDNode_get_string(digit)))
      return digit->prev != NULL &&
             strcmp(NJDNode_get_pos_group1(digit->prev), NJD_SET_DIGIT_KAZU) == 0;
   /* 漢字で書いた小数点は、「数詞＋点」の形と、辞書に1語で載っている「一点」「零点」の両方を見る */
   if ((strcmp(NJDNode_get_string(digit), "一点") == 0 &&
        strcmp(NJDNode_get_read(digit), "イッテン") == 0) ||
       (strcmp(NJDNode_get_string(digit), "零点") == 0 &&
        strcmp(NJDNode_get_read(digit), "レイテン") == 0))
      return 1;
   integer = digit->prev;
   if (integer != NULL && strcmp(NJDNode_get_string(integer), "ー") == 0)
      integer = integer->prev;
   return strcmp(NJDNode_get_string(digit), "点") == 0 && integer != NULL &&
          strcmp(NJDNode_get_pos_group1(integer), NJD_SET_DIGIT_KAZU) == 0;
}

static int is_calendar_day_enumeration(NJDNode *digit)
{
   NJDNode *month = digit;
   const char *str;
   int i, length;

   /* 「5月1．2日」「5 月 1．2 日」は月の直後の日付を小数で表すことはないため、列挙した日を「フツカ」などの和語で読む */
   if (digit->next == NULL || strcmp(NJDNode_get_string(digit->next), NJD_SET_DIGIT_NICHI) != 0)
      return 0;
   while (month != NULL && strcmp(NJDNode_get_pos_group1(month), NJD_SET_DIGIT_KAZU) == 0)
      month = month->prev;
   if (month == NULL || !is_period(NJDNode_get_string(month)))
      return 0;
   month = month->prev;
   if (month == NULL || strcmp(NJDNode_get_pos_group1(month), NJD_SET_DIGIT_KAZU) != 0)
      return 0;
   while (month != NULL && strcmp(NJDNode_get_pos_group1(month), NJD_SET_DIGIT_KAZU) == 0)
      month = month->prev;
   if (month == NULL)
      return 0;
   str = NJDNode_get_string(month);
   /* 「5 月」のように月が別の形態素に分かれる場合は直前の数詞も調べ、「月1.5日」だけの表記と区別する */
   if (strcmp(str, NJD_SET_DIGIT_GATSU) == 0)
      return month->prev != NULL &&
             (strcmp(NJDNode_get_pos_group3(month), "暦月") == 0 ||
              calendar_month_number(month) != 0);
   /* 辞書に1語で登録された「５月」「１２月」も数詞と月の組として扱い、「今月1.5日働く」の日数は小数のまま読む */
   while (*str != '\0') {
      length = strtopcmp(str, NJD_SET_DIGIT_TEN);
      if (length < 0)
         for (i = 0; njd_set_digit_rule_numeral_list1[i] != NULL; i += 3) {
            length = strtopcmp(str, njd_set_digit_rule_numeral_list1[i]);
            if (length > 0)
               break;
         }
      if (length <= 0)
         return 0;
      str += length;
      if (strcmp(str, NJD_SET_DIGIT_GATSU) == 0)
         return 1;
   }
   return 0;
}

static int uses_native_one_two(NJDNode * counter)
{
   int i;
   NJDNode *digit = counter->prev;
   /* 「十一」の「一」のような末尾の桁は除き、前に数詞がない「一」「二」だけを「ヒト」「フタ」と読む対象にする */
   if (digit == NULL || (digit->prev != NULL &&
       strcmp(NJDNode_get_pos_group1(digit->prev), NJD_SET_DIGIT_KAZU) == 0))
      return 0;
   if (strcmp(NJDNode_get_string(digit), "一") != 0 &&
       strcmp(NJDNode_get_string(digit), "二") != 0)
      return 0;
   for (i = 0; njd_set_digit_rule_numerative_class3[i] != NULL; i += 2)
      if (strcmp(NJDNode_get_string(counter), njd_set_digit_rule_numerative_class3[i]) == 0 &&
          strcmp(NJDNode_get_read(counter), njd_set_digit_rule_numerative_class3[i + 1]) == 0)
         return 1;
   return 0;
}
#endif

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

#if defined(CHARSET_UTF_8) && !defined(ASCII_HEADER)
static int identifier_numerical_reading(NJDNode *start, NJDNode *end);
static int long_number_digit_reading(NJDNode *start, NJDNode *end);
static int has_quantity_expression(NJDNode *start, NJDNode *node);
static int is_number_hyphen(NJDNode *node);
static int is_latin_word(NJDNode *node);
static NJDNode *next_phone_group(NJDNode *end, int phone_context, int postal_context);
#endif

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
#if defined(CHARSET_UTF_8) && !defined(ASCII_HEADER)
         /* 「1.618」と同じく、漢字の「点」で書いた「一点六一八」の小数部も1桁ずつ読む */
         if (is_decimal_digit(s))
            numerical_reading = -1;
         /* 「モハ205」は短い「ニヒャクゴ」の位取りにし、「3248」のように長い番号は桁読みにする */
         else if (identifier_numerical_reading(s, final_digit)) {
            numerical_reading = 1;
            /* 「COVID-19」「AB-12」の英字の語の後のハイフンは、休止を置かずに語と数を別のアクセント句で続けて読む (「コ＼ビッド」「ジューキュ＼ー」) */
            if (s->prev != NULL && is_number_hyphen(s->prev) && is_latin_word(s->prev->prev))
               NJDNode_set_pos_group3(s->prev, "数の区切り");
         }
         /* 「5823901746283915」のように兆以上の位が要る長い数字列は、位取りが短い「1000000000000」を除いて桁読みにする */
         else if (long_number_digit_reading(s, final_digit))
            numerical_reading = -1;
         else
#endif
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

/* 8 の後で「ハッ」と「ハチ」の両方に読める助数詞のうち、「ハチ」を既定にするもの */
static const char *njd_set_digit_rule_hachi_counters[] = {
   "か国", "か所", "か月", "か条", "件", "体", "個", "分", "品", "回", "地区", "地点", "坪", "基",
   "局", "巻", "店舗", "曲", "期", "校", "桁", "機", "歩", "版", "票", "箱", "粒", "級", "編", "羽",
   "貫", "貫目", "軒", "階", "騎", "派", "脚",
   NULL
};

static int ends_with_sokuon_or_hatsuon(const char *pron)
{
   size_t length = strlen(pron);
   return length >= strlen("ッ") &&
          (strcmp(pron + length - strlen("ッ"), "ッ") == 0 || strcmp(pron + length - strlen("ン"), "ン") == 0);
}

static int is_sokuon_odaka_counter(NJDNode *counter)
{
   static const char *counters[] = {
      "発", "匹", "冊", "室", "隻", "拍", "泊", "客", "脚", "曲", "局", "尺", "色", "食", "節", "滴", NULL
   };
   int i;
   for (i = 0; counters[i] != NULL; i++)
      if (strcmp(NJDNode_get_string(counter), counters[i]) == 0)
         return 1;
   return 0;
}

#if defined(CHARSET_UTF_8) && !defined(ASCII_HEADER)
static int is_time_of_day_word(NJDNode *node)
{
   static const char *words[] = {"午前", "午後", "朝", "夕", "夜", "未明", "早朝", "深夜", "夕方", "正午", "昼", "付",
                                 "付け", NULL};
   int i;
   if (node == NULL)
      return 0;
   for (i = 0; words[i] != NULL; i++)
      if (strcmp(NJDNode_get_string(node), words[i]) == 0)
         return 1;
   return 0;
}

static int is_news_date_context(NJDNode *following)
{
   NJDNode *node;
   /* 報道の書き出しの「1日、」「政府は1日、」や「1日の夜」「一日午後」「1日付」の「1日」は、月がなくても日付の「ツイタチ」と読む */
   /* 日数の「1日に3回」「1日かかる」「1日の生活」は「イチニチ」のまま読む */
   if (following == NULL)
      return 0;
   if (strcmp(NJDNode_get_string(following), "、") == 0 || strcmp(NJDNode_get_string(following), "，") == 0 ||
       is_time_of_day_word(following))
      return 1;
   if (strcmp(NJDNode_get_string(following), "の") == 0)
      return is_time_of_day_word(following->next);
   /* 「1日と2日を休みに」のように日付を並べる形と、「1日から8月です」のように月の名前が続く形も日付として読む */
   if (strcmp(NJDNode_get_string(following), "と") == 0) {
      for (node = following->next; node != NULL && strcmp(NJDNode_get_pos_group1(node), NJD_SET_DIGIT_KAZU) == 0;
           node = node->next);
      return node != NULL && node != following->next && strcmp(NJDNode_get_string(node), NJD_SET_DIGIT_NICHI) == 0;
   }
   if (strcmp(NJDNode_get_string(following), "から") == 0 && following->next != NULL)
      return strstr(NJDNode_get_string(following->next), NJD_SET_DIGIT_GATSU) != NULL;
   return 0;
}

static int is_day_count_context(NJDNode *digit, NJDNode *following)
{
   static const char *prefixes[] = {"第", "丸", "まる", "毎", "各", "約", "ほぼ", "たった", "わずか", NULL};
   static const char *day_words[] = {"今日", "きょう", "本日", "明日", "あした", "あす", "昨日", "きのう", NULL};
   NJDNode *node = digit->prev;
   int i;
   /* 「大会第1日、」の序数や、「丸1日、」「約1日、」の期間を表す語の後の「1日」は、読点が続いても日数の「イチニチ」と読む */
   /* 「今日も1日、」「今日1日、」のように、日を指す語の後で「その日のあいだ」を表す「1日」も日数として読む */
   /* 日を指す語の後でも、「本日1日付で」「明日1日午前10時に」のように「付」や時間帯の語が続くときは日付なので、読点が続くときだけにする */
   if (node == NULL)
      return 0;
   for (i = 0; prefixes[i] != NULL; i++)
      if (strcmp(NJDNode_get_string(node), prefixes[i]) == 0)
         return 1;
   if (strcmp(NJDNode_get_string(following), "、") != 0 && strcmp(NJDNode_get_string(following), "，") != 0)
      return 0;
   if (strcmp(NJDNode_get_string(node), "も") == 0 && node->prev != NULL)
      node = node->prev;
   for (i = 0; day_words[i] != NULL; i++)
      if (strcmp(NJDNode_get_string(node), day_words[i]) == 0)
         return 1;
   return 0;
}

static int is_counter_after_digits(NJDNode *node)
{
   return node != NULL && (strcmp(NJDNode_get_pos_group2(node), NJD_SET_DIGIT_JOSUUSHI) == 0 ||
                           search_numerative_class(njd_set_digit_rule_counter_words, node));
}

static const char *ratio_or_fraction_reading_of_fun(NJDNode *fun)
{
   NJDNode *node;
   /* 「3割2分」「5分5厘」の「分」は割合の単位なので「ブ」と読む */
   for (node = fun->prev; node != NULL && strcmp(NJDNode_get_pos_group1(node), NJD_SET_DIGIT_KAZU) == 0;
        node = node->prev);
   if (node != NULL && strcmp(NJDNode_get_string(node), "割") == 0)
      return "ブ";
   for (node = fun->next; node != NULL && strcmp(NJDNode_get_pos_group1(node), NJD_SET_DIGIT_KAZU) == 0;
        node = node->next);
   if (node == fun->next)
      return NULL;
   if (node != NULL && strcmp(NJDNode_get_string(node), "厘") == 0)
      return "ブ";
   /* 「千分一」「四分三」のように「の」を省いて書いた分数は「ブン」と読む */
   /* 「3分5秒」のように後ろの数に助数詞が続くときは時間の「フン」のままにする */
   if (is_counter_after_digits(node))
      return NULL;
   return "ブン";
}
#endif

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
      /* 「場所」「選」「機種」「地区」は辞書に名詞,一般 と名詞,接尾,一般 の行しかないが、数詞に続くときは助数詞としてアクセントを決める */
      /* 助数詞の行を辞書に足すと、「同一機種」「唯一地区」の「一」まで数詞に分けて「ドーイチキシュ」と読むので、名詞,一般 の行のまま扱う */
      if (strcmp(NJDNode_get_pos_group1(node), NJD_SET_DIGIT_KAZU) != 0 ||
          (strcmp(NJDNode_get_pos_group2(counter), NJD_SET_DIGIT_JOSUUSHI) != 0 &&
           strcmp(NJDNode_get_pos_group1(counter), NJD_SET_DIGIT_FUKUSHIKANOU) != 0 &&
           strcmp(NJDNode_get_string(counter), "場所") != 0 && strcmp(NJDNode_get_string(counter), "選") != 0 &&
           strcmp(NJDNode_get_string(counter), "機種") != 0 && strcmp(NJDNode_get_string(counter), "地区") != 0))
         continue;
      digit = get_digit(node, 0);
      is_compound = node->prev != NULL &&
         strcmp(NJDNode_get_pos_group1(node->prev), NJD_SET_DIGIT_KAZU) == 0;

      /* 「個」は数詞の末尾にアクセント核を置く前部末型で結合する */
      /* 「選」も数詞の後では前部末型で「サ＼ンセン」「ゴ＼セン」と読み、辞書の接尾辞の行の平板型は使わない */
      if (strcmp(NJDNode_get_string(counter), "個") == 0 || strcmp(NJDNode_get_string(counter), "選") == 0)
         NJDNode_set_chain_rule(counter, "C3");
      /* 「機種」「地区」は助数詞の核を保って「イチキ＼シュ」「イチチ＼ク」と読む */
      if (strcmp(NJDNode_get_string(counter), "機種") == 0 || strcmp(NJDNode_get_string(counter), "地区") == 0)
         NJDNode_set_chain_rule(counter, "C1");

      /* 「人」は2桁以上の数と「六」「七」「八」「九 (キュー)」の後で前部末型にし、1拍で読む「四 (ヨ)」「五」「九 (ク)」の後では助数詞のアクセント核をそのまま使う */
      /* 1語で数を表す「十」「百」「千」「何」の後も、2桁以上の数と同じく前部末型にする (「ジュ＼ーニン」「ヒャク＼ニン」「ナ＼ンニン」) */
      if (strcmp(NJDNode_get_string(counter), NJD_SET_DIGIT_NIN) == 0 &&
          ((is_compound && !(NJDNode_get_mora_size(node) == 1 &&
                            (digit == 4 || digit == 5 || digit == 9))) ||
           (!is_compound && (digit == 6 || digit == 7 || digit == 8 ||
                            (digit == 9 && NJDNode_get_mora_size(node) == 2) ||
                            strcmp(NJDNode_get_string(node), NJD_SET_DIGIT_TEN) == 0 ||
                            strcmp(NJDNode_get_string(node), njd_set_digit_rule_numeral_list5[1]) == 0 ||
                            strcmp(NJDNode_get_string(node), njd_set_digit_rule_numeral_list5[2]) == 0 ||
                            strcmp(NJDNode_get_string(node), "何") == 0))))
         NJDNode_set_chain_rule(counter, "C3");

      /* 前部末型でアクセント核が撥音・長音・促音に当たる数詞は、核を1拍前へずらす */
      if (strcmp(NJDNode_get_chain_rule(counter), "C3") == 0 &&
          (digit == 3 || ((digit == 4 || digit == 9) && NJDNode_get_mora_size(node) == 2) ||
           strcmp(NJDNode_get_string(node), NJD_SET_DIGIT_TEN) == 0 ||
           strcmp(NJDNode_get_string(node), njd_set_digit_rule_numeral_list5[2]) == 0 ||
           strcmp(NJDNode_get_pron(node), "ナン") == 0))
         NJDNode_set_chain_rule(counter, "F4@-1");

      /* 「場所」は和語の「ヒト」「フタ」の後だけ前部末型にし (「ヒト＼バショ」)、ほかの数の後は平板型にする (「サンバショ」「ジューバショ」) */
      /* 名詞,一般 の行は複合語 (「居場所」「置き場所」) の結合規則を持つので、辞書の行でなくここで数詞の後だけを変える */
      if (strcmp(NJDNode_get_string(counter), "場所") == 0)
         NJDNode_set_chain_rule(counter, !is_compound && (strcmp(NJDNode_get_read(node), "ヒト") == 0 ||
                                                         strcmp(NJDNode_get_read(node), "フタ") == 0) ? "C3" : "C4");

      /* 「尺」は二・五・六の後、「節」は六の後、「足」は一の後で尾高型にする (「ニシャク＼」「ロクセツ＼」「イッソク＼」) */
      /* 11〜19 の一の位も同じ型にし (「ジューロクシャク＼」「ジュ＼ー・ゴシャク＼」)、21以上の数の後は前部末型のままにする */
      if ((!is_compound ||
           (strcmp(NJDNode_get_string(node->prev), NJD_SET_DIGIT_TEN) == 0 &&
            (node->prev->prev == NULL ||
             strcmp(NJDNode_get_pos_group1(node->prev->prev), NJD_SET_DIGIT_KAZU) != 0))) &&
          ((strcmp(NJDNode_get_string(counter), "尺") == 0 && (digit == 2 || digit == 5 || digit == 6)) ||
           (strcmp(NJDNode_get_string(counter), "節") == 0 && digit == 6) ||
           (strcmp(NJDNode_get_string(counter), "足") == 0 && digit == 1 && !is_compound)))
         NJDNode_set_chain_rule(counter, "F4@2");

      /* 「石」は一・六・八の後と、単独の「十」の後で尾高型にする (「イッコク＼」「ジュッコク＼」) */
      if (strcmp(NJDNode_get_string(counter), "石") == 0 &&
          (digit == 1 || digit == 6 || digit == 8 ||
           (!is_compound && strcmp(NJDNode_get_string(node), NJD_SET_DIGIT_TEN) == 0)))
         NJDNode_set_chain_rule(counter, "F4@2");

      /* 「十一日」「十二日」などは尾高型で結合する */
      /* 順番を表す「第2日」「第10日」の1桁の数と単独の「十」も、同じく一・二・六・七・八・十の後で尾高型にする */
      /* 「1.2日」の小数部や「2、3日」の概数は「ニ＼ニチ」のままにするので、「第」の後に限る */
      if (strcmp(NJDNode_get_string(counter), NJD_SET_DIGIT_NICHI) == 0 &&
          ((is_compound && (digit == 1 || digit == 2 || digit == 6 || digit == 7 || digit == 8)) ||
           (!is_compound && node->prev != NULL && strcmp(NJDNode_get_string(node->prev), "第") == 0 &&
            (digit == 1 || digit == 2 || digit == 6 || digit == 7 || digit == 8 ||
             strcmp(NJDNode_get_string(node), NJD_SET_DIGIT_TEN) == 0))))
         NJDNode_set_chain_rule(counter, "F4@2");

      /* 「一円」「十円」「百円」「千円」などと、「万」に短い助数詞が続く場合 (「一万個」など) は平板型にする */
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

      /* 「五」に続く「本」「枚」「台」「代」「番」「年」「題」「段」「問」は、数詞の桁にかかわらず後部を平板型にする */
      if (digit == 5 && (strcmp(NJDNode_get_string(counter), "本") == 0 ||
                        strcmp(NJDNode_get_string(counter), "枚") == 0 ||
                        strcmp(NJDNode_get_string(counter), "台") == 0 ||
                        strcmp(NJDNode_get_string(counter), "代") == 0 ||
                        strcmp(NJDNode_get_string(counter), "番") == 0 ||
                        strcmp(NJDNode_get_string(counter), "年") == 0 ||
                        strcmp(NJDNode_get_string(counter), "題") == 0 ||
                        strcmp(NJDNode_get_string(counter), "段") == 0 ||
                        strcmp(NJDNode_get_string(counter), "問") == 0))
         NJDNode_set_chain_rule(counter, "F5");
      /* 「三」に続く「番」も、数詞の桁にかかわらず平板型にする (「サンバン」「ニ＼ジュー・サンバン」) */
      /* 11〜19を1句で読む「十三番」は「ジューサ＼ンバン」と核を置くので、前に別の数がない「十」に続く「三」は除く */
      if (digit == 3 && strcmp(NJDNode_get_string(counter), "番") == 0 &&
          !(is_compound && strcmp(NJDNode_get_string(node->prev), NJD_SET_DIGIT_TEN) == 0 &&
            (node->prev->prev == NULL ||
             strcmp(NJDNode_get_pos_group1(node->prev->prev), NJD_SET_DIGIT_KAZU) != 0)))
         NJDNode_set_chain_rule(counter, "F5");
      /* 「年」は1拍で読む「四 (ヨ)」「九 (ク)」の後と、単独の「三」の後でも平板型にする */
      if (strcmp(NJDNode_get_string(counter), "年") == 0 &&
          ((NJDNode_get_mora_size(node) == 1 && (digit == 4 || digit == 9)) ||
           (!is_compound && digit == 3)))
         NJDNode_set_chain_rule(counter, "F5");

      /* 「二十回」などの10の倍数と「千回」は、助数詞より前にアクセント核を置く */
      if (strcmp(NJDNode_get_string(counter), "回") == 0 &&
          (strcmp(NJDNode_get_string(node), njd_set_digit_rule_numeral_list5[2]) == 0 ||
           (is_compound && strcmp(NJDNode_get_string(node), NJD_SET_DIGIT_TEN) == 0)))
         NJDNode_set_chain_rule(counter, "F4@-1");
      /* 「二十選」「二十等」も、「ニジュ＼ッセン」「ニジュ＼ットー」と助数詞より前にアクセント核を置く */
      if ((strcmp(NJDNode_get_string(counter), "選") == 0 || strcmp(NJDNode_get_string(counter), "等") == 0) &&
          is_compound && strcmp(NJDNode_get_string(node), NJD_SET_DIGIT_TEN) == 0)
         NJDNode_set_chain_rule(counter, "F4@-1");
      /* 「発」「匹」「冊」などは、促音で終わる数詞 (「イッ」「ロッ」「ハッ」「ジュッ」「ヒャッ」) に続くときだけ尾高型にする (「イッパツ＼」「ロッピキ＼」) */
      /* ほかの数詞では前部末型のまま「サ＼ンパツ」と読み、同じ促音でも「頭」「点」「個」などは尾高型にしない */
      /* 「二十冊」などの十の倍数は「ニジュ＼ッサツ」と「ジュ」の直後で下がるので、尾高型にしない */
      if (is_sokuon_odaka_counter(counter) &&
          !(is_compound && strcmp(NJDNode_get_string(node), NJD_SET_DIGIT_TEN) == 0) &&
          strlen(NJDNode_get_pron(node)) >= 3 &&
          strcmp(NJDNode_get_pron(node) + strlen(NJDNode_get_pron(node)) - 3, "ッ") == 0) {
         char rule[8];
         snprintf(rule, sizeof(rule), "F4@%d", NJDNode_get_mora_size(counter));
         NJDNode_set_chain_rule(counter, rule);
      }
      /* 「二十点」「二十戦」などの10の倍数も、「ニジュ＼ッテン」「ニジュ＼ッセン」と助数詞より前にアクセント核を置く */
      if ((strcmp(NJDNode_get_string(counter), "点") == 0 ||
           strcmp(NJDNode_get_string(counter), "戦") == 0) &&
          is_compound && strcmp(NJDNode_get_string(node), NJD_SET_DIGIT_TEN) == 0)
         NJDNode_set_chain_rule(counter, "F4@-1");

      /* 袖丈の「1分袖」「10分袖」は、「ブ」と読む「分」と名詞の「袖」の間で句を切らず、「イチブ＼ソデ」「ジューブ＼ソデ」と「ブ」の後で下げる */
      if (strcmp(NJDNode_get_string(counter), "分") == 0 && strcmp(NJDNode_get_pron(counter), "ブ") == 0 &&
          counter->next != NULL && strcmp(NJDNode_get_string(counter->next), "袖") == 0) {
         NJDNode_set_chain_flag(counter->next, 1);
         NJDNode_set_chain_rule(counter->next, "C3");
      }

      /* 「日目」は尾高型、「人前」は平板型にし、別のノードに分かれた「目」「前」まで1つのアクセント句にまとめる */
      if (counter->next != NULL && NJDNode_get_chain_flag(counter->next) != 0 &&
          strcmp(NJDNode_get_pos_group1(counter->next), "接尾") == 0) {
         /* 「幕目」も「日目」と同じく尾高型にする (「ナナマクメ＼」) */
         if ((strcmp(NJDNode_get_string(counter), NJD_SET_DIGIT_NICHI) == 0 ||
              strcmp(NJDNode_get_string(counter), "幕") == 0) &&
             strcmp(NJDNode_get_string(counter->next), "目") == 0)
            NJDNode_set_chain_rule(counter->next, "F4@1");
         if (strcmp(NJDNode_get_string(counter), NJD_SET_DIGIT_NIN) == 0 &&
             strcmp(NJDNode_get_string(counter->next), "前") == 0)
            NJDNode_set_chain_rule(counter->next, "F5");
      }
   }

   /* NHK アクセント辞典の付録の数詞と助数詞の表では、11〜19は「十」と一の位を通常1つのアクセント句で読む */
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

      /* 「時半」「時間」のように助数詞が複数のノードに分かれる場合は、後ろのノードの拍数も合計する */
      counter_mora_size = 0;
      for (tail = counter; tail != NULL; tail = tail->next) {
         if (tail != counter &&
             (NJDNode_get_chain_flag(tail) == 0 ||
              strcmp(NJDNode_get_pos_group1(tail), "接尾") != 0))
            break;
         counter_mora_size += NJDNode_get_mora_size(tail);
      }
      is_short_counter = counter != NULL && counter_mora_size <= 2;

#if defined(CHARSET_UTF_8) && !defined(ASCII_HEADER)
      /* 「第11番」「約11人」のように前に接頭詞などがある11〜19も、1桁の「第3番」(「ダ＼イ・サ＼ンバン」) と同じく「十」から新しいアクセント句を始める */
      /* 一の位の数詞は助数詞の前で句の頭にされているが、ここで「十」とつなぎ直すので、「十」の側で句を切る */
      if (counter != NULL && node->prev != NULL &&
          (is_counter_after_digits(counter) ||
           strcmp(NJDNode_get_pos_group1(counter), NJD_SET_DIGIT_FUKUSHIKANOU) == 0 ||
           strcmp(NJDNode_get_pos_group1(counter), NJD_SET_DIGIT_KAZU) == 0))
         NJDNode_set_chain_flag(node, 0);
#endif

      /* 11〜19に続く「球」「週」などは平板型にし、「機種」「地区」は助数詞のアクセント核をそのまま使う */
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

      /* 次の助数詞は、「十五階」「十五勝」のように一の位が「ゴ」「ヨ」「ク」でも1つのアクセント句で読むので、2つの句に分ける対象から外す */
      if (is_short_counter &&
          (strcmp(NJDNode_get_string(counter), "階") == 0 ||
           strcmp(NJDNode_get_string(counter), "級") == 0 ||
           strcmp(NJDNode_get_string(counter), "型") == 0 ||
           strcmp(NJDNode_get_string(counter), "巡") == 0 ||
           strcmp(NJDNode_get_string(counter), "勝") == 0 ||
           strcmp(NJDNode_get_string(counter), "乗") == 0 ||
           strcmp(NJDNode_get_string(counter), "敗") == 0 ||
           strcmp(NJDNode_get_string(counter), "度目") == 0 ||
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

      /* 一の位を1拍の「ゴ」「ヨ」「ク」で読む数に短い助数詞が続くとき (「十五分」「十四時」「十九時」) は、NHK アクセント辞典に最初に載っている形に合わせて2つのアクセント句に分ける */
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

#if defined(CHARSET_UTF_8) && !defined(ASCII_HEADER)
static void set_kaikyuu_counter_accents(NJD *njd)
{
   NJDNode *node;

   /* 「1階級」「3階級」は辞書の「階」と「級」に分かれ、「階」を建物の階の助数詞として「イッカイ」「サンガイ」と読んでいた */
   /* 「階級」の助数詞の行を辞書に足すと「同一階級」の「一」まで数詞に分けるので、数詞の後の「階」「級」の並びをここで「階級」として読む */
   /* 「一」「八」は促音にせず「イチカ＼イキュー」「ハチカ＼イキュー」、「三」「何」の後も濁らず「サンカ＼イキュー」と読み、「階」の1拍目の後で下げる */
   for (node = njd->head; node != NULL; node = node->next) {
      if (strcmp(NJDNode_get_string(node), "階") != 0 || node->prev == NULL || node->next == NULL ||
          strcmp(NJDNode_get_pos_group1(node->prev), NJD_SET_DIGIT_KAZU) != 0 ||
          strcmp(NJDNode_get_string(node->next), "級") != 0 ||
          strcmp(NJDNode_get_pos_group1(node->next), "接尾") != 0)
         continue;
      if (strcmp(NJDNode_get_string(node->prev), "一") == 0 || strcmp(NJDNode_get_string(node->prev), "八") == 0) {
         NJDNode_set_read(node->prev, strcmp(NJDNode_get_string(node->prev), "一") == 0 ? "イチ" : "ハチ");
         NJDNode_set_pron(node->prev, strcmp(NJDNode_get_string(node->prev), "一") == 0 ? "イチ" : "ハチ");
         NJDNode_set_mora_size(node->prev, 2);
      }
      NJDNode_set_read(node, "カイ");
      NJDNode_set_pron(node, "カイ");
      NJDNode_set_chain_rule(node, "C2");
      NJDNode_set_chain_rule(node->next, "F1");
   }
}

static void set_man_yen_accents(NJD *njd)
{
   NJDNode *node;
   NJDNode *number;

   /* 「万円」は、「万」の前が1桁の数か単独の「十」「百」「千」なら平板型にし (「ニマンエン」「ジューマンエン」「ヒャクマンエン」) */
   /* 位の字に数が付く「二十万円」「何百万円」「一千万円」と、「数十万円」「十数万円」は「マ」の後で下げる (「ニジューマ＼ンエン」) */
   /* 「十万」「千万」は辞書の1語で解析されることがあるので、前に数がある「十万」は位の字に数が付く形として扱う */
   /* 「二十五万円」のように「五」から句を分けた数は、後ろの句を1桁の数と同じく平板型の「ゴマンエン」にする */
   /* 11〜19の一の位は set_digit_accent_rules() の最後で「十」につなぎ直されるので、句の切れ目が決まったここで判定する */
   for (node = njd->head; node != NULL && node->next != NULL; node = node->next) {
      if (strcmp(NJDNode_get_string(node->next), "円") != 0 ||
          strcmp(NJDNode_get_pos_group1(node), NJD_SET_DIGIT_KAZU) != 0 ||
          (strcmp(NJDNode_get_string(node), njd_set_digit_rule_numeral_list5[3]) != 0 &&
           strcmp(NJDNode_get_string(node), "十万") != 0 && strcmp(NJDNode_get_string(node), "千万") != 0))
         continue;
      /* 「万」の前の数を、「十万」「千万」は自分自身を、位の字に数が付く形かを見る起点にする */
      /* 「二〇 万円」は「万」の前に無音の空白境界があり、後処理で前の数とつなぐので、境界を読み飛ばして空白のない形と同じく判定する */
      number = node;
      if (strcmp(NJDNode_get_string(node), njd_set_digit_rule_numeral_list5[3]) == 0)
         for (number = node->prev; number != NULL && strcmp(NJDNode_get_pos_group3(number), "空白境界") == 0;
              number = number->prev);
      if (number != NULL && number != node && strcmp(NJDNode_get_pos_group1(number), NJD_SET_DIGIT_KAZU) != 0)
         continue;
      if (number != NULL && number->prev != NULL && NJDNode_get_chain_flag(number) == 1 &&
          strcmp(NJDNode_get_pos_group1(number->prev), NJD_SET_DIGIT_KAZU) == 0)
         NJDNode_set_chain_rule(node->next, "F4@-1");
      else
         NJDNode_set_chain_rule(node->next, "F5");
   }
}
#endif

#if defined(CHARSET_UTF_8) && !defined(ASCII_HEADER)
/* 番号の各桁を通常の位取り変換から外し、既存の NJD ノードのまま読む */
typedef struct NJDNumberSequence {
   NJDNode *start;
   NJDNode *end;
   struct NJDNumberSequence *next;
} NJDNumberSequence;

static int number_digit(NJDNode *node)
{
   const char *str;
   if (node == NULL)
      return -1;
   str = NJDNode_get_string(node);
   /* 「０」「〇」「七」のような1桁の表記だけを扱い、「十」「百」やローマ数字は位取りに残す */
   if (strlen(str) != 3 ||
       strstr("０１２３４５６７８９〇零一二三四五六七八九", str) == NULL)
      return -1;
   return get_digit(node, 0);
}

static NJDNode *number_end(NJDNode *start, int *size)
{
   NJDNode *end = start;
   *size = 1;
   while (number_digit(end->next) >= 0) {
      end = end->next;
      (*size)++;
   }
   return end;
}

static int is_number_hyphen(NJDNode *node)
{
   const char *str = NJDNode_get_string(node);
   /* 「070-3224-5679」「070ー3224ー5679」の数字間の区切りを同じ休止として扱う */
   return strcmp(str, "−") == 0 || strcmp(str, "－") == 0 || strcmp(str, "ー") == 0 ||
          strcmp(str, "‐") == 0 || strcmp(str, "‑") == 0 || strcmp(str, "‒") == 0 ||
          strcmp(str, "–") == 0 || strcmp(str, "—") == 0 ||
          strcmp(str, "-") == 0;
}

static int is_special_phone_number(NJDNode *start, NJDNode *end)
{
   static const int numbers[] = {104, 110, 113, 115, 117, 118, 119, 171, 177, 184, 186, 188, 189};
   NJDNode *node;
   int value = 0, size = 0, digit;
   size_t i;
   /* 番号案内・警察・故障受付・電報・時報・海上保安・消防・災害用伝言ダイヤル・天気予報・番号の通知・消費者ホットライン・児童相談の、広く知られた 1XY の番号 */
   /* 数字の並びごとに呼ばれるので、「09012345678」のような長い並びで値を組み立てて int の範囲を超えないよう、先に3桁かを確かめる */
   for (node = start; node != end->next; node = node->next) {
      if (number_digit(node) < 0)
         return 0;
      size++;
   }
   if (size != 3)
      return 0;
   for (node = start; node != end->next; node = node->next) {
      digit = number_digit(node);
      value = value * 10 + digit;
   }
   for (i = 0; i < sizeof(numbers) / sizeof(numbers[0]); i++)
      if (value == numbers[i])
         return 1;
   return 0;
}

static int number_size(NJDNode *start, NJDNode *end)
{
   NJDNode *node;
   int size = 0;
   for (node = start; node != end->next; node = node->next)
      size++;
   return size;
}

static int follows_phone_service_name(NJDNode *start)
{
   static const char *services[] = {"番号案内", "時報", "天気予報", "伝言", "伝言ダイヤル", "災害用伝言ダイヤル",
                                    "警察", "消防", "救急", NULL};
   NJDNode *node = start->prev;
   char buff[MAXBUFLEN];
   int i;
   /* 「番号案内は104です」「時報は117」のように、その番号のサービスの名前が助詞を挟んで前にあるかを見る */
   while (node != NULL && (strcmp(NJDNode_get_pos_group3(node), "空白境界") == 0 ||
                           (strcmp(NJDNode_get_pos(node), "助詞") == 0 &&
                            (strcmp(NJDNode_get_string(node), "は") == 0 ||
                             strcmp(NJDNode_get_string(node), "が") == 0 ||
                             strcmp(NJDNode_get_string(node), "の") == 0 ||
                             strcmp(NJDNode_get_string(node), "も") == 0))))
      node = node->prev;
   if (node == NULL)
      return 0;
   for (i = 0; services[i] != NULL; i++) {
      if (strcmp(NJDNode_get_string(node), services[i]) == 0)
         return 1;
      /* 「番号」「案内」のように2語に分けて解析されたサービスの名前もつなげて見る */
      if (node->prev != NULL) {
         snprintf(buff, sizeof(buff), "%s%s", NJDNode_get_string(node->prev), NJDNode_get_string(node));
         if (strcmp(buff, services[i]) == 0)
            return 1;
      }
   }
   return 0;
}

static int has_phone_context(NJDNode *start, NJDNode *end)
{
   NJDNode *node;
   int distance = 0;
   /* 「119に電話する」「188に相談」「110へ通報」のように、広く知られた 1XY の番号に電話をかける語が続くときは発信先の番号として桁読みする */
   /* 間の助詞は「に」「へ」「を」「で」を許し、「100に電話料金を足す」のような普通の数や、「話者1に電話」のように名詞へ付く数字は対象にしない */
   node = end->next;
   while (node != NULL && strcmp(NJDNode_get_pos_group3(node), "空白境界") == 0)
      node = node->next;
   if (is_special_phone_number(start, end) &&
       (start->prev == NULL || strcmp(NJDNode_get_pos(start->prev), "名詞") != 0)) {
      if (node != NULL && strcmp(NJDNode_get_pos(node), "助詞") == 0 &&
          (strcmp(NJDNode_get_string(node), "に") == 0 || strcmp(NJDNode_get_string(node), "へ") == 0 ||
           strcmp(NJDNode_get_string(node), "を") == 0 || strcmp(NJDNode_get_string(node), "で") == 0)) {
         node = node->next;
         while (node != NULL && strcmp(NJDNode_get_pos_group3(node), "空白境界") == 0)
            node = node->next;
      }
      if (node != NULL &&
          (strcmp(NJDNode_get_string(node), "電話") == 0 || strcmp(NJDNode_get_string(node), "通報") == 0 ||
           strcmp(NJDNode_get_string(node), "伝言") == 0 ||
           strcmp(NJDNode_get_string(node), "連絡") == 0 || strcmp(NJDNode_get_string(node), "ダイヤル") == 0 ||
           strcmp(NJDNode_get_string(node), "相談") == 0 || strcmp(NJDNode_get_string(node), "コール") == 0 ||
           (strcmp(NJDNode_get_pos(node), "動詞") == 0 &&
            (strcmp(NJDNode_get_orig(node), "かける") == 0 || strcmp(NJDNode_get_orig(node), "掛ける") == 0))))
         return 1;
      if (follows_phone_service_name(start))
         return 1;
   }
   /* 「市外局番213の、486ー2435」「電話 03 1234 5678」は区切りと数字をたどり、電話番号の最後の組まで文脈を保つ */
   for (node = start->prev; node != NULL && distance < 16; node = node->prev, distance++) {
      /* 見出しが「電話」だけのときは、「電話は100ある」のような3桁以下の数を数量として位取りで読む */
      /* 「電話は03の1234の5678」のように後ろに番号の組が続くときは、最初の組が3桁以下でも電話番号として読む */
      if (strcmp(NJDNode_get_string(node), "電話") == 0 && number_size(start, end) <= 3 &&
          next_phone_group(end, 1, 0) == NULL)
         break;
      if (strcmp(NJDNode_get_string(node), "電話") == 0 ||
          strcmp(NJDNode_get_string(node), "電話番号") == 0 ||
          strcmp(NJDNode_get_string(node), "ファクス") == 0 ||
          strcmp(NJDNode_get_string(node), "市外局番") == 0 ||
          strcmp(NJDNode_get_string(node), "局番") == 0)
         return 1;
      /* 「受け付け電話番号は、03・3355・1881」のように「電話」「番号」が別々に解析されても、隣り合う語を番号の見出しとして扱う */
      if (strcmp(NJDNode_get_string(node), "番号") == 0 && node->prev != NULL &&
          strcmp(NJDNode_get_string(node->prev), "電話") == 0)
         return 1;
      /* 「電話で100と200を足す」「電話料金は1.5円」は、番号の区切りに当たらない語で探索を終える */
      if (number_digit(node) < 0 && !is_number_hyphen(node) &&
          strcmp(NJDNode_get_pos_group3(node), "空白境界") != 0 &&
          strcmp(NJDNode_get_string(node), "（") != 0 &&
          strcmp(NJDNode_get_string(node), "）") != 0 &&
          strcmp(NJDNode_get_string(node), "の") != 0 &&
          strcmp(NJDNode_get_string(node), "は") != 0 &&
          strcmp(NJDNode_get_pron(node), "、") != 0)
         break;
   }
   return 0;
}

static NJDNode *next_phone_group(NJDNode *end, int phone_context, int postal_context)
{
   NJDNode *separator = end->next;
   if (separator == NULL)
      return NULL;
   /* 「03(1234)5678」の括弧と、電話・郵便と確認できる「〒104・8011」の中点もグループ境界になる */
   if (is_number_hyphen(separator) ||
       ((phone_context || postal_context) && strcmp(NJDNode_get_string(separator), "・") == 0) ||
       strcmp(NJDNode_get_string(separator), "（") == 0 ||
       strcmp(NJDNode_get_string(separator), "）") == 0 ||
       strcmp(NJDNode_get_pos_group3(separator), "空白境界") == 0)
      separator = separator->next;
   /* 「電話番号213の486の2435」の「の」は発音を残し、後ろの数字だけ桁読みにする */
   else if (phone_context && strcmp(NJDNode_get_string(separator), "の") == 0) {
      separator = separator->next;
      while (separator != NULL && strcmp(NJDNode_get_pron(separator), "、") == 0)
         separator = separator->next;
   } else {
      return NULL;
   }
   return number_digit(separator) >= 0 ? separator : NULL;
}

static int has_postal_context(NJDNode *start)
{
   NJDNode *node;
   int distance = 0;
   /* 「〒1234567」「郵便番号1234567」は、ハイフンがなくても7桁を桁読みする */
   for (node = start->prev; node != NULL && distance < 8; node = node->prev, distance++) {
      if (strcmp(NJDNode_get_string(node), "〒") == 0 ||
          strcmp(NJDNode_get_string(node), "郵便番号") == 0)
         return 1;
      /* 「郵便番号102-8661」で「郵便」「番号」が別々に解析されても、隣り合う語を郵便番号の見出しとして扱う */
      if (strcmp(NJDNode_get_string(node), "番号") == 0 && node->prev != NULL &&
          strcmp(NJDNode_get_string(node->prev), "郵便") == 0)
         return 1;
      if (strcmp(NJDNode_get_string(node), "。") == 0)
         break;
   }
   return 0;
}

static int has_identifier_context(NJDNode *start)
{
   NJDNode *node;
   int distance = 0;
   /* 「モハ205-3248」「型番123-4567」は、3桁と4桁の組でも形式番号として読み、記号の発音を保つ */
   for (node = start->prev; node != NULL && distance < 16; node = node->prev, distance++) {
      if (strcmp(NJDNode_get_string(node), "型番") == 0 ||
          strcmp(NJDNode_get_string(node), "型式") == 0 ||
          strcmp(NJDNode_get_string(node), "形式") == 0 ||
          strcmp(NJDNode_get_string(node), "ＥＦ") == 0 ||
          strcmp(NJDNode_get_string(node), "ＥＤ") == 0 ||
          strcmp(NJDNode_get_string(node), "ＥＨ") == 0 ||
          strcmp(NJDNode_get_string(node), "ＤＤ") == 0 ||
          strcmp(NJDNode_get_string(node), "ＤＥ") == 0 ||
          strcmp(NJDNode_get_string(node), "モハ") == 0 ||
          strcmp(NJDNode_get_string(node), "クハ") == 0 ||
          strcmp(NJDNode_get_string(node), "キハ") == 0)
         return 1;
      /* 「型番3248の商品を9876円で購入」は、型番と後ろの数量の間にある語で探索を終える */
      if (number_digit(node) < 0 && !is_number_hyphen(node) &&
          strcmp(NJDNode_get_pos_group3(node), "空白境界") != 0)
         break;
   }
   return 0;
}

static int protect_number_sequence(NJDNumberSequence **sequences, NJDNode *start, NJDNode *end)
{
   NJDNode *node;
   NJDNumberSequence *sequence = (NJDNumberSequence *) calloc(1, sizeof(NJDNumberSequence));
   if (sequence == NULL) {
      fprintf(stderr, "WARNING: Failed to allocate number sequence in njd_set_digit.\n");
      return 0;
   }
   sequence->start = start;
   sequence->end = end;
   sequence->next = *sequences;
   *sequences = sequence;
   /* 「070」の0を位取りで消したり、最後の2を助数詞規則で変えたりしないよう、数詞処理の間だけ一般名詞にする */
   for (node = start; node != end->next; node = node->next)
      NJDNode_set_pos_group1(node, "一般");
   return 1;
}

static void set_phone_digit_reading(NJDNode *start, NJDNode *end, int first_group_size)
{
   static const char *readings[] = {
      "ゼロ", "イチ", "ニー", "サン", "ヨン", "ゴー", "ロク", "ナナ", "ハチ", "キュー"
   };
   static const int accents[] = {1, 2, 1, 0, 1, 1, 1, 1, 1, 1};
   NJDNode *node;
   int digit, previous_digit = 0, index = 0, group_index = 0;
   int total = number_size(start, end);
   int second_group_size = first_group_size == 4 ? 3 : 4;
   int group_size = first_group_size > 0 ? first_group_size : total;
   for (node = start; node != end->next; node = node->next, index++) {
      /* 「07032245679」は休止のない3-4-4の組として数え、組ごとにアクセント句を分ける */
      if (first_group_size > 0 &&
          (index == first_group_size || index == first_group_size + second_group_size)) {
         group_index = 0;
         group_size = index == first_group_size ? second_group_size : total - first_group_size - second_group_size;
      }
      /* 保護中は品詞を一時変更しているため、値を調べる間だけ数詞に戻す */
      NJDNode_set_pos_group1(node, NJD_SET_DIGIT_KAZU);
      digit = number_digit(node);
      NJDNode_set_pos_group1(node, "一般");
      NJDNode_set_read(node, (char *) readings[digit]);
      NJDNode_set_pron(node, (char *) readings[digit]);
      /* 3桁の組の真ん中の0は、前後が0以外のときだけ部屋番号と同じく「マル」と読む (「104」は「イチマルヨン」、「802」は「ハチマルニー」) */
      /* 0が続く「500」「009」と、組の先頭と末尾の0 (「110」「070」)、4桁の組の0は「ゼロ」のまま読む */
      if (digit == 0 && group_size == 3 && group_index == 1 && previous_digit != 0) {
         NJDNode_set_pos_group1(node->next, NJD_SET_DIGIT_KAZU);
         if (number_digit(node->next) != 0) {
            NJDNode_set_read(node, "マル");
            NJDNode_set_pron(node, "マル");
         }
         NJDNode_set_pos_group1(node->next, "一般");
      }
      previous_digit = digit;
      NJDNode_set_mora_size(node, 2);
      NJDNode_set_acc(node, accents[digit]);
      NJDNode_set_chain_rule(node, "C5");
      NJDNode_set_chain_flag(node, group_index % 2 == 0 ? 0 : 1);
      /* 「32」は4モーラの核3で「サンニ＼ー」、「123」の最後の3は平板の別句にする */
      if (group_index % 2 == 1)
         NJDNode_set_acc(node->prev, 3);
      group_index++;
   }
}

static void set_postal_zero_reading(NJDNode *start, NJDNode *end)
{
   NJDNode *node;
   int has_nonzero = 0, digit;
   /* 郵便番号の「102」「100」は、0以外の数字の後に続く0を「マル」と読み、組の頭の「0001」の0は「ゼロ」のまま読む */
   for (node = start; node != end->next; node = node->next) {
      NJDNode_set_pos_group1(node, NJD_SET_DIGIT_KAZU);
      digit = number_digit(node);
      NJDNode_set_pos_group1(node, "一般");
      if (digit != 0) {
         has_nonzero = 1;
         continue;
      }
      if (has_nonzero == 0)
         continue;
      NJDNode_set_read(node, "マル");
      NJDNode_set_pron(node, "マル");
      NJDNode_set_acc(node, 0);
      /* 「イチマル」は2桁の組で「イチマ＼ル」にする */
      if (node != start && NJDNode_get_chain_flag(node) == 1)
         NJDNode_set_acc(node->prev, 3);
   }
}

static int is_written_digit_sequence(NJDNode *start, NJDNode *end)
{
   NJDNode *node, *following = end->next;
   int i, has_zero = 0;
   for (node = start; node != end->next; node = node->next)
      if (strcmp(NJDNode_get_string(node), "〇") == 0)
         has_zero = 1;
   /* 「二〇 万円」の無音の空白境界を読み飛ばし、「二〇万円」と同じく「ニジューマンエン」と読む */
   while (following != NULL && strcmp(NJDNode_get_pos_group3(following), "空白境界") == 0)
      following = following->next;
   /* 「二〇万円」「一二〇万円」は「〇」を含み、百以上の位が続くので「ニジューマンエン」「ヒャクニジューマンエン」と数量として読む */
   /* 「〇」のない漢数字列は概数を表すことがあるため、この数量への変換の対象から外す */
   if (has_zero && following != NULL &&
       strcmp(NJDNode_get_pos_group1(following), NJD_SET_DIGIT_KAZU) == 0)
      for (i = 1; njd_set_digit_rule_numeral_list5[i] != NULL; i++)
         if (strcmp(NJDNode_get_string(following), njd_set_digit_rule_numeral_list5[i]) == 0)
            return 0;
   /* 「三〇・四〇代」「一〇・二〇人」のように中黒で数を並べるときは、後ろの数と同じ読み方にそろえ、「サンジュー・ヨンジューダイ」と読む */
   if (following != NULL && strcmp(NJDNode_get_string(following), "・") == 0 &&
       is_kanji_digit_string(following->next)) {
      for (node = following->next; is_kanji_digit_string(node->next); node = node->next);
      return is_written_digit_sequence(following->next, node);
   }
   /* 「一九九五年」「一一七一円」の数量は位取りを保ち、「一二号室」「八〇二号室」は番号の表記を優先する */
   if (following != NULL && strcmp(NJDNode_get_pos_group2(following), NJD_SET_DIGIT_JOSUUSHI) == 0 &&
       strcmp(NJDNode_get_string(following), "号室") != 0 &&
       strcmp(NJDNode_get_string(following), "号線") != 0 &&
       strcmp(NJDNode_get_string(following), "号機") != 0)
      return 0;
   /* 「一二号室」「八〇二号室」は桁読みの表記、「12号室」の算用数字とは区別する */
   for (node = start; node != end->next; node = node->next) {
      if (strstr("０１２３４５６７８９", NJDNode_get_string(node)) != NULL)
         return 0;
   }
   /* 「三〇九,三〇八」の後ろの組は、「〇」を書いた桁読みの組に「,」で続くので、同じく桁読みの表記とみなす */
   /* 前の組は桁読みとして品詞を一時的に変えているので、数字かどうかは表記で確かめる */
   if (start->prev != NULL && is_comma(NJDNode_get_string(start->prev)) &&
       is_kanji_digit_string(start->prev->prev)) {
      for (node = start->prev->prev; is_kanji_digit_string(node->prev); node = node->prev);
      return start != end && has_written_zero(node, start->prev->prev);
   }
   return start != end &&
          (start->prev == NULL ||
           strcmp(NJDNode_get_pos_group1(start->prev), NJD_SET_DIGIT_KAZU) != 0);
}

static int is_room_or_road_counter(NJDNode *counter)
{
   return counter != NULL &&
          (strcmp(NJDNode_get_string(counter), "号室") == 0 ||
           strcmp(NJDNode_get_string(counter), "号線") == 0);
}

static int is_identifier_counter(NJDNode *counter)
{
   /* 「02番」「01号室」「YDT-03型」は名前として使う番号で、数量を表す「03本」「01個」と区別する */
   return is_room_or_road_counter(counter) ||
          (counter != NULL &&
           (strcmp(NJDNode_get_string(counter), "号機") == 0 ||
            strcmp(NJDNode_get_string(counter), "号車") == 0 ||
            strcmp(NJDNode_get_string(counter), "番線") == 0 ||
            strcmp(NJDNode_get_string(counter), "便") == 0 ||
            strcmp(NJDNode_get_string(counter), "型") == 0 ||
            strcmp(NJDNode_get_string(counter), "番") == 0 ||
            strcmp(NJDNode_get_string(counter), "番地") == 0));
}

static int positional_number_mora_size(NJDNode *start, NJDNode *end)
{
   static const int mora_sizes[4][10] = {
      {0, 2, 1, 2, 2, 1, 2, 2, 2, 2},
      {0, 2, 3, 4, 4, 3, 4, 4, 4, 4},
      {0, 2, 3, 4, 4, 3, 4, 4, 4, 4},
      {0, 2, 3, 4, 4, 3, 4, 4, 4, 4}
   };
   NJDNode *node;
   int position = 0, mora_size = 0, group_nonzero = 0, digit;
   /* 「1001」はセンイチの4モーラ、「2139」はニセンヒャクサンジューキューの11モーラとして比べる */
   for (node = end; ; node = node->prev, position++) {
      digit = number_digit(node);
      if (digit < 0)
         return 0;
      mora_size += mora_sizes[position % 4][digit];
      group_nonzero |= digit != 0;
      if (position % 4 == 3 || node == start) {
         /* 「10000」のイチマンのように、値のある4桁組には万・億などの2モーラを加える */
         if (position >= 4 && group_nonzero)
            mora_size += 2;
         group_nonzero = 0;
      }
      if (node == start)
         break;
   }
   return mora_size;
}

static int prefers_positional_number(NJDNode *start, NJDNode *end, int size)
{
   /* 「2139」の位取りは桁読みより3モーラ長いので位取りを保ち、5モーラ長くなる「3248」は桁読みにする */
   return positional_number_mora_size(start, end) <= size * 2 + 3;
}

static int is_latin_word(NJDNode *node)
{
   const unsigned char *str;
   if (node == NULL)
      return 0;
   str = (const unsigned char *) NJDNode_get_string(node);
   if (*str == '\0')
      return 0;
   while (*str != '\0') {
      /* 全角の「Ａ」〜「Ｚ」と「ａ」〜「ｚ」は、UTF-8 で EF BC A1〜BA と EF BD 81〜9A になる */
      if (str[0] == 0xEF && str[1] == 0xBC && str[2] >= 0xA1 && str[2] <= 0xBA)
         str += 3;
      else if (str[0] == 0xEF && str[1] == 0xBD && str[2] >= 0x81 && str[2] <= 0x9A)
         str += 3;
      else if ((str[0] >= 'A' && str[0] <= 'Z') || (str[0] >= 'a' && str[0] <= 'z'))
         str++;
      else
         return 0;
   }
   return 1;
}

static int identifier_numerical_reading(NJDNode *start, NJDNode *end)
{
   NJDNode *node;
   int size = 0;
   /* 「型番AB-1200」「AB-12」のように英字とハイフンに続く型番も、位取りで短く読めれば「センニヒャク」と位取りで読む */
   /* ハイフンの後の数字は上流の判定で桁読みになるので、英字の語の直後のハイフンを型番の文脈として扱う */
   if (!has_identifier_context(start) &&
       !(start->prev != NULL && is_number_hyphen(start->prev) && is_latin_word(start->prev->prev)))
      return 0;
   for (node = start; ; node = node->next) {
      if (number_digit(node) < 0)
         return 0;
      size++;
      if (node == end)
         break;
   }
   return prefers_positional_number(start, end, size);
}

static int long_number_digit_reading(NJDNode *start, NJDNode *end)
{
   NJDNode *node;
   int size = 0;
   for (node = start; ; node = node->next) {
      if (number_digit(node) < 0)
         return 0;
      size++;
      if (node == end)
         break;
   }
   /* 区切りのない13桁以上の数は数量として書かれることがまれで、位取りすると「チョー」「ケー」を含む長い読みになる */
   /* 12桁以下は「3248」のような数量も位取りで読むので、型番などの文脈がない限りこの判定に含めない */
   if (size < 13 || prefers_positional_number(start, end, size))
      return 0;
   /* 京以上の位は数量でもふつう口にしないので、17桁以上は位取りの方が短い場合を除いて、文脈によらず桁読みする */
   if (size > 16)
      return 1;
   node = end->next;
   while (node != NULL && strcmp(NJDNode_get_pos_group3(node), "空白境界") == 0)
      node = node->next;
   /* 「1234567890123円」「1234567890123.45」は直後の助数詞や小数点で数量と分かるので、位取りで読む */
   /* 前の負号で手がかりの合計が打ち消される「-1234567890123円」も、直後の助数詞を先に見て数量として読む */
   if (node != NULL &&
       (strcmp(NJDNode_get_pos_group2(node), NJD_SET_DIGIT_JOSUUSHI) == 0 ||
        strcmp(NJDNode_get_pos_group1(node), NJD_SET_DIGIT_FUKUSHIKANOU) == 0 ||
        is_decimal_point(node)))
      return 0;
   /* 「約1234567890123」は前の数接続の語で数量と分かるので、位取りで読む */
   if (get_digit_sequence_score(start, end) > 0)
      return 0;
   /* 「1234567890123以上」「1234567890123に増えた」の数量表現が続く数も、位取りで読む */
   return !has_quantity_expression(start, node);
}

static void set_identifier_digit_reading(NJDNode *start, NJDNode *end)
{
   NJDNode *node, *counter = end->next;
   int digit;
   set_phone_digit_reading(start, end, 0);
   /* 「802号室」「一〇二」は0をマルと読むが、電話・郵便の0はゼロのままにする */
   for (node = start; node != end->next; node = node->next) {
      NJDNode_set_pos_group1(node, NJD_SET_DIGIT_KAZU);
      digit = number_digit(node);
      NJDNode_set_pos_group1(node, "一般");
      if (digit == 0) {
         NJDNode_set_read(node, "マル");
         NJDNode_set_pron(node, "マル");
         NJDNode_set_acc(node, 0);
      }
      if (node != start && NJDNode_get_chain_flag(node) == 1)
         NJDNode_set_acc(node->prev, 3);
   }
   /* 「802号室」の末尾は「ニゴ＼ーシツ」で結合し、助数詞の直前の2・5は1モーラに戻す */
   if (counter != NULL && strcmp(NJDNode_get_pos_group2(counter), NJD_SET_DIGIT_JOSUUSHI) == 0) {
      NJDNode_set_pos_group1(end, NJD_SET_DIGIT_KAZU);
      digit = number_digit(end);
      NJDNode_set_pos_group1(end, "一般");
      if (digit == 2 || digit == 5) {
         NJDNode_set_read(end, digit == 2 ? "ニ" : "ゴ");
         NJDNode_set_pron(end, digit == 2 ? "ニ" : "ゴ");
         NJDNode_set_mora_size(end, 1);
      }
      NJDNode_set_chain_flag(counter, 1);
      NJDNode_set_chain_rule(counter, "C1");
      /* 「国道409号線」の最後の「キューゴーセン」は平板の句にする */
      if (strcmp(NJDNode_get_string(counter), "号線") == 0) {
         NJDNode_set_acc(counter, 0);
         NJDNode_set_chain_rule(counter, "C4");
      }
   }
}

static void set_zero_padded_reading(NJDNode *start, NJDNode *end)
{
   NJDNode *node, *counter = end->next;
   set_identifier_digit_reading(start, end);
   /* 「01号室」「001号機」の明示された0埋めは、マルにせずゼロと発音する */
   for (node = start; node != end->next; node = node->next) {
      if (strcmp(NJDNode_get_pron(node), "マル") == 0) {
         NJDNode_set_read(node, "ゼロ");
         NJDNode_set_pron(node, "ゼロ");
         /* 「03本」の前半のように最後のゼロが単独の句になるときは、NHK アクセント辞典の「ゼ＼ロ」の核を使う */
         if (node == end && NJDNode_get_chain_flag(node) == 0 && counter != NULL &&
             strcmp(NJDNode_get_pos_group1(counter), NJD_SET_DIGIT_KAZU) == 0)
            NJDNode_set_acc(node, 1);
      }
   }
   /* 「02番」は1モーラのニと前部末型の番を結合して「ゼロニ＼バン」にする */
   if (counter != NULL && strcmp(NJDNode_get_string(counter), "番") == 0)
      NJDNode_set_chain_rule(counter, "C3");
}

static NJDNode *restore_grouped_number_commas(NJDNode *start, NJDNode *end, int size)
{
   NJDNode *node, *group_end = end;
   int group_size;

   /* 「1,050円」「1,234,567」は先頭が1〜3桁、カンマの後が3桁の数として既存の位取り処理へ渡す */
   if (size > 3 || (start->prev != NULL && is_comma(NJDNode_get_string(start->prev))) ||
       has_written_zero(start, end))
      return NULL;
   while (group_end->next != NULL && is_comma(NJDNode_get_string(group_end->next))) {
      node = group_end->next->next;
      if (number_digit(node) < 0)
         return NULL;
      group_end = number_end(node, &group_size);
      if (group_size != 3)
         return NULL;
   }
   if (group_end == end)
      return NULL;
   /* 「1,050.5円」のカンマは発音設定で読点になるため、整数部の数詞へ戻して位取りし、列挙の「1,2」は休止を保つ */
   for (node = start; node != group_end; node = node->next) {
      if (is_comma(NJDNode_get_string(node))) {
         NJDNode_set_pos(node, "名詞");
         NJDNode_set_pos_group1(node, NJD_SET_DIGIT_KAZU);
      }
   }
   return group_end;
}

static int has_quantity_expression(NJDNode *start, NJDNode *node)
{
   const char *str, *orig;
   NJDNode *label;
   if (node == NULL)
      return 0;
   str = NJDNode_get_string(node);
   /* 「電話番号110から」「電話番号は110より」は番号の見出しがあるので桁読みし、「電話は100から200」は数量として位取りする */
   if (strcmp(str, "から") == 0 || strcmp(str, "まで") == 0 || strcmp(str, "より") == 0) {
      label = start->prev;
      while (label != NULL && (strcmp(NJDNode_get_pos_group3(label), "空白境界") == 0 ||
                              strcmp(NJDNode_get_string(label), "は") == 0))
         label = label->prev;
      return label == NULL || (strcmp(NJDNode_get_string(label), "電話番号") != 0 &&
                               strcmp(NJDNode_get_string(label), "番号") != 0);
   }
   /* 「電話は100以上」「100から200」「100ほど」は数量の範囲や概数を表すので、通常の位取りで「ヒャク」と読む */
   if (strcmp(str, "以上") == 0 || strcmp(str, "以下") == 0 ||
       strcmp(str, "未満") == 0 || strcmp(str, "超") == 0 ||
       strcmp(str, "近く") == 0 || strcmp(str, "ほど") == 0 ||
       strcmp(str, "くらい") == 0 || strcmp(str, "ぐらい") == 0)
      return 1;
   /* 「100に増えた」「100増えた」「100と比べた」は直後の述語の原形で数量と判定し、発信先の「119に電話する」と区別する */
   if (strcmp(NJDNode_get_pos(node), "助詞") == 0) {
      node = node->next;
      while (node != NULL && strcmp(NJDNode_get_pos_group3(node), "空白境界") == 0)
         node = node->next;
   }
   if (node == NULL)
      return 0;
   orig = NJDNode_get_orig(node);
   /* 「100に増加した」「100に減少した」「100と比較した」も直後の数量の述語として扱い、位取りで読む */
   if (strcmp(NJDNode_get_pos_group1(node), "サ変接続") == 0 &&
       (strcmp(orig, "増加") == 0 || strcmp(orig, "減少") == 0 || strcmp(orig, "比較") == 0)) {
      node = node->next;
      while (node != NULL && strcmp(NJDNode_get_pos_group3(node), "空白境界") == 0)
         node = node->next;
      return node != NULL && strcmp(NJDNode_get_pos(node), "動詞") == 0 &&
             strcmp(NJDNode_get_orig(node), "する") == 0;
   }
   if (strcmp(NJDNode_get_pos(node), "動詞") != 0)
      return 0;
   return strcmp(orig, "増える") == 0 || strcmp(orig, "増す") == 0 ||
          strcmp(orig, "増やす") == 0 ||
          strcmp(orig, "減る") == 0 || strcmp(orig, "減らす") == 0 ||
          strcmp(orig, "比べる") == 0 || strcmp(orig, "超える") == 0 ||
          strcmp(orig, "越える") == 0 || strcmp(orig, "上回る") == 0 ||
          strcmp(orig, "下回る") == 0;
}

static NJDNumberSequence *prepare_number_sequences(NJD *njd)
{
   NJDNumberSequence *sequences = NULL;
   NJDNode *node, *start[3], *end[3], *next, *separator;
   int size[3], groups, total, phone_context, postal_context, first_group, has_quantity_suffix;

   for (node = njd->head; node != NULL; node = node->next) {
      if (number_digit(node) < 0 || is_decimal_digit(node))
         continue;
      start[0] = node;
      end[0] = number_end(node, &size[0]);
      next = restore_grouped_number_commas(node, end[0], size[0]);
      /* 「12,005人」の「005」は独立したゼロ埋め番号ではなく、数全体の下3桁として読む */
      if (next != NULL) {
         node = next;
         continue;
      }
      phone_context = has_phone_context(node, end[0]);
      postal_context = has_postal_context(node);
      total = size[0];
      groups = 1;
      while (groups < 3 && (next = next_phone_group(end[groups - 1], phone_context, postal_context)) != NULL) {
         start[groups] = next;
         end[groups] = number_end(next, &size[groups]);
         total += size[groups];
         groups++;
      }
      /* 「123-4567.89円」「〒印を1234567枚印刷する」は、小数点や助数詞が続く数量として通常の数詞処理へ渡す */
      next = end[groups - 1]->next;
      /* 「☎0967(44)0336 1泊」の「1泊」は空白で区切られた別の数量として、電話番号の桁読みを保つ */
      while (next != NULL && strcmp(NJDNode_get_pos_group3(next), "空白境界") == 0 &&
             number_digit(next->next) < 0)
         next = next->next;
      /* 「電話は100以上」は数量として位取りで読み、3組に区切った「電話番号は0120-123-456から」は発信元の番号として桁読みを保つ */
      has_quantity_suffix = next != NULL &&
         (strcmp(NJDNode_get_pos_group1(next), NJD_SET_DIGIT_KAZU) == 0 ||
          strcmp(NJDNode_get_pos_group2(next), NJD_SET_DIGIT_JOSUUSHI) == 0 ||
          is_decimal_point(next) || is_digit_group_comma(next) ||
          (groups == 1 && phone_context && has_quantity_expression(node, next)));
      /* 「一〇・五」「電話料金は1.5円」「1,234円」は通常の小数・桁区切り処理に任せ、文脈と桁数で確認できる「電話番号03・1234・5678」「〒104・8011」だけを番号として読む */
      /* 後ろに数字のない「一〇一.」「四〇五,電話」の「.」「,」と、「〇」を書いた「三〇九,三〇八」の「,」は句読点なので、番号の判定を続ける */
      if (end[0]->next != NULL &&
          (is_decimal_point(end[0]->next) ||
           (is_digit_group_comma(end[0]->next) && !has_written_zero(start[0], end[0]))) &&
          !(groups == 3 && phone_context && (total == 10 || total == 11) && !has_quantity_suffix) &&
          !(groups == 2 && postal_context && size[0] == 3 && size[1] == 4 && !has_quantity_suffix)) {
         node = end[0];
         continue;
      }

      /* 「03-1234-5678」「212-836-1725」は合計10〜11桁、「市外局番213の486ー2435」は文脈で電話と判定する */
      if (groups == 3 && !has_quantity_suffix &&
          (((total == 10 || total == 11) && !has_identifier_context(node)) || phone_context)) {
         for (groups = 0; groups < 3; groups++) {
            if (protect_number_sequence(&sequences, start[groups], end[groups]))
               set_phone_digit_reading(start[groups], end[groups], 0);
            /* 「070ー3224」の長音記号を休止へ変え、「の」はそのまま発音する */
            for (separator = end[groups]->next;
                 groups < 2 && separator != start[groups + 1]; separator = separator->next) {
               if (is_number_hyphen(separator) || strcmp(NJDNode_get_string(separator), "・") == 0) {
                  NJDNode_set_pron(separator, "、");
                  NJDNode_set_mora_size(separator, 0);
                  NJDNode_set_chain_flag(separator, 0);
               }
            }
         }
         node = end[2];
         continue;
      }

      /* 「07032245679」「電話番号110」は桁読み、「1234567890」や「電話回線は3本」の数量は位取りを保つ */
      if (groups == 1 &&
          (((size[0] == 10 || size[0] == 11) && number_digit(node) == 0) || phone_context) &&
          (node->prev == NULL ||
           strcmp(NJDNode_get_pos_group1(node->prev), NJD_SET_DIGIT_KAZU) != 0) &&
          !has_quantity_suffix) {
         first_group = 0;
         if (number_digit(node) == 0 && size[0] >= 10) {
            /* 「08001234567」は0800-123-4567と区切るため、4桁の接頭辞を携帯番号より先に照合する */
            if (number_digit(node->next->next->next) == 0 &&
                     ((number_digit(node->next) == 1 && number_digit(node->next->next) == 2) ||
                      (number_digit(node->next) == 5 && number_digit(node->next->next) == 7) ||
                      (number_digit(node->next) == 8 && number_digit(node->next->next) == 0)))
               first_group = 4;
            /* 「070」「080」「090」「050」「060」は3桁、「03」「06」は2桁の接頭辞にする */
            else if (size[0] == 11 && number_digit(node->next) >= 5 &&
                     number_digit(node->next->next) == 0)
               first_group = 3;
            else if (size[0] == 10 && (number_digit(node->next) == 3 || number_digit(node->next) == 6))
               first_group = 2;
         }
         if (protect_number_sequence(&sequences, node, end[0]))
            set_phone_digit_reading(node, end[0], first_group);
      }
      /* 「〒123-4567」「123-4567」は3桁と4桁を別々に数え、ハイフンで休止するが、助数詞が続く「価格は123-4567円」は通常の数量として扱う */
      else if (groups == 2 && size[0] == 3 && size[1] == 4 &&
               !has_quantity_suffix &&
               (postal_context || !has_identifier_context(node))) {
         for (groups = 0; groups < 2; groups++) {
            if (protect_number_sequence(&sequences, start[groups], end[groups])) {
               set_phone_digit_reading(start[groups], end[groups], 0);
               if (postal_context)
                  set_postal_zero_reading(start[groups], end[groups]);
            }
         }
         for (separator = end[0]->next; separator != start[1]; separator = separator->next) {
            if (is_number_hyphen(separator) || strcmp(NJDNode_get_string(separator), "・") == 0) {
               /* 「〒102-8661」は郵便番号の読み方で、区切りを休止でなく「イチマルニーノ」の「ノ」と読む */
               if (postal_context) {
                  NJDNode_set_pos(separator, "助詞");
                  NJDNode_set_pos_group1(separator, "連体化");
                  NJDNode_set_read(separator, "ノ");
                  NJDNode_set_pron(separator, "ノ");
                  NJDNode_set_acc(separator, 1);
                  NJDNode_set_mora_size(separator, 1);
                  NJDNode_set_chain_rule(separator, "助動詞%F2@0/助詞%F2@0/動詞%F2@1/形容詞%F1");
                  NJDNode_set_chain_flag(separator, 1);
               } else {
                  NJDNode_set_pron(separator, "、");
                  NJDNode_set_mora_size(separator, 0);
                  NJDNode_set_chain_flag(separator, 0);
               }
            }
         }
         node = end[1];
         continue;
      }
      /* 「郵便番号1234567」は休止のない3-4の組として桁読みする */
      /* 0は区切りのある「〒100-0001」と同じく、組ごとに郵便番号の読み方で「マル」にする */
      else if (groups == 1 && size[0] == 7 && postal_context && !has_quantity_suffix) {
         if (protect_number_sequence(&sequences, node, end[0])) {
            set_phone_digit_reading(node, end[0], 3);
            next = node->next->next;
            set_postal_zero_reading(node, next);
            set_postal_zero_reading(next->next, end[0]);
         }
      }
      /* 「01号室」「02番」は全桁を番号として保ち、「03本」「01個」「04人」は末尾を通常の助数詞処理に渡して「サンボン」「イッコ」「ヨニン」を作る */
      else if (size[0] > 1 && number_digit(node) == 0) {
         next = end[0];
         separator = end[0]->next;
         /* 「03千円」「08百円」は末尾の数字を位の数詞と結合し、「ゼロサンゼンエン」「ゼロハッピャクエン」と読む */
         /* 「０５ 月」は暦の月として復元済みなので、助数詞の核を決めるまで全桁を保つ */
         /* 「0730時」の末尾の0は助数詞の音便で変わらないので、元の桁の組み方を保つ */
         if (separator != NULL &&
             (strcmp(NJDNode_get_pos_group2(separator), NJD_SET_DIGIT_JOSUUSHI) == 0 ||
              strcmp(NJDNode_get_pos_group1(separator), NJD_SET_DIGIT_KAZU) == 0) &&
             strcmp(NJDNode_get_pos_group3(separator), "暦月") != 0 &&
             !is_identifier_counter(separator) && number_digit(end[0]) > 0)
            next = end[0]->prev;
         if (protect_number_sequence(&sequences, node, next))
            set_zero_padded_reading(node, next);
      }
      /* 「802号室」「国道409号線」は桁読み、「12号室」と位を明示した「十二号室」は位取りに残す */
      else if (is_written_digit_sequence(node, end[0]) ||
               (size[0] >= 3 && is_room_or_road_counter(end[0]->next) && number_digit(node) != 0 &&
                (size[0] == 3 || !prefers_positional_number(node, end[0], size[0]))) ||
               /* 「型番3248の商品を9876円で購入」の価格は位取りで読み、型番の文脈で読むのは助数詞のない数か番号の助数詞に限る */
               (size[0] >= 4 && number_digit(node) != 0 &&
                (end[0]->next == NULL ||
                 strcmp(NJDNode_get_pos_group2(end[0]->next), NJD_SET_DIGIT_JOSUUSHI) != 0 ||
                 is_identifier_counter(end[0]->next)) &&
                ((end[0]->next != NULL && strcmp(NJDNode_get_string(end[0]->next), "号機") == 0) ||
                 has_identifier_context(node)) && !prefers_positional_number(node, end[0], size[0]))) {
         if (protect_number_sequence(&sequences, node, end[0]))
            set_identifier_digit_reading(node, end[0]);
      }
      node = end[0];
   }
   return sequences;
}

static void finish_number_sequences(NJDNumberSequence *sequences)
{
   NJDNumberSequence *next;
   NJDNode *node;
   while (sequences != NULL) {
      /* 「070」の表層と桁数を保ったまま、公開される品詞は元の数詞へ戻す */
      for (node = sequences->start; node != sequences->end->next; node = node->next)
         NJDNode_set_pos_group1(node, NJD_SET_DIGIT_KAZU);
      next = sequences->next;
      free(sequences);
      sequences = next;
   }
}

static void set_identifier_numerical_accents(NJD *njd)
{
   NJDNode *start, *end, *node, *counter;
   int has_thousand;
   for (start = njd->head; start != NULL; start = start->next) {
      if (strcmp(NJDNode_get_pos_group1(start), NJD_SET_DIGIT_KAZU) != 0)
         continue;
      end = start;
      has_thousand = 0;
      while (end->next != NULL && strcmp(NJDNode_get_pos_group1(end->next), NJD_SET_DIGIT_KAZU) == 0)
         end = end->next;
      for (node = start; ; node = node->next) {
         has_thousand |= strcmp(NJDNode_get_string(node), "千") == 0;
         if (node == end)
            break;
      }
      counter = end->next;
      /* 「モハ205」は「ニヒャクゴ＼」と続け、通常の数量の「205円」は既存の結合規則を使う */
      if ((!has_thousand && !has_identifier_context(start)) || (counter != NULL &&
          strcmp(NJDNode_get_pos_group2(counter), NJD_SET_DIGIT_JOSUUSHI) == 0 &&
          !is_room_or_road_counter(counter) && strcmp(NJDNode_get_string(counter), "号機") != 0)) {
         start = end;
         continue;
      }
      for (node = start; ; node = node->next) {
         /* 「2139号機」はニセンの後で区切り、「ヒャクサ＼ンジュー」を1句にする */
         if (strcmp(NJDNode_get_string(node), "百") == 0 && node->prev != NULL &&
             strcmp(NJDNode_get_string(node->prev), "千") == 0)
            NJDNode_set_chain_flag(node, 0);
         /* 「1032」「1021」は単独のセンと十の位をつなぎ、「センサ＼ンジュー」「センニ＼ジュー」と読む */
         if (node != start && node->prev != NULL &&
             (strcmp(NJDNode_get_string(node->prev), "千") == 0 ||
              strcmp(NJDNode_get_string(node->prev), "百") == 0) &&
             number_digit(node) > 0 &&
             (NJDNode_get_mora_size(node->prev) == 2) &&
             (node->prev == start || strcmp(NJDNode_get_string(node->prev), "百") == 0)) {
            NJDNode_set_chain_flag(node, 1);
            NJDNode_set_chain_rule(node, "C1");
            /* 「セン＋サンジュー」では、十の位の核を先にサンの1拍目へ置いてセンと結合する */
            if (node->next != NULL && strcmp(NJDNode_get_string(node->next), "十") == 0) {
               /* 「1052」「1062」「1082」は「センゴジ＼ュー」「センロクジ＼ュー」「センハチジ＼ュー」と読む */
               if (number_digit(node) == 5 || number_digit(node) == 6 || number_digit(node) == 8)
                  NJDNode_set_acc(node, NJDNode_get_mora_size(node) + 1);
               /* 「1072」は「センナナ＼ジュー」と読み、十の位の末尾に核を置く */
               else if (number_digit(node) == 7)
                  NJDNode_set_acc(node, NJDNode_get_mora_size(node));
               /* 「1021」「1032」は「センニ＼ジュー」「センサ＼ンジュー」と読む */
               else
                  NJDNode_set_acc(node, 1);
            }
         }
         if (node == end)
            break;
      }
      /* 「1021」の最後のイチは別句の平板にし、「1001号機」のイチは後ろの号機の核と結合する */
      if (counter == NULL || strcmp(NJDNode_get_pos_group2(counter), NJD_SET_DIGIT_JOSUUSHI) != 0) {
         if (NJDNode_get_chain_flag(end) == 0 && strcmp(NJDNode_get_string(end), "一") == 0)
            NJDNode_set_acc(end, 0);
      }
      start = end;
   }
}

static void set_railway_series_accent(NJD *njd)
{
   NJDNode *prefix, *start, *end, *node;
   int has_large_unit;
   for (prefix = njd->head; prefix != NULL; prefix = prefix->next) {
      if (strcmp(NJDNode_get_string(prefix), "ＥＦ") != 0 &&
          strcmp(NJDNode_get_string(prefix), "ＥＤ") != 0 &&
          strcmp(NJDNode_get_string(prefix), "ＥＨ") != 0 &&
          strcmp(NJDNode_get_string(prefix), "ＤＤ") != 0 &&
          strcmp(NJDNode_get_string(prefix), "ＤＥ") != 0)
         continue;
      start = prefix->next;
      if (start == NULL || strcmp(NJDNode_get_pos_group1(start), NJD_SET_DIGIT_KAZU) != 0)
         continue;
      end = start;
      has_large_unit = 0;
      while (end->next != NULL && strcmp(NJDNode_get_pos_group1(end->next), NJD_SET_DIGIT_KAZU) == 0)
         end = end->next;
      for (node = start; ; node = node->next) {
         has_large_unit |= strcmp(NJDNode_get_string(node), "千") == 0 ||
                           strcmp(NJDNode_get_string(node), "万") == 0;
         if (node == end)
            break;
      }
      /* 「EF65 1032号機」の形式名65だけを「ロクジューゴ」の1句の平板にし、個体番号1032は独立させる */
      if (!has_large_unit && (end->next == NULL ||
          strcmp(NJDNode_get_pos_group2(end->next), NJD_SET_DIGIT_JOSUUSHI) != 0)) {
         for (node = start; ; node = node->next) {
            NJDNode_set_chain_flag(node, node == start ? 0 : 1);
            NJDNode_set_chain_rule(node, "C4");
            NJDNode_set_acc(node, 0);
            if (node == end)
               break;
         }
      }
   }
}

static int is_aviation_word(NJDNode *node)
{
   static const char *words[] = {
      "航空", "航空機", "飛行機", "空港", "便名", "全日空", "日本航空",
      "搭乗", "ＪＡＬ", "ＡＮＡ", "ＳＫＹ", "ＡＤＯ", "ＳＦＪ", "ＳＮＡ",
      "ＪＴＡ", "ＪＡＣ", "ＯＲＣ", "ＩＢＸ", "ＡＫＸ", "ＲＡＣ", "ＨＡＣ", NULL
   };
   int i;
   for (i = 0; words[i] != NULL; i++) {
      if (strcmp(NJDNode_get_string(node), words[i]) == 0)
         return 1;
   }
   return 0;
}

static void set_flight_number_accent(NJD *njd)
{
   NJDNode *counter, *node, *start;
   int is_flight;
   for (counter = njd->head; counter != NULL; counter = counter->next) {
      if (strcmp(NJDNode_get_string(counter), "便") != 0 || counter->prev == NULL ||
          strcmp(NJDNode_get_pos_group1(counter->prev), NJD_SET_DIGIT_KAZU) != 0)
         continue;
      start = counter->prev;
      while (start->prev != NULL && strcmp(NJDNode_get_pos_group1(start->prev), NJD_SET_DIGIT_KAZU) == 0)
         start = start->prev;
      /* 「JAL3便」「飛行機の226便」は番号に直接付く航空会社名や「飛行機」で便名と判定し、「航空会社は3便を欠航した」は運航する便の本数として読む */
      node = start->prev;
      if (node != NULL && strcmp(NJDNode_get_string(node), "の") == 0)
         node = node->prev;
      is_flight = node != NULL && is_aviation_word(node);
      /* 「3便に搭乗する」は搭乗する便の番号とし、「3便を増便する」「3便を運航する」の本数は通常の核を保つ */
      node = counter->next;
      if (node != NULL && strcmp(NJDNode_get_string(node), "に") == 0 && node->next != NULL &&
          strcmp(NJDNode_get_string(node->next), "搭乗") == 0)
         is_flight = 1;
      /* NHK アクセント辞典の便名の推奨型に合わせ、「サンビン」は平板、複数句の「226便」は最後の「ロクビン」を平板にする */
      if (is_flight) {
         NJDNode_set_chain_flag(counter, 1);
         NJDNode_set_chain_rule(counter, "C4");
         NJDNode_set_acc(counter, 0);
      }
   }
}
#endif

#if defined(CHARSET_UTF_8) && !defined(ASCII_HEADER)
static void set_written_place_chain_rule(NJDNode *node, const char *list[])
{
   NJDNode rule_node;
   const char *str = NJDNode_get_string(node);
   size_t length = strlen(str);
   int i;
   for (i = 1; list[i] != NULL; i++) {
      if (strncmp(list[i], str, length) != 0 || list[i][length] != ',')
         continue;
      NJDNode_initialize(&rule_node);
      NJDNode_load(&rule_node, (char *) list[i]);
      NJDNode_set_chain_rule(node, NJDNode_get_chain_rule(&rule_node));
      NJDNode_clear(&rule_node);
      return;
   }
}

static void set_written_place_chain_rules(NJD *njd)
{
   NJDNode *node;
   /* 漢字で書いた「七百十三」の「百」「十」は辞書の行の結合規則 (C3) を持ち、算用数字の「713」から作る位の字は規則の表の値を持つ */
   /* 同じ数を同じアクセントで読むため、数詞に続く位の字には、算用数字から作るときと同じ結合規則を使う */
   for (node = njd->head; node != NULL; node = node->next) {
      if (node->prev == NULL ||
          strcmp(NJDNode_get_pos_group1(node), NJD_SET_DIGIT_KAZU) != 0 ||
          strcmp(NJDNode_get_pos_group1(node->prev), NJD_SET_DIGIT_KAZU) != 0)
         continue;
      /* 「万」「億」は算用数字の「13億」でも辞書の行のまま読むので、表の値にそろえるのは「十」「百」「千」だけにする */
      set_written_place_chain_rule(node, njd_set_digit_rule_numeral_list2);
   }
}

/* 縦書きで「三・五％」「一・五倍」のように中黒を小数点に使うとき、後ろに続く量の単位 */
static const char *njd_set_digit_rule_vertical_decimal_units[] = {
   "％", "パーセント", "割", "倍", "度", "円", "ドル", "キロ", "キロメートル", "キログラム",
   "メートル", "センチ", "センチメートル", "ミリ", "ミリメートル", "グラム", "ミリグラム",
   "リットル", "ミリリットル", "トン", "ヘクタール",
   NULL
};

static void normalize_old_place_characters(NJD *njd)
{
   NJDNode *node;
   /* 大字の「拾」と旧字体の「萬」は位の表にないので、「十」「万」に置き換えて「拾万円」「一萬円」を位取りの1句で読む */
   for (node = njd->head; node != NULL; node = node->next) {
      /* 「十数」の「数」は助数詞の行で解析されて「十本」と同じく「ジュッスー」と促音化するので、位に続く数詞の「数」として扱う */
      if (strcmp(NJDNode_get_string(node), "数") == 0 &&
          strcmp(NJDNode_get_pos_group2(node), NJD_SET_DIGIT_JOSUUSHI) == 0 && node->prev != NULL &&
          (strcmp(NJDNode_get_string(node->prev), "十") == 0 ||
           strcmp(NJDNode_get_string(node->prev), "百") == 0 ||
           strcmp(NJDNode_get_string(node->prev), "千") == 0)) {
         NJDNode_set_pos_group1(node, NJD_SET_DIGIT_KAZU);
         NJDNode_set_pos_group2(node, "*");
      }
      if (strcmp(NJDNode_get_pos_group1(node), NJD_SET_DIGIT_KAZU) != 0)
         continue;
      if (strcmp(NJDNode_get_string(node), "拾") == 0) {
         NJDNode_set_string(node, "十");
         NJDNode_set_orig(node, "十");
      } else if (strcmp(NJDNode_get_string(node), "萬") == 0) {
         NJDNode_set_string(node, "万");
         NJDNode_set_orig(node, "万");
      }
   }
}

static int is_kanji_digit_or_small_place_string(NJDNode *node)
{
   const char *str;
   if (node == NULL)
      return 0;
   if (is_kanji_digit_string(node))
      return 1;
   str = NJDNode_get_string(node);
   return strcmp(str, "十") == 0 || strcmp(str, "百") == 0 || strcmp(str, "千") == 0;
}

static int is_vertical_decimal_unit(NJDNode *node)
{
   const char *str = NJDNode_get_string(node);
   int i;
   /* 「平方メートル」「立方メートル」のように「平方」「立方」を前に付けた単位も、後ろの単位で判定する */
   if (strncmp(str, "平方", strlen("平方")) == 0 || strncmp(str, "立方", strlen("立方")) == 0)
      str += strlen("平方");
   for (i = 0; njd_set_digit_rule_vertical_decimal_units[i] != NULL; i++)
      if (strcmp(str, njd_set_digit_rule_vertical_decimal_units[i]) == 0)
         return 1;
   return 0;
}

static void set_vertical_decimal_points(NJD *njd)
{
   NJDNode *node, *digit, *first, *last;
   int has_zero, has_unit, has_counter;
   /* 漢数字の間の中黒は、ふつうは「一・二年生」「三・四月」「三〇・四〇代」のように数を並べる区切りだが、縦書きでは小数点にも使う */
   /* 小数点として読むのは、次のどれかで小数と分かるときだけにする */
   /* (a) 前の数が「〇・五」「〇・〇三」のように「〇」で始まる */
   /* (b) 後ろの数の直後に「三・五％」「一・五倍」「百五十二・二平方メートル」のように量の単位が続くか、「三・五万トン」のように「万」「億」「兆」が続く */
   /*     前の数は「二十・五％」のように「十」「百」「千」で終わってもよい */
   /* (c) 前の数が「一〇・五」のように「〇」を書く桁読みの表記で、後ろの数が「〇」で終わらず、後ろに助数詞も続かない */
   /* 「二〇・三〇年代」「一〇・二〇人」は後ろの数が「〇」で終わるので、並びのまま読む */
   for (node = njd->head; node != NULL; node = node->next) {
      if (strcmp(NJDNode_get_string(node), "・") != 0 || !is_kanji_digit_or_small_place_string(node->prev) ||
          !is_kanji_digit_string(node->next))
         continue;
      has_zero = 0;
      for (first = node->prev; ; first = first->prev) {
         if (strcmp(NJDNode_get_string(first), "〇") == 0)
            has_zero = 1;
         if (!is_kanji_digit_or_small_place_string(first->prev))
            break;
      }
      for (last = node->next; is_kanji_digit_string(last->next); last = last->next);
      has_unit = 0;
      has_counter = 0;
      digit = last->next;
      if (digit != NULL) {
         has_unit = is_vertical_decimal_unit(digit) || strcmp(NJDNode_get_string(digit), "万") == 0 ||
                    strcmp(NJDNode_get_string(digit), "億") == 0 || strcmp(NJDNode_get_string(digit), "兆") == 0;
         has_counter = strcmp(NJDNode_get_pos_group2(digit), NJD_SET_DIGIT_JOSUUSHI) == 0;
      }
      if (strcmp(NJDNode_get_string(first), "〇") == 0 || has_unit ||
          (has_zero && strcmp(NJDNode_get_string(last), "〇") != 0 && !has_counter))
         NJDNode_set_string(node, NJD_SET_DIGIT_TEN1);
   }
}

static int starts_with_digit_character(NJDNode *node)
{
   static const char *digits[] = {"０", "１", "２", "３", "４", "５", "６", "７", "８", "９",
                                  "〇", "一", "二", "三", "四", "五", "六", "七", "八", "九", NULL};
   const char *str = NJDNode_get_string(node);
   int i;
   for (i = 0; digits[i] != NULL; i++)
      if (strncmp(str, digits[i], strlen(digits[i])) == 0)
         return 1;
   return 0;
}

static void mark_number_list_separators(NJD *njd)
{
   NJDNode *node;
   /* 「1・2年生」「3・4月」「1・2・3」の数の間の中黒は数を並べる区切りなので、前後を別々の数として読む */
   /* 「3・4月」の後ろは辞書の1語「４月」になるので、後ろは数字で始まる語まで含める */
   /* 電話番号・郵便番号として保護した数は品詞が「数」でなくなるので、その区切りの休止や「ノ」は変えない */
   for (node = njd->head; node != NULL; node = node->next) {
      if (strcmp(NJDNode_get_string(node), "・") == 0 && node->prev != NULL && node->next != NULL &&
          strcmp(NJDNode_get_pos_group1(node->prev), NJD_SET_DIGIT_KAZU) == 0 &&
          (strcmp(NJDNode_get_pos_group1(node->next), NJD_SET_DIGIT_KAZU) == 0 ||
           starts_with_digit_character(node->next)))
         NJDNode_set_pos_group3(node, "数の区切り");
   }
}

static int is_touten(NJDNode *node)
{
   return node != NULL &&
          (strcmp(NJDNode_get_string(node), "、") == 0 || strcmp(NJDNode_get_string(node), "，") == 0);
}

static int is_approximate_number_comma(NJDNode *node)
{
   return is_touten(node) && strcmp(NJDNode_get_pos_group3(node), "数の区切り") == 0;
}

static int day_word_digit(NJDNode *node)
{
   static const char *days[] = {"二日", "三日", "四日", "五日", "六日", "七日", "八日", "九日",
                                "２日", "３日", "４日", "５日", "６日", "７日", "８日", "９日", NULL};
   int i;
   /* MeCab が「三日」「５日」を1語にした語の数を返す */
   for (i = 0; days[i] != NULL; i++)
      if (strcmp(NJDNode_get_string(node), days[i]) == 0)
         return i % 8 + 2;
   return -1;
}

static int counter_word_digit(NJDNode *node, const char **counter)
{
   static const char *digits[] = {"二", "三", "四", "五", "六", "七", "八", "九",
                                  "２", "３", "４", "５", "６", "７", "８", "９", NULL};
   static const char *counters[] = {"人", "日間", NULL};
   const char *str = NJDNode_get_string(node);
   int i, j;
   /* MeCab が「二人」「八人」「三日間」のように、1桁の数と助数詞を1語にした語の数と助数詞を返す */
   for (i = 0; digits[i] != NULL; i++) {
      if (strncmp(str, digits[i], strlen(digits[i])) != 0)
         continue;
      for (j = 0; counters[j] != NULL; j++) {
         if (strcmp(str + strlen(digits[i]), counters[j]) == 0) {
            *counter = counters[j];
            return i % 8 + 2;
         }
      }
   }
   return -1;
}

static int approximate_tail_digit(NJDNode *node)
{
   const char *counter;
   int digit;
   /* 後ろの数は、1桁の数に助数詞が続く形 (「5人」) か、辞書の1語の「三日」「５日」「二人」「三日間」に限る */
   if (strcmp(NJDNode_get_pos_group1(node), NJD_SET_DIGIT_KAZU) == 0)
      return node->next != NULL && strcmp(NJDNode_get_pos_group1(node->next), NJD_SET_DIGIT_KAZU) != 0 &&
             is_counter_after_digits(node->next) ? get_digit(node, 0) : -1;
   digit = counter_word_digit(node, &counter);
   return digit > 0 ? digit : day_word_digit(node);
}

static void mark_approximate_number_commas(NJD *njd)
{
   NJDNode *node, *first;
   int x;
   /* 「4、5日」「二、三人」「15、6年前」のように、読点で隣り合う1桁の数を並べて助数詞を続けた概数は、休止を置かずに読む */
   /* 印は中黒の区切りと同じ「数の区切り」を使い、njd2jpcommon が読点の発音を空にする */
   for (node = njd->head; node != NULL; node = node->next) {
      if (!is_touten(node) || node->prev == NULL || node->next == NULL ||
          strcmp(NJDNode_get_pos_group1(node->prev), NJD_SET_DIGIT_KAZU) != 0)
         continue;
      x = get_digit(node->prev, 0);
      if (x < 1 || x > 8 || approximate_tail_digit(node->next) != x + 1)
         continue;
      for (first = node->prev; first->prev != NULL &&
           strcmp(NJDNode_get_pos_group1(first->prev), NJD_SET_DIGIT_KAZU) == 0; first = first->prev);
      /* 「5月4、5日」は日付を並べたもの、「1、2、3日」は3つ以上の数の並びなので、概数にしない */
      if (first->prev != NULL &&
          (strstr(NJDNode_get_string(first->prev), NJD_SET_DIGIT_GATSU) != NULL ||
           (is_touten(first->prev) && first->prev->prev != NULL &&
            strcmp(NJDNode_get_pos_group1(first->prev->prev), NJD_SET_DIGIT_KAZU) == 0)))
         continue;
      NJDNode_set_pos_group3(node, "数の区切り");
   }
}

static void set_approximate_number_readings(NJD *njd)
{
   static const char *digit_prons[] = {"ニ", "サン", "ヨン", "ゴ", "ロク", "ナナ", "ハチ", "キュウ"};
   static const char *digit_mora_prons[] = {"ニ", "サン", "ヨン", "ゴ", "ロク", "ナナ", "ハチ", "キュー"};
   /* 「人」の前の数の読み (「ヨニン」の「ヨ」) */
   static const char *person_prons[] = {"ニ", "サン", "ヨ", "ゴ", "ロク", "ナナ", "ハチ", "キュウ"};
   static const char *person_mora_prons[] = {"ニ", "サン", "ヨ", "ゴ", "ロク", "ナナ", "ハチ", "キュー"};
   NJDNode *node, *first;
   NJDNode day;
   const char *counter;
   char buff[MAXBUFLEN];
   int i, y;
   for (node = njd->head; node != NULL; node = node->next) {
      if (is_approximate_number_comma(node)) {
         /* 概数の前の「4」は「シ」と読み (「シゴニチ」「シゴニン」)、後ろの数と1つのアクセント句にする */
         if (get_digit(node->prev, 0) == 4) {
            NJDNode_set_pron(node->prev, "シ");
            NJDNode_set_mora_size(node->prev, 1);
         }
         /* 1語の「三日」(「ミッカ」) は日付の読みなので、概数の日数の「サンニチ」に直す */
         y = day_word_digit(node->next);
         if (y > 0) {
            snprintf(buff, sizeof(buff), "%sニチ", digit_prons[y - 2]);
            NJDNode_set_read(node->next, buff);
            snprintf(buff, sizeof(buff), "%sニチ", digit_mora_prons[y - 2]);
            NJDNode_set_pron(node->next, buff);
            NJDNode_set_mora_size(node->next, (int) strlen(buff) / 3);
            NJDNode_set_acc(node->next, 0);
         }
         /* 1語の「二人」(「フタリ」)、「八人」、「三日間」(「ミッカカン」) も、概数では助数詞の漢語の読みの「ニニン」「ハチニン」「サンニチカン」に直す */
         y = counter_word_digit(node->next, &counter);
         if (y > 0) {
            if (strcmp(counter, "人") == 0) {
               snprintf(buff, sizeof(buff), "%sニン", person_prons[y - 2]);
               NJDNode_set_read(node->next, buff);
               snprintf(buff, sizeof(buff), "%sニン", person_mora_prons[y - 2]);
            } else {
               snprintf(buff, sizeof(buff), "%sニチカン", digit_prons[y - 2]);
               NJDNode_set_read(node->next, buff);
               snprintf(buff, sizeof(buff), "%sニチカン", digit_mora_prons[y - 2]);
            }
            NJDNode_set_pron(node->next, buff);
            NJDNode_set_mora_size(node->next, (int) strlen(buff) / 3);
            NJDNode_set_acc(node->next, 0);
         }
         NJDNode_set_chain_flag(node->next, 1);
         continue;
      }
      /* 「5月4、5日」の前の日も、後ろの「5日」(「イツカ」) と同じく日付の読み (「ヨッカ」) にする */
      if (!is_touten(node) || node->prev == NULL || node->next == NULL ||
          strcmp(NJDNode_get_pos_group1(node->prev), NJD_SET_DIGIT_KAZU) != 0)
         continue;
      if (day_word_digit(node->next) < 0 &&
          (node->next->next == NULL || strcmp(NJDNode_get_string(node->next->next), NJD_SET_DIGIT_NICHI) != 0))
         continue;
      first = node->prev;
      if (first->prev == NULL || strstr(NJDNode_get_string(first->prev), NJD_SET_DIGIT_GATSU) == NULL)
         continue;
      for (i = 0; njd_set_digit_rule_conv_table5[i] != NULL; i += 2) {
         if (strcmp(NJDNode_get_string(first), njd_set_digit_rule_conv_table5[i]) == 0) {
            NJDNode_initialize(&day);
            NJDNode_load(&day, njd_set_digit_rule_conv_table5[i + 1]);
            NJDNode_set_read(first, NJDNode_get_read(&day));
            NJDNode_set_pron(first, NJDNode_get_pron(&day));
            NJDNode_set_acc(first, NJDNode_get_acc(&day));
            NJDNode_set_mora_size(first, NJDNode_get_mora_size(&day));
            NJDNode_clear(&day);
            break;
         }
      }
   }
}

static void set_approximate_counter_accents(NJD *njd)
{
   /* 「1、2回」「2、3本」のように平板型で読む助数詞 (「2、3人」は「2、3」の後だけ) */
   static const char *flat_counters[] = {"回", "冊", "度", "年", "遍", "本", NULL};
   /* 「2、3日」「1、2時間」「1、2か月」は、助数詞の結合規則のまま「ニサ＼ンニチ」「イチニジ＼カン」「イチニカ＼ゲツ」と読む */
   static const char *accented_counters[] = {"日", "時", "か月", NULL};
   NJDNode *node, *counter;
   int x, i, is_flat, is_accented;

   /* 概数の読点は句の頭にされていて、句の核は前の数 (「ニ＼」) のものが使われるので、本編に載る組み合わせだけ前の数の句につなぐ */
   /* 「1、2」「2、3」の後の助数詞と「3、4日」だけを扱い、ほかの数の並びと「1、2人」(「イチ＼ニニン」) は今までどおりに読む */
   for (node = njd->head; node != NULL; node = node->next) {
      /* 「3、4日」は、書き言葉の「三四日」と同じく和語の「ヨッカ」で「サ＼ンヨッカ」と読む */
      if (is_approximate_number_comma(node) && get_digit(node->prev, 0) == 3 && node->next != NULL &&
          NJDNode_get_chain_flag(node->next) == 1 &&
          (day_word_digit(node->next) == 4 ||
           (get_digit(node->next, 0) == 4 && node->next->next != NULL &&
            NJDNode_get_chain_flag(node->next->next) == 1 &&
            strcmp(NJDNode_get_string(node->next->next), NJD_SET_DIGIT_NICHI) == 0))) {
         if (day_word_digit(node->next) == 4) {
            NJDNode_set_read(node->next, "ヨッカ");
            NJDNode_set_pron(node->next, "ヨッカ");
            NJDNode_set_mora_size(node->next, 3);
         } else {
            NJDNode_set_read(node->next, "ヨッ");
            NJDNode_set_pron(node->next, "ヨッ");
            NJDNode_set_mora_size(node->next, 2);
            NJDNode_set_read(node->next->next, "カ");
            NJDNode_set_pron(node->next->next, "カ");
            NJDNode_set_mora_size(node->next->next, 1);
            NJDNode_set_chain_rule(node->next->next, "F1");
         }
         NJDNode_set_chain_flag(node, 1);
         NJDNode_set_chain_rule(node->next, "F1");
         NJDNode_set_acc(node->prev, 1);
         continue;
      }
      /* 漢数字の「二、三日」は「三日」が1語になるので、算用数字の「2、3日」と同じく「三日」の「サ」の後で下げて「ニサ＼ンニチ」と読む */
      if (is_approximate_number_comma(node) && get_digit(node->prev, 0) == 2 && node->next != NULL &&
          NJDNode_get_chain_flag(node->next) == 1 && day_word_digit(node->next) == 3) {
         NJDNode_set_chain_flag(node, 1);
         NJDNode_set_acc(node->next, 1);
         NJDNode_set_chain_rule(node->next, "C1");
         continue;
      }
      if (!is_approximate_number_comma(node) || node->next == NULL || node->next->next == NULL ||
          NJDNode_get_chain_flag(node->next) != 1 || NJDNode_get_chain_flag(node->next->next) != 1)
         continue;
      x = get_digit(node->prev, 0);
      if (x != 1 && x != 2)
         continue;
      counter = node->next->next;
      is_flat = strcmp(NJDNode_get_string(counter), NJD_SET_DIGIT_NIN) == 0 && x == 2;
      for (i = 0; flat_counters[i] != NULL; i++)
         if (strcmp(NJDNode_get_string(counter), flat_counters[i]) == 0)
            is_flat = 1;
      is_accented = 0;
      for (i = 0; accented_counters[i] != NULL; i++)
         if (strcmp(NJDNode_get_string(counter), accented_counters[i]) == 0)
            is_accented = 1;
      /* 「2、3日」は本編に載るが「1、2日」は載らず、「1、2か月」は載るが「2、3か月」は載らない */
      if ((strcmp(NJDNode_get_string(counter), NJD_SET_DIGIT_NICHI) == 0 && x != 2) ||
          (strcmp(NJDNode_get_string(counter), "か月") == 0 && x != 1) ||
          (strcmp(NJDNode_get_string(counter), "時") == 0 &&
           (counter->next == NULL || strcmp(NJDNode_get_string(counter->next), "間") != 0)))
         is_accented = 0;
      if (!is_flat && !is_accented)
         continue;
      NJDNode_set_chain_flag(node, 1);
      if (is_flat)
         NJDNode_set_chain_rule(counter, "F5");
   }
}

static void set_number_list_separator_phrases(NJD *njd)
{
   NJDNode *node;
   /* 数を並べる中黒の後ろの数から別のアクセント句にする (「イチ」「ニネンセー」) */
   /* 中黒の語は残し、休止を置かないよう JPCommon へ渡すときに発音を空にする (njd2jpcommon) */
   for (node = njd->head; node != NULL; node = node->next) {
      if (strcmp(NJDNode_get_string(node), "・") == 0 &&
          strcmp(NJDNode_get_pos_group3(node), "数の区切り") == 0 && node->next != NULL)
         NJDNode_set_chain_flag(node->next, 0);
   }
}
#endif

#if defined(CHARSET_UTF_8) && !defined(ASCII_HEADER)
static void set_fraction_readings_of_fun_words(NJD *njd)
{
   static const char *words[][2] = {{"三分", "サンブン"}, {"四分", "ヨンブン"}, {NULL, NULL}};
   NJDNode *node, *next;
   int i;
   /* 時間量の1語の「三分」「四分」(「サンプン」「ヨンプン」) に「の」を挟まずに数が続く「四分三」は、分数として「ヨンブンサン」と読む */
   for (node = njd->head; node != NULL && node->next != NULL; node = node->next) {
      if (strcmp(NJDNode_get_pos_group1(node->next), NJD_SET_DIGIT_KAZU) != 0 ||
          (node->prev != NULL && strcmp(NJDNode_get_pos_group1(node->prev), NJD_SET_DIGIT_KAZU) == 0))
         continue;
      for (next = node->next; next != NULL && strcmp(NJDNode_get_pos_group1(next), NJD_SET_DIGIT_KAZU) == 0;
           next = next->next);
      /* 「三分5秒」のように後ろの数に助数詞が続くときは時間量のまま読む */
      if (is_counter_after_digits(next))
         continue;
      for (i = 0; words[i][0] != NULL; i++) {
         if (strcmp(NJDNode_get_string(node), words[i][0]) == 0) {
            NJDNode_set_read(node, words[i][1]);
            NJDNode_set_pron(node, words[i][1]);
            break;
         }
      }
   }
}

static void set_news_date_first_day(NJD *njd)
{
   NJDNode *node;
   NJDNode *following;
   /* 報道の「1日、」「1日の夜」「一日午後」「1日付」の「1日」は、月がなくても日付の「ツイタチ」と読む */
   /* 漢数字の「一日、」は「今日も一日、」「一日、32ドル」のように日数を表すことが多いので、漢数字は時間帯の語が続くときだけにする */
   for (node = njd->head; node != NULL && node->next != NULL; node = node->next) {
      if ((strcmp(NJDNode_get_string(node), "１") != 0 && strcmp(NJDNode_get_string(node), "一") != 0) ||
          strcmp(NJDNode_get_pos_group1(node), NJD_SET_DIGIT_KAZU) != 0 ||
          (node->prev != NULL && strcmp(NJDNode_get_pos_group1(node->prev), NJD_SET_DIGIT_KAZU) == 0) ||
          strcmp(NJDNode_get_string(node->next), NJD_SET_DIGIT_NICHI) != 0 ||
          !is_news_date_context(node->next->next) || is_day_count_context(node, node->next->next))
         continue;
      following = node->next->next;
      /* 「一日付で」は「1日付で」と違って日数の「イチニチ」とも読めるので、漢数字は午前・午後のような時間帯の語だけを見る */
      if (strcmp(NJDNode_get_string(node), "一") == 0 &&
          (strncmp(NJDNode_get_string(following), "付", strlen("付")) == 0 ||
           (!is_time_of_day_word(following) &&
            !(strcmp(NJDNode_get_string(following), "の") == 0 && is_time_of_day_word(following->next)))))
         continue;
      NJDNode_load(node, NJD_SET_DIGIT_TSUITACHI);
      /* 数詞の語がなくなると後の数詞の処理まで進まないので、読みを1語に移した「日」はここで外す */
      NJD_remove_node(njd, node->next);
   }
}

static void set_kurai_after_day_words(NJD *njd)
{
   NJDNode *node;
   const char *str;
   size_t length;
   /* 「十日位」の「十日」は地名の行に取られて接尾辞の「イ」が続くので、日数の語の後の「位」を程度の「クライ」に直す */
   for (node = njd->head; node != NULL && node->next != NULL; node = node->next) {
      str = NJDNode_get_string(node);
      length = strlen(str);
      if ((!starts_with_digit_character(node) && strncmp(str, "十", strlen("十")) != 0) ||
          length < strlen(NJD_SET_DIGIT_NICHI) ||
          strcmp(str + length - strlen(NJD_SET_DIGIT_NICHI), NJD_SET_DIGIT_NICHI) != 0 ||
          strcmp(NJDNode_get_string(node->next), "位") != 0 || strcmp(NJDNode_get_read(node->next), "イ") != 0)
         continue;
      NJDNode_set_read(node->next, "クライ");
      NJDNode_set_pron(node->next, "クライ");
      NJDNode_set_mora_size(node->next, 3);
      NJDNode_set_acc(node->next, 0);
   }
}

static void set_day_word_suffix_accents(NJD *njd)
{
   NJDNode *node;
   const char *str;
   size_t length;
   /* 辞書や数詞の処理が1語にした日付の語 (「一日」「二日」「四日」「千日」) に続く「目」は、数詞の後の「日」に続く「目」と同じく尾高型で結合する (「イチニチメ＼」「フツカメ＼」) */
   for (node = njd->head; node != NULL && node->next != NULL; node = node->next) {
      str = NJDNode_get_string(node);
      length = strlen(str);
      if ((!starts_with_digit_character(node) && strncmp(str, "十", strlen("十")) != 0 &&
           strncmp(str, "千", strlen("千")) != 0) ||
          length <= strlen(NJD_SET_DIGIT_NICHI) ||
          strcmp(str + length - strlen(NJD_SET_DIGIT_NICHI), NJD_SET_DIGIT_NICHI) != 0 ||
          strcmp(NJDNode_get_string(node->next), "目") != 0 ||
          strcmp(NJDNode_get_pos_group1(node->next), "接尾") != 0)
         continue;
      NJDNode_set_chain_flag(node->next, 1);
      NJDNode_set_chain_rule(node->next, "F4@1");
   }
}

static int is_kango_after_one(NJDNode *node)
{
   /* 「一」と分けて解析されても「一」を促音にする漢語は、1字の接尾辞 (「一審」「一国」「一書」「1死」「1速」「1庁」) と次の語に限る */
   /* 「一単位」「一企業」「第一世代」のような2字以上の語や、「一皮」「一冬」のような和語の1字の名詞は、「イチ」と読むことが多い */
   /* 「1中のグラフ」「一小」のように、学校名と図表の番号のどちらにもなる「中」「小」も「イチ」のまま読む */
   static const char *words[] = {"譜", "党", "佐", "体性", "生涯", "戸建", "回転", "小節", "食分", "車線", "科目",
                                 "課長", NULL};
   const char *str = NJDNode_get_string(node);
   int i;
   for (i = 0; words[i] != NULL; i++)
      if (strcmp(str, words[i]) == 0)
         return 1;
   return strlen(str) == 3 && strcmp(NJDNode_get_pos_group1(node), "接尾") == 0 &&
          strcmp(NJDNode_get_pos_group2(node), NJD_SET_DIGIT_JOSUUSHI) != 0 &&
          strcmp(str, "中") != 0 && strcmp(str, "小") != 0;
}

static void geminate_one_before_kango(NJD *njd)
{
   static const char *voiceless_heads[] = {"カ", "キ", "ク", "ケ", "コ", "サ", "シ", "ス", "セ", "ソ", "タ", "チ",
                                           "ツ", "テ", "ト", NULL};
   static const char *h_heads[][2] = {
      {"ハ", "パ"}, {"ヒ", "ピ"}, {"フ", "プ"}, {"ヘ", "ペ"}, {"ホ", "ポ"}, {NULL, NULL}
   };
   NJDNode *node, *next;
   const char *pron;
   char buff[MAXBUFLEN];
   int i;
   /* MeCab が「一」と後ろの漢語を分けた「一審」「一国」「1死」「第1譜」では、「イッシン」「イッコク」「ダイイップ」と促音にする */
   for (node = njd->head; node != NULL && node->next != NULL; node = node->next) {
      next = node->next;
      if (strcmp(NJDNode_get_string(node), "一") != 0 ||
          strcmp(NJDNode_get_pos_group1(node), NJD_SET_DIGIT_KAZU) != 0 ||
          strcmp(NJDNode_get_pron(node), "イチ") != 0 || !is_kango_after_one(next))
         continue;
      /* 「二十一」のような数の末尾や「0.1」のような小数の桁、「2‐1‐1」のような図表の番号の「一」は対象にしない */
      if (node->prev != NULL &&
          (strcmp(NJDNode_get_pos_group1(node->prev), NJD_SET_DIGIT_KAZU) == 0 || is_number_hyphen(node->prev)))
         continue;
      if (is_decimal_digit(node))
         continue;
      pron = NJDNode_get_pron(next);
      for (i = 0; voiceless_heads[i] != NULL; i++)
         if (strncmp(pron, voiceless_heads[i], strlen(voiceless_heads[i])) == 0)
            break;
      if (voiceless_heads[i] == NULL) {
         for (i = 0; h_heads[i][0] != NULL; i++)
            if (strncmp(pron, h_heads[i][0], strlen(h_heads[i][0])) == 0)
               break;
         if (h_heads[i][0] == NULL)
            continue;
         /* 「第1譜」の「フ」のように、ハ行は促音の後で半濁音にする */
         snprintf(buff, sizeof(buff), "%s%s", h_heads[i][1], pron + strlen(h_heads[i][0]));
         NJDNode_set_pron(next, buff);
      }
      NJDNode_set_pron(node, "イッ");
   }
}
#endif

void njd_set_digit(NJD * njd)
{
   int i, j;
   NJDNode *s = NULL;
   NJDNode *e = NULL;
   NJDNode *node;
   int find = 0;
#if defined(CHARSET_UTF_8) && !defined(ASCII_HEADER)
   NJDNumberSequence *number_sequences;
   normalize_old_place_characters(njd);
   set_vertical_decimal_points(njd);
   set_written_place_chain_rules(njd);
   mark_written_group_quantities(njd);
   /* 「3 本」の「ホン」を助数詞へ戻してから、「サンボン」の濁音化とアクセント結合を適用する */
   restore_counter_features(njd);
   number_sequences = prepare_number_sequences(njd);
   mark_number_list_separators(njd);
   mark_approximate_number_commas(njd);
   /* 「十日位」は数詞の語を含まず、後の数詞の処理まで進まないので、ここで「位」を直す */
   set_kurai_after_day_words(njd);
   /* 辞書の1語の「一日目」「千日目」は数詞の語を含まず、後の数詞の処理まで進まないので、ここでも「目」を結合する */
   set_day_word_suffix_accents(njd);
   set_news_date_first_day(njd);
   /* 「070」のように全桁を番号として保護した文でも、通常の数詞処理の後で品詞を戻す */
   if (number_sequences != NULL)
      find = 1;
#endif

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
         if (get_digit(node->prev, 0) == 0) {
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
         /* 小数部の数詞だけをスキップし、単位や一般名詞、空白を表す境界の後にある小数点も処理する */
         while (node && strcmp(NJDNode_get_pos_group1(node), NJD_SET_DIGIT_KAZU) == 0)
            node = node->next;
         if (node)
            node = node->next;
      } else {
         node = node->next;
      }
   }

   for (node = njd->head->next; node != NULL; node = node->next) {
#if defined(CHARSET_UTF_8) && !defined(ASCII_HEADER)
      /* 「1.1本」の小数の桁は整数の「1本」と違って促音化・濁音化せず、「イッテンイチホン」と読む */
      if (strcmp(NJDNode_get_pos_group1(node->prev), NJD_SET_DIGIT_KAZU) == 0 &&
          is_decimal_digit(node->prev) == 1)
         continue;
#endif
      if (strcmp(NJDNode_get_pos_group1(node->prev), NJD_SET_DIGIT_KAZU) == 0) {
         if (strcmp(NJDNode_get_pos_group2(node), NJD_SET_DIGIT_JOSUUSHI) == 0
             || strcmp(NJDNode_get_pos_group1(node), NJD_SET_DIGIT_FUKUSHIKANOU) == 0
#if defined(CHARSET_UTF_8) && !defined(ASCII_HEADER)
             /* 一般名詞や接尾辞として解析された助数詞も、小数の桁の後でなければ助数詞として扱う */
             || (is_decimal_digit(node->prev) == 0 &&
                 search_numerative_class(njd_set_digit_rule_counter_words, node))) {
            /* 「8個」「8回」の「ハッ」と、それに伴う助数詞の濁音・半濁音は後で戻すことがあるので、変える前の発音を控える */
            char counter_pron_before[64];
            strncpy(counter_pron_before, NJDNode_get_pron(node), sizeof(counter_pron_before) - 1);
            counter_pron_before[sizeof(counter_pron_before) - 1] = '\0';
            /* 数詞に続く「部屋」は「ヘヤ」、相撲部屋などの複合語は辞書の「ベヤ」を使う */
            if (strcmp(NJDNode_get_string(node), "部屋") == 0 &&
                strcmp(NJDNode_get_read(node), "ベヤ") == 0) {
               NJDNode_set_read(node, "ヘヤ");
               NJDNode_set_pron(node, "ヘヤ");
            }
            /* 数詞に続く「石」は石高の単位として「コク」を使う */
            if (strcmp(NJDNode_get_string(node), "石") == 0) {
               NJDNode_set_read(node, "コク");
               NJDNode_set_pron(node, "コク");
            }
            /* tsqyomi が数詞の後の「玉」「元」に「ギョク」「モト」の行を選んでも、数える単位の「タマ」「ゲン」と読む (「フタタマ」「ジューハチゲン」) */
            if (strcmp(NJDNode_get_string(node), "玉") == 0 && strcmp(NJDNode_get_read(node), "ギョク") == 0) {
               NJDNode_set_read(node, "タマ");
               NJDNode_set_pron(node, "タマ");
               NJDNode_set_mora_size(node, 2);
            }
            if (strcmp(NJDNode_get_string(node), "元") == 0 && strcmp(NJDNode_get_read(node), "モト") == 0) {
               NJDNode_set_read(node, "ゲン");
               NJDNode_set_pron(node, "ゲン");
               NJDNode_set_mora_size(node, 2);
            }
            /* 数詞に続く「里」は距離の単位として「リ」を使い、村里の「サト」とは読まない */
            if (strcmp(NJDNode_get_string(node), "里") == 0) {
               NJDNode_set_read(node, "リ");
               NJDNode_set_pron(node, "リ");
               NJDNode_set_mora_size(node, 1);
            }
            /* convert digit pron */
#if defined(CHARSET_UTF_8) && !defined(ASCII_HEADER)
            if (strcmp(NJDNode_get_string(node), "分") == 0 && ratio_or_fraction_reading_of_fun(node) != NULL) {
               /* 割合の「ブ」と、「の」を省いた分数の「ブン」は、数詞を促音化しない (「サンワリニブ」「センブンイチ」) */
               const char *fun_reading = ratio_or_fraction_reading_of_fun(node);
               NJDNode_set_read(node, fun_reading);
               NJDNode_set_pron(node, fun_reading);
               NJDNode_set_mora_size(node, strcmp(fun_reading, "ブ") == 0 ? 1 : 2);
            }
            else
#endif
            if (strcmp(NJDNode_get_string(node), "分") == 0 &&
                strcmp(NJDNode_get_read(node), "ブン") == 0) {
               /* 分数の分母の「ブン」の前では、時間量の「ヒャップン」「ロップン」のように数詞を促音化しない (「ヒャクブンノイチ」) */
            }
            else if (strcmp(NJDNode_get_string(node), "分") == 0 && node->next != NULL &&
                strcmp(NJDNode_get_string(node->next), "袖") == 0) {
               /* 「七分袖」の「分」は時間の「フン」ではなく割合の「ブ」と読み、数詞は「シ」「シチ」「ク」の形を使う */
               NJDNode_set_read(node, "ブ");
               NJDNode_set_pron(node, "ブ");
               NJDNode_set_mora_size(node, 1);
               convert_digit_pron(njd_set_digit_rule_conv_table1e, node->prev);
            }
            /* 「階」と「三階級」の「階」は、一・六・十・百だけを促音化し、建物の「8階」は「ハッカイ」でなく「ハチカイ」と読む */
            else if (strcmp(NJDNode_get_string(node), "階") == 0)
               convert_digit_pron(njd_set_digit_rule_conv_table1l, node->prev);
            /* 「カラット」は十と百だけを促音化する */
            else if (strcmp(NJDNode_get_string(node), "カラット") == 0)
               convert_digit_pron(njd_set_digit_rule_conv_table_ten_hundred, node->prev);
            /* 「とおり」は八と十を促音化する */
            else if (strcmp(NJDNode_get_string(node), "とおり") == 0)
               convert_digit_pron(njd_set_digit_rule_conv_table_eight_ten, node->prev);
            else if (strcmp(NJDNode_get_string(node), "棟") == 0 &&
                     strcmp(NJDNode_get_read(node), "ムネ") == 0) {
               /* 「ムネ」と読む「棟」の前では、数詞を促音化しない (「ハチムネ」「ジュームネ」) */
            }
            /* 数字の「組」は学級などの番号として読み、一は促音化せずに「イチクミ」、六・十・百は促音化する */
            else if (strcmp(NJDNode_get_string(node), "組") == 0)
               convert_digit_pron(njd_set_digit_rule_conv_table1i, node->prev);
            else if (strcmp(NJDNode_get_string(node), "試合") == 0) {
               /* 数量の「一試合」は「イッシアイ」、試合番号の「第一試合」は「ダイイチシアイ」 */
               if (node->prev->prev == NULL ||
                   strcmp(NJDNode_get_string(node->prev->prev), "第") != 0)
                  convert_digit_pron(njd_set_digit_rule_conv_table1j, node->prev);
            }
            /* 「石」は一・六・十・百を促音化し、八は「ハチ」のまま読む */
            else if (strcmp(NJDNode_get_string(node), "石") == 0)
               convert_digit_pron(njd_set_digit_rule_conv_table1l, node->prev);
            /* 「年生」は「年」と「生」に分かれて解析された場合も、四を「ヨ」と読み、七は「ナナ」のまま読む */
            else if (strcmp(NJDNode_get_string(node), "年生") == 0 ||
                     (strcmp(NJDNode_get_string(node), "年") == 0 && node->next != NULL &&
                      strcmp(NJDNode_get_string(node->next), "生") == 0))
               convert_digit_pron(njd_set_digit_rule_conv_table1b, node->prev);
            /* 「里」と、「ヤ」と読む「夜」の前では、七を「シチ」と読む (「シチリ」「シチヤ」) */
            else if (strcmp(NJDNode_get_string(node), "里") == 0 ||
                     (strcmp(NJDNode_get_string(node), "夜") == 0 &&
                      strcmp(NJDNode_get_read(node), "ヤ") == 0))
               convert_digit_pron(njd_set_digit_rule_conv_table_seven, node->prev);
            /* ゴルフの「7アンダー」のように、ほかの数詞が前にない数に「アンダー」が続く場合は、数詞を英語で「セブン」と読む */
            else if (strcmp(NJDNode_get_string(node), "アンダー") == 0 &&
                     (node->prev->prev == NULL ||
                      strcmp(NJDNode_get_pos_group1(node->prev->prev), NJD_SET_DIGIT_KAZU) != 0))
               convert_digit_pron(njd_set_digit_rule_conv_table_under, node->prev);
            else
#else
             ) {
#endif
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
            /* 表にない「ｋＷ」「ｋＨｚ」のような大文字を含む英字の単位も、「キロ」と読むものは「キロワット」と同じく「ロッキロワット」と促音にする */
            else if (search_numerative_class(njd_set_digit_rule_numerative_class1i, node) == 1 ||
                     (strncmp(NJDNode_get_string(node), "ｋ", strlen("ｋ")) == 0 &&
                      strncmp(NJDNode_get_read(node), "キロ", strlen("キロ")) == 0))
               convert_digit_pron(njd_set_digit_rule_conv_table1i, node->prev);
            else if (search_numerative_class(njd_set_digit_rule_numerative_class1j, node) == 1)
               convert_digit_pron(njd_set_digit_rule_conv_table1j, node->prev);
            else if (search_numerative_class(njd_set_digit_rule_numerative_class1k, node) == 1)
               convert_digit_pron(njd_set_digit_rule_conv_table1k, node->prev);
#if defined(CHARSET_UTF_8) && !defined(ASCII_HEADER)
            /* 「課」「缶」「球」などは一・六・十・百を促音化し、八は「ハチ」のまま読む */
            else if (search_numerative_class(njd_set_digit_rule_numerative_class1l, node) == 1)
               convert_digit_pron(njd_set_digit_rule_conv_table1l, node->prev);
            /* 「ワ」と読む「把」の前では、十を促音化する (「ジュッパ」) */
            if (strcmp(NJDNode_get_string(node), "把") == 0 &&
                strcmp(NJDNode_get_read(node), "ワ") == 0)
               convert_digit_pron(njd_set_digit_rule_conv_table1k, node->prev);
            /* convert numerative pron */
            if (uses_native_one_two(node)) {
               /* 「一箱」の「ヒト」のように和語で数える場合は、助数詞を半濁音や濁音に変えない (「ヒトハコ」) */
            } else if ((strcmp(NJDNode_get_string(node), "分") == 0 &&
                        (strcmp(NJDNode_get_read(node), "ブ") == 0 ||
                         strcmp(NJDNode_get_read(node), "ブン") == 0)) ||
                       (strcmp(NJDNode_get_string(node), "階") == 0 && node->next != NULL &&
                        strcmp(NJDNode_get_string(node->next), "級") == 0) ||
                       (strcmp(NJDNode_get_string(node), "波") == 0 &&
                        strcmp(NJDNode_get_string(node->prev), "八") == 0) ||
                       (strcmp(NJDNode_get_string(node), "鉢") == 0 &&
                        strcmp(NJDNode_get_string(node->prev), "四") == 0)) {
               /* 「一分袖」の「ブ」、分数の「ブン」、「三階級」の「カイ」、「八波」の「ハ」、「四鉢」の「ハチ」は、助数詞を半濁音や濁音に変えない */
            }
            /* 「袋」は十の後だけ「プクロ」と半濁音にする */
            else if (strcmp(NJDNode_get_string(node), "袋") == 0)
               convert_numerative_pron(njd_set_digit_rule_conv_table_ten_semivoiced, node->prev, node);
            /* 「寸」は三と何の後だけ「ズン」と濁音にし (「サンズン」「ナンズン」)、十は「ジッ」と読む (「ジッスン」) */
            else if (strcmp(NJDNode_get_string(node), "寸") == 0) {
               convert_numerative_pron(njd_set_digit_rule_conv_table2f, node->prev, node);
               if (strcmp(NJDNode_get_string(node->prev), "十") == 0)
                  NJDNode_set_pron(node->prev, "ジッ");
            }
            /* 「ワ」と読む「把」は十の後で「パ」、「羽」は千と万の後で「バ」と読む */
            else if (strcmp(NJDNode_get_string(node), "把") == 0 &&
                     strcmp(NJDNode_get_read(node), "ワ") == 0 &&
                     strcmp(NJDNode_get_string(node->prev), "十") == 0)
               NJDNode_set_pron(node, "パ");
            else if (strcmp(NJDNode_get_string(node), "羽") == 0 &&
                     strcmp(NJDNode_get_read(node), "ワ") == 0 &&
                     (strcmp(NJDNode_get_string(node->prev), "千") == 0 ||
                      strcmp(NJDNode_get_string(node->prev), "万") == 0))
               NJDNode_set_pron(node, "バ");
            else
#endif
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
#if defined(CHARSET_UTF_8) && !defined(ASCII_HEADER)
            /* 8 の後で促音にも読める助数詞は、既定を促音にしない「ハチ」にする (「ハチコ」「ハチカイ」「ハチフン」) */
            /* 促音の方が明らかに多い「本」(「ハッポン」) は表に入れず、促音の形しかない「発」「匹」なども変えない */
            if (strcmp(NJDNode_get_string(node->prev), "八") == 0 &&
                strcmp(NJDNode_get_pron(node->prev), "ハッ") == 0 &&
                search_numerative_class(njd_set_digit_rule_hachi_counters, node) == 1) {
               NJDNode_set_pron(node->prev, "ハチ");
               NJDNode_set_mora_size(node->prev, 2);
               NJDNode_set_pron(node, counter_pron_before);
            }
            /* tsqyomi が「十八歩」「2歩」の「歩」に促音の後の「ポ」の行を選ぶと、促音でも撥音でもない「ハチ」「ニ」の後でも半濁音が残るので、清音の「ホ」へ戻す */
            if (search_numerative_class(njd_set_digit_rule_hachi_counters, node) == 1 &&
                !ends_with_sokuon_or_hatsuon(NJDNode_get_pron(node->prev))) {
               static const char *handakuon[] = {"パ", "ピ", "プ", "ペ", "ポ"};
               static const char *seion[] = {"ハ", "ヒ", "フ", "ヘ", "ホ"};
               const char *pron = NJDNode_get_pron(node);
               char buff[64];
               int i;
               for (i = 0; i < 5; i++) {
                  if (strncmp(pron, handakuon[i], strlen(handakuon[i])) == 0 &&
                      strlen(pron) < sizeof(buff)) {
                     strcpy(buff, seion[i]);
                     strcat(buff, pron + strlen(handakuon[i]));
                     NJDNode_set_pron(node, buff);
                     break;
                  }
               }
            }
#endif
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
#if defined(CHARSET_UTF_8) && !defined(ASCII_HEADER)
                        /* 「十数万円」の「数」は前の「十」と1つの数なので、「数」の前で句を切らない */
                        if (!(strcmp(NJDNode_get_string(node->prev), "数") == 0 && node->prev->prev != NULL &&
                              (strcmp(NJDNode_get_string(node->prev->prev), "十") == 0 ||
                               strcmp(NJDNode_get_string(node->prev->prev), "百") == 0 ||
                               strcmp(NJDNode_get_string(node->prev->prev), "千") == 0)))
#endif
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
#if defined(CHARSET_UTF_8) && !defined(ASCII_HEADER)
                           /* 「十数万円」「十数人」の「数」は前の位と1つの数なので、「ジュースー」と句を続ける */
                           if (strcmp(NJDNode_get_string(node), "数") == 0) {
                              NJDNode_set_chain_flag(node, 1);
                              break;
                           }
#endif
                           NJDNode_set_chain_flag(node, 0);
                           break;
                        }
                     }
#if defined(CHARSET_UTF_8) && !defined(ASCII_HEADER)
                     /* 「百万円」「何百万円」「三千万円」の「十」「百」「千」に続く「万」以上の位は、同じ数として句を続ける */
                     /* 助数詞の前で句の頭にされた「万」を、ここでつなぎ直す */
                     if (i <= 2)
                        for (j = 3; njd_set_digit_rule_numeral_list5[j] != NULL; j++)
                           if (strcmp(NJDNode_get_string(node), njd_set_digit_rule_numeral_list5[j]) == 0) {
                              NJDNode_set_chain_flag(node, 1);
                              break;
                           }
#endif
                     break;
                  }
               }
#if defined(CHARSET_UTF_8) && !defined(ASCII_HEADER)
               /* 「数十万円」は「十万」が辞書の1語なので、「数」「何」「幾」に続く「十万」「百万」などの位で始まる数も句を続ける */
               if ((strcmp(NJDNode_get_string(node->prev), "数") == 0 ||
                    strcmp(NJDNode_get_string(node->prev), "何") == 0 ||
                    strcmp(NJDNode_get_string(node->prev), "幾") == 0) &&
                   strlen(NJDNode_get_string(node)) > 3)
                  for (i = 0; i <= 2; i++)
                     if (strncmp(NJDNode_get_string(node), njd_set_digit_rule_numeral_list5[i],
                                 strlen(njd_set_digit_rule_numeral_list5[i])) == 0) {
                        NJDNode_set_chain_flag(node, 1);
                        break;
                     }
#endif
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
#if defined(CHARSET_UTF_8) && !defined(ASCII_HEADER)
          /* 「1.5日」「0.2 人」の小数部は整数用の「イツカ」「フタリ」に変えず、「ゴニチ」「ニニン」と読む */
          (is_decimal_digit(node) == 0 || is_calendar_day_enumeration(node)) &&
#endif
          (node->prev == NULL
           || strcmp(NJDNode_get_pos(node->prev), NJD_SET_DIGIT_KIGOU) == 0
           || strcmp(NJDNode_get_pos_group1(node->prev), NJD_SET_DIGIT_KAZU) != 0)
          && (strcmp(NJDNode_get_pos_group2(node->next), NJD_SET_DIGIT_JOSUUSHI) == 0
              || strcmp(NJDNode_get_pos_group1(node->next), NJD_SET_DIGIT_FUKUSHIKANOU) == 0
#if defined(CHARSET_UTF_8) && !defined(ASCII_HEADER)
              || (is_decimal_digit(node) == 0 &&
                  search_numerative_class(njd_set_digit_rule_counter_words, node->next))
#endif
              )) {
         /* convert class3 */
         for (i = 0; njd_set_digit_rule_numerative_class3[i] != NULL; i += 2) {
#if defined(CHARSET_UTF_8) && !defined(ASCII_HEADER)
            /* 「第1幕」「第2日」のように「第」の後の数は順番を表すので、和語の「ヒト」「フタ」にしない */
            if (node->prev != NULL && strcmp(NJDNode_get_string(node->prev), "第") == 0)
               break;
            /* 「幕目」の前では数詞を和語の「ヒト」「フタ」「ミ」などに変えず、「イチマクメ」「サンマクメ」と読む */
            if (strcmp(NJDNode_get_string(node->next), "幕") == 0 &&
                node->next->next != NULL && strcmp(NJDNode_get_string(node->next->next), "目") == 0)
               break;
#endif
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
#if defined(CHARSET_UTF_8) && !defined(ASCII_HEADER)
         /* 神仏を数える「柱」の前では、三・四・六・八・十を和語の「ミ」「ヨ」「ム」「ヤ」「ト」と読む */
         if (strcmp(NJDNode_get_string(node->next), "柱") == 0 &&
             strcmp(NJDNode_get_read(node->next), "ハシラ") == 0)
            convert_digit_pron(njd_set_digit_rule_conv_table_native, node);
#endif
         /* person */
         /* 「一人前」は「イチニンマエ」と読むので、「人」に「前」が続くときは「ヒトリ」「フタリ」に変えない */
         if (strcmp(NJDNode_get_string(node->next), NJD_SET_DIGIT_NIN) == 0 &&
             (node->next->next == NULL ||
              strcmp(NJDNode_get_string(node->next->next), "前") != 0)) {
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
#if defined(CHARSET_UTF_8) && !defined(ASCII_HEADER)
            /* 「第2日」「第3日目」は何日目かを表す順番なので、「フツカ」「ミッカ」でなく「ダイニニチ」「ダイサンニチメ」と読む */
            if (node->prev != NULL && strcmp(NJDNode_get_string(node->prev), "第") == 0) {
            } else
            /* 「4、5日」「二、三日」の概数の「日」は日付でなく日数なので、「イツカ」「ミッカ」でなく「ニチ」と読む */
            if (is_approximate_number_comma(node->prev)) {
            } else
#endif
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

#if defined(CHARSET_UTF_8) && !defined(ASCII_HEADER)
   set_fraction_readings_of_fun_words(njd);
   geminate_one_before_kango(njd);
   set_approximate_number_readings(njd);
   set_number_list_separator_phrases(njd);
#endif
   NJD_remove_silent_node(njd);
   if (njd->head == NULL)
      return;

   set_digit_accent_rules(njd);
#if defined(CHARSET_UTF_8) && !defined(ASCII_HEADER)
   set_kaikyuu_counter_accents(njd);
   set_man_yen_accents(njd);
   set_approximate_counter_accents(njd);
   set_day_word_suffix_accents(njd);
   finish_number_sequences(number_sequences);
   set_restored_month_accents(njd);
   set_identifier_numerical_accents(njd);
   set_flight_number_accent(njd);
   set_railway_series_accent(njd);
   set_written_group_readings(njd);
#endif
}

NJD_SET_DIGIT_C_END;

#endif                          /* !NJD_SET_DIGIT_C */

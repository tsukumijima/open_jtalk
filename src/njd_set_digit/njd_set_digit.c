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
         else if (identifier_numerical_reading(s, final_digit))
            numerical_reading = 1;
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
   "貫", "貫目", "軒", "階", "騎",
   NULL
};

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

      /* 「人」は2桁以上の数と「六」「七」「八」「九 (キュー)」の後で前部末型にし、1拍で読む「四 (ヨ)」「五」「九 (ク)」の後では助数詞のアクセント核をそのまま使う */
      if (strcmp(NJDNode_get_string(counter), NJD_SET_DIGIT_NIN) == 0 &&
          ((is_compound && !(NJDNode_get_mora_size(node) == 1 &&
                            (digit == 4 || digit == 5 || digit == 9))) ||
           (!is_compound && (digit == 6 || digit == 7 || digit == 8 ||
                            (digit == 9 && NJDNode_get_mora_size(node) == 2)))))
         NJDNode_set_chain_rule(counter, "C3");

      /* 前部末型でアクセント核が撥音・長音・促音に当たる数詞は、核を1拍前へずらす */
      if (strcmp(NJDNode_get_chain_rule(counter), "C3") == 0 &&
          (digit == 3 || ((digit == 4 || digit == 9) && NJDNode_get_mora_size(node) == 2) ||
           strcmp(NJDNode_get_string(node), NJD_SET_DIGIT_TEN) == 0 ||
           strcmp(NJDNode_get_string(node), njd_set_digit_rule_numeral_list5[2]) == 0))
         NJDNode_set_chain_rule(counter, "F4@-1");

      /* 「石」は一・六・八の後と、単独の「十」の後で尾高型にする (「イッコク＼」「ジュッコク＼」) */
      if (strcmp(NJDNode_get_string(counter), "石") == 0 &&
          (digit == 1 || digit == 6 || digit == 8 ||
           (!is_compound && strcmp(NJDNode_get_string(node), NJD_SET_DIGIT_TEN) == 0)))
         NJDNode_set_chain_rule(counter, "F4@2");

      /* 「十一日」「十二日」などは尾高型で結合する */
      if (strcmp(NJDNode_get_string(counter), NJD_SET_DIGIT_NICHI) == 0 && is_compound &&
          (digit == 1 || digit == 2 || digit == 6 || digit == 7 || digit == 8))
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

      /* 「五」に続く「本」「枚」「台」「代」「番」「年」は、数詞の桁にかかわらず後部を平板型にする */
      if (digit == 5 && (strcmp(NJDNode_get_string(counter), "本") == 0 ||
                        strcmp(NJDNode_get_string(counter), "枚") == 0 ||
                        strcmp(NJDNode_get_string(counter), "台") == 0 ||
                        strcmp(NJDNode_get_string(counter), "代") == 0 ||
                        strcmp(NJDNode_get_string(counter), "番") == 0 ||
                        strcmp(NJDNode_get_string(counter), "年") == 0))
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

      /* 「日目」は尾高型、「人前」は平板型にし、別のノードに分かれた「目」「前」まで1つのアクセント句にまとめる */
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

static int has_phone_context(NJDNode *start, NJDNode *end)
{
   NJDNode *node;
   int distance = 0;
   /* 「119に電話する」「119 に電話する」は直後の「に電話」で発信先の番号と判定し、「イチイチキュー」と読む */
   node = end->next;
   while (node != NULL && strcmp(NJDNode_get_pos_group3(node), "空白境界") == 0)
      node = node->next;
   /* 「話者1に電話」のように名詞へ直接付く数字は名前の一部なので、独立した番号だけを後続の「に電話」で判定する */
   if (node != NULL && strcmp(NJDNode_get_string(node), "に") == 0 &&
       (start->prev == NULL || strcmp(NJDNode_get_pos(start->prev), "名詞") != 0)) {
      node = node->next;
      while (node != NULL && strcmp(NJDNode_get_pos_group3(node), "空白境界") == 0)
         node = node->next;
      if (node != NULL && strcmp(NJDNode_get_string(node), "電話") == 0)
         return 1;
   }
   /* 「市外局番213の、486ー2435」「電話 03 1234 5678」は区切りと数字をたどり、電話番号の最後の組まで文脈を保つ */
   for (node = start->prev; node != NULL && distance < 16; node = node->prev, distance++) {
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
   int digit, index = 0, group_index = 0;
   for (node = start; node != end->next; node = node->next, index++) {
      /* 「07032245679」は休止のない3-4-4の組として数え、組ごとにアクセント句を分ける */
      if (first_group_size > 0 &&
          (index == first_group_size || index == first_group_size + (first_group_size == 4 ? 3 : 4)))
         group_index = 0;
      /* 保護中は品詞を一時変更しているため、値を調べる間だけ数詞に戻す */
      NJDNode_set_pos_group1(node, NJD_SET_DIGIT_KAZU);
      digit = number_digit(node);
      NJDNode_set_pos_group1(node, "一般");
      NJDNode_set_read(node, (char *) readings[digit]);
      NJDNode_set_pron(node, (char *) readings[digit]);
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

static int identifier_numerical_reading(NJDNode *start, NJDNode *end)
{
   NJDNode *node;
   int size = 0;
   if (!has_identifier_context(start))
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
      else if (groups == 1 && size[0] == 7 && postal_context && !has_quantity_suffix) {
         if (protect_number_sequence(&sequences, node, end[0]))
            set_phone_digit_reading(node, end[0], 3);
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
   set_written_place_chain_rules(njd);
   /* 「3 本」の「ホン」を助数詞へ戻してから、「サンボン」の濁音化とアクセント結合を適用する */
   restore_counter_features(njd);
   number_sequences = prepare_number_sequences(njd);
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
            /* convert digit pron */
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
            else if (search_numerative_class(njd_set_digit_rule_numerative_class1i, node) == 1)
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
            /* 「寸」は三の後だけ「ズン」と濁音にする */
            else if (strcmp(NJDNode_get_string(node), "寸") == 0)
               convert_numerative_pron(njd_set_digit_rule_conv_table2f, node->prev, node);
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
            /* tsqyomi が「十八歩」の「歩」に促音の後の「ポ」の行を選ぶと、「ハチ」の後でも半濁音が残るので清音の「ホ」へ戻す */
            if (strcmp(NJDNode_get_string(node->prev), "八") == 0 &&
                strcmp(NJDNode_get_pron(node->prev), "ハチ") == 0 &&
                search_numerative_class(njd_set_digit_rule_hachi_counters, node) == 1) {
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
#if defined(CHARSET_UTF_8) && !defined(ASCII_HEADER)
   finish_number_sequences(number_sequences);
   set_restored_month_accents(njd);
   set_identifier_numerical_accents(njd);
   set_flight_number_accent(njd);
   set_railway_series_accent(njd);
#endif
}

NJD_SET_DIGIT_C_END;

#endif                          /* !NJD_SET_DIGIT_C */

//  MeCab -- Yet Another Part-of-Speech and Morphological Analyzer
//
//
//  Copyright(C) 2001-2006 Taku Kudo <taku@chasen.org>
//  Copyright(C) 2004-2006 Nippon Telegraph and Telephone Corporation

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

#ifndef MECAB_CPP
#define MECAB_CPP

#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <vector>

#include "mecab.h"

#ifdef __cplusplus
#define MECAB_CPP_START extern "C" {
#define MECAB_CPP_END   }
#else
#define MECAB_CPP_START
#define MECAB_CPP_END
#endif                          /* __CPLUSPLUS */

MECAB_CPP_START;

BOOL Mecab_initialize(Mecab *m)
{
   m->feature = NULL;
   m->size = 0;
   m->model = NULL;
   m->tagger = NULL;
   m->lattice = NULL;
   return TRUE;
}

BOOL Mecab_load(Mecab *m, const char *dicdir)
{
   return Mecab_load_with_userdic(m, dicdir, NULL);
}

void Mecab_print_load_error(const char *dicdir, const char *userdic) {
   if (userdic == NULL) {
      fprintf(stderr, "ERROR: Mecab_load() in mecab.cpp: Cannot open %s.\n", dicdir);
   } else {
      fprintf(stderr, "ERROR: Mecab_load_with_userdic() in mecab.cpp: Cannot open %s or %s.\n", dicdir, userdic);
   }
}

void Mecab_free_argv(char **argv, int argc_to_free) {
   int index;

   if (argv == NULL) {
      return;
   }

   for (index = 0; index < argc_to_free; ++index) {
      free(argv[index]);
   }
   free(argv);
}


BOOL Mecab_load_with_userdic(Mecab *m, const char *dicdir, const char *userdic)
{
   int i;
   int argc;
   if (userdic == NULL) {
      argc = 3;
   } else {
      argc = 5;
   }
   char **argv;

   if(m == NULL)
      return FALSE;

   if(dicdir == NULL || strlen(dicdir) == 0)
      return FALSE;

   Mecab_clear(m);

   argv = (char **) malloc(sizeof(char *) * argc);
   if (argv == NULL) {
      return FALSE;
   }
   for (i = 0; i < argc; ++i) {
      argv[i] = NULL;
   }

   argv[0] = strdup("mecab");
   argv[1] = strdup("-d");
   argv[2] = strdup(dicdir);

   if (userdic != NULL) {
      argv[3] = strdup("-u");
      argv[4] = strdup(userdic);
   }
   for (i = 0; i < argc; ++i) {
      if (argv[i] == NULL) {
         Mecab_free_argv(argv, argc);
         return FALSE;
      }
   }

   MeCab::Model *model = MeCab::createModel(argc, argv);

   Mecab_free_argv(argv, argc);

   if(model == NULL) {
      Mecab_print_load_error(dicdir, userdic);
      return FALSE;
   }

   MeCab::Tagger *tagger = model->createTagger();
   if(tagger == NULL) {
      delete model;
      Mecab_print_load_error(dicdir, userdic);
      return FALSE;
   }

   MeCab::Lattice *lattice = model->createLattice();
   if(lattice == NULL) {
      delete model;
      delete tagger;
      Mecab_print_load_error(dicdir, userdic);
      return FALSE;
   }

   m->model = (void *) model;
   m->tagger = (void *) tagger;
   m->lattice = (void *) lattice;

   return TRUE;
}

/* 漢数字の1字 (UTF-8 で3バイト) から始まるかを返す。digit_only が真なら「十」「百」などの位の字を含めない */
static bool Mecab_starts_with_kanji_numeral(const char *str, size_t size, size_t pos, bool digit_only)
{
   static const char *digits[] = {"〇", "一", "二", "三", "四", "五", "六", "七", "八", "九"};
   static const char *places[] = {"十", "百", "千", "万", "億", "兆"};
   size_t i;
   if (pos + 3 > size)
      return false;
   for (i = 0; i < sizeof(digits) / sizeof(digits[0]); i++)
      if (memcmp(str + pos, digits[i], 3) == 0)
         return true;
   if (digit_only)
      return false;
   for (i = 0; i < sizeof(places) / sizeof(places[0]); i++)
      if (memcmp(str + pos, places[i], 3) == 0)
         return true;
   return false;
}

/* 1桁の数字 (漢数字か、text2mecab が全角にした算用数字) が pos で終わるかを返す */
static bool Mecab_ends_with_digit(const char *str, size_t size, size_t pos)
{
   if (pos < 3)
      return false;
   /* 全角の「０」〜「９」は EF BC 90 〜 EF BC 99 */
   if ((unsigned char) str[pos - 3] == 0xEF && (unsigned char) str[pos - 2] == 0xBC &&
       (unsigned char) str[pos - 1] >= 0x90 && (unsigned char) str[pos - 1] <= 0x99)
      return true;
   return Mecab_starts_with_kanji_numeral(str, size, pos - 3, true);
}

/* 1桁の数字が pos から始まるかを返す */
static bool Mecab_starts_with_digit(const char *str, size_t size, size_t pos)
{
   return Mecab_ends_with_digit(str, size, pos + 3);
}

/* 電話番号などの数字の組を区切るハイフンの長さを返す。ハイフンでなければ0を返す */
static size_t Mecab_hyphen_length_at(const char *str, size_t size, size_t pos, bool ends_at_pos)
{
   static const char *hyphens[] = {"−", "－", "‐", "‑", "‒", "–", "—", "ー", "-"};
   size_t i, length;
   for (i = 0; i < sizeof(hyphens) / sizeof(hyphens[0]); i++) {
      length = strlen(hyphens[i]);
      if (ends_at_pos) {
         if (pos >= length && memcmp(str + pos - length, hyphens[i], length) == 0)
            return length;
      } else if (pos + length <= size && memcmp(str + pos, hyphens[i], length) == 0) {
         return length;
      }
   }
   return 0;
}

static bool Mecab_is_numeral_node(const MeCab::Node *node)
{
   return node->feature != NULL && strncmp(node->feature, "名詞,数,", strlen("名詞,数,")) == 0;
}

/* 語の表層が、位の字を含む漢数字だけでできているかを返す */
static bool Mecab_is_kanji_numeral_word(const char *str, size_t size, const MeCab::Node *node)
{
   const size_t begin = node->surface - str;
   size_t pos;
   if (node->length == 0 || node->length % 3 != 0)
      return false;
   for (pos = begin; pos < begin + node->length; pos += 3)
      if (!Mecab_starts_with_kanji_numeral(str, size, pos, false))
         return false;
   return true;
}

/* 漢数字の並びを、算用数字と同じく1字ずつの形態素に分ける必要がある並びを探し、各字の境目の位置を返す
   1. 「二十三」を「二」+「十三」、「二十八時間」を「二」+「十」+「八時間」のように、数詞の後で並びの途中から始まる語を選んだ並び
      「十三」「八時間」「十六日」は辞書の語なので、数の一部として読むと区切りが崩れ、位取りとアクセントが算用数字と食い違う
      並びの先頭から始まる「八百屋」「二十歳」「五十嵐」のような語は、数ではない語として残す
      区切るのは漢数字だけでできた語の並びと、その後ろの語の先頭の漢数字に限り、「唯一」+「一人」の「唯一」や、
      ユーザー辞書の「正一」+「一万円」の「正一」のように、漢数字以外の字を含む語の中には区切りを入れない
   2. 「〇三-九九-〇〇」のように、ハイフンを挟んで数字とつながる桁読みの漢数字の並び
      電話番号の組の「九九」を掛け算の「クク」、「〇〇〇」をハイフンとまとめた1つの記号として解析させない */
static void Mecab_collect_numeral_boundaries(MeCab::Lattice *lattice, std::vector<size_t> *boundaries)
{
   const char *str = lattice->sentence();
   const size_t size = lattice->size();
   const MeCab::Node *node, *prev = NULL;
   size_t begin, end, pos, hyphen_length, chain_begin = 0;
   bool is_junction, is_linked;

   for (node = lattice->bos_node(); node != NULL; node = node->next) {
      if (node->stat == MECAB_BOS_NODE || node->stat == MECAB_EOS_NODE) {
         prev = NULL;
         continue;
      }
      pos = node->surface - str;
      if (prev != NULL && prev->surface + prev->length == node->surface) {
         /* 数詞の後の複数字の語 (「二」+「十三」) と、数詞の前の複数字の語 (「十三」+「万」) を、数の途中の区切りとみなす */
         is_junction = Mecab_is_kanji_numeral_word(str, size, prev) &&
                       Mecab_starts_with_kanji_numeral(str, size, pos, false) &&
                       ((Mecab_is_numeral_node(prev) && node->length > 3) ||
                        (Mecab_is_numeral_node(node) && prev->length > 3));
         if (is_junction) {
            /* 前は漢数字だけの語が続く範囲の先頭から、後ろはこの語の中の先頭の漢数字の範囲までを1字ずつに分ける */
            for (end = pos; end < pos + node->length && Mecab_starts_with_kanji_numeral(str, size, end, false); end += 3);
            for (begin = chain_begin; begin <= end; begin += 3)
               boundaries->push_back(begin);
         }
      }
      /* 漢数字だけの語が隣り合って続く範囲の先頭を覚えておく */
      if (Mecab_is_kanji_numeral_word(str, size, node) &&
          (prev == NULL || prev->surface + prev->length != node->surface ||
           !Mecab_is_kanji_numeral_word(str, size, prev)))
         chain_begin = pos;
      prev = node;
   }

   for (pos = 0; pos < size;) {
      if (!Mecab_starts_with_kanji_numeral(str, size, pos, true)) {
         pos++;
         continue;
      }
      for (begin = pos, end = pos; Mecab_starts_with_kanji_numeral(str, size, end, true); end += 3);
      hyphen_length = Mecab_hyphen_length_at(str, size, begin, true);
      is_linked = hyphen_length > 0 && Mecab_ends_with_digit(str, size, begin - hyphen_length);
      hyphen_length = Mecab_hyphen_length_at(str, size, end, false);
      is_linked = is_linked || (hyphen_length > 0 && Mecab_starts_with_digit(str, size, end + hyphen_length));
      if (is_linked) {
         for (; begin <= end; begin += 3)
            boundaries->push_back(begin);
      }
      pos = end;
   }
}

/* 文を解析し、漢数字の並びを1字ずつに分ける必要があれば、その境目に境界制約を付けて解析し直す
   OpenJTalk の Mecab_analysis() と、tsqyomi が使う候補解析の両方から呼び、同じ最良経路を返す */
BOOL Mecab_parse_lattice_with_numeral_boundaries(void *tagger, void *lattice, const char *str)
{
   MeCab::Tagger *mecab_tagger = (MeCab::Tagger *) tagger;
   MeCab::Lattice *mecab_lattice = (MeCab::Lattice *) lattice;
   std::vector<size_t> boundaries;
   size_t i;

   mecab_lattice->set_sentence(str);
   if (mecab_tagger->parse(mecab_lattice) == false)
      return FALSE;
   Mecab_collect_numeral_boundaries(mecab_lattice, &boundaries);
   if (boundaries.empty())
      return TRUE;

   /* set_sentence() は前回の解析のノードと境界制約を消すので、文を設定し直してから制約を付ける */
   mecab_lattice->set_sentence(str);
   for (i = 0; i < boundaries.size(); i++)
      mecab_lattice->set_boundary_constraint(boundaries[i], MECAB_TOKEN_BOUNDARY);
   return mecab_tagger->parse(mecab_lattice) ? TRUE : FALSE;
}

BOOL Mecab_analysis(Mecab *m, const char *str)
{
   if(m->model == NULL || m->tagger == NULL || m->lattice == NULL || str == NULL)
      return FALSE;

   if(m->size > 0 || m->feature != NULL)
      Mecab_refresh(m);

   MeCab::Tagger *tagger = (MeCab::Tagger *) m->tagger;
   MeCab::Lattice *lattice = (MeCab::Lattice *) m->lattice;

   if(Mecab_parse_lattice_with_numeral_boundaries(tagger, lattice, str) == FALSE) {
      lattice->clear();
      return FALSE;
   }

   for (const MeCab::Node* node = lattice->bos_node(); node; node = node->next) {
      if(node->stat != MECAB_BOS_NODE && node->stat != MECAB_EOS_NODE)
         m->size++;
   }

   if(m->size == 0)
      return TRUE;

   m->feature = (char **) calloc(m->size, sizeof(char *));
   if (m->feature == NULL) {
      m->size = 0;
      lattice->clear();
      return FALSE;
   }
   int index = 0;
   for (const MeCab::Node* node = lattice->bos_node(); node; node = node->next) {
      if(node->stat != MECAB_BOS_NODE && node->stat != MECAB_EOS_NODE) {
         std::string f(node->surface, node->length);
         f += ",";
         f += node->feature;
         m->feature[index] = strdup(f.c_str());
         if (m->feature[index] == NULL) {
            for (int cleanup_index = 0; cleanup_index < index; ++cleanup_index) {
               free(m->feature[cleanup_index]);
            }
            free(m->feature);
            m->feature = NULL;
            m->size = 0;
            lattice->clear();
            return FALSE;
         }
         index++;
      }
   }

   /* lattice->clear() はここでは呼ばない。
      OpenJTalk.run_mecab_detailed() が Mecab_analysis() 後に lattice ノードを走査するため、
      lattice の解放は Mecab_refresh() に委譲する。
      元のコードでは lattice->clear() をここで呼んでいたが、
      clear() は end_nodes_ を空にするため、その後の mecab_lattice_get_bos_node() が
      空ベクタにアクセスする未定義動作を引き起こしていた。 */

   return TRUE;
}

BOOL Mecab_print(Mecab *m)
{
   int i;

   for(i = 0; i < m->size; i++)
      printf("%s\n", m->feature[i]);
   return TRUE;
}

int Mecab_get_size(Mecab *m)
{
   return m->size;
}

char **Mecab_get_feature(Mecab *m)
{
   return m->feature;
}

BOOL Mecab_refresh(Mecab *m)
{
   int i;

   if(m->feature != NULL) {
      for(i = 0; i < m->size; i++)
         free(m->feature[i]);
      free(m->feature);
      m->feature = NULL;
      m->size = 0;
   }

   /* Mecab_analysis() が lattice->clear() を呼ばなくなったため、
      ここで lattice の FreeList をリセットする。
      これにより lattice ノードの走査が完了した後に安全にクリーンアップされる。 */
   if(m->lattice != NULL) {
      MeCab::Lattice *lattice = (MeCab::Lattice *) m->lattice;
      lattice->clear();
   }

   return TRUE;
}

BOOL Mecab_clear(Mecab *m)
{
   Mecab_refresh(m);

   if(m->lattice) {
      MeCab::Lattice *lattice = (MeCab::Lattice *) m->lattice;
      delete lattice;
      m->lattice = NULL;
   }

   if(m->tagger) {
      MeCab::Tagger *tagger = (MeCab::Tagger *) m->tagger;
      delete tagger;
      m->tagger = NULL;
   }

   if(m->model) {
      MeCab::Model *model = (MeCab::Model *) m->model;
      delete model;
      m->model = NULL;
   }

   return TRUE;
}

MECAB_CPP_END;

#endif                          /* !MECAB_CPP */

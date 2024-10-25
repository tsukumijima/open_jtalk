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

#ifndef NJD_SET_UNVOICED_VOWEL_RULE_H
#define NJD_SET_UNVOICED_VOWEL_RULE_H

#ifdef __cplusplus
#define NJD_SET_UNVOICED_VOWEL_RULE_H_START extern "C" {
#define NJD_SET_UNVOICED_VOWEL_RULE_H_END   }
#else
#define NJD_SET_UNVOICED_VOWEL_RULE_H_START
#define NJD_SET_UNVOICED_VOWEL_RULE_H_END
#endif                          /* __CPLUSPLUS */

NJD_SET_UNVOICED_VOWEL_RULE_H_START;

/*
  ̵���Ҳ�: k ky s sh t ty ch ts h f hy p py
  Rule 0 �ե��顼��̵�������ʤ�
  Rule 1 ��ư��Ρ֤Ǥ��פȡ֤ޤ��פΡ֤��פ�̵����
  Rule 2 ư�졤��ư�졤����Ρ֤��פ�̵�������䤹��
  Rule 3 ³����̵�������ʤ�
  Rule 4 ��������ȳˤ�̵�������ʤ�
  Rule 5 ̵���Ҳ�(k ky s sh t ty ch ts h f hy p py)�˰Ϥޤ줿��i�פȡ�u�פ�̵����
         �㳰��s->s, s->sh, f->f, f->h, f->hy, h->f, h->h, h->hy
*/

#define NJD_SET_UNVOICED_VOWEL_FILLER "�ե��顼"
#define NJD_SET_UNVOICED_VOWEL_DOUSHI "ư��"
#define NJD_SET_UNVOICED_VOWEL_JODOUSHI "��ư��"
#define NJD_SET_UNVOICED_VOWEL_JOSHI "����"
#define NJD_SET_UNVOICED_VOWEL_KANDOUSHI "��ư��"
#define NJD_SET_UNVOICED_VOWEL_TOUTEN "��"
#define NJD_SET_UNVOICED_VOWEL_QUESTION "��"
#define NJD_SET_UNVOICED_VOWEL_EXCLAMATION "��" // added by me
#define NJD_SET_UNVOICED_VOWEL_QUOTATION "��"
#define NJD_SET_UNVOICED_VOWEL_SHI "��"
#define NJD_SET_UNVOICED_VOWEL_MA "��"
#define NJD_SET_UNVOICED_VOWEL_DE "��"
#define NJD_SET_UNVOICED_VOWEL_CHOUON "��"
#define NJD_SET_UNVOICED_VOWEL_SU "��"

static const char *njd_set_unvoiced_vowel_candidate_list1[] = {
   "����",                    /* s i */
   "����",                    /* s i */
   "��",                      /* s u */
   NULL
};

static const char *njd_set_unvoiced_vowel_next_mora_list1[] = {
   "��",                       /* k ky */
   "��",
   "��",
   "��",
   "��",
   "��",                       /* t ty ch ts */
   "��",
   "��",
   "��",
   "��",
   "��",                       /* h f hy */
   "��",
   "��",
   "��",
   "��",
   "��",                       /* p py */
   "��",
   "��",
   "��",
   "��",
   NULL
};

static const char *njd_set_unvoiced_vowel_candidate_list2[] = {
   "�ե�",                     /* f i */
   "��",                       /* h i */
   "��",                       /* f u */
   NULL
};

static const char *njd_set_unvoiced_vowel_next_mora_list2[] = {
   "��",                       /* k ky */
   "��",
   "��",
   "��",
   "��",
   "��",                       /* s sh */
   "��",
   "��",
   "��",
   "��",
   "��",                       /* t ty ch ts */
   "��",
   "��",
   "��",
   "��",
   "��",                       /* p py */
   "��",
   "��",
   "��",
   "��",
   NULL
};

static const char *njd_set_unvoiced_vowel_candidate_list3[] = {
   "����",                    /* ky u */
   "����",                    /* sh u */
   "����",                    /* ch u */
   "�ĥ�",                    /* ts i */
   "�ҥ�",                    /* hy u */
   "�ԥ�",                    /* py u */
   "�ƥ�",                    /* ty u */
   "�ȥ�",                    /* t u */
   "�ƥ�",                    /* t i */
   "��",                       /* k i */
   "��",                       /* k u */
   "��",                       /* sh i */
   "��",                       /* ch i */
   "��",                       /* ts u */
   "��",                       /* p i */
   "��",                       /* p u */
   NULL
};

static const char *njd_set_unvoiced_vowel_next_mora_list3[] = {
   "��",                       /* k ky */
   "��",
   "��",
   "��",
   "��",
   "��",                       /* s sh */
   "��",
   "��",
   "��",
   "��",
   "��",                       /* t ty ch ts */
   "��",
   "��",
   "��",
   "��",
   "��",                       /* h f hy */
   "��",
   "��",
   "��",
   "��",
   "��",                       /* p py */
   "��",
   "��",
   "��",
   "��",
   NULL
};

static const char *njd_set_unvoiced_vowel_mora_list[] = {
   "����",
   "����",
   "����",
   "����",
   "����", 
   "����", 
   "����",
   "����", 
   "����", 
   "����", 
   "����",
   "����",
   "��",
   "�ǥ�",
   "����",
   "�ե�",
   "����",
   "����",
   "����",
   "����",
   "����",
   "����",
   "����",
   "��",
   "��",
   "��",
   "��",
   "��",
   "��",
   "��",
   "��",
   "��",
   "���",
   "���",
   "���",
   "�ꥧ",
   "��",
   "��",
   "��",
   "��",
   "��",
   "��",
   "��",
   "��",
   "��",
   "��",
   "��",
   "�ߥ�",
   "�ߥ�",
   "�ߥ�",
   "�ߥ�",
   "��",
   "��",
   "��",
   "��",
   "��",
   "��",
   "��",
   "��",
   "��",
   "��",
   "�ե�",
   "�ե�",
   "�ե�",
   "�ե�",
   "��",
   "�ԥ�",
   "�ԥ�",
   "�ԥ�",
   "�ԥ�",
   "��",
   "�ӥ�",
   "�ӥ�",
   "�ӥ�",
   "�ӥ�",
   "��",
   "�ҥ�",
   "�ҥ�",
   "�ҥ�",
   "�ҥ�",
   "��",
   "��",
   "��",
   "��",
   "��",
   "��",
   "��",
   "�˥�",
   "�˥�",
   "�˥�",
   "�˥�",
   "��",
   "��",
   "�ɥ�",
   "��",
   "�ȥ�",
   "��",
   "�ǥ�",
   "�ǥ�",
   "�ǥ�",
   "�ǥ�",
   "��",
   "�ƥ�",
   "�ƥ�",
   "�ƥ�",
   "�ƥ�",
   "��",
   "��",
   "�ĥ�",
   "�ĥ�",
   "�ĥ�",
   "�ĥ�",
   "��",
   "��",
   "��",
   "����",
   "����",
   "����",
   "����",
   "��",
   "��",
   "��",
   "��",
   "��",
   "��",
   "��",
   "����",
   "��",
   "����",
   "��",
   "����",
   "����",
   "����",
   "����",
   "��",
   "����",
   "����",
   "����",
   "����",
   "��",
   "��",
   "��",
   "��",
   "��",
   "��",
   "��",
   "��",
   "��",
   "����",
   "����",
   "����",
   "����",
   "��",
   "����",
   "����",
   "����",
   "����",
   "��",
   "��",
   "��",
   "��",
   "��",
   "��",
   "��",
   "����",
   "����",
   "����",
   "��",
   "��",
   "����",
   "��",
   "��",
   "��",
   "��",
   "��",
   NULL
};

NJD_SET_UNVOICED_VOWEL_RULE_H_END;

#endif                          /* !NJD_SET_UNVOICED_VOWEL_RULE_H */

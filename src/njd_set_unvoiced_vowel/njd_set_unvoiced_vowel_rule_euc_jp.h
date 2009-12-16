/* ----------------------------------------------------------------- */
/*           The HMM-Based Speech Synthesis System (HTS)             */
/*           Open JTalk developed by HTS Working Group               */
/*           http://open-jtalk.sourceforge.net/                      */
/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2008-2009  Nagoya Institute of Technology          */
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

/*
  ̵���Ҳ�: k ky s sh t ty ch ts h f hy p py
  Rule 1 ��ư��Ρ֤Ǥ��פȡ֤ޤ��פΡ֤��פ�̵����
  Rule 2 ³����̵�������ʤ�
  Rule 3 ��������ȳˤ�̵�������ʤ�
  Rule 4 ̵���Ҳ�(k ky s sh t ty ch ts h f hy p py)�˰Ϥޤ줿��i�פȡ�u�פ�̵����
*/

#define NJD_SET_UNVOICED_VOWEL_QUOTATION "��"
#define NJD_SET_UNVOICED_VOWEL_QUESTION "��"
#define NJD_SET_UNVOICED_VOWEL_TOUTEN "��"

#define NJD_SET_UNVOICED_VOWEL_JODOUSHI "��ư��"

static const char *njd_set_unvoiced_vowel_jodoushi_table[] = {
   "�ǥ�", "�ǥ���", "2",
   "�ޥ�", "�ޥ���", "2",
   NULL, NULL
};

static const char *njd_set_unvoiced_vowel_candidate_list[] = {
   "����",                      /* ky u */
   "����",                      /* sh u */
   "����",                      /* s i */
   "����",                      /* ch u */
   "�ĥ�",                      /* ts i */
   "�ҥ�",                      /* hy u */
   "�ե�",                      /* f i */
   "�ԥ�",                      /* py u */
   "�ƥ�",                      /* ty u */
   "�ȥ�",                      /* t u */
   "�ƥ�",                      /* t i */
   "��",                        /* k i */
   "��",                        /* k u */
   "��",                        /* sh i */
   "��",                        /* s u */
   "��",                        /* ch i */
   "��",                        /* ts u */
   "��",                        /* h i */
   "��",                        /* f u */
   "��",                        /* p i */
   "��",                        /* p u */
   NULL
};

static const char *njd_set_unvoiced_vowel_next_mora_list[] = {
   "��",                        /* k ky */
   "��",
   "��",
   "��",
   "��",
   "��",                        /* s sh */
   "��",
   "��",
   "��",
   "��",
   "��",                        /* t ty ch ts */
   "��",
   "��",
   "��",
   "��",
   "��",                        /* h f hy */
   "��",
   "��",
   "��",
   "��",
   "��",                        /* p py */
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
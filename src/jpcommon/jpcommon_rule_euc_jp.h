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

#ifndef JPCOMMON_RULE_H
#define JPCOMMON_RULE_H

#ifdef __cplusplus
#define JPCOMMON_RULE_H_START extern "C" {
#define JPCOMMON_RULE_H_END   }
#else
#define JPCOMMON_RULE_H_START
#define JPCOMMON_RULE_H_END
#endif                          /* __CPLUSPLUS */

JPCOMMON_RULE_H_START;

#define JPCOMMON_MORA_UNVOICE "��"
#define JPCOMMON_MORA_LONG_VOWEL "��"
#define JPCOMMON_MORA_SHORT_PAUSE "��"
#define JPCOMMON_MORA_QUESTION "��"
#define JPCOMMON_MORA_EXCLAMATION "��" // added by me
#define JPCOMMON_PHONEME_SHORT_PAUSE "pau"
#define JPCOMMON_PHONEME_SILENT "sil"
#define JPCOMMON_PHONEME_UNKNOWN "xx"
#define JPCOMMON_FLAG_QUESTION "1"
#define JPCOMMON_FLAG_EXCLAMATION "1" // added by me

static const char *jpcommon_unvoice_list[] = {
   "a", "A",
   "i", "I",
   "u", "U",
   "e", "E",
   "o", "O",
   NULL, NULL
};

static const char *jpcommon_mora_list[] = {
   "����", "kw", "a",
   "����", "kw", "i",
   "����", "kw", "u",
   "����", "kw", "e",
   "����", "kw", "o",
   "����", "kw", "a",
   "����", "gw", "a",
   "����", "gw", "i",
   "����", "gw", "u",
   "����", "gw", "e",
   "����", "gw", "o",
   "����", "gw", "a",
   "�ǥ�", "dy", "e",
   "����", "s", "i",
   "�ե�", "fy", "u",
   "��", "w", "a",
   "����", "by", "o",
   "����", "by", "u",
   "����", "by", "a",
   "����", "v", "o",
   "����", "v", "e",
   "����", "v", "i",
   "����", "v", "a",
   "��", "v", "u",
   "��", "N", NULL,
   "��", "o", NULL,
   "��", "e", NULL,
   "��", "i", NULL,
   "��", "w", "a",
   "��", "w", "a",
   "��", "r", "o",
   "��", "r", "e",
   "��", "r", "u",
   "���", "ry", "o",
   "���", "ry", "u",
   "���", "ry", "a",
   "�ꥧ", "ry", "e",
   "��", "r", "i",
   "��", "r", "a",
   "��", "y", "o",
   "��", "y", "o",
   "��", "y", "u",
   "��", "y", "u",
   "��", "y", "a",
   "��", "y", "a",
   "��", "m", "o",
   "��", "m", "e",
   "��", "m", "u",
   "�ߥ�", "my", "o",
   "�ߥ�", "my", "u",
   "�ߥ�", "my", "a",
   "�ߥ�", "my", "e",
   "��", "m", "i",
   "��", "m", "a",
   "��", "p", "o",
   "��", "b", "o",
   "��", "h", "o",
   "��", "p", "e",
   "��", "b", "e",
   "��", "h", "e",
   "��", "p", "u",
   "��", "b", "u",
   "�ե�", "f", "o",
   "�ե�", "f", "e",
   "�ե�", "f", "i",
   "�ե�", "f", "a",
   "��", "f", "u",
   "�ԥ�", "py", "o",
   "�ԥ�", "py", "u",
   "�ԥ�", "py", "a",
   "�ԥ�", "py", "e",
   "��", "p", "i",
   "�ӥ�", "by", "o",
   "�ӥ�", "by", "u",
   "�ӥ�", "by", "a",
   "�ӥ�", "by", "e",
   "��", "b", "i",
   "�ҥ�", "hy", "o",
   "�ҥ�", "hy", "u",
   "�ҥ�", "hy", "a",
   "�ҥ�", "hy", "e",
   "��", "h", "i",
   "��", "p", "a",
   "��", "b", "a",
   "��", "h", "a",
   "��", "n", "o",
   "��", "n", "e",
   "��", "n", "u",
   "�˥�", "ny", "o",
   "�˥�", "ny", "u",
   "�˥�", "ny", "a",
   "�˥�", "ny", "e",
   "��", "n", "i",
   "��", "n", "a",
   "�ɥ�", "d", "u",
   "��", "d", "o",
   "�ȥ�", "t", "u",
   "��", "t", "o",
   "�ǥ�", "dy", "o",
   "�ǥ�", "dy", "u",
   "�ǥ�", "dy", "a",
   "�ǥ�", "d", "i",
   "��", "d", "e",
   "�ƥ�", "ty", "o",
   "�ƥ�", "ty", "u",
   "�ƥ�", "ty", "a",
   "�ƥ�", "t", "i",
   "��", "t", "e",
   "��", "z", "u",
   "�ĥ�", "ts", "o",
   "�ĥ�", "ts", "e",
   "�ĥ�", "ts", "i",
   "�ĥ�", "ts", "a",
   "��", "ts", "u",
   "��", "cl", NULL,
   "��", "j", "i",
   "����", "ch", "o",
   "����", "ch", "u",
   "����", "ch", "a",
   "����", "ch", "e",
   "��", "ch", "i",
   "��", "d", "a",
   "��", "t", "a",
   "��", "z", "o",
   "��", "s", "o",
   "��", "z", "e",
   "��", "s", "e",
   "����", "z", "i",
   "��", "z", "u",
   "����", "s", "i",
   "��", "s", "u",
   "����", "j", "o",
   "����", "j", "u",
   "����", "j", "a",
   "����", "j", "e",
   "��", "j", "i",
   "����", "sh", "o",
   "����", "sh", "u",
   "����", "sh", "a",
   "����", "sh", "e",
   "��", "sh", "i",
   "��", "z", "a",
   "��", "s", "a",
   "��", "g", "o",
   "��", "k", "o",
   "��", "g", "e",
   "��", "k", "e",
   "��", "k", "e",
   "����", "gw", "a",
   "��", "g", "u",
   "����", "kw", "a",
   "��", "k", "u",
   "����", "gy", "o",
   "����", "gy", "u",
   "����", "gy", "a",
   "����", "gy", "e",
   "��", "g", "i",
   "����", "ky", "o",
   "����", "ky", "u",
   "����", "ky", "a",
   "����", "ky", "e",
   "��", "k", "i",
   "��", "g", "a",
   "��", "k", "a",
   "��", "o", NULL,
   "��", "o", NULL,
   "��", "e", NULL,
   "��", "e", NULL,
   "����", "w", "o",
   "����", "w", "e",
   "����", "w", "i",
   "��", "u", NULL,
   "��", "u", NULL,
   "����", "y", "e",
   "��", "i", NULL,
   "��", "i", NULL,
   "��", "a", NULL,
   "��", "a", NULL,
   NULL, NULL, NULL
};

static const char *jpcommon_pos_list[] = {
   "����¾", "xx",
   "��ư��", "09",
   "����", "xx",
   "������", "19",
   "���ƻ�", "01",
   "����-����¾", "23",
   "����-�ʽ���", "13",
   "����-������", "24",
   "����-������", "14",
   "����-��³����", "12",
   "����-������", "11",
   "��ư��", "10",
   "��³��", "08",
   "��Ƭ��", "16",
   "��Ƭ��-������Ū", "16",
   "��Ƭ��-���ƻ�Ū", "16",
   "��Ƭ��-ư��Ū", "16",
   "��Ƭ��-̾��Ū", "16",
   "������-������Ū", "15",
   "������-���ƻ�Ū", "15",
   "������-ư��Ū", "15",
   "������-̾��Ū", "15",
   "��̾��", "04",
   "ư��", "20",
   "ư��-��Ω", "17",
   "����", "06",
   "̾��-������³", "03",
   "̾��-��ͭ̾��", "18",
   "̾��-����", "05",
   "̾��-��Ω", "22",
   "̾��-����̾��", "02",
   "Ϣ�λ�", "07",
   "�ե��顼", "25",
   NULL, NULL
};


static const char *jpcommon_cform_list[] = {
   "*", "xx",
   "����¾", "6",
   "�����", "4",
   "���ܷ�", "2",
   "̤����", "0",
   "̿���", "5",
   "Ϣ�η�", "3",
   "Ϣ�ѷ�", "1",
   NULL, NULL
};

static const char *jpcommon_ctype_list[] = {
   "*", "xx",
   "�����ѳ�", "5",
   "�����ѳ�", "4",
   "����ѳ�", "6",
   "����", "3",
   "���ƻ�", "7",
   "����", "1",
   "����", "6",
   "��ư��", "7",
   "����", "6",
   "���Ѳ�", "6",
   "ʸ���ư��", "6",
   NULL, NULL
};

JPCOMMON_RULE_H_END;

#endif                          /* !JPCOMMON_RULE_H */

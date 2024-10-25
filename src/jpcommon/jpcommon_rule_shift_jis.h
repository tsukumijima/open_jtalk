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

#define JPCOMMON_MORA_UNVOICE "�f"
#define JPCOMMON_MORA_LONG_VOWEL "�["
#define JPCOMMON_MORA_SHORT_PAUSE "�A"
#define JPCOMMON_MORA_QUESTION "�H"
#define JPCOMMON_MORA_EXCLAMATION "�J"
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
   "�N�@", "kw", "a",
   "�N�B", "kw", "i",
   "�N�D", "kw", "u",
   "�N�F", "kw", "e",
   "�N�H", "kw", "o",
   "�N��", "kw", "a",
   "�O�@", "gw", "a",
   "�O�B", "gw", "i",
   "�O�D", "gw", "u",
   "�O�F", "gw", "e",
   "�O�H", "gw", "o",
   "�O��", "gw", "a",
   "�f�F", "dy", "e",
   "�V�B", "s", "i",
   "�t��", "fy", "u",
   "��", "w", "a",
   "����", "by", "o",
   "����", "by", "u",
   "����", "by", "a",
   "���H", "v", "o",
   "���F", "v", "e",
   "���B", "v", "i",
   "���@", "v", "a",
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
   "����", "ry", "o",
   "����", "ry", "u",
   "����", "ry", "a",
   "���F", "ry", "e",
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
   "�~��", "my", "o",
   "�~��", "my", "u",
   "�~��", "my", "a",
   "�~�F", "my", "e",
   "�~", "m", "i",
   "�}", "m", "a",
   "�|", "p", "o",
   "�{", "b", "o",
   "�z", "h", "o",
   "�y", "p", "e",
   "�x", "b", "e",
   "�w", "h", "e",
   "�v", "p", "u",
   "�u", "b", "u",
   "�t�H", "f", "o",
   "�t�F", "f", "e",
   "�t�B", "f", "i",
   "�t�@", "f", "a",
   "�t", "f", "u",
   "�s��", "py", "o",
   "�s��", "py", "u",
   "�s��", "py", "a",
   "�s�F", "py", "e",
   "�s", "p", "i",
   "�r��", "by", "o",
   "�r��", "by", "u",
   "�r��", "by", "a",
   "�r�F", "by", "e",
   "�r", "b", "i",
   "�q��", "hy", "o",
   "�q��", "hy", "u",
   "�q��", "hy", "a",
   "�q�F", "hy", "e",
   "�q", "h", "i",
   "�p", "p", "a",
   "�o", "b", "a",
   "�n", "h", "a",
   "�m", "n", "o",
   "�l", "n", "e",
   "�k", "n", "u",
   "�j��", "ny", "o",
   "�j��", "ny", "u",
   "�j��", "ny", "a",
   "�j�F", "ny", "e",
   "�j", "n", "i",
   "�i", "n", "a",
   "�h�D", "d", "u",
   "�h", "d", "o",
   "�g�D", "t", "u",
   "�g", "t", "o",
   "�f��", "dy", "o",
   "�f��", "dy", "u",
   "�f��", "dy", "a",
   "�f�B", "d", "i",
   "�f", "d", "e",
   "�e��", "ty", "o",
   "�e��", "ty", "u",
   "�e��", "ty", "a",
   "�e�B", "t", "i",
   "�e", "t", "e",
   "�d", "z", "u",
   "�c�H", "ts", "o",
   "�c�F", "ts", "e",
   "�c�B", "ts", "i",
   "�c�@", "ts", "a",
   "�c", "ts", "u",
   "�b", "cl", NULL,
   "�a", "j", "i",
   "�`��", "ch", "o",
   "�`��", "ch", "u",
   "�`��", "ch", "a",
   "�`�F", "ch", "e",
   "�`", "ch", "i",
   "�_", "d", "a",
   "�^", "t", "a",
   "�]", "z", "o",
   "�\", "s", "o",
   "�[", "z", "e",
   "�Z", "s", "e",
   "�Y�B", "z", "i",
   "�Y", "z", "u",
   "�X�B", "s", "i",
   "�X", "s", "u",
   "�W��", "j", "o",
   "�W��", "j", "u",
   "�W��", "j", "a",
   "�W�F", "j", "e",
   "�W", "j", "i",
   "�V��", "sh", "o",
   "�V��", "sh", "u",
   "�V��", "sh", "a",
   "�V�F", "sh", "e",
   "�V", "sh", "i",
   "�U", "z", "a",
   "�T", "s", "a",
   "�S", "g", "o",
   "�R", "k", "o",
   "�Q", "g", "e",
   "�P", "k", "e",
   "��", "k", "e",
   "�O��", "gw", "a",
   "�O", "g", "u",
   "�N��", "kw", "a",
   "�N", "k", "u",
   "�M��", "gy", "o",
   "�M��", "gy", "u",
   "�M��", "gy", "a",
   "�M�F", "gy", "e",
   "�M", "g", "i",
   "�L��", "ky", "o",
   "�L��", "ky", "u",
   "�L��", "ky", "a",
   "�L�F", "ky", "e",
   "�L", "k", "i",
   "�K", "g", "a",
   "�J", "k", "a",
   "�I", "o", NULL,
   "�H", "o", NULL,
   "�G", "e", NULL,
   "�F", "e", NULL,
   "�E�H", "w", "o",
   "�E�F", "w", "e",
   "�E�B", "w", "i",
   "�E", "u", NULL,
   "�D", "u", NULL,
   "�C�F", "y", "e",
   "�C", "i", NULL,
   "�B", "i", NULL,
   "�A", "a", NULL,
   "�@", "a", NULL,
   NULL, NULL, NULL
};

static const char *jpcommon_pos_list[] = {
   "���̑�", "xx",
   "������", "09",
   "�L��", "xx",
   "�`��", "19",
   "�`�e��", "01",
   "����-���̑�", "23",
   "����-�i����", "13",
   "����-�W����", "24",
   "����-�I����", "14",
   "����-�ڑ�����", "12",
   "����-������", "11",
   "������", "10",
   "�ڑ���", "08",
   "�ړ���", "16",
   "�ړ���-�`�󎌓I", "16",
   "�ړ���-�`�e���I", "16",
   "�ړ���-�����I", "16",
   "�ړ���-�����I", "16",
   "�ڔ���-�`�󎌓I", "15",
   "�ڔ���-�`�e���I", "15",
   "�ڔ���-�����I", "15",
   "�ڔ���-�����I", "15",
   "�㖼��", "04",
   "����", "20",
   "����-�񎩗�", "17",
   "����", "06",
   "����-�T�ϐڑ�", "03",
   "����-�ŗL����", "18",
   "����-����", "05",
   "����-�񎩗�", "22",
   "����-���ʖ���", "02",
   "�A�̎�", "07",
   "�t�B���[", "25",
   NULL, NULL
};


static const char *jpcommon_cform_list[] = {
   "*", "xx",
   "���̑�", "6",
   "����`", "4",
   "��{�`", "2",
   "���R�`", "0",
   "���ߌ`", "5",
   "�A�̌`", "3",
   "�A�p�`", "1",
   NULL, NULL
};

static const char *jpcommon_ctype_list[] = {
   "*", "xx",
   "�J�s�ϊi", "5",
   "�T�s�ϊi", "4",
   "���s�ϊi", "6",
   "��i", "3",
   "�`�e��", "7",
   "�ܒi", "1",
   "�l�i", "6",
   "������", "7",
   "��i", "6",
   "�s�ω�", "6",
   "���ꏕ����", "6",
   NULL, NULL
};

JPCOMMON_RULE_H_END;

#endif                          /* !JPCOMMON_RULE_H */

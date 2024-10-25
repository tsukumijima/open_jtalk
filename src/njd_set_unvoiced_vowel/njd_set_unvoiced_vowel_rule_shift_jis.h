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
  �����q��: k ky s sh t ty ch ts h f hy p py
  Rule 0 �t�B���[�͖��������Ȃ�
  Rule 1 �������́u�ł��v�Ɓu�܂��v�́u���v��������
  Rule 2 �����C�������C�����́u���v�͖��������₷��
  Rule 3 �����Ė��������Ȃ�
  Rule 4 �A�N�Z���g�j�Ŗ��������Ȃ�
  Rule 5 �����q��(k ky s sh t ty ch ts h f hy p py)�Ɉ͂܂ꂽ�ui�v�Ɓuu�v��������
         ��O�Fs->s, s->sh, f->f, f->h, f->hy, h->f, h->h, h->hy
*/

#define NJD_SET_UNVOICED_VOWEL_FILLER "�t�B���["
#define NJD_SET_UNVOICED_VOWEL_DOUSHI "����"
#define NJD_SET_UNVOICED_VOWEL_JODOUSHI "������"
#define NJD_SET_UNVOICED_VOWEL_JOSHI "����"
#define NJD_SET_UNVOICED_VOWEL_KANDOUSHI "������"
#define NJD_SET_UNVOICED_VOWEL_TOUTEN "�A"
#define NJD_SET_UNVOICED_VOWEL_QUESTION "�H"
#define NJD_SET_UNVOICED_VOWEL_EXCLAMATION "�J" // added by me
#define NJD_SET_UNVOICED_VOWEL_QUOTATION "�f"
#define NJD_SET_UNVOICED_VOWEL_SHI "�V"
#define NJD_SET_UNVOICED_VOWEL_MA "�}"
#define NJD_SET_UNVOICED_VOWEL_DE "�f"
#define NJD_SET_UNVOICED_VOWEL_CHOUON "�["
#define NJD_SET_UNVOICED_VOWEL_SU "�X"

static const char *njd_set_unvoiced_vowel_candidate_list1[] = {
   "�V�B",                    /* s i */
   "�X�B",                    /* s i */
   "�X",                      /* s u */
   NULL
};

static const char *njd_set_unvoiced_vowel_next_mora_list1[] = {
   "�J",                       /* k ky */
   "�L",
   "�N",
   "�P",
   "�R",
   "�^",                       /* t ty ch ts */
   "�`",
   "�c",
   "�e",
   "�g",
   "�n",                       /* h f hy */
   "�q",
   "�t",
   "�w",
   "�z",
   "�p",                       /* p py */
   "�s",
   "�v",
   "�y",
   "�|",
   NULL
};

static const char *njd_set_unvoiced_vowel_candidate_list2[] = {
   "�t�B",                     /* f i */
   "�q",                       /* h i */
   "�t",                       /* f u */
   NULL
};

static const char *njd_set_unvoiced_vowel_next_mora_list2[] = {
   "�J",                       /* k ky */
   "�L",
   "�N",
   "�P",
   "�R",
   "�T",                       /* s sh */
   "�V",
   "�X",
   "�Z",
   "�\",
   "�^",                       /* t ty ch ts */
   "�`",
   "�c",
   "�e",
   "�g",
   "�p",                       /* p py */
   "�s",
   "�v",
   "�y",
   "�|",
   NULL
};

static const char *njd_set_unvoiced_vowel_candidate_list3[] = {
   "�L��",                    /* ky u */
   "�V��",                    /* sh u */
   "�`��",                    /* ch u */
   "�c�B",                    /* ts i */
   "�q��",                    /* hy u */
   "�s��",                    /* py u */
   "�e��",                    /* ty u */
   "�g�D",                    /* t u */
   "�e�B",                    /* t i */
   "�L",                       /* k i */
   "�N",                       /* k u */
   "�V",                       /* sh i */
   "�`",                       /* ch i */
   "�c",                       /* ts u */
   "�s",                       /* p i */
   "�v",                       /* p u */
   NULL
};

static const char *njd_set_unvoiced_vowel_next_mora_list3[] = {
   "�J",                       /* k ky */
   "�L",
   "�N",
   "�P",
   "�R",
   "�T",                       /* s sh */
   "�V",
   "�X",
   "�Z",
   "�\",
   "�^",                       /* t ty ch ts */
   "�`",
   "�c",
   "�e",
   "�g",
   "�n",                       /* h f hy */
   "�q",
   "�t",
   "�w",
   "�z",
   "�p",                       /* p py */
   "�s",
   "�v",
   "�y",
   "�|",
   NULL
};

static const char *njd_set_unvoiced_vowel_mora_list[] = {
   "�N�@",
   "�N�B",
   "�N�D",
   "�N�F",
   "�N�H", 
   "�O�@", 
   "�O�B",
   "�O�D", 
   "�O�F", 
   "�O�H", 
   "�N��",
   "�O��",
   "��",
   "�f�F",
   "�V�B",
   "�t�B",
   "����",
   "����",
   "����",
   "���H",
   "���F",
   "���B",
   "���@",
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
   "���F",
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
   "�~��",
   "�~��",
   "�~��",
   "�~�F",
   "�~",
   "�}",
   "�|",
   "�{",
   "�z",
   "�y",
   "�x",
   "�w",
   "�v",
   "�u",
   "�t�H",
   "�t�F",
   "�t�B",
   "�t�@",
   "�t",
   "�s��",
   "�s��",
   "�s��",
   "�s�F",
   "�s",
   "�r��",
   "�r��",
   "�r��",
   "�r�F",
   "�r",
   "�q��",
   "�q��",
   "�q��",
   "�q�F",
   "�q",
   "�p",
   "�o",
   "�n",
   "�m",
   "�l",
   "�k",
   "�j��",
   "�j��",
   "�j��",
   "�j�F",
   "�j",
   "�i",
   "�h�D",
   "�h",
   "�g�D",
   "�g",
   "�f��",
   "�f��",
   "�f��",
   "�f�B",
   "�f",
   "�e��",
   "�e��",
   "�e��",
   "�e�B",
   "�e",
   "�d",
   "�c�H",
   "�c�F",
   "�c�B",
   "�c�@",
   "�c",
   "�b",
   "�a",
   "�`��",
   "�`��",
   "�`��",
   "�`�F",
   "�`",
   "�_",
   "�^",
   "�]",
   "�\",
   "�[",
   "�Z",
   "�Y�B",
   "�Y",
   "�X�B",
   "�X",
   "�W��",
   "�W��",
   "�W��",
   "�W�F",
   "�W",
   "�V��",
   "�V��",
   "�V��",
   "�V�F",
   "�V",
   "�U",
   "�T",
   "�S",
   "�R",
   "�Q",
   "�P",
   "�O",
   "�N",
   "�M��",
   "�M��",
   "�M��",
   "�M�F",
   "�M",
   "�L��",
   "�L��",
   "�L��",
   "�L�F",
   "�L",
   "�K",
   "�J",
   "�I",
   "�H",
   "�G",
   "�F",
   "�E�H",
   "�E�F",
   "�E�B",
   "�E",
   "�D",
   "�C�F",
   "�C",
   "�B",
   "�A",
   "�@",
   "�[",
   NULL
};

NJD_SET_UNVOICED_VOWEL_RULE_H_END;

#endif                          /* !NJD_SET_UNVOICED_VOWEL_RULE_H */

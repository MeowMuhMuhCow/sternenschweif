/* SEG004 - Klang-Objekte (seg004 in SCHWEIF.EXE): digitale Klaenge (Miles-AIL-DSP-Treiber, Doppelpuffer
 * g_snd_buf0/g_snd_buf1 zu je 0x2800 Bytes, Nachladen im Hintergrund) und Musik (XMIDI-Treiber, 8 Sequenzen).
 * Objekt struct sound (0x89 Bytes): g_snd_music, g_snd_fx, g_snd_speech. Klangnummer id: Bit 0x8000 =
 * Sprachausgabe (Datei fileB bzw. bei id > 1000 g_file_speech_ext), Bit 0x4000 = im Archiv, sonst Datei fileA
 * bzw. (src == SND_SRC_ARCHIVE) immer Archiv. snd_service: digitale Puffer nachladen (snd_queue), bei Ende
 * snd_stop; Musik: Lautstaerke nachfuehren, beendete Sequenzen stoppen; ohne Treiber CD-Musik (g_cd_music).
 * snd_play/snd_select/snd_start/snd_load: Klang laden und starten (Lautstaerke g_vol_fx bzw. g_vol_speech,
 * Maus waehrend des Abspielens aus bei g_snd_hide_mouse). snd_speech: Sprachausgabe (g_speech_on), wartet bis
 * Klangeffekte fertig sind.
 * BCC -mlarge -3 -Ot -c -Y SEG004.C (ohne -O)   -> 3132/3132 Bytes GLEICH
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH)
 * - Objektparameter wie in s_proto.h als void far * (Aufrufer uebergeben g_snd_*), Zugriff ueber den Cast-Makro
 *   SND (kein eigener Zeiger - gleicher Code). AIL-Treiberfunktionen R22 nach der AIL-2-Schnittstelle benannt.
 */
#include "schweif.h"

#define SND ((struct sound far *)p)

/* snd_pmode (smsw ax / and ax,1) steht im Original am Ende von seg003 (Segmenttabelle: seg004 beginnt erst danach) */

signed short snd_seq_state(void far *p, signed short i)
{
	if (!SND->disabled) {
		if (SND->desc->type == AIL_XMIDI_DRVR)
			return AIL_sequence_status(SND->drv, SND->seq[i]);
		else if (SND->desc->type == AIL_DSP_DRVR)
			return AIL_sound_buffer_status(SND->drv, SND->buf);
	}
	return 0;
}

unsigned char huge *snd_cur_buf(void far *p)
{
	return !SND->buf ? g_snd_buf0 : g_snd_buf1;
}

void snd_queue(void far *p, unsigned char huge *data, unsigned long len)
{
	struct snd_buf far *sb;
	unsigned long rem;

	sb = &SND->sbuf2;
	sb->data = data;
	sb->len = (rem = SND->remain) > len ? len : rem;
	AIL_format_sound_buffer(SND->drv, sb);
	AIL_register_sound_buffer(SND->drv, SND->buf, sb);
	SND->remain -= sb->len;
	SND->pos += SND_BUF_SIZE;
	SND->buf ^= 1;
}

void snd_service(void far *p)
{
	signed short st;
	signed short nr;

	if (!SND->disabled) {
		if (SND->desc->type == AIL_DSP_DRVR) {
			if (g_snd_mouse_hidden == 1 && g_snd_hide_mouse == 1 && !SND->playing) {
				g_snd_mouse_hidden = 0;
				mouse_show(0);
			}
			if (SND->playing == 1) {
				st = AIL_sound_buffer_status(SND->drv, SND->buf);
				if (st == DAC_DONE || st == DAC_STOPPED) {
					if (SND->remain <= 0) {
						st = AIL_sound_buffer_status(SND->drv, SND->buf ^ 1);
						if (st == DAC_DONE || st == DAC_STOPPED)
							snd_stop(p, 0);
					} else {
						if (SND->src == SND_SRC_FILES) {
							if (SND->id & SND_ID_SPEECH) {
								if ((nr = SND->id & 0x7fff) > 1000) {
									file_open(g_file_speech_ext, nr - 1000);
									file_seek(g_file_speech_ext, SND->pos, 0);
									file_read(g_file_speech_ext, snd_cur_buf(p), SND_BUF_SIZE);
									file_close(g_file_speech_ext);
								} else {
									file_open(SND->file_speech, nr);
									file_seek(SND->file_speech, SND->pos, 0);
									file_read(SND->file_speech, snd_cur_buf(p), SND_BUF_SIZE);
									file_close(SND->file_speech);
								}
							} else if (SND->id & SND_ID_ARCHIVE) {
								arc_open(SND->arch, SND->id & 0x3fff);
								arc_seek(SND->arch, SND->pos, 0);
								arc_read(SND->arch, snd_cur_buf(p), SND_BUF_SIZE);
								arc_close(SND->arch);
							} else {
								file_open(SND->file, SND->id);
								file_seek(SND->file, SND->pos, 0);
								file_read(SND->file, snd_cur_buf(p), SND_BUF_SIZE);
								file_close(SND->file);
							}
						} else {
							arc_open(SND->arch, SND->id);
							arc_seek(SND->arch, SND->pos, 0);
							arc_read(SND->arch, snd_cur_buf(p), SND_BUF_SIZE);
							arc_close(SND->arch);
						}
						snd_queue(p, snd_cur_buf(p), SND_BUF_SIZE);
						st = AIL_sound_buffer_status(SND->drv, SND->buf);
						if (st == DAC_DONE || st == DAC_STOPPED) {
							st = AIL_sound_buffer_status(SND->drv, SND->buf ^ 1);
							if (st == DAC_DONE || st == DAC_STOPPED)
								AIL_start_digital_playback(SND->drv);
						}
					}
				}
			}
		} else if (SND->desc->type == AIL_XMIDI_DRVR) {
			if (g_snd_vol_req == 0x7f)
				Sound_volume2(p, g_vol_music, 0);
			for (st = 0; st < 8; st++) {
				if (AIL_relative_volume(SND->drv, SND->seq[st]) == 0 && snd_seq_state(p, st) == SEQ_PLAYING)
					snd_stop(p, st);
			}
		}
		return;
	}
	if (p == g_snd_music && g_cd_music == 1)
		CD_update(g_cd);
}

void snd_set_vol(signed short a, signed short b)
{
	g_snd_vol_req = b;
	g_snd_vol_req2 = a;
}

signed short snd_state(void far *p, signed short i)
{
	if (!SND->disabled) {
		if (SND->desc->type == AIL_XMIDI_DRVR)
			return AIL_sequence_status(SND->drv, SND->seq[i]);
		else if (SND->desc->type == AIL_DSP_DRVR)
			return SND->playing;
	}
	return 0;
}

signed short snd_busy(void far *p, signed short i)
{
	if (SND->enabled == 1) {
		return snd_state(p, i);
		return;		/* toter Ruecksprung, im Original vorhanden */
	}
	return 0;
}

void snd_speech(void far *p, signed short id_arg)
{
	register signed short id = id_arg;
	signed short ok = 0;

	if (g_speech_on && SND->drv == 1) {
		if (id > 1000) {
			if (g_speech_ext_avail)
				ok = File_exists(g_file_speech_ext, id - 1000);
		} else
			ok = File_exists(g_file_speech, id);
		if (ok == 1) {
			while (snd_speech_busy(p) == 1 || snd_busy(g_snd_fx, 0) == 1) {
				snd_service(g_snd_music);
				snd_service(g_snd_fx);
				snd_speech_service(g_snd_speech);
				wait_for_vsync();
			}
			snd_select(g_snd_fx, id | SND_ID_SPEECH);
		}
	}
}

signed short snd_speech_busy(void far *p)
{
	if (g_speech_on && SND->drv == 1 && (((struct sound far *)g_snd_fx)->id & SND_ID_SPEECH)) {
		return snd_state(g_snd_fx, 0);
		return;		/* toter Ruecksprung, im Original vorhanden */
	}
	return 0;
}

void snd_speech_service(void far *p)
{
	if (g_speech_on && SND->drv == 1)
		snd_service(g_snd_fx);
}

void snd_play2(void far *p, signed short id)
{
	snd_play(p, id);
}

void snd_select(void far *p, signed short id_arg)
{
	register signed short id = id_arg;

	SND->id_req = id;
	if (!SND->disabled && snd_busy(p, 0) == 0) {
		SND->id = id;
		snd_start(p);
		snd_service(p);
	}
}

void snd_play(void far *p, signed short id_arg)
{
	register signed short id = id_arg;

	SND->id_req = id;
	if (SND->enabled == 1) {
		if (p == g_snd_music && g_cd_music == 1) {
			CD_music(g_cd, id);
			return;
		}
		if (File_exists(SND->file, id) == 1) {
			if (SND->desc->type == AIL_DSP_DRVR) {
				snd_select(p, id);
				return;
			}
			if (SND->desc->type == AIL_XMIDI_DRVR) {
				SND->id = id;
				Sound_play(p, id);
			}
		}
	}
}

signed short snd_exists(void far *p, signed short id_arg)
{
	register signed short id = id_arg;

	if (SND->src == SND_SRC_FILES && file_load(SND->file, id, SND->load_buf))
		return 1;
	else if (SND->src == SND_SRC_ARCHIVE && arc_load(SND->arch, id, SND->load_buf))
		return 1;
	return 0;
}

struct snd_buf far *snd_load(void far *p, signed short id_arg)
{
	register signed short id = id_arg;
	signed short nr;

	if (!SND->disabled && SND->desc->type == AIL_DSP_DRVR) {
		if (SND->src == SND_SRC_FILES) {
			if (id & SND_ID_SPEECH) {
				if ((nr = id & 0x7fff) > 1000) {
					file_open(g_file_speech_ext, nr - 1000);
					file_read(g_file_speech_ext, g_snd_buf0, SND_BUF_SIZE);
					file_close(g_file_speech_ext);
				} else {
					file_open(SND->file_speech, nr);
					file_read(SND->file_speech, g_snd_buf0, SND_BUF_SIZE);
					file_close(SND->file_speech);
				}
			} else if (id & SND_ID_ARCHIVE) {
				arc_open(SND->arch, id & 0x3fff);
				arc_read(SND->arch, g_snd_buf0, SND_BUF_SIZE);
				arc_close(SND->arch);
			} else {
				file_open(SND->file, id);
				file_read(SND->file, g_snd_buf0, SND_BUF_SIZE);
				file_close(SND->file);
			}
		} else {
			arc_open(SND->arch, id);
			arc_read(SND->arch, g_snd_buf0, SND_BUF_SIZE);
			arc_close(SND->arch);
		}
		AIL_index_VOC_block(SND->drv, g_snd_buf0, -1, &SND->sbuf);
		SND->sbuf2 = SND->sbuf;
		SND->buf = 0;
		return &SND->sbuf;
	}
	return 0;
}

void snd_start(void far *p)
{
	unsigned long skip;		/* Kopflaenge der VOC-Datei im ersten Puffer */

	if ((SND->remain = snd_load(p, SND->id)->len) > 0) {
		skip = (unsigned short)(SND->sbuf.data - g_snd_buf0);
		snd_queue(p, SND->sbuf.data, SND_BUF_SIZE - skip);
		if (SND->id & SND_ID_SPEECH)
			AIL_set_digital_playback_volume(SND->drv, g_vol_speech);
		else
			AIL_set_digital_playback_volume(SND->drv, g_vol_fx);
		if (!g_snd_mouse_hidden && g_snd_hide_mouse) {
			g_snd_mouse_hidden = 1;
			mouse_hide(0);
		}
		AIL_start_digital_playback(SND->drv);
		SND->playing = 1;
	}
}

void snd_stop(void far *p, signed short i_arg)
{
	register signed short i = i_arg;

	if (!SND->disabled) {
		if (SND->desc->type == AIL_XMIDI_DRVR) {
			if (SND->seq[i] != -1) {
				AIL_stop_sequence(SND->drv, SND->seq[i]);
				while (AIL_sequence_status(SND->drv, SND->seq[i]) == SEQ_PLAYING)
					wait_for_vsync();
				AIL_release_sequence_handle(SND->drv, SND->seq[i]);
				SND->seq[i] = -1;
			}
			Sound_nocallback(p);
			return;
		}
		if (SND->desc->type == AIL_DSP_DRVR) {
			SND->playing = 0;
			SND->pos = 0;
		}
	}
}

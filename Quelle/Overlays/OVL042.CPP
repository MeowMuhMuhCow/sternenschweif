/* OVL042 - Overlay O042 (Segment 0x1fed in SCHWEIF.EXE): Klangobjekt Sound (s_class.h, 0x89 Bytes = struct sound,
 * wie SEG004.C) auf dem Miles-Treiber AIL (R22). Vier Konstruktoren (Klaenge aus Dateiobjekten file/file_speech bzw.
 * Archiv arch; Zaehler g_ail_timer_users -> AIL_startup beim ersten Objekt, Sequenzen -1), Destruktor (letztes
 * Objekt -> AIL_shutdown, Puffer seq_state[8]/timbre_cache/load_buf und die Doppelpuffer g_snd_buf0/1 frei).
 * play(n): Musikstueck n laden (music, Klangfarben timbres, start) bzw. weiterspielen, Rueckruf snd_set_vol setzen
 * (callback). load(name): Treiberdatei in einen 16-Byte-ausgerichteten Puffer (EMS_norm_ptr). timbres: je
 * Sequenz AIL_register_sequence, fehlende Klangfarben (bank/patch, AIL_timbre_request) aus der Bibliothek
 * TIMBRE_LIB_MT (MT-32) bzw. TIMBRE_LIB per timbre (Eintraege g_timbre_entry, Laenge g_timbre_size) nachladen.
 * init(name, typ, io, irq): Treiber laden, Beschreibung desc, "Forte UltraSound" -> g_snd_hide_mouse, Typ 3:
 * Zustands-/Klangfarbenpuffer, Typ 2: Doppelpuffer 0x2800. drvname: n-ten Geraetenamen des Treibers nach dst.
 * resume/stop1/stop/pause, volume (begrenzt auf g_vol_music, 0 -> stop; CD-Musik ueber CD_stop/CD_volume),
 * volume2/volume3, wait, mtype (Endung "AD"/"OPL"/"MT" -> SND_MTYPE_*), nocallback. Danach Speech (Wort +0 = an,
 * g_snd_speech): stop/volume/wait fuer g_snd_fx (nur bei g_speech_on), set, active (!g_speech_off && an), get_pos.
 *
 * BCC -mlarge -3 -Ot -c -Y OVL042.CPP   -> 4071/4071 Bytes GLEICH (18.10.2026), cobjfix laeuft (mit -Og ebenso)
 *   (Namensfassung mit schweif.h, Welle N20: 19.10.2026 GLEICH, LINKGLEICH)
 * - Keine Intrinsics (strcmp/strcpy als Aufrufe) -> ohne -Oi.
 * - Zwei Konstruktoren mit je zwei far-Zeigern: Typen File/struct arc unterscheiden (Overload), Reihenfolge der
 *   Zuweisungen folgt dem Code, nicht der Parameterliste.
 * - start/volume: Parameter als register (SI), Hilfsvariable ohne register (DI); umgekehrt vertauscht BCC.
 * - music endet mit "return snd_exists(..)" (int), sonst fehlt der tote Epilog.
 * - load: EMS_norm_ptr bekommt die ausgerichtete Adresse (unsigned long) als far-Zeiger -> zwei push word statt
 *   push dword; erst dann wird der Sprung um den if-Rumpf near (jnz/jmp).
 * - resume/pause: "return;" im ersten Zweig -> Epilogkopie auch ohne -Og.
 * - g_snd_buf0 ist huge: Nulltest ueber (void far *) (cmp dword), sonst mov/or (huge-Vergleich).
 * - Rueckruf snd_set_vol als Funktionszeiger (push seg/push offset).
 * - active(): inline cd_off() { return !g_speech_off; } -> neg/sbb/inc, dann mov ax,1/xor ax,ax.
 * - O042_0C19 ruft AIL_resume_sequence bzw. AIL_resume_digital_playback (resume), O042_0D18 AIL_stop_sequence bzw.
 *   AIL_pause_digital_playback (pause) - die erste Rekonstruktion hatte die beiden Namen vertauscht.
 * Auffaelliges: der Destruktor JEDES Sound-Objekts gibt die gemeinsamen Doppelpuffer g_snd_buf0/1 frei; der
 *   Treiberpuffer drv_buf (aus load) wird nie freigegeben; drvname zaehlt ohne Grenze durch die Namensliste.
 */
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <dos.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:4244..4272, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */
/* Phase B: eigenes BSS (ds:c7de..c7ec) in Adressreihenfolge */
signed short g_snd_mouse_hidden;
signed short g_snd_hide_mouse;
unsigned short g_timbre_size;
struct timbre_entry g_timbre_entry;
signed short g_ems_free_pages;

#define TIMBRE_LIB	0x14		/* Klangfarbenbibliothek (General/OPL) im Hauptarchiv */
#define TIMBRE_LIB_MT	0x46		/* Klangfarbenbibliothek fuer MT-32 */
#define SND_FX		((Sound far *)g_snd_fx)

Sound::Sound(File far *f, struct arc far *ar, File far *fs)
{
	signed short i;

	arch = ar;
	file = f;
	file_speech = fs;
	src = SND_SRC_FILES;
	if (!g_ail_timer_users++)
		AIL_startup();
	enabled = 1;
	disabled = 1;
	music_nr = -1;
	for (i = 0; i < 8; i++)
		seq[i] = -1;
	id_req = -1;
}

Sound::Sound(File far *f, File far *fs)
{
	signed short i;

	file = f;
	file_speech = fs;
	src = SND_SRC_FILES;
	if (!g_ail_timer_users++)
		AIL_startup();
	enabled = 1;
	disabled = 1;
	music_nr = -1;
	for (i = 0; i < 8; i++)
		seq[i] = -1;
	id_req = -1;
}

Sound::Sound(struct arc far *ar)
{
	signed short i;

	file = (File far *)g_file_main;
	arch = ar;
	src = SND_SRC_ARCHIVE;
	if (!g_ail_timer_users++)
		AIL_startup();
	enabled = 1;
	disabled = 1;
	music_nr = -1;
	for (i = 0; i < 8; i++)
		seq[i] = -1;
	id_req = -1;
}

Sound::Sound(struct arc far *ar, File far *f)
{
	signed short i;

	file = f;
	arch = ar;
	src = SND_SRC_ARCHIVE;
	if (!g_ail_timer_users++)
		AIL_startup();
	enabled = 1;
	disabled = 1;
	music_nr = -1;
	for (i = 0; i < 8; i++)
		seq[i] = -1;
	id_req = -1;
}

Sound::~Sound()
{
	signed short i;

#if FIX_0068
	/* Fix #68: Treiberpuffer freigeben, sobald AIL beendet ist */
	if (!--g_ail_timer_users) {
		AIL_shutdown(0, 0);
		if (drv_buf != 0)
			drv_buf = mem_free(drv_buf);
	}
#else
	if (!--g_ail_timer_users)
		AIL_shutdown(0, 0);
#endif
	if (timbre_cache != 0)
		timbre_cache = mem_free(timbre_cache);
	for (i = 0; i < 8; i++)
		if (seq_state[i])
			seq_state[i] = mem_free(seq_state[i]);
	if (load_buf != 0)
		load_buf = mem_free(load_buf);
	if ((void far *)g_snd_buf0 != 0) {
		g_snd_buf0 = (unsigned char huge *)mem_free(g_snd_buf0);
		g_snd_buf1 = (unsigned char huge *)mem_free(g_snd_buf1);
	}
}

void Sound::play(register signed short n)
{
	if (!disabled) {
		if (music_nr == n) {
			if (snd_seq_state(this, 0) != SEQ_PLAYING) {
				stop();
				start(0);
			}
		} else {
			stop();
			id_req = n;
			music(music_nr = n);
			timbres();
			start(0);
		}
	}
	callback((void (far *)())snd_set_vol);
}

void far *Sound::load(char far *name)
{
	register signed short fd;
	signed long len;
	void far *p;
	unsigned long a;

	if ((fd = open(name, O_RDONLY | O_BINARY)) != -1) {
		len = filelength(fd);
		drv_buf = mem_alloc(len + 16);
		a = (unsigned long)drv_buf + 15;
		a &= ~0xfL;
		p = EMS_norm_ptr((void far *)a);
		read(fd, p, len);
		close(fd);
		return p;
	}
	return 0;
}

void Sound::unload()
{
	if (drv_buf != 0)
		drv_buf = mem_free(drv_buf);
}

signed short Sound::timbres()
{
	signed short i;
	signed short lib;
	unsigned short v;
	void far *p;
	register signed short bank;
	register signed short patch;

	lib = mtype() == SND_MTYPE_MT ? TIMBRE_LIB_MT : TIMBRE_LIB;
	if (file_open(g_file_main, lib) != -1) {
		for (i = 0; i < 8; i++) {
			if ((seq[i] = AIL_register_sequence(drv, load_buf, i, seq_state[i], 0)) != -1) {
				if (mtype() != SND_MTYPE_OTHER) {
					while ((v = AIL_timbre_request(drv, seq[i])) != 0xffff) {
						bank = v >> 8;
						patch = v & 0xff;
						if ((p = timbre(bank, patch)) != 0) {
							AIL_install_timbre(drv, bank, patch, p);
							p = mem_free(p);
						}
					}
				}
			}
		}
		file_close(g_file_main);
		return 1;
	}
	return 0;
}

signed short Sound::start(register signed short loop)
{
	signed short vol;

	if (!disabled && enabled == 1) {
		if (loop) {
			if ((vol = g_vol_music / 3) < 0)
				vol = 0;
			AIL_set_relative_volume(drv, seq[0], vol, 0);
		}
		AIL_start_sequence(drv, seq[loop]);
		AIL_set_relative_volume(drv, seq[loop], g_vol_music, 0);
		return 1;
	}
	return 0;
}

void far *Sound::timbre(register signed short bank, register signed short patch)
{
	unsigned short far *p;

	file_seek(g_file_main, 0L, 0);
	do {
		file_read(g_file_main, &g_timbre_entry, 6L);
		if (g_timbre_entry.bank == -1)
			return 0;
	} while (g_timbre_entry.bank != bank || g_timbre_entry.patch != patch);
	file_seek(g_file_main, g_timbre_entry.off, 0);
	file_read(g_file_main, &g_timbre_size, 2L);
	p = (unsigned short far *)mem_alloc(g_timbre_size);
	*p = g_timbre_size;
	file_read(g_file_main, p + 1, g_timbre_size - 2);
	return p;
}

signed short Sound::music(register signed short n)
{
	signed long len;

	if (load_buf != 0)
		load_buf = mem_free(load_buf);
	len = src == SND_SRC_FILES ? file_length(file, n) : arc_file_size(arch, n);
	load_buf = mem_alloc(len);
	return snd_exists(this, n);
}

void Sound::stop_music()
{
	if (desc->type == AIL_XMIDI_DRVR) {
		if (load_buf != 0)
			load_buf = mem_free(load_buf);
		id_req = id;
		music_nr = id = -1;
	}
}

signed short Sound::init(char far *name, signed short type, signed short io, signed short irq)
{
	register signed short i;

	if ((drv_image = load(name)) != 0) {
		if ((drv = AIL_register_driver(drv_image)) != -1) {
			desc = (struct ail_drv_desc far *)AIL_describe_driver(drv);
			if (!strcmp(desc->dev_names, "Forte UltraSound(TM) Digital Sound"))
				g_snd_hide_mouse = 1;
			if (desc->type == type) {
				if (io == -1)
					io = desc->io;
				if (irq == -1)
					irq = desc->irq;
				if (AIL_detect_device(drv, io, irq, desc->dma, desc->drq)) {
					AIL_init_driver(drv, io, irq, desc->dma, desc->drq);
					if (type == AIL_XMIDI_DRVR) {
						state_size = AIL_state_table_size(drv);
						for (i = 0; i < 8; i++)
							seq_state[i] = mem_alloc(state_size);
						cache_size = AIL_default_timbre_cache_size(drv);
						if (cache_size != 0) {
							timbre_cache = mem_alloc(cache_size);
							AIL_define_timbre_cache(drv, timbre_cache, cache_size);
						}
					} else if (type == AIL_DSP_DRVR && (void far *)g_snd_buf0 == 0) {
						g_snd_buf0 = (unsigned char huge *)mem_alloc(0x2800L);
						g_snd_buf1 = (unsigned char huge *)mem_alloc(0x2800L);
					}
					disabled = 0;
					volume2(g_vol_music, 0);
					return 1;
				}
			}
		}
	}
	disabled = 1;
	return 0;
}

char far *Sound::drvname(char far *name, signed short type, signed short idx, struct ail_drv_desc far *out,
	char far *dst)
{
	register signed short k = 0;
	char far *p = 0;

	if ((drv_image = load(name)) != 0) {
		if ((drv = AIL_register_driver(drv_image)) != -1) {
			desc = (struct ail_drv_desc far *)AIL_describe_driver(drv);
			*out = *desc;
			if (desc->type == type) {
				p = desc->dev_names;
#if FIX_0068
				/* Fix #68: Namensliste nur bis zu ihrem Ende durchsuchen */
				while (k != idx)
					if (!*p++) {
						k++;
						if (!*p)
							break;
					}
#else
				while (k != idx)
					if (!*p++)
						k++;
#endif
				strcpy(dst, p);
				if (!*p)
					p = 0;
			}
			AIL_release_driver_handle(drv);
		}
		unload();
	}
	return p;
}

void Sound::resume(signed short i)
{
	if (!disabled && enabled == 1) {
		if (desc->type == AIL_XMIDI_DRVR) {
			AIL_resume_sequence(drv, seq[i]);
			return;
		}
		if (desc->type == AIL_DSP_DRVR)
			AIL_resume_digital_playback(drv);
	}
}

void Sound::stop1(signed short i)
{
	if (enabled == 1)
		snd_stop(this, i);
}

void Sound::stop()
{
	register signed short i;

	if (enabled == 1) {
		if (this == (Sound far *)g_snd_music && g_cd_music == 1) {
			CD_stop(g_cd);
		} else if (desc->type == AIL_XMIDI_DRVR) {
			for (i = 0; i < 8; i++)
				snd_stop(this, i);
		} else {
			snd_stop(this, 0);
		}
		stop_music();
	}
}

void Sound::pause(signed short i)
{
	if (!disabled && enabled == 1) {
		if (desc->type == AIL_DSP_DRVR) {
			AIL_pause_digital_playback(drv);
			return;
		}
		if (desc->type == AIL_XMIDI_DRVR)
			AIL_stop_sequence(drv, seq[i]);
	}
}

void Sound::volume(register signed short vol, signed short b, signed short i)
{
	signed short m;

	if (!disabled) {
		if (desc->type == AIL_DSP_DRVR) {
			AIL_set_digital_playback_volume(drv, vol);
		} else if (desc->type == AIL_XMIDI_DRVR) {
			if ((m = g_vol_music) < vol)
				vol = m;
			AIL_set_relative_volume(drv, seq[i], vol, b);
		}
		if (!vol)
			stop();
		return;
	}
	if (this == (Sound far *)g_snd_music && g_cd_music == 1) {
		if ((m = g_vol_music) < vol)
			vol = m;
		CD_volume(g_cd, vol);
	}
}

void Sound::volume2(signed short vol, signed short i)
{
	if (enabled == 1)
		volume(vol, 0, i);
}

void Sound::volume3(signed short vol, signed short b, signed short i)
{
	if (enabled == 1)
		volume(vol, b, i);
}

void Sound::wait()
{
	while (playing == 1) {
		game_idle();
		wait_for_vsync();
	}
}

signed short Sound::mtype()
{
	char far *s = desc->suffix;
	signed short r;

	if (!strcmp(s, "AD"))
		r = SND_MTYPE_AD;
	else if (!strcmp(s, "OPL"))
		r = SND_MTYPE_OPL;
	else if (!strcmp(s, "MT"))
		r = SND_MTYPE_MT;
	else
		r = SND_MTYPE_OTHER;
	return r;
}

void Sound::callback(void (far *cb)())
{
	g_snd_vol_req = 0;
	AIL_install_callback(drv, cb);
}

void Sound::nocallback()
{
	AIL_cancel_callback(drv);
	g_snd_vol_req = 0;
}

void Speech::stop()
{
	if (g_speech_on != 0 && on == 1)
		SND_FX->stop();
}

void Speech::volume(signed short vol)
{
	if (g_speech_on != 0 && on == 1)
		SND_FX->volume(vol, 0, 0);
}

void Speech::set(signed short v)
{
	on = v;
}

inline signed short cd_off() { return !g_speech_off; }

signed short Speech::active()
{
	return cd_off() && on == 1;
}

void Speech::wait()
{
	if (g_speech_on != 0 && on == 1)
		SND_FX->wait();
}

signed short Speech::get_pos()
{
	return g_speech_pos & 0x3fff;
}

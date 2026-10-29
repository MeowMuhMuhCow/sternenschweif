;===========================================================================
; SEG022.ASM - Miles Audio Interface Library (AIL) 2.14, API-Modul
; Sternenschweif (DSA 2), Attic 1994 - Rekonstruktion, bytegleich mit
; SCHWEIF.EXE C1.02 (TASM 3.1 /ml)
;
; Zugekauft: "IBM Audio Interface Library -- Application Program Interface
; module" (AIL.ASM) von John Miles, Miles Design, Inc., (C) 1991, 1992.
; Version 2.14 vom 24.09.1992 (DP_handle in AIL_register_driver und das
; Zuruecksetzen von timer_value beim ersten Zeitgeber sind enthalten; die
; Konstante current_rev steht wie in der Quelle auf 211). Uebersetzt mit
; USE_INT8 = TRUE (Zeitgeber ueber int 8, nicht RTC), DIGPAK = TRUE,
; DIGPAK_SHARE = FALSE, MODEL MEDIUM, 8086-Code.
; Die Namen der Prozeduren, Daten und Konstanten folgen der von Miles
; veroeffentlichten Quelle (AIL.ASM, AIL.INC, AIL.MAC, AIL.H; AIL2-SDK, als
; freie Quelle u. a. unter github.com/Tronix286/AIL2). Kommentare deutsch.
;
; Inhalt:    1. Zeitgeberdienste: bis zu 16 Zeitgeber (+ Nr. 16 fuer den
;               alten BIOS-Zeitgeber) auf einem gemeinsamen int-8-Takt
;               (DDA: jeder Zeitgeber zaehlt Mikrosekunden bis timer_value).
;            2. Treiberverwaltung: bis zu 16 Treiber (.ADV); jede API-
;               Funktion ruft ueber find_proc/call_driver die gleichnamige
;               Funktion im Treiber (Funktionsnummern AIL_*).
;            3. DIGPAK-Schale (DP_*): ein DIGPAK-Treiber (.COM, int 66h)
;               wird wie ein AIL-DSP-Treiber angesprochen.
; Aufrufer:  SEG004 (Klang), O036/O042/O043 (Musik/Treiber), far
; Aufruf:    far, C-Konvention (cdecl), Parameter ab [bp+6]; alle
;            oeffentlichen Prozeduren sichern DS, SI, DI. Die Stubs der
;            Treiberfunktionen laden AX = Funktionsnummer und springen nach
;            call_driver, der die Parameter unveraendert an den Treiber gibt.
; Daten:     alle im Codesegment (AIL "Local data"), angesprochen ueber CS
;            mit Offsets zum Segmentrahmen R22. Der Rahmen beginnt 8 Bytes vor
;            diesem Modul, die Offsets stehen daher als Konstanten (equ) und
;            nicht als Marken (Marken ergaben Fixups, die das Original nicht
;            hat). Unmittelbare Code-Offsets ebenso (Endung _ofs).
;            Befehle mit [reg+Offset < 80h] stehen als db, weil das Original
;            die Verschiebung 16 Bit breit codiert (TASM waehlte 8 Bit).
; Herkunft:  aus SCHWEIF.EXE R22 (ab +0x8) mit asm2tasm.py zerlegt;
;            ;@nnnn = Offset im Segmentrahmen R22.
;===========================================================================
	.386p
	include	..\names\inc\s_fix.inc	; Schalter der Fehlerbehebungen (FIX_nnnn, tools/mkfix.py)
; ---------------------------------------------------------------------------
; Makros aus AIL.MAC (Miles)
; ---------------------------------------------------------------------------
; ZEROS - n Nullbytes (wie "db n dup (0)", aber als einzelne Bytes: dup ergaebe
; LIDATA-Saetze, die tools/objorder.py nicht verarbeitet)
ZEROS	macro	n
	rept	n
	db	0
	endm
	endm
; POP_F - popf ersetzen (popf-Fehler fruehrer 80286): "or bh,00" verdeckt ein
; iret, das per push cs/call angesprungen wird und die Flags vom Stack holt.
POP_F	macro
	local	@@intel_bytes
	db	80h
@@intel_bytes:
	iret
	db	00h
	push	cs
	call	@@intel_bytes
	endm
; PUSH_C - Konstante auf den Stack legen (so setzt TASM "push Konstante" im
; 8086-Modus um; AIL.ASM schreibt "call proc C,konstante")
PUSH_C	macro	val
	push	ax
	push	bp
	mov	bp,sp
	mov	word ptr [bp+2],val
	pop	bp
	endm
; ADD_PTR - 32-Bit-Wert zu einem far-Zeiger addieren, Ergebnis normalisiert
ADD_PTR	macro	add_l,add_h,pseg,poff
	push	bx
	push	cx
	mov	bx,pseg
	xor	cx,cx
	shl	bx,1
	rcl	cx,1
	shl	bx,1
	rcl	cx,1
	shl	bx,1
	rcl	cx,1
	shl	bx,1
	rcl	cx,1
	add	bx,poff
	adc	cx,0
	add	bx,add_l
	adc	cx,add_h
	mov	poff,bx
	and	poff,0fh
	shr	cx,1
	rcr	bx,1
	shr	cx,1
	rcr	bx,1
	shr	cx,1
	rcr	bx,1
	shr	cx,1
	rcr	bx,1
	mov	pseg,bx
	pop	cx
	pop	bx
	endm
; FAR_TO_HUGE - far-Zeiger normalisieren (Offset 0..15)
FAR_TO_HUGE	macro	fp_seg,fp_off
	push	ax
	push	bx
	mov	ax,fp_seg
	mov	bx,fp_off
	shr	bx,1
	shr	bx,1
	shr	bx,1
	shr	bx,1
	add	ax,bx
	mov	fp_seg,ax
	and	fp_off,0fh
	pop	bx
	pop	ax
	endm
; ---------------------------------------------------------------------------
; Funktionsnummern der Treiber (AIL.INC)
; ---------------------------------------------------------------------------
AIL_DESC_DRVR	equ	100
AIL_DET_DEV	equ	101
AIL_INIT_DRVR	equ	102
AIL_SERVE_DRVR	equ	103
AIL_SHUTDOWN_DRVR	equ	104
AIL_INDEX_VOC_BLK	equ	120
AIL_REG_SND_BUFF	equ	121
AIL_SND_BUFF_STAT	equ	122
AIL_P_VOC_FILE	equ	123
AIL_VOC_PB_STAT	equ	124
AIL_START_D_PB	equ	125
AIL_STOP_D_PB	equ	126
AIL_PAUSE_D_PB	equ	127
AIL_RESUME_D_PB	equ	128
AIL_SET_D_PB_VOL	equ	129
AIL_D_PB_VOL	equ	130
AIL_SET_D_PB_PAN	equ	131
AIL_D_PB_PAN	equ	132
AIL_F_VOC_FILE	equ	133
AIL_F_SND_BUFF	equ	134
AIL_STATE_TAB_SIZE	equ	150
AIL_REG_SEQ	equ	151
AIL_REL_SEQ_HND	equ	152
AIL_T_CACHE_SIZE	equ	153
AIL_DEFINE_T_CACHE	equ	154
AIL_T_REQ	equ	155
AIL_INSTALL_T	equ	156
AIL_PROTECT_T	equ	157
AIL_UNPROTECT_T	equ	158
AIL_T_STATUS	equ	159
AIL_START_SEQ	equ	170
AIL_STOP_SEQ	equ	171
AIL_RESUME_SEQ	equ	173
AIL_SEQ_STAT	equ	174
AIL_REL_VOL	equ	175
AIL_REL_TEMPO	equ	176
AIL_SET_REL_VOL	equ	177
AIL_SET_REL_TEMPO	equ	178
AIL_BEAT_CNT	equ	179
AIL_BAR_CNT	equ	180
AIL_BRA_INDEX	equ	181
AIL_CON_VAL	equ	182
AIL_SET_CON_VAL	equ	183
AIL_CHAN_NOTES	equ	185
AIL_SEND_CV_MSG	equ	186
AIL_SEND_SYSEX_MSG	equ	187
AIL_WRITE_DISP	equ	188
AIL_INSTALL_CB	equ	189
AIL_CANCEL_CB	equ	190
AIL_LOCK_CHAN	equ	191
AIL_MAP_SEQ_CHAN	equ	192
AIL_RELEASE_CHAN	equ	193
AIL_TRUE_SEQ_CHAN	equ	194
; Zustaende (AIL.INC)
DAC_STOPPED	equ	0
DAC_PAUSED	equ	1
DAC_PLAYING	equ	2
DAC_DONE	equ	3
; DIGPAK-Funktionen (int 66h, AX)
DP_DIGPLAY	equ	688h	; 1: DigPlay
DP_MASSAGE_AUDIO	equ	68ah	; 3: MassageAudio
DP_DIGPLAY2	equ	68bh	; 4: DigPlay2
DP_AUDIO_CAPS	equ	68ch	; 5: AudioCapabilities
DP_SET_CALLBACK	equ	68eh	; 7: SetCallBackAddress
DP_STOP_SOUND	equ	68fh	; 8: StopSound
DP_SHARE_TIMER	equ	693h	; 12: ShareTimer
; DP_attribs (Ergebnis von AudioCapabilities)
BKGND_PLAY	equ	00000001b
MODIFIES_DATA	equ	00000010b
DOWNSAMPLES	equ	00000100b
USES_TICK	equ	00001000b
SHARES_TIMER	equ	00010000b
BLK_SIZE	equ	32768	; groesster Block je int-66h-Aufruf
BIOS_H		equ	16	; Zeitgeber des alten BIOS-int-8
; ---------------------------------------------------------------------------
; Lage der AIL-Daten im Codesegment (Offsets zum Segmentrahmen R22)
; ---------------------------------------------------------------------------
active_timers	equ	0008h	; dw  Zahl angemeldeter Zeitgeber
timer_busy	equ	000ah	; dw  Wiedereintrittszaehler der int-8-Routine
timer_callback	equ	000ch	; dd 17 dup  Rueckruf je Zeitgeber
callback_ds	equ	0050h	; dw 17 dup  DS des Rueckrufs
timer_status	equ	0072h	; dw 17 dup  0 frei, 1 angehalten, 2 laeuft
timer_elapsed	equ	0094h	; dd 17 dup  aufgelaufene Mikrosekunden
timer_value	equ	00d8h	; dd 17 dup  Periode in Mikrosekunden
timer_period	equ	011ch	; dd  Takt des int 8 in Mikrosekunden
bios_callback	equ	0120h	; dd  alter int-8-Vektor
current_timer	equ	0124h	; dw  laufender Zeitgeber der ISR (-1 = abbrechen)
temp_period	equ	0126h	; dd  Zwischenwert in program_timers
PIT_divisor	equ	012ah	; dw  zuletzt programmierter PIT-Teiler
index_base	equ	012ch	; dd 16 dup  Funktionstabelle je Treiber
assigned_timer	equ	016ch	; dw 16 dup  Zeitgeber je Treiber (-1 = keiner)
driver_active	equ	018ch	; dw 16 dup  Treiber initialisiert
drvproc		equ	01ach	; dd  (unbenutzt)
cur_drvr	equ	01b0h	; dw  Laufvariable Treiber
rtn_off		equ	01b2h	; dw  (unbenutzt)
rtn_seg		equ	01b4h	; dw  (unbenutzt)
timer_handle	equ	01b6h	; dw  Zeitgeber von AIL_init_driver
stack_check	equ	01b8h	; db 'Test'  Ueberlaufpruefung des ISR-Stapels
intstack	equ	03bch	; Obergrenze des ISR-Stapels (512 Bytes darunter)
old_ss		equ	03bch	; dw  SS:SP des unterbrochenen Programms
old_sp		equ	03beh	; dw
current_rev	equ	03c0h	; dw  211: API-Stand fuer die Versionspruefung der Treiber
; DIGPAK-Daten
DP_IDI_index	equ	0d84h	; Funktionstabelle der DIGPAK-Schale
DP_DDT		equ	0dbah	; nachgebildete Treiberbeschreibung (drvr_desc)
DP_devname_s	equ	0dc4h	; dw  Segment der Geraetenamen (= CS)
DP_uses_INT8	equ	0dceh	; dw  0: DIGPAK haelt die Musik an, -1: in Ordnung
DP_name		equ	0dd2h	; db 80 dup  Geraetename des DIGPAK-Treibers
call_off	equ	0e22h	; dw  DP_call: far-Zeiger fuer Installation/Entfernen
call_seg	equ	0e24h	; dw
DP_handle	equ	0e26h	; dw  AIL-Handle des DIGPAK-Treibers
DP_registered	equ	0e28h	; dw  DIGPAK-Treiber angemeldet
DP_installed	equ	0e2ah	; dw  DIGPAK-Treiber installiert (int 66h aktiv)
DP_base		equ	0e2ch	; dd  Einsprung des .COM-Treibers (seg-10h:100h)
DP_attribs	equ	0e30h	; dw  Faehigkeiten (DP_AUDIO_CAPS)
DPS		equ	0e32h	; DP_sndstruc fuer die Ausgabe
DPT		equ	0e3eh	; DP_sndstruc fuer das Vorformatieren
sndptr		equ	0	; DP_sndstruc: dd Datenzeiger
sndlen		equ	4	;   dw Laenge
sndstat		equ	6	;   dd Zeiger auf das Statuswort
sndfreq		equ	10	;   dw Abtastrate
buff_data_o	equ	0e4ah	; dw 2 dup  Puffer 0/1: Datenoffset
buff_data_s	equ	0e4eh	; dw 2 dup  Datensegment
buff_len_l	equ	0e52h	; dw 2 dup  Laenge unten
buff_len_h	equ	0e56h	; dw 2 dup  Laenge oben
buff_sample	equ	0e5ah	; dw 2 dup  Abtastrate (SB-Zeitkonstante)
buff_status	equ	0e5eh	; dw 2 dup  DAC_*
buffer_mode	equ	0e62h	; dw  0 VOC-Datei, 1 Doppelpuffer
DAC_status	equ	0e64h	; dw  DAC_*
current_buffer	equ	0e66h	; dw  Index (* 2) des laufenden Puffers
block_ptr	equ	0e68h	; dd  aktueller VOC-Block
loop_ptr	equ	0e6ch	; dd  Anfang der VOC-Wiederholung
loop_cnt	equ	0e70h	; dw  Wiederholungen (0ffffh = endlos)
DMA_ptr		equ	0e72h	; dd  naechster Datenblock ("DMA"-Nachbildung)
DMA_len_l	equ	0e76h	; dw  Restlaenge unten
DMA_len_h	equ	0e78h	; dw  Restlaenge oben
packing		equ	0e7ah	; dw  (unbenutzt)
playing_flag	equ	0e7ch	; dw  Statuswort fuer DIGPAK
DP_timer	equ	0e7eh	; dw  Platzhalter-Zeitgeber der DIGPAK-Schale
AIL_hooked	equ	0e80h	; dw  int 8 eingehaengt
; Code-Offsets, die als Zahl im Code stehen (ohne Fixup wie im Original)
API_timer_ofs	equ	0412h
bios_caller_ofs	equ	051fh
AIL_interrupt_divisor_ofs	equ	0a15h
CB_play_VOC_ofs	equ	1065h
CB_play_buffer_ofs	equ	1084h
	locals
SEG022_TEXT	segment byte public use16 'CODE'
	assume	cs:SEG022_TEXT, ds:nothing
	public	F_R22_03C2
	public	F_R22_03FA
	public	F_R22_04E3
	public	F_R22_0526
	public	F_R22_0572
	public	F_R22_05A2
	public	F_R22_05CB
	public	F_R22_05F5
	public	F_R22_0640
	public	F_R22_06D5
	public	F_R22_0720
	public	F_R22_0785
	public	F_R22_0850
	public	F_R22_0899
	public	F_R22_08B7
	public	F_R22_08FD
	public	F_R22_0943
	public	F_R22_0998
	public	F_R22_09D9
	public	F_R22_0A15
	public	F_R22_0A29
	public	F_R22_0B0C
	public	F_R22_0B53
	public	F_R22_0B79
	public	F_R22_0B7F
	public	F_R22_0C31
	public	F_R22_0C64
	public	F_R22_0C6A
	public	F_R22_0C70
	public	F_R22_0C76
	public	F_R22_0C8E
	public	F_R22_0C9A
	public	F_R22_0CA0
	public	F_R22_0CA6
	public	F_R22_0CBE
	public	F_R22_0CC4
	public	F_R22_0CCA
	public	F_R22_0CD0
	public	F_R22_0CD6
	public	F_R22_0CDC
	public	F_R22_0CE2
	public	F_R22_0CFA
	public	F_R22_0D00
	public	F_R22_0D06
	public	F_R22_0D0C
	public	F_R22_0D12
	public	F_R22_0D1E
	public	F_R22_0D60
	public	F_R22_0D66
	public	F_R22_0EB9
	public	F_R22_0EF0
	public	F_R22_0F37
	public	F_R22_0F46
	public	F_R22_0F5D
	public	F_R22_0F98
	public	F_R22_104B
	public	F_R22_10B8
	public	F_R22_110D
	public	F_R22_1134
	public	F_R22_128B
	public	F_R22_12DE
	public	F_R22_1309
	public	F_R22_13E0
	public	F_R22_1460
	public	F_R22_146E
	public	F_R22_14D3
	public	F_R22_1506
	public	F_R22_162C
	public	F_R22_168B
	public	F_R22_16A0
	public	F_R22_1746
	public	F_R22_1837
	public	F_R22_1874
	public	F_R22_18BC
	public	F_R22_1900
; ===========================================================================
; Lokale Daten (AIL.ASM "Local data"); Namen s. equ oben
; ===========================================================================
	dw	0		;@0008 active_timers
	dw	0		;@000a timer_busy
	ZEROS	17*4	;@000c timer_callback (dd 17 dup)
	ZEROS	17*2	;@0050 callback_ds (dw 17 dup)
	ZEROS	17*2	;@0072 timer_status (dw 17 dup)
	ZEROS	17*4	;@0094 timer_elapsed (dd 17 dup)
	ZEROS	17*4	;@00d8 timer_value (dd 17 dup)
	dd	0		;@011c timer_period
	dd	0		;@0120 bios_callback
	dw	0		;@0124 current_timer
	dd	0		;@0126 temp_period
	dw	0		;@012a PIT_divisor
	ZEROS	16*4	;@012c index_base (dd 16 dup)
	ZEROS	16*2	;@016c assigned_timer (dw 16 dup)
	ZEROS	16*2	;@018c driver_active (dw 16 dup)
	dd	0		;@01ac drvproc
	dw	0		;@01b0 cur_drvr
	dw	0		;@01b2 rtn_off
	dw	0		;@01b4 rtn_seg
	dw	0		;@01b6 timer_handle
	db	'Test'		;@01b8 stack_check
	ZEROS	256*2	;@01bc Stapel der int-8-Routine, Obergrenze intstack (dw 256 dup)
	dw	0		;@03bc old_ss
	dw	0		;@03be old_sp
	dw	211		;@03c0 current_rev
; ===========================================================================
; Interne Prozeduren
; ===========================================================================
; ---------------------------------------------------------------------------
; find_proc - Adresse der Treiberfunktion AX im Treiber BX suchen
; Ein:  AX = Funktionsnummer (AIL_*), BX = Treiberhandle
; Aus:  DX:AX = Einsprung im Treiber, 0:0 = Funktion fehlt oder Handle
;       ungueltig. Wiedereintrittsfest (nur Register).
; Die Funktionstabelle des Treibers (index_base[BX]) besteht aus Paaren
; (Nummer, Offset), Ende -1; der Treiber ist auf Offset 0 gelinkt.
; ---------------------------------------------------------------------------
F_R22_03C2	label	far	; Linkname von find_proc
find_proc:
	cmp	bx,010h	;@03c2
	jnc	short @@bad_handle	;@03c5
	shl	bx,1	;@03c7
	shl	bx,1	;@03c9
	les	bx,cs:[bx+index_base]	;@03cb
	mov	cx,es	;@03d0
	or	cx,bx	;@03d2
	jz	short @@bad_handle	;@03d4
@@find_proc:
	mov	cx,es:[bx]	;@03d6
	cmp	cx,ax	;@03d9
	jz	short @@found	;@03db
	add	bx,4	;@03dd
	cmp	cx,-1	;@03e0
	jnz	@@find_proc	;@03e3
	mov	ax,0	;@03e5
	mov	dx,0	;@03e8
	retf	;@03eb
@@bad_handle:
	mov	ax,0	;@03ec
	mov	dx,0	;@03ef
	retf	;@03f2
@@found:
	mov	ax,es:[bx+2]	;@03f3
	mov	dx,es	;@03f7
	retf	;@03f9
; ---------------------------------------------------------------------------
; call_driver - Treiberfunktion AX aufrufen
; Ein:  AX = Funktionsnummer, [sp+4] = Treiberhandle (erster Parameter des
;       Aufrufers; die Stubs springen mit dessen Rueckkehradresse hierher)
; Aus:  was der Treiber liefert; DX:AX = 0, wenn die Funktion fehlt
; Der Treiber wird per push dx/push ax/retf angesprungen und kehrt direkt
; zum Aufrufer des Stubs zurueck.
; ---------------------------------------------------------------------------
F_R22_03FA	label	far	; Linkname von call_driver
call_driver:
	mov	bx,sp	;@03fa
	mov	bx,ss:[bx+4]	;@03fc
	push	cs	;@0400
	call	near ptr find_proc	;@0401
	cmp	ax,0	;@0404
	jnz	short @@do_call	;@0407
	cmp	dx,0	;@0409
	jz	short @@invalid_call	;@040c
@@do_call:
	push	dx	;@040e
	push	ax	;@040f
	retf	;@0410
@@invalid_call:
	retf	;@0411
; ---------------------------------------------------------------------------
; API_timer - neue int-8-Routine (Zeitgeberverteiler)
; Bei jedem Takt (timer_period Mikrosekunden): fuer jeden laufenden Zeitgeber
; 0..16 timer_elapsed += timer_period; ab timer_value wird abgezogen und der
; Rueckruf far gerufen (mit DS = callback_ds). Zeitgeber 16 (BIOS_H) ruft den
; alten int 8 (bios_caller). Eigener Stapel (512 Bytes im Codesegment), nur
; beim ersten Eintritt; ein Wiedereintritt beendet sich sofort mit EOI.
; Am Ende Stapelpruefung: ist 'Test' ueberschrieben, haelt sie mit int 3 an.
; ---------------------------------------------------------------------------
API_timer:
	inc	word ptr cs:[timer_busy]	;@0412
	cld	;@0417
	push	ax	;@0418
	push	bx	;@0419
	push	cx	;@041a
	push	dx	;@041b
	push	si	;@041c
	push	di	;@041d
	push	bp	;@041e
	push	es	;@041f
	push	ds	;@0420
	cmp	word ptr cs:[timer_busy],1	;@0421
	jz	short @@first_entry	;@0427
	db	0e9h,08eh,0	; jmp @@exit	;@0429 Wiedereintritt: gleich zum Ende
@@first_entry:
	mov	cs:[old_ss],ss	;@042c
	mov	cs:[old_sp],sp	;@0431
	mov	ax,cs	;@0436
	mov	ss,ax	;@0438
	mov	sp,intstack	;@043a eigener Stapel
	mov	word ptr cs:[current_timer],0	;@043d
@@for_timer:
	mov	si,cs:[current_timer]	;@0444
	shl	si,1	;@0449
	db	02eh,083h,0bch,072h,0,2	; cmp word cs:[si+timer_status] (disp16),byte +0x2	;@044b laeuft der Zeitgeber?
	db	075h,050h	; jnz @@next_timer	;@0451
	db	02eh,08eh,09ch,050h,0	; mov ds,cs:[si+callback_ds] (disp16)	;@0453 DS des Rueckrufs
	shl	si,1	;@0458
	mov	ax,cs:[si+timer_elapsed]	;@045a
	mov	dx,cs:[si+timer_elapsed+2]	;@045f
	add	ax,cs:[timer_period]	;@0464
	adc	dx,cs:[timer_period+2]	;@0469
	cmp	dx,cs:[si+timer_value+2]	;@046e
	jc	short @@dec_timer	;@0473
	ja	short @@timer_tick	;@0475
	cmp	ax,cs:[si+timer_value]	;@0477
	jnc	short @@timer_tick	;@047c
@@dec_timer:
	mov	cs:[si+timer_elapsed],ax	;@047e
	mov	cs:[si+timer_elapsed+2],dx	;@0483
	db	0ebh,019h	; jmp short @@next_timer	;@0488
@@timer_tick:
	sub	ax,cs:[si+timer_value]	;@048a
	sbb	dx,cs:[si+timer_value+2]	;@048f
	mov	cs:[si+timer_elapsed],ax	;@0494
	mov	cs:[si+timer_elapsed+2],dx	;@0499
	db	02eh,0ffh,09ch,0ch,0	; call far cs:[si+timer_callback] (disp16)	;@049e Zeitgeber abgelaufen: Rueckruf
@@next_timer:
	inc	word ptr cs:[current_timer]	;@04a3 current_timer = -1 bricht die Runde ab
	cmp	word ptr cs:[current_timer],010h	;@04a8
	jna	@@for_timer	;@04ae
	mov	ss,cs:[old_ss]	;@04b0
	mov	sp,cs:[old_sp]	;@04b5
@@exit:
	pop	ds	;@04ba
	pop	es	;@04bb
	pop	bp	;@04bc
	pop	di	;@04bd
	pop	si	;@04be
	pop	dx	;@04bf
	pop	cx	;@04c0
	pop	bx	;@04c1
	mov	al,020h	;@04c2 EOI an den Interrupt-Controller
	out	020h,al	;@04c4
	pop	ax	;@04c6
	cmp	word ptr cs:[stack_check],06554h	;@04c7 'Te' 'st' noch da?
	jnz	short @@stack_fault	;@04ce
	cmp	word ptr cs:[stack_check+2],07473h	;@04d0
	jnz	short @@stack_fault	;@04d7
	dec	word ptr cs:[timer_busy]	;@04d9
	iret	;@04de
@@stack_fault:
	sti	;@04df
	db	0cch	; int3	;@04e0 Stapel zu klein: hier anhalten
	jmp	short @@stack_fault	;@04e1
; ---------------------------------------------------------------------------
; init_DDA_arrays - Zaehler aller Zeitgeber loeschen
; Ein:  -
; Aus:  -   (timer_period = -1, timer_status/_elapsed/_value = 0)
; ---------------------------------------------------------------------------
F_R22_04E3	label	far	; Linkname von init_DDA_arrays
init_DDA_arrays:
	push	ds	;@04e3
	push	si	;@04e4
	push	di	;@04e5
	pushf	;@04e6
	cli	;@04e7
	cld	;@04e8
	mov	word ptr cs:[timer_period],0ffffh	;@04e9
	mov	word ptr cs:[timer_period+2],0ffffh	;@04f0
	push	cs	;@04f7
	pop	es	;@04f8
	mov	di,timer_status	;@04f9
	mov	cx,011h	;@04fc
	mov	ax,0	;@04ff
	rep	stosw	;@0502
	mov	di,timer_elapsed	;@0504
	mov	cx,022h	;@0507
	rep	stosw	;@050a
	mov	di,timer_value	;@050c
	mov	cx,022h	;@050f
	rep	stosw	;@0512
	POP_F		;@0514
	pop	di	;@051b
	pop	si	;@051c
	pop	ds	;@051d
	retf	;@051e
; ---------------------------------------------------------------------------
; bios_caller - Rueckruf des Zeitgebers BIOS_H: alten int 8 aufrufen
; (pushf + call far wie ein Interrupt), damit die BIOS-Uhr weiterlaeuft
; ---------------------------------------------------------------------------
bios_caller:
	pushf	;@051f
	call	dword ptr cs:[bios_callback]	;@0520
	retf	;@0525
; ---------------------------------------------------------------------------
; hook_timer_process - int 8 uebernehmen
; Ein:  -
; Aus:  -
; Alten Vektor nach bios_callback, als Rueckruf des Zeitgebers BIOS_H
; bios_caller eintragen, int 8 auf API_timer setzen.
; ---------------------------------------------------------------------------
F_R22_0526	label	far	; Linkname von hook_timer_process
hook_timer_process:
	push	ds	;@0526
	push	si	;@0527
	push	di	;@0528
	pushf	;@0529
	cli	;@052a
	mov	ax,0	;@052b
	mov	es,ax	;@052e
	mov	bx,es:[020h]	;@0530
	mov	es,es:[022h]	;@0535
	mov	cs:[bios_callback],bx	;@053a
	mov	cs:[bios_callback+2],es	;@053f
	mov	bx,bios_caller_ofs	;@0544
	mov	cs:[timer_callback+BIOS_H*4],bx	;@0547
	mov	cs:[timer_callback+BIOS_H*4+2],cs	;@054c
	mov	ax,cs	;@0551
	mov	ds,ax	;@0553
	mov	dx,API_timer_ofs	;@0555
	mov	ax,0	;@0558
	mov	es,ax	;@055b
	mov	es:[020h],dx	;@055d
	mov	es:[022h],ds	;@0562
	POP_F		;@0567
	pop	di	;@056e
	pop	si	;@056f
	pop	ds	;@0570
	retf	;@0571
; ---------------------------------------------------------------------------
; unhook_timer_process - alten int 8 herstellen
; Ein:  -
; Aus:  -   (current_timer = -1: keine weiteren Rueckrufe in dieser Runde)
; ---------------------------------------------------------------------------
F_R22_0572	label	far	; Linkname von unhook_timer_process
unhook_timer_process:
	push	ds	;@0572
	push	si	;@0573
	push	di	;@0574
	pushf	;@0575
	cli	;@0576
	mov	word ptr cs:[current_timer],0ffffh	;@0577
	mov	dx,cs:[bios_callback]	;@057e
	mov	ds,cs:[bios_callback+2]	;@0583
	mov	ax,0	;@0588
	mov	es,ax	;@058b
	mov	es:[020h],dx	;@058d
	mov	es:[022h],ds	;@0592
	POP_F		;@0597
	pop	di	;@059e
	pop	si	;@059f
	pop	ds	;@05a0
	retf	;@05a1
; ---------------------------------------------------------------------------
; set_PIT_divisor - Zeitgeberbaustein 8253 (Kanal 0) programmieren
; Ein:  [bp+6] Teiler (0 = 65536, 1193181 Hz / Teiler)
; Aus:  -   (Teiler auch nach PIT_divisor)
; ---------------------------------------------------------------------------
F_R22_05A2	label	far	; Linkname von set_PIT_divisor
set_PIT_divisor:
	push	bp	;@05a2
	mov	bp,sp	;@05a3
	push	ds	;@05a5
	push	si	;@05a6
	push	di	;@05a7
	pushf	;@05a8
	cli	;@05a9
	mov	al,036h	;@05aa Kanal 0, Modus 3, erst unteres, dann oberes Byte
	out	043h,al	;@05ac
	mov	ax,[bp+6]	;@05ae
	mov	cs:[PIT_divisor],ax	;@05b1
	jmp	short @@io_delay1	;@05b5 jmp $+2: E/A-Pause
@@io_delay1:
	out	040h,al	;@05b7
	mov	al,ah	;@05b9
	jmp	short @@io_delay2	;@05bb jmp $+2: E/A-Pause
@@io_delay2:
	out	040h,al	;@05bd
	POP_F		;@05bf
	pop	di	;@05c6
	pop	si	;@05c7
	pop	ds	;@05c8
	pop	bp	;@05c9
	retf	;@05ca
; ---------------------------------------------------------------------------
; set_PIT_period - Zeitgeber auf eine Periode in Mikrosekunden stellen
; Ein:  [bp+6] Periode in us
; Aus:  -
; Teiler = us * 10000 / 8380 (abgerundet gegen Drift); ab 54925 us
; (BIOS-Vorgabe 55 ms) Teiler 0 = 65536.
; ---------------------------------------------------------------------------
F_R22_05CB	label	far	; Linkname von set_PIT_period
set_PIT_period:
	push	bp	;@05cb
	mov	bp,sp	;@05cc
	push	ds	;@05ce
	push	si	;@05cf
	push	di	;@05d0
	mov	ax,0	;@05d1
	cmp	word ptr [bp+6],54925	;@05d4
	jnc	short @@set_PIT	;@05d9
	mov	ax,[bp+6]	;@05db
	mov	bx,8380	;@05de
	mov	cx,10000	;@05e1
	mul	cx	;@05e4
	div	bx	;@05e6
@@set_PIT:
	push	ax	;@05e8
	push	cs	;@05e9
	call	near ptr set_PIT_divisor	;@05ea
	add	sp,2	;@05ed
	pop	di	;@05f0
	pop	si	;@05f1
	pop	ds	;@05f2
	pop	bp	;@05f3
	retf	;@05f4
; ---------------------------------------------------------------------------
; ul_divide - 32-Bit-Division ohne Vorzeichen
; Ein:  [bp+6] Zaehler (dd), [bp+0ah] Nenner (dd)
; Aus:  DX:AX = Quotient
; Kurzweg mit div, wenn Nenner und Zaehler-Hochwort es erlauben, sonst
; schriftlich (32 Schritte, Rest in DI:SI).
; ---------------------------------------------------------------------------
F_R22_05F5	label	far	; Linkname von ul_divide
ul_divide:
	push	bp	;@05f5
	mov	bp,sp	;@05f6
	push	ds	;@05f8
	push	si	;@05f9
	push	di	;@05fa
	mov	ax,[bp+6]	;@05fb
	mov	dx,[bp+8]	;@05fe
	mov	bx,[bp+0ah]	;@0601
	mov	cx,[bp+0ch]	;@0604
	or	cx,cx	;@0607
	jnz	short @@long_div	;@0609
	or	dx,dx	;@060b
	jz	short @@short_div	;@060d
	or	bx,bx	;@060f
	jz	short @@short_div	;@0611
@@long_div:
	mov	bp,cx	;@0613
	mov	cx,020h	;@0615
	xor	di,di	;@0618
	xor	si,si	;@061a
@@div_loop:
	shl	ax,1	;@061c
	rcl	dx,1	;@061e
	rcl	si,1	;@0620
	rcl	di,1	;@0622
	cmp	di,bp	;@0624
	jc	short @@cont_loop	;@0626
	ja	short @@bump_quot	;@0628
	cmp	si,bx	;@062a
	jc	short @@cont_loop	;@062c
@@bump_quot:
	sub	si,bx	;@062e
	sbb	di,bp	;@0630
	inc	ax	;@0632
@@cont_loop:
	loop	@@div_loop	;@0633
	jmp	short @@end_div	;@0635
@@short_div:
	div	bx	;@0637
	xor	dx,dx	;@0639
@@end_div:
	pop	di	;@063b
	pop	si	;@063c
	pop	ds	;@063d
	pop	bp	;@063e
	retf	;@063f
; ---------------------------------------------------------------------------
; program_timers - int-8-Takt nach dem schnellsten aktiven Zeitgeber setzen
; Ein:  -
; Aus:  -
; Kleinstes timer_value aller belegten Zeitgeber (mit BIOS_H, also hoechstens
; 54925 us); hat sich der Takt geaendert: neu setzen, alle timer_elapsed = 0,
; current_timer = -1.
; ---------------------------------------------------------------------------
F_R22_0640	label	far	; Linkname von program_timers
program_timers:
	push	ds	;@0640
	push	si	;@0641
	push	di	;@0642
	pushf	;@0643
	cli	;@0644
	cld	;@0645
	mov	word ptr cs:[temp_period],0ffffh	;@0646
	mov	word ptr cs:[temp_period+2],0ffffh	;@064d
	mov	si,0	;@0654
@@for_timer:
	mov	bx,si	;@0657
	shl	bx,1	;@0659
	db	02eh,083h,0bfh,072h,0,0	; cmp word cs:[bx+timer_status] (disp16),byte +0x0	;@065b Zeitgeber belegt?
	jz	short @@next_timer	;@0661
	shl	bx,1	;@0663
	mov	ax,cs:[bx+timer_value]	;@0665
	mov	dx,cs:[bx+timer_value+2]	;@066a
	cmp	dx,cs:[temp_period+2]	;@066f
	jc	short @@set_temp	;@0674
	ja	short @@next_timer	;@0676
	cmp	ax,cs:[temp_period]	;@0678
	jnc	short @@next_timer	;@067d
@@set_temp:
	mov	cs:[temp_period],ax	;@067f
	mov	cs:[temp_period+2],dx	;@0683
@@next_timer:
	inc	si	;@0688
	cmp	si,010h	;@0689
	jna	@@for_timer	;@068c
	mov	ax,cs:[temp_period]	;@068e
	mov	dx,cs:[temp_period+2]	;@0692
	cmp	ax,cs:[timer_period]	;@0697
	jnz	short @@must_reset	;@069c
	cmp	dx,cs:[timer_period+2]	;@069e
	jz	short @@no_change	;@06a3
@@must_reset:
	mov	word ptr cs:[current_timer],0ffffh	;@06a5
	mov	cs:[timer_period],ax	;@06ac
	mov	cs:[timer_period+2],dx	;@06b0
	push	ax	;@06b5
	push	cs	;@06b6
	call	near ptr set_PIT_period	;@06b7
	add	sp,2	;@06ba
	push	cs	;@06bd
	pop	es	;@06be
	mov	di,timer_elapsed	;@06bf
	mov	cx,022h	;@06c2
	mov	ax,0	;@06c5
	rep	stosw	;@06c8
@@no_change:
	POP_F		;@06ca
	pop	di	;@06d1
	pop	si	;@06d2
	pop	ds	;@06d3
	retf	;@06d4
; ===========================================================================
; Zeitgeberdienste (Process services)
; ===========================================================================
; ---------------------------------------------------------------------------
; AIL_startup - AIL initialisieren
; Ein:  -
; Aus:  -   (keine Zeitgeber, keine Treiber, DIGPAK nicht angemeldet)
; ---------------------------------------------------------------------------
F_R22_06D5	label	far	; Linkname von AIL_startup
AIL_startup:
	push	ds	;@06d5
	push	si	;@06d6
	push	di	;@06d7
	pushf	;@06d8
	cli	;@06d9
	mov	word ptr cs:[active_timers],0	;@06da
	mov	word ptr cs:[timer_busy],0	;@06e1
	mov	word ptr cs:[DP_registered],0	;@06e8
	cld	;@06ef
	mov	ax,cs	;@06f0
	mov	es,ax	;@06f2
	mov	di,index_base	;@06f4
	mov	cx,020h	;@06f7
	mov	ax,0	;@06fa
	rep	stosw	;@06fd
	mov	di,assigned_timer	;@06ff
	mov	cx,010h	;@0702
	mov	ax,0ffffh	;@0705
	rep	stosw	;@0708
	mov	di,driver_active	;@070a
	mov	cx,010h	;@070d
	mov	ax,0	;@0710
	rep	stosw	;@0713
	POP_F		;@0715
	pop	di	;@071c
	pop	si	;@071d
	pop	ds	;@071e
	retf	;@071f
; ---------------------------------------------------------------------------
; AIL_shutdown - alle AIL-Mittel schnell freigeben
; Ein:  [bp+6] char far *signoff_msg (an die Treiber weitergegeben)
; Aus:  -
; Fuer jeden belegten Treiber: Zeitgeber freigeben, AIL_shutdown_driver;
; dann AIL_release_all_timers.
; ---------------------------------------------------------------------------
F_R22_0720	label	far	; Linkname von AIL_shutdown
AIL_shutdown:
	push	bp	;@0720
	mov	bp,sp	;@0721
	push	ds	;@0723
	push	si	;@0724
	push	di	;@0725
	pushf	;@0726
	cli	;@0727
	mov	word ptr cs:[cur_drvr],0	;@0728
@@for_slot:
	mov	si,cs:[cur_drvr]	;@072f
	shl	si,1	;@0734
	mov	dx,cs:[si+assigned_timer]	;@0736
	shl	si,1	;@073b
	mov	ax,cs:[si+index_base]	;@073d
	or	ax,cs:[si+index_base+2]	;@0742
	jz	short @@next_slot	;@0747
	cmp	dx,-1	;@0749
	jz	short @@shut_down	;@074c
	push	dx	;@074e
	push	cs	;@074f
	db	0e8h,0fdh,0	; call AIL_release_timer_handle	;@0750
	add	sp,2	;@0753
@@shut_down:
	push	word ptr [bp+8]	;@0756
	push	word ptr [bp+6]	;@0759
	push	word ptr cs:[cur_drvr]	;@075c
	push	cs	;@0761
	db	0e8h,0cch,4	; call AIL_shutdown_driver	;@0762
	add	sp,6	;@0765
@@next_slot:
	inc	word ptr cs:[cur_drvr]	;@0768
	cmp	word ptr cs:[cur_drvr],010h	;@076d
	jnz	@@for_slot	;@0773
	push	cs	;@0775
	db	0e8h,020h,1	; call AIL_release_all_timers	;@0776
	POP_F		;@0779
	pop	di	;@0780
	pop	si	;@0781
	pop	ds	;@0782
	pop	bp	;@0783
	retf	;@0784
; ---------------------------------------------------------------------------
; AIL_register_timer - Zeitgeber anmelden
; Ein:  [bp+6] Rueckruf (far, cdecl); DS des Aufrufers wird mitgemerkt
; Aus:  AX = Handle 0..15, -1 = keiner frei
; Der Zeitgeber ist danach angehalten (Status 1), Periode -1. Beim ersten
; Zeitgeber: Zaehler loeschen, int 8 uebernehmen, BIOS_H mit 54925 us
; starten.
; ---------------------------------------------------------------------------
F_R22_0785	label	far	; Linkname von AIL_register_timer
AIL_register_timer:
	push	bp	;@0785
	mov	bp,sp	;@0786
	push	ds	;@0788
	push	si	;@0789
	push	di	;@078a
	pushf	;@078b
	cli	;@078c
	mov	cx,ds	;@078d
	mov	bx,0	;@078f
@@find_free:
	db	02eh,083h,0bfh,072h,0,0	; cmp word cs:[bx+timer_status] (disp16),byte +0x0	;@0792
	jz	short @@found	;@0798
	add	bx,2	;@079a
	cmp	bx,020h	;@079d
	jc	@@find_free	;@07a0
	mov	ax,0ffffh	;@07a2
	db	0e9h,09ch,0	; jmp @@return	;@07a5 keiner frei: AX = -1
@@found:
	mov	ax,bx	;@07a8
	shr	ax,1	;@07aa
	db	02eh,0c7h,087h,072h,0,1,0	; mov word cs:[bx+timer_status] (disp16),0x1	;@07ac timer_status = 1 (angehalten)
	db	02eh,089h,08fh,050h,0	; mov cs:[bx+callback_ds] (disp16),cx	;@07b3 callback_ds = DS
	shl	bx,1	;@07b8
	lds	si,[bp+6]	;@07ba
	db	02eh,089h,0b7h,0ch,0	; mov cs:[bx+timer_callback] (disp16),si	;@07bd timer_callback
	db	02eh,08ch,09fh,0eh,0	; mov cs:[bx+timer_callback+2] (disp16),ds	;@07c2
	mov	word ptr cs:[bx+timer_value],0ffffh	;@07c7
	mov	word ptr cs:[bx+timer_value+2],0ffffh	;@07ce
	inc	word ptr cs:[active_timers]	;@07d5
	cmp	word ptr cs:[active_timers],1	;@07da
	db	075h,062h	; jnz @@return	;@07e0 nicht der erste: fertig
	push	ax	;@07e2
	push	cs	;@07e3
	call	near ptr init_DDA_arrays	;@07e4
	mov	word ptr cs:[timer_status+BIOS_H*2],1	;@07e7 timer_status[BIOS_H] = 1
	push	cs	;@07ee
	call	near ptr hook_timer_process	;@07ef
	PUSH_C	0	;@07f2
	PUSH_C	54925	;@07fc
	PUSH_C	BIOS_H	;@0806
	push	cs	;@0810 AIL_set_timer_period(BIOS_H, 54925)
	db	0e8h,02fh,1	; call AIL_set_timer_period	;@0811
	add	sp,6	;@0814
	PUSH_C	BIOS_H	;@0817
	push	cs	;@0821 AIL_start_timer(BIOS_H)
	db	0e8h,092h,0	; call AIL_start_timer	;@0822
	add	sp,2	;@0825
	pop	ax	;@0828
	mov	bx,ax	;@0829
	shl	bx,1	;@082b
	db	02eh,0c7h,087h,072h,0,1,0	; mov word cs:[bx+timer_status] (disp16),0x1	;@082d (init_DDA_arrays hat ihn geloescht)
	shl	bx,1	;@0834
	mov	word ptr cs:[bx+timer_value],0ffffh	;@0836
	mov	word ptr cs:[bx+timer_value+2],0ffffh	;@083d
@@return:
	POP_F		;@0844
	pop	di	;@084b
	pop	si	;@084c
	pop	ds	;@084d
	pop	bp	;@084e
	retf	;@084f
; ---------------------------------------------------------------------------
; AIL_release_timer_handle - Zeitgeber freigeben
; Ein:  [bp+6] Handle (-1 wird ignoriert)
; Aus:  -
; Mit dem letzten Zeitgeber: PIT auf 65536 und alten int 8 herstellen.
; ---------------------------------------------------------------------------
F_R22_0850	label	far	; Linkname von AIL_release_timer_handle
AIL_release_timer_handle:
	push	bp	;@0850
	mov	bp,sp	;@0851
	push	ds	;@0853
	push	si	;@0854
	push	di	;@0855
	pushf	;@0856
	cli	;@0857
	mov	bx,[bp+6]	;@0858
	cmp	bx,-1	;@085b
	db	074h,02dh	; jz @@return	;@085e Handle -1: nichts tun
	shl	bx,1	;@0860
	db	02eh,083h,0bfh,072h,0,0	; cmp word cs:[bx+timer_status] (disp16),byte +0x0	;@0862
	db	074h,023h	; jz @@return	;@0868 nicht belegt: nichts tun
	db	02eh,0c7h,087h,072h,0,0,0	; mov word cs:[bx+timer_status] (disp16),0x0	;@086a
	dec	word ptr cs:[active_timers]	;@0871
	jnz	short @@return	;@0876
	PUSH_C	0	;@0878
	push	cs	;@0882
	call	near ptr set_PIT_divisor	;@0883
	add	sp,2	;@0886
	push	cs	;@0889
	call	near ptr unhook_timer_process	;@088a
@@return:
	POP_F		;@088d
	pop	di	;@0894
	pop	si	;@0895
	pop	ds	;@0896
	pop	bp	;@0897
	retf	;@0898
; ---------------------------------------------------------------------------
; AIL_release_all_timers - Zeitgeber 15..0 freigeben
; ---------------------------------------------------------------------------
F_R22_0899	label	far	; Linkname von AIL_release_all_timers
AIL_release_all_timers:
	push	ds	;@0899
	push	si	;@089a
	push	di	;@089b
	pushf	;@089c
	cli	;@089d
	mov	si,0fh	;@089e
@@release_it:
	push	si	;@08a1
	push	cs	;@08a2
	call	near ptr AIL_release_timer_handle	;@08a3
	add	sp,2	;@08a6
	dec	si	;@08a9
	jnl	@@release_it	;@08aa
	POP_F		;@08ac
	pop	di	;@08b3
	pop	si	;@08b4
	pop	ds	;@08b5
	retf	;@08b6
; ---------------------------------------------------------------------------
; AIL_start_timer - angehaltenen Zeitgeber starten (Status 1 -> 2)
; Ein:  [bp+6] Handle
; ---------------------------------------------------------------------------
F_R22_08B7	label	far	; Linkname von AIL_start_timer
AIL_start_timer:
	push	bp	;@08b7
	mov	bp,sp	;@08b8
	push	ds	;@08ba
	push	si	;@08bb
	push	di	;@08bc
	pushf	;@08bd
	cli	;@08be
	mov	bx,[bp+6]	;@08bf
	shl	bx,1	;@08c2
	db	02eh,083h,0bfh,072h,0,1	; cmp word cs:[bx+timer_status] (disp16),byte +0x1	;@08c4
	db	075h,7	; jnz @@return	;@08ca
	db	02eh,0c7h,087h,072h,0,2,0	; mov word cs:[bx+timer_status] (disp16),0x2	;@08cc
@@return:
	POP_F		;@08d3
	pop	di	;@08da
	pop	si	;@08db
	pop	ds	;@08dc
	pop	bp	;@08dd
	retf	;@08de
; ---------------------------------------------------------------------------
; AIL_start_all_timers - Zeitgeber 15..0 starten (ohne Linknamen, unbenutzt)
; ---------------------------------------------------------------------------
AIL_start_all_timers:
	push	ds	;@08df
	push	si	;@08e0
	push	di	;@08e1
	pushf	;@08e2
	cli	;@08e3
	mov	si,0fh	;@08e4
@@start_it:
	push	si	;@08e7
	push	cs	;@08e8
	call	near ptr AIL_start_timer	;@08e9
	add	sp,2	;@08ec
	dec	si	;@08ef
	jnl	@@start_it	;@08f0
	POP_F		;@08f2
	pop	di	;@08f9
	pop	si	;@08fa
	pop	ds	;@08fb
	retf	;@08fc
; ---------------------------------------------------------------------------
; AIL_stop_timer - laufenden Zeitgeber anhalten (Status 2 -> 1)
; Ein:  [bp+6] Handle
; ---------------------------------------------------------------------------
F_R22_08FD	label	far	; Linkname von AIL_stop_timer
AIL_stop_timer:
	push	bp	;@08fd
	mov	bp,sp	;@08fe
	push	ds	;@0900
	push	si	;@0901
	push	di	;@0902
	pushf	;@0903
	cli	;@0904
	mov	bx,[bp+6]	;@0905
	shl	bx,1	;@0908
	db	02eh,083h,0bfh,072h,0,2	; cmp word cs:[bx+timer_status] (disp16),byte +0x2	;@090a
	db	075h,7	; jnz @@return	;@0910
	db	02eh,0c7h,087h,072h,0,1,0	; mov word cs:[bx+timer_status] (disp16),0x1	;@0912
@@return:
	POP_F		;@0919
	pop	di	;@0920
	pop	si	;@0921
	pop	ds	;@0922
	pop	bp	;@0923
	retf	;@0924
; ---------------------------------------------------------------------------
; AIL_stop_all_timers - Zeitgeber 15..0 anhalten (ohne Linknamen, unbenutzt)
; ---------------------------------------------------------------------------
AIL_stop_all_timers:
	push	ds	;@0925
	push	si	;@0926
	push	di	;@0927
	pushf	;@0928
	cli	;@0929
	mov	si,0fh	;@092a
@@stop_it:
	push	si	;@092d
	push	cs	;@092e
	call	near ptr AIL_stop_timer	;@092f
	add	sp,2	;@0932
	dec	si	;@0935
	jnl	@@stop_it	;@0936
	POP_F		;@0938
	pop	di	;@093f
	pop	si	;@0940
	pop	ds	;@0941
	retf	;@0942
; ---------------------------------------------------------------------------
; AIL_set_timer_period - Periode eines Zeitgebers in Mikrosekunden
; Ein:  [bp+6] Handle, [bp+8] Periode (dd, us)
; Aus:  -
; Zeitgeber waehrend der Rechnung anhalten, timer_value setzen,
; timer_elapsed = 0, program_timers, alten Status herstellen.
; ---------------------------------------------------------------------------
F_R22_0943	label	far	; Linkname von AIL_set_timer_period
AIL_set_timer_period:
	push	bp	;@0943
	mov	bp,sp	;@0944
	push	ds	;@0946
	push	si	;@0947
	push	di	;@0948
	pushf	;@0949
	cli	;@094a
	mov	bx,[bp+6]	;@094b
	shl	bx,1	;@094e
	db	02eh,08bh,087h,072h,0	; mov ax,cs:[bx+timer_status] (disp16)	;@0950 alten Status merken
	push	ax	;@0955
	db	02eh,0c7h,087h,072h,0,1,0	; mov word cs:[bx+timer_status] (disp16),0x1	;@0956 waehrenddessen angehalten
	shl	bx,1	;@095d
	mov	ax,[bp+8]	;@095f
	mov	dx,[bp+0ah]	;@0962
	mov	cs:[bx+timer_value],ax	;@0965
	mov	cs:[bx+timer_value+2],dx	;@096a
	mov	word ptr cs:[bx+timer_elapsed],0	;@096f
	mov	word ptr cs:[bx+timer_elapsed+2],0	;@0976
	push	cs	;@097d
	call	near ptr program_timers	;@097e
	pop	ax	;@0981
	mov	bx,[bp+6]	;@0982
	shl	bx,1	;@0985
	db	02eh,089h,087h,072h,0	; mov cs:[bx+timer_status] (disp16),ax	;@0987 Status zurueck
	POP_F		;@098c
	pop	di	;@0993
	pop	si	;@0994
	pop	ds	;@0995
	pop	bp	;@0996
	retf	;@0997
; ---------------------------------------------------------------------------
; AIL_set_timer_frequency - Frequenz eines Zeitgebers in Hertz
; Ein:  [bp+6] Handle, [bp+8] Frequenz (dd, Hz)
; Aus:  -   (Periode = 1000000 / Hz ueber ul_divide)
; ---------------------------------------------------------------------------
F_R22_0998	label	far	; Linkname von AIL_set_timer_frequency
AIL_set_timer_frequency:
	push	bp	;@0998
	mov	bp,sp	;@0999
	push	ds	;@099b
	push	si	;@099c
	push	di	;@099d
	pushf	;@099e
	cli	;@099f
	push	word ptr [bp+0ah]	;@09a0
	push	word ptr [bp+8]	;@09a3
	PUSH_C	0fh	;@09a6 1000000 = 000f4240h
	PUSH_C	04240h	;@09b0
	push	cs	;@09ba
	call	near ptr ul_divide	;@09bb
	add	sp,8	;@09be
	push	dx	;@09c1
	push	ax	;@09c2
	push	word ptr [bp+6]	;@09c3
	push	cs	;@09c6
	call	near ptr AIL_set_timer_period	;@09c7
	add	sp,6	;@09ca
	POP_F		;@09cd
	pop	di	;@09d4
	pop	si	;@09d5
	pop	ds	;@09d6
	pop	bp	;@09d7
	retf	;@09d8
; ---------------------------------------------------------------------------
; AIL_set_timer_divisor - Periode als PIT-Teiler
; Ein:  [bp+6] Handle, [bp+8] Teiler (0 = 65536)
; Aus:  -   (us = Teiler * 10000 / 11932)
; ---------------------------------------------------------------------------
F_R22_09D9	label	far	; Linkname von AIL_set_timer_divisor
AIL_set_timer_divisor:
	push	bp	;@09d9
	mov	bp,sp	;@09da
	push	ds	;@09dc
	push	si	;@09dd
	push	di	;@09de
	pushf	;@09df
	cli	;@09e0
	cmp	word ptr [bp+8],0	;@09e1
	jnz	short @@nonzero	;@09e5
	mov	ax,54925	;@09e7
	mov	dx,0	;@09ea
	jmp	short @@set_AXDX	;@09ed
@@nonzero:
	mov	ax,10000	;@09ef
	mov	bx,11932	;@09f2
	mul	word ptr [bp+8]	;@09f5
	div	bx	;@09f8
	mov	dx,0	;@09fa
@@set_AXDX:
	push	dx	;@09fd
	push	ax	;@09fe
	push	word ptr [bp+6]	;@09ff
	push	cs	;@0a02
	call	near ptr AIL_set_timer_period	;@0a03
	add	sp,6	;@0a06
	POP_F		;@0a09
	pop	di	;@0a10
	pop	si	;@0a11
	pop	ds	;@0a12
	pop	bp	;@0a13
	retf	;@0a14
; ---------------------------------------------------------------------------
; AIL_interrupt_divisor - zuletzt programmierter PIT-Teiler
; Ein:  -
; Aus:  AX = PIT_divisor
; Wird den Treibern bei AIL_DESC_DRVR als far-Zeiger mitgegeben.
; ---------------------------------------------------------------------------
F_R22_0A15	label	far	; Linkname von AIL_interrupt_divisor
AIL_interrupt_divisor:
	push	ds	;@0a15
	push	si	;@0a16
	push	di	;@0a17
	pushf	;@0a18
	cli	;@0a19
	mov	ax,cs:[PIT_divisor]	;@0a1a
	POP_F		;@0a1e
	pop	di	;@0a25
	pop	si	;@0a26
	pop	ds	;@0a27
	retf	;@0a28
; ===========================================================================
; Treiberverwaltung (Installation services)
; ===========================================================================
; ---------------------------------------------------------------------------
; AIL_register_driver - Treiber im Speicher anmelden
; Ein:  [bp+6] far-Zeiger auf den geladenen Treiber
; Aus:  AX = Handle 0..15, -1 = kein Platz, kein AIL-Treiber oder zu neu
; Ein DIGPAK-Treiber ("DIGPAK" ab +3) wird ueber die DIGPAK-Schale
; angemeldet (nur einer). Sonst muss ab +2 "Copyright" stehen; das erste
; Wort ist der Abstand zur Funktionstabelle. Die Treiberbeschreibung muss
; min_API_version <= current_rev haben.
; ---------------------------------------------------------------------------
F_R22_0A29	label	far	; Linkname von AIL_register_driver
AIL_register_driver:
	push	bp	;@0a29
	mov	bp,sp	;@0a2a
	push	ds	;@0a2c
	push	si	;@0a2d
	push	di	;@0a2e
	pushf	;@0a2f
	cli	;@0a30
	mov	word ptr cs:[cur_drvr],0	;@0a31
@@find_handle:
	mov	si,cs:[cur_drvr]	;@0a38
	shl	si,1	;@0a3d
	shl	si,1	;@0a3f
	mov	ax,cs:[si+index_base]	;@0a41
	or	ax,cs:[si+index_base+2]	;@0a46
	jz	short @@found	;@0a4b
	inc	word ptr cs:[cur_drvr]	;@0a4d
	cmp	word ptr cs:[cur_drvr],010h	;@0a52
	jnz	@@find_handle	;@0a58
	mov	ax,0ffffh	;@0a5a
	jmp	short @@return	;@0a5d
@@found:
	les	di,[bp+6]	;@0a5f
	cmp	word ptr es:[di+3],04944h	;@0a62 'DI' 'PG' 'KA': DIGPAK-Treiber?
	jnz	short @@check_ADV	;@0a68
	cmp	word ptr es:[di+5],05047h	;@0a6a
	jnz	short @@check_ADV	;@0a70
	cmp	word ptr es:[di+7],04b41h	;@0a72
	jz	short @@register_DP	;@0a78
@@check_ADV:
	mov	ax,0ffffh	;@0a7a
	cmp	word ptr es:[di+2],06f43h	;@0a7d 'Co' 'py': AIL-Treiber?
	jnz	short @@return	;@0a83
	cmp	word ptr es:[di+4],07970h	;@0a85
	jnz	short @@return	;@0a8b
	add	di,es:[di]	;@0a8d Copyright-Text ueberspringen
	mov	cs:[si+index_base],di	;@0a90
	mov	cs:[si+index_base+2],es	;@0a95
	push	word ptr cs:[cur_drvr]	;@0a9a
	push	cs	;@0a9f
	call	near ptr AIL_describe_driver	;@0aa0
	add	sp,2	;@0aa3
	mov	es,dx	;@0aa6
	mov	di,ax	;@0aa8
	or	dx,ax	;@0aaa
	mov	ax,0ffffh	;@0aac
	jz	short @@return	;@0aaf
	mov	dx,es:[di]	;@0ab1
	cmp	dx,cs:[current_rev]	;@0ab4 min_API_version > current_rev?
	ja	short @@return	;@0ab9
@@valid_handle:
	mov	ax,cs:[cur_drvr]	;@0abb
@@return:
	POP_F		;@0abf
	pop	di	;@0ac6
	pop	si	;@0ac7
	pop	ds	;@0ac8
	pop	bp	;@0ac9
	retf	;@0aca
@@register_DP:
	mov	ax,0ffffh	;@0acb
	cmp	word ptr cs:[DP_registered],0	;@0ace
	jnz	@@return	;@0ad4
	mov	word ptr cs:[DP_registered],1	;@0ad6
	mov	ax,cs:[cur_drvr]	;@0add
	mov	cs:[DP_handle],ax	;@0ae1
	mov	ax,es	;@0ae5
	sub	ax,010h	;@0ae7 .COM-Einsprung: ES-10h:DI+100h
	add	di,0100h	;@0aea
	mov	cs:[DP_base+2],ax	;@0aee
	mov	cs:[DP_base],di	;@0af2
	mov	word ptr cs:[si+index_base],DP_IDI_index	;@0af7 Funktionstabelle der DIGPAK-Schale
	mov	cs:[si+index_base+2],cs	;@0afe
	mov	word ptr cs:[DP_installed],0	;@0b03
	jmp	short @@valid_handle	;@0b0a
; ---------------------------------------------------------------------------
; AIL_release_driver_handle - Treiberhandle freigeben
; Ein:  [bp+6] Handle
; Aus:  -
; ---------------------------------------------------------------------------
F_R22_0B0C	label	far	; Linkname von AIL_release_driver_handle
AIL_release_driver_handle:
	push	bp	;@0b0c
	mov	bp,sp	;@0b0d
	push	ds	;@0b0f
	push	si	;@0b10
	push	di	;@0b11
	pushf	;@0b12
	cli	;@0b13
	mov	bx,[bp+6]	;@0b14
	cmp	bx,010h	;@0b17
	jnc	short @@exit	;@0b1a
	shl	bx,1	;@0b1c
	shl	bx,1	;@0b1e
	mov	word ptr cs:[bx+index_base],0	;@0b20
	mov	word ptr cs:[bx+index_base+2],0	;@0b27
	cmp	word ptr cs:[DP_registered],0	;@0b2e
	jz	short @@exit	;@0b34
	mov	ax,[bp+6]	;@0b36
	cmp	ax,cs:[DP_handle]	;@0b39
	jnz	short @@exit	;@0b3e
	mov	word ptr cs:[DP_registered],0	;@0b40
@@exit:
	POP_F		;@0b47
	pop	di	;@0b4e
	pop	si	;@0b4f
	pop	ds	;@0b50
	pop	bp	;@0b51
	retf	;@0b52
; ---------------------------------------------------------------------------
; AIL_describe_driver - Treiberbeschreibung holen
; Ein:  [bp+6] Handle
; Aus:  DX:AX = far-Zeiger auf drvr_desc (0 = Fehler)
; Uebergibt dem Treiber zusaetzlich die Adresse von AIL_interrupt_divisor.
; ---------------------------------------------------------------------------
F_R22_0B53	label	far	; Linkname von AIL_describe_driver
AIL_describe_driver:
	push	bp	;@0b53
	mov	bp,sp	;@0b54
	push	ax	;@0b56
	push	bp	;@0b57
	mov	bp,sp	;@0b58
	db	0c7h,046h,2	; mov word [bp+0x2],0x108d  (Relokation)	;@0b5a push SEG AIL_interrupt_divisor (8086-Form mit Relokation)
	dw	SEG022_TEXT	;@0b5a
	pop	bp	;@0b5f
	PUSH_C	AIL_interrupt_divisor_ofs	;@0b60
	push	word ptr [bp+6]	;@0b6a
	mov	ax,AIL_DESC_DRVR	;@0b6d
	push	cs	;@0b70
	call	near ptr call_driver	;@0b71
	add	sp,6	;@0b74
	pop	bp	;@0b77
	retf	;@0b78
; ---------------------------------------------------------------------------
; AIL_detect_device - Geraet an den Ein/Ausgabe-Werten suchen (Treiberstub)
; Ein:  [bp+6] Handle, IO, IRQ, DMA, DRQ
; Aus:  AX != 0 gefunden
; ---------------------------------------------------------------------------
F_R22_0B79	label	far	; Linkname von AIL_detect_device
AIL_detect_device:
	mov	ax,AIL_DET_DEV	;@0b79
	jmp	near ptr call_driver	;@0b7c
; ---------------------------------------------------------------------------
; AIL_init_driver - Treiber initialisieren
; Ein:  [bp+6] Handle, [bp+8] IO, [bp+0ah] IRQ, [bp+0ch] DMA, [bp+0eh] DRQ
; Aus:  -
; Will der Treiber bedient werden (svc_rate != -1) und hat er AIL_SERVE_DRVR,
; bekommt er einen Zeitgeber mit svc_rate Hz, der nach der Initialisierung
; startet.
; ---------------------------------------------------------------------------
F_R22_0B7F	label	far	; Linkname von AIL_init_driver
AIL_init_driver:
	push	bp	;@0b7f
	mov	bp,sp	;@0b80
	push	ds	;@0b82
	push	si	;@0b83
	push	di	;@0b84
	pushf	;@0b85
	cli	;@0b86
	cmp	word ptr [bp+6],010h	;@0b87
	jc	short @@valid	;@0b8b
	jmp	near ptr @@return	;@0b8d
@@valid:
	mov	word ptr cs:[timer_handle],0ffffh	;@0b90
	push	word ptr [bp+6]	;@0b97
	push	cs	;@0b9a
	call	near ptr AIL_describe_driver	;@0b9b
	add	sp,2	;@0b9e
	mov	es,dx	;@0ba1
	mov	di,ax	;@0ba3
	mov	si,es:[di+014h]	;@0ba5 svc_rate
	cmp	si,-1	;@0ba9
	jz	short @@do_init	;@0bac
	mov	ax,AIL_SERVE_DRVR	;@0bae
	mov	bx,[bp+6]	;@0bb1
	push	cs	;@0bb4
	call	near ptr find_proc	;@0bb5
	mov	bx,ax	;@0bb8
	or	bx,dx	;@0bba
	jz	short @@do_init	;@0bbc
	mov	es,dx	;@0bbe
	mov	bx,ax	;@0bc0
	push	es	;@0bc2
	push	bx	;@0bc3
	push	cs	;@0bc4
	call	near ptr AIL_register_timer	;@0bc5 AIL_register_timer(serve_driver)
	add	sp,4	;@0bc8
	mov	bx,[bp+6]	;@0bcb
	shl	bx,1	;@0bce
	mov	cs:[bx+assigned_timer],ax	;@0bd0
	mov	cs:[timer_handle],ax	;@0bd5
	PUSH_C	0	;@0bd9
	push	si	;@0be3
	push	ax	;@0be4
	push	cs	;@0be5
	call	near ptr AIL_set_timer_frequency	;@0be6 AIL_set_timer_frequency(timer, svc_rate)
	add	sp,6	;@0be9
@@do_init:
	push	word ptr [bp+0eh]	;@0bec
	push	word ptr [bp+0ch]	;@0bef
	push	word ptr [bp+0ah]	;@0bf2
	push	word ptr [bp+8]	;@0bf5
	push	word ptr [bp+6]	;@0bf8
	mov	ax,AIL_INIT_DRVR	;@0bfb
	push	cs	;@0bfe
	call	near ptr call_driver	;@0bff
	add	sp,0ah	;@0c02
	mov	bx,[bp+6]	;@0c05
	shl	bx,1	;@0c08
	mov	word ptr cs:[bx+driver_active],1	;@0c0a
	cmp	word ptr cs:[timer_handle],-1	;@0c11
	jz	short @@return	;@0c17
	push	word ptr cs:[timer_handle]	;@0c19
	push	cs	;@0c1e
	call	near ptr AIL_start_timer	;@0c1f
	add	sp,2	;@0c22
@@return:
	POP_F		;@0c25
	pop	di	;@0c2c
	pop	si	;@0c2d
	pop	ds	;@0c2e
	pop	bp	;@0c2f
	retf	;@0c30
; ---------------------------------------------------------------------------
; AIL_shutdown_driver - Treiber beenden
; Ein:  [sp+4] Handle, dahinter signoff_msg (far)
; Aus:  -
; Nur wenn initialisiert: Zeitgeber freigeben, dann AIL_SHUTDOWN_DRVR im
; Treiber (Sprung nach call_driver).
; ---------------------------------------------------------------------------
F_R22_0C31	label	far	; Linkname von AIL_shutdown_driver
AIL_shutdown_driver:
	mov	bx,sp	;@0c31
	mov	bx,ss:[bx+4]	;@0c33
	cmp	bx,010h	;@0c37
	jnc	short @@exit	;@0c3a
	shl	bx,1	;@0c3c
	mov	dx,0	;@0c3e
	xchg	dx,cs:[bx+driver_active]	;@0c41
	cmp	dx,0	;@0c46
	jz	short @@exit	;@0c49
	mov	dx,cs:[bx+assigned_timer]	;@0c4b
	cmp	dx,-1	;@0c50
	jz	short @@shut_down	;@0c53
	push	dx	;@0c55
	push	cs	;@0c56
	call	near ptr AIL_release_timer_handle	;@0c57
	add	sp,2	;@0c5a
@@shut_down:
	mov	ax,AIL_SHUTDOWN_DRVR	;@0c5d
	jmp	near ptr call_driver	;@0c60
@@exit:
	retf	;@0c63
; ===========================================================================
; Treiberfunktionen (Performance services): AX = Nummer, weiter in
; call_driver. Parameter wie in AIL.H, erster Parameter das Treiberhandle.
; ===========================================================================
F_R22_0C64	label	far	; Linkname von AIL_index_VOC_block
AIL_index_VOC_block:
	mov	ax,AIL_INDEX_VOC_BLK	;@0c64
	jmp	near ptr call_driver	;@0c67
F_R22_0C6A	label	far	; Linkname von AIL_register_sound_buffer
AIL_register_sound_buffer:
	mov	ax,AIL_REG_SND_BUFF	;@0c6a
	jmp	near ptr call_driver	;@0c6d
F_R22_0C70	label	far	; Linkname von AIL_format_sound_buffer
AIL_format_sound_buffer:
	mov	ax,AIL_F_SND_BUFF	;@0c70
	jmp	near ptr call_driver	;@0c73
F_R22_0C76	label	far	; Linkname von AIL_sound_buffer_status
AIL_sound_buffer_status:
	mov	ax,AIL_SND_BUFF_STAT	;@0c76
	jmp	near ptr call_driver	;@0c79
AIL_play_VOC_file:			; (ohne Linknamen, im Spiel unbenutzt)
	mov	ax,AIL_P_VOC_FILE	;@0c7c
	jmp	near ptr call_driver	;@0c7f
AIL_format_VOC_file:			; (ohne Linknamen, im Spiel unbenutzt)
	mov	ax,AIL_F_VOC_FILE	;@0c82
	jmp	near ptr call_driver	;@0c85
AIL_VOC_playback_status:			; (ohne Linknamen, im Spiel unbenutzt)
	mov	ax,AIL_VOC_PB_STAT	;@0c88
	jmp	near ptr call_driver	;@0c8b
F_R22_0C8E	label	far	; Linkname von AIL_start_digital_playback
AIL_start_digital_playback:
	mov	ax,AIL_START_D_PB	;@0c8e
	jmp	near ptr call_driver	;@0c91
AIL_stop_digital_playback:			; (ohne Linknamen, im Spiel unbenutzt)
	mov	ax,AIL_STOP_D_PB	;@0c94
	jmp	near ptr call_driver	;@0c97
F_R22_0C9A	label	far	; Linkname von AIL_pause_digital_playback
AIL_pause_digital_playback:
	mov	ax,AIL_PAUSE_D_PB	;@0c9a
	jmp	near ptr call_driver	;@0c9d
F_R22_0CA0	label	far	; Linkname von AIL_resume_digital_playback
AIL_resume_digital_playback:
	mov	ax,AIL_RESUME_D_PB	;@0ca0
	jmp	near ptr call_driver	;@0ca3
F_R22_0CA6	label	far	; Linkname von AIL_set_digital_playback_volume
AIL_set_digital_playback_volume:
	mov	ax,AIL_SET_D_PB_VOL	;@0ca6
	jmp	near ptr call_driver	;@0ca9
AIL_digital_playback_volume:			; (ohne Linknamen, im Spiel unbenutzt)
	mov	ax,AIL_D_PB_VOL	;@0cac
	jmp	near ptr call_driver	;@0caf
AIL_set_digital_playback_panpot:			; (ohne Linknamen, im Spiel unbenutzt)
	mov	ax,AIL_SET_D_PB_PAN	;@0cb2
	jmp	near ptr call_driver	;@0cb5
AIL_digital_playback_panpot:			; (ohne Linknamen, im Spiel unbenutzt)
	mov	ax,AIL_D_PB_PAN	;@0cb8
	jmp	near ptr call_driver	;@0cbb
F_R22_0CBE	label	far	; Linkname von AIL_state_table_size
AIL_state_table_size:
	mov	ax,AIL_STATE_TAB_SIZE	;@0cbe
	jmp	near ptr call_driver	;@0cc1
F_R22_0CC4	label	far	; Linkname von AIL_register_sequence
AIL_register_sequence:
	mov	ax,AIL_REG_SEQ	;@0cc4
	jmp	near ptr call_driver	;@0cc7
F_R22_0CCA	label	far	; Linkname von AIL_release_sequence_handle
AIL_release_sequence_handle:
	mov	ax,AIL_REL_SEQ_HND	;@0cca
	jmp	near ptr call_driver	;@0ccd
F_R22_0CD0	label	far	; Linkname von AIL_default_timbre_cache_size
AIL_default_timbre_cache_size:
	mov	ax,AIL_T_CACHE_SIZE	;@0cd0
	jmp	near ptr call_driver	;@0cd3
F_R22_0CD6	label	far	; Linkname von AIL_define_timbre_cache
AIL_define_timbre_cache:
	mov	ax,AIL_DEFINE_T_CACHE	;@0cd6
	jmp	near ptr call_driver	;@0cd9
F_R22_0CDC	label	far	; Linkname von AIL_timbre_request
AIL_timbre_request:
	mov	ax,AIL_T_REQ	;@0cdc
	jmp	near ptr call_driver	;@0cdf
F_R22_0CE2	label	far	; Linkname von AIL_install_timbre
AIL_install_timbre:
	mov	ax,AIL_INSTALL_T	;@0ce2
	jmp	near ptr call_driver	;@0ce5
AIL_protect_timbre:			; (ohne Linknamen, im Spiel unbenutzt)
	mov	ax,AIL_PROTECT_T	;@0ce8
	jmp	near ptr call_driver	;@0ceb
AIL_unprotect_timbre:			; (ohne Linknamen, im Spiel unbenutzt)
	mov	ax,AIL_UNPROTECT_T	;@0cee
	jmp	near ptr call_driver	;@0cf1
AIL_timbre_status:			; (ohne Linknamen, im Spiel unbenutzt)
	mov	ax,AIL_T_STATUS	;@0cf4
	jmp	near ptr call_driver	;@0cf7
F_R22_0CFA	label	far	; Linkname von AIL_start_sequence
AIL_start_sequence:
	mov	ax,AIL_START_SEQ	;@0cfa
	jmp	near ptr call_driver	;@0cfd
F_R22_0D00	label	far	; Linkname von AIL_stop_sequence
AIL_stop_sequence:
	mov	ax,AIL_STOP_SEQ	;@0d00
	jmp	near ptr call_driver	;@0d03
F_R22_0D06	label	far	; Linkname von AIL_resume_sequence
AIL_resume_sequence:
	mov	ax,AIL_RESUME_SEQ	;@0d06
	jmp	near ptr call_driver	;@0d09
F_R22_0D0C	label	far	; Linkname von AIL_sequence_status
AIL_sequence_status:
	mov	ax,AIL_SEQ_STAT	;@0d0c
	jmp	near ptr call_driver	;@0d0f
F_R22_0D12	label	far	; Linkname von AIL_relative_volume
AIL_relative_volume:
	mov	ax,AIL_REL_VOL	;@0d12
	jmp	near ptr call_driver	;@0d15
AIL_relative_tempo:			; (ohne Linknamen, im Spiel unbenutzt)
	mov	ax,AIL_REL_TEMPO	;@0d18
	jmp	near ptr call_driver	;@0d1b
F_R22_0D1E	label	far	; Linkname von AIL_set_relative_volume
AIL_set_relative_volume:
	mov	ax,AIL_SET_REL_VOL	;@0d1e
	jmp	near ptr call_driver	;@0d21
AIL_set_relative_tempo:			; (ohne Linknamen, im Spiel unbenutzt)
	mov	ax,AIL_SET_REL_TEMPO	;@0d24
	jmp	near ptr call_driver	;@0d27
AIL_beat_count:			; (ohne Linknamen, im Spiel unbenutzt)
	mov	ax,AIL_BEAT_CNT	;@0d2a
	jmp	near ptr call_driver	;@0d2d
AIL_measure_count:			; (ohne Linknamen, im Spiel unbenutzt)
	mov	ax,AIL_BAR_CNT	;@0d30
	jmp	near ptr call_driver	;@0d33
AIL_branch_index:			; (ohne Linknamen, im Spiel unbenutzt)
	mov	ax,AIL_BRA_INDEX	;@0d36
	jmp	near ptr call_driver	;@0d39
AIL_controller_value:			; (ohne Linknamen, im Spiel unbenutzt)
	mov	ax,AIL_CON_VAL	;@0d3c
	jmp	near ptr call_driver	;@0d3f
AIL_set_controller_value:			; (ohne Linknamen, im Spiel unbenutzt)
	mov	ax,AIL_SET_CON_VAL	;@0d42
	jmp	near ptr call_driver	;@0d45
AIL_channel_notes:			; (ohne Linknamen, im Spiel unbenutzt)
	mov	ax,AIL_CHAN_NOTES	;@0d48
	jmp	near ptr call_driver	;@0d4b
AIL_send_channel_voice_message:			; (ohne Linknamen, im Spiel unbenutzt)
	mov	ax,AIL_SEND_CV_MSG	;@0d4e
	jmp	near ptr call_driver	;@0d51
AIL_send_sysex_message:			; (ohne Linknamen, im Spiel unbenutzt)
	mov	ax,AIL_SEND_SYSEX_MSG	;@0d54
	jmp	near ptr call_driver	;@0d57
AIL_write_display:			; (ohne Linknamen, im Spiel unbenutzt)
	mov	ax,AIL_WRITE_DISP	;@0d5a
	jmp	near ptr call_driver	;@0d5d
F_R22_0D60	label	far	; Linkname von AIL_install_callback
AIL_install_callback:
	mov	ax,AIL_INSTALL_CB	;@0d60
	jmp	near ptr call_driver	;@0d63
F_R22_0D66	label	far	; Linkname von AIL_cancel_callback
AIL_cancel_callback:
	mov	ax,AIL_CANCEL_CB	;@0d66
	jmp	near ptr call_driver	;@0d69
AIL_lock_channel:			; (ohne Linknamen, im Spiel unbenutzt)
	mov	ax,AIL_LOCK_CHAN	;@0d6c
	jmp	near ptr call_driver	;@0d6f
AIL_map_sequence_channel:			; (ohne Linknamen, im Spiel unbenutzt)
	mov	ax,AIL_MAP_SEQ_CHAN	;@0d72
	jmp	near ptr call_driver	;@0d75
AIL_release_channel:			; (ohne Linknamen, im Spiel unbenutzt)
	mov	ax,AIL_RELEASE_CHAN	;@0d78
	jmp	near ptr call_driver	;@0d7b
AIL_true_sequence_channel:			; (ohne Linknamen, im Spiel unbenutzt)
	mov	ax,AIL_TRUE_SEQ_CHAN	;@0d7e
	jmp	near ptr call_driver	;@0d81
; ===========================================================================
; DIGPAK-Schale: Daten (AIL.ASM "DIGPAK Integrated Driver Interface")
; ===========================================================================
	dw	AIL_DESC_DRVR,13e0h	;@0d84 DP_IDI_index: DP_describe_driver
	dw	AIL_DET_DEV,1460h	;@0d88 DP_detect_device
	dw	AIL_INIT_DRVR,146eh	;@0d8c DP_init_driver
	dw	AIL_SHUTDOWN_DRVR,14d3h	;@0d90 DP_shutdown_driver
	dw	AIL_P_VOC_FILE,16a0h	;@0d94 DP_play_VOC_file
	dw	AIL_START_D_PB,1874h	;@0d98 DP_start_d_pb
	dw	AIL_STOP_D_PB,18bch	;@0d9c DP_stop_d_pb
	dw	AIL_VOC_PB_STAT,1900h	;@0da0 DP_get_VOC_status
	dw	AIL_INDEX_VOC_BLK,1506h	;@0da4 DP_index_VOC_blk
	dw	AIL_REG_SND_BUFF,162ch	;@0da8 DP_register_sb
	dw	AIL_SND_BUFF_STAT,168bh	;@0dac DP_get_sb_status
	dw	AIL_F_VOC_FILE,1746h	;@0db0 DP_format_VOC_file
	dw	AIL_F_SND_BUFF,1837h	;@0db4 DP_format_sb
	dw	-1			;@0db8 Ende der Tabelle
	dw	210		;@0dba DP_DDT: min_API_version 2.10
	dw	2		;@0dbc drvr_type 2: Sound-Blaster-DSP-Nachbildung
	db	'VOC',0		;@0dbe data_suffix
	dw	DP_name		;@0dc2 dev_names (Offset)
	dw	0		;@0dc4 DP_devname_s (Segment, = CS)
	dw	-1,-1,-1,-1	;@0dc6 def_IO, def_IRQ, def_DMA, def_DRQ (DIGPAK regelt sie selbst)
	dw	0		;@0dce DP_uses_INT8 (svc_rate)
	dw	0		;@0dd0 dsp_size: keine Anzeige
	ZEROS	80	;@0dd2 DP_name (db 80 dup)
	dw	0		;@0e22 call_off
	dw	0		;@0e24 call_seg
	dw	0		;@0e26 DP_handle
	dw	0		;@0e28 DP_registered
	dw	0		;@0e2a DP_installed
	dd	0		;@0e2c DP_base
	dw	0		;@0e30 DP_attribs
	ZEROS	6*2	;@0e32 DPS (dw 6 dup)
	ZEROS	6*2	;@0e3e DPT (dw 6 dup)
	ZEROS	2*2	;@0e4a buff_data_o (dw 2 dup)
	ZEROS	2*2	;@0e4e buff_data_s (dw 2 dup)
	ZEROS	2*2	;@0e52 buff_len_l (dw 2 dup)
	ZEROS	2*2	;@0e56 buff_len_h (dw 2 dup)
	ZEROS	2*2	;@0e5a buff_sample (dw 2 dup)
	ZEROS	2*2	;@0e5e buff_status (dw 2 dup)
	dw	0		;@0e62 buffer_mode
	dw	0		;@0e64 DAC_status
	dw	0		;@0e66 current_buffer
	dd	0		;@0e68 block_ptr
	dd	0		;@0e6c loop_ptr
	dw	0		;@0e70 loop_cnt
	dd	0		;@0e72 DMA_ptr
	dw	0		;@0e76 DMA_len_l
	dw	0		;@0e78 DMA_len_h
	dw	0		;@0e7a packing
	dw	0		;@0e7c playing_flag
	dw	0		;@0e7e DP_timer
	dw	0		;@0e80 AIL_hooked
; ---------------------------------------------------------------------------
; AIL_unhook - int 8 fuer eine DIGPAK-Ausgabe freigeben
; Ein:  -
; Aus:  -
; Braucht der DIGPAK-Treiber den Zeitgeber (USES_TICK) und sind AIL-
; Zeitgeber aktiv und eingehaengt: alten int 8 herstellen, AIL_hooked = 0.
; Der Zweig ab 0eabh (DP_SHARE_TIMER) ist nur mit DIGPAK_SHARE erreichbar
; und hier tot.
; ---------------------------------------------------------------------------
AIL_unhook:
	push	ds	;@0e82
	push	si	;@0e83
	push	di	;@0e84
	test	word ptr cs:[DP_attribs],8	;@0e85
	jz	short @@exit	;@0e8c
	cmp	word ptr cs:[active_timers],0	;@0e8e
	jz	short @@exit	;@0e94
	cmp	word ptr cs:[AIL_hooked],0	;@0e96
	jz	short @@exit	;@0e9c
	push	cs	;@0e9e
	call	near ptr unhook_timer_process	;@0e9f
	mov	word ptr cs:[AIL_hooked],0	;@0ea2
	jmp	short @@exit	;@0ea9
	mov	ax,0693h	;@0eab
	mov	dx,cs:[PIT_divisor]	;@0eae
	int	066h	;@0eb3
@@exit:
	pop	di	;@0eb5
	pop	si	;@0eb6
	pop	ds	;@0eb7
	retf	;@0eb8
; ---------------------------------------------------------------------------
; AIL_hook - int 8 nach einer DIGPAK-Ausgabe wieder uebernehmen
; Ein:  -
; Aus:  -
; Gegenstueck zu AIL_unhook: PIT auf AIL_interrupt_divisor, int 8 auf
; API_timer, AIL_hooked = 1.
; ---------------------------------------------------------------------------
F_R22_0EB9	label	far	; Linkname von AIL_hook
AIL_hook:
	push	ds	;@0eb9
	push	si	;@0eba
	push	di	;@0ebb
	test	word ptr cs:[DP_attribs],8	;@0ebc
	jz	short @@exit	;@0ec3
	cmp	word ptr cs:[active_timers],0	;@0ec5
	jz	short @@exit	;@0ecb
	cmp	word ptr cs:[AIL_hooked],0	;@0ecd
	jnz	short @@exit	;@0ed3
	push	cs	;@0ed5
	call	near ptr AIL_interrupt_divisor	;@0ed6
	push	ax	;@0ed9
	push	cs	;@0eda
	call	near ptr set_PIT_divisor	;@0edb
	add	sp,2	;@0ede
	push	cs	;@0ee1
	call	near ptr hook_timer_process	;@0ee2
	mov	word ptr cs:[AIL_hooked],1	;@0ee5
@@exit:
	pop	di	;@0eec
	pop	si	;@0eed
	pop	ds	;@0eee
	retf	;@0eef
; ---------------------------------------------------------------------------
; sub_ptr - lineare Differenz zweier far-Zeiger (unbenutzt)
; Ein:  [bp+6] Off1, [bp+8] Seg1, [bp+0ah] Off2, [bp+0ch] Seg2
; Aus:  DX:AX = Zeiger 2 - Zeiger 1
; ---------------------------------------------------------------------------
F_R22_0EF0	label	far	; Linkname von sub_ptr
sub_ptr:
	push	bp	;@0ef0
	mov	bp,sp	;@0ef1
	push	ds	;@0ef3
	push	si	;@0ef4
	push	di	;@0ef5
	mov	ax,[bp+0ch]	;@0ef6
	mov	dx,0	;@0ef9
	shl	ax,1	;@0efc
	rcl	dx,1	;@0efe
	shl	ax,1	;@0f00
	rcl	dx,1	;@0f02
	shl	ax,1	;@0f04
	rcl	dx,1	;@0f06
	shl	ax,1	;@0f08
	rcl	dx,1	;@0f0a
	add	ax,[bp+0ah]	;@0f0c
	adc	dx,0	;@0f0f
	mov	bx,[bp+8]	;@0f12
	mov	cx,0	;@0f15
	shl	bx,1	;@0f18
	rcl	cx,1	;@0f1a
	shl	bx,1	;@0f1c
	rcl	cx,1	;@0f1e
	shl	bx,1	;@0f20
	rcl	cx,1	;@0f22
	shl	bx,1	;@0f24
	rcl	cx,1	;@0f26
	add	bx,[bp+6]	;@0f28
	adc	cx,0	;@0f2b
	sub	ax,bx	;@0f2e
	sbb	dx,cx	;@0f30
	pop	di	;@0f32
	pop	si	;@0f33
	pop	ds	;@0f34
	pop	bp	;@0f35
	retf	;@0f36
; ---------------------------------------------------------------------------
; block_type - Typ des aktuellen VOC-Blocks
; Aus:  AX = Typbyte an block_ptr
; ---------------------------------------------------------------------------
F_R22_0F37	label	far	; Linkname von block_type
block_type:
	push	ds	;@0f37
	push	si	;@0f38
	push	di	;@0f39
	lds	si,cs:[block_ptr]	;@0f3a
	lodsb	;@0f3f
	mov	ah,0	;@0f40
	pop	di	;@0f42
	pop	si	;@0f43
	pop	ds	;@0f44
	retf	;@0f45
; ---------------------------------------------------------------------------
; marker_num - Markennummer des aktuellen VOC-Blocks
; Aus:  AX = Nummer (Blocktyp 4), sonst -1
; ---------------------------------------------------------------------------
F_R22_0F46	label	far	; Linkname von marker_num
marker_num:
	push	ds	;@0f46
	push	si	;@0f47
	push	di	;@0f48
	lds	si,cs:[block_ptr]	;@0f49
	cmp	byte ptr [si],4	;@0f4e
	mov	ax,0ffffh	;@0f51
	jnz	short @@exit	;@0f54
	mov	ax,[si+4]	;@0f56
@@exit:
	pop	di	;@0f59
	pop	si	;@0f5a
	pop	ds	;@0f5b
	retf	;@0f5c
; ---------------------------------------------------------------------------
; get_sample_rate - Abtastrate aus der Sound-Blaster-Zeitkonstante
; Ein:  [bp+6] Zeitkonstante (Byte)
; Aus:  DX:AX = 1000000 / (256 - Zeitkonstante) Hz
; ---------------------------------------------------------------------------
F_R22_0F5D	label	far	; Linkname von get_sample_rate
get_sample_rate:
	push	bp	;@0f5d
	mov	bp,sp	;@0f5e
	push	ds	;@0f60
	push	si	;@0f61
	push	di	;@0f62
	mov	bx,[bp+6]	;@0f63
	mov	bh,0	;@0f66
	mov	ax,0100h	;@0f68
	sub	ax,bx	;@0f6b
	PUSH_C	0	;@0f6d
	push	ax	;@0f77
	PUSH_C	0fh	;@0f78
	PUSH_C	04240h	;@0f82
	push	cs	;@0f8c
	call	near ptr ul_divide	;@0f8d
	add	sp,8	;@0f90
	pop	di	;@0f93
	pop	si	;@0f94
	pop	ds	;@0f95
	pop	bp	;@0f96
	retf	;@0f97
; ---------------------------------------------------------------------------
; DP_preformat - Daten fuer den DIGPAK-Treiber vorformatieren
; Ein:  [bp+6] Abtastrate, [bp+8] Daten (far), [bp+0ch] Laenge (dd)
; Aus:  -
; Nur wenn der Treiber die Daten aendert (MODIFIES_DATA): in Bloecken zu
; BLK_SIZE MassageAudio (int 66h) ueber DPT.
; ---------------------------------------------------------------------------
F_R22_0F98	label	far	; Linkname von DP_preformat
DP_preformat:
	push	bp	;@0f98
	mov	bp,sp	;@0f99
	push	ds	;@0f9b
	push	si	;@0f9c
	push	di	;@0f9d
	test	word ptr cs:[DP_attribs],2	;@0f9e
	jnz	short @@do_it	;@0fa5
	jmp	near ptr @@exit	;@0fa7
@@do_it:
	mov	word ptr cs:[DPT+sndstat],playing_flag	;@0faa
	mov	cs:[DPT+sndstat+2],cs	;@0fb1
	mov	ax,[bp+6]	;@0fb6
	mov	cs:[DPT+sndfreq],ax	;@0fb9
	lds	si,[bp+8]	;@0fbd
@@while_gt_BLK:
	mov	cs:[DPT+sndptr],si	;@0fc0
	mov	cs:[DPT+sndptr+2],ds	;@0fc5
	cmp	word ptr [bp+0eh],0	;@0fca
	ja	short @@gt_BLK	;@0fce
	cmp	word ptr [bp+0ch],08000h	;@0fd0
	jna	short @@remainder	;@0fd5
@@gt_BLK:
	mov	word ptr cs:[DPT+sndlen],08000h	;@0fd7
	push	ds	;@0fde
	push	si	;@0fdf
	push	cs	;@0fe0
	pop	ds	;@0fe1
	mov	si,DPT	;@0fe2
	mov	ax,DP_MASSAGE_AUDIO	;@0fe5
	int	066h	;@0fe8
	pop	si	;@0fea
	pop	ds	;@0feb
	ADD_PTR	08000h,0,ds,si	;@0fec
	sub	word ptr [bp+0ch],08000h	;@1027
	sbb	word ptr [bp+0eh],0	;@102c
	jmp	short @@while_gt_BLK	;@1030
@@remainder:
	mov	cx,[bp+0ch]	;@1032
	jcxz	short @@exit	;@1035
	mov	cs:[DPT+sndlen],cx	;@1037
	push	cs	;@103c
	pop	ds	;@103d
	mov	si,DPT	;@103e
	mov	ax,DP_MASSAGE_AUDIO	;@1041
	int	066h	;@1044
@@exit:
	pop	di	;@1046
	pop	si	;@1047
	pop	ds	;@1048
	pop	bp	;@1049
	retf	;@104a
; ---------------------------------------------------------------------------
; CB_set_vect - Rueckruf am Blockende beim DIGPAK-Treiber setzen
; Ein:  [bp+6] Rueckruf (far; 0:0 = keiner)
; Aus:  -   (int 66h, DP_SET_CALLBACK)
; ---------------------------------------------------------------------------
F_R22_104B	label	far	; Linkname von CB_set_vect
CB_set_vect:
	push	bp	;@104b
	mov	bp,sp	;@104c
	push	ds	;@104e
	push	si	;@104f
	push	di	;@1050
	mov	ax,cs	;@1051
	mov	ds,ax	;@1053
	mov	bx,[bp+6]	;@1055
	mov	dx,[bp+8]	;@1058
	mov	ax,DP_SET_CALLBACK	;@105b
	int	066h	;@105e
	pop	di	;@1060
	pop	si	;@1061
	pop	ds	;@1062
	pop	bp	;@1063
	retf	;@1064
; ---------------------------------------------------------------------------
; CB_play_VOC - Rueckruf am Blockende bei VOC-Ausgabe
; Naechstes Stueck senden oder zum naechsten VOC-Block weitergehen.
; ---------------------------------------------------------------------------
CB_play_VOC:
	cld	;@1065
	push	cs	;@1066
	call	near ptr AIL_hook	;@1067
	mov	ax,cs:[DMA_len_l]	;@106a
	or	ax,cs:[DMA_len_h]	;@106e
	jz	short @@end_of_block	;@1073
	push	cs	;@1075
	call	near ptr xfer_chunk	;@1076
	jmp	short @@exit	;@1079
@@end_of_block:
	push	cs	;@107b
	call	near ptr next_block	;@107c
	push	cs	;@107f
	call	near ptr process_block	;@1080
@@exit:
	retf	;@1083
; ---------------------------------------------------------------------------
; CB_play_buffer - Rueckruf am Blockende bei Doppelpuffer-Ausgabe
; Naechstes Stueck senden oder den Puffer als fertig melden und den
; naechsten unbespielten Puffer starten.
; ---------------------------------------------------------------------------
CB_play_buffer:
	cld	;@1084
	push	cs	;@1085
	call	near ptr AIL_hook	;@1086
	mov	ax,cs:[DMA_len_l]	;@1089
	or	ax,cs:[DMA_len_h]	;@108d
	jz	short @@end_of_block	;@1092
	push	cs	;@1094
	call	near ptr xfer_chunk	;@1095
	jmp	short @@exit	;@1098
@@end_of_block:
	mov	bx,cs:[current_buffer]	;@109a
	mov	word ptr cs:[bx+buff_status],3	;@109f
	push	cs	;@10a6
	call	near ptr next_buffer	;@10a7
	cmp	ax,0ffffh	;@10aa
	jz	short @@exit	;@10ad
	push	ax	;@10af
	push	cs	;@10b0
	call	near ptr process_buffer	;@10b1
	add	sp,2	;@10b4
@@exit:
	retf	;@10b7
; ---------------------------------------------------------------------------
; next_block - block_ptr auf den naechsten VOC-Block (Laenge 24 Bit)
; ---------------------------------------------------------------------------
F_R22_10B8	label	far	; Linkname von next_block
next_block:
	push	ds	;@10b8
	push	si	;@10b9
	push	di	;@10ba
	lds	si,cs:[block_ptr]	;@10bb
	inc	si	;@10c0
	lodsw	;@10c1
	mov	dl,[si]	;@10c2
	mov	dh,0	;@10c4
	inc	si	;@10c6
	ADD_PTR	ax,dx,ds,si	;@10c7
	mov	cs:[block_ptr],si	;@10ff
	mov	cs:[block_ptr+2],ds	;@1104
	pop	di	;@1109
	pop	si	;@110a
	pop	ds	;@110b
	retf	;@110c
; ---------------------------------------------------------------------------
; next_buffer - angemeldeten, unbespielten Puffer suchen
; Aus:  AX = 0 oder 1; -1 = keiner (DAC_status = DAC_DONE)
; ---------------------------------------------------------------------------
F_R22_110D	label	far	; Linkname von next_buffer
next_buffer:
	push	ds	;@110d
	push	si	;@110e
	push	di	;@110f
	mov	ax,0	;@1110
	cmp	word ptr cs:[buff_status],0	;@1113
	jz	short @@return	;@1119
	mov	ax,1	;@111b
	cmp	word ptr cs:[buff_status+2],0	;@111e
	jz	short @@return	;@1124
	mov	word ptr cs:[DAC_status],3	;@1126
	mov	ax,0ffffh	;@112d
@@return:
	pop	di	;@1130
	pop	si	;@1131
	pop	ds	;@1132
	retf	;@1133
; ---------------------------------------------------------------------------
; process_block - aktuellen VOC-Block abarbeiten (auch aus dem Rueckruf)
; Typ 0/4: Ende, 1: neue Sprachdaten (Rate setzen), 2: Fortsetzung,
; 6/7: Wiederholung Anfang/Ende, sonst ueberspringen.
; ---------------------------------------------------------------------------
F_R22_1134	label	far	; Linkname von process_block
process_block:
	push	ds	;@1134
	push	si	;@1135
	push	di	;@1136
@@do_block:
	push	cs	;@1137
	call	near ptr block_type	;@1138
	cmp	ax,0	;@113b
	jz	short @@terminate	;@113e
	cmp	ax,1	;@1140
	jnz	short @@chk_cont	;@1143
	jmp	near ptr @@new_voice	;@1145
@@chk_cont:
	cmp	ax,2	;@1148
	jz	short @@cont_voice	;@114b
	cmp	ax,4	;@114d
	jz	short @@terminate	;@1150
	cmp	ax,6	;@1152
	jz	short @@rept_loop	;@1155
	cmp	ax,7	;@1157
	jz	short @@end_loop	;@115a
	jmp	short @@skip_block	;@115c
@@terminate:
	mov	word ptr cs:[DAC_status],3	;@115e
	jmp	near ptr @@exit	;@1165
@@skip_block:
	push	cs	;@1168
	call	near ptr next_block	;@1169
	jmp	short @@do_block	;@116c
@@rept_loop:
	lds	si,cs:[block_ptr]	;@116e
	mov	ax,[si+4]	;@1173
	mov	cs:[loop_cnt],ax	;@1176
	push	cs	;@117a
	call	near ptr next_block	;@117b
	lds	si,cs:[block_ptr]	;@117e
	mov	cs:[loop_ptr],si	;@1183
	mov	cs:[loop_ptr+2],ds	;@1188
	jmp	short @@do_block	;@118d
@@end_loop:
	cmp	word ptr cs:[loop_cnt],0	;@118f
	jz	@@skip_block	;@1195
	lds	si,cs:[loop_ptr]	;@1197
	mov	cs:[block_ptr],si	;@119c
	mov	cs:[block_ptr+2],ds	;@11a1
	cmp	word ptr cs:[loop_cnt],-1	;@11a6
	jz	@@do_block	;@11ac
	dec	word ptr cs:[loop_cnt]	;@11ae
	jmp	short @@do_block	;@11b3
@@cont_voice:
	lds	si,cs:[block_ptr]	;@11b5
	mov	ax,CB_play_VOC_ofs	;@11ba
	push	cs	;@11bd
	push	ax	;@11be
	push	cs	;@11bf
	call	near ptr CB_set_vect	;@11c0
	add	sp,4	;@11c3
	mov	ax,[si+1]	;@11c6
	mov	dl,[si+3]	;@11c9
	mov	dh,0	;@11cc
	ADD_PTR	4,0,ds,si	;@11ce
	push	dx	;@1208
	push	ax	;@1209
	push	ds	;@120a
	push	si	;@120b
	push	cs	;@120c
	call	near ptr DMA_transfer	;@120d
	add	sp,8	;@1210
	jmp	short @@exit	;@1213
@@new_voice:
	lds	si,cs:[block_ptr]	;@1215
	push	word ptr [si+4]	;@121a
	push	cs	;@121d
	call	near ptr get_sample_rate	;@121e
	add	sp,2	;@1221
	mov	cs:[DPS+sndfreq],ax	;@1224
	mov	ax,CB_play_VOC_ofs	;@1228
	push	cs	;@122b
	push	ax	;@122c
	push	cs	;@122d
	call	near ptr CB_set_vect	;@122e
	add	sp,4	;@1231
	mov	ax,[si+1]	;@1234
	mov	dl,[si+3]	;@1237
	mov	dh,0	;@123a
	sub	ax,2	;@123c
	sbb	dx,0	;@123f
	ADD_PTR	6,0,ds,si	;@1242
	push	dx	;@127c
	push	ax	;@127d
	push	ds	;@127e
	push	si	;@127f
	push	cs	;@1280
	call	near ptr DMA_transfer	;@1281
	add	sp,8	;@1284
@@exit:
	pop	di	;@1287
	pop	si	;@1288
	pop	ds	;@1289
	retf	;@128a
; ---------------------------------------------------------------------------
; process_buffer - Puffer abspielen
; Ein:  [bp+6] Puffernummer 0/1
; ---------------------------------------------------------------------------
F_R22_128B	label	far	; Linkname von process_buffer
process_buffer:
	push	bp	;@128b
	mov	bp,sp	;@128c
	push	ds	;@128e
	push	si	;@128f
	push	di	;@1290
	mov	si,[bp+6]	;@1291
	shl	si,1	;@1294
	mov	word ptr cs:[si+buff_status],2	;@1296
	mov	cs:[current_buffer],si	;@129d
	push	word ptr cs:[si+buff_sample]	;@12a2
	push	cs	;@12a7
	call	near ptr get_sample_rate	;@12a8
	add	sp,2	;@12ab
	mov	cs:[DPS+sndfreq],ax	;@12ae
	mov	ax,CB_play_buffer_ofs	;@12b2
	push	cs	;@12b5
	push	ax	;@12b6
	push	cs	;@12b7
	call	near ptr CB_set_vect	;@12b8
	add	sp,4	;@12bb
	push	word ptr cs:[si+buff_len_h]	;@12be
	push	word ptr cs:[si+buff_len_l]	;@12c3
	push	word ptr cs:[si+buff_data_s]	;@12c8
	push	word ptr cs:[si+buff_data_o]	;@12cd
	push	cs	;@12d2
	call	near ptr DMA_transfer	;@12d3
	add	sp,8	;@12d6
	pop	di	;@12d9
	pop	si	;@12da
	pop	ds	;@12db
	pop	bp	;@12dc
	retf	;@12dd
; ---------------------------------------------------------------------------
; DMA_transfer - nachgebildete DMA-Uebertragung beginnen
; Ein:  [bp+6] Daten (far), [bp+0ah] Laenge unten, [bp+0ch] Laenge oben
; Merkt Zeiger und Laenge und sendet das erste Stueck (xfer_chunk).
; ---------------------------------------------------------------------------
F_R22_12DE	label	far	; Linkname von DMA_transfer
DMA_transfer:
	push	bp	;@12de
	mov	bp,sp	;@12df
	push	ds	;@12e1
	push	si	;@12e2
	push	di	;@12e3
	lds	si,[bp+6]	;@12e4
	mov	ax,[bp+0ah]	;@12e7
	mov	dx,[bp+0ch]	;@12ea
	mov	cs:[DMA_ptr],si	;@12ed
	mov	cs:[DMA_ptr+2],ds	;@12f2
	mov	cs:[DMA_len_l],ax	;@12f7
	mov	cs:[DMA_len_h],dx	;@12fb
	push	cs	;@1300
	call	near ptr xfer_chunk	;@1301
	pop	di	;@1304
	pop	si	;@1305
	pop	ds	;@1306
	pop	bp	;@1307
	retf	;@1308
; ---------------------------------------------------------------------------
; xfer_chunk - naechstes Stueck (hoechstens BLK_SIZE) an DIGPAK senden
; Locals: [bp-2] blk_len.
; ---------------------------------------------------------------------------
F_R22_1309	label	far	; Linkname von xfer_chunk
xfer_chunk:
	push	bp	;@1309
	mov	bp,sp	;@130a
	sub	sp,2	;@130c
	push	ds	;@130f
	push	si	;@1310
	push	di	;@1311
	mov	ax,08000h	;@1312
	cmp	word ptr cs:[DMA_len_h],0	;@1315
	ja	short @@set_size	;@131b
	cmp	cs:[DMA_len_l],ax	;@131d
	ja	short @@set_size	;@1322
	mov	ax,cs:[DMA_len_l]	;@1324
@@set_size:
	mov	[bp-2],ax	;@1328
	mov	cs:[DPS+sndlen],ax	;@132b
	mov	word ptr cs:[DPS+sndstat],playing_flag	;@132f
	mov	cs:[DPS+sndstat+2],cs	;@1336
	lds	si,cs:[DMA_ptr]	;@133b
	FAR_TO_HUGE	ds,si	;@1340
	mov	cs:[DPS+sndptr],si	;@1357
	mov	cs:[DPS+sndptr+2],ds	;@135c
	ADD_PTR	[bp-2],0,ds,si	;@1361
	mov	cs:[DMA_ptr],si	;@139b
	mov	cs:[DMA_ptr+2],ds	;@13a0
	mov	ax,cs:[DMA_len_l]	;@13a5
	mov	dx,cs:[DMA_len_h]	;@13a9
	sub	ax,[bp-2]	;@13ae
	sbb	dx,0	;@13b1
	mov	cs:[DMA_len_h],dx	;@13b4
	mov	cs:[DMA_len_l],ax	;@13b9
	push	cs	;@13bd
	db	0e8h,0c1h,0fah	; call AIL_unhook	;@13be
	mov	ax,cs	;@13c1
	mov	ds,ax	;@13c3
	mov	si,DPS	;@13c5
	mov	ax,0688h	;@13c8
	test	word ptr cs:[DP_attribs],2	;@13cb
	jz	short @@DigPlay	;@13d2
	mov	ax,068bh	;@13d4
@@DigPlay:
	int	066h	;@13d7
	pop	di	;@13d9
	pop	si	;@13da
	pop	ds	;@13db
	mov	sp,bp	;@13dc
	pop	bp	;@13de
	retf	;@13df
; ===========================================================================
; DIGPAK-Schale: oeffentliche Funktionen (ueber DP_IDI_index)
; ===========================================================================
; ---------------------------------------------------------------------------
; DP_describe_driver - DIGPAK installieren (einmal) und DP_DDT liefern
; Ein:  [bp+6] Handle
; Aus:  DX:AX = DP_DDT; Geraetename aus dem DIGPAK-Kopf nach DP_name
; ---------------------------------------------------------------------------
F_R22_13E0	label	far	; Linkname von DP_describe_driver
DP_describe_driver:
	push	bp	;@13e0
	mov	bp,sp	;@13e1
	push	ds	;@13e3
	push	si	;@13e4
	push	di	;@13e5
	cmp	word ptr cs:[DP_installed],0	;@13e6
	jnz	short @@get_desc	;@13ec
	les	di,cs:[DP_base]	;@13ee
	mov	cs:[call_seg],es	;@13f3
	add	di,0100h	;@13f8
	mov	cs:[call_off],di	;@13fc
	call	dword ptr cs:[call_off]	;@1401 Installationsaufruf (DP_base + 100h)
	mov	ax,DP_AUDIO_CAPS	;@1406
	int	066h	;@1409
	mov	cs:[DP_attribs],ax	;@140b
	mov	ax,0ffffh	;@140f
	test	word ptr cs:[DP_attribs],8	;@1412
	jz	short @@set_int_svc	;@1419
	mov	ax,0	;@141b
@@set_int_svc:
	mov	cs:[DP_uses_INT8],ax	;@141e
	mov	word ptr cs:[DP_installed],1	;@1422
@@get_desc:
	les	di,cs:[DP_base]	;@1429
	mov	si,0	;@142e
	mov	cx,04eh	;@1431
@@copy_name:
	mov	al,es:[di+0ch]	;@1434
	cmp	al,020h	;@1438
	jc	short @@end_string	;@143a
	mov	cs:[si+DP_name],al	;@143c
	inc	si	;@1441
	inc	di	;@1442
	loop	@@copy_name	;@1443
@@end_string:
	mov	byte ptr cs:[si+DP_name],0	;@1445
	mov	byte ptr cs:[si+DP_name+1],0	;@144b
	mov	dx,cs	;@1451
	mov	cs:[DP_devname_s],dx	;@1453
	mov	ax,DP_DDT	;@1458
	pop	di	;@145b
	pop	si	;@145c
	pop	ds	;@145d
	pop	bp	;@145e
	retf	;@145f
; ---------------------------------------------------------------------------
; DP_detect_device - immer 1 (DIGPAK-Geraet vorhanden)
; ---------------------------------------------------------------------------
F_R22_1460	label	far	; Linkname von DP_detect_device
DP_detect_device:
	push	bp	;@1460
	mov	bp,sp	;@1461
	push	ds	;@1463
	push	si	;@1464
	push	di	;@1465
	mov	ax,1	;@1466
	pop	di	;@1469
	pop	si	;@146a
	pop	ds	;@146b
	pop	bp	;@146c
	retf	;@146d
; ---------------------------------------------------------------------------
; DP_init_driver - Platzhalter-Zeitgeber anmelden, Zustand zuruecksetzen
; ---------------------------------------------------------------------------
F_R22_146E	label	far	; Linkname von DP_init_driver
DP_init_driver:
	push	bp	;@146e
	mov	bp,sp	;@146f
	push	ds	;@1471
	push	si	;@1472
	push	di	;@1473
	PUSH_C	0	;@1474
	PUSH_C	0	;@147e
	push	cs	;@1488
	call	near ptr AIL_register_timer	;@1489
	add	sp,4	;@148c
	mov	cs:[DP_timer],ax	;@148f
	mov	word ptr cs:[AIL_hooked],1	;@1493
	PUSH_C	0	;@149a
	push	cs	;@14a4
	call	near ptr DP_describe_driver	;@14a5
	add	sp,2	;@14a8
	mov	word ptr cs:[loop_cnt],0	;@14ab
	mov	word ptr cs:[DAC_status],0	;@14b2
	mov	word ptr cs:[buffer_mode],1	;@14b9
	mov	word ptr cs:[buff_status],3	;@14c0
	mov	word ptr cs:[buff_status+2],3	;@14c7
	pop	di	;@14ce
	pop	si	;@14cf
	pop	ds	;@14d0
	pop	bp	;@14d1
	retf	;@14d2
; ---------------------------------------------------------------------------
; DP_shutdown_driver - Ausgabe anhalten, DIGPAK entfernen (DP_base + 103h),
; Platzhalter-Zeitgeber freigeben
; ---------------------------------------------------------------------------
F_R22_14D3	label	far	; Linkname von DP_shutdown_driver
DP_shutdown_driver:
	push	bp	;@14d3
	mov	bp,sp	;@14d4
	push	ds	;@14d6
	push	si	;@14d7
	push	di	;@14d8
	push	cs	;@14d9
	call	near ptr DP_stop_d_pb	;@14da
	les	di,cs:[DP_base]	;@14dd
	mov	cs:[call_seg],es	;@14e2
	add	di,0103h	;@14e7
	mov	cs:[call_off],di	;@14eb
	call	dword ptr cs:[call_off]	;@14f0
	push	word ptr cs:[DP_timer]	;@14f5
	push	cs	;@14fa
	call	near ptr AIL_release_timer_handle	;@14fb
	add	sp,2	;@14fe
	pop	di	;@1501
	pop	si	;@1502
	pop	ds	;@1503
	pop	bp	;@1504
	retf	;@1505
; ---------------------------------------------------------------------------
; DP_index_VOC_blk - Sprachblock einer VOC-Datei in einen sound_buff
; Ein:  [bp+6] Handle, [bp+8] VOC-Datei (far), [bp+0ch] Marke (-1 = erster
;       Block), [bp+0eh] sound_buff far
; Aus:  AX = 1 gefunden, 0 nicht
; ---------------------------------------------------------------------------
F_R22_1506	label	far	; Linkname von DP_index_VOC_blk
DP_index_VOC_blk:
	push	bp	;@1506
	mov	bp,sp	;@1507
	push	ds	;@1509
	push	si	;@150a
	push	di	;@150b
	lds	si,[bp+8]	;@150c
	mov	ax,[si+014h]	;@150f
	ADD_PTR	ax,0,ds,si	;@1512
	mov	bx,[bp+0ch]	;@154b
@@get_type:
	mov	al,[si]	;@154e
	mov	ah,0	;@1550
	cmp	ax,0	;@1552
	jnz	short @@chk_voice	;@1555
	jmp	near ptr @@exit	;@1557
@@chk_voice:
	cmp	ax,1	;@155a
	jnz	short @@chk_marker	;@155d
	cmp	bx,-1	;@155f
	jz	short @@vblk_found	;@1562
	jmp	short @@next_blk	;@1564
@@chk_marker:
	cmp	ax,4	;@1566
	jnz	short @@next_blk	;@1569
	cmp	bx,[si+4]	;@156b
	jnz	short @@next_blk	;@156e
	mov	bx,0ffffh	;@1570
@@next_blk:
	inc	si	;@1573
	lodsw	;@1574
	mov	dl,[si]	;@1575
	mov	dh,0	;@1577
	inc	si	;@1579
	ADD_PTR	ax,dx,ds,si	;@157a
	jmp	short @@get_type	;@15b2
@@vblk_found:
	les	di,[bp+0eh]	;@15b4
	mov	al,[si+4]	;@15b7
	mov	ah,0	;@15ba
	mov	es:[di+2],ax	;@15bc
	mov	al,[si+5]	;@15c0
	mov	ah,0	;@15c3
	mov	es:[di],ax	;@15c5
	mov	ax,[si+1]	;@15c8
	mov	dl,[si+3]	;@15cb
	mov	dh,0	;@15ce
	sub	ax,2	;@15d0
	sbb	dx,0	;@15d3
	mov	es:[di+8],ax	;@15d6
	mov	es:[di+0ah],dx	;@15da
	mov	dx,ds	;@15de
	mov	ax,si	;@15e0
	ADD_PTR	6,0,dx,ax	;@15e2
	mov	es:[di+4],ax	;@161c
	mov	es:[di+6],dx	;@1620
	mov	ax,1	;@1624
@@exit:
	pop	di	;@1627
	pop	si	;@1628
	pop	ds	;@1629
	pop	bp	;@162a
	retf	;@162b
; ---------------------------------------------------------------------------
; DP_register_sb - Puffer 0/1 fuer die Doppelpuffer-Ausgabe anmelden
; Ein:  [bp+6] Handle, [bp+8] Puffernummer, [bp+0ah] sound_buff far
; ---------------------------------------------------------------------------
F_R22_162C	label	far	; Linkname von DP_register_sb
DP_register_sb:
	push	bp	;@162c
	mov	bp,sp	;@162d
	push	ds	;@162f
	push	si	;@1630
	push	di	;@1631
	cmp	word ptr cs:[buffer_mode],0	;@1632
	jnz	short @@get_bufnum	;@1638
	PUSH_C	0	;@163a
	push	cs	;@1644
	call	near ptr DP_stop_d_pb	;@1645
	add	sp,2	;@1648
	mov	word ptr cs:[buffer_mode],1	;@164b
@@get_bufnum:
	mov	di,[bp+8]	;@1652
	shl	di,1	;@1655
	lds	si,[bp+0ah]	;@1657
	mov	ax,[si+2]	;@165a
	mov	cs:[di+buff_sample],ax	;@165d
	les	bx,[si+4]	;@1662
	mov	cs:[di+buff_data_o],bx	;@1665
	mov	cs:[di+buff_data_s],es	;@166a
	mov	ax,[si+8]	;@166f
	mov	cs:[di+buff_len_l],ax	;@1672
	mov	ax,[si+0ah]	;@1677
	mov	cs:[di+buff_len_h],ax	;@167a
	mov	word ptr cs:[di+buff_status],0	;@167f
	pop	di	;@1686
	pop	si	;@1687
	pop	ds	;@1688
	pop	bp	;@1689
	retf	;@168a
; ---------------------------------------------------------------------------
; DP_get_sb_status - Zustand eines Puffers (DAC_*)
; ---------------------------------------------------------------------------
F_R22_168B	label	far	; Linkname von DP_get_sb_status
DP_get_sb_status:
	push	bp	;@168b
	mov	bp,sp	;@168c
	push	ds	;@168e
	push	si	;@168f
	push	di	;@1690
	mov	bx,[bp+8]	;@1691
	shl	bx,1	;@1694
	mov	ax,cs:[bx+buff_status]	;@1696
	pop	di	;@169b
	pop	si	;@169c
	pop	ds	;@169d
	pop	bp	;@169e
	retf	;@169f
; ---------------------------------------------------------------------------
; DP_play_VOC_file - VOC-Datei zur Ausgabe vorbereiten
; Ein:  [bp+6] Handle, [bp+8] VOC-Datei (far), [bp+0ch] Marke (-1 = Anfang)
; ---------------------------------------------------------------------------
F_R22_16A0	label	far	; Linkname von DP_play_VOC_file
DP_play_VOC_file:
	push	bp	;@16a0
	mov	bp,sp	;@16a1
	sub	sp,4	;@16a3
	push	ds	;@16a6
	push	si	;@16a7
	push	di	;@16a8
	PUSH_C	0	;@16a9
	push	cs	;@16b3
	call	near ptr DP_stop_d_pb	;@16b4
	add	sp,2	;@16b7
	mov	word ptr cs:[buffer_mode],0	;@16ba
	les	di,[bp+8]	;@16c1
	mov	[bp-4],di	;@16c4
	mov	[bp-2],es	;@16c7
	mov	word ptr cs:[DAC_status],3	;@16ca
	lds	si,[bp-4]	;@16d1
	mov	ax,[si+014h]	;@16d4
	ADD_PTR	ax,0,ds,si	;@16d7
	mov	cs:[block_ptr],si	;@1710
	mov	cs:[block_ptr+2],ds	;@1715
	cmp	word ptr [bp+0ch],-1	;@171a
	jz	short @@do_it	;@171e
@@find_blk:
	push	cs	;@1720
	call	near ptr block_type	;@1721
	cmp	ax,0	;@1724
	jz	short @@exit	;@1727
	push	cs	;@1729
	call	near ptr marker_num	;@172a
	mov	si,ax	;@172d
	push	cs	;@172f
	call	near ptr next_block	;@1730
	cmp	si,[bp+0ch]	;@1733
	jnz	@@find_blk	;@1736
@@do_it:
	mov	word ptr cs:[DAC_status],0	;@1738
@@exit:
	pop	di	;@173f
	pop	si	;@1740
	pop	ds	;@1741
	mov	sp,bp	;@1742
	pop	bp	;@1744
	retf	;@1745
; ---------------------------------------------------------------------------
; DP_format_VOC_file - VOC-Datei fuer den DIGPAK-Treiber vorformatieren
; Ein:  [bp+6] Handle, [bp+8] VOC-Datei (far), [bp+0ch] Marke
; Locals: [bp-4] block_file, [bp-6] pack, [bp-8] freq.
; ---------------------------------------------------------------------------
F_R22_1746	label	far	; Linkname von DP_format_VOC_file
DP_format_VOC_file:
	push	bp	;@1746
	mov	bp,sp	;@1747
	sub	sp,8	;@1749
	push	ds	;@174c
	push	si	;@174d
	push	di	;@174e
	mov	byte ptr [bp-6],0ffh	;@174f
	les	di,[bp+8]	;@1753
	mov	[bp-4],di	;@1756
	mov	[bp-2],es	;@1759
	lds	si,[bp-4]	;@175c
	mov	ax,[si+014h]	;@175f
	ADD_PTR	ax,0,ds,si	;@1762
	mov	cs:[block_ptr],si	;@179b
	mov	cs:[block_ptr+2],ds	;@17a0
	cmp	word ptr [bp+0ch],-1	;@17a5
	jz	short @@preform_blk	;@17a9
@@form_find:
	push	cs	;@17ab
	call	near ptr block_type	;@17ac
	cmp	ax,0	;@17af
	jz	@@exit	;@17b2
	db	09ah	; call far marker_num (TLINK: nop/push cs/call near)	;@17b6 far call marker_num (der Linker macht nop/push cs/call near daraus)
	dd	marker_num	;@17b6
	mov	si,ax	;@17bb
	push	cs	;@17bd
	call	near ptr next_block	;@17be
	cmp	si,[bp+0ch]	;@17c1
	jnz	@@form_find	;@17c4
@@preform_blk:
	push	cs	;@17c6
	call	near ptr block_type	;@17c7
	cmp	ax,0	;@17ca
	jz	short @@exit	;@17cd
	cmp	ax,1	;@17cf
	jnz	short @@not_vdata	;@17d2
	lds	si,cs:[block_ptr]	;@17d4
	push	word ptr [si+4]	;@17d9
	push	cs	;@17dc
	call	near ptr get_sample_rate	;@17dd
	add	sp,2	;@17e0
	mov	[bp-8],ax	;@17e3
	mov	al,[si+5]	;@17e6
	and	al,0fh	;@17e9
	mov	[bp-6],al	;@17eb
	mov	ax,[si+1]	;@17ee
	mov	dx,[si+3]	;@17f1
	mov	dh,0	;@17f4
	sub	ax,2	;@17f6
	sbb	dx,0	;@17f9
	add	si,6	;@17fc
	jmp	short @@preform	;@17ff
@@not_vdata:
	cmp	ax,2	;@1801
	jnz	short @@preform_next	;@1804
	lds	si,cs:[block_ptr]	;@1806
	mov	ax,[si+1]	;@180b
	mov	dx,[si+3]	;@180e
	mov	dh,0	;@1811
	add	si,4	;@1813
@@preform:
	cmp	byte ptr [bp-6],0	;@1816
	jnz	short @@preform_next	;@181a
	push	dx	;@181c
	push	ax	;@181d
	push	ds	;@181e
	push	si	;@181f
	push	word ptr [bp-8]	;@1820
	push	cs	;@1823
	call	near ptr DP_preformat	;@1824
	add	sp,0ah	;@1827
@@preform_next:
	push	cs	;@182a
	call	near ptr next_block	;@182b
	jmp	short @@preform_blk	;@182e
@@exit:
	pop	di	;@1830
	pop	si	;@1831
	pop	ds	;@1832
	mov	sp,bp	;@1833
	pop	bp	;@1835
	retf	;@1836
; ---------------------------------------------------------------------------
; DP_format_sb - sound_buff fuer den DIGPAK-Treiber vorformatieren
; Ein:  [bp+6] Handle, [bp+8] sound_buff far
; Locals: [bp-4] ssize.
; Originalfehler #300 (Miles AIL 2.14): nach get_sample_rate wird mit
; add sp,4 ein Wort zu viel abgeraeumt. pop di/si/ds holen dann die falschen
; Werte; DS kommt aus ssize zurueck. Nur mit einem DIGPAK-Treiber erreichbar
; (das Spiel laedt AIL-Treiber .ADV).
; ---------------------------------------------------------------------------
F_R22_1837	label	far	; Linkname von DP_format_sb
DP_format_sb:
	push	bp	;@1837
	mov	bp,sp	;@1838
	sub	sp,4	;@183a
	push	ds	;@183d
	push	si	;@183e
	push	di	;@183f
	lds	si,[bp+8]	;@1840
	mov	ax,[si+8]	;@1843
	mov	dx,[si+0ah]	;@1846
	mov	[bp-4],ax	;@1849
	mov	[bp-2],dx	;@184c
	push	word ptr [si+2]	;@184f
	push	cs	;@1852
	call	near ptr get_sample_rate	;@1853
IF FIX_0300
	add	sp,2		; Fix #300: nur das gepushte Wort abraeumen
ELSE
	add	sp,4	;@1856 Fehler #300: 2 Bytes gepusht, 4 abgeraeumt
ENDIF
	push	word ptr [bp-2]	;@1859
	push	word ptr [bp-4]	;@185c
	push	word ptr [si+6]	;@185f
	push	word ptr [si+4]	;@1862
	push	ax	;@1865
	push	cs	;@1866
	call	near ptr DP_preformat	;@1867
	add	sp,0ah	;@186a
	pop	di	;@186d
	pop	si	;@186e
	pop	ds	;@186f
	mov	sp,bp	;@1870
	pop	bp	;@1872
	retf	;@1873
; ---------------------------------------------------------------------------
; DP_start_d_pb - Ausgabe starten (VOC-Datei oder Doppelpuffer)
; ---------------------------------------------------------------------------
F_R22_1874	label	far	; Linkname von DP_start_d_pb
DP_start_d_pb:
	push	bp	;@1874
	mov	bp,sp	;@1875
	push	ds	;@1877
	push	si	;@1878
	push	di	;@1879
	cmp	word ptr cs:[buffer_mode],0	;@187a
	jz	short @@voc_mode	;@1880
	cmp	word ptr cs:[DAC_status],2	;@1882
	jz	short @@exit	;@1888
	push	cs	;@188a
	call	near ptr next_buffer	;@188b
	cmp	ax,0ffffh	;@188e
	jz	short @@exit	;@1891
	mov	word ptr cs:[DAC_status],2	;@1893
	push	ax	;@189a
	push	cs	;@189b
	call	near ptr process_buffer	;@189c
	add	sp,2	;@189f
	jmp	short @@exit	;@18a2
@@voc_mode:
	cmp	word ptr cs:[DAC_status],0	;@18a4
	jnz	short @@exit	;@18aa
	mov	word ptr cs:[DAC_status],2	;@18ac
	push	cs	;@18b3
	call	near ptr process_block	;@18b4
@@exit:
	pop	di	;@18b7
	pop	si	;@18b8
	pop	ds	;@18b9
	pop	bp	;@18ba
	retf	;@18bb
; ---------------------------------------------------------------------------
; DP_stop_d_pb - Ausgabe anhalten, Rueckruf entfernen, int 8 zurueck
; ---------------------------------------------------------------------------
F_R22_18BC	label	far	; Linkname von DP_stop_d_pb
DP_stop_d_pb:
	push	bp	;@18bc
	mov	bp,sp	;@18bd
	push	ds	;@18bf
	push	si	;@18c0
	push	di	;@18c1
	mov	word ptr cs:[DAC_status],0	;@18c2
	PUSH_C	0	;@18c9
	PUSH_C	0	;@18d3
	push	cs	;@18dd
	call	near ptr CB_set_vect	;@18de
	add	sp,4	;@18e1
	mov	ax,DP_STOP_SOUND	;@18e4
	int	066h	;@18e7
	mov	word ptr cs:[buff_status],3	;@18e9
	mov	word ptr cs:[buff_status+2],3	;@18f0
	push	cs	;@18f7
	call	near ptr AIL_hook	;@18f8
	pop	di	;@18fb
	pop	si	;@18fc
	pop	ds	;@18fd
	pop	bp	;@18fe
	retf	;@18ff
; ---------------------------------------------------------------------------
; DP_get_VOC_status - Zustand der VOC-Ausgabe (DAC_status)
; ---------------------------------------------------------------------------
F_R22_1900	label	far	; Linkname von DP_get_VOC_status
DP_get_VOC_status:
	push	bp	;@1900
	mov	bp,sp	;@1901
	push	ds	;@1903
	push	si	;@1904
	push	di	;@1905
	mov	ax,cs:[DAC_status]	;@1906
	pop	di	;@190a
	pop	si	;@190b
	pop	ds	;@190c
	pop	bp	;@190d
	retf	;@190e
SEG022_TEXT	ends
	end

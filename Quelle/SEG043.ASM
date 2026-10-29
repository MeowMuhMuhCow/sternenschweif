;===========================================================================
; SEG043.ASM - Grafikmodus, Bildschirmseite, Palette (BIOS int 10h, DAC)
; Sternenschweif (DSA 2), Attic 1994 - Rekonstruktion, bytegleich mit
; SCHWEIF.EXE C1.02 (TASM 3.1 /ml)
;
; Inhalt:    Videomodus setzen/sichern/herstellen, Bildschirmseite waehlen,
;            Farben setzen (direkt ueber den DAC, Ports 3C8h/3C9h) und lesen
;            (int 10h/1017h), Zeiger auf das InDOS-Byte holen (int 21h/34h).
; Aufrufer:  O032 (Start/Ende: save_display_stat, restore_video_mode,
;            set_video_mode, dos_get_indos_ptr), SEG002 (get_color),
;            O034/O044 u. a. (get_palette). vga_set_color, vga_set_palette und
;            set_display_page haben keinen far-Aufrufer (C benutzt die
;            gleichnamigen Routinen in SEG002).
; Aufruf:    far, C-Konvention, Stackrahmen bp, Parameter ab [bp+6].
;            Alle BIOS-/DAC-Zugriffe mit gesperrten Interrupts (cli/sti).
; Register:  DS, ES, SI, DI gesichert; AX, BX, CX, DX veraendert.
; Daten:     keine.
; Herkunft:  aus SCHWEIF.EXE R43 (ab +0xc) mit asm2tasm.py zerlegt;
;            ;@nnnn = Offset im Segment R43.
;===========================================================================
	.386p
DAC_WRITE_INDEX	equ	03c8h	; VGA-DAC: Schreibindex
DAC_DATA	equ	03c9h	; VGA-DAC: Daten (R, G, B je 6 Bit)
	locals
SEG043_TEXT	segment word public use16 'CODE'
	assume	cs:SEG043_TEXT, ds:nothing
	public	F_R43_000C
	public	F_R43_0024
	public	F_R43_003C
	public	F_R43_0072
	public	F_R43_0099
	public	F_R43_00C3
	public	F_R43_00F2
	public	F_R43_0111
	public	F_R43_0130
; ---------------------------------------------------------------------------
; set_video_mode - Videomodus setzen (int 10h, AH = 0)
; Ein:  [bp+6] Modus
; Aus:  -
; ---------------------------------------------------------------------------
F_R43_000C	label	far	; Linkname von set_video_mode
set_video_mode:
	push	bp	;@000c
	mov	bp,sp	;@000d
	push	ds	;@000f
	push	es	;@0010
	push	si	;@0011
	push	di	;@0012
	cli	;@0013
	mov	ah,0	;@0014
	mov	al,[bp+6]	;@0016
	int	010h	;@0019
	sti	;@001b
	pop	di	;@001c
	pop	si	;@001d
	pop	es	;@001e
	pop	ds	;@001f
	mov	sp,bp	;@0020
	pop	bp	;@0022
	retf	;@0023
; ---------------------------------------------------------------------------
; set_display_page - aktive Bildschirmseite waehlen (int 10h, AH = 5)
; Ein:  [bp+6] Seite
; Aus:  -
; ---------------------------------------------------------------------------
F_R43_0024	label	far	; Linkname von set_display_page
set_display_page:
	push	bp	;@0024
	mov	bp,sp	;@0025
	push	ds	;@0027
	push	es	;@0028
	push	si	;@0029
	push	di	;@002a
	cli	;@002b
	mov	al,[bp+6]	;@002c
	mov	ah,5	;@002f
	int	010h	;@0031
	sti	;@0033
	pop	di	;@0034
	pop	si	;@0035
	pop	es	;@0036
	pop	ds	;@0037
	mov	sp,bp	;@0038
	pop	bp	;@003a
	retf	;@003b
; ---------------------------------------------------------------------------
; save_display_stat - Videozustand sichern (Name wie Bright-Eyes, DSA 1)
; Ein:  [bp+6] far-Zeiger auf struct video_state (4 Worte)
; Aus:  +0 Seite, +2 Modus, +4 Spalten (int 10h/0Fh),
;       +6 Textzeilen (int 10h/1130h, DL + 1)
; bp wird um int 10h/1130h gesichert, weil der Aufruf ES:BP ueberschreibt.
; ---------------------------------------------------------------------------
F_R43_003C	label	far	; Linkname von save_display_stat
save_display_stat:
	push	bp	;@003c
	mov	bp,sp	;@003d
	push	ds	;@003f
	push	es	;@0040
	push	si	;@0041
	push	di	;@0042
	cli	;@0043
	push	bp	;@0044 int 10h/1130h ueberschreibt BP
	les	di,[bp+6]	;@0045
	mov	ah,0fh	;@0048
	int	010h	;@004a AL = Modus, AH = Spalten, BH = Seite
	mov	dx,ax	;@004c
	xor	ax,ax	;@004e
	mov	al,bh	;@0050
	stosw	;@0052 +0 Seite
	mov	al,dl	;@0053
	stosw	;@0055 +2 Modus
	mov	al,dh	;@0056
	stosw	;@0058 +4 Spalten
	push	es	;@0059
	mov	ax,01130h	;@005a Zeichensatzinfo: DL = Zeilen - 1
	mov	bh,2	;@005d
	int	010h	;@005f
	xor	dh,dh	;@0061
	inc	dx	;@0063
	pop	es	;@0064
	mov	es:[di],dx	;@0065 +6 Zeilen
	pop	bp	;@0068
	sti	;@0069
	pop	di	;@006a
	pop	si	;@006b
	pop	es	;@006c
	pop	ds	;@006d
	mov	sp,bp	;@006e
	pop	bp	;@0070
	retf	;@0071
; ---------------------------------------------------------------------------
; restore_video_mode - mit save_display_stat gesicherten Zustand herstellen
; Ein:  [bp+6] far-Zeiger auf struct video_state
; Aus:  -   (set_video_mode(+2), set_display_page(+0))
; ---------------------------------------------------------------------------
F_R43_0072	label	far	; Linkname von restore_video_mode
restore_video_mode:
	push	bp	;@0072
	mov	bp,sp	;@0073
	push	ds	;@0075
	push	es	;@0076
	push	si	;@0077
	push	di	;@0078
	cli	;@0079
	push	bp	;@007a
	les	di,[bp+6]	;@007b
	push	word ptr es:[di+2]	;@007e
	push	cs	;@0082
	call	near ptr set_video_mode	;@0083
	push	word ptr es:[di]	;@0086
	push	cs	;@0089
	call	near ptr set_display_page	;@008a
	pop	cx	;@008d Parameter abraeumen
	pop	cx	;@008e
	pop	bp	;@008f
	sti	;@0090
	pop	di	;@0091
	pop	si	;@0092
	pop	es	;@0093
	pop	ds	;@0094
	mov	sp,bp	;@0095
	pop	bp	;@0097
	retf	;@0098
; ---------------------------------------------------------------------------
; vga_set_color - eine Farbe direkt in den DAC schreiben (in DSA 1 set_color)
; Ein:  [bp+6] far-Zeiger auf 3 Bytes R, G, B; [bp+0ah] Farbnummer
; Aus:  -   (Werte auf 6 Bit begrenzt)
; Unbenutzt; das Spiel benutzt set_color in SEG002.
; ---------------------------------------------------------------------------
F_R43_0099	label	far	; Linkname von vga_set_color
vga_set_color:
	push	bp	;@0099
	mov	bp,sp	;@009a
	push	ds	;@009c
	push	es	;@009d
	push	si	;@009e
	push	di	;@009f
	cli	;@00a0
	mov	dx,DAC_WRITE_INDEX	;@00a1
	mov	al,[bp+0ah]	;@00a4
	out	dx,al	;@00a7
	lds	si,[bp+6]	;@00a8
	mov	dx,DAC_DATA	;@00ab
	lodsb	;@00ae
	and	al,03fh	;@00af
	out	dx,al	;@00b1
	lodsb	;@00b2
	and	al,03fh	;@00b3
	out	dx,al	;@00b5
	lodsb	;@00b6
	and	al,03fh	;@00b7
	out	dx,al	;@00b9
	sti	;@00ba
	pop	di	;@00bb
	pop	si	;@00bc
	pop	es	;@00bd
	pop	ds	;@00be
	mov	sp,bp	;@00bf
	pop	bp	;@00c1
	retf	;@00c2
; ---------------------------------------------------------------------------
; vga_set_palette - Farbbereich direkt in den DAC schreiben (DSA 1 set_palette)
; Ein:  [bp+6] far-Zeiger auf n * 3 Bytes, [bp+0ah] erste Farbe, [bp+0ch] n
; Aus:  -
; Unbenutzt; das Spiel benutzt set_palette in SEG002.
; ---------------------------------------------------------------------------
F_R43_00C3	label	far	; Linkname von vga_set_palette
vga_set_palette:
	push	bp	;@00c3
	mov	bp,sp	;@00c4
	push	ds	;@00c6
	push	es	;@00c7
	push	si	;@00c8
	push	di	;@00c9
	cli	;@00ca
	mov	dx,DAC_WRITE_INDEX	;@00cb
	mov	al,[bp+0ah]	;@00ce
	out	dx,al	;@00d1
	lds	si,[bp+6]	;@00d2
	mov	dx,DAC_DATA	;@00d5
	mov	cx,[bp+0ch]	;@00d8
@@next_color:
	lodsb	;@00db
	and	al,03fh	;@00dc
	out	dx,al	;@00de
	lodsb	;@00df
	and	al,03fh	;@00e0
	out	dx,al	;@00e2
	lodsb	;@00e3
	and	al,03fh	;@00e4
	out	dx,al	;@00e6
	loop	@@next_color	;@00e7
	sti	;@00e9
	pop	di	;@00ea
	pop	si	;@00eb
	pop	es	;@00ec
	pop	ds	;@00ed
	mov	sp,bp	;@00ee
	pop	bp	;@00f0
	retf	;@00f1
; ---------------------------------------------------------------------------
; get_color - eine Farbe aus dem DAC lesen (int 10h/1017h, CX = 1)
; Ein:  [bp+6] far-Zeiger auf 3 Bytes, [bp+0ah] Farbnummer
; Aus:  -
; ---------------------------------------------------------------------------
F_R43_00F2	label	far	; Linkname von get_color
get_color:
	push	bp	;@00f2
	mov	bp,sp	;@00f3
	push	ds	;@00f5
	push	es	;@00f6
	push	si	;@00f7
	push	di	;@00f8
	cli	;@00f9
	mov	ax,01017h	;@00fa
	mov	cx,1	;@00fd
	mov	bx,[bp+0ah]	;@0100
	les	dx,[bp+6]	;@0103
	int	010h	;@0106
	sti	;@0108
	pop	di	;@0109
	pop	si	;@010a
	pop	es	;@010b
	pop	ds	;@010c
	mov	sp,bp	;@010d
	pop	bp	;@010f
	retf	;@0110
; ---------------------------------------------------------------------------
; get_palette - Farbbereich aus dem DAC lesen (int 10h/1017h)
; Ein:  [bp+6] far-Zeiger auf n * 3 Bytes, [bp+0ah] erste Farbe, [bp+0ch] n
; Aus:  -
; ---------------------------------------------------------------------------
F_R43_0111	label	far	; Linkname von get_palette
get_palette:
	push	bp	;@0111
	mov	bp,sp	;@0112
	push	ds	;@0114
	push	es	;@0115
	push	si	;@0116
	push	di	;@0117
	cli	;@0118
	mov	ax,01017h	;@0119
	mov	cx,[bp+0ch]	;@011c
	mov	bx,[bp+0ah]	;@011f
	les	dx,[bp+6]	;@0122
	int	010h	;@0125
	sti	;@0127
	pop	di	;@0128
	pop	si	;@0129
	pop	es	;@012a
	pop	ds	;@012b
	mov	sp,bp	;@012c
	pop	bp	;@012e
	retf	;@012f
; ---------------------------------------------------------------------------
; dos_get_indos_ptr - Zeiger auf das InDOS-Byte (int 21h, AH = 34h)
; Ein:  -
; Aus:  DX:AX = ES:BX des Aufrufs
; O032 legt ihn in g_indos_flag ab; die Maus-ISR (R23) prueft damit, ob
; gerade DOS laeuft.
; ---------------------------------------------------------------------------
F_R43_0130	label	far	; Linkname von dos_get_indos_ptr
dos_get_indos_ptr:
	push	bp	;@0130
	mov	bp,sp	;@0131
	push	ds	;@0133
	push	es	;@0134
	push	si	;@0135
	push	di	;@0136
	mov	ax,03400h	;@0137
	int	021h	;@013a
	mov	ax,bx	;@013c
	mov	dx,es	;@013e
	pop	di	;@0140
	pop	si	;@0141
	pop	es	;@0142
	pop	ds	;@0143
	mov	sp,bp	;@0144
	pop	bp	;@0146
	retf	;@0147
SEG043_TEXT	ends
	end

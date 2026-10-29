;===========================================================================
; SEG026.ASM - Schriftzeichen zeichnen; Fehlerfalle fuer den Overlay-Lader
; Sternenschweif (DSA 2), Attic 1994 - Rekonstruktion, bytegleich mit
; SCHWEIF.EXE C1.02 (TASM 3.1 /ml)
;
; Inhalt:    glyph_blit, glyph_blit_bg: ein Zeichenbitmuster (ein Byte je
;            Punkt, 0 = Hintergrund) ueber eine Farbtabelle (xlat) in einen
;            Bildpuffer mit 320 Bytes je Zeile zeichnen.
;            Dazu toter Debug-Code ohne Aufrufer: ein Waechter fuer den
;            Overlay-Lader (int 3Fh, Borland VROOMM). Wird ein Overlay
;            nachgeladen, waehrend eine Interrupt-Routine laeuft (Zeitgeber,
;            Kampf-/Ansichts-Rueckruf, Maus), haelt er das Programm an und
;            faerbt Farbe 0 rot; sonst blitzt Farbe 0 kurz rot auf.
; Aufrufer:  SEG020 (Schrift, font_draw), far. Der Waechter: keiner.
; Aufruf:    far, C-Konvention, Stackrahmen mit enter:
;              void glyph_blit(dst, src, pal, w, h)
;              void glyph_blit_bg(dst, src, pal, w, h, bg)
;            [bp+6] dst far, [bp+0ah] src far (Bitmuster, w*h Bytes),
;            [bp+0eh] pal far (Farbtabelle fuer xlat), [bp+12h] w,
;            [bp+14h] h, [bp+16h] bg (nur glyph_blit_bg)
; Register:  DS, SI, DI gesichert; AX, BX, CX, DX, ES, FS, EAX, EBX
;            veraendert.
; Daten:     g_old_int3f (eigenes Datenstueck DMR26, ds:8664);
;            der Waechter liest g_timer_isr_busy, g_view_timer_busy,
;            g_fight_timer_busy und g_78d8 (Maus-ISR-Sperre).
; Herkunft:  aus SCHWEIF.EXE R26 (ab +0x2) mit asm2tasm.py zerlegt;
;            ;@nnnn = Offset im Segment R26.
;===========================================================================
	.386p
; Daten im Datensegment; Adressen als Konstanten (Code ohne Fixups wie im Original)
g_old_int3f	equ	08664h	; dd  alter Vektor int 3Fh (Overlay-Lader), Stueck DMR26
g_timer_isr_busy	equ	0c536h	; db  Zeitgeber-ISR laeuft (R23)
g_view_timer_busy	equ	0cb6eh	; db  view_timer_cb laeuft
g_fight_timer_busy	equ	0cb6fh	; db  fight_timer_cb laeuft
g_mouse_isr_lock	equ	078d8h	; db  Maus-ISR gesperrt/laeuft (g_78d8; R23 mouse_isr, O199 Regler)
SCREEN_W	equ	320
	locals
SEG026_TEXT	segment byte public use16 'CODE'
	assume	cs:SEG026_TEXT, ds:nothing
	public	F_R26_0002
	public	F_R26_0040
; ---------------------------------------------------------------------------
; glyph_blit - Zeichen durchsichtig zeichnen
; Ein:  dst, src, pal, w, h (s. Kopf)
; Aus:  -
; Fuer jeden Punkt: Wert aus src; 0 bleibt durchsichtig, sonst pal[Wert]
; (xlat ueber DS:BX = pal) nach dst. Zeilenabstand im Ziel 320.
; ---------------------------------------------------------------------------
F_R26_0002	label	far	; Linkname von glyph_blit
glyph_blit:
	enter	0,0	;@0002
	push	ds	;@0006
	push	si	;@0007
	push	di	;@0008
	les	di,[bp+6]	;@0009 ES:DI = dst
	lfs	si,[bp+0ah]	;@000c FS:SI = src
	lds	bx,[bp+0eh]	;@0010 DS:BX = pal
	sub	eax,eax	;@0013
	movzx	ebx,bx	;@0016
	mov	cl,[bp+014h]	;@001a CL = h
@@next_row:
	mov	ch,[bp+012h]	;@001d CH = w
	push	di	;@0020
@@next_pixel:
	db	064h,0ach	; fs lodsb	;@0021 fs lodsb: Punkt holen
	or	al,al	;@0023
	jz	@@skip	;@0025
	xlatb	;@0029 Farbe ueber Tabelle
	mov	es:[di],al	;@002a
@@skip:
	inc	di	;@002d
	dec	ch	;@002e
	jnz	@@next_pixel	;@0030
	pop	di	;@0032
	add	di,SCREEN_W	;@0033 naechste Zielzeile
	dec	cl	;@0037
	jnz	@@next_row	;@0039
	pop	di	;@003b
	pop	si	;@003c
	pop	ds	;@003d
	leave	;@003e
	retf	;@003f
; ---------------------------------------------------------------------------
; glyph_blit_bg - Zeichen mit Hintergrundfarbe zeichnen
; Ein:  dst, src, pal, w, h, bg (s. Kopf)
; Aus:  -
; Wie glyph_blit, aber Punkte mit Wert 0 bekommen die Farbe bg (ohne Tabelle).
; ---------------------------------------------------------------------------
F_R26_0040	label	far	; Linkname von glyph_blit_bg
glyph_blit_bg:
	enter	0,0	;@0040
	push	ds	;@0044
	push	si	;@0045
	push	di	;@0046
	les	di,[bp+6]	;@0047
	lfs	si,[bp+0ah]	;@004a
	lds	bx,[bp+0eh]	;@004e
	mov	dl,[bp+016h]	;@0051 DL = bg
	mov	cl,[bp+014h]	;@0054
@@next_row:
	mov	ch,[bp+012h]	;@0057
	push	di	;@005a
@@next_pixel:
	db	064h,0ach	; fs lodsb	;@005b
	or	al,al	;@005d
	jz	@@background	;@005f
	xlatb	;@0063 Farbe ueber Tabelle
	mov	es:[di],al	;@0064
	inc	di	;@0067
	dec	ch	;@0068
	jnz	@@next_pixel	;@006a
	pop	di	;@006c
	add	di,SCREEN_W	;@006d
	dec	cl	;@0071
	jnz	@@next_row	;@0073
	pop	di	;@0075
	pop	si	;@0076
	pop	ds	;@0077
	leave	;@0078
	retf	;@0079
@@background:
	mov	es:[di],dl	;@007a Hintergrundfarbe (zweite Kopie der Schleife)
	inc	di	;@007d
	dec	ch	;@007e
	jnz	@@next_pixel	;@0080
	pop	di	;@0082
	add	di,SCREEN_W	;@0083
	dec	cl	;@0087
	jnz	@@next_row	;@0089
	pop	di	;@008b
	pop	si	;@008c
	pop	ds	;@008d
	leave	;@008e
	retf	;@008f
; ---------------------------------------------------------------------------
; ovl_trap_isr - Waechter fuer den Overlay-Lader (int 3Fh), Debug-Code
; Wird statt des Borland-Overlay-Interrupts angesprungen (ovl_trap_on setzt
; den Vektor auf cs:0090). Laeuft gerade eine Interrupt-Routine, darf kein
; Overlay geladen werden: dann Endlosschleife, Farbe 0 = rot (Programm haelt).
; Sonst Farbe 0 kurz rot, wieder schwarz, weiter zum alten int 3Fh.
; Ohne Aufrufer im Spiel (kein Einsprung, kein Verweis).
; Originalfehler #303: AX/BX/CX werden je Sperre mit einer eigenen Farbe
; geladen (weiss, magenta, gelb, tuerkis), ausgegeben wird aber immer 3fh,0,0.
; ---------------------------------------------------------------------------
ovl_trap_isr:
	push	dx	;@0090
	push	cx	;@0091
	push	bx	;@0092
	push	ax	;@0093
	cmp	byte ptr ds:[g_timer_isr_busy],0	;@0094
	jnz	@@in_timer_isr	;@0099
	cmp	byte ptr ds:[g_view_timer_busy],0	;@009d
	jnz	@@in_view_timer	;@00a2
	cmp	byte ptr ds:[g_fight_timer_busy],0	;@00a6
	jnz	@@in_fight_timer	;@00ab
	cmp	byte ptr ds:[g_mouse_isr_lock],0	;@00af
	jnz	@@in_mouse_isr	;@00b4
	cli	;@00b8
	mov	dx,03c8h	;@00b9 DAC Farbe 0 = (3fh, 0, 0) rot
	mov	al,0	;@00bc
	out	dx,al	;@00be
	mov	dx,03c9h	;@00bf
	mov	al,03fh	;@00c2
	out	dx,al	;@00c4
	mov	al,0	;@00c5
	out	dx,al	;@00c7
	mov	al,0	;@00c8
	out	dx,al	;@00ca
	sti	;@00cb
	mov	cx,0100h	;@00cc kurz warten
@@delay:
	loop	@@delay	;@00cf
	cli	;@00d1
	mov	dx,03c8h	;@00d2 DAC Farbe 0 = schwarz
	mov	al,0	;@00d5
	out	dx,al	;@00d7
	mov	dx,03c9h	;@00d8
	mov	al,0	;@00db
	out	dx,al	;@00dd
	mov	al,0	;@00de
	out	dx,al	;@00e0
	mov	al,0	;@00e1
	out	dx,al	;@00e3
	sti	;@00e4
	pop	ax	;@00e5
	pop	bx	;@00e6
	pop	cx	;@00e7
	pop	dx	;@00e8
	jmp	dword ptr ds:[g_old_int3f]	;@00e9 weiter zum alten int 3Fh
@@in_timer_isr:
	mov	ax,03fh	;@00ed
	mov	bx,03fh	;@00f0
	mov	cx,03fh	;@00f3
	jmp	@@halt	;@00f6
@@in_view_timer:
	mov	ax,03fh	;@00f9
	mov	bx,0	;@00fc
	mov	cx,03fh	;@00ff
	jmp	@@halt	;@0102
@@in_fight_timer:
	mov	ax,03fh	;@0105
	mov	bx,03fh	;@0108
	mov	cx,0	;@010b
	jmp	@@halt	;@010e
@@in_mouse_isr:
	mov	ax,0	;@0111
	mov	bx,03fh	;@0114
	mov	cx,03fh	;@0117
@@halt:
	cli	;@011a
	mov	dx,03c8h	;@011b
	mov	al,0	;@011e
	out	dx,al	;@0120
	mov	dx,03c9h	;@0121
	mov	al,03fh	;@0124 immer rot, AX/BX/CX ungenutzt (Fehler #303)
	out	dx,al	;@0126
	mov	al,0	;@0127
	out	dx,al	;@0129
	mov	al,0	;@012a
	out	dx,al	;@012c
	sti	;@012d
	jmp	short @@halt	;@012e Endlosschleife: Programm angehalten
; ---------------------------------------------------------------------------
; ovl_trap_on - Waechter einhaengen (int 3Fh -> cs:0090, alter Vektor nach
; g_old_int3f). Ohne Aufrufer.
; ---------------------------------------------------------------------------
ovl_trap_on:
	push	ds	;@0130
	push	es	;@0131
	push	ax	;@0132
	push	bx	;@0133
	push	dx	;@0134
	mov	ah,035h	;@0135
	mov	al,03fh	;@0137
	int	021h	;@0139
	mov	ds:[g_old_int3f],bx	;@013b ds:[g_old_int3f] = alter Vektor
	mov	ds:[g_old_int3f+2],es	;@013f
	mov	ah,025h	;@0143
	mov	al,03fh	;@0145
	mov	dx,090h	;@0147 neuer Vektor = ovl_trap_isr
	push	cs	;@014a
	pop	ds	;@014b
	int	021h	;@014c
	pop	dx	;@014e
	pop	bx	;@014f
	pop	ax	;@0150
	pop	es	;@0151
	pop	ds	;@0152
	retf	;@0153
; ---------------------------------------------------------------------------
; ovl_trap_off - Waechter aushaengen (alten int-3Fh-Vektor zuruecksetzen).
; Ohne Aufrufer.
; ---------------------------------------------------------------------------
ovl_trap_off:
	push	ds	;@0154
	push	ax	;@0155
	push	dx	;@0156
	mov	ah,025h	;@0157
	mov	al,03fh	;@0159
	mov	dx,ds:[g_old_int3f]	;@015b
	mov	ds,ds:[g_old_int3f+2]	;@015f
	int	021h	;@0163
	pop	dx	;@0165
	pop	ax	;@0166
	pop	ds	;@0167
	retf	;@0168
SEG026_TEXT	ends
	end

;===========================================================================
; SEG039.ASM - XMS (Extended Memory ueber den XMS-Treiber, HIMEM.SYS)
; Sternenschweif (DSA 2), Attic 1994 - Rekonstruktion, bytegleich mit
; SCHWEIF.EXE C1.02 (TASM 3.1 /ml)
;
; Inhalt:    xms_present (Treiber suchen, Einsprung merken) und je eine
;            duenne Huelle um die XMS-Funktionen 05h..11h: A20 lokal an/aus,
;            A20-Zustand, freier Speicher, Block anlegen/freigeben/kopieren/
;            sperren/entsperren, UMB anfordern/freigeben.
; Aufrufer:  O032 (Start), O010/O014/O046/O096/O190 (Auslagerung), R38
;            xms_move_any u. a.; xms_largest_kb, xms_a20_state und die UMB-
;            Routinen sind unbenutzt.
; Aufruf:    far, C-Konvention, Stackrahmen bp, Parameter ab [bp+6].
;            Rueckgabe als long: AX = Ergebnis, DX = Fehlercode << 8
;            (DH = BL des Treibers, 0 = in Ordnung). Ausnahme xms_lock:
;            DX:AX = lineare Adresse, Fehler in DH.
; Register:  DS, ES, SI, DI gesichert; AX, BX, DX veraendert.
; Daten:     g_xms_present, g_xms_entry (eigenes Datenstueck DMR39,
;            ds:86a0..86a6). g_xms_entry zeigt anfangs auf xms_no_driver.
; Herkunft:  aus SCHWEIF.EXE R39 (ab +0xe) mit asm2tasm.py zerlegt;
;            ;@nnnn = Offset im Segment R39.
;===========================================================================
	.386p
	include	..\names\inc\s_fix.inc	; Schalter der Fehlerbehebungen (FIX_nnnn, tools/mkfix.py)
; Daten im Datensegment (Stueck DMR39); Adressen als Konstanten
g_xms_present	equ	086a0h	; dw  1 = XMS-Treiber gefunden
g_xms_entry	equ	086a2h	; dd  Einsprung des XMS-Treibers (Vorgabe: xms_no_driver)
; XMS-Funktionen (AH beim Aufruf von g_xms_entry)
XMS_LOCAL_A20_ON	equ	05h
XMS_LOCAL_A20_OFF	equ	06h
XMS_QUERY_A20	equ	07h
XMS_QUERY_FREE	equ	08h
XMS_ALLOC	equ	09h
XMS_FREE	equ	0ah
XMS_MOVE	equ	0bh
XMS_LOCK	equ	0ch
XMS_UNLOCK	equ	0dh
XMS_UMB_ALLOC	equ	10h
XMS_UMB_FREE	equ	11h
	locals
SEG039_TEXT	segment byte public use16 'CODE'
	assume	cs:SEG039_TEXT, ds:nothing
	public	F_R39_000E
	public	F_R39_0012
	public	F_R39_0045
	public	F_R39_0062
	public	F_R39_0082
	public	F_R39_00A5
	public	F_R39_00C4
	public	F_R39_00E3
	public	F_R39_0101
	public	F_R39_0120
	public	F_R39_013F
	public	F_R39_015E
	public	F_R39_017A
	public	F_R39_0196
; ---------------------------------------------------------------------------
; xms_no_driver - Vorgabe fuer g_xms_entry, solange kein Treiber bekannt ist
; Ein:  wie ein XMS-Aufruf (AH = Funktion)
; Aus:  AX = 0, BL = 80h (Funktion nicht vorhanden), dann weiter in
;       xms_present (kein retf!): AX = g_xms_present.
; Originalfehler #304: Ist ein Treiber da, aber xms_present noch nicht
; gerufen, findet der Durchlauf ihn jetzt und meldet AX = 1, als waere die
; gewuenschte Funktion gelungen.
; ---------------------------------------------------------------------------
F_R39_000E	label	far	; Linkname von xms_no_driver
xms_no_driver:
	xor	ax,ax	;@000e
	mov	bl,080h	;@0010
IF FIX_0304
	retf			; Fix #304: nicht in xms_present weiterlaufen
ENDIF
; ---------------------------------------------------------------------------
; xms_present - XMS-Treiber suchen
; Ein:  -
; Aus:  AX = 1 vorhanden, 0 nicht
; Beim ersten Erfolg: int 2Fh/4300h (AL = 80h: installiert), int 2Fh/4310h
; liefert ES:BX = Einsprung nach g_xms_entry; g_xms_present = 1.
; ---------------------------------------------------------------------------
F_R39_0012	label	far	; Linkname von xms_present
xms_present:
	push	bp	;@0012
	mov	bp,sp	;@0013
	push	ds	;@0015
	push	es	;@0016
	push	si	;@0017
	push	di	;@0018
	cmp	word ptr ds:[g_xms_present],0	;@0019 schon gefunden?
	jnz	short @@done	;@001e
	mov	ax,04300h	;@0020 XMS installiert?
	int	02fh	;@0023
	cmp	al,080h	;@0025
	jnz	short @@done	;@0027
	mov	ax,04310h	;@0029 Einsprung holen
	int	02fh	;@002c
	mov	ds:[g_xms_entry],bx	;@002e
	mov	ds:[g_xms_entry+2],es	;@0032
	inc	word ptr ds:[g_xms_present]	;@0036
@@done:
	mov	ax,ds:[g_xms_present]	;@003a
	pop	di	;@003d
	pop	si	;@003e
	pop	es	;@003f
	pop	ds	;@0040
	mov	sp,bp	;@0041
	pop	bp	;@0043
	retf	;@0044
; ---------------------------------------------------------------------------
; xms_largest_kb - groesster freier Block in KB
; Ein:  -
; Aus:  AX = KB (XMS 08h: AX), DX = Fehler << 8. Unbenutzt.
; ---------------------------------------------------------------------------
F_R39_0045	label	far	; Linkname von xms_largest_kb
xms_largest_kb:
	push	bp	;@0045
	mov	bp,sp	;@0046
	push	ds	;@0048
	push	es	;@0049
	push	si	;@004a
	push	di	;@004b
	mov	ah,XMS_QUERY_FREE	;@004c
	call	dword ptr ds:[g_xms_entry]	;@004e
	xor	dx,dx	;@0052
	or	ax,ax	;@0054
	jnz	short @@done	;@0056
	mov	dh,bl	;@0058
@@done:
	pop	di	;@005a
	pop	si	;@005b
	pop	es	;@005c
	pop	ds	;@005d
	mov	sp,bp	;@005e
	pop	bp	;@0060
	retf	;@0061
; ---------------------------------------------------------------------------
; xms_free_kb - freier XMS-Speicher insgesamt in KB
; Ein:  -
; Aus:  AX = KB (XMS 08h: DX), DX = Fehler << 8
; ---------------------------------------------------------------------------
F_R39_0062	label	far	; Linkname von xms_free_kb
xms_free_kb:
	push	bp	;@0062
	mov	bp,sp	;@0063
	push	ds	;@0065
	push	es	;@0066
	push	si	;@0067
	push	di	;@0068
	mov	ah,XMS_QUERY_FREE	;@0069
	call	dword ptr ds:[g_xms_entry]	;@006b
	or	ax,ax	;@006f
	mov	ax,dx	;@0071
	mov	dx,0	;@0073
	jnz	short @@done	;@0076
	mov	dh,bl	;@0078
@@done:
	pop	di	;@007a
	pop	si	;@007b
	pop	es	;@007c
	pop	ds	;@007d
	mov	sp,bp	;@007e
	pop	bp	;@0080
	retf	;@0081
; ---------------------------------------------------------------------------
; xms_alloc_kb - XMS-Block anlegen
; Ein:  [bp+6] Groesse in KB
; Aus:  AX = Handle, DX = Fehler << 8
; ---------------------------------------------------------------------------
F_R39_0082	label	far	; Linkname von xms_alloc_kb
xms_alloc_kb:
	push	bp	;@0082
	mov	bp,sp	;@0083
	push	ds	;@0085
	push	es	;@0086
	push	si	;@0087
	push	di	;@0088
	mov	ah,XMS_ALLOC	;@0089
	mov	dx,[bp+6]	;@008b
	call	dword ptr ds:[g_xms_entry]	;@008e
	or	ax,ax	;@0092
	mov	ax,dx	;@0094
	mov	dx,0	;@0096
	jnz	short @@done	;@0099
	mov	dh,bl	;@009b
@@done:
	pop	di	;@009d
	pop	si	;@009e
	pop	es	;@009f
	pop	ds	;@00a0
	mov	sp,bp	;@00a1
	pop	bp	;@00a3
	retf	;@00a4
; ---------------------------------------------------------------------------
; xms_free_block - XMS-Block freigeben
; Ein:  [bp+6] Handle
; Aus:  DX = Fehler << 8 (0 = in Ordnung)
; ---------------------------------------------------------------------------
F_R39_00A5	label	far	; Linkname von xms_free_block
xms_free_block:
	push	bp	;@00a5
	mov	bp,sp	;@00a6
	push	ds	;@00a8
	push	es	;@00a9
	push	si	;@00aa
	push	di	;@00ab
	mov	ah,XMS_FREE	;@00ac
	mov	dx,[bp+6]	;@00ae
	call	dword ptr ds:[g_xms_entry]	;@00b1
	xor	dx,dx	;@00b5
	dec	ax	;@00b7
	jz	short @@done	;@00b8
	mov	dh,bl	;@00ba
@@done:
	pop	di	;@00bc
	pop	si	;@00bd
	pop	es	;@00be
	pop	ds	;@00bf
	mov	sp,bp	;@00c0
	pop	bp	;@00c2
	retf	;@00c3
; ---------------------------------------------------------------------------
; xms_move - Speicherblock verschieben (XMS 0Bh)
; Ein:  [bp+6] far-Zeiger auf den Kopierauftrag (EMM-Struktur: Laenge,
;       Quell-Handle/-Offset, Ziel-Handle/-Offset)
; Aus:  DX = Fehler << 8
; Originalfehler #305: lds setzt DS auf das Segment des Auftrags, bevor
; g_xms_entry ueber DS gelesen wird. Richtig nur, wenn der Auftrag im
; Datensegment liegt (alle Aufrufer uebergeben &g_xms_move, also folgenlos).
; ---------------------------------------------------------------------------
F_R39_00C4	label	far	; Linkname von xms_move
xms_move:
	push	bp	;@00c4
	mov	bp,sp	;@00c5
	push	ds	;@00c7
	push	es	;@00c8
	push	si	;@00c9
	push	di	;@00ca
	mov	ah,XMS_MOVE	;@00cb
	lds	si,[bp+6]	;@00cd
	call	dword ptr ds:[g_xms_entry]	;@00d0 DS = Segment des Auftrags (Fehler #305)
	xor	dx,dx	;@00d4
	dec	ax	;@00d6
	jz	short @@done	;@00d7
	mov	dh,bl	;@00d9
@@done:
	pop	di	;@00db
	pop	si	;@00dc
	pop	es	;@00dd
	pop	ds	;@00de
IF FIX_0304
				; Fix #304: SP = BP schon nach den pops (spart 2 Bytes fuer das retf oben)
ELSE
	mov	sp,bp	;@00df
ENDIF
	pop	bp	;@00e1
	retf	;@00e2
; ---------------------------------------------------------------------------
; xms_lock - XMS-Block sperren, lineare Adresse holen
; Ein:  [bp+6] Handle
; Aus:  DX:AX = 32-Bit-Adresse; bei Fehler DH = Fehlercode
; ---------------------------------------------------------------------------
F_R39_00E3	label	far	; Linkname von xms_lock
xms_lock:
	push	bp	;@00e3
	mov	bp,sp	;@00e4
	push	ds	;@00e6
	push	es	;@00e7
	push	si	;@00e8
	push	di	;@00e9
	mov	ah,XMS_LOCK	;@00ea
	mov	dx,[bp+6]	;@00ec
	call	dword ptr ds:[g_xms_entry]	;@00ef
	xchg	ax,bx	;@00f3
	dec	bx	;@00f4
	jz	short @@done	;@00f5
	mov	dh,al	;@00f7
@@done:
	pop	di	;@00f9
	pop	si	;@00fa
	pop	es	;@00fb
	pop	ds	;@00fc
	mov	sp,bp	;@00fd
	pop	bp	;@00ff
	retf	;@0100
; ---------------------------------------------------------------------------
; xms_unlock - Sperre aufheben
; Ein:  [bp+6] Handle
; Aus:  DX = Fehler << 8
; ---------------------------------------------------------------------------
F_R39_0101	label	far	; Linkname von xms_unlock
xms_unlock:
	push	bp	;@0101
	mov	bp,sp	;@0102
	push	ds	;@0104
	push	es	;@0105
	push	si	;@0106
	push	di	;@0107
	mov	ah,XMS_UNLOCK	;@0108
	mov	dx,[bp+6]	;@010a
	call	dword ptr ds:[g_xms_entry]	;@010d
	xor	dx,dx	;@0111
	dec	ax	;@0113
	jz	short @@done	;@0114
	mov	dh,bl	;@0116
@@done:
	pop	di	;@0118
	pop	si	;@0119
	pop	es	;@011a
	pop	ds	;@011b
	mov	sp,bp	;@011c
	pop	bp	;@011e
	retf	;@011f
; ---------------------------------------------------------------------------
; xms_umb_alloc - oberen Speicherblock (UMB) anfordern
; Ein:  [bp+6] Groesse in Paragraphen
; Aus:  AX = Segment, DX = tatsaechliche Groesse;
;       bei Fehler AX = groesster verfuegbarer UMB, DH = DL = Fehlercode.
; Unbenutzt.
; ---------------------------------------------------------------------------
F_R39_0120	label	far	; Linkname von xms_umb_alloc
xms_umb_alloc:
	push	bp	;@0120
	mov	bp,sp	;@0121
	push	ds	;@0123
	push	es	;@0124
	push	si	;@0125
	push	di	;@0126
	mov	ah,XMS_UMB_ALLOC	;@0127
	mov	dx,[bp+6]	;@0129
	call	dword ptr ds:[g_xms_entry]	;@012c
	xchg	ax,bx	;@0130
	dec	bx	;@0131
	jz	short @@done	;@0132
	xchg	ax,dx	;@0134
	mov	dh,dl	;@0135
@@done:
	pop	di	;@0137
	pop	si	;@0138
	pop	es	;@0139
	pop	ds	;@013a
	mov	sp,bp	;@013b
	pop	bp	;@013d
	retf	;@013e
; ---------------------------------------------------------------------------
; xms_umb_free - UMB freigeben
; Ein:  [bp+6] Segment
; Aus:  DX = Fehler << 8. Unbenutzt.
; ---------------------------------------------------------------------------
F_R39_013F	label	far	; Linkname von xms_umb_free
xms_umb_free:
	push	bp	;@013f
	mov	bp,sp	;@0140
	push	ds	;@0142
	push	es	;@0143
	push	si	;@0144
	push	di	;@0145
	mov	ah,XMS_UMB_FREE	;@0146
	mov	dx,[bp+6]	;@0148
	call	dword ptr ds:[g_xms_entry]	;@014b
	xor	dx,dx	;@014f
	dec	ax	;@0151
	jz	short @@done	;@0152
	mov	dh,bl	;@0154
@@done:
	pop	di	;@0156
	pop	si	;@0157
	pop	es	;@0158
	pop	ds	;@0159
	mov	sp,bp	;@015a
	pop	bp	;@015c
	retf	;@015d
; ---------------------------------------------------------------------------
; xms_local_a20_on - A20 lokal einschalten (XMS 05h)
; Ein:  -
; Aus:  DX = Fehler << 8
; O096 vor der 3D-Ansicht (Texturen ueber Flat Real Mode jenseits 1 MB).
; ---------------------------------------------------------------------------
F_R39_015E	label	far	; Linkname von xms_local_a20_on
xms_local_a20_on:
	push	bp	;@015e
	mov	bp,sp	;@015f
	push	ds	;@0161
	push	es	;@0162
	push	si	;@0163
	push	di	;@0164
	mov	ah,XMS_LOCAL_A20_ON	;@0165
	call	dword ptr ds:[g_xms_entry]	;@0167
	xor	dx,dx	;@016b
	dec	ax	;@016d
	jz	short @@done	;@016e
	mov	dh,bl	;@0170
@@done:
	pop	di	;@0172
	pop	si	;@0173
	pop	es	;@0174
	pop	ds	;@0175
	mov	sp,bp	;@0176
	pop	bp	;@0178
	retf	;@0179
; ---------------------------------------------------------------------------
; xms_local_a20_off - A20 lokal ausschalten (XMS 06h)
; Ein:  -
; Aus:  DX = Fehler << 8
; O096 beim Verlassen der 3D-Ansicht.
; ---------------------------------------------------------------------------
F_R39_017A	label	far	; Linkname von xms_local_a20_off
xms_local_a20_off:
	push	bp	;@017a
	mov	bp,sp	;@017b
	push	ds	;@017d
	push	es	;@017e
	push	si	;@017f
	push	di	;@0180
	mov	ah,XMS_LOCAL_A20_OFF	;@0181
	call	dword ptr ds:[g_xms_entry]	;@0183
	xor	dx,dx	;@0187
	dec	ax	;@0189
	jz	short @@done	;@018a
	mov	dh,bl	;@018c
@@done:
	pop	di	;@018e
	pop	si	;@018f
	pop	es	;@0190
	pop	ds	;@0191
	mov	sp,bp	;@0192
	pop	bp	;@0194
	retf	;@0195
; ---------------------------------------------------------------------------
; xms_a20_state - A20-Zustand abfragen (XMS 07h)
; Ein:  -
; Aus:  AX = 1 an, 0 aus; DX = Fehler << 8. Unbenutzt.
; ---------------------------------------------------------------------------
F_R39_0196	label	far	; Linkname von xms_a20_state
xms_a20_state:
	push	bp	;@0196
	mov	bp,sp	;@0197
	push	ds	;@0199
	push	es	;@019a
	push	si	;@019b
	push	di	;@019c
	mov	ah,XMS_QUERY_A20	;@019d
	call	dword ptr ds:[g_xms_entry]	;@019f
	xor	dx,dx	;@01a3
	or	ax,ax	;@01a5
	jnz	short @@done	;@01a7
	mov	dh,bl	;@01a9
@@done:
	pop	di	;@01ab
	pop	si	;@01ac
	pop	es	;@01ad
	pop	ds	;@01ae
	mov	sp,bp	;@01af
	pop	bp	;@01b1
	retf	;@01b2
SEG039_TEXT	ends
	end

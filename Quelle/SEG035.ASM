;===========================================================================
; SEG035.ASM - Flat Real Mode ("Unreal Mode") fuer Texturen im XMS
; Sternenschweif (DSA 2), Attic 1994 - Rekonstruktion, bytegleich mit
; SCHWEIF.EXE C1.02 (TASM 3.1 /ml)
;
; Inhalt:    flat_enter schaltet kurz in den Protected Mode, laedt DS, ES,
;            FS, GS und SS mit einem 4-GB-Datendeskriptor und kehrt in den
;            Real Mode zurueck: die Segmentgrenzen bleiben 4 GB, 32-Bit-
;            Adressen ueber 1 MB werden im Real Mode moeglich (gesperrter
;            XMS-Block, xms_lock). flat_leave stellt die 64-KB-Grenzen wieder
;            her. flat_copy kopiert zwischen linearen Adressen.
;            GDT und Arbeitszellen liegen im Codesegment (Offsets 0Eh..42h).
;            Die beiden far-Spruenge zurueck in den Real Mode werden zur
;            Laufzeit mit CS gepatcht (selbstmodifizierender Code).
; Aufrufer:  O010, O014, O046, O096 (3D-Ansicht mit Texturen im XMS),
;            SEG038 xms_move_any (flat_copy); far
; Aufruf:    far; flat_enter/flat_leave ohne Parameter und ohne Stackrahmen,
;            flat_copy mit enter und C-Konvention.
; Register:  flat_enter/flat_leave sichern alle Register (pushad, Segment-
;            register); flat_copy sichert DS, ES, ESI, EDI, ECX.
; Daten:     nur im Codesegment (s. u.).
; Herkunft:  aus SCHWEIF.EXE R35 (ab +0xe) mit asm2tasm.py zerlegt;
;            ;@nnnn = Offset im Segment R35.
;===========================================================================
	.386p
	include	..\names\inc\s_fix.inc	; Schalter der Fehlerbehebungen (FIX_nnnn, tools/mkfix.py)
; Arbeitszellen im Codesegment (Offsets relativ zum Segmentrahmen R35; der
; Code adressiert sie ueber DS = Codesegment)
FR_GDTR		equ	0eh	; dw Grenze, dd Basis (lgdt)
FR_GDT		equ	16h	; GDT: 5 Deskriptoren
FR_DESC_CODE4G	equ	1eh	; Selektor 08h: Code, Basis = CS (flat_enter)
FR_DESC_DATA4G	equ	26h	; Selektor 10h: Daten, Grenze 4 GB
FR_DESC_DATA64K	equ	2eh	; Selektor 18h: Daten, Grenze 64 KB
FR_DESC_CODE64K	equ	36h	; Selektor 20h: Code, Basis = CS (flat_leave)
FR_SAVE_SS	equ	3eh	; gesichertes SS
FR_SAVE_SP	equ	40h	; gesichertes SP
FR_SAVE_CMOS	equ	42h	; gesicherter Wert von Port 70h (NMI-Maske)
FR_JMP1_SEG	equ	0e0h	; Segmentwort im far jmp von flat_enter (00dd)
FR_JMP2_SEG	equ	0190h	; Segmentwort im far jmp von flat_leave (018d)
	locals
SEG035_TEXT	segment byte public use16 'CODE'
	assume	cs:SEG035_TEXT, ds:nothing
	public	F_R35_0043
	public	F_R35_0101
	public	F_R35_01AE
; ---------------------------------------------------------------------------
; Arbeitszellen und GDT im Codesegment (Offset 0Eh..42h des Segmentrahmens).
; Deskriptor: Grenze 0..15, Basis 0..23, Zugriff, Flags/Grenze 16..19, Basis 24..31
; ---------------------------------------------------------------------------
	dw	0	;@000e FR_GDTR: Grenze (wird auf 27h gesetzt)
	dd	0	;@0010 FR_GDTR+2: lineare Basis der GDT
	dw	0	;@0014 (Fuellwort)
	dw	0,0,0,0	;@0016 FR_GDT: Nulldeskriptor
	db	0ffh,0ffh,0,0,0,09ah,08fh,0	;@001e Selektor 08h: Code 16 Bit, Basis = CS (eingetragen)
	db	0ffh,0ffh,0,0,0,092h,08fh,0	;@0026 Selektor 10h: Daten, Grenze 4 GB (G = 1)
	db	0ffh,0ffh,0,0,0,092h,0,0	;@002e Selektor 18h: Daten, Grenze 64 KB
	db	0ffh,0ffh,0,0,0,09ah,0,0	;@0036 Selektor 20h: Code 16 Bit, 64 KB
	dw	0	;@003e FR_SAVE_SS
	dw	0	;@0040 FR_SAVE_SP
	db	0	;@0042 FR_SAVE_CMOS
; ---------------------------------------------------------------------------
; flat_enter - Flat Real Mode einschalten
; Ein:  -
; Aus:  AX = 1 eingeschaltet, 0 nicht moeglich (CPU schon im Protected/V86-
;       Modus, z. B. unter EMM386: smsw meldet PE)
; Ablauf: NMI sperren (Port 70h Bit 7), Basen der Deskriptoren 08h (CS) und
; 10h (DS) eintragen, GDTR laden, PE setzen, far jmp 08h:00c8, Segment-
; register mit 10h (4 GB) laden, PE loeschen, far jmp CS:00e2 (Segment
; zur Laufzeit eingetragen), SS:SP und NMI-Maske zurueck.
; Die Aufrufer werten das Ergebnis nicht aus (s. Fehler #306).
; ---------------------------------------------------------------------------
F_R35_0043	label	far	; Linkname von flat_enter
flat_enter:
	smsw	ax	;@0043 PE-Bit: schon Protected/V86?
	and	ax,1	;@0046
	jz	@@real_mode	;@0049
	mov	ax,0	;@004d
	retf	;@0050
@@real_mode:
	pushad	;@0051
	push	ds	;@0053
	push	es	;@0054
	push	fs	;@0055
	push	gs	;@0057
	db	0b8h	; mov ax,0x1af4  (Relokation)	;@0059 mov ax,SEG035_TEXT (Relokation)
	dw	SEG035_TEXT	;@0059
	mov	ds,ax	;@005c
	cli	;@005e
	mov	ds:[FR_SAVE_SS],ss	;@005f
	mov	ds:[FR_SAVE_SP],sp	;@0063 SS:SP sichern
	cli	;@0067
	in	al,070h	;@0068 NMI sperren
	mov	ds:[FR_SAVE_CMOS],al	;@006a
	or	al,080h	;@006d
	out	070h,al	;@006f
	mov	ax,cs	;@0071
	mov	cs:[FR_JMP1_SEG],ax	;@0073 Segment des far jmp bei 00dd eintragen
	movzx	eax,ax	;@0077
	shl	eax,4	;@007b
	mov	ds:[FR_DESC_CODE4G+2],ax	;@007f Basis Code-Deskriptor = CS * 16
	ror	eax,010h	;@0082
	mov	ds:[FR_DESC_CODE4G+4],al	;@0086
	mov	ax,ds	;@0089
	movzx	eax,ax	;@008b
	shl	eax,4	;@008f
	push	eax	;@0093
	mov	ds:[FR_DESC_DATA4G+2],ax	;@0095 Basis Daten-Deskriptor = DS * 16
	ror	eax,010h	;@0098
	mov	ds:[FR_DESC_DATA4G+4],al	;@009c
	pop	eax	;@009f
	xor	ebx,ebx	;@00a1 GDT-Basis = DS * 16 + FR_GDT
	mov	bx,FR_GDT	;@00a4
	add	eax,ebx	;@00a7
	mov	ds:[FR_GDTR+2],eax	;@00aa
	mov	word ptr ds:[FR_GDTR],027h	;@00ae GDT-Grenze 27h = 5 Deskriptoren
	lgdt	ds:[FR_GDTR]	;@00b4
	mov	eax,cr0	;@00b9
	or	eax,1	;@00bc Protected Mode ein
	mov	cr0,eax	;@00c0
	db	0eah,0c8h,0,8,0	; jmp 0x8:0xc8	;@00c3 jmp 08h:00c8 (Code-Deskriptor)
	mov	ax,010h	;@00c8 Selektor 10h: Grenze 4 GB
	mov	ds,ax	;@00cb
	mov	es,ax	;@00cd
	mov	fs,ax	;@00cf
	mov	gs,ax	;@00d1
	mov	ss,ax	;@00d3
	mov	eax,cr0	;@00d5 Protected Mode aus
	and	al,0feh	;@00d8
	mov	cr0,eax	;@00da
	db	0eah,0e2h,0,0,0	; jmp 0x0:0xe2	;@00dd jmp CS:00e2 (Segment bei 0073 eingetragen)
	db	0b8h	; mov ax,0x1af4  (Relokation)	;@00e2 mov ax,SEG035_TEXT (Relokation)
	dw	SEG035_TEXT	;@00e2
	mov	ds,ax	;@00e5
	mov	ss,ds:[FR_SAVE_SS]	;@00e7 SS:SP zurueck
	mov	sp,ds:[FR_SAVE_SP]	;@00eb
	mov	al,ds:[FR_SAVE_CMOS]	;@00ef NMI-Maske zurueck
	out	070h,al	;@00f2
	sti	;@00f4
	pop	gs	;@00f5
	pop	fs	;@00f7
	pop	es	;@00f9
	pop	ds	;@00fa
	popad	;@00fb
	mov	ax,1	;@00fd
	retf	;@0100
; ---------------------------------------------------------------------------
; flat_leave - Flat Real Mode ausschalten (64-KB-Grenzen herstellen)
; Ein:  -
; Aus:  -
; Wie flat_enter, aber mit den Deskriptoren 20h (Code) und 18h (Daten,
; Grenze 64 KB). Prueft den Prozessorzustand nicht: unter einem V86-Monitor
; loest mov eax,cr0 eine Ausnahme aus (Fehler #306).
; ---------------------------------------------------------------------------
F_R35_0101	label	far	; Linkname von flat_leave
flat_leave:
	pushad	;@0101
	push	ds	;@0103
	push	es	;@0104
	push	fs	;@0105
	push	gs	;@0107
	db	0b8h	; mov ax,0x1af4  (Relokation)	;@0109 mov ax,SEG035_TEXT (Relokation)
	dw	SEG035_TEXT	;@0109
	mov	ds,ax	;@010c
	cli	;@010e
	mov	ds:[FR_SAVE_SS],ss	;@010f
	mov	ds:[FR_SAVE_SP],sp	;@0113
	cli	;@0117
	in	al,070h	;@0118
	mov	ds:[FR_SAVE_CMOS],al	;@011a
	or	al,080h	;@011d
	out	070h,al	;@011f
	mov	ax,cs	;@0121
	mov	cs:[FR_JMP2_SEG],ax	;@0123 Segment des far jmp bei 018d eintragen
	movzx	eax,ax	;@0127
	shl	eax,4	;@012b
	mov	ds:[FR_DESC_CODE64K+2],ax	;@012f Basis Code-Deskriptor 20h = CS * 16
	ror	eax,010h	;@0132
	mov	ds:[FR_DESC_CODE64K+4],al	;@0136
	mov	ax,ds	;@0139
	movzx	eax,ax	;@013b
	shl	eax,4	;@013f
	push	eax	;@0143
	mov	ds:[FR_DESC_DATA64K+2],ax	;@0145 Basis Daten-Deskriptor 18h = DS * 16
	ror	eax,010h	;@0148
	mov	ds:[FR_DESC_DATA64K+4],al	;@014c
	pop	eax	;@014f
	xor	ebx,ebx	;@0151
	mov	bx,FR_GDT	;@0154
	add	eax,ebx	;@0157
	mov	ds:[FR_GDTR+2],eax	;@015a
	mov	word ptr ds:[FR_GDTR],027h	;@015e
	lgdt	ds:[FR_GDTR]	;@0164
	mov	eax,cr0	;@0169
	or	eax,1	;@016c
	mov	cr0,eax	;@0170
	db	0eah,078h,1,020h,0	; jmp 0x20:0x178	;@0173 jmp 20h:0178
	mov	ax,018h	;@0178 Selektor 18h: Grenze 64 KB
	mov	ds,ax	;@017b
	mov	es,ax	;@017d
	mov	fs,ax	;@017f
	mov	gs,ax	;@0181
	mov	ss,ax	;@0183
	mov	eax,cr0	;@0185
	and	al,0feh	;@0188
	mov	cr0,eax	;@018a
	db	0eah,092h,1,0,0	; jmp 0x0:0x192	;@018d jmp CS:0192 (Segment bei 0123 eingetragen)
	db	0b8h	; mov ax,0x1af4  (Relokation)	;@0192 mov ax,SEG035_TEXT (Relokation)
	dw	SEG035_TEXT	;@0192
	mov	ds,ax	;@0195
	mov	ss,ds:[FR_SAVE_SS]	;@0197
	mov	sp,ds:[FR_SAVE_SP]	;@019b
	mov	al,ds:[FR_SAVE_CMOS]	;@019f
	out	070h,al	;@01a2
	sti	;@01a4
	pop	gs	;@01a5
	pop	fs	;@01a7
	pop	es	;@01a9
	pop	ds	;@01aa
	popad	;@01ab
	retf	;@01ad
; ---------------------------------------------------------------------------
; flat_copy - Bytes zwischen linearen Adressen kopieren (Flat Real Mode)
; Ein:  [bp+6] Quellsegment, [bp+8] Quelloffset (32 Bit),
;       [bp+0ch] Zielsegment, [bp+0eh] Zieloffset (32 Bit),
;       [bp+12h] Laenge in Bytes (32 Bit)
; Aus:  -
; Quelle = seg * 16 + off, Ziel ebenso, DS = ES = 0, rep movsw mit 32-Bit-
; Adressen. Setzt flat_enter voraus.
; Originalfehler #307: Laenge / 2 Worte, ein ungerades letztes Byte wird
; nicht kopiert.
; ---------------------------------------------------------------------------
F_R35_01AE	label	far	; Linkname von flat_copy
flat_copy:
	enter	0,0	;@01ae
	push	ds	;@01b2
	push	es	;@01b3
	push	esi	;@01b4
	push	edi	;@01b6
IF FIX_0307
				; Fix #307: ECX nicht sichern (C-Aufrufer halten darin nichts), Platz fuer jnc/movsb
ELSE
	push	ecx	;@01b8
ENDIF
	sub	esi,esi	;@01ba
	mov	ds,si	;@01bd
	mov	es,si	;@01bf
	mov	si,[bp+6]	;@01c1
	shl	esi,4	;@01c4 ESI = Quellsegment * 16 ...
	add	esi,[bp+8]	;@01c8 ... + Quelloffset
	movzx	edi,word ptr [bp+0ch]	;@01cc
	shl	edi,4	;@01d1
	add	edi,[bp+0eh]	;@01d5
	mov	ecx,[bp+012h]	;@01d9 ECX = Laenge
	jecxz	short @@done	;@01dd
	shr	ecx,1	;@01e0 in Worten (ungerades Byte faellt weg, Fehler #307)
	cld	;@01e3
	db	0f3h,067h,0a5h	; rep a32 movsw	;@01e4 rep movsw mit 32-Bit-Adressen (a32)
IF FIX_0307
	jnc	short @@done		; Fix #307: gerade Laenge fertig
	db	067h,0a4h		; a32 movsb: ungerades letztes Byte
@@done:
ELSE
@@done:
	pop	ecx	;@01e7
ENDIF
	pop	edi	;@01e9
	pop	esi	;@01eb
	pop	es	;@01ed
	pop	ds	;@01ee
	leave	;@01ef
	retf	;@01f0
SEG035_TEXT	ends
	end

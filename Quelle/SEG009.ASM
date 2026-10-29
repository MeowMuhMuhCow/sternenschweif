;===========================================================================
; SEG009.ASM - Differenz zweier huge-Zeiger
; Sternenschweif (DSA 2), Attic 1994 - Rekonstruktion, bytegleich mit
; SCHWEIF.EXE C1.02 (TASM 3.1 /ml)
;
; Inhalt:    huge_ptr_diff, eine Abwandlung der Borland-Hilfsroutine fuer
;            die Differenz zweier huge-Zeiger (PSBH@), die das Ergebnis
;            zusaetzlich in EAX liefert.
; Aufrufer:  SEG004 (Klangpuffer), SEG014 (C-Code, far call mit Registern)
; Aufruf:    far, Parameter in Registern (kein Stackrahmen):
;              DX:AX = Zeiger 1 (DX = Segment, AX = Offset)
;              CX:BX = Zeiger 2 (CX = Segment, BX = Offset)
;            Rueckgabe: DX:AX = EAX = lin(Zeiger 1) - lin(Zeiger 2),
;            vorzeichenbehaftet (20-Bit-Adressen, Bit 20..31 aus CH erweitert)
; Register:  AX, BX, CX, DX, EAX veraendert; DI gesichert.
; Daten:     keine.
; Herkunft:  aus SCHWEIF.EXE R09 (ab +0x4) mit asm2tasm.py zerlegt;
;            ;@nnnn = Offset im Segment R09.
;===========================================================================
	.386p
	; keine lokalen Marken
SEG009_TEXT	segment byte public use16 'CODE'
	assume	cs:SEG009_TEXT, ds:nothing
	public	F_R09_002D
; ---------------------------------------------------------------------------
; Unbenutzte Kopie von huge_ptr_diff (gleiche Bytes, kein Einsprung, kein
; Aufrufer). Vermutlich eine zweite Fassung im Quelltext, die mitgelinkt wurde.
; ---------------------------------------------------------------------------
huge_ptr_diff_copy:
	push	di	;@0004
	mov	di,cx	;@0005
	mov	ch,dh	;@0007
	mov	cl,4	;@0009
	shl	dx,cl	;@000b
	shr	ch,cl	;@000d
	add	dx,ax	;@000f
	adc	ch,0	;@0011
	mov	ax,di	;@0014
	shl	di,cl	;@0016
	shr	ah,cl	;@0018
	add	bx,di	;@001a
	adc	ah,0	;@001c
	sub	dx,bx	;@001f
	sbb	ch,ah	;@0021
	mov	al,ch	;@0023
	cbw	;@0025
	xchg	ax,dx	;@0026
	push	dx	;@0027
	push	ax	;@0028
	pop	eax	;@0029
	pop	di	;@002b
	retf	;@002c
; ---------------------------------------------------------------------------
; huge_ptr_diff - lineare Differenz zweier huge-Zeiger
; Ein:  DX:AX = Zeiger 1, CX:BX = Zeiger 2 (je Segment:Offset)
; Aus:  DX:AX und EAX = (DX*16 + AX) - (CX*16 + BX), 32 Bit mit Vorzeichen
; Weg:  beide Zeiger auf 20-Bit-Adressen (Hochteil in CH bzw. AH) bringen,
;       subtrahieren, Hochteil per cbw erweitern, ueber den Stack nach EAX.
; ---------------------------------------------------------------------------
F_R09_002D	label	far	; Linkname von huge_ptr_diff
huge_ptr_diff:
	push	di	;@002d
	mov	di,cx	;@002e DI = Segment 2 merken
	mov	ch,dh	;@0030 CH = Segment 1 >> 8 ...
	mov	cl,4	;@0032
	shl	dx,cl	;@0034 DX = Segment 1 << 4
	shr	ch,cl	;@0036 ... >> 4 = Bits 16..19
	add	dx,ax	;@0038 + Offset 1, Uebertrag in CH
	adc	ch,0	;@003a
	mov	ax,di	;@003d AX = Segment 2
	shl	di,cl	;@003f DI = Segment 2 << 4
	shr	ah,cl	;@0041 AH = Bits 16..19 von Zeiger 2
	add	bx,di	;@0043 BX = lin(Zeiger 2) unten
	adc	ah,0	;@0045
	sub	dx,bx	;@0048 Differenz unten
	sbb	ch,ah	;@004a Differenz oben (Bits 16..23)
	mov	al,ch	;@004c
	cbw	;@004e auf 16 Bit erweitern
	xchg	ax,dx	;@004f AX = unten, DX = oben
	push	dx	;@0050
	push	ax	;@0051
	pop	eax	;@0052 EAX = DX:AX
	pop	di	;@0054
	retf	;@0055
SEG009_TEXT	ends
	end

;===========================================================================
; SEG036.ASM - EMS (Expanded Memory, LIM EMS 3.2/4.0 ueber int 67h)
; Sternenschweif (DSA 2), Attic 1994 - Rekonstruktion, bytegleich mit
; SCHWEIF.EXE C1.02 (TASM 3.1 /ml)
;
; Inhalt:    EMS_init (Treiber suchen, Seitenrahmen holen), Seiten anlegen,
;            freigeben, einblenden, freie Seiten zaehlen, Seitenzuordnung
;            sichern/herstellen, far-Zeiger normalisieren (EMS_norm_ptr,
;            auch fuer XMS-/Flat-Pfade und R27 benutzt).
;            Namen wie in Bright-Eyes (DSA 1, ems.cpp).
; Aufrufer:  EMS_norm_ptr: 22 far-Aufrufer, dazu R27 near;
;            EMS_get_num_pages_unalloced: SEG005. Alle anderen sind im Spiel
;            unbenutzt: die Auslagerung spricht int 67h aus C an (R02 EMS_map,
;            O010 u. a.). Das Modul ist wohl aus DSA 1 uebernommen.
; Aufruf:    far, C-Konvention, Stackrahmen bp, Parameter ab [bp+6]; die
;            beiden internen Hilfen EMS_installed und EMS_get_frame_ptr sind
;            near (push cs / call near, ret).
;            Rueckgabe der int-67h-Aufrufe: Status in AH; die Routinen
;            liefern 0 bzw. den Wert aus BX/DX, bei Fehler 0.
; Register:  DS, ES, SI, DI gesichert; AX, BX, CX, DX veraendert.
; Daten:     g_ems_frame_ptr (far-Zeiger auf den Seitenrahmen, Stueck DMR36,
;            ds:869a), g_str_emmxxxx0 ("EMMXXXX0", ds:8692; ohne eigenes
;            Public, liegt im Stueck DMR27 am Ende von _g_8678, gehoert
;            inhaltlich zu diesem Modul).
; Herkunft:  aus SCHWEIF.EXE R36 (ab +0x2) mit asm2tasm.py zerlegt;
;            ;@nnnn = Offset im Segment R36.
;===========================================================================
	.386p
; Daten im Datensegment; Adressen als Konstanten (Code ohne Fixups wie im Original)
g_str_emmxxxx0	equ	08692h	; db 'EMMXXXX0' Geraetename des EMS-Treibers
g_ems_frame_ptr	equ	0869ah	; dd  Seitenrahmen (Offset 0, Segment aus int 67h/41h); das Spiel selbst
			; benutzt g_ems_frame (ds:b16c, R02)
EMM_NAME_OFS	equ	0ah	; Geraetename im Kopf des Geraetetreibers
	locals
	extrn	F_LXLSH@:far
SEG036_TEXT	segment word public use16 'CODE'
	assume	cs:SEG036_TEXT, ds:nothing
	public	F_R36_002B
	public	F_R36_0049
	public	F_R36_0065
	public	F_R36_0084
	public	F_R36_009D
	public	F_R36_00BC
	public	F_R36_00F1
	public	F_R36_0117
	public	F_R36_012E
; ---------------------------------------------------------------------------
; EMS_installed - ist ein EMS-Treiber geladen? (near)
; Ein:  -
; Aus:  AX = 1 ja, 0 nein
; Holt den int-67h-Vektor (int 21h/35h) und vergleicht ES:000A mit
; "EMMXXXX0". cx = 9 vergleicht ein Byte zu viel; das neunte Byte
; (hinter dem Namen) entscheidet aber nicht mit: loope endet mit cx = 0.
; ---------------------------------------------------------------------------
EMS_installed:
	push	bp	;@0002
	mov	bp,sp	;@0003
	push	ds	;@0005
	push	es	;@0006
	push	si	;@0007
	push	di	;@0008
	mov	ax,03567h	;@0009 ES:BX = Vektor int 67h
	int	021h	;@000c
	mov	di,EMM_NAME_OFS	;@000e
	mov	si,g_str_emmxxxx0	;@0011
	mov	cx,9	;@0014
@@compare:
	cmpsb	;@0017 Geraetenamen vergleichen
	loope	@@compare	;@0018
	xor	ax,ax	;@001a
	or	cx,cx	;@001c
	jnz	short @@done	;@001e
	mov	ax,1	;@0020
@@done:
	pop	di	;@0023
	pop	si	;@0024
	pop	es	;@0025
	pop	ds	;@0026
	mov	sp,bp	;@0027
	pop	bp	;@0029
	ret	;@002a
; ---------------------------------------------------------------------------
; EMS_get_frame_ptr - Seitenrahmen holen (near, ret; Linkname trotzdem far)
; Ein:  -
; Aus:  DX:AX = Segment des Seitenrahmens:0 (bei Fehler 0:0)
; int 67h, AH = 41h (Get Page Frame Address).
; ---------------------------------------------------------------------------
F_R36_002B	label	far	; Linkname von EMS_get_frame_ptr
EMS_get_frame_ptr:
	push	bp	;@002b
	mov	bp,sp	;@002c
	push	ds	;@002e
	push	es	;@002f
	push	si	;@0030
	push	di	;@0031
	mov	ax,04100h	;@0032
	int	067h	;@0035
	or	ah,ah	;@0037
	jz	short @@done	;@0039
	xor	bx,bx	;@003b
@@done:
	mov	dx,bx	;@003d
	xor	ax,ax	;@003f
	pop	di	;@0041
	pop	si	;@0042
	pop	es	;@0043
	pop	ds	;@0044
	mov	sp,bp	;@0045
	pop	bp	;@0047
	ret	;@0048
; ---------------------------------------------------------------------------
; EMS_get_num_pages_unalloced - Zahl der freien 16-KB-Seiten
; Ein:  -
; Aus:  AX = freie Seiten (bei Fehler 0)
; int 67h, AH = 42h (Get Unallocated Page Count).
; ---------------------------------------------------------------------------
F_R36_0049	label	far	; Linkname von EMS_get_num_pages_unalloced
EMS_get_num_pages_unalloced:
	push	bp	;@0049
	mov	bp,sp	;@004a
	push	ds	;@004c
	push	es	;@004d
	push	si	;@004e
	push	di	;@004f
	mov	ax,04200h	;@0050
	int	067h	;@0053
	or	ah,ah	;@0055
	jz	short @@done	;@0057
	xor	bx,bx	;@0059
@@done:
	mov	ax,bx	;@005b
	pop	di	;@005d
	pop	si	;@005e
	pop	es	;@005f
	pop	ds	;@0060
	mov	sp,bp	;@0061
	pop	bp	;@0063
	retf	;@0064
; ---------------------------------------------------------------------------
; EMS_alloc_pages - Seiten anlegen
; Ein:  [bp+6] Zahl der Seiten
; Aus:  AX = Handle (bei Fehler 0)
; int 67h, AH = 43h (Allocate Pages). Unbenutzt (die Spielmodule rufen
; den Treiber selbst).
; ---------------------------------------------------------------------------
F_R36_0065	label	far	; Linkname von EMS_alloc_pages
EMS_alloc_pages:
	push	bp	;@0065
	mov	bp,sp	;@0066
	push	ds	;@0068
	push	es	;@0069
	push	si	;@006a
	push	di	;@006b
	mov	ax,04300h	;@006c
	mov	bx,[bp+6]	;@006f
	int	067h	;@0072
	or	ah,ah	;@0074
	jz	short @@done	;@0076
	xor	dx,dx	;@0078
@@done:
	mov	ax,dx	;@007a
	pop	di	;@007c
	pop	si	;@007d
	pop	es	;@007e
	pop	ds	;@007f
	mov	sp,bp	;@0080
	pop	bp	;@0082
	retf	;@0083
; ---------------------------------------------------------------------------
; EMS_free_pages - Handle mit seinen Seiten freigeben
; Ein:  [bp+6] Handle
; Aus:  AL = Status (0 = in Ordnung)
; int 67h, AH = 45h (Deallocate Pages). Unbenutzt.
; ---------------------------------------------------------------------------
F_R36_0084	label	far	; Linkname von EMS_free_pages
EMS_free_pages:
	push	bp	;@0084
	mov	bp,sp	;@0085
	push	ds	;@0087
	push	es	;@0088
	push	si	;@0089
	push	di	;@008a
	mov	ax,04500h	;@008b
	mov	dx,[bp+6]	;@008e
	int	067h	;@0091
	mov	al,ah	;@0093
	pop	di	;@0095
	pop	si	;@0096
	pop	es	;@0097
	pop	ds	;@0098
	mov	sp,bp	;@0099
	pop	bp	;@009b
	retf	;@009c
; ---------------------------------------------------------------------------
; EMS_map_memory - logische Seite in den Seitenrahmen einblenden
; Ein:  [bp+6] Handle, [bp+8] logische Seite, [bp+0ah] physische Seite 0..3
; Aus:  AL = Status (0 = in Ordnung)
; int 67h, AH = 44h (Map Handle Page). Unbenutzt.
; ---------------------------------------------------------------------------
F_R36_009D	label	far	; Linkname von EMS_map_memory
EMS_map_memory:
	push	bp	;@009d
	mov	bp,sp	;@009e
	push	ds	;@00a0
	push	es	;@00a1
	push	si	;@00a2
	push	di	;@00a3
	mov	ax,04400h	;@00a4
	mov	al,[bp+0ah]	;@00a7
	mov	bx,[bp+8]	;@00aa
	mov	dx,[bp+6]	;@00ad
	int	067h	;@00b0
	mov	al,ah	;@00b2
	pop	di	;@00b4
	pop	si	;@00b5
	pop	es	;@00b6
	pop	ds	;@00b7
	mov	sp,bp	;@00b8
	pop	bp	;@00ba
	retf	;@00bb
; ---------------------------------------------------------------------------
; EMS_norm_ptr - far-Zeiger normalisieren (Offset 0..15)
; Ein:  [bp+6] Offset, [bp+8] Segment
; Aus:  DX:AX = normalisierter Zeiger
; Rechnet lin = seg * 16 + off (32 Bit ueber LXLSH@ der Borland-Laufzeit),
; DX = lin >> 4 (nach << 12 der obere Teil), AX = off & 0fh.
; 22 far-Aufrufer, dazu R27 near ueber den gemeinsamen Codeabschnitt.
; ---------------------------------------------------------------------------
F_R36_00BC	label	far	; Linkname von EMS_norm_ptr
EMS_norm_ptr:
	push	bp	;@00bc
	mov	bp,sp	;@00bd
	push	ds	;@00bf
	push	es	;@00c0
	push	si	;@00c1
	push	di	;@00c2
	xor	dx,dx	;@00c3 DX:AX = Segment
	mov	ax,[bp+8]	;@00c5
	mov	cx,4	;@00c8
	call	far ptr F_LXLSH@	;@00cb << 4
	mov	word ptr [bp+8],0	;@00d0 Segment-Platz als Hochteil 0 benutzen
	add	ax,[bp+6]	;@00d5 + Offset = lineare Adresse
	adc	dx,[bp+8]	;@00d8
	mov	cx,0ch	;@00db
	call	far ptr F_LXLSH@	;@00de << 12: DX = lin >> 4
	mov	ax,[bp+6]	;@00e3
	and	ax,0fh	;@00e6 AX = Offset & 0fh
	pop	di	;@00e9
	pop	si	;@00ea
	pop	es	;@00eb
	pop	ds	;@00ec
	mov	sp,bp	;@00ed
	pop	bp	;@00ef
	retf	;@00f0
; ---------------------------------------------------------------------------
; EMS_init - EMS pruefen und Seitenrahmen merken
; Ein:  -
; Aus:  AX = 1 EMS vorhanden (g_ems_frame_ptr gesetzt), 0 nicht vorhanden
; Unbenutzt.
; ---------------------------------------------------------------------------
F_R36_00F1	label	far	; Linkname von EMS_init
EMS_init:
	push	bp	;@00f1
	mov	bp,sp	;@00f2
	push	ds	;@00f4
	push	es	;@00f5
	push	si	;@00f6
	push	di	;@00f7
	call	near ptr EMS_installed	;@00f8
	or	ax,ax	;@00fb
	jz	short @@done	;@00fd
	call	near ptr EMS_get_frame_ptr	;@00ff
	mov	ds:[g_ems_frame_ptr+2],dx	;@0102 g_ems_frame_ptr = Rahmen:0
	mov	word ptr ds:[g_ems_frame_ptr],0	;@0106
	mov	ax,1	;@010c
@@done:
	pop	di	;@010f
	pop	si	;@0110
	pop	es	;@0111
	pop	ds	;@0112
	mov	sp,bp	;@0113
	pop	bp	;@0115
	retf	;@0116
; ---------------------------------------------------------------------------
; EMS_save_page_map - Seitenzuordnung des Handles sichern
; Ein:  [bp+6] Handle
; Aus:  AH = Status
; int 67h, AH = 47h (Save Page Map). Unbenutzt.
; ---------------------------------------------------------------------------
F_R36_0117	label	far	; Linkname von EMS_save_page_map
EMS_save_page_map:
	push	bp	;@0117
	mov	bp,sp	;@0118
	push	ds	;@011a
	push	es	;@011b
	push	si	;@011c
	push	di	;@011d
	mov	ax,04700h	;@011e
	mov	dx,[bp+6]	;@0121
	int	067h	;@0124
	pop	di	;@0126
	pop	si	;@0127
	pop	es	;@0128
	pop	ds	;@0129
	mov	sp,bp	;@012a
	pop	bp	;@012c
	retf	;@012d
; ---------------------------------------------------------------------------
; EMS_restore_page_map - gesicherte Seitenzuordnung herstellen
; Ein:  [bp+6] Handle
; Aus:  AH = Status
; int 67h, AH = 48h (Restore Page Map). Unbenutzt.
; ---------------------------------------------------------------------------
F_R36_012E	label	far	; Linkname von EMS_restore_page_map
EMS_restore_page_map:
	push	bp	;@012e
	mov	bp,sp	;@012f
	push	ds	;@0131
	push	es	;@0132
	push	si	;@0133
	push	di	;@0134
	mov	ax,04800h	;@0135
	mov	dx,[bp+6]	;@0138
	int	067h	;@013b
	pop	di	;@013d
	pop	si	;@013e
	pop	es	;@013f
	pop	ds	;@0140
	mov	sp,bp	;@0141
	pop	bp	;@0143
	retf	;@0144
SEG036_TEXT	ends
	end

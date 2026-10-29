; ============================================================================================================
; SEG024 - 3D-Ansicht: Polygone rastern und in den Zeichenpuffer fuellen (handgeschriebener 386-Assembler)
; ============================================================================================================
;
; Aufbau der Engine (zusammen mit SEG025 und der C-Seite SEG012 draw_object):
;
;   SEG025 obj_project   Punkte eines Objekts in Kamerakoordinaten drehen, an der Ebene z = 0 abschneiden,
;                        auf den Schirm projizieren, Rueckseiten und Flaechen ausserhalb verwerfen und die
;                        sichtbaren Flaechen nach Tiefe sortiert verketten (Maleralgorithmus: fern -> nah).
;   SEG012 draw_object   je sichtbarer Flaeche (fern -> nah):  edges_clear
;                                                              poly_edges bzw. poly_edges_persp
;                                                              poly_tex_cols[_m] / poly_tex_rows[_m] / poly_flat
;   SEG024 (dieses Modul) Kantentabellen fuellen (Kantenverfolgung) und daraus die Flaeche zeichnen.
;
; Datenfluss einer Flaeche:
;
;   g_vtx_buf (x, y, z je Ecke, projiziert) --poly_edges--> Kantentabellen g_edge_* --poly_tex_*--> Zeichenpuffer
;   face3d.uv (u, v je Ecke)                --------/                                              (g_renderbuf_ptr)
;
;   Kantentabellen (DS, je VIEW_W = 208 Worte): fuer jede Spalte (bzw. Zeile, s. u.) die kleinste und groesste
;   Koordinate der Flaeche und die Werte z, u, v an diesen beiden Enden. Eine Flaeche ist konvex, also genuegen
;   zwei Enden je Spalte. edges_clear setzt lo = 7FFFh, hi = 8001h (leer).
;
; Zwei Fuellrichtungen (Parameter "horizontal" von poly_edges, in SEG012 aus FACE_TEXTURED | FACE_NEAR):
;
;   spaltenweise (horizontal = 0)  Tabellenindex = Bildschirmspalte x (0..207), Werte = y. Fuer senkrechte
;                                  Flaechen (Waende) ist die Tiefe entlang einer Spalte fest, die affine
;                                  Interpolation entlang der Spalte ist dann exakt: poly_tex_cols,
;                                  poly_tex_cols_m, poly_flat.
;   zeilenweise (horizontal = 1)   poly_edges vertauscht x und y: Index = Bildschirmzeile y (0..133), Werte = x.
;                                  Fuer waagerechte Flaechen (Boden, Decke) ist die Tiefe entlang einer Zeile
;                                  fest: poly_tex_rows, poly_tex_rows_m.
;   Die Perspektive steckt allein in den Kanten: poly_edges_persp interpoliert u, v, z entlang der Kante
;   perspektivisch korrekt (fuer nahe Flaechen, SEG012: zmin < 100 und zmax < 400), poly_edges linear.
;
; Aufrufer aus C (SEG012 draw_object; fill_scene/copy_3d_to_screen auch O010, O096, O198):
;   edges_clear()  poly_edges(n, idx, vtx, uv, horizontal)  poly_edges_persp(dieselben)
;   poly_tex_cols(shade)  poly_tex_cols_m(shade)  poly_tex_rows(shade)  poly_tex_rows_m(shade)
;   poly_flat(shade, color)  fill_scene(color)  copy_3d_to_screen()
;   Alle far, Parameter auf dem Stapel (C-Konvention, Aufrufer raeumt ab). Die Linknamen bleiben F_R24_xxxx
;   (die Aufrufer in der Assembler-Linkfassung benutzen sie), der sprechende Name ist der Prozedurname.
;
; Festkomma-Formate:
;   Kantenverfolgung linear (edge_trace): Schritte 16.16, der ganze Teil als Wort, vom Bruchteil nur das obere
;     Byte (8 Bit) in Byte-Akkus, Start 80h (= 0,5, rundet). Weiter per add/adc.
;   Kantenverfolgung perspektivisch (edge_trace_persp): Zaehler und Nenner << 14 als Doppelworte,
;     Nenner w = z + g_proj_dist (Augabstand der Projektion), Wert = Zaehler / Nenner + Anfangswert.
;   Texturschritt in den Fuellroutinen: u mit 8-Bit-Bruchteil in AL (Schritt CL) und ganzem Schritt in EDX,
;     v nach Bresenham: ganzer Zeilenschritt (v-Zeilen * Texturbreite) in EBP, 8-Bit-Bruchteil in AH (Schritt
;     CH), beim Uebertrag eine Texturzeile mehr (tx_vwrap = +-Breite). ESI = Texeladresse.
;   Schattierung: FS = Segment einer von 32 Farbumsetztabellen (g_shade_segs), Stufe = g_fog_table[z / 16]
;     + Helligkeit des Aufrufers (0..7), hoechstens 31. Farbe 0 ist bei den _m-Varianten durchsichtig.
;
; Bildschirmpuffer: g_renderbuf_ptr zeigt auf einen Puffer mit 320 Bytes je Zeile, das 3D-Fenster ist 208 x 134
;   Pixel gross und liegt ab Versatz VIEW_OFS (0). copy_3d_to_screen kopiert es nach A000:028F (Zeile 2,
;   Spalte 15 des VGA-Bildschirms 320 x 200).
;
; Selbstveraendernder Code: Die Texturen liegen entweder im EMS-Fenster (16-Bit-Adresse) oder ohne EMS im
;   Flat Real Mode (SEG035) an einer linearen 32-Bit-Adresse. Die Schritte lesen das Texel mit "mov bl,[esi]"
;   (67h 8Ah 1Eh). Ist g_ems_avail gesetzt, ueberschreibt jede Fuellroutine beim ersten Aufruf alle ihre
;   Schritte mit "mov bl,ds:[si]" (3Eh 8Ah 1Ch, gleiche Laenge). Die Sprungtabelle der Routine liefert die
;   Adressen.
;
; Abgerollte Schleifen: jede Textur-Fuellroutine enthaelt ihren Pixelschritt 134- (Spalte) bzw. 208-mal
;   hintereinander (Makro rept). Fuer n Pixel springt sie per "push adresse / retn" auf Schritt VIEW_H - n bzw.
;   VIEW_W - n, die Sprungtabelle dahinter haelt die Adressen aller Schritte.
;
; Rekonstruktion: aus SCHWEIF.EXE R24 (asm2tasm.py), lesbar gemacht 19.10.2026 (Auftrag C), bytegleich mit
;   TASM 3.1 /ml. Hinweise zur Bytegleichheit:
;   - Vorwaertsspruenge erzeugt TASM (ein Durchlauf) lang und fuellt kurze mit NOPs auf (Bytes wie im Original).
;   - Variablen im Codesegment und Sprungtabellen werden als feste Versaetze angesprochen (Konstanten, keine
;     Fixups). Sprungtabellen werden vor ihrer Definition benutzt, das Makro AT prueft ihre Lage.
;   - "align 4" und "dup" erzeugen LIDATA (tools/objorder.py kennt nur LEDATA), daher PAD4 und rept.
;   - "ret" in einer far-Prozedur waere RETF; der Sprung in die abgerollte Schleife ist "retn".
; ============================================================================================================
	.386p
	locals

; ---- Konstanten ----------------------------------------------------------------------------------------------
SCREEN_W	equ	320		; Bytes je Zeile im Zeichenpuffer und auf dem Bildschirm
VIEW_W		equ	208		; Breite des 3D-Fensters = Eintraege je Kantentabelle
VIEW_H		equ	134		; Hoehe des 3D-Fensters
VIEW_OFS	equ	0		; Versatz des 3D-Fensters im Zeichenpuffer
VIEW_SCR_OFS	equ	2 * SCREEN_W + 15	; Lage des 3D-Fensters auf dem VGA-Bildschirm (A000:028F)
EDGE_EMPTY_LO	equ	7fffh		; leere Spalte: lo > hi
EDGE_EMPTY_HI	equ	8001h
SHADE_MAX	equ	31		; hoechste Schattierungsstufe (32 Tabellen)
FRAC_HALF	equ	80h		; Startwert der 8-Bit-Bruchteile (0,5)
OP_A32		equ	67h		; erstes Byte von "mov bl,[esi]" (Adressgroesse 32 Bit)
PATCH_W		equ	8a3eh		; "ds: mov bl,[si]" = 3Eh 8Ah 1Ch: erste zwei Bytes ...
PATCH_B		equ	1ch		; ... und das dritte

; ---- Daten im Datensegment (DGROUP, feste Adressen; Namen wie in names/inc/s_data.h) -----------------------
; Kantentabellen (Datenstueck dieses Moduls, ds:78da-85e4), je VIEW_W Worte:
g_edge_lo	equ	78dah		; kleinste Koordinate je Spalte (bzw. Zeile)       (7FFFh = leer)
g_edge_hi	equ	7a7ah		; groesste Koordinate                              (8001h = leer)
g_edge_lo_z	equ	7c1ah		; z am Ende lo (Tiefe, bestimmt den Nebel)
g_edge_hi_z	equ	7dbah		; z am Ende hi
g_edge_lo_u	equ	7f5ah		; Texturspalte u am Ende lo
g_edge_lo_v	equ	80fah		; Texturzeile v am Ende lo
g_edge_hi_u	equ	829ah		; u am Ende hi
g_edge_hi_v	equ	843ah		; v am Ende hi
g_cur_tex	equ	85dah		; struct tex_ref: Lage der aktuellen Textur (Doppelwort Offset, Wort Segment)
g_cur_tex_w	equ	85e0h		; Breite der aktuellen Textur (= Zeilenschritt im Texturspeicher)
; Daten anderer Module:
g_shade_segs	equ	85e4h		; Segmente der 32 Farbumsetztabellen zu 256 Bytes (O096)
g_fog_table	equ	8624h		; Schattierungsstufe je Tiefe z / 16 (32 Bytes, R12 fog_table)
g_proj_dist	equ	8c78h		; Augabstand der Projektion (R25 view_set_projection, 32)
g_renderbuf_ptr	equ	0b128h		; far-Zeiger auf den Zeichenpuffer (Offset, Segment)
g_ems_avail	equ	0c69ah		; EMS vorhanden: Texturen liegen im EMS-Fenster

; ---- Hilfsmakros ---------------------------------------------------------------------------------------------
; AT ofs: Abbruch, wenn die aktuelle Lage im Segment nicht ofs ist (prueft die Konstanten der Sprungtabellen).
AT	macro	ofs
	if	($ - r24_base) ne (ofs)
	.err
	endif
	endm

; PAD4: mit Fuellbefehlen auf eine durch 4 teilbare Adresse auffuellen (wie align 4, aber ohne LIDATA).
PAD4	macro
	if	(($ - r24_base) and 3) eq 1
	xchg	bx,bx
	nop
	elseif	(($ - r24_base) and 3) eq 2
	xchg	bx,bx
	elseif	(($ - r24_base) and 3) eq 3
	nop
	endif
	endm

; JT_ENT pfx,k: Eintrag k einer Sprungtabelle = Adresse des Schritts k (Symbol pfx&k aus dem Schrittmakro).
JT_ENT	macro	pfx,k
	dw	pfx&k
	endm


_DATA	segment word public 'DATA'
_DATA	ends
DGROUP	group	_DATA

SEG024_TEXT	segment byte public use16 'CODE'
	assume	cs:SEG024_TEXT, ds:nothing
	public	F_R24_0000		; Modulanfang (Variablen, keine Funktion)
	public	F_R24_0010		; edge_trace
	public	F_R24_025A		; edge_trace_persp
	public	F_R24_04F3		; poly_edges
	public	F_R24_0560		; poly_edges_persp
	public	F_R24_05CD		; edges_clear
	public	F_R24_05F6		; fill_scene
	public	F_R24_0627		; copy_3d_to_screen
	public	F_R24_0668		; poly_tex_cols
	public	F_R24_198A		; poly_tex_cols_m
	public	F_R24_2FCE		; poly_tex_rows
	public	F_R24_4B29		; poly_tex_rows_m
	public	F_R24_6C35		; poly_flat

; ============================================================================================================
; Variablen der Kantenverfolgung (im Codesegment, CS-relativ)
; ============================================================================================================
F_R24_0000	label	far
r24_base	label	byte		; Segmentanfang: Bezug fuer alle CS-Versaetze
et_du		=	$ - r24_base
		dw	0		; ganzzahliger u-Schritt je Spalte
et_dv		=	$ - r24_base
		dw	0		; ganzzahliger v-Schritt
et_dpos		=	$ - r24_base
		dw	0		; ganzzahliger Schritt der eingetragenen Koordinate (y bzw. x)
et_count	=	$ - r24_base
		dw	0		; verbleibende Tabellenplaetze (auch edge_trace_persp)
et_dz		=	$ - r24_base
		dw	0		; ganzzahliger z-Schritt
et_z		=	$ - r24_base
		dw	0		; laufendes z
et_dz_f		=	$ - r24_base
		db	0		; Bruchteil des z-Schritts (8 Bit)
et_pos_f	=	$ - r24_base
		db	0		; Bruchteil-Akku der Koordinate
et_u_f		=	$ - r24_base
		db	0		; Bruchteil-Akku u
et_z_f		=	$ - r24_base
		db	0		; Bruchteil-Akku z  (der v-Akku ist DH, die Schritte CL/CH/DL)

; ============================================================================================================
; edge_trace - eine Kante linear in die Kantentabellen eintragen
; ------------------------------------------------------------------------------------------------------------
; Aufruf:  push cs / call near (aus poly_edges), far-Rahmen, Parameter [bp+6..]:
;            a = (x0, y0, z0), b = (x1, y1, z1)   Ecken (projiziert, x/y Bildschirm, z Tiefe)
;            (u0, v0), (u1, v1)                   Texturkoordinaten der Ecken
;            horizontal                           != 0: x und y tauschen (zeilenweise Tabellen)
; Aufgabe: fuer jede Tabellenspalte x zwischen x0 und x1 den Punkt der Kante (y, z, u, v) bestimmen und in
;          g_edge_lo/_hi eintragen, wenn y kleiner bzw. groesser als der bisherige Eintrag ist. Spalten
;          links ausserhalb werden nur mitgerechnet, rechts ausserhalb beendet die Kante.
; Rechnung: Schritt = Differenz / (Spaltenzahl) in 16.16, Bruchteile 8 Bit, Akkus starten bei 0,5. Der Teiler
;          ist die Zahl der Spalten, nicht die der Abstaende: der Endwert wird knapp nicht erreicht (die
;          Textur wird so nicht ueber ihren Rand hinaus gelesen).
; Register: zerstoert EAX, EBX, ECX, DX; SI/DI/BP gesichert.
; ============================================================================================================
et_x0		equ	<[bp+6]>	; nach dem Tausch: Tabellenindex (Spalte bzw. Zeile)
et_y0		equ	<[bp+8]>	; nach dem Tausch: eingetragene Koordinate
et_z0		equ	<[bp+0ah]>
et_x1		equ	<[bp+0ch]>
et_y1		equ	<[bp+0eh]>
et_z1		equ	<[bp+10h]>
et_u0		equ	<[bp+12h]>	; (u0, v0) als Doppelwort
et_v0		equ	<[bp+14h]>
et_u1		equ	<[bp+16h]>	; (u1, v1)
et_v1		equ	<[bp+18h]>
et_horiz	equ	<[bp+1ah]>

edge_trace	proc	far
F_R24_0010	label	far
	enter	0,0
	push	si
	push	di
	cmp	word ptr et_horiz,0
	jz	@@ordered
	mov	ax,et_x0		; waagerechte Flaeche: x <-> y in beiden Ecken
	mov	bx,et_y0
	mov	et_y0,ax
	mov	et_x0,bx
	mov	ax,et_x1
	mov	bx,et_y1
	mov	et_y1,ax
	mov	et_x1,bx
@@ordered:
	cmp	word ptr et_x0,0	; ganz links ausserhalb?
	jnl	@@not_left
	cmp	word ptr et_x1,0
	jnl	@@not_left
	pop	di
	pop	si
	leave
	retf
@@not_left:
	cmp	word ptr et_x0,VIEW_W	; ganz rechts ausserhalb?
	jl	@@visible
	cmp	word ptr et_x1,VIEW_W
	jl	@@visible
	pop	di
	pop	si
	leave
	retf
@@visible:
	mov	ax,et_x0		; Ecken so ordnen, dass x0 <= x1
	cmp	ax,et_x1
	jng	@@sorted
	mov	eax,et_x0		; (x, y) als Doppelwort tauschen
	xchg	eax,et_x1
	mov	et_x0,eax
	mov	ax,et_z0
	xchg	ax,et_z1
	mov	et_z0,ax
	mov	eax,et_u0		; (u, v) tauschen
	xchg	eax,et_u1
	mov	et_u0,eax
@@sorted:
	mov	bx,et_x1
	sub	bx,et_x0
	jnz	@@slope
	; -- senkrechte Kante (eine Spalte): beide Ecken eintragen
	mov	si,et_x0
	shl	si,1
	cmp	si,VIEW_W * 2
	jnc	@@vert_b
	mov	ax,et_y0
	cmp	[si+g_edge_lo],ax
	jng	@@vert_a_hi
	mov	[si+g_edge_lo],ax
	mov	bx,et_z0
	mov	[si+g_edge_lo_z],bx
	mov	bx,et_u0
	mov	[si+g_edge_lo_u],bx
	mov	bx,et_v0
	mov	[si+g_edge_lo_v],bx
@@vert_a_hi:
	cmp	[si+g_edge_hi],ax
	jnl	@@vert_b
	mov	[si+g_edge_hi],ax
	mov	bx,et_z0
	mov	[si+g_edge_hi_z],bx
	mov	bx,et_u0
	mov	[si+g_edge_hi_u],bx
	mov	bx,et_v0
	mov	[si+g_edge_hi_v],bx
@@vert_b:
	cmp	si,VIEW_W * 2
	jnc	@@vert_done
	mov	ax,et_y1
	cmp	[si+g_edge_lo],ax
	jng	@@vert_b_hi
	mov	[si+g_edge_lo],ax
	mov	bx,et_z1
	mov	[si+g_edge_lo_z],bx
	mov	bx,et_u1
	mov	[si+g_edge_lo_u],bx
	mov	bx,et_v1
	mov	[si+g_edge_lo_v],bx
@@vert_b_hi:
	cmp	[si+g_edge_hi],ax
	jnl	@@vert_done
	mov	[si+g_edge_hi],ax
	mov	bx,et_z1
	mov	[si+g_edge_hi_z],bx
	mov	bx,et_u1
	mov	[si+g_edge_hi_u],bx
	mov	bx,et_v1
	mov	[si+g_edge_hi_v],bx
@@vert_done:
	pop	di
	pop	si
	leave
	retf
	; -- schraege Kante: Schritte je Spalte (Teiler = Spaltenzahl)
@@slope:
	inc	bx
	mov	ax,bx
	cwde
	mov	ebx,eax
	mov	ax,et_y1		; Koordinate: ganzer Schritt -> et_dpos, Bruchteil -> CL
	sub	ax,et_y0
	shl	eax,10h
	cdq
	idiv	ebx
	mov	cl,ah
	sar	eax,10h
	mov	cs:[et_dpos],ax
	mov	ax,et_z1		; z: et_dz, et_dz_f
	sub	ax,et_z0
	shl	eax,10h
	cdq
	idiv	ebx
	mov	cs:[et_dz_f],ah
	sar	eax,10h
	mov	cs:[et_dz],ax
	mov	ax,et_u1		; u: et_du, CH
	sub	ax,et_u0
	shl	eax,10h
	cdq
	idiv	ebx
	mov	ch,ah
	sar	eax,10h
	mov	cs:[et_du],ax
	mov	ax,et_v1		; v: et_dv, DL
	sub	ax,et_v0
	shl	eax,10h
	cdq
	idiv	ebx
	mov	dl,ah
	sar	eax,10h
	mov	cs:[et_dv],ax
	mov	si,et_x0
	mov	ax,et_x1
	sub	ax,si
	inc	ax
	jz	@@done			; (kommt nicht vor, x1 > x0)
	mov	cs:[et_count],ax
	shl	si,1			; SI = Tabellenindex * 2
	mov	bx,et_y0		; BX = Koordinate
	mov	di,et_u0		; DI = u
	mov	ax,et_z0
	mov	cs:[et_z],ax
	push	bp
	mov	bp,et_v0		; BP = v (ab hier keine Parameter mehr)
	mov	byte ptr cs:[et_pos_f],FRAC_HALF
	mov	byte ptr cs:[et_u_f],FRAC_HALF
	mov	byte ptr cs:[et_z_f],FRAC_HALF
	mov	dh,FRAC_HALF		; DH = Bruchteil-Akku v
@@column:
	cmp	si,VIEW_W * 2
	jnl	@@right_out		; rechts aus dem Fenster: fertig
	jnc	@@step			; negativ (links ausserhalb): nur weiterrechnen
	cmp	[si+g_edge_lo],bx
	jng	@@check_hi
	mov	[si+g_edge_lo],bx
	mov	[si+g_edge_lo_z],ax
	mov	[si+g_edge_lo_u],di
	mov	[si+g_edge_lo_v],bp
@@check_hi:
	cmp	[si+g_edge_hi],bx
	jnl	@@step
	mov	[si+g_edge_hi],bx
	mov	[si+g_edge_hi_z],ax
	mov	[si+g_edge_hi_u],di
	mov	[si+g_edge_hi_v],bp
@@step:
	add	cs:[et_pos_f],cl	; Koordinate += Schritt (Bruchteil mit Uebertrag)
	adc	bx,cs:[et_dpos]
	add	cs:[et_u_f],ch		; u
	adc	di,cs:[et_du]
	add	dh,dl			; v
	adc	bp,cs:[et_dv]
	mov	al,cs:[et_dz_f]		; z
	add	cs:[et_z_f],al
	mov	ax,cs:[et_z]
	adc	ax,cs:[et_dz]
	mov	cs:[et_z],ax
	inc	si
	inc	si
	dec	word ptr cs:[et_count]
	jnz	@@column
@@right_out:
	pop	bp
@@done:
	pop	di
	pop	si
	leave
	retf
edge_trace	endp

; ============================================================================================================
; edge_trace_persp - eine Kante mit perspektivisch korrekter Interpolation eintragen
; ------------------------------------------------------------------------------------------------------------
; Aufruf und Parameter wie edge_trace (aus poly_edges_persp).
; Aufgabe: wie edge_trace, aber z, u, v laufen nicht linear in der Bildschirmspalte, sondern linear im Raum
;          (fuer nahe Flaechen, bei denen die Verzerrung sichtbar waere). Mit w = z + g_proj_dist und dem
;          Bildschirmparameter s (0..1) gilt
;              wert(s) = wert0 + (wert1 - wert0) * s * w0 / ((1 - s) * w1 + s * w0).
;          Zaehler ([bp-18h..-20h]) und Nenner ([bp-28h]) wachsen je Spalte um feste Schritte (<< 14), je Spalte
;          drei Divisionen. Die Koordinate selbst laeuft linear (16.16 in [bp-24h]).
; Register: zerstoert EAX, EBX, ECX, EDX; SI/DI gesichert.
; ============================================================================================================
ep_du_w		equ	<[bp-4]>	; Schritt Zaehler u: (u1 - u0) << 14 / n * w0
ep_dv_w		equ	<[bp-8]>	; Schritt Zaehler v
ep_dz_w		equ	<[bp-0ch]>	; Schritt Zaehler z
ep_dpos		equ	<[bp-10h]>	; Schritt der Koordinate (16.16)
ep_dw		equ	<[bp-14h]>	; Schritt des Nenners: (w0 - w1) << 14 / n
ep_num_u	equ	<[bp-18h]>	; Zaehler u
ep_num_v	equ	<[bp-1ch]>	; Zaehler v
ep_num_z	equ	<[bp-20h]>	; Zaehler z
ep_pos		equ	<[bp-24h]>	; Koordinate (16.16)
ep_den		equ	<[bp-28h]>	; Nenner, beginnt bei w1 << 14

edge_trace_persp	proc	far
F_R24_025A	label	far
	enter	44,0
	push	si
	push	di
	cmp	word ptr et_horiz,0
	jz	@@ordered
	mov	ax,et_x0		; waagerechte Flaeche: x <-> y
	mov	bx,et_y0
	mov	et_y0,ax
	mov	et_x0,bx
	mov	ax,et_x1
	mov	bx,et_y1
	mov	et_y1,ax
	mov	et_x1,bx
@@ordered:
	cmp	word ptr et_x0,0
	jnl	@@not_left
	cmp	word ptr et_x1,0
	jnl	@@not_left
	pop	di
	pop	si
	leave
	retf
@@not_left:
	cmp	word ptr et_x0,VIEW_W
	jl	@@visible
	cmp	word ptr et_x1,VIEW_W
	jl	@@visible
	pop	di
	pop	si
	leave
	retf
@@visible:
	mov	ax,et_x0
	cmp	ax,et_x1
	jng	@@sorted
	mov	eax,et_x0
	xchg	eax,et_x1
	mov	et_x0,eax
	mov	ax,et_z0
	xchg	ax,et_z1
	mov	et_z0,ax
	mov	eax,et_u0
	xchg	eax,et_u1
	mov	et_u0,eax
@@sorted:
	mov	bx,et_x1
	sub	bx,et_x0
	jnz	@@slope
	; -- senkrechte Kante: beide Ecken eintragen (wie edge_trace)
	mov	si,et_x0
	shl	si,1
	cmp	si,VIEW_W * 2
	jnc	@@vert_b
	mov	ax,et_y0
	cmp	[si+g_edge_lo],ax
	jng	@@vert_a_hi
	mov	[si+g_edge_lo],ax
	mov	bx,et_z0
	mov	[si+g_edge_lo_z],bx
	mov	bx,et_u0
	mov	[si+g_edge_lo_u],bx
	mov	bx,et_v0
	mov	[si+g_edge_lo_v],bx
@@vert_a_hi:
	cmp	[si+g_edge_hi],ax
	jnl	@@vert_b
	mov	[si+g_edge_hi],ax
	mov	bx,et_z0
	mov	[si+g_edge_hi_z],bx
	mov	bx,et_u0
	mov	[si+g_edge_hi_u],bx
	mov	bx,et_v0
	mov	[si+g_edge_hi_v],bx
@@vert_b:
	cmp	si,VIEW_W * 2
	jnc	@@vert_done
	mov	ax,et_y1
	cmp	[si+g_edge_lo],ax
	jng	@@vert_b_hi
	mov	[si+g_edge_lo],ax
	mov	bx,et_z1
	mov	[si+g_edge_lo_z],bx
	mov	bx,et_u1
	mov	[si+g_edge_lo_u],bx
	mov	bx,et_v1
	mov	[si+g_edge_lo_v],bx
@@vert_b_hi:
	cmp	[si+g_edge_hi],ax
	jnl	@@vert_done
	mov	[si+g_edge_hi],ax
	mov	bx,et_z1
	mov	[si+g_edge_hi_z],bx
	mov	bx,et_u1
	mov	[si+g_edge_hi_u],bx
	mov	bx,et_v1
	mov	[si+g_edge_hi_v],bx
@@vert_done:
	pop	di
	pop	si
	leave
	retf
	; -- schraege Kante: Schritte fuer Koordinate, Nenner und Zaehler (Perspektive)
@@slope:
	inc	bx
	mov	ax,bx
	cwde
	mov	ebx,eax			; EBX = n (Spaltenzahl)
	mov	ax,et_y1
	sub	ax,et_y0
	shl	eax,10h
	cdq
	idiv	ebx
	mov	ep_dpos,eax
	mov	ax,et_z0		; Nennerschritt (w0 - w1) << 14 / n
	sub	ax,et_z1
	movsx	eax,ax
	shl	eax,0eh
	cdq
	idiv	ebx
	mov	ep_dw,eax
	mov	cx,et_z0		; ECX = w0 = z0 + Augabstand
	add	cx,ds:[g_proj_dist]
	movsx	ecx,cx
	mov	ax,et_u1		; Zaehlerschritte (wert1 - wert0) << 14 / n * w0
	sub	ax,et_u0
	movsx	eax,ax
	shl	eax,0eh
	cdq
	idiv	ebx
	imul	ecx
	mov	ep_du_w,eax
	mov	ax,et_v1
	sub	ax,et_v0
	movsx	eax,ax
	shl	eax,0eh
	cdq
	idiv	ebx
	imul	ecx
	mov	ep_dv_w,eax
	mov	ax,et_z1
	sub	ax,et_z0
	movsx	eax,ax
	shl	eax,0eh
	cdq
	idiv	ebx
	imul	ecx
	mov	ep_dz_w,eax
	mov	si,et_x0
	mov	ax,et_x1
	sub	ax,si
	inc	ax
	jz	@@done
	mov	cs:[et_count],ax
	shl	si,1
	movsx	eax,word ptr et_y0	; Koordinate (16.16)
	shl	eax,10h
	mov	ep_pos,eax
	movsx	eax,word ptr et_z1	; Nenner beginnt bei w1 << 14
	add	ax,ds:[g_proj_dist]
	shl	eax,0eh
	mov	ep_den,eax
	sub	eax,eax			; Zaehler = 0
	mov	ep_num_u,eax
	mov	ep_num_v,eax
	mov	ep_num_z,eax
@@column:
	cmp	si,VIEW_W * 2
	jnl	@@done			; rechts aus dem Fenster
	jnc	@@step			; links ausserhalb: nur weiterrechnen
	mov	eax,ep_num_u		; u = Zaehler / Nenner + u0  (Perspektive)
	cdq
	idiv	dword ptr ep_den
	add	ax,et_u0
	mov	cx,ax
	mov	eax,ep_num_v		; v
	cdq
	idiv	dword ptr ep_den
	add	ax,et_v0
	mov	di,ax
	mov	eax,ep_num_z		; z
	cdq
	idiv	dword ptr ep_den
	add	ax,et_z0
	mov	ebx,ep_pos		; Koordinate = ganzer Teil
	shr	ebx,10h
	cmp	[si+g_edge_lo],bx
	jng	@@check_hi
	mov	[si+g_edge_lo],bx
	mov	[si+g_edge_lo_z],ax
	mov	[si+g_edge_lo_u],cx
	mov	[si+g_edge_lo_v],di
@@check_hi:
	cmp	[si+g_edge_hi],bx
	jnl	@@step
	mov	[si+g_edge_hi],bx
	mov	[si+g_edge_hi_z],ax
	mov	[si+g_edge_hi_u],cx
	mov	[si+g_edge_hi_v],di
@@step:
	mov	eax,ep_du_w
	add	ep_num_u,eax
	mov	eax,ep_dv_w
	add	ep_num_v,eax
	mov	eax,ep_dz_w
	add	ep_num_z,eax
	mov	eax,ep_dpos
	add	ep_pos,eax
	mov	eax,ep_dw
	add	ep_den,eax
	inc	si
	inc	si
	dec	word ptr cs:[et_count]
	jnz	@@column
@@done:
	pop	di
	pop	si
	leave
	retf
edge_trace_persp	endp

; ============================================================================================================
; poly_edges - alle Kanten eines Polygons linear in die Kantentabellen eintragen
; poly_edges_persp - dasselbe mit perspektivischer Interpolation (edge_trace_persp)
; ------------------------------------------------------------------------------------------------------------
; C:      void poly_edges(short n, short far *idx, struct vtx far *vtx, short far *uv, short horizontal);
; Stapel: [bp+6] n Ecken, [bp+8] idx (Eckenindizes face3d.idx), [bp+0ch] vtx (g_vtx_buf),
;         [bp+10h] uv (face3d.uv, je Ecke u, v), [bp+14h] horizontal (zeilenweise Tabellen)
; Aufgabe: fuer jede Kante (Ecke i -> Ecke i + 1, die letzte zurueck zur ersten) edge_trace aufrufen mit
;          vtx[idx[i]], vtx[idx[i + 1]], uv[i], uv[i + 1], horizontal.
;          Vorher muss edges_clear die Tabellen geleert haben.
; Register: zerstoert EAX, EBX, ECX, EDX, ESI, EDI (obere Haelften); DS wird wiederhergestellt.
; ============================================================================================================
pe_n		equ	<[bp+6]>	; Zahl der Ecken (wird zu n * 2)
pe_idx		equ	<[bp+8]>
pe_vtx		equ	<[bp+0ch]>
pe_uv		equ	<[bp+10h]>
pe_horiz	equ	<[bp+14h]>
pe_ds		equ	<[bp-2]>	; gesichertes DS

poly_edges	proc	far
F_R24_04F3	label	far
	enter	2,0
	push	si
	push	di
	mov	pe_ds,ds
	shl	word ptr pe_n,1
	sub	ebx,ebx			; BX = Ecke i * 2
@@edge:
	lds	si,pe_idx
	movzx	ecx,word ptr [bx+si]	; ECX = idx[i]
	mov	edi,ebx
	add	bx,2			; naechste Ecke, nach der letzten wieder die erste
	cmp	bx,pe_n
	jnz	@@next_ok
	sub	bx,bx
@@next_ok:
	push	bx
	push	word ptr pe_horiz
	movzx	edx,word ptr [bx+si]	; EDX = idx[i + 1]
	mov	esi,ebx
	lds	bx,pe_uv
	push	dword ptr [ebx+esi*2]	; (u, v) der Ecke i + 1
	push	dword ptr [ebx+edi*2]	; (u, v) der Ecke i
	xchg	esi,edx
	xchg	edi,ecx
	lds	bx,pe_vtx
	push	word ptr [ebx+esi*8+4]	; z der Ecke i + 1
	push	dword ptr [ebx+esi*8]	; (x, y) der Ecke i + 1
	push	word ptr [ebx+edi*8+4]	; z der Ecke i
	push	dword ptr [ebx+edi*8]	; (x, y) der Ecke i
	mov	ds,pe_ds
	push	cs
	call	near ptr edge_trace
	add	sp,16h
	pop	bx
	and	bx,bx			; zurueck bei der ersten Ecke: fertig
	jnz	@@edge
	pop	di
	pop	si
	leave
	retf
poly_edges	endp

poly_edges_persp	proc	far
F_R24_0560	label	far
	enter	2,0
	push	si
	push	di
	mov	pe_ds,ds
	shl	word ptr pe_n,1
	sub	ebx,ebx
@@edge:
	lds	si,pe_idx
	movzx	ecx,word ptr [bx+si]
	mov	edi,ebx
	add	bx,2
	cmp	bx,pe_n
	jnz	@@next_ok
	sub	bx,bx
@@next_ok:
	push	bx
	push	word ptr pe_horiz
	movzx	edx,word ptr [bx+si]
	mov	esi,ebx
	lds	bx,pe_uv
	push	dword ptr [ebx+esi*2]
	push	dword ptr [ebx+edi*2]
	xchg	esi,edx
	xchg	edi,ecx
	lds	bx,pe_vtx
	push	word ptr [ebx+esi*8+4]
	push	dword ptr [ebx+esi*8]
	push	word ptr [ebx+edi*8+4]
	push	dword ptr [ebx+edi*8]
	mov	ds,pe_ds
	push	cs
	call	near ptr edge_trace_persp
	add	sp,16h
	pop	bx
	and	bx,bx
	jnz	@@edge
	pop	di
	pop	si
	leave
	retf
poly_edges_persp	endp

; ============================================================================================================
; edges_clear - Kantentabellen leeren (BE: fill_smth)
; ------------------------------------------------------------------------------------------------------------
; C:       void edges_clear(void);
; Aufgabe: g_edge_lo[] = 7FFFh, g_edge_hi[] = 8001h (je VIEW_W Worte), damit jede eingetragene Kante
;          kleiner bzw. groesser ist. Vor jeder Flaeche (SEG012 draw_object).
; Register: zerstoert EAX, CX, ES.
; ============================================================================================================
edges_clear	proc	far
F_R24_05CD	label	far
	push	si
	push	di
	mov	di,g_edge_lo
	mov	ax,DGROUP
	mov	es,ax
	cld
	mov	cx,VIEW_W / 2
	mov	eax,EDGE_EMPTY_LO * 10001h
	rep	stosd
	mov	di,g_edge_hi
	mov	eax,EDGE_EMPTY_HI * 10001h
	mov	cx,VIEW_W / 2
	rep	stosd
	pop	di
	pop	si
	retf
edges_clear	endp

; ============================================================================================================
; fill_scene - 3D-Fenster im Zeichenpuffer mit einer Farbe fuellen (BE-Name)
; ------------------------------------------------------------------------------------------------------------
; C:       void fill_scene(short color);   [bp+6] Farbe (unteres Byte)
; Aufgabe: VIEW_H Zeilen zu VIEW_W Bytes ab g_renderbuf_ptr + VIEW_OFS setzen (Himmel/Hintergrund, O198).
; Register: zerstoert EAX, BX, CX, DX.
; ============================================================================================================
fill_scene	proc	far
F_R24_05F6	label	far
	enter	0,0
	push	es
	push	di
	les	di,ds:[g_renderbuf_ptr]
	add	di,VIEW_OFS
	mov	al,[bp+6]		; Farbe in alle vier Bytes von EAX
	mov	ah,al
	mov	bx,ax
	shl	eax,10h
	mov	ax,bx
	cld
	mov	dx,VIEW_H
@@row:
	push	di
	mov	cx,VIEW_W / 4
	rep	stosd
	pop	di
	add	di,SCREEN_W
	dec	dx
	jnz	@@row
	pop	di
	pop	es
	leave
	retf
fill_scene	endp

; ============================================================================================================
; copy_3d_to_screen - 3D-Fenster aus dem Zeichenpuffer auf den VGA-Bildschirm kopieren (BE-Name)
; ------------------------------------------------------------------------------------------------------------
; C:       void copy_3d_to_screen(void);
; Aufgabe: VIEW_H Zeilen zu VIEW_W Bytes von g_renderbuf_ptr + VIEW_OFS nach A000:VIEW_SCR_OFS.
; Register: zerstoert AX, CX, DX.
; ============================================================================================================
copy_3d_to_screen	proc	far
F_R24_0627	label	far
	push	ds
	push	si
	push	es
	push	di
	lds	si,ds:[g_renderbuf_ptr]
	add	si,VIEW_OFS
	mov	ax,0a000h
	mov	es,ax
	mov	di,VIEW_SCR_OFS
	cld
	mov	dx,VIEW_H
@@row:
	push	di
	push	si
	mov	cx,VIEW_W / 4
	rep	movsd
	pop	si
	pop	di
	add	si,SCREEN_W
	add	di,SCREEN_W
	dec	dx
	jnz	@@row
	pop	di
	pop	es
	pop	si
	pop	ds
	retf
copy_3d_to_screen	endp

; ============================================================================================================
; Variablen der Textur-Fuellroutinen (im Codesegment)
; ============================================================================================================
tx_unused	=	$ - r24_base
		dd	0		; (nicht benutzt)
tx_vwrap	=	$ - r24_base
		dd	0		; Uebertrag des v-Bruchteils: +- Texturbreite (eine Texturzeile)
tx_vstep	=	$ - r24_base
		dd	0		; ganzzahliger v-Schritt je Pixel: (dv / n) * Texturbreite
tx_ustep	=	$ - r24_base
		dd	0		; ganzzahliger u-Schritt je Pixel

; ============================================================================================================
; Pixelschritte der Textur-Fuellroutinen (je Routine VIEW_H- bzw. VIEW_W-mal abgerollt)
; ------------------------------------------------------------------------------------------------------------
; Eingang aller Schritte:
;   ESI   Texeladresse (EMS: DS:SI, sonst linear DS = 0:ESI)   EDX   ganzzahliger u-Schritt
;   AL/CL Bruchteil-Akku/-Schritt u                            AH/CH Bruchteil-Akku/-Schritt v
;   EBP   ganzzahliger v-Schritt (tx_vstep)                    BH = 0, BL = Texel
;   FS    Farbumsetztabelle (Schattierung)                     ES:DI Zeichenpuffer (vorversetzt)
; Schritt k schreibt nach ES:[DI + k * SCREEN_W] (Spalte) bzw. ES:[DI + k] (Zeile). Das Symbol pfx&k haelt die
; Lage des Schritts fuer die Sprungtabelle; sein erstes Byte ist die Patchstelle (s. Kopf).
; ============================================================================================================
; Spalte, deckend
STEP_COL	macro	pfx,k
	local	nowrap
pfx&k		=	$ - r24_base
	mov	bl,[esi]		; Texel holen (EMS: gepatcht zu mov bl,ds:[si])
	add	al,cl			; u-Bruchteil weiter ...
	mov	bl,fs:[bx]		; Farbe schattieren
	adc	esi,edx			; ... u ganzzahlig mit Uebertrag
	mov	es:[di+(k)*SCREEN_W],bl	; Pixel setzen
	add	ah,ch			; v-Bruchteil weiter
	jnc	nowrap
	add	esi,cs:[tx_vwrap]	; Uebertrag: eine Texturzeile mehr
nowrap:
	add	esi,ebp			; ganzzahliger v-Schritt
	endm

; Spalte, Farbe 0 durchsichtig
STEP_COL_M	macro	pfx,k
	local	clear, nowrap
pfx&k		=	$ - r24_base
	mov	bl,[esi]
	add	al,cl
	adc	esi,edx
	or	bl,bl			; Farbe 0: Pixel auslassen
	jz	clear
	mov	bl,fs:[bx]
	mov	es:[di+(k)*SCREEN_W],bl
clear:
	add	ah,ch
	jnc	nowrap
	add	esi,cs:[tx_vwrap]
nowrap:
	add	esi,ebp
	endm

; Zeile, deckend (u-Schritt hier nur 16 Bit: adc si,dx, s. ORIGINALFEHLER #350)
STEP_ROW	macro	pfx,k
	local	nowrap
pfx&k		=	$ - r24_base
	mov	bl,[esi]
	add	al,cl
	mov	bl,fs:[bx]
	adc	si,dx
	mov	es:[di+(k)],bl
	add	ah,ch
	jnc	nowrap
	add	esi,cs:[tx_vwrap]
nowrap:
	add	esi,ebp
	endm

; Zeile, Farbe 0 durchsichtig
STEP_ROW_M	macro	pfx,k
	local	clear, nowrap
pfx&k		=	$ - r24_base
	mov	bl,[esi]
	add	al,cl
	adc	esi,edx
	or	bl,bl
	jz	clear
	mov	bl,fs:[bx]
	mov	es:[di+(k)],bl
clear:
	add	ah,ch
	jnc	nowrap
	add	esi,cs:[tx_vwrap]
nowrap:
	add	esi,ebp
	endm

; ============================================================================================================
; TEX_COLS step,pfx,jtab - Rumpf von poly_tex_cols / poly_tex_cols_m (spaltenweise, senkrechte Flaechen)
; ------------------------------------------------------------------------------------------------------------
; C:      void poly_tex_cols(short shade);   [bp+6] Helligkeit 0 (hell) .. 7 (dunkel)
; Eingabe: Kantentabellen (Index = Spalte, lo/hi = oberes/unteres y), g_cur_tex (Textur, bei EMS schon
;          eingeblendet), g_cur_tex_w.
; Ablauf je Spalte:
;   1. Schattierung: Stufe = g_fog_table[z_oben / 16] + shade (hoechstens 31), FS = g_shade_segs[Stufe].
;   2. Hoehe n = hi - lo + 1; Spalte unterhalb des Fensters oder leer: weiter. Ist hi < 0, ist die (konvexe)
;      Flaeche zu Ende.
;   3. Texturschritte fuer n Pixel: v nach Bresenham (ganze Zeilen * Breite + Bruchteil), u mit 8-Bit-Bruchteil.
;   4. Clipping oben: lo < 0 -> Textur fuer -lo Pixel vorrechnen. Clipping unten: hi > VIEW_H - 1 -> n kuerzen.
;   5. DI so vorversetzen, dass Schritt VIEW_H - n die Zeile lo trifft, und per Sprungtabelle dorthin.
; Lokale: [bp-2] n (Pixel der Spalte), [bp-4] hi.
; ============================================================================================================
TEX_COLS	macro	step,pfx,jtab
	enter	6,0
	push	si
	push	di
	; -- einmalig: Texelzugriffe fuer das EMS-Fenster auf 16 Bit umstellen (selbstveraendernder Code)
	cmp	word ptr ds:[g_ems_avail],0
	jz	@@scan
	mov	bx,cs:[jtab]			; Schritt 0 noch im Urzustand (67h)?
	cmp	byte ptr cs:[bx],OP_A32
	jnz	@@scan
	sub	si,si
	mov	cx,VIEW_H
@@patch:
	mov	bx,cs:[si+jtab]
	mov	word ptr cs:[bx],PATCH_W	; mov bl,[esi] -> ds: mov bl,[si]
	mov	byte ptr cs:[bx+2],PATCH_B
	add	si,2
	loop	@@patch
	; -- erste belegte Spalte suchen (hi != 8001h und hi >= 0)
@@scan:
	push	ds
	pop	es
	mov	di,g_edge_hi
	cld
	mov	cx,VIEW_W
	mov	ax,EDGE_EMPTY_HI
@@scan_next:
	repe	scasw
	and	cx,cx
	jnz	@@found
@@return:
	pop	di
	pop	si
	leave
	retf
@@found:
	cmp	word ptr [di-2],0
	jl	@@scan_next
	mov	si,di
	sub	si,g_edge_hi + 2		; SI = Spalte * 2
	mov	ax,ds:[g_renderbuf_ptr+2]
	mov	es,ax
@@column:
	; -- 1. Schattierung aus der Tiefe am oberen Ende
	mov	di,[si+g_edge_lo_z]
	sar	di,4
	cmp	di,SHADE_MAX
	jna	@@fog_ok
	mov	di,SHADE_MAX
@@fog_ok:
	mov	al,[di+g_fog_table]
	cbw
	add	ax,[bp+6]
	cmp	ax,SHADE_MAX
	jna	@@shade_ok
	mov	ax,SHADE_MAX
@@shade_ok:
	mov	di,ax
	shl	di,1
	mov	fs,[di+g_shade_segs]
	; -- 2. Spaltenhoehe
	mov	ax,[si+g_edge_hi]
	cmp	ax,0
	jl	@@return			; Flaeche zu Ende
	mov	[bp-4],ax
	mov	bx,[si+g_edge_lo]
	cmp	bx,VIEW_H
	jg	@@next_column			; unterhalb des Fensters
	sub	ax,bx
	cmp	ax,0
	jng	@@next_column
	inc	ax
	mov	[bp-2],ax			; n = hi - lo + 1
	; -- 3. Texturschritte: v (Bresenham ueber die Texturbreite)
	movzx	ebx,word ptr ds:[g_cur_tex_w]
	mov	cs:[tx_vwrap],ebx
	sub	di,di
	mov	ax,[si+g_edge_hi_v]
	sub	ax,[si+g_edge_lo_v]
	jns	@@dv_pos
	neg	ax
	inc	di				; DI = 1: v laeuft rueckwaerts
@@dv_pos:
	cwd
	idiv	word ptr [bp-2]			; ganze Zeilen je Pixel, Rest in DX
	mov	cx,dx
	mul	bx				; * Texturbreite
	and	di,di
	jz	@@dv_sign
	neg	ax
	neg	dword ptr cs:[tx_vwrap]
@@dv_sign:
	movsx	eax,ax
	mov	cs:[tx_vstep],eax
	movsx	ebx,word ptr [bp-2]
	mov	ax,cx				; Bruchteil = Rest / n (8 Bit) -> CH
	shl	eax,10h
	cdq
	idiv	ebx
	mov	ch,ah
	;    u (16.16, Bruchteil 8 Bit -> CL, ganzer Teil -> tx_ustep)
	mov	ax,[si+g_edge_hi_u]
	sub	ax,[si+g_edge_lo_u]
	shl	eax,10h
	cdq
	idiv	ebx
	mov	cl,ah
	shr	eax,10h
	movsx	edx,ax
	mov	cs:[tx_ustep],edx
	push	si
	push	bp
	push	ds
	; -- DI = Puffer + (lo + n - VIEW_H) * SCREEN_W + Spalte: Schritt k zeichnet Zeile lo + k - (VIEW_H - n)
	mov	di,[si+g_edge_lo]
	mov	bx,di
	add	di,[bp-2]
	sub	di,VIEW_H
	mov	ax,di
	shl	di,8
	shl	ax,6
	add	di,ax
	mov	ax,si
	shr	ax,1
	add	di,ax
	add	di,ds:[g_renderbuf_ptr]
	add	di,VIEW_OFS
	; -- Texeladresse am oberen Ende: v * Breite + u
	mov	ax,[si+g_edge_lo_v]
	mul	word ptr ds:[g_cur_tex_w]
	add	ax,[si+g_edge_lo_u]
	movzx	esi,ax
	mov	ax,FRAC_HALF * 101h		; beide Bruchteil-Akkus 0,5
	mov	edx,cs:[tx_ustep]
	; -- 4. Clipping oben: fuer die Pixel oberhalb des Fensters nur die Textur weiterschalten
	and	bx,bx
	jns	@@top_ok
@@pre_step:
	add	al,cl
	adc	si,dx
	add	ah,ch
	jnc	@@pre_nowrap
	add	si,cs:[tx_vwrap]
@@pre_nowrap:
	add	si,cs:[tx_vstep]
	dec	word ptr [bp-2]
	inc	bx
	jnz	@@pre_step
@@top_ok:
	lds	eax,ds:[g_cur_tex]		; DS = Textursegment (EMS-Fenster bzw. 0), ESI += Textur-Offset
	add	esi,eax
	;    Clipping unten
	mov	bx,[bp-4]
	cmp	bx,VIEW_H - 1
	jng	@@bottom_ok
	sub	bx,VIEW_H - 1
	sub	[bp-2],bx
	imul	bx,bx,SCREEN_W
	sub	di,bx
@@bottom_ok:
	mov	bx,[bp-2]
	cmp	bx,0
	jng	@@column_done
	cmp	bx,VIEW_H
	jg	@@column_done
	; -- 5. Einsprung: Schritt VIEW_H - n aus der Sprungtabelle
	push	ax
	neg	bx
	add	bx,VIEW_H
	shl	bx,1
	mov	ax,cs:[bx+jtab]
	pop	bx
	mov	ebp,cs:[tx_vstep]
	push	ax
	mov	ax,bx
	mov	bh,0
	retn					; "Sprung" in die abgerollte Schleife
	PAD4
n	=	0
	rept	VIEW_H
	step	pfx,%n
n	=	n + 1
	endm
@@column_done:
	pop	ds
	pop	bp
	pop	si
@@next_column:
	add	si,2
	cmp	si,VIEW_W * 2
	jc	@@column
	pop	di
	pop	si
	leave
	retf
	; -- Sprungtabelle: Adresse jedes Schritts
	AT	jtab
n	=	0
	rept	VIEW_H
	JT_ENT	pfx,%n
n	=	n + 1
	endm
	endm

; ============================================================================================================
; TEX_ROWS step,pfx,jtab - Rumpf von poly_tex_rows / poly_tex_rows_m (zeilenweise, Boden und Decke)
; ------------------------------------------------------------------------------------------------------------
; C:      void poly_tex_rows(short shade);   [bp+6] Helligkeit 0..7
; Eingabe: Kantentabellen mit vertauschten Achsen (Index = Zeile, lo/hi = linkes/rechtes x).
; Ablauf wie TEX_COLS, nur je Zeile (VIEW_H Zeilen, VIEW_W Pixel). Unterschiede:
;   - Die Zeile beginnt ein Pixel frueher (lo - 1) und ist hi - lo + 2 Pixel breit (schliesst Luecken zwischen
;     benachbarten Bodenflaechen). Die Pruefung "Breite <= 0" fehlt hier (cmp ohne Sprung), die Zeile wird
;     dann spaeter bei n <= 0 verworfen.
;   - Die Stufe wird nach dem Klemmen zusaetzlich mit 31 maskiert.
;   - Beim Vorrechnen am linken Rand laeuft u in 32 Bit (adc esi,edx), v in 16 Bit.
; ============================================================================================================
TEX_ROWS	macro	step,pfx,jtab
	enter	6,0
	push	si
	push	di
	; -- einmalig: Texelzugriffe fuer das EMS-Fenster auf 16 Bit umstellen
	cmp	word ptr ds:[g_ems_avail],0
	jz	@@scan
	mov	bx,cs:[jtab]
	cmp	byte ptr cs:[bx],OP_A32
	jnz	@@scan
	sub	si,si
	mov	cx,VIEW_W
@@patch:
	mov	bx,cs:[si+jtab]
	mov	word ptr cs:[bx],PATCH_W
	mov	byte ptr cs:[bx+2],PATCH_B
	add	si,2
	loop	@@patch
	; -- erste belegte Zeile suchen
@@scan:
	push	ds
	pop	es
	mov	di,g_edge_hi
	cld
	mov	cx,VIEW_H
	mov	ax,EDGE_EMPTY_HI
@@scan_next:
	repe	scasw
	and	cx,cx
	jnz	@@found
@@return:
	pop	di
	pop	si
	leave
	retf
@@found:
	cmp	word ptr [di-2],0
	jl	@@scan_next
	mov	si,di
	sub	si,g_edge_hi + 2		; SI = Zeile * 2
	mov	ax,ds:[g_renderbuf_ptr+2]
	mov	es,ax
@@row:
	; -- Schattierung aus der Tiefe am linken Ende
	mov	di,[si+g_edge_lo_z]
	sar	di,4
	cmp	di,SHADE_MAX
	jna	@@fog_ok
	mov	di,SHADE_MAX
@@fog_ok:
	mov	al,[di+g_fog_table]
	cbw
	add	ax,[bp+6]
	cmp	ax,SHADE_MAX
	jna	@@shade_ok
	mov	ax,SHADE_MAX
@@shade_ok:
	mov	di,ax
	and	di,SHADE_MAX
	shl	di,1
	mov	fs,[di+g_shade_segs]
	; -- Zeilenbreite
	mov	ax,[si+g_edge_hi]
	cmp	ax,0
	jl	@@return
	mov	[bp-4],ax
	mov	bx,[si+g_edge_lo]
	cmp	bx,VIEW_W
	jg	@@next_row			; rechts ausserhalb
	sub	ax,bx
	cmp	ax,0				; (ohne Sprung)
	add	ax,2
	mov	[bp-2],ax			; n = hi - lo + 2
	; -- Texturschritte (wie TEX_COLS)
	movzx	ebx,word ptr ds:[g_cur_tex_w]
	mov	cs:[tx_vwrap],ebx
	sub	di,di
	mov	ax,[si+g_edge_hi_v]
	sub	ax,[si+g_edge_lo_v]
	jns	@@dv_pos
	neg	ax
	inc	di
@@dv_pos:
	cwd
	idiv	word ptr [bp-2]
	mov	cx,dx
	mul	bx
	and	di,di
	jz	@@dv_sign
	neg	ax
	neg	dword ptr cs:[tx_vwrap]
@@dv_sign:
	movsx	eax,ax
	mov	cs:[tx_vstep],eax
	movsx	ebx,word ptr [bp-2]
	mov	ax,cx
	shl	eax,10h
	cdq
	idiv	ebx
	mov	ch,ah
	mov	ax,[si+g_edge_hi_u]
	sub	ax,[si+g_edge_lo_u]
	shl	eax,10h
	cdq
	idiv	ebx
	mov	cl,ah
	shr	eax,10h
	movsx	edx,ax
	mov	cs:[tx_ustep],edx
	push	si
	push	bp
	push	ds
	; -- DI = Puffer + Zeile * SCREEN_W + (lo - 1) + n - VIEW_W
	mov	di,[si+g_edge_lo]
	dec	di
	mov	bx,di
	add	di,[bp-2]
	sub	di,VIEW_W
	mov	ax,si				; Zeile * 2 * (32 + 128) = Zeile * 320
	shl	si,5
	shl	ax,7
	add	ax,si
	shr	si,5
	add	di,ax
	add	di,ds:[g_renderbuf_ptr]
	add	di,VIEW_OFS
	mov	ax,[si+g_edge_lo_v]
	mul	word ptr ds:[g_cur_tex_w]
	add	ax,[si+g_edge_lo_u]
	movzx	esi,ax
	mov	ax,FRAC_HALF * 101h
	mov	edx,cs:[tx_ustep]
	; -- Clipping links: Textur vorrechnen
	and	bx,bx
	jns	@@left_ok
@@pre_step:
	add	al,cl
	adc	esi,edx
	add	ah,ch
	jnc	@@pre_nowrap
	add	si,cs:[tx_vwrap]
@@pre_nowrap:
	add	si,cs:[tx_vstep]
	dec	word ptr [bp-2]
	inc	bx
	jnz	@@pre_step
@@left_ok:
	lds	eax,ds:[g_cur_tex]
	add	esi,eax
	;    Clipping rechts
	mov	bx,[bp-4]
	cmp	bx,VIEW_W - 1
	jng	@@right_ok
	sub	bx,VIEW_W - 1
	sub	[bp-2],bx
	sub	di,bx
@@right_ok:
	mov	bx,[bp-2]
	cmp	bx,0
	jng	@@row_done
	cmp	bx,VIEW_W
	jg	@@row_done
	; -- Einsprung: Schritt VIEW_W - n
	push	ax
	neg	bx
	add	bx,VIEW_W
	shl	bx,1
	mov	ax,cs:[bx+jtab]
	pop	bx
	mov	ebp,cs:[tx_vstep]
	push	ax
	mov	ax,bx
	mov	bh,0
	retn
	PAD4
n	=	0
	rept	VIEW_W
	step	pfx,%n
n	=	n + 1
	endm
@@row_done:
	pop	ds
	pop	bp
	pop	si
@@next_row:
	add	si,2
	cmp	si,VIEW_H * 2
	jc	@@row
	pop	di
	pop	si
	leave
	retf
	AT	jtab
n	=	0
	rept	VIEW_W
	JT_ENT	pfx,%n
n	=	n + 1
	endm
	endm

; Lage der Sprungtabellen (werden im Rumpf vor ihrer Definition benutzt; AT prueft sie)
JT_COLS		equ	187eh
JT_COLS_M	equ	2ec2h
JT_ROWS		equ	4989h
JT_ROWS_M	equ	6a95h

; ============================================================================================================
; poly_tex_cols - texturierte Flaeche spaltenweise zeichnen (Waende, Haeuser)
; ============================================================================================================
poly_tex_cols	proc	far
F_R24_0668	label	far
	TEX_COLS	STEP_COL,tc_,JT_COLS
poly_tex_cols	endp

; ============================================================================================================
; poly_tex_cols_m - wie poly_tex_cols, Texturfarbe 0 durchsichtig (FACE_MASKED: Baeume, Straeucher, Gitter)
; ============================================================================================================
poly_tex_cols_m	proc	far
F_R24_198A	label	far
	TEX_COLS	STEP_COL_M,tcm_,JT_COLS_M
poly_tex_cols_m	endp

; ============================================================================================================
; poly_tex_rows - texturierte Flaeche zeilenweise zeichnen (Boden, Decke)
; ============================================================================================================
poly_tex_rows	proc	far
F_R24_2FCE	label	far
	TEX_ROWS	STEP_ROW,tr_,JT_ROWS
poly_tex_rows	endp

; ============================================================================================================
; poly_tex_rows_m - wie poly_tex_rows, Texturfarbe 0 durchsichtig
; ============================================================================================================
poly_tex_rows_m	proc	far
F_R24_4B29	label	far
	TEX_ROWS	STEP_ROW_M,trm_,JT_ROWS_M
poly_tex_rows_m	endp

; ============================================================================================================
; poly_flat - einfarbige Flaeche spaltenweise zeichnen
; ------------------------------------------------------------------------------------------------------------
; C:       void poly_flat(short shade, short color);
; Stapel:  [bp+6] Helligkeit 0..7 (Index in g_shade_segs, ohne Nebel), [bp+8] Palettenfarbe (face3d +17h)
; Aufgabe: Farbe einmal ueber die Umsetztabelle der Stufe schattieren, dann jede belegte Spalte von lo bis hi
;          fuellen (geklemmt auf 0..VIEW_H-1), 16 Pixel je Durchlauf abgerollt.
; Register: zerstoert AX, BX, CX, ES.
; ============================================================================================================
pf_shade	equ	<[bp+6]>
pf_color	equ	<[bp+8]>

poly_flat	proc	far
F_R24_6C35	label	far
	enter	6,0
	push	si
	push	di
	mov	si,pf_shade
	shl	si,1
	mov	es,[si+g_shade_segs]
	mov	si,pf_color
	mov	si,es:[si]			; schattierte Farbe (unteres Byte)
	mov	pf_color,si
	sub	si,si
@@scan:
	cmp	word ptr [si+g_edge_hi],0	; erste Spalte mit hi >= 0
	jnl	@@column
	add	si,2
	cmp	si,VIEW_W * 2
	jc	@@scan
	pop	di
	pop	si
	leave
	retf
@@column:
	mov	cx,[si+g_edge_hi]
	cmp	cx,0
	jl	@@next_column
	mov	[bp-4],cx
	mov	bx,[si+g_edge_lo]
	cmp	bx,VIEW_H
	jnl	@@next_column
	sub	cx,bx
	inc	cx				; CX = n = hi - lo + 1
	cmp	cx,0
	jng	@@next_column
	cmp	bx,0				; Clipping oben
	jnl	@@top_ok
	add	cx,bx
	cmp	cx,0
	jl	@@next_column
	sub	bx,bx
@@top_ok:
	mov	di,bx				; DI = Puffer + lo * 320 + Spalte
	shl	di,6
	shl	bx,8
	add	di,bx
	add	di,ds:[g_renderbuf_ptr]
	add	di,VIEW_OFS
	mov	ax,si
	shr	ax,1
	add	di,ax
	mov	bx,[bp-4]			; Clipping unten
	cmp	bx,VIEW_H - 1
	jng	@@bottom_ok
	sub	bx,VIEW_H - 1
	sub	cx,bx
@@bottom_ok:
	cmp	cx,0
	jng	@@next_column
	cmp	cx,VIEW_H
	jg	@@next_column
	mov	ax,ds:[g_renderbuf_ptr+2]
	push	ds
	mov	ds,ax
	mov	al,pf_color
	mov	bx,cx
	shr	cx,4				; 16 Pixel je Durchlauf
	jz	@@rest
@@by16:
	mov	[di],al
	mov	[di+SCREEN_W],al
	mov	[di+2*SCREEN_W],al
	mov	[di+3*SCREEN_W],al
	mov	[di+4*SCREEN_W],al
	mov	[di+5*SCREEN_W],al
	mov	[di+6*SCREEN_W],al
	mov	[di+7*SCREEN_W],al
	mov	[di+8*SCREEN_W],al
	mov	[di+9*SCREEN_W],al
	mov	[di+10*SCREEN_W],al
	mov	[di+11*SCREEN_W],al
	mov	[di+12*SCREEN_W],al
	mov	[di+13*SCREEN_W],al
	mov	[di+14*SCREEN_W],al
	mov	[di+15*SCREEN_W],al
	add	di,16*SCREEN_W
	loop	@@by16
@@rest:
	mov	cx,bx				; restliche n mod 16 Pixel
	and	cx,0fh
	jz	@@column_done
@@by1:
	mov	[di],al
	add	di,SCREEN_W
	loop	@@by1
@@column_done:
	pop	ds
@@next_column:
	add	si,2
	cmp	si,VIEW_W * 2
	jc	@@column
	pop	di
	pop	si
	leave
	retf
poly_flat	endp

SEG024_TEXT	ends
	end

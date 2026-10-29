; ============================================================================================================
; SEG025 - 3D-Ansicht: Objekt in Kamerakoordinaten drehen, abschneiden, projizieren, Flaechen sortieren
;          (handgeschriebener 386-Assembler)
; ============================================================================================================
;
; Aufbau: Teil der 3D-Engine (Ueberblick im Kopf von SEG024). Dieses Modul bringt ein Objekt (struct obj3d)
;   vom Raum auf den Bildschirm. SEG012 draw_object ruft obj_project einmal je Objekt (Kartenzelle) auf und
;   zeichnet danach die sichtbaren Flaechen in der hier gebauten Reihenfolge mit SEG024.
;
; obj_project(obj), Ablauf:
;   1. Lage des Objekts relativ zum Auge (obj - g_cam_eye) mit den negierten Kamerawinkeln g_cam_neg drehen
;      -> Ursprung des Objekts in Kamerakoordinaten.
;   2. Jeden Eckpunkt mit (Kamerawinkel + Objektwinkel) drehen und den Ursprung addieren -> Punktpuffer
;      (Flaechenblock g_face_seg ab +204h, 8 Bytes je Punkt). Bewegte Eckpunkte (x = VTX_PATH) nehmen ihre
;      Lage aus g_vtx_paths je nach Phase des Objekts (Tueren, Animationen).
;   3. Flaechen mit einer Ecke hinter dem Auge (z < 0) an der Ebene z = 0 abschneiden (clip_face, neue Ecken
;      hinten an den Punktpuffer, die beschnittene Flaeche als Kopie im Codesegment).
;   4. Alle Punkte mit z >= 0 projizieren: x' = 104 + x * scale / 16 * dist / (z + dist), y' = 67 + ...
;   5. Je Flaeche: Normale drehen, Eintrag in g_face_list (struct face_ent), Rueckseite verwerfen (Flag 1),
;      ganz ausserhalb des Fensters verwerfen, nach groesstem z in die Liste g_face_next einsortieren
;      (absteigend: fern zuerst, Maleralgorithmus).
;
; Aufrufer aus C: obj_project (SEG012 draw_object), view_set_projection (O014 Dungeon, O046 Stadt: 90, 32),
;   tex_copy_flat (O010, O198: Texturen/Himmel ohne EMS aus dem Flat-Real-Mode-Speicher kopieren).
;   clip_face ist modulintern (far, nur von obj_project). Linknamen F_R25_xxxx bleiben, die sprechenden Namen
;   sind die Prozedurnamen.
;
; Festkomma-Formate:
;   Winkel 0..3FFh (10 Bit = 360 Grad), g_sin_table: 1024 Worte sin * 32768 (1.15), cos(a) = sin(a + 100h).
;   Drehung: (a * sin) >> 15 per "imul / shld dx,ax,1" (Ergebnis in DX). Koordinaten 16 Bit ganzzahlig.
;   Projektion: Ganzzahl, (x * scale) >> 4 als 32-Bit-Zwischenwert, dann * dist / (z + dist).
;
; Bildschirm: Mittelpunkt (104, 67) des 3D-Fensters 208 x 134 (SEG024). Trivial-Verwurf gegen 0..208, 0..134.
;
; Flaechenblock (Segment g_face_seg, O096 view_alloc_lists, ueber GS angesprochen):
;   +000h  next[129]   Verkettung der sichtbaren Flaechen, next[0] = erste (1-basiert)  (g_face_next)
;   +102h  depth[129]  Sortierschluessel = groesstes z der Flaeche                     (g_face_depth)
;   +204h  vtx[]       Punktpuffer (x, y, z, w), hoechstens 128 Punkte                  (g_vtx_buf)
;   FS haelt waehrend obj_project den ersten Punktindex des Objekts (Flaechenindizes sind objektrelativ).
;
; Rekonstruktion: aus SCHWEIF.EXE R25 (asm2tasm.py), lesbar gemacht 19.10.2026 (Auftrag C), bytegleich mit
;   TASM 3.1 /ml. Das Segment beginnt im Programm 0Ah Bytes hinter einer Paragraphengrenze (wortausgerichtet
;   hinter SEG024), CS zeigt auf diese Grenze: alle CS-Versaetze sind Lage im Modul + CS_BIAS.
; ============================================================================================================
	.386p
	include	..\names\inc\s_fix.inc	; Schalter der Fehlerbehebungen (FIX_nnnn, tools/mkfix.py)
	locals

; ---- Konstanten ----------------------------------------------------------------------------------------------
CS_BIAS		equ	0ah		; Lage des Modulanfangs relativ zu CS
VIEW_CX		equ	104		; Mitte des 3D-Fensters (208 x 134)
VIEW_CY		equ	67
VIEW_W		equ	208
VIEW_H		equ	134
VTX_PATH	equ	7fffh		; vtx.x: bewegter Eckpunkt, vtx.y = Index in g_vtx_paths
VTX_MAX		equ	129		; Punktpuffer: hoechstens VTX_MAX - 1 Punkte
FACE_MAX	equ	129		; Flaechenliste g_face_list und Zeigerfeld face_ptrs: hoechstens 129 Eintraege
CLIP_MAX	equ	16		; Flaechen, die je Objekt beschnitten werden koennen
FACE_SIZE	equ	48h		; sizeof(struct face3d)
FENT_SIZE	equ	12h		; sizeof(struct face_ent)
FACE_CULL	equ	0001h		; face3d.flags: einseitig, Rueckseite verwerfen

; struct face3d
F_NX		equ	00h		; Normale (x, y, z)
F_CNX		equ	08h		; Normale in Kamerakoordinaten (cx, cy, cz; von obj_project geschrieben)
F_CNY		equ	0ah
F_CNZ		equ	0ch
F_NVERTS	equ	10h
F_FLAGS		equ	12h
F_TEX		equ	14h
F_IDX		equ	18h		; Eckenindizes (objektrelativ)
F_UV		equ	28h		; (u, v) je Ecke
F_HDR_DWORDS	equ	6		; Kopf bis einschliesslich tex (18h Bytes)
; struct obj3d
O_X		equ	00h		; Lage (x, y, z)
O_ROT		equ	08h		; Drehwinkel (ax, ay, az)
O_VL		equ	20h		; Punktliste (n, vtx[n] ab +2)
O_NFACES	equ	24h
O_FACES		equ	26h		; Zeiger auf Zeigerfeld der Flaechen
O_PHASE		equ	2ah		; Phasen (bewegte Eckpunkte)
; Flaechenblock (GS)
FB_NEXT		equ	000h
FB_DEPTH	equ	102h
FB_VX		equ	204h		; Punkt k: FB_VX + k * 8 = x, +2 y, +4 z
FB_VY		equ	206h
FB_VZ		equ	208h

; ---- Daten im Datensegment (feste Adressen; Namen wie in names/inc/s_data.h) --------------------------------
g_cam_eye_x	equ	8c34h		; Augenpunkt (x, y, z)
g_cam_eye_y	equ	8c36h
g_cam_eye_z	equ	8c38h
g_cam_neg	equ	8c54h		; negierte Kamerawinkel [0] x/y-Ebene, [1] y/z, [2] x/z
g_proj_dist	equ	8c78h		; Kopien der Projektionswerte fuer C/SEG024
g_proj_scale	equ	8c7ah
g_nr_vis_faces	equ	8c7ch		; Zahl der Eintraege in g_face_list
g_nr_clip_faces	equ	8c7eh		; belegte Plaetze in clip_buf (R12 setzt 0 je Objekt)
g_nr_vtx	equ	8c82h		; belegte Punkte im Punktpuffer
g_sin_table	equ	8c84h		; Sinus 1.15, 1024 Worte
g_face_list	equ	94c2h		; far-Zeiger: struct face_ent[FACE_MAX - 1]
g_vtx_paths	equ	94c6h		; far-Zeiger je bewegtem Eckpunkt (struct vtx_path)
g_face_seg	equ	9850h		; Segment des Flaechenblocks

; ---- Hilfsmakros ---------------------------------------------------------------------------------------------
ZEROS	macro	nbytes			; nbytes Nullbytes (rept statt dup: kein LIDATA)
	rept	(nbytes) / 2
	dw	0
	endm
	endm


SEG025_TEXT	segment word public use16 'CODE'
	assume	cs:SEG025_TEXT, ds:nothing
	public	F_R25_04B6		; clip_face
	public	F_R25_06C4		; view_set_projection
	public	F_R25_08E4		; obj_project
	public	F_R25_0FF4		; tex_copy_flat

; ============================================================================================================
; Variablen im Codesegment (CS-Versatz = Lage im Modul + CS_BIAS)
; ============================================================================================================
r25_base	label	byte
pj_scale	=	$ - r25_base + CS_BIAS
		dw	0		; Massstab der Projektion (* 1/16), view_set_projection
pj_dist		=	$ - r25_base + CS_BIAS
		dw	0		; Augabstand der Projektion
clip_buf	=	$ - r25_base + CS_BIAS
		ZEROS	<CLIP_MAX * FACE_SIZE>	; beschnittene Flaechen (struct face3d)
; Kante der gerade beschnittenen Flaeche: Ecke a, Ecke b, Schnittpunkt
cl_a		=	$ - r25_base + CS_BIAS
		dw	0, 0, 0, 0	; Ecke a (x, y, z, w) aus dem Punktpuffer
cl_a_idx	=	$ - r25_base + CS_BIAS
		dw	0		; absoluter Punktindex von a
cl_b		=	$ - r25_base + CS_BIAS
		dw	0, 0, 0, 0	; Ecke b
cl_b_idx	=	$ - r25_base + CS_BIAS
		dw	0
cl_new		=	$ - r25_base + CS_BIAS
		dw	0, 0, 0, 0	; Schnittpunkt mit z = 0 (z, w bleiben 0)
cl_uv_a		=	$ - r25_base + CS_BIAS
		dw	0, 0		; (u, v) von a
cl_uv_b		=	$ - r25_base + CS_BIAS
		dw	0, 0		; (u, v) von b
cl_uv_new	=	$ - r25_base + CS_BIAS
		dw	0, 0		; (u, v) am Schnittpunkt

; ============================================================================================================
; clip_face - Flaeche an der Ebene z = 0 abschneiden (Teil des Polygons hinter dem Auge entfernen)
; ------------------------------------------------------------------------------------------------------------
; Aufruf:  far (nur aus obj_project), keine Stapelparameter.
; Eingang: ES:DI Flaeche (struct face3d), FS erster Punktindex des Objekts, GS Flaechenblock (Punktpuffer mit
;          gedrehten Punkten), g_nr_vtx, g_nr_clip_faces.
; Ausgang: ES:DI = beschnittene Kopie in clip_buf (CS). Ist clip_buf voll (CLIP_MAX), bleibt ES:DI
;          unveraendert (die Flaeche faellt dann spaeter wegen z < 0 heraus).
; Aufgabe: Kopf der Flaeche (Normalen, Flags, Textur) kopieren, dann die Ecken nach Sutherland-Hodgman:
;          je Kante a -> b:  a vorn und b vorn: a uebernehmen
;                            a vorn, b hinten:  a und den Schnittpunkt uebernehmen
;                            a hinten, b vorn:  nur den Schnittpunkt
;                            beide hinten:      nichts.
;          Schnittpunkt: t = |za| / |za - zb|, x/y/u/v = a + (b - a) * t, z = 0; er kommt als neuer Punkt
;          hinten an den Punktpuffer (g_nr_vtx + 1).
; Lokale:  [bp-2] Lage der Kopie in CS.
; Register: zerstoert EAX, BX, CX, EDX, SI; ES:DI s. o.
; ============================================================================================================
clip_face	proc	far
F_R25_04B6	label	far
	enter	2,0
	mov	si,ds:[g_nr_clip_faces]
	cmp	si,CLIP_MAX
	jnz	@@have_slot
	leave				; Puffer voll: Flaeche unbeschnitten lassen
	retf
@@have_slot:
	inc	word ptr ds:[g_nr_clip_faces]
	imul	si,si,FACE_SIZE
	db	081h,0c6h		; add si,clip_buf (Langform mit Wort wie "add si,offset clip_buf";
	dw	clip_buf		;  TASM wuerde fuer die Konstante die Kurzform 83h waehlen)
	mov	[bp-2],si
	; -- Kopf der Flaeche (bis einschliesslich tex) nach CS kopieren
	push	si
	push	di
	push	ds
	push	es
	xchg	si,di
	mov	ax,es
	mov	ds,ax
	mov	ax,cs
	mov	es,ax
	mov	cx,F_HDR_DWORDS
	cld
	rep	movsd
	pop	es
	pop	ds
	pop	di
	pop	si
	mov	word ptr cs:[si+F_NVERTS],0
	sub	bx,bx			; BX = Ecke a * 2
	mov	cx,es:[di+F_NVERTS]
@@edge:
	push	bx
	push	cx
	; -- Ecke a holen
	mov	si,fs
	mov	ax,es:[bx+di+F_IDX]
	add	si,ax
	mov	cs:[cl_a_idx],si
	shl	si,3
	mov	eax,gs:[si+FB_VX]
	mov	cs:[cl_a],eax		; (x, y)
	mov	eax,gs:[si+FB_VZ]
	mov	cs:[cl_a+4],eax		; (z, w)
	shl	bx,1
	mov	eax,es:[bx+di+F_UV]
	mov	cs:[cl_uv_a],eax
	shr	bx,1
	; -- Ecke b = naechste (nach der letzten die erste)
	cmp	cx,1
	jnz	@@b_next
	sub	bx,bx
	jmp	@@b_set
@@b_next:
	add	bx,2
@@b_set:
	mov	si,fs
	mov	ax,es:[bx+di+F_IDX]
	add	si,ax
	mov	cs:[cl_b_idx],si
	shl	si,3
	mov	eax,gs:[si+FB_VX]
	mov	cs:[cl_b],eax
	mov	eax,gs:[si+FB_VZ]
	mov	cs:[cl_b+4],eax
	shl	bx,1
	mov	eax,es:[bx+di+F_UV]
	mov	cs:[cl_uv_b],eax
	shr	bx,1
	; -- Faelle nach den Vorzeichen von za und zb
	cmp	word ptr cs:[cl_a+4],0
	jl	@@a_behind
	cmp	word ptr cs:[cl_b+4],0
	jnl	@@keep_a		; beide vorn
	jmp	@@intersect		; a vorn, b hinten
@@a_behind:
	cmp	word ptr cs:[cl_b+4],0
	jl	@@next_edge		; beide hinten
	; -- Schnittpunkt mit z = 0: wert = a + (b - a) * |za| / |za - zb|
@@intersect:
	push	bx
	mov	bx,cs:[cl_a+4]
	sub	bx,cs:[cl_b+4]
	jns	@@dz_abs
	neg	bx
@@dz_abs:
	mov	si,cs:[cl_a+4]
	and	si,si
	jns	@@za_abs
	neg	si
@@za_abs:
	mov	ax,cs:[cl_b]		; x
	sub	ax,cs:[cl_a]
	imul	si
	idiv	bx
	add	ax,cs:[cl_a]
	mov	cs:[cl_new],ax
	mov	ax,cs:[cl_b+2]		; y
	sub	ax,cs:[cl_a+2]
	imul	si
	idiv	bx
	add	ax,cs:[cl_a+2]
	mov	cs:[cl_new+2],ax
	mov	word ptr cs:[cl_new+4],0	; z = 0
	mov	ax,cs:[cl_uv_b]		; u
	sub	ax,cs:[cl_uv_a]
	imul	si
	idiv	bx
	add	ax,cs:[cl_uv_a]
	mov	cs:[cl_uv_new],ax
	mov	ax,cs:[cl_uv_b+2]	; v
	sub	ax,cs:[cl_uv_a+2]
	imul	si
	idiv	bx
	add	ax,cs:[cl_uv_a+2]
	mov	cs:[cl_uv_new+2],ax
	pop	bx
	cmp	word ptr cs:[cl_a+4],0
	jl	@@emit_new
	; a vorn: erst a uebernehmen
	mov	si,[bp-2]
	mov	bx,cs:[si+F_NVERTS]
	inc	word ptr cs:[si+F_NVERTS]
	shl	bx,1
	mov	ax,cs:[cl_a_idx]
	mov	dx,fs
	sub	ax,dx
	mov	cs:[bx+si+F_IDX],ax
	shl	bx,1
	mov	eax,cs:[cl_uv_a]
	mov	cs:[bx+si+F_UV],eax
@@emit_new:
	; Schnittpunkt als neuer Punkt g_nr_vtx in den Punktpuffer und in die Flaeche
	mov	si,[bp-2]
	mov	bx,cs:[si+F_NVERTS]
	inc	word ptr cs:[si+F_NVERTS]
	shl	bx,1
	mov	ax,ds:[g_nr_vtx]
	mov	dx,fs
	sub	ax,dx
	mov	cs:[bx+si+F_IDX],ax
	shl	bx,1
	mov	edx,cs:[cl_uv_new]
	mov	cs:[bx+si+F_UV],edx
	mov	bx,ds:[g_nr_vtx]
	shl	bx,3
	mov	eax,cs:[cl_new]
	mov	gs:[bx+FB_VX],eax
	mov	eax,cs:[cl_new+4]
	mov	gs:[bx+FB_VZ],eax
	inc	word ptr ds:[g_nr_vtx]
	jmp	@@next_edge
@@keep_a:
	mov	si,[bp-2]
	mov	bx,cs:[si+F_NVERTS]
	inc	word ptr cs:[si+F_NVERTS]
	shl	bx,1
	mov	ax,cs:[cl_a_idx]
	mov	dx,fs
	sub	ax,dx
	mov	cs:[bx+si+F_IDX],ax
	shl	bx,1
	mov	eax,cs:[cl_uv_a]
	mov	cs:[bx+si+F_UV],eax
@@next_edge:
	pop	cx
	pop	bx
	add	bx,2
	loop	@@edge_more
	jmp	short @@done
@@edge_more:
	jmp	near ptr @@edge
@@done:
	mov	ax,cs			; Ergebnis: ES:DI = Kopie
	mov	es,ax
	mov	di,[bp-2]
	leave
	retf
clip_face	endp

; ============================================================================================================
; view_set_projection - Massstab und Augabstand der Projektion setzen
; ------------------------------------------------------------------------------------------------------------
; C:       void view_set_projection(short scale, short dist);   (O014, O046: 90, 32)
; Aufgabe: pj_scale/pj_dist (CS, fuer obj_project) und die Kopien g_proj_scale/g_proj_dist (DS, SEG024
;          edge_trace_persp) setzen.
; ============================================================================================================
view_set_projection	proc	far
F_R25_06C4	label	far
	enter	0,0
	mov	ax,[bp+6]
	mov	cs:[pj_scale],ax
	mov	ds:[g_proj_scale],ax
	mov	ax,[bp+8]
	mov	cs:[pj_dist],ax
	mov	ds:[g_proj_dist],ax
	leave
	retf
view_set_projection	endp

; Zeiger auf die Flaechen des aktuellen Objekts (beschnittene zeigen nach clip_buf)
face_ptrs	=	$ - r25_base + CS_BIAS
		ZEROS	<FACE_MAX * 4>	; far-Zeiger je Flaeche
face_ptrs_len	=	$ - r25_base + CS_BIAS
		dw	0		; belegte Bytes in face_ptrs

; ============================================================================================================
; obj_project - Objekt drehen, abschneiden, projizieren und seine sichtbaren Flaechen einsortieren
; ------------------------------------------------------------------------------------------------------------
; C:       short obj_project(struct obj3d far *obj);   [bp+6]
; Eingang: g_cam_eye_*, g_cam_neg, g_nr_vtx/g_nr_vis_faces/g_nr_clip_faces (R12 setzt sie vor dem ersten
;          Objekt auf 0), g_face_seg, g_face_list, g_vtx_paths.
; Ausgang: AX = 1 ok, 0 = Puffer voll (ueber 128 Punkte, ueber 129 sichtbare Flaechen oder Zeigerfeld voll;
;          R12: "CalcBuffer full."). Punktpuffer, g_face_list, Verkettung next/depth, Zaehler fortgeschrieben.
;          Die Normale jeder Flaeche in Kamerakoordinaten steht danach in face3d +8/+0Ah/+0Ch.
; Lokale:  s. Liste unten. FS = erster Punktindex des Objekts, GS = Flaechenblock (beide gesichert).
; Register: zerstoert EAX, BX, CX, EDX, ES.
; ============================================================================================================
op_obj		equ	<[bp+6]>	; struct obj3d far *
op_face		equ	<[bp-4]>	; aktuelle Flaeche (Offset; Segment [bp-2])
op_face_seg	equ	<[bp-2]>
op_oz		equ	<[bp-8]>	; Ursprung des Objekts in Kamerakoordinaten (x, y, z)
op_oy		equ	<[bp-0ah]>
op_ox		equ	<[bp-0ch]>
op_dz		equ	<[bp-10h]>	; Objekt minus Auge
op_dy		equ	<[bp-12h]>
op_dx		equ	<[bp-14h]>
op_a2		equ	<[bp-18h]>	; Winkel Kamera + Objekt: [2] x/z-Ebene, [1] y/z, [0] x/y
op_a1		equ	<[bp-1ah]>
op_a0		equ	<[bp-1ch]>
op_sin0		equ	<[bp-2eh]>	; Sinus und Kosinus der drei Winkel (1.15)
op_sin1		equ	<[bp-30h]>
op_sin2		equ	<[bp-32h]>
op_cos0		equ	<[bp-34h]>
op_cos1		equ	<[bp-36h]>
op_cos2		equ	<[bp-38h]>
op_left		equ	<[bp-3eh]>	; Ecken links / rechts / ober- / unterhalb des Fensters
op_right	equ	<[bp-40h]>
op_above	equ	<[bp-42h]>
op_below	equ	<[bp-44h]>
op_nverts	equ	<[bp-46h]>
op_zmin		equ	<[bp-48h]>
op_zmax		equ	<[bp-4ah]>	; Sortierschluessel

obj_project	proc	far
F_R25_08E4	label	far
	enter	74,0
	push	si
	push	di
	push	fs
	push	gs
	; -- 1. Lage des Objekts relativ zum Auge
	les	di,op_obj
	mov	ax,es:[di+O_X]
	sub	ax,ds:[g_cam_eye_x]
	mov	op_dx,ax
	mov	ax,es:[di+O_X+2]
	sub	ax,ds:[g_cam_eye_y]
	mov	op_dy,ax
	mov	ax,es:[di+O_X+4]
	sub	ax,ds:[g_cam_eye_z]
	mov	op_dz,ax
	;    Sinus/Kosinus der Kamerawinkel (cos = sin(a + 100h), Index * 2 -> + 200h)
	mov	si,ds:[g_cam_neg]
	and	si,3ffh
	shl	si,1
	mov	ax,[si+g_sin_table]
	mov	op_sin0,ax
	add	si,200h
	and	si,7feh
	mov	ax,[si+g_sin_table]
	mov	op_cos0,ax
	mov	si,ds:[g_cam_neg+2]
	and	si,3ffh
	shl	si,1
	mov	ax,[si+g_sin_table]
	mov	op_sin1,ax
	add	si,200h
	and	si,7feh
	mov	ax,[si+g_sin_table]
	mov	op_cos1,ax
	mov	si,ds:[g_cam_neg+4]
	and	si,3ffh
	shl	si,1
	mov	ax,[si+g_sin_table]
	mov	op_sin2,ax
	add	si,200h
	and	si,7feh
	mov	ax,[si+g_sin_table]
	mov	op_cos2,ax
	;    Ursprung drehen: x/z-Ebene (Winkel 2), y/z (1), x/y (0); (a * b) >> 15 steht jeweils in DX
	push	cx
	mov	ax,op_dz		; z' = dz cos2 - dx sin2
	imul	word ptr op_cos2
	shld	dx,ax,1
	mov	cx,dx
	mov	ax,op_dx
	imul	word ptr op_sin2
	shld	dx,ax,1
	sub	cx,dx
	mov	ax,op_dz
	mov	op_oz,cx
	imul	word ptr op_sin2	; x' = dz sin2 + dx cos2
	shld	dx,ax,1
	mov	cx,dx
	mov	ax,op_dx
	imul	word ptr op_cos2
	shld	dx,ax,1
	add	cx,dx
	mov	op_ox,cx
	mov	ax,op_dy		; y'' = dy cos1 - z' sin1
	imul	word ptr op_cos1
	shld	dx,ax,1
	mov	cx,dx
	mov	ax,op_oz
	imul	word ptr op_sin1
	shld	dx,ax,1
	sub	cx,dx
	mov	ax,op_dy
	mov	op_oy,cx
	imul	word ptr op_sin1	; z'' = dy sin1 + z' cos1
	shld	dx,ax,1
	mov	cx,dx
	mov	ax,op_oz
	imul	word ptr op_cos1
	shld	dx,ax,1
	add	cx,dx
	mov	op_oz,cx
	mov	ax,op_ox		; x''' = x' cos0 - y'' sin0
	imul	word ptr op_cos0
	shld	dx,ax,1
	mov	cx,dx
	mov	ax,op_oy
	imul	word ptr op_sin0
	shld	dx,ax,1
	sub	cx,dx
	mov	ax,op_ox
	mov	op_ox,cx
	imul	word ptr op_sin0	; y''' = x' sin0 + y'' cos0
	shld	dx,ax,1
	mov	cx,dx
	mov	ax,op_oy
	imul	word ptr op_cos0
	shld	dx,ax,1
	add	cx,dx
	mov	op_oy,cx
	pop	cx
	; -- 2. Winkel fuer die Punkte: Kamera + Objekt
	mov	ax,ds:[g_cam_neg]
	add	ax,es:[di+O_ROT]
	mov	op_a0,ax
	mov	ax,ds:[g_cam_neg+2]
	add	ax,es:[di+O_ROT+2]
	mov	op_a1,ax
	mov	ax,ds:[g_cam_neg+4]
	add	ax,es:[di+O_ROT+4]
	mov	op_a2,ax
	mov	si,op_a0
	and	si,3ffh
	shl	si,1
	mov	ax,[si+g_sin_table]
	mov	op_sin0,ax
	add	si,200h
	and	si,7feh
	mov	ax,[si+g_sin_table]
	mov	op_cos0,ax
	mov	si,op_a1
	and	si,3ffh
	shl	si,1
	mov	ax,[si+g_sin_table]
	mov	op_sin1,ax
	add	si,200h
	and	si,7feh
	mov	ax,[si+g_sin_table]
	mov	op_cos1,ax
	mov	si,op_a2
	and	si,3ffh
	shl	si,1
	mov	ax,[si+g_sin_table]
	mov	op_sin2,ax
	add	si,200h
	and	si,7feh
	mov	ax,[si+g_sin_table]
	mov	op_cos2,ax
	;    Punkte des Objekts hinten an den Punktpuffer
	mov	si,ds:[g_nr_vtx]
	mov	gs,ds:[g_face_seg]
	mov	fs,si			; FS = erster Punktindex des Objekts
	shl	si,3
	les	di,es:[di+O_VL]
	mov	cx,es:[di]		; Zahl der Punkte
	add	ds:[g_nr_vtx],cx
	cmp	word ptr ds:[g_nr_vtx],VTX_MAX
	jnc	@@fail			; Punktpuffer voll
@@vertex:				; ES:DI + 2 = aktueller Punkt
	cmp	word ptr es:[di+2],VTX_PATH
	jnz	@@fixed_vertex
	;    bewegter Eckpunkt: Lage e[phase mod n] aus g_vtx_paths[y]
	push	es
	push	di
	mov	bx,es:[di+4]
	shl	bx,2
	les	di,[bx+g_vtx_paths]
	mov	dx,es:[di]		; n Lagen
	mov	bx,es:[di+2]		; welche Phase des Objekts
	shl	bx,1
	push	es
	push	di
	les	di,op_obj
	mov	ax,es:[bx+di+O_PHASE]
	pop	di
	pop	es
	mov	bx,dx
	cwd
	div	bx
	shl	dx,3			; DX = (phase mod n) * 8
	add	di,dx
	add	di,4			; ES:DI = Lage (x, y, z)
	push	cx
	mov	ax,es:[di+4]		; drehen wie unten, Quelle (x, y, z) ab +0
	imul	word ptr op_cos2
	shld	dx,ax,1
	mov	cx,dx
	mov	ax,es:[di]
	imul	word ptr op_sin2
	shld	dx,ax,1
	sub	cx,dx
	mov	ax,es:[di+4]
	mov	gs:[si+FB_VZ],cx
	imul	word ptr op_sin2
	shld	dx,ax,1
	mov	cx,dx
	mov	ax,es:[di]
	imul	word ptr op_cos2
	shld	dx,ax,1
	add	cx,dx
	mov	gs:[si+FB_VX],cx
	mov	ax,es:[di+2]
	imul	word ptr op_cos1
	shld	dx,ax,1
	mov	cx,dx
	mov	ax,gs:[si+FB_VZ]
	imul	word ptr op_sin1
	shld	dx,ax,1
	sub	cx,dx
	mov	ax,es:[di+2]
	mov	gs:[si+FB_VY],cx
	imul	word ptr op_sin1
	shld	dx,ax,1
	mov	cx,dx
	mov	ax,gs:[si+FB_VZ]
	imul	word ptr op_cos1
	shld	dx,ax,1
	add	cx,dx
	mov	gs:[si+FB_VZ],cx
	mov	ax,gs:[si+FB_VX]
	imul	word ptr op_cos0
	shld	dx,ax,1
	mov	cx,dx
	mov	ax,gs:[si+FB_VY]
	imul	word ptr op_sin0
	shld	dx,ax,1
	sub	cx,dx
	mov	ax,gs:[si+FB_VX]
	mov	gs:[si+FB_VX],cx
	imul	word ptr op_sin0
	shld	dx,ax,1
	mov	cx,dx
	mov	ax,gs:[si+FB_VY]
	imul	word ptr op_cos0
	shld	dx,ax,1
	add	cx,dx
	mov	gs:[si+FB_VY],cx
	pop	cx
	pop	di
	pop	es
	jmp	near ptr @@add_origin
@@fixed_vertex:
	;    fester Eckpunkt (x, y, z ab +2): x/z-Ebene, y/z, x/y drehen
	push	cx
	mov	ax,es:[di+6]		; z' = z cos2 - x sin2
	imul	word ptr op_cos2
	shld	dx,ax,1
	mov	cx,dx
	mov	ax,es:[di+2]
	imul	word ptr op_sin2
	shld	dx,ax,1
	sub	cx,dx
	mov	ax,es:[di+6]
	mov	gs:[si+FB_VZ],cx
	imul	word ptr op_sin2	; x' = z sin2 + x cos2
	shld	dx,ax,1
	mov	cx,dx
	mov	ax,es:[di+2]
	imul	word ptr op_cos2
	shld	dx,ax,1
	add	cx,dx
	mov	gs:[si+FB_VX],cx
	mov	ax,es:[di+4]		; y'' = y cos1 - z' sin1
	imul	word ptr op_cos1
	shld	dx,ax,1
	mov	cx,dx
	mov	ax,gs:[si+FB_VZ]
	imul	word ptr op_sin1
	shld	dx,ax,1
	sub	cx,dx
	mov	ax,es:[di+4]
	mov	gs:[si+FB_VY],cx
	imul	word ptr op_sin1	; z'' = y sin1 + z' cos1
	shld	dx,ax,1
	mov	cx,dx
	mov	ax,gs:[si+FB_VZ]
	imul	word ptr op_cos1
	shld	dx,ax,1
	add	cx,dx
	mov	gs:[si+FB_VZ],cx
	mov	ax,gs:[si+FB_VX]	; x''' = x' cos0 - y'' sin0
	imul	word ptr op_cos0
	shld	dx,ax,1
	mov	cx,dx
	mov	ax,gs:[si+FB_VY]
	imul	word ptr op_sin0
	shld	dx,ax,1
	sub	cx,dx
	mov	ax,gs:[si+FB_VX]
	mov	gs:[si+FB_VX],cx
	imul	word ptr op_sin0	; y''' = x' sin0 + y'' cos0
	shld	dx,ax,1
	mov	cx,dx
	mov	ax,gs:[si+FB_VY]
	imul	word ptr op_cos0
	shld	dx,ax,1
	add	cx,dx
	mov	gs:[si+FB_VY],cx
	pop	cx
@@add_origin:
	mov	ax,gs:[si+FB_VX]	; + Ursprung des Objekts
	add	ax,op_ox
	mov	gs:[si+FB_VX],ax
	mov	ax,gs:[si+FB_VY]
	add	ax,op_oy
	mov	gs:[si+FB_VY],ax
	mov	ax,gs:[si+FB_VZ]
	add	ax,op_oz
	mov	gs:[si+FB_VZ],ax
	add	di,8
	add	si,8
	loop	@@vertex_more
	jmp	short @@clip_faces
@@vertex_more:
	jmp	near ptr @@vertex
	; -- 3. Flaechen mit einer Ecke hinter dem Auge abschneiden, Zeiger aller Flaechen sammeln
@@clip_faces:
	mov	word ptr cs:[face_ptrs_len],0
	sub	bx,bx			; BX = Flaeche * 4
	les	di,op_obj
	mov	cx,es:[di+O_NFACES]
@@clip_face:
	push	bx
	push	cx
	les	di,op_obj
	les	di,es:[di+O_FACES]
	les	di,es:[bx+di]		; ES:DI = Flaeche
	mov	cx,es:[di+F_NVERTS]
	sub	bx,bx
@@clip_corner:
	mov	si,fs
	add	si,es:[bx+di+F_IDX]
	shl	si,3
	mov	ax,gs:[si+FB_VZ]
	cmp	ax,0
	jnl	short @@corner_front	; (jnl + nop: so im Original)
	nop
	db	09ah			; call far clip_face (TLINK macht daraus nop / push cs / call near)
	dd	clip_face
	jmp	@@store_face
@@corner_front:
	add	bx,2
	loop	@@clip_corner
@@store_face:
	mov	si,cs:[face_ptrs_len]
	cmp	si,FACE_MAX * 4
	jl	@@ptr_ok
	pop	cx
	pop	bx
	jmp	near ptr @@fail		; Zeigerfeld voll
@@ptr_ok:
	add	word ptr cs:[face_ptrs_len],4
	mov	cs:[si+face_ptrs],di
	mov	cs:[si+face_ptrs+2],es
	pop	cx
	pop	bx
	add	bx,4
	loop	@@clip_face
	; -- 4. Punkte (auch die neuen Schnittpunkte) projizieren
	mov	si,fs
	mov	cx,ds:[g_nr_vtx]
	sub	cx,si
	shl	si,3
@@project:
	cmp	word ptr gs:[si+FB_VZ],0
	jl	@@project_next		; hinter dem Auge
	mov	ax,gs:[si+FB_VX]	; x' = VIEW_CX + (x * scale >> 4) * dist / (z + dist)
	imul	word ptr cs:[pj_scale]
	shrd	ax,dx,4
	imul	word ptr cs:[pj_dist]
	mov	bx,gs:[si+FB_VZ]
	add	bx,cs:[pj_dist]
	idiv	bx
	add	ax,VIEW_CX
	mov	gs:[si+FB_VX],ax
	mov	ax,gs:[si+FB_VY]	; y' = VIEW_CY + ...
	imul	word ptr cs:[pj_scale]
	shrd	ax,dx,4
	imul	word ptr cs:[pj_dist]
	idiv	bx
	add	ax,VIEW_CY
	mov	gs:[si+FB_VY],ax
@@project_next:
	add	si,8
	loop	@@project
	; -- 5. je Flaeche: Normale drehen, Listeneintrag, Sichtbarkeit, einsortieren
	sub	bx,bx
	les	di,op_obj
	mov	cx,es:[di+O_NFACES]
@@face:
	push	bx
	push	cx
	les	di,cs:[bx+face_ptrs]
	mov	op_face,di
	mov	op_face_seg,es
	mov	cx,es:[di+F_FLAGS]
	push	cx
	;    Normale (nx, ny, nz) -> (cx, cy, cz) in face3d +8, wie die Punkte gedreht
	mov	ax,es:[di+F_NX+4]
	imul	word ptr op_cos2
	shld	dx,ax,1
	mov	cx,dx
	mov	ax,es:[di+F_NX]
	imul	word ptr op_sin2
	shld	dx,ax,1
	sub	cx,dx
	mov	ax,es:[di+F_NX+4]
	mov	es:[di+F_CNZ],cx
	imul	word ptr op_sin2
	shld	dx,ax,1
	mov	cx,dx
	mov	ax,es:[di+F_NX]
	imul	word ptr op_cos2
	shld	dx,ax,1
	add	cx,dx
	mov	es:[di+F_CNX],cx
	mov	ax,es:[di+F_NX+2]
	imul	word ptr op_cos1
	shld	dx,ax,1
	mov	cx,dx
	mov	ax,es:[di+F_CNZ]
	imul	word ptr op_sin1
	shld	dx,ax,1
	sub	cx,dx
	mov	ax,es:[di+F_NX+2]
	mov	es:[di+F_CNY],cx
	imul	word ptr op_sin1
	shld	dx,ax,1
	mov	cx,dx
	mov	ax,es:[di+F_CNZ]
	imul	word ptr op_cos1
	shld	dx,ax,1
	add	cx,dx
	mov	es:[di+F_CNZ],cx
	mov	ax,es:[di+F_CNX]
	imul	word ptr op_cos0
	shld	dx,ax,1
	mov	cx,dx
	mov	ax,es:[di+F_CNY]
	imul	word ptr op_sin0
	shld	dx,ax,1
	sub	cx,dx
	mov	ax,es:[di+F_CNX]
	mov	es:[di+F_CNX],cx
	imul	word ptr op_sin0
	shld	dx,ax,1
	mov	cx,dx
	mov	ax,es:[di+F_CNY]
	imul	word ptr op_cos0
	shld	dx,ax,1
	add	cx,dx
	mov	es:[di+F_CNY],cx
	pop	cx			; CX = Flags
	;    Eintrag in g_face_list (struct face_ent): Flaeche, FS, (cx, cy), (cz, +0Eh), tex
	mov	eax,es:[di+F_CNX]
	mov	edx,es:[di+F_CNZ]
	mov	si,ds:[g_nr_vis_faces]
IF FIX_0351
	cmp	si,FACE_MAX - 1	; Fix #351: Verkettung next/depth hat nur Platz fuer 1..128
ELSE
	cmp	si,FACE_MAX
ENDIF
	jnc	@@fail			; Liste voll
	imul	si,si,FENT_SIZE
	add	si,ds:[g_face_list]
	push	dword ptr es:[di+F_TEX]
	mov	bx,es
	mov	es,ds:[g_face_list+2]
	mov	es:[si],di		; +00h Flaeche
	mov	es:[si+2],bx
	mov	es:[si+4],fs		; +04h erster Punktindex
	mov	es:[si+6],eax		; +06h Normale x, y (Kamera)
	mov	es:[si+0ah],edx		; +0Ah Normale z = Winkel zur Kamera (Schattierung)
	pop	dword ptr es:[si+0eh]	; +0Eh Textur bzw. Ereignis
	mov	es,bx
	;    Rueckseite? (nur einseitige Flaechen): Kreuzprodukt der ersten drei Ecken im Bild
	sub	dx,dx
	test	cx,FACE_CULL
	jz	@@facing
	mov	si,fs
	mov	bx,si
	mov	dx,si
	add	si,es:[di+F_IDX]	; Ecke 0, 1, 2
	add	bx,es:[di+F_IDX+2]
	add	dx,es:[di+F_IDX+4]
	mov	di,dx
	shl	si,3
	shl	bx,3
	shl	di,3
	mov	ax,gs:[bx+FB_VY]	; (y1 - y0) * (x2 - x0)
	sub	ax,gs:[si+FB_VY]
	mov	cx,gs:[di+FB_VX]
	sub	cx,gs:[si+FB_VX]
	imul	cx
	push	ax
	push	dx
	mov	ax,gs:[bx+FB_VX]	; (x1 - x0) * (y2 - y0)
	sub	ax,gs:[si+FB_VX]
	mov	cx,gs:[di+FB_VY]
	sub	cx,gs:[si+FB_VY]
	imul	cx
	pop	cx
	pop	bx
	sub	ax,bx			; DX:AX = Differenz, < 0: Rueckseite
	sbb	dx,cx
@@facing:
	cmp	dx,0
	jl	@@face_next
	;    Ecken pruefen: z < 0 -> verwerfen; zmin/zmax; Zaehler je Fensterrand
	les	di,op_face
	mov	cx,es:[di+F_NVERTS]
	mov	op_nverts,cx
	sub	bx,bx
	mov	op_left,bx
	mov	op_right,bx
	mov	op_above,bx
	mov	op_below,bx
	mov	word ptr op_zmax,0
	mov	word ptr op_zmin,7fffh
@@corner:
	mov	si,fs
	add	si,es:[bx+di+F_IDX]
	shl	si,3
	mov	ax,gs:[si+FB_VZ]
	cmp	ax,0
	jl	@@face_next		; Ecke hinter dem Auge (nicht beschnitten)
	cmp	ax,op_zmax
	jl	@@zmax_ok
	mov	op_zmax,ax
@@zmax_ok:
	cmp	ax,op_zmin
	jg	@@zmin_ok
	mov	op_zmin,ax
@@zmin_ok:
	mov	ax,gs:[si+FB_VX]
	cmp	ax,0
	jnl	@@not_left
	inc	word ptr op_left
@@not_left:
	cmp	ax,VIEW_W
	jng	@@not_right
	inc	word ptr op_right
@@not_right:
	mov	ax,gs:[si+FB_VY]
	cmp	ax,0
	jnl	@@not_above
	inc	word ptr op_above
@@not_above:
	cmp	ax,VIEW_H
	jng	@@not_below
	inc	word ptr op_below
@@not_below:
	add	bx,2
	loop	@@corner
	mov	bx,op_nverts		; alle Ecken jenseits desselben Randes: unsichtbar
	cmp	op_left,bx
	jz	@@face_next
	cmp	op_right,bx
	jz	@@face_next
	cmp	op_above,bx
	jz	@@face_next
	cmp	op_below,bx
	jz	@@face_next
	;    einsortieren: Liste absteigend nach zmax (1-basiert, next[0] = erste)
	mov	ax,op_zmax
	mov	cx,ds:[g_nr_vis_faces]
	and	cx,cx
	jnz	@@insert
	mov	word ptr gs:[FB_NEXT],1	; erste Flaeche
	mov	gs:[FB_DEPTH+2],ax
	jmp	@@counted
@@insert:
	mov	si,gs:[FB_NEXT]
	shl	si,1
	sub	di,di			; DI = Vorgaenger * 2 (0 = Kopf)
@@search:
	cmp	gs:[si+FB_DEPTH],ax
	jl	@@insert_here		; erster mit kleinerem z: davor einfuegen
	mov	di,si
	mov	si,gs:[si+FB_NEXT]
	shl	si,1
	loop	@@search
@@insert_here:
	mov	bx,ds:[g_nr_vis_faces]
	inc	bx			; neue Flaeche k = Anzahl + 1
	mov	gs:[di+FB_NEXT],bx	; next[Vorgaenger] = k
	shl	bx,1
	shr	si,1
	mov	gs:[bx+FB_NEXT],si	; next[k] = Nachfolger
	mov	gs:[bx+FB_DEPTH],ax	; depth[k] = zmax
@@counted:
	inc	word ptr ds:[g_nr_vis_faces]
@@face_next:
	pop	cx
	pop	bx
	add	bx,4
	loop	@@face_more
	jmp	short @@ok
@@face_more:
	jmp	near ptr @@face
@@ok:
	mov	ax,1
	pop	gs
	pop	fs
	pop	di
	pop	si
	leave
	retf
@@fail:
	mov	ax,0
	pop	gs
	pop	fs
	pop	di
	pop	si
	leave
	retf
obj_project	endp

; ============================================================================================================
; tex_copy_flat - Speicher mit 32-Bit-Adressen kopieren (Flat Real Mode)
; ------------------------------------------------------------------------------------------------------------
; C:       void tex_copy_flat(short src_seg, long src_off, short dst_seg, long dst_off, long n);
; Stapel:  [bp+6] Quellsegment, [bp+8] Quelloffset (32 Bit), [bp+0ch] Zielsegment, [bp+0eh] Zieloffset (32 Bit),
;          [bp+12h] Anzahl Bytes (32 Bit)
; Aufgabe: n Bytes von src_seg:src_off nach dst_seg:dst_off kopieren, erst n mod 4 Bytes einzeln, dann
;          Doppelworte. Mit Segment 0 und linearem Offset liest es Texturen ohne EMS aus dem Speicher ueber 1 MB
;          (SEG035 hat die Segmentgrenzen auf 4 GB gesetzt). O010/O198: Texturen, Himmel.
; Register: zerstoert ECX, ES.
; ============================================================================================================
tex_copy_flat	proc	far
F_R25_0FF4	label	far
	enter	0,0
	push	ds
	push	esi
	push	edi
	mov	ecx,[bp+12h]
	jumps				; (Original: jecxz mit Platz fuer einen langen Sprung, fuenf NOPs)
	jecxz	@@done
	nojumps
	mov	ds,[bp+6]
	mov	es,[bp+0ch]
	mov	esi,[bp+8]
	mov	edi,[bp+0eh]
	cld
	and	ecx,3
	db	0f3h,067h,0a4h		; rep movs byte ptr es:[edi],[esi]  (Praefixfolge F3 67 wie im Original)
	mov	ecx,[bp+12h]
	shr	ecx,2
	db	0f3h,066h,067h,0a5h	; rep movs dword ptr es:[edi],[esi]
@@done:
	pop	edi
	pop	esi
	pop	ds
	leave
	retf
tex_copy_flat	endp

SEG025_TEXT	ends
	end

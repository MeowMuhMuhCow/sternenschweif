/* OVL221 - Overlay O221 (Segment 0x23d0 in SCHWEIF.EXE): 17 leere Funktionen (Platzhalter im Original, je
 * push bp / mov bp,sp / pop bp / retf). Kein Modul ruft sie auf (evidenz.tsv: 0 Aufrufer, keine Verweise in den
 * Fixups der uebrigen Module) - daher die Namen o221_stub_0 .. o221_stub_16 nach der Reihenfolge im Segment
 * (o221_stub_N = F_O221_<5 * N>).
 *
 * BCC -mlarge -3 -Ot -c -Y OVL221.C   -> 85/85 Bytes GLEICH, cobjfix laeuft (Referenz: o221/OVL221.ASM mit TASM /ml
 *   uebersetzt, wovn24/ref - ref_obj hat kein OVL221.OBJ)
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 */
#include "schweif.h"

void o221_stub_0(void)
{
}

void o221_stub_1(void)
{
}

void o221_stub_2(void)
{
}

void o221_stub_3(void)
{
}

void o221_stub_4(void)
{
}

void o221_stub_5(void)
{
}

void o221_stub_6(void)
{
}

void o221_stub_7(void)
{
}

void o221_stub_8(void)
{
}

void o221_stub_9(void)
{
}

void o221_stub_10(void)
{
}

void o221_stub_11(void)
{
}

void o221_stub_12(void)
{
}

void o221_stub_13(void)
{
}

void o221_stub_14(void)
{
}

void o221_stub_15(void)
{
}

void o221_stub_16(void)
{
}

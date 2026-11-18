/* SEG038 - XMS-Verschiebung (Auftrag struct mem_move): mit EMS-Treiber (V86-Modus) ueber den XMS-Treiber
 * (R39 xms_move), sonst selbst ueber Flat Real Mode (R35) mit der linearen Adresse des gesperrten XMS-Blocks
 * (seg038 in SCHWEIF.EXE)
 * BCC -mlarge -3 -O -Oc -Ol -Om -Ot -Ov -Z -c -Y SEG038.C   -> 250/250 Bytes GLEICH
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH)
 * - Auftrag wie in s_proto.h als void far *, Zugriff ueber den Cast-Makro MOVE. Handle 0 = konventioneller
 *   Speicher, die Adresse steht dann als seg:off im Auftrag (Segment fuer flat_copy getrennt).
 * - Genau eine Seite ist ein XMS-Block (from_xms: Quelle, sonst Ziel); der Block wird fuer die Kopie gesperrt.
 * - "if (!use_xms)" im Flat-Zweig ist immer wahr (so im Original).
 */
#include "schweif.h"

#define MOVE	((struct mem_move far *)move)

signed long xms_move_any(void far *move)
{
	register signed short use_xms;
	register unsigned short handle;
	signed short from_xms;
	void far *block;
	unsigned short src_seg;
	unsigned short dst_seg;
	unsigned long src_off;
	unsigned long dst_off;

	if ((use_xms = ems_present()) != 0) {
		return xms_move((unsigned long)move);
	} else {
		from_xms = MOVE->src_handle ? 1 : 0;
		handle = from_xms ? MOVE->src_handle : MOVE->dst_handle;
		block = xms_lock(handle);

		if (MOVE->src_handle == 0) {
			src_seg = ((unsigned short far *)&MOVE->src)[1];
			src_off = (unsigned short)MOVE->src;
			dst_seg = 0;
			dst_off = MOVE->dst + (unsigned long)block;
		} else {
			src_seg = 0;
			src_off = MOVE->src + (unsigned long)block;
			dst_seg = ((unsigned short far *)&MOVE->dst)[1];
			dst_off = (unsigned short)MOVE->dst;
		}

		if (!use_xms)
			flat_enter();
		flat_copy(src_seg, src_off, dst_seg, dst_off, MOVE->len);
		xms_unlock(handle);
		if (!use_xms)
			flat_leave();
	}
	return 0;
}

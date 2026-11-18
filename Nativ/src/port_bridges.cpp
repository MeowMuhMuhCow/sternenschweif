/* Auto-generiert von Port/tools/generate_bridges.py */
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

extern "C" void *mem_alloc(unsigned long size);
extern "C" void *mem_free(void *p);

void *operator new(size_t sz) { return mem_alloc((unsigned long)(sz ? sz : 1)); }
void *operator new[](size_t sz) { return mem_alloc((unsigned long)(sz ? sz : 1)); }
void operator delete(void *p) noexcept { if (p) mem_free(p); }
void operator delete[](void *p) noexcept { if (p) mem_free(p); }
void operator delete(void *p, size_t) noexcept { if (p) mem_free(p); }
void operator delete[](void *p, size_t) noexcept { if (p) mem_free(p); }

extern "C" void *_vector_new_(void *ptr, size_t elem_size, unsigned int count, unsigned int mode, void (*ctor)(void *)) {
    (void)mode;
    if (!ptr) ptr = mem_alloc((unsigned long)(count * elem_size));
    if (ctor && ptr) {
        for (unsigned int i = 0; i < count; ++i) {
            ctor((char *)ptr + i * elem_size);
        }
    }
    return ptr;
}

extern "C" void gb_delete(void *self, short flags);
extern "C" void nvf_delete(void *self, short flags);
extern "C" void _wrap_Bmp_dtor(void *self) { gb_delete(self, 2); }
extern "C" void _wrap_Nvf_dtor(void *self) { nvf_delete(self, 2); }
extern "C" void *dsa_mk_fp(unsigned short seg, unsigned long off);
extern "C" signed long Nvf_load8_call(void *self, short id, void *dst, void *src, short nr, signed char type, short *w, short *h, short stride) __asm__("__ZN3Nvf4loadEsPvS0_saPsS1_s");
extern "C" void _wrap_Nvf_load9(void *self, short id, void *dst, short src_lo, short src_hi, short nr, short type, short *w, short *h, short stride) {
    void *src = (src_lo == 0 && src_hi == 0) ? 0 : dsa_mk_fp((unsigned short)src_hi, (unsigned short)src_lo);
    Nvf_load8_call(self, id, dst, src, nr, (signed char)type, w, h, stride);
}

/* C++ -> C/C++ Jump-Stubs */
__asm__(".text\n\t.p2align 2\n\t.globl __Z12_vector_new_Pvjmjz\n__Z12_vector_new_Pvjmjz:\n\tjmp __vector_new_");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN3Bmp10free_linesEv\n__ZN3Bmp10free_linesEv:\n\tjmp _gb_free_lines");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN3Bmp3getEPvss\n__ZN3Bmp3getEPvss:\n\tjmp _gb_save");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN3Bmp3putEPvss\n__ZN3Bmp3putEPvss:\n\tjmp _gb_blit");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN3Bmp3putEPvsss\n__ZN3Bmp3putEPvsss:\n\tjmp _gb_blit_m");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN3Bmp4dataEv\n__ZN3Bmp4dataEv:\n\tjmp _gb_data");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN3Bmp4fillEs\n__ZN3Bmp4fillEs:\n\tjmp _gb_fill");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN3Bmp4initEssPv\n__ZN3Bmp4initEssPv:\n\tjmp _gb_init");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN3Bmp4loadEPv\n__ZN3Bmp4loadEPv:\n\tjmp _gb_load_mem");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN3Bmp4overEPS_sss\n__ZN3Bmp4overEPS_sss:\n\tjmp _gb_put");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN3Bmp4sizeEPsS0_\n__ZN3Bmp4sizeEPsS0_:\n\tjmp _gb_get_size");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN3Bmp5clearEv\n__ZN3Bmp5clearEv:\n\tjmp _gb_clear");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN3Bmp5scaleEPS_\n__ZN3Bmp5scaleEPS_:\n\tjmp _gb_scale");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN3Bmp8put_partEPvssss\n__ZN3Bmp8put_partEPvssss:\n\tjmp _gb_blit_wh");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN3Bmp8put_partEPvsssss\n__ZN3Bmp8put_partEPvsssss:\n\tjmp _gb_blit_whm");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN3Bmp8put_partEPvsssssss\n__ZN3Bmp8put_partEPvsssssss:\n\tjmp _gb_blit_ab");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN3Bmp8set_baseEPv\n__ZN3Bmp8set_baseEPv:\n\tjmp _gb_set_base");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN3BmpC1Ess\n__ZN3BmpC1Ess:\n\tjmp _gb_new_alloc");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN3BmpC1EssPv\n__ZN3BmpC1EssPv:\n\tjmp _gb_new");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN3BmpC1Ev\n__ZN3BmpC1Ev:\n\tjmp _gb_new0");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN3BmpD1Ev\n__ZN3BmpD1Ev:\n\tjmp __wrap_Bmp_dtor");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN3Inv10draw_slotsEPv\n__ZN3Inv10draw_slotsEPv:\n\tjmp __ZN6InvObj10draw_slotsEPv");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN3Inv10set_cursorEs\n__ZN3Inv10set_cursorEs:\n\tjmp __ZN6InvObj10set_cursorEs");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN3Inv12reset_cursorEv\n__ZN3Inv12reset_cursorEv:\n\tjmp __ZN6InvObj12reset_cursorEv");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN3Inv12update_marksEv\n__ZN3Inv12update_marksEv:\n\tjmp __ZN6InvObj12update_marksEv");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN3Inv8reselectEv\n__ZN3Inv8reselectEv:\n\tjmp __ZN6InvObj8reselectEv");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN3Inv9draw_slotEPvss\n__ZN3Inv9draw_slotEPvss:\n\tjmp __ZN6InvObj9draw_slotEPvss");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN3InvC1Ev\n__ZN3InvC1Ev:\n\tjmp __ZN6InvObjC1Ev");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN3InvD1Ev\n__ZN3InvD1Ev:\n\tjmp __ZN6InvObjD1Ev");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN3Nvf4loadEsPvssssPsS1_s\n__ZN3Nvf4loadEsPvssssPsS1_s:\n\tjmp __wrap_Nvf_load9");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4C0123runEv\n__ZN4C0123runEv:\n\tjmp __ZN5Store3runEv");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4C0125checkEv\n__ZN4C0125checkEv:\n\tjmp __ZN5Store7is_openEv");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4C012C1Ev\n__ZN4C012C1Ev:\n\tjmp __ZN5StoreC1Ev");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4C012D1Ev\n__ZN4C012D1Ev:\n\tjmp __ZN5StoreD1Ev");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4C0315f0093Ev\n__ZN4C0315f0093Ev:\n\tjmp __ZN2Ev11market_hallEv");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4C0315f00e0Ev\n__ZN4C0315f00e0Ev:\n\tjmp __ZN2Ev10magistrateEv");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4C0315f0149Ev\n__ZN4C0315f0149Ev:\n\tjmp __ZN2Ev5eolanEv");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4C0315f0212Ev\n__ZN4C0315f0212Ev:\n\tjmp __ZN2Ev11house_low49Ev");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4C0315f0230Ev\n__ZN4C0315f0230Ev:\n\tjmp __ZN2Ev7academyEv");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4C0315f024eEv\n__ZN4C0315f024eEv:\n\tjmp __ZN2Ev6smithyEv");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4C0315f0266Ev\n__ZN4C0315f0266Ev:\n\tjmp __ZN2Ev9showhouseEv");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4C0315f06a6Ev\n__ZN4C0315f06a6Ev:\n\tjmp __ZN2Ev10guardhouseEv");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4C0315f0870Ev\n__ZN4C0315f0870Ev:\n\tjmp __ZN2Ev10city_gate1Ev");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4C0315f08a7Ev\n__ZN4C0315f08a7Ev:\n\tjmp __ZN2Ev10city_gate2Ev");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4C0315f08deEv\n__ZN4C0315f08deEv:\n\tjmp __ZN2Ev13fortress_gateEv");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4C0315f0a2eEv\n__ZN4C0315f0a2eEv:\n\tjmp __ZN2Ev12trade_officeEv");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4C0315f0a73Ev\n__ZN4C0315f0a73Ev:\n\tjmp __ZN2Ev10altrogolanEv");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4C0315f0ab1Ev\n__ZN4C0315f0ab1Ev:\n\tjmp __ZN2Ev6ailianEv");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4C0315f0c56Ev\n__ZN4C0315f0c56Ev:\n\tjmp __ZN2Ev8vindariaEv");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4C0315f0d1eEv\n__ZN4C0315f0d1eEv:\n\tjmp __ZN2Ev7jandoraEv");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4C0315f0d53Ev\n__ZN4C0315f0d53Ev:\n\tjmp __ZN2Ev6draganEv");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4C0315f0f62Ev\n__ZN4C0315f0f62Ev:\n\tjmp __ZN2Ev7talk_49Ev");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4C0315f0fd7Ev\n__ZN4C0315f0fd7Ev:\n\tjmp __ZN2Ev6zaskiaEv");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4C0315f1039Ev\n__ZN4C0315f1039Ev:\n\tjmp __ZN2Ev7talk_4aEv");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4C0315f10aeEv\n__ZN4C0315f10aeEv:\n\tjmp __ZN2Ev15peddler_redheadEv");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4C0315f1130Ev\n__ZN4C0315f1130Ev:\n\tjmp __ZN2Ev7falkertEv");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4C0315f11b9Ev\n__ZN4C0315f11b9Ev:\n\tjmp __ZN2Ev9deringornEv");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4C0315f1216Ev\n__ZN4C0315f1216Ev:\n\tjmp __ZN2Ev13offering_bowlEv");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4C0319f184_0479Ev\n__ZN4C0319f184_0479Ev:\n\tjmp __ZN2Ev12show_pic_197Ev");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4C031C1Ev\n__ZN4C031C1Ev:\n\tjmp __ZN2EvC1Ev");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4C031D1Ev\n__ZN4C031D1Ev:\n\tjmp __ZN2EvD1Ev");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4C0443runEv\n__ZN4C0443runEv:\n\tjmp _temple_run");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4C0445checkEv\n__ZN4C0445checkEv:\n\tjmp _temple_is_open");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4C044C1Ev\n__ZN4C044C1Ev:\n\tjmp __ZN6TempleC1Ev");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4C044D1Ev\n__ZN4C044D1Ev:\n\tjmp __ZN6TempleD1Ev");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4C0703runEv\n__ZN4C0703runEv:\n\tjmp __ZN7Lodging3runEv");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4C0705checkEv\n__ZN4C0705checkEv:\n\tjmp __ZN7Lodging7is_openEv");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4C070C1Ev\n__ZN4C070C1Ev:\n\tjmp __ZN7LodgingC1Ev");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4C070D1Ev\n__ZN4C070D1Ev:\n\tjmp __ZN7LodgingD1Ev");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4C0713runEa\n__ZN4C0713runEa:\n\tjmp __ZN4Camp3runEa");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4C071C1Ev\n__ZN4C071C1Ev:\n\tjmp __ZN4CampC1Ev");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4C071D1Ev\n__ZN4C071D1Ev:\n\tjmp __ZN4CampD1Ev");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4C1665f0d3bEv\n__ZN4C1665f0d3bEv:\n\tjmp __ZN5Sites9challengeEv");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4C1665f1313Ev\n__ZN4C1665f1313Ev:\n\tjmp __ZN5Sites7rematchEv");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4C1665f1687Ev\n__ZN4C1665f1687Ev:\n\tjmp __ZN5Sites13evening_fightEv");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4C1665f1a37Ev\n__ZN4C1665f1a37Ev:\n\tjmp __ZN5Sites7exploreEv");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4C166C1Ev\n__ZN4C166C1Ev:\n\tjmp __ZN5SitesC1Ev");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4C166D1Ev\n__ZN4C166D1Ev:\n\tjmp __ZN5SitesD1Ev");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4C1733runEv\n__ZN4C1733runEv:\n\tjmp __ZN5House3runEv");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4C173C1Ev\n__ZN4C173C1Ev:\n\tjmp __ZN5HouseC1Ev");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4C173D1Ev\n__ZN4C173D1Ev:\n\tjmp __ZN5HouseD1Ev");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4C1743runEv\n__ZN4C1743runEv:\n\tjmp __ZN6Healer3runEv");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4C1745checkEv\n__ZN4C1745checkEv:\n\tjmp __ZN6Healer7is_openEv");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4C174C1Ev\n__ZN4C174C1Ev:\n\tjmp __ZN6HealerC1Ev");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4C174D1Ev\n__ZN4C174D1Ev:\n\tjmp __ZN6HealerD1Ev");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4C1783runEv\n__ZN4C1783runEv:\n\tjmp __ZN3Inn3runEv");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4C1785checkEv\n__ZN4C1785checkEv:\n\tjmp __ZN3Inn7is_openEv");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4C178C1Ev\n__ZN4C178C1Ev:\n\tjmp __ZN3InnC1Ev");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4C178D1Ev\n__ZN4C178D1Ev:\n\tjmp __ZN3InnD1Ev");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4C2263runEv\n__ZN4C2263runEv:\n\tjmp __ZN4Shop3runEv");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4C2265checkEv\n__ZN4C2265checkEv:\n\tjmp __ZN4Shop7is_openEv");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4C226C1Ev\n__ZN4C226C1Ev:\n\tjmp __ZN4ShopC1Ev");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4C226D1Ev\n__ZN4C226D1Ev:\n\tjmp __ZN4ShopD1Ev");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4C2303runEv\n__ZN4C2303runEv:\n\tjmp __ZN5Smith3runEv");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4C2305checkEv\n__ZN4C2305checkEv:\n\tjmp __ZN5Smith10check_openEv");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4C230C1Ev\n__ZN4C230C1Ev:\n\tjmp __ZN5SmithC1Ev");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4C230D1Ev\n__ZN4C230D1Ev:\n\tjmp __ZN5SmithD1Ev");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4C2403runEv\n__ZN4C2403runEv:\n\tjmp __ZN6Tavern3runEv");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4C2405checkEv\n__ZN4C2405checkEv:\n\tjmp __ZN6Tavern5admitEv");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4C240C1Ev\n__ZN4C240C1Ev:\n\tjmp __ZN6TavernC1Ev");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4C240D1Ev\n__ZN4C240D1Ev:\n\tjmp __ZN6TavernD1Ev");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast11spell_blitzEv\n__ZN4Cast11spell_blitzEv:\n\tjmp _spell_blitz");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast11spell_radauEv\n__ZN4Cast11spell_radauEv:\n\tjmp _spell_radau");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast12spell_balsamEv\n__ZN4Cast12spell_balsamEv:\n\tjmp _spell_balsam");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast13spell_analuesEv\n__ZN4Cast13spell_analuesEv:\n\tjmp _spell_analues");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast13spell_foramenEv\n__ZN4Cast13spell_foramenEv:\n\tjmp _spell_foramen");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast13spell_paralueEv\n__ZN4Cast13spell_paralueEv:\n\tjmp _spell_paralue");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast13spell_spurlosEv\n__ZN4Cast13spell_spurlosEv:\n\tjmp _spell_spurlos");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast14spell_adleraugEv\n__ZN4Cast14spell_adleraugEv:\n\tjmp _spell_adleraug");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast14spell_aeolitusEv\n__ZN4Cast14spell_aeolitusEv:\n\tjmp _spell_aeolitus");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast14spell_exposamiEv\n__ZN4Cast14spell_exposamiEv:\n\tjmp _spell_exposami");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast14spell_heptagonEv\n__ZN4Cast14spell_heptagonEv:\n\tjmp _spell_heptagon");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast14spell_mutabiliEv\n__ZN4Cast14spell_mutabiliEv:\n\tjmp _spell_mutabili");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast14spell_salanderEv\n__ZN4Cast14spell_salanderEv:\n\tjmp _spell_salander");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast14spell_sanftmutEv\n__ZN4Cast14spell_sanftmutEv:\n\tjmp _spell_sanftmut");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast14spell_sensibarEv\n__ZN4Cast14spell_sensibarEv:\n\tjmp _spell_sensibar");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast14spell_visibiliEv\n__ZN4Cast14spell_visibiliEv:\n\tjmp _spell_visibili");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast15spell_abvenenumEv\n__ZN4Cast15spell_abvenenumEv:\n\tjmp _spell_abvenenum");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast15spell_armatrutzEv\n__ZN4Cast15spell_armatrutzEv:\n\tjmp _spell_armatrutz");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast15spell_claudibusEv\n__ZN4Cast15spell_claudibusEv:\n\tjmp _spell_claudibus");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast15spell_eisenrostEv\n__ZN4Cast15spell_eisenrostEv:\n\tjmp _spell_eisenrost");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast15spell_feuerbannEv\n__ZN4Cast15spell_feuerbannEv:\n\tjmp _spell_feuerbann");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast15spell_flim_flamEv\n__ZN4Cast15spell_flim_flamEv:\n\tjmp _spell_flim_flam");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast15spell_motoricusEv\n__ZN4Cast15spell_motoricusEv:\n\tjmp _spell_motoricus");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast15spell_silentiumEv\n__ZN4Cast15spell_silentiumEv:\n\tjmp _spell_silentium");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast15spell_solidiridEv\n__ZN4Cast15spell_solidiridEv:\n\tjmp _spell_solidirid");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast15spell_ueber_eisEv\n__ZN4Cast15spell_ueber_eisEv:\n\tjmp _spell_ueber_eis");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast15spell_zwingtanzEv\n__ZN4Cast15spell_zwingtanzEv:\n\tjmp _spell_zwingtanz");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast16spell_adler_wolfEv\n__ZN4Cast16spell_adler_wolfEv:\n\tjmp _spell_adler_wolf");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast16spell_dunkelheitEv\n__ZN4Cast16spell_dunkelheitEv:\n\tjmp _spell_dunkelheit");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast16spell_duplicatusEv\n__ZN4Cast16spell_duplicatusEv:\n\tjmp _spell_duplicatus");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast16spell_furor_blutEv\n__ZN4Cast16spell_furor_blutEv:\n\tjmp _spell_furor_blut");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast16spell_hexenblickEv\n__ZN4Cast16spell_hexenblickEv:\n\tjmp _spell_hexenblick");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast16spell_ignifaxiusEv\n__ZN4Cast16spell_ignifaxiusEv:\n\tjmp _spell_ignifaxius");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast16spell_kraehenrufEv\n__ZN4Cast16spell_kraehenrufEv:\n\tjmp _spell_kraehenruf");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast16spell_respondamiEv\n__ZN4Cast16spell_respondamiEv:\n\tjmp _spell_respondami");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast16spell_saft_kraftEv\n__ZN4Cast16spell_saft_kraftEv:\n\tjmp _spell_saft_kraft");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast17spell_axxeleratusEv\n__ZN4Cast17spell_axxeleratusEv:\n\tjmp _spell_axxeleratus");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast17spell_ch_steigernEv\n__ZN4Cast17spell_ch_steigernEv:\n\tjmp _spell_ch_steigern");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast17spell_chamaelioniEv\n__ZN4Cast17spell_chamaelioniEv:\n\tjmp _spell_chamaelioni");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast17spell_ff_steigernEv\n__ZN4Cast17spell_ff_steigernEv:\n\tjmp _spell_ff_steigern");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast17spell_fulminictusEv\n__ZN4Cast17spell_fulminictusEv:\n\tjmp _spell_fulminictus");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast17spell_ge_steigernEv\n__ZN4Cast17spell_ge_steigernEv:\n\tjmp _spell_ge_steigern");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast17spell_grosse_gierEv\n__ZN4Cast17spell_grosse_gierEv:\n\tjmp _spell_grosse_gier");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast17spell_hexenknotenEv\n__ZN4Cast17spell_hexenknotenEv:\n\tjmp _spell_hexenknoten");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast17spell_horriphobusEv\n__ZN4Cast17spell_horriphobusEv:\n\tjmp _spell_horriphobus");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast17spell_in_steigernEv\n__ZN4Cast17spell_in_steigernEv:\n\tjmp _spell_in_steigern");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast17spell_kk_steigernEv\n__ZN4Cast17spell_kk_steigernEv:\n\tjmp _spell_kk_steigern");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast17spell_kl_steigernEv\n__ZN4Cast17spell_kl_steigernEv:\n\tjmp _spell_kl_steigern");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast17spell_mu_steigernEv\n__ZN4Cast17spell_mu_steigernEv:\n\tjmp _spell_mu_steigern");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast17spell_nekropathiaEv\n__ZN4Cast17spell_nekropathiaEv:\n\tjmp _spell_nekropathia");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast17spell_penetrizzelEv\n__ZN4Cast17spell_penetrizzelEv:\n\tjmp _spell_penetrizzel");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast17spell_somnigravisEv\n__ZN4Cast17spell_somnigravisEv:\n\tjmp _spell_somnigravis");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast18spell_klarum_purumEv\n__ZN4Cast18spell_klarum_purumEv:\n\tjmp _spell_klarum_purum");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast18spell_odem_arcanumEv\n__ZN4Cast18spell_odem_arcanumEv:\n\tjmp _spell_odem_arcanum");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast18spell_plumbumbarumEv\n__ZN4Cast18spell_plumbumbarumEv:\n\tjmp _spell_plumbumbarum");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast18spell_ruhe_koerperEv\n__ZN4Cast18spell_ruhe_koerperEv:\n\tjmp _spell_ruhe_koerper");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast18spell_skelettariusEv\n__ZN4Cast18spell_skelettariusEv:\n\tjmp _spell_skelettarius");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast19spell_ecliptifactusEv\n__ZN4Cast19spell_ecliptifactusEv:\n\tjmp _spell_ecliptifactus");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast19spell_eigenschaftenEv\n__ZN4Cast19spell_eigenschaftenEv:\n\tjmp _spell_eigenschaften");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast19spell_hexenspeichelEv\n__ZN4Cast19spell_hexenspeichelEv:\n\tjmp _spell_hexenspeichel");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast19spell_scharfes_augeEv\n__ZN4Cast19spell_scharfes_augeEv:\n\tjmp _spell_scharfes_auge");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast19spell_sturmgebruellEv\n__ZN4Cast19spell_sturmgebruellEv:\n\tjmp _spell_sturmgebruell");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast19spell_transversalisEv\n__ZN4Cast19spell_transversalisEv:\n\tjmp _spell_transversalis");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast20spell_magischer_raubEv\n__ZN4Cast20spell_magischer_raubEv:\n\tjmp _spell_magischer_raub");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast21spell_hartes_schmelzeEv\n__ZN4Cast21spell_hartes_schmelzeEv:\n\tjmp _spell_hartes_schmelze");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast22spell_harmlose_gestaltEv\n__ZN4Cast22spell_harmlose_gestaltEv:\n\tjmp _spell_harmlose_gestalt");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast22spell_in_see_und_flussEv\n__ZN4Cast22spell_in_see_und_flussEv:\n\tjmp _spell_in_see_und_fluss");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast22spell_tiere_besprechenEv\n__ZN4Cast22spell_tiere_besprechenEv:\n\tjmp _spell_tiere_besprechen");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast22spell_weiches_erstarreEv\n__ZN4Cast22spell_weiches_erstarreEv:\n\tjmp _spell_weiches_erstarre");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast23spell_grosse_verwirrungEv\n__ZN4Cast23spell_grosse_verwirrungEv:\n\tjmp _spell_grosse_verwirrung");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast24spell_brenne_toter_stoffEv\n__ZN4Cast24spell_brenne_toter_stoffEv:\n\tjmp _spell_brenne_toter_stoff");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast24spell_geister_austreibenEv\n__ZN4Cast24spell_geister_austreibenEv:\n\tjmp _spell_geister_austreiben");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast24spell_nihilatio_gravitasEv\n__ZN4Cast24spell_nihilatio_gravitasEv:\n\tjmp _spell_nihilatio_gravitas");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast25spell_geister_beschwoerenEv\n__ZN4Cast25spell_geister_beschwoerenEv:\n\tjmp _spell_geister_beschwoeren");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast27spell_arcano_psychostabilisEv\n__ZN4Cast27spell_arcano_psychostabilisEv:\n\tjmp _spell_arcano_psychostabilis");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast28spell_elementare_beschwoerenEv\n__ZN4Cast28spell_elementare_beschwoerenEv:\n\tjmp _spell_elementare_beschwoeren");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN4Cast30spell_herr_ueber_das_tierreichEv\n__ZN4Cast30spell_herr_ueber_das_tierreichEv:\n\tjmp _spell_herr_ueber_das_tierreich");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN6NvfMem6unpackEPvS0_scPsS1_s\n__ZN6NvfMem6unpackEPvS0_scPsS1_s:\n\tjmp _nvf_unpack");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN6NvfMem6unpackEPvS0_ssPsS1_s\n__ZN6NvfMem6unpackEPvS0_ssPsS1_s:\n\tjmp _nvf_unpack");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN6NvfMemC1Ev\n__ZN6NvfMemC1Ev:\n\tjmp _nvf_new");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN6NvfMemD1Ev\n__ZN6NvfMemD1Ev:\n\tjmp __wrap_Nvf_dtor");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN7CharGen3runEs\n__ZN7CharGen3runEs:\n\tjmp _chargen_run");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN7CharGen3runEv\n__ZN7CharGen3runEv:\n\tjmp _chargen_run");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN7NvfMem06unpackEPvS0_ssPsS1_s\n__ZN7NvfMem06unpackEPvS0_ssPsS1_s:\n\tjmp _nvf_unpack");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN7NvfMem0C1Ev\n__ZN7NvfMem0C1Ev:\n\tjmp _nvf_new");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN7NvfMem0D1Ev\n__ZN7NvfMem0D1Ev:\n\tjmp __wrap_Nvf_dtor");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN7NvfMemS6unpackEPvS0_ssPsS1_s\n__ZN7NvfMemS6unpackEPvS0_ssPsS1_s:\n\tjmp _nvf_unpack");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN7NvfMemSC1Ev\n__ZN7NvfMemSC1Ev:\n\tjmp _nvf_new");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN7NvfMemSD1Ev\n__ZN7NvfMemSD1Ev:\n\tjmp __wrap_Nvf_dtor");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN7NvfMemU6unpackEPvS0_scPtS1_s\n__ZN7NvfMemU6unpackEPvS0_scPtS1_s:\n\tjmp _nvf_unpack");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN7NvfMemUC1Ev\n__ZN7NvfMemUC1Ev:\n\tjmp _nvf_new");
__asm__(".text\n\t.p2align 2\n\t.globl __ZN7NvfMemUD1Ev\n__ZN7NvfMemUD1Ev:\n\tjmp __wrap_Nvf_dtor");

/* C -> C++ Konstruktor-, Destruktor- und Methoden-Bruecken */
extern "C" void __ZN5Anim3C1Ev_call(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) __asm__("__ZN5Anim3C1Ev");
extern "C" void *Anim3_ctor(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) {
    if (!self) self = mem_alloc(4192);
    __ZN5Anim3C1Ev_call(self, a1, a2, a3, a4, a5, a6, a7, a8);
    return self;
}
extern "C" void __ZN5Anim3D1Ev_call(void *self) __asm__("__ZN5Anim3D1Ev");
extern "C" void Anim3_dtor(void *self, short flags) {
    if (!self) return;
    __ZN5Anim3D1Ev_call(self);
    if (flags & 1) mem_free(self);
}
__asm__(".text\n\t.p2align 2\n\t.globl _Anim3_run\n_Anim3_run:\n\tjmp __ZN5Anim33runEv");
extern "C" void __ZN4AnimC1Esss_call(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) __asm__("__ZN4AnimC1Esss");
extern "C" void *Anim_ctor(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) {
    if (!self) self = mem_alloc(4192);
    __ZN4AnimC1Esss_call(self, a1, a2, a3, a4, a5, a6, a7, a8);
    return self;
}
extern "C" void __ZN4AnimD1Ev_call(void *self) __asm__("__ZN4AnimD1Ev");
extern "C" void Anim_dtor(void *self, short flags) {
    if (!self) return;
    __ZN4AnimD1Ev_call(self);
    if (flags & 1) mem_free(self);
}
__asm__(".text\n\t.p2align 2\n\t.globl _Anim_pal\n_Anim_pal:\n\tjmp __ZN4Anim7pal_setEss");
__asm__(".text\n\t.p2align 2\n\t.globl _Anim_rect\n_Anim_rect:\n\tjmp __ZN4Anim4rectEPsS0_S0_S0_");
__asm__(".text\n\t.p2align 2\n\t.globl _Anim_running\n_Anim_running:\n\tjmp __ZN4Anim7runningEv");
__asm__(".text\n\t.p2align 2\n\t.globl _Anim_select_seq\n_Anim_select_seq:\n\tjmp __ZN4Anim10select_seqEs");
__asm__(".text\n\t.p2align 2\n\t.globl _Anim_setbg\n_Anim_setbg:\n\tjmp __ZN4Anim5setbgEP3Bmp");
__asm__(".text\n\t.p2align 2\n\t.globl _Anim_setbuf\n_Anim_setbuf:\n\tjmp __ZN4Anim6setbufEssPv");
__asm__(".text\n\t.p2align 2\n\t.globl _Anim_size\n_Anim_size:\n\tjmp __ZN4Anim4sizeEPsS0_");
__asm__(".text\n\t.p2align 2\n\t.globl _Anim_start\n_Anim_start:\n\tjmp __ZN4Anim5startEsss");
__asm__(".text\n\t.p2align 2\n\t.globl _Anim_step\n_Anim_step:\n\tjmp __ZN4Anim8wait_cueEv");
__asm__(".text\n\t.p2align 2\n\t.globl _Anim_stop\n_Anim_stop:\n\tjmp __ZN4Anim4stopEv");
__asm__(".text\n\t.p2align 2\n\t.globl _Anim_tick\n_Anim_tick:\n\tjmp __ZN4Anim4tickEv");
__asm__(".text\n\t.p2align 2\n\t.globl _Anim_unpack\n_Anim_unpack:\n\tjmp __ZN4Anim6unpackEP3Bmp");
__asm__(".text\n\t.p2align 2\n\t.globl _Anim_wait\n_Anim_wait:\n\tjmp __ZN4Anim10wait_frameEs");
extern "C" void __ZN7AutomapC1Ev_call(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) __asm__("__ZN7AutomapC1Ev");
extern "C" void *Automap_ctor(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) {
    if (!self) self = mem_alloc(4192);
    __ZN7AutomapC1Ev_call(self, a1, a2, a3, a4, a5, a6, a7, a8);
    return self;
}
extern "C" void __ZN7AutomapD1Ev_call(void *self) __asm__("__ZN7AutomapD1Ev");
extern "C" void Automap_dtor(void *self, short flags) {
    if (!self) return;
    __ZN7AutomapD1Ev_call(self);
    if (flags & 1) mem_free(self);
}
extern "C" void __ZN4BarsC1Ev_call(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) __asm__("__ZN4BarsC1Ev");
extern "C" void *Bars_ctor(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) {
    if (!self) self = mem_alloc(4192);
    __ZN4BarsC1Ev_call(self, a1, a2, a3, a4, a5, a6, a7, a8);
    return self;
}
__asm__(".text\n\t.p2align 2\n\t.globl _Bars_draw\n_Bars_draw:\n\tjmp __ZN4Bars4drawEsssssPv");
__asm__(".text\n\t.p2align 2\n\t.globl _Bars_tick\n_Bars_tick:\n\tjmp __ZN4Bars4tickEv");
__asm__(".text\n\t.p2align 2\n\t.globl _CD_music\n_CD_music:\n\tjmp __ZN2CD5musicEs");
__asm__(".text\n\t.p2align 2\n\t.globl _CD_stop\n_CD_stop:\n\tjmp __ZN2CD4stopEv");
__asm__(".text\n\t.p2align 2\n\t.globl _CD_update\n_CD_update:\n\tjmp __ZN2CD6updateEv");
__asm__(".text\n\t.p2align 2\n\t.globl _CD_volume\n_CD_volume:\n\tjmp __ZN2CD6volumeEs");
extern "C" void __ZN5CacheC1Esl_call(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) __asm__("__ZN5CacheC1Esl");
extern "C" void *Cache_ctor(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) {
    if (!self) self = mem_alloc(4192);
    __ZN5CacheC1Esl_call(self, a1, a2, a3, a4, a5, a6, a7, a8);
    return self;
}
extern "C" void __ZN5CacheD1Ev_call(void *self) __asm__("__ZN5CacheD1Ev");
extern "C" void Cache_dtor(void *self, short flags) {
    if (!self) return;
    __ZN5CacheD1Ev_call(self);
    if (flags & 1) mem_free(self);
}
extern "C" void __ZN4CampC1Ev_call(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) __asm__("__ZN4CampC1Ev");
extern "C" void *Camp_ctor(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) {
    if (!self) self = mem_alloc(4192);
    __ZN4CampC1Ev_call(self, a1, a2, a3, a4, a5, a6, a7, a8);
    return self;
}
extern "C" void __ZN4CampD1Ev_call(void *self) __asm__("__ZN4CampD1Ev");
extern "C" void Camp_dtor(void *self, short flags) {
    if (!self) return;
    __ZN4CampD1Ev_call(self);
    if (flags & 1) mem_free(self);
}
extern "C" void __ZN5ChestC1Ess_call(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) __asm__("__ZN5ChestC1Ess");
extern "C" void *Chest_ctor(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) {
    if (!self) self = mem_alloc(4192);
    __ZN5ChestC1Ess_call(self, a1, a2, a3, a4, a5, a6, a7, a8);
    return self;
}
extern "C" void __ZN5ChestD1Ev_call(void *self) __asm__("__ZN5ChestD1Ev");
extern "C" void Chest_dtor(void *self, short flags) {
    if (!self) return;
    __ZN5ChestD1Ev_call(self);
    if (flags & 1) mem_free(self);
}
extern "C" void __ZN4ConvC1Ev_call(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) __asm__("__ZN4ConvC1Ev");
extern "C" void *Conv_ctor(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) {
    if (!self) self = mem_alloc(4192);
    __ZN4ConvC1Ev_call(self, a1, a2, a3, a4, a5, a6, a7, a8);
    return self;
}
extern "C" void __ZN4ConvD1Ev_call(void *self) __asm__("__ZN4ConvD1Ev");
extern "C" void Conv_dtor(void *self, short flags) {
    if (!self) return;
    __ZN4ConvD1Ev_call(self);
    if (flags & 1) mem_free(self);
}
extern "C" void __ZN8CopyProtC1Ev_call(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) __asm__("__ZN8CopyProtC1Ev");
extern "C" void *CopyProt_ctor(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) {
    if (!self) self = mem_alloc(4192);
    __ZN8CopyProtC1Ev_call(self, a1, a2, a3, a4, a5, a6, a7, a8);
    return self;
}
extern "C" void __ZN8CopyProtD1Ev_call(void *self) __asm__("__ZN8CopyProtD1Ev");
extern "C" void CopyProt_dtor(void *self, short flags) {
    if (!self) return;
    __ZN8CopyProtD1Ev_call(self);
    if (flags & 1) mem_free(self);
}
__asm__(".text\n\t.p2align 2\n\t.globl _Date_alloc\n_Date_alloc:\n\tjmp __ZN4Date5allocEv");
extern "C" void __ZN4DateC1Ev_call(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) __asm__("__ZN4DateC1Ev");
extern "C" void *Date_ctor(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) {
    if (!self) self = mem_alloc(4192);
    __ZN4DateC1Ev_call(self, a1, a2, a3, a4, a5, a6, a7, a8);
    return self;
}
__asm__(".text\n\t.p2align 2\n\t.globl _Date_draw\n_Date_draw:\n\tjmp __ZN4Date4drawEs");
__asm__(".text\n\t.p2align 2\n\t.globl _Date_free\n_Date_free:\n\tjmp __ZN4Date7releaseEv");
__asm__(".text\n\t.p2align 2\n\t.globl _Date_load\n_Date_load:\n\tjmp __ZN4Date4loadEv");
__asm__(".text\n\t.p2align 2\n\t.globl _Date_print\n_Date_print:\n\tjmp __ZN4Date5printEv");
__asm__(".text\n\t.p2align 2\n\t.globl _Date_save\n_Date_save:\n\tjmp __ZN4Date4saveEv");
extern "C" void __ZN5DiaryC1Ev_call(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) __asm__("__ZN5DiaryC1Ev");
extern "C" void *Diary_ctor(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) {
    if (!self) self = mem_alloc(4192);
    __ZN5DiaryC1Ev_call(self, a1, a2, a3, a4, a5, a6, a7, a8);
    return self;
}
extern "C" void __ZN3DlgC1Ev_call(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) __asm__("__ZN3DlgC1Ev");
extern "C" void *Dlg_ctor(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) {
    if (!self) self = mem_alloc(4192);
    __ZN3DlgC1Ev_call(self, a1, a2, a3, a4, a5, a6, a7, a8);
    return self;
}
extern "C" void __ZN2EvC1Ev_call(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) __asm__("__ZN2EvC1Ev");
extern "C" void *Ev_ctor(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) {
    if (!self) self = mem_alloc(4192);
    __ZN2EvC1Ev_call(self, a1, a2, a3, a4, a5, a6, a7, a8);
    return self;
}
extern "C" void __ZN2EvD1Ev_call(void *self) __asm__("__ZN2EvD1Ev");
extern "C" void Ev_dtor(void *self, short flags) {
    if (!self) return;
    __ZN2EvD1Ev_call(self);
    if (flags & 1) mem_free(self);
}
extern "C" void __ZN11area_eventsD1Ev_call(void *self) __asm__("__ZN11area_eventsD1Ev");
extern "C" void Event_dtor(void *self, short flags) {
    if (!self) return;
    __ZN11area_eventsD1Ev_call(self);
    if (flags & 1) mem_free(self);
}
extern "C" void __ZN7FigAnimC1EP7ani_seqs_call(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) __asm__("__ZN7FigAnimC1EP7ani_seqs");
extern "C" void *FigAnim_ctor(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) {
    if (!self) self = mem_alloc(4192);
    __ZN7FigAnimC1EP7ani_seqs_call(self, a1, a2, a3, a4, a5, a6, a7, a8);
    return self;
}
extern "C" void __ZN7FigAnimD1Ev_call(void *self) __asm__("__ZN7FigAnimD1Ev");
extern "C" void FigAnim_dtor(void *self, short flags) {
    if (!self) return;
    __ZN7FigAnimD1Ev_call(self);
    if (flags & 1) mem_free(self);
}
extern "C" void __ZN4FileC1EPcPhPS0_S0_s_call(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) __asm__("__ZN4FileC1EPcPhPS0_S0_s");
extern "C" void *File_ctor(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) {
    if (!self) self = mem_alloc(4192);
    __ZN4FileC1EPcPhPS0_S0_s_call(self, a1, a2, a3, a4, a5, a6, a7, a8);
    return self;
}
extern "C" void __ZN4FileD1Ev_call(void *self) __asm__("__ZN4FileD1Ev");
extern "C" void File_dtor(void *self, short flags) {
    if (!self) return;
    __ZN4FileD1Ev_call(self);
    if (flags & 1) mem_free(self);
}
__asm__(".text\n\t.p2align 2\n\t.globl _File_exists\n_File_exists:\n\tjmp __ZN4File6existsEs");
__asm__(".text\n\t.p2align 2\n\t.globl _File_find\n_File_find:\n\tjmp __ZN4File4findEPc");
__asm__(".text\n\t.p2align 2\n\t.globl _File_read_all\n_File_read_all:\n\tjmp __ZN4File8read_allEPv");
__asm__(".text\n\t.p2align 2\n\t.globl _File_write\n_File_write:\n\tjmp __ZN4File5writeEPvl");
extern "C" void __ZN6FinaleC1Ev_call(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) __asm__("__ZN6FinaleC1Ev");
extern "C" void *Finale_ctor(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) {
    if (!self) self = mem_alloc(4192);
    __ZN6FinaleC1Ev_call(self, a1, a2, a3, a4, a5, a6, a7, a8);
    return self;
}
extern "C" void __ZN6FinaleD1Ev_call(void *self) __asm__("__ZN6FinaleD1Ev");
extern "C" void Finale_dtor(void *self, short flags) {
    if (!self) return;
    __ZN6FinaleD1Ev_call(self);
    if (flags & 1) mem_free(self);
}
__asm__(".text\n\t.p2align 2\n\t.globl _Finale_run\n_Finale_run:\n\tjmp __ZN6Finale3runEv");
extern "C" void __ZN4FontC1Es_call(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) __asm__("__ZN4FontC1Es");
extern "C" void *Font_ctor(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) {
    if (!self) self = mem_alloc(4192);
    __ZN4FontC1Es_call(self, a1, a2, a3, a4, a5, a6, a7, a8);
    return self;
}
extern "C" void __ZN4FontD1Ev_call(void *self) __asm__("__ZN4FontD1Ev");
extern "C" void Font_dtor(void *self, short flags) {
    if (!self) return;
    __ZN4FontD1Ev_call(self);
    if (flags & 1) mem_free(self);
}
extern "C" void __ZN5groupC1Ev_call(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) __asm__("__ZN5groupC1Ev");
extern "C" void *Group_ctor(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) {
    if (!self) self = mem_alloc(4192);
    __ZN5groupC1Ev_call(self, a1, a2, a3, a4, a5, a6, a7, a8);
    return self;
}
extern "C" void __ZN4InitC1Ev_call(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) __asm__("__ZN4InitC1Ev");
extern "C" void *Init_ctor(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) {
    if (!self) self = mem_alloc(4192);
    __ZN4InitC1Ev_call(self, a1, a2, a3, a4, a5, a6, a7, a8);
    return self;
}
extern "C" void __ZN4InitD1Ev_call(void *self) __asm__("__ZN4InitD1Ev");
extern "C" void Init_dtor(void *self, short flags) {
    if (!self) return;
    __ZN4InitD1Ev_call(self);
    if (flags & 1) mem_free(self);
}
extern "C" void __ZN5IntroC1Ev_call(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) __asm__("__ZN5IntroC1Ev");
extern "C" void *Intro_ctor_o039(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) {
    if (!self) self = mem_alloc(4192);
    __ZN5IntroC1Ev_call(self, a1, a2, a3, a4, a5, a6, a7, a8);
    return self;
}
extern "C" void __ZN5IntroD1Ev_call(void *self) __asm__("__ZN5IntroD1Ev");
extern "C" void Intro_dtor_o039(void *self, short flags) {
    if (!self) return;
    __ZN5IntroD1Ev_call(self);
    if (flags & 1) mem_free(self);
}
__asm__(".text\n\t.p2align 2\n\t.globl _Intro_run_o039\n_Intro_run_o039:\n\tjmp __ZN5Intro3runEv");
extern "C" void __ZN6InvObjC1Ev_call(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) __asm__("__ZN6InvObjC1Ev");
extern "C" void *Inv_ctor(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) {
    if (!self) self = mem_alloc(4192);
    __ZN6InvObjC1Ev_call(self, a1, a2, a3, a4, a5, a6, a7, a8);
    return self;
}
extern "C" void __ZN6InvObjD1Ev_call(void *self) __asm__("__ZN6InvObjD1Ev");
extern "C" void Inv_dtor(void *self, short flags) {
    if (!self) return;
    __ZN6InvObjD1Ev_call(self);
    if (flags & 1) mem_free(self);
}
extern "C" void __ZN5ItemsC1Ev_call(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) __asm__("__ZN5ItemsC1Ev");
extern "C" void *Items_ctor(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) {
    if (!self) self = mem_alloc(4192);
    __ZN5ItemsC1Ev_call(self, a1, a2, a3, a4, a5, a6, a7, a8);
    return self;
}
extern "C" void __ZN5ItemsD1Ev_call(void *self) __asm__("__ZN5ItemsD1Ev");
extern "C" void Items_dtor(void *self, short flags) {
    if (!self) return;
    __ZN5ItemsD1Ev_call(self);
    if (flags & 1) mem_free(self);
}
extern "C" void __ZN7KeylistC1Es_call(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) __asm__("__ZN7KeylistC1Es");
extern "C" void *Keylist_ctor(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) {
    if (!self) self = mem_alloc(4192);
    __ZN7KeylistC1Es_call(self, a1, a2, a3, a4, a5, a6, a7, a8);
    return self;
}
extern "C" void __ZN7KeylistD1Ev_call(void *self) __asm__("__ZN7KeylistD1Ev");
extern "C" void Keylist_dtor(void *self, short flags) {
    if (!self) return;
    __ZN7KeylistD1Ev_call(self);
    if (flags & 1) mem_free(self);
}
extern "C" void __ZN4LockC1Es_call(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) __asm__("__ZN4LockC1Es");
extern "C" void *Lock_ctor(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) {
    if (!self) self = mem_alloc(4192);
    __ZN4LockC1Es_call(self, a1, a2, a3, a4, a5, a6, a7, a8);
    return self;
}
extern "C" void __ZN4LockD1Ev_call(void *self) __asm__("__ZN4LockD1Ev");
extern "C" void Lock_dtor(void *self, short flags) {
    if (!self) return;
    __ZN4LockD1Ev_call(self);
    if (flags & 1) mem_free(self);
}
__asm__(".text\n\t.p2align 2\n\t.globl _Lst_add\n_Lst_add:\n\tjmp __ZN9FightMsgs3addEssaPvaS0_");
__asm__(".text\n\t.p2align 2\n\t.globl _Lst_add0\n_Lst_add0:\n\tjmp __ZN9FightMsgs4add0Ess");
__asm__(".text\n\t.p2align 2\n\t.globl _Lst_add2\n_Lst_add2:\n\tjmp __ZN9FightMsgs4add2EssaPv");
extern "C" void __ZN9FightMsgsC1Ev_call(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) __asm__("__ZN9FightMsgsC1Ev");
extern "C" void *Lst_ctor(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) {
    if (!self) self = mem_alloc(4192);
    __ZN9FightMsgsC1Ev_call(self, a1, a2, a3, a4, a5, a6, a7, a8);
    return self;
}
extern "C" void __ZN9FightMsgsD1Ev_call(void *self) __asm__("__ZN9FightMsgsD1Ev");
extern "C" void Lst_dtor(void *self, short flags) {
    if (!self) return;
    __ZN9FightMsgsD1Ev_call(self);
    if (flags & 1) mem_free(self);
}
__asm__(".text\n\t.p2align 2\n\t.globl _Lst_init\n_Lst_init:\n\tjmp __ZN9FightMsgs4initEv");
__asm__(".text\n\t.p2align 2\n\t.globl _Lst_run\n_Lst_run:\n\tjmp __ZN9FightMsgs3runEv");
__asm__(".text\n\t.p2align 2\n\t.globl _Lst_set\n_Lst_set:\n\tjmp __ZN9FightMsgs10set_screenEl");
__asm__(".text\n\t.p2align 2\n\t.globl _Lvl_run\n_Lvl_run:\n\tjmp __ZN3Lvl3runEP4hero");
extern "C" void __ZN3NvfC1Ev_call(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) __asm__("__ZN3NvfC1Ev");
extern "C" void *Nvf_ctor(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) {
    if (!self) self = mem_alloc(4192);
    __ZN3NvfC1Ev_call(self, a1, a2, a3, a4, a5, a6, a7, a8);
    return self;
}
extern "C" void __ZN3NvfD1Ev_call(void *self) __asm__("__ZN3NvfD1Ev");
extern "C" void Nvf_dtor(void *self, short flags) {
    if (!self) return;
    __ZN3NvfD1Ev_call(self);
    if (flags & 1) mem_free(self);
}
extern "C" void __ZN3OptC1Ev_call(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) __asm__("__ZN3OptC1Ev");
extern "C" void *Opt_ctor_o199(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) {
    if (!self) self = mem_alloc(4192);
    __ZN3OptC1Ev_call(self, a1, a2, a3, a4, a5, a6, a7, a8);
    return self;
}
extern "C" void __ZN3OptD1Ev_call(void *self) __asm__("__ZN3OptD1Ev");
extern "C" void Opt_dtor_o199(void *self, short flags) {
    if (!self) return;
    __ZN3OptD1Ev_call(self);
    if (flags & 1) mem_free(self);
}
__asm__(".text\n\t.p2align 2\n\t.globl _Opt_run_o199\n_Opt_run_o199:\n\tjmp __ZN3Opt3runEv");
extern "C" void __ZN6Outro1C1Ev_call(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) __asm__("__ZN6Outro1C1Ev");
extern "C" void *Outro1_ctor(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) {
    if (!self) self = mem_alloc(4192);
    __ZN6Outro1C1Ev_call(self, a1, a2, a3, a4, a5, a6, a7, a8);
    return self;
}
extern "C" void __ZN6Outro1D1Ev_call(void *self) __asm__("__ZN6Outro1D1Ev");
extern "C" void Outro1_dtor(void *self, short flags) {
    if (!self) return;
    __ZN6Outro1D1Ev_call(self);
    if (flags & 1) mem_free(self);
}
__asm__(".text\n\t.p2align 2\n\t.globl _Outro1_run\n_Outro1_run:\n\tjmp __ZN6Outro13runEv");
extern "C" void __ZN6Outro2C1Ev_call(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) __asm__("__ZN6Outro2C1Ev");
extern "C" void *Outro2_ctor(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) {
    if (!self) self = mem_alloc(4192);
    __ZN6Outro2C1Ev_call(self, a1, a2, a3, a4, a5, a6, a7, a8);
    return self;
}
extern "C" void __ZN6Outro2D1Ev_call(void *self) __asm__("__ZN6Outro2D1Ev");
extern "C" void Outro2_dtor(void *self, short flags) {
    if (!self) return;
    __ZN6Outro2D1Ev_call(self);
    if (flags & 1) mem_free(self);
}
__asm__(".text\n\t.p2align 2\n\t.globl _Outro2_run\n_Outro2_run:\n\tjmp __ZN6Outro23runEv");
extern "C" void __ZN7PortTxtC1Ev_call(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) __asm__("__ZN7PortTxtC1Ev");
extern "C" void *PortTxt_ctor(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) {
    if (!self) self = mem_alloc(4192);
    __ZN7PortTxtC1Ev_call(self, a1, a2, a3, a4, a5, a6, a7, a8);
    return self;
}
extern "C" void __ZN7PortTxtD1Ev_call(void *self) __asm__("__ZN7PortTxtD1Ev");
extern "C" void PortTxt_dtor(void *self, short flags) {
    if (!self) return;
    __ZN7PortTxtD1Ev_call(self);
    if (flags & 1) mem_free(self);
}
extern "C" void __ZN6ScreenC1Ev_call(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) __asm__("__ZN6ScreenC1Ev");
extern "C" void *Screen_ctor(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) {
    if (!self) self = mem_alloc(4192);
    __ZN6ScreenC1Ev_call(self, a1, a2, a3, a4, a5, a6, a7, a8);
    return self;
}
extern "C" void __ZN5SoundC1EP4FileP3arcS1__call(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) __asm__("__ZN5SoundC1EP4FileP3arcS1_");
extern "C" void *Sound_ctor(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) {
    if (!self) self = mem_alloc(4192);
    __ZN5SoundC1EP4FileP3arcS1__call(self, a1, a2, a3, a4, a5, a6, a7, a8);
    return self;
}
extern "C" void __ZN5SoundC1EP3arcP4File_call(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) __asm__("__ZN5SoundC1EP3arcP4File");
extern "C" void *Sound_ctor_o042(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) {
    if (!self) self = mem_alloc(4192);
    __ZN5SoundC1EP3arcP4File_call(self, a1, a2, a3, a4, a5, a6, a7, a8);
    return self;
}
extern "C" void __ZN5SoundD1Ev_call(void *self) __asm__("__ZN5SoundD1Ev");
extern "C" void Sound_dtor(void *self, short flags) {
    if (!self) return;
    __ZN5SoundD1Ev_call(self);
    if (flags & 1) mem_free(self);
}
__asm__(".text\n\t.p2align 2\n\t.globl _Sound_mtype\n_Sound_mtype:\n\tjmp __ZN5Sound5mtypeEv");
__asm__(".text\n\t.p2align 2\n\t.globl _Sound_nocallback\n_Sound_nocallback:\n\tjmp __ZN5Sound10nocallbackEv");
__asm__(".text\n\t.p2align 2\n\t.globl _Sound_play\n_Sound_play:\n\tjmp __ZN5Sound4playEs");
__asm__(".text\n\t.p2align 2\n\t.globl _Sound_start\n_Sound_start:\n\tjmp __ZN5Sound5startEs");
__asm__(".text\n\t.p2align 2\n\t.globl _Sound_stop_music\n_Sound_stop_music:\n\tjmp __ZN5Sound10stop_musicEv");
__asm__(".text\n\t.p2align 2\n\t.globl _Sound_volume2\n_Sound_volume2:\n\tjmp __ZN5Sound7volume2Ess");
__asm__(".text\n\t.p2align 2\n\t.globl _Sound_volume3\n_Sound_volume3:\n\tjmp __ZN5Sound7volume3Esss");
__asm__(".text\n\t.p2align 2\n\t.globl _Sound_wait\n_Sound_wait:\n\tjmp __ZN5Sound4waitEv");
__asm__(".text\n\t.p2align 2\n\t.globl _Speech_set\n_Speech_set:\n\tjmp __ZN6Speech3setEs");
__asm__(".text\n\t.p2align 2\n\t.globl _Speech_stop\n_Speech_stop:\n\tjmp __ZN6Speech4stopEv");
__asm__(".text\n\t.p2align 2\n\t.globl _Speech_volume\n_Speech_volume:\n\tjmp __ZN6Speech6volumeEs");
__asm__(".text\n\t.p2align 2\n\t.globl _Speech_wait\n_Speech_wait:\n\tjmp __ZN6Speech4waitEv");
extern "C" void __ZN5StashC1Ess_call(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) __asm__("__ZN5StashC1Ess");
extern "C" void *Stash_ctor(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) {
    if (!self) self = mem_alloc(4192);
    __ZN5StashC1Ess_call(self, a1, a2, a3, a4, a5, a6, a7, a8);
    return self;
}
extern "C" void __ZN5StashD1Ev_call(void *self) __asm__("__ZN5StashD1Ev");
extern "C" void Stash_dtor(void *self, short flags) {
    if (!self) return;
    __ZN5StashD1Ev_call(self);
    if (flags & 1) mem_free(self);
}
extern "C" void __ZN4TMapC1Ev_call(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) __asm__("__ZN4TMapC1Ev");
extern "C" void *TMap_ctor(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) {
    if (!self) self = mem_alloc(4192);
    __ZN4TMapC1Ev_call(self, a1, a2, a3, a4, a5, a6, a7, a8);
    return self;
}
extern "C" void __ZN4TMapD1Ev_call(void *self) __asm__("__ZN4TMapD1Ev");
extern "C" void TMap_dtor(void *self, short flags) {
    if (!self) return;
    __ZN4TMapD1Ev_call(self);
    if (flags & 1) mem_free(self);
}
extern "C" void __ZN8TempFileC1Ev_call(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) __asm__("__ZN8TempFileC1Ev");
extern "C" void *TempFile_ctor(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) {
    if (!self) self = mem_alloc(4192);
    __ZN8TempFileC1Ev_call(self, a1, a2, a3, a4, a5, a6, a7, a8);
    return self;
}
extern "C" void __ZN4TownC1Esss_call(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) __asm__("__ZN4TownC1Esss");
extern "C" void *Town_ctor(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) {
    if (!self) self = mem_alloc(4192);
    __ZN4TownC1Esss_call(self, a1, a2, a3, a4, a5, a6, a7, a8);
    return self;
}
extern "C" void __ZN4TownD1Ev_call(void *self) __asm__("__ZN4TownD1Ev");
extern "C" void Town_dtor(void *self, short flags) {
    if (!self) return;
    __ZN4TownD1Ev_call(self);
    if (flags & 1) mem_free(self);
}
extern "C" void __ZN6TravelC1Ev_call(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) __asm__("__ZN6TravelC1Ev");
extern "C" void *Travel_ctor(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) {
    if (!self) self = mem_alloc(4192);
    __ZN6TravelC1Ev_call(self, a1, a2, a3, a4, a5, a6, a7, a8);
    return self;
}
extern "C" void __ZN6TravelD1Ev_call(void *self) __asm__("__ZN6TravelD1Ev");
extern "C" void Travel_dtor(void *self, short flags) {
    if (!self) return;
    __ZN6TravelD1Ev_call(self);
    if (flags & 1) mem_free(self);
}
extern "C" void __ZN3WinD1Ev_call(void *self) __asm__("__ZN3WinD1Ev");
extern "C" void Win_dtor(void *self, short flags) {
    if (!self) return;
    __ZN3WinD1Ev_call(self);
    if (flags & 1) mem_free(self);
}
extern "C" void __ZN2WxC1Ev_call(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) __asm__("__ZN2WxC1Ev");
extern "C" void *Wx_ctor(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) {
    if (!self) self = mem_alloc(4192);
    __ZN2WxC1Ev_call(self, a1, a2, a3, a4, a5, a6, a7, a8);
    return self;
}
extern "C" void __ZN2WxD1Ev_call(void *self) __asm__("__ZN2WxD1Ev");
extern "C" void Wx_dtor(void *self, short flags) {
    if (!self) return;
    __ZN2WxD1Ev_call(self);
    if (flags & 1) mem_free(self);
}
__asm__(".text\n\t.p2align 2\n\t.globl _Wx_get\n_Wx_get:\n\tjmp __ZN2Wx3getEv");
__asm__(".text\n\t.p2align 2\n\t.globl _Wx_next\n_Wx_next:\n\tjmp __ZN2Wx4nextEv");
extern "C" void __ZN3AniC1Ev_call(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) __asm__("__ZN3AniC1Ev");
extern "C" void *ani_ctor(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) {
    if (!self) self = mem_alloc(4192);
    __ZN3AniC1Ev_call(self, a1, a2, a3, a4, a5, a6, a7, a8);
    return self;
}
extern "C" void __ZN3AniD1Ev_call(void *self) __asm__("__ZN3AniD1Ev");
extern "C" void ani_dtor(void *self, short flags) {
    if (!self) return;
    __ZN3AniD1Ev_call(self);
    if (flags & 1) mem_free(self);
}
__asm__(".text\n\t.p2align 2\n\t.globl _ani_halt\n_ani_halt:\n\tjmp __ZN3Ani4haltEv");
__asm__(".text\n\t.p2align 2\n\t.globl _ani_load\n_ani_load:\n\tjmp __ZN3Ani4loadEs");
__asm__(".text\n\t.p2align 2\n\t.globl _ani_play_until\n_ani_play_until:\n\tjmp __ZN3Ani10play_untilEsa");
__asm__(".text\n\t.p2align 2\n\t.globl _ani_set_pal_size\n_ani_set_pal_size:\n\tjmp __ZN3Ani12set_pal_sizeEs");
__asm__(".text\n\t.p2align 2\n\t.globl _ani_set_pos\n_ani_set_pos:\n\tjmp __ZN3Ani7set_posEss");
__asm__(".text\n\t.p2align 2\n\t.globl _ani_stop\n_ani_stop:\n\tjmp __ZN3Ani4stopEv");
__asm__(".text\n\t.p2align 2\n\t.globl _anim_free_all\n_anim_free_all:\n\tjmp __ZN3Ani8free_allEv");
__asm__(".text\n\t.p2align 2\n\t.globl _anim_load\n_anim_load:\n\tjmp __ZN3Ani10load_resetEss");
__asm__(".text\n\t.p2align 2\n\t.globl _anim_load_variant\n_anim_load_variant:\n\tjmp __ZN3Ani12load_variantEssa");
__asm__(".text\n\t.p2align 2\n\t.globl _anim_reset\n_anim_reset:\n\tjmp __ZN3Ani5resetEs");
__asm__(".text\n\t.p2align 2\n\t.globl _anim_select_variant\n_anim_select_variant:\n\tjmp __ZN3Ani14select_variantEsa");
__asm__(".text\n\t.p2align 2\n\t.globl _anim_tick\n_anim_tick:\n\tjmp __ZN3Ani4tickEv");
__asm__(".text\n\t.p2align 2\n\t.globl _automap_cell_read\n_automap_cell_read:\n\tjmp __ZN7Automap9cell_readEsss");
__asm__(".text\n\t.p2align 2\n\t.globl _automap_cell_write\n_automap_cell_write:\n\tjmp __ZN7Automap10cell_writeEsssa");
__asm__(".text\n\t.p2align 2\n\t.globl _automap_close\n_automap_close:\n\tjmp __ZN7Automap5closeEv");
__asm__(".text\n\t.p2align 2\n\t.globl _automap_draw\n_automap_draw:\n\tjmp __ZN7Automap4drawEv");
__asm__(".text\n\t.p2align 2\n\t.globl _automap_frame\n_automap_frame:\n\tjmp __ZN7Automap5frameEv");
__asm__(".text\n\t.p2align 2\n\t.globl _automap_free_cells\n_automap_free_cells:\n\tjmp __ZN7Automap10free_cellsEv");
__asm__(".text\n\t.p2align 2\n\t.globl _automap_init\n_automap_init:\n\tjmp __ZN7Automap4initEs");
__asm__(".text\n\t.p2align 2\n\t.globl _automap_load\n_automap_load:\n\tjmp __ZN7Automap4loadEv");
__asm__(".text\n\t.p2align 2\n\t.globl _automap_load_seen\n_automap_load_seen:\n\tjmp __ZN7Automap9load_seenEv");
__asm__(".text\n\t.p2align 2\n\t.globl _automap_open\n_automap_open:\n\tjmp __ZN7Automap4openEsss");
__asm__(".text\n\t.p2align 2\n\t.globl _automap_random_cell\n_automap_random_cell:\n\tjmp __ZN7Automap11random_cellEv");
__asm__(".text\n\t.p2align 2\n\t.globl _automap_run\n_automap_run:\n\tjmp __ZN7Automap3runEv");
__asm__(".text\n\t.p2align 2\n\t.globl _automap_save\n_automap_save:\n\tjmp __ZN7Automap4saveEv");
__asm__(".text\n\t.p2align 2\n\t.globl _automap_set_town_type\n_automap_set_town_type:\n\tjmp __ZN7Automap13set_town_typeEs");
__asm__(".text\n\t.p2align 2\n\t.globl _automap_show\n_automap_show:\n\tjmp __ZN7Automap4showEv");
__asm__(".text\n\t.p2align 2\n\t.globl _cache_clear\n_cache_clear:\n\tjmp __ZN5Cache5clearEv");
__asm__(".text\n\t.p2align 2\n\t.globl _cache_purge\n_cache_purge:\n\tjmp __ZN5Cache5purgeEs");
__asm__(".text\n\t.p2align 2\n\t.globl _camp_reset_watch\n_camp_reset_watch:\n\tjmp __ZN4Camp11reset_watchEaa");
__asm__(".text\n\t.p2align 2\n\t.globl _camp_run\n_camp_run:\n\tjmp __ZN4Camp3runEa");
__asm__(".text\n\t.p2align 2\n\t.globl _cast_damage\n_cast_damage:\n\tjmp __ZN4Cast6damageEs");
__asm__(".text\n\t.p2align 2\n\t.globl _cast_target_pa\n_cast_target_pa:\n\tjmp __ZN4Cast9target_paEv");
__asm__(".text\n\t.p2align 2\n\t.globl _conv_clr_counter\n_conv_clr_counter:\n\tjmp __ZN4Conv11clr_counterEs");
__asm__(".text\n\t.p2align 2\n\t.globl _conv_dec_counter\n_conv_dec_counter:\n\tjmp __ZN4Conv11dec_counterEs");
__asm__(".text\n\t.p2align 2\n\t.globl _conv_load\n_conv_load:\n\tjmp __ZN4Conv4loadEv");
__asm__(".text\n\t.p2align 2\n\t.globl _conv_menu\n_conv_menu:\n\tjmp __ZN4Conv4menuEs");
__asm__(".text\n\t.p2align 2\n\t.globl _conv_print\n_conv_print:\n\tjmp __ZN4Conv5printEPcs");
__asm__(".text\n\t.p2align 2\n\t.globl _conv_run\n_conv_run:\n\tjmp __ZN4Conv3runEs");
__asm__(".text\n\t.p2align 2\n\t.globl _conv_save\n_conv_save:\n\tjmp __ZN4Conv4saveEv");
__asm__(".text\n\t.p2align 2\n\t.globl _conv_set_menu\n_conv_set_menu:\n\tjmp __ZN4Conv8set_menuEsz");
__asm__(".text\n\t.p2align 2\n\t.globl _conv_set_topic\n_conv_set_topic:\n\tjmp __ZN4Conv9set_topicEss");
__asm__(".text\n\t.p2align 2\n\t.globl _conv_topic_on\n_conv_topic_on:\n\tjmp __ZN4Conv8topic_onEs");
__asm__(".text\n\t.p2align 2\n\t.globl _conv_unlock_topic\n_conv_unlock_topic:\n\tjmp __ZN4Conv12unlock_topicEs");
__asm__(".text\n\t.p2align 2\n\t.globl _date_season\n_date_season:\n\tjmp __ZN4Date6seasonEv");
__asm__(".text\n\t.p2align 2\n\t.globl _diary_add\n_diary_add:\n\tjmp __ZN5Diary3addEPcs");
__asm__(".text\n\t.p2align 2\n\t.globl _diary_load_page\n_diary_load_page:\n\tjmp __ZN5Diary9load_pageEt");
__asm__(".text\n\t.p2align 2\n\t.globl _diary_lock\n_diary_lock:\n\tjmp __ZN5Diary8lock_incEv");
__asm__(".text\n\t.p2align 2\n\t.globl _diary_run\n_diary_run:\n\tjmp __ZN5Diary3runEv");
__asm__(".text\n\t.p2align 2\n\t.globl _diary_unlock\n_diary_unlock:\n\tjmp __ZN5Diary8lock_decEv");
__asm__(".text\n\t.p2align 2\n\t.globl _dlg_box\n_dlg_box:\n\tjmp __ZN3Dlg3boxEPvPcS1_sz");
__asm__(".text\n\t.p2align 2\n\t.globl _dlg_box_anim\n_dlg_box_anim:\n\tjmp __ZN3Dlg8box_animEPvPcS1_sz");
__asm__(".text\n\t.p2align 2\n\t.globl _dlg_box_bmp\n_dlg_box_bmp:\n\tjmp __ZN3Dlg7box_bmpEP3BmpPcS2_sz");
__asm__(".text\n\t.p2align 2\n\t.globl _dlg_choose\n_dlg_choose:\n\tjmp __ZN3Dlg6chooseEPcsz");
__asm__(".text\n\t.p2align 2\n\t.globl _dlg_fill_area\n_dlg_fill_area:\n\tjmp __ZN3Dlg9fill_areaEPv");
__asm__(".text\n\t.p2align 2\n\t.globl _dlg_input\n_dlg_input:\n\tjmp __ZN3Dlg5inputEsPcsss");
__asm__(".text\n\t.p2align 2\n\t.globl _dlg_message\n_dlg_message:\n\tjmp __ZN3Dlg7messageEPc");
__asm__(".text\n\t.p2align 2\n\t.globl _dlg_message_flag\n_dlg_message_flag:\n\tjmp __ZN3Dlg10input_textEsPc");
__asm__(".text\n\t.p2align 2\n\t.globl _dlg_overlaps\n_dlg_overlaps:\n\tjmp __ZN3Dlg8overlapsEssss");
__asm__(".text\n\t.p2align 2\n\t.globl _dlg_select_pic\n_dlg_select_pic:\n\tjmp __ZN3Win10select_picEss");
__asm__(".text\n\t.p2align 2\n\t.globl _dlg_set16\n_dlg_set16:\n\tjmp __ZN3Dlg9set_widthEs");
__asm__(".text\n\t.p2align 2\n\t.globl _dlg_set_pos_x\n_dlg_set_pos_x:\n\tjmp __ZN3Dlg9set_pos_xEs");
__asm__(".text\n\t.p2align 2\n\t.globl _dlg_set_pos_y\n_dlg_set_pos_y:\n\tjmp __ZN3Dlg9set_pos_yEs");
__asm__(".text\n\t.p2align 2\n\t.globl _dlg_yes_no\n_dlg_yes_no:\n\tjmp __ZN3Dlg10yes_no_boxEPc");
__asm__(".text\n\t.p2align 2\n\t.globl _dng07_prison_day\n_dng07_prison_day:\n\tjmp __Z16dng07_prison_dayP11area_events");
__asm__(".text\n\t.p2align 2\n\t.globl _draw_loc_icons\n_draw_loc_icons:\n\tjmp __ZN6Screen9set_iconsEsz");
__asm__(".text\n\t.p2align 2\n\t.globl _enemy_attack_damage\n_enemy_attack_damage:\n\tjmp __ZN5Enemy13attack_damageEPvs");
__asm__(".text\n\t.p2align 2\n\t.globl _enemy_attack_value\n_enemy_attack_value:\n\tjmp __ZN5Enemy8at_valueEv");
__asm__(".text\n\t.p2align 2\n\t.globl _enemy_blocks\n_enemy_blocks:\n\tjmp __ZN5Enemy6blocksEv");
__asm__(".text\n\t.p2align 2\n\t.globl _enemy_cast_spell\n_enemy_cast_spell:\n\tjmp __ZN5Enemy10cast_spellEa");
__asm__(".text\n\t.p2align 2\n\t.globl _enemy_damage_range\n_enemy_damage_range:\n\tjmp __ZN5Enemy12damage_rangeEsPsS0_");
__asm__(".text\n\t.p2align 2\n\t.globl _enemy_is_fire\n_enemy_is_fire:\n\tjmp __ZN5Enemy7is_fireEv");
__asm__(".text\n\t.p2align 2\n\t.globl _enemy_is_gfx_17_29\n_enemy_is_gfx_17_29:\n\tjmp __ZN5Enemy12is_gfx_17_29Ev");
__asm__(".text\n\t.p2align 2\n\t.globl _enemy_is_gfx_19_3f\n_enemy_is_gfx_19_3f:\n\tjmp __ZN5Enemy12is_gfx_19_3fEv");
__asm__(".text\n\t.p2align 2\n\t.globl _enemy_is_gfx_1b\n_enemy_is_gfx_1b:\n\tjmp __ZN5Enemy9is_gfx_1bEv");
__asm__(".text\n\t.p2align 2\n\t.globl _enemy_is_gfx_1c\n_enemy_is_gfx_1c:\n\tjmp __ZN5Enemy9is_gfx_1cEv");
__asm__(".text\n\t.p2align 2\n\t.globl _enemy_is_gfx_1f\n_enemy_is_gfx_1f:\n\tjmp __ZN5Enemy9is_gfx_1fEv");
__asm__(".text\n\t.p2align 2\n\t.globl _enemy_is_gfx_24_18\n_enemy_is_gfx_24_18:\n\tjmp __ZN5Enemy12is_gfx_24_18Ev");
__asm__(".text\n\t.p2align 2\n\t.globl _enemy_is_gfx_25_35_36_23\n_enemy_is_gfx_25_35_36_23:\n\tjmp __ZN5Enemy18is_gfx_25_35_36_23Ev");
__asm__(".text\n\t.p2align 2\n\t.globl _enemy_is_gfx_2a\n_enemy_is_gfx_2a:\n\tjmp __ZN5Enemy9is_gfx_2aEv");
__asm__(".text\n\t.p2align 2\n\t.globl _enemy_is_gfx_42\n_enemy_is_gfx_42:\n\tjmp __ZN5Enemy9is_gfx_42Ev");
__asm__(".text\n\t.p2align 2\n\t.globl _enemy_is_humanoid\n_enemy_is_humanoid:\n\tjmp __ZN5Enemy11is_humanoidEv");
__asm__(".text\n\t.p2align 2\n\t.globl _enemy_load_pic\n_enemy_load_pic:\n\tjmp __ZN5Enemy8load_picEPv");
__asm__(".text\n\t.p2align 2\n\t.globl _enemy_name\n_enemy_name:\n\tjmp __ZN5Enemy4nameEv");
__asm__(".text\n\t.p2align 2\n\t.globl _enemy_parade_value\n_enemy_parade_value:\n\tjmp __ZN5Enemy8pa_valueEv");
__asm__(".text\n\t.p2align 2\n\t.globl _enemy_range_damage\n_enemy_range_damage:\n\tjmp __ZN5Enemy12range_damageEP4hero");
__asm__(".text\n\t.p2align 2\n\t.globl _enemy_range_kind\n_enemy_range_kind:\n\tjmp __ZN5Enemy10range_kindEv");
__asm__(".text\n\t.p2align 2\n\t.globl _enemy_set_target_enemy\n_enemy_set_target_enemy:\n\tjmp __ZN5Enemy16set_target_enemyEP5enemy");
__asm__(".text\n\t.p2align 2\n\t.globl _enemy_set_target_hero\n_enemy_set_target_hero:\n\tjmp __ZN5Enemy15set_target_heroEP4hero");
__asm__(".text\n\t.p2align 2\n\t.globl _enemy_sub_ae\n_enemy_sub_ae:\n\tjmp __ZN5Enemy6sub_aeEs");
__asm__(".text\n\t.p2align 2\n\t.globl _enemy_sub_le\n_enemy_sub_le:\n\tjmp __ZN5Enemy6sub_leEss");
__asm__(".text\n\t.p2align 2\n\t.globl _enemy_target_enemy\n_enemy_target_enemy:\n\tjmp __ZN5Enemy12target_enemyEv");
__asm__(".text\n\t.p2align 2\n\t.globl _enemy_target_hero\n_enemy_target_hero:\n\tjmp __ZN5Enemy11target_heroEv");
__asm__(".text\n\t.p2align 2\n\t.globl _enemy_type_check\n_enemy_type_check:\n\tjmp __ZN5Enemy10type_checkEv");
__asm__(".text\n\t.p2align 2\n\t.globl _enter_location\n_enter_location:\n\tjmp __Z14enter_locationv");
__asm__(".text\n\t.p2align 2\n\t.globl _ev_bridge_toll\n_ev_bridge_toll:\n\tjmp __ZN2Ev11bridge_tollEv");
__asm__(".text\n\t.p2align 2\n\t.globl _ev_gavron_chase\n_ev_gavron_chase:\n\tjmp __ZN2Ev12gavron_chaseEv");
__asm__(".text\n\t.p2align 2\n\t.globl _ev_offering_bowl\n_ev_offering_bowl:\n\tjmp __ZN2Ev13offering_bowlEv");
__asm__(".text\n\t.p2align 2\n\t.globl _ev_show_pic_197\n_ev_show_pic_197:\n\tjmp __ZN2Ev12show_pic_197Ev");
__asm__(".text\n\t.p2align 2\n\t.globl _event_dragon_claw\n_event_dragon_claw:\n\tjmp __ZN11area_events11dragon_clawEP4hero");
__asm__(".text\n\t.p2align 2\n\t.globl _event_fight\n_event_fight:\n\tjmp __ZN11area_events5fightEssssssssssss");
__asm__(".text\n\t.p2align 2\n\t.globl _event_fight1\n_event_fight1:\n\tjmp __ZN11area_events6fight1Esss");
__asm__(".text\n\t.p2align 2\n\t.globl _event_fight2\n_event_fight2:\n\tjmp __ZN11area_events6fight2Essssss");
__asm__(".text\n\t.p2align 2\n\t.globl _event_notice\n_event_notice:\n\tjmp __ZN11area_events6noticeEPc");
__asm__(".text\n\t.p2align 2\n\t.globl _event_set_cell\n_event_set_cell:\n\tjmp __ZN11area_events8set_cellEssass");
__asm__(".text\n\t.p2align 2\n\t.globl _event_set_cell_bits\n_event_set_cell_bits:\n\tjmp __ZN11area_events13set_cell_bitsEsss");
__asm__(".text\n\t.p2align 2\n\t.globl _event_teleport\n_event_teleport:\n\tjmp __ZN11area_events8teleportEs");
__asm__(".text\n\t.p2align 2\n\t.globl _fig_list_add\n_fig_list_add:\n\tjmp __ZN7FigList3addEh");
__asm__(".text\n\t.p2align 2\n\t.globl _fig_list_exists\n_fig_list_exists:\n\tjmp __ZN7FigList6existsEh");
__asm__(".text\n\t.p2align 2\n\t.globl _fig_list_find\n_fig_list_find:\n\tjmp __ZN7FigList4findEh");
__asm__(".text\n\t.p2align 2\n\t.globl _fig_list_find_anim\n_fig_list_find_anim:\n\tjmp __ZN7FigList9find_animEPv");
__asm__(".text\n\t.p2align 2\n\t.globl _fig_list_find_twofield\n_fig_list_find_twofield:\n\tjmp __ZN7FigList13find_twofieldEh");
__asm__(".text\n\t.p2align 2\n\t.globl _fig_list_getpos\n_fig_list_getpos:\n\tjmp __ZN7FigList6getposEhPsS0_");
__asm__(".text\n\t.p2align 2\n\t.globl _fig_list_hide\n_fig_list_hide:\n\tjmp __ZN7FigList4hideEh");
__asm__(".text\n\t.p2align 2\n\t.globl _fig_list_purge\n_fig_list_purge:\n\tjmp __ZN7FigList5purgeEv");
__asm__(".text\n\t.p2align 2\n\t.globl _fig_list_remove\n_fig_list_remove:\n\tjmp __ZN7FigList6removeEha");
__asm__(".text\n\t.p2align 2\n\t.globl _fig_list_resort\n_fig_list_resort:\n\tjmp __ZN7FigList6resortEv");
__asm__(".text\n\t.p2align 2\n\t.globl _fig_list_show\n_fig_list_show:\n\tjmp __ZN7FigList4showEh");
__asm__(".text\n\t.p2align 2\n\t.globl _file_close\n_file_close:\n\tjmp __ZN4File5closeEv");
__asm__(".text\n\t.p2align 2\n\t.globl _file_getname\n_file_getname:\n\tjmp __ZN4File7getnameEs");
__asm__(".text\n\t.p2align 2\n\t.globl _file_length\n_file_length:\n\tjmp __ZN4File6lengthEs");
__asm__(".text\n\t.p2align 2\n\t.globl _file_length2\n_file_length2:\n\tjmp __ZN4File6lengthEv");
__asm__(".text\n\t.p2align 2\n\t.globl _file_load\n_file_load:\n\tjmp __ZN4File4loadEsPv");
__asm__(".text\n\t.p2align 2\n\t.globl _file_open\n_file_open:\n\tjmp __ZN4File4openEs");
__asm__(".text\n\t.p2align 2\n\t.globl _file_open_mode\n_file_open_mode:\n\tjmp __ZN4File4openEss");
__asm__(".text\n\t.p2align 2\n\t.globl _file_read\n_file_read:\n\tjmp __ZN4File4readEPvl");
__asm__(".text\n\t.p2align 2\n\t.globl _file_seek\n_file_seek:\n\tjmp __ZN4File4seekEls");
__asm__(".text\n\t.p2align 2\n\t.globl _font_set_line_h\n_font_set_line_h:\n\tjmp __ZN4Font10set_line_hEs");
__asm__(".text\n\t.p2align 2\n\t.globl _group_add_ap\n_group_add_ap:\n\tjmp __ZN5group6add_apEs");
__asm__(".text\n\t.p2align 2\n\t.globl _group_add_ap_each\n_group_add_ap_each:\n\tjmp __ZN5group11add_ap_eachEs");
__asm__(".text\n\t.p2align 2\n\t.globl _group_add_effect\n_group_add_effect:\n\tjmp __ZN5group10add_effectElaaa");
__asm__(".text\n\t.p2align 2\n\t.globl _group_add_hero\n_group_add_hero:\n\tjmp __ZN5group8add_heroEP4heros");
__asm__(".text\n\t.p2align 2\n\t.globl _group_add_le\n_group_add_le:\n\tjmp __ZN5group6add_leEs");
__asm__(".text\n\t.p2align 2\n\t.globl _group_add_money\n_group_add_money:\n\tjmp __ZN5group9add_moneyEl");
__asm__(".text\n\t.p2align 2\n\t.globl _group_all_have_item\n_group_all_have_item:\n\tjmp __ZN5group13all_have_itemEss");
__asm__(".text\n\t.p2align 2\n\t.globl _group_any_in_jail\n_group_any_in_jail:\n\tjmp __ZN5group11any_in_jailEv");
__asm__(".text\n\t.p2align 2\n\t.globl _group_clr_hunger\n_group_clr_hunger:\n\tjmp __ZN5group10clr_hungerEv");
__asm__(".text\n\t.p2align 2\n\t.globl _group_clr_hunger_thirst\n_group_clr_hunger_thirst:\n\tjmp __ZN5group17clr_hunger_thirstEv");
__asm__(".text\n\t.p2align 2\n\t.globl _group_clr_thirst\n_group_clr_thirst:\n\tjmp __ZN5group10clr_thirstEv");
__asm__(".text\n\t.p2align 2\n\t.globl _group_count_alive\n_group_count_alive:\n\tjmp __ZN5group11count_aliveEv");
__asm__(".text\n\t.p2align 2\n\t.globl _group_count_check1\n_group_count_check1:\n\tjmp __ZN5group12count_check1Ev");
__asm__(".text\n\t.p2align 2\n\t.globl _group_count_check2\n_group_count_check2:\n\tjmp __ZN5group12count_check2Ev");
__asm__(".text\n\t.p2align 2\n\t.globl _group_count_check2b\n_group_count_check2b:\n\tjmp __ZN5group13count_check2bEv");
__asm__(".text\n\t.p2align 2\n\t.globl _group_count_check3\n_group_count_check3:\n\tjmp __ZN5group12count_check3Ev");
__asm__(".text\n\t.p2align 2\n\t.globl _group_count_free_knapsack\n_group_count_free_knapsack:\n\tjmp __ZN5group19count_free_knapsackEv");
__asm__(".text\n\t.p2align 2\n\t.globl _group_dec_weapons_bf\n_group_dec_weapons_bf:\n\tjmp __ZN5group14dec_weapons_bfEv");
__asm__(".text\n\t.p2align 2\n\t.globl _group_draw_hero_bar\n_group_draw_hero_bar:\n\tjmp __ZN5group13draw_hero_barEss");
__asm__(".text\n\t.p2align 2\n\t.globl _group_draw_portraits\n_group_draw_portraits:\n\tjmp __ZN5group14draw_portraitsEv");
__asm__(".text\n\t.p2align 2\n\t.globl _group_drop_item\n_group_drop_item:\n\tjmp __ZN5group9drop_itemEs");
__asm__(".text\n\t.p2align 2\n\t.globl _group_drop_most_items\n_group_drop_most_items:\n\tjmp __ZN5group15drop_most_itemsEv");
__asm__(".text\n\t.p2align 2\n\t.globl _group_find_drink\n_group_find_drink:\n\tjmp __ZN5group10find_drinkEP4heroPsS2_S2_");
__asm__(".text\n\t.p2align 2\n\t.globl _group_find_food\n_group_find_food:\n\tjmp __ZN5group9find_foodEP4heroPsS2_S2_");
__asm__(".text\n\t.p2align 2\n\t.globl _group_free_slot\n_group_free_slot:\n\tjmp __ZN5group9free_slotEv");
__asm__(".text\n\t.p2align 2\n\t.globl _group_get_ill\n_group_get_ill:\n\tjmp __ZN5group7get_illEss");
__asm__(".text\n\t.p2align 2\n\t.globl _group_get_on_route\n_group_get_on_route:\n\tjmp __ZN5group12get_on_routeEv");
__asm__(".text\n\t.p2align 2\n\t.globl _group_give_item\n_group_give_item:\n\tjmp __ZN5group9give_itemEss");
__asm__(".text\n\t.p2align 2\n\t.globl _group_give_item_each\n_group_give_item_each:\n\tjmp __ZN5group14give_item_eachEss");
__asm__(".text\n\t.p2align 2\n\t.globl _group_has_dwarf\n_group_has_dwarf:\n\tjmp __ZN5group9has_dwarfEv");
__asm__(".text\n\t.p2align 2\n\t.globl _group_has_elf\n_group_has_elf:\n\tjmp __ZN5group7has_elfEv");
__asm__(".text\n\t.p2align 2\n\t.globl _group_has_hero\n_group_has_hero:\n\tjmp __ZN5group8has_heroEP4hero");
__asm__(".text\n\t.p2align 2\n\t.globl _group_heal_full\n_group_heal_full:\n\tjmp __ZN5group9heal_fullEv");
__asm__(".text\n\t.p2align 2\n\t.globl _group_hero_pos\n_group_hero_pos:\n\tjmp __ZN5group8hero_posEP4hero");
__asm__(".text\n\t.p2align 2\n\t.globl _group_hero_with_item\n_group_hero_with_item:\n\tjmp __ZN5group14hero_with_itemEss");
__asm__(".text\n\t.p2align 2\n\t.globl _group_inc_skill\n_group_inc_skill:\n\tjmp __ZN5group9inc_skillEs");
__asm__(".text\n\t.p2align 2\n\t.globl _group_infect\n_group_infect:\n\tjmp __ZN5group6infectEssP4hero");
__asm__(".text\n\t.p2align 2\n\t.globl _group_load\n_group_load:\n\tjmp __ZN5group4loadEv");
__asm__(".text\n\t.p2align 2\n\t.globl _group_mark_can_use\n_group_mark_can_use:\n\tjmp __ZN5group12mark_can_useEs");
__asm__(".text\n\t.p2align 2\n\t.globl _group_mark_weapon\n_group_mark_weapon:\n\tjmp __ZN5group11mark_weaponEPcs");
__asm__(".text\n\t.p2align 2\n\t.globl _group_refill_water\n_group_refill_water:\n\tjmp __ZN5group12refill_waterEv");
__asm__(".text\n\t.p2align 2\n\t.globl _group_remove_hero\n_group_remove_hero:\n\tjmp __ZN5group11remove_heroEs");
__asm__(".text\n\t.p2align 2\n\t.globl _group_remove_item\n_group_remove_item:\n\tjmp __ZN5group11remove_itemEs");
__asm__(".text\n\t.p2align 2\n\t.globl _group_remove_npc\n_group_remove_npc:\n\tjmp __ZN5group10remove_npcEP4hero");
__asm__(".text\n\t.p2align 2\n\t.globl _group_reset_rest\n_group_reset_rest:\n\tjmp __ZN5group10reset_restEv");
__asm__(".text\n\t.p2align 2\n\t.globl _group_same_place\n_group_same_place:\n\tjmp __ZN5group10same_placeEPS_");
__asm__(".text\n\t.p2align 2\n\t.globl _group_save\n_group_save:\n\tjmp __ZN5group4saveEv");
__asm__(".text\n\t.p2align 2\n\t.globl _group_set_on_route\n_group_set_on_route:\n\tjmp __ZN5group12set_on_routeEs");
__asm__(".text\n\t.p2align 2\n\t.globl _group_set_slot\n_group_set_slot:\n\tjmp __ZN5group12dismiss_heroEss");
__asm__(".text\n\t.p2align 2\n\t.globl _group_sleep\n_group_sleep:\n\tjmp __ZN5group5sleepEs");
__asm__(".text\n\t.p2align 2\n\t.globl _group_sub_hunger_thirst\n_group_sub_hunger_thirst:\n\tjmp __ZN5group17sub_hunger_thirstEss");
__asm__(".text\n\t.p2align 2\n\t.globl _group_sub_hunger_thirst_max\n_group_sub_hunger_thirst_max:\n\tjmp __ZN5group21sub_hunger_thirst_maxEss");
__asm__(".text\n\t.p2align 2\n\t.globl _group_sub_le\n_group_sub_le:\n\tjmp __ZN5group6sub_leEs");
__asm__(".text\n\t.p2align 2\n\t.globl _group_sub_le_dice\n_group_sub_le_dice:\n\tjmp __ZN5group11sub_le_diceEaas");
__asm__(".text\n\t.p2align 2\n\t.globl _group_sub_le_keep\n_group_sub_le_keep:\n\tjmp __ZN5group11sub_le_keepEss");
__asm__(".text\n\t.p2align 2\n\t.globl _group_sub_le_range\n_group_sub_le_range:\n\tjmp __ZN5group12sub_le_rangeEss");
__asm__(".text\n\t.p2align 2\n\t.globl _group_sub_money\n_group_sub_money:\n\tjmp __ZN5group9sub_moneyEl");
__asm__(".text\n\t.p2align 2\n\t.globl _group_sum_ap\n_group_sum_ap:\n\tjmp __ZN5group6sum_apEv");
__asm__(".text\n\t.p2align 2\n\t.globl _inn_apply_magic\n_inn_apply_magic:\n\tjmp __ZN3Inn11apply_magicEv");
__asm__(".text\n\t.p2align 2\n\t.globl _inn_apply_skill\n_inn_apply_skill:\n\tjmp __ZN3Inn11apply_skillEv");
__asm__(".text\n\t.p2align 2\n\t.globl _inn_eat\n_inn_eat:\n\tjmp __ZN3Inn3eatEv");
__asm__(".text\n\t.p2align 2\n\t.globl _inn_room\n_inn_room:\n\tjmp __ZN3Inn4roomEv");
__asm__(".text\n\t.p2align 2\n\t.globl _inn_talk\n_inn_talk:\n\tjmp __ZN3Inn4talkEv");
__asm__(".text\n\t.p2align 2\n\t.globl _inn_theft_morning\n_inn_theft_morning:\n\tjmp __ZN3Inn13theft_morningEv");
__asm__(".text\n\t.p2align 2\n\t.globl _inn_theft_night\n_inn_theft_night:\n\tjmp __ZN3Inn11theft_nightEv");
__asm__(".text\n\t.p2align 2\n\t.globl _inv_confirm\n_inv_confirm:\n\tjmp __ZN3Inv7confirmEv");
__asm__(".text\n\t.p2align 2\n\t.globl _inv_draw\n_inv_draw:\n\tjmp __ZN3Inv4drawEv");
__asm__(".text\n\t.p2align 2\n\t.globl _inv_draw_slot\n_inv_draw_slot:\n\tjmp __ZN6InvObj9draw_slotEPvss");
__asm__(".text\n\t.p2align 2\n\t.globl _inv_next_hero\n_inv_next_hero:\n\tjmp __ZN3Inv9next_heroEv");
__asm__(".text\n\t.p2align 2\n\t.globl _inv_reselect\n_inv_reselect:\n\tjmp __ZN6InvObj8reselectEv");
__asm__(".text\n\t.p2align 2\n\t.globl _inv_reset_cursor\n_inv_reset_cursor:\n\tjmp __ZN6InvObj12reset_cursorEv");
__asm__(".text\n\t.p2align 2\n\t.globl _inv_sel_next\n_inv_sel_next:\n\tjmp __ZN6InvObj8sel_nextEv");
__asm__(".text\n\t.p2align 2\n\t.globl _inv_sel_prev\n_inv_sel_prev:\n\tjmp __ZN6InvObj8sel_prevEv");
__asm__(".text\n\t.p2align 2\n\t.globl _inv_set_cursor\n_inv_set_cursor:\n\tjmp __ZN6InvObj10set_cursorEs");
__asm__(".text\n\t.p2align 2\n\t.globl _inv_toggle_body\n_inv_toggle_body:\n\tjmp __ZN3Inv11toggle_bodyEv");
__asm__(".text\n\t.p2align 2\n\t.globl _inv_warn_signal\n_inv_warn_signal:\n\tjmp __ZN3Inv11warn_signalEv");
__asm__(".text\n\t.p2align 2\n\t.globl _item_consume\n_item_consume:\n\tjmp __ZN7ItemUse7consumeEv");
__asm__(".text\n\t.p2align 2\n\t.globl _items_change_price\n_items_change_price:\n\tjmp __ZN5Items12change_priceEss");
__asm__(".text\n\t.p2align 2\n\t.globl _keylist_add\n_keylist_add:\n\tjmp __ZN7Keylist3addEst");
__asm__(".text\n\t.p2align 2\n\t.globl _keylist_load\n_keylist_load:\n\tjmp __ZN7Keylist4loadEPv");
__asm__(".text\n\t.p2align 2\n\t.globl _load_pic_list\n_load_pic_list:\n\tjmp __ZN5Cache9load_listEPsPtss");
__asm__(".text\n\t.p2align 2\n\t.globl _nvfile_count\n_nvfile_count:\n\tjmp __ZN3Nvf5countEs");
__asm__(".text\n\t.p2align 2\n\t.globl _nvfile_dims\n_nvfile_dims:\n\tjmp __ZN3Nvf4dimsEsPsS0_");
__asm__(".text\n\t.p2align 2\n\t.globl _nvfile_info\n_nvfile_info:\n\tjmp __ZN3Nvf4infoEssPsS0_");
__asm__(".text\n\t.p2align 2\n\t.globl _nvfile_load\n_nvfile_load:\n\tjmp __ZN3Nvf4loadEsPvS0_saPsS1_s");
__asm__(".text\n\t.p2align 2\n\t.globl _nvfile_pal\n_nvfile_pal:\n\tjmp __ZN3Nvf3palEsPvt");
__asm__(".text\n\t.p2align 2\n\t.globl _ofile_close\n_ofile_close:\n\tjmp __ZN8TempFile5closeEv");
__asm__(".text\n\t.p2align 2\n\t.globl _ofile_create\n_ofile_create:\n\tjmp __ZN8TempFile4openEss");
__asm__(".text\n\t.p2align 2\n\t.globl _ofile_length\n_ofile_length:\n\tjmp __ZN8TempFile6lengthEv");
__asm__(".text\n\t.p2align 2\n\t.globl _ofile_length_id\n_ofile_length_id:\n\tjmp __ZN8TempFile6lengthEs");
__asm__(".text\n\t.p2align 2\n\t.globl _ofile_load\n_ofile_load:\n\tjmp __ZN8TempFile4loadEsPv");
__asm__(".text\n\t.p2align 2\n\t.globl _ofile_open\n_ofile_open:\n\tjmp __ZN8TempFile4openEs");
__asm__(".text\n\t.p2align 2\n\t.globl _ofile_open_mode\n_ofile_open_mode:\n\tjmp __ZN8TempFile4openEsst");
__asm__(".text\n\t.p2align 2\n\t.globl _ofile_read\n_ofile_read:\n\tjmp __ZN8TempFile4readEPvl");
__asm__(".text\n\t.p2align 2\n\t.globl _ofile_seek\n_ofile_seek:\n\tjmp __ZN8TempFile4seekEls");
__asm__(".text\n\t.p2align 2\n\t.globl _ofile_write\n_ofile_write:\n\tjmp __ZN8TempFile5writeEPvl");
__asm__(".text\n\t.p2align 2\n\t.globl _opening_fade_in\n_opening_fade_in:\n\tjmp __ZN7Opening7fade_inEPh");
__asm__(".text\n\t.p2align 2\n\t.globl _opening_scene3\n_opening_scene3:\n\tjmp __ZN7Opening6scene3Ev");
__asm__(".text\n\t.p2align 2\n\t.globl _saver_anim\n_saver_anim:\n\tjmp __ZN5saver4animEv");
__asm__(".text\n\t.p2align 2\n\t.globl _saver_tick\n_saver_tick:\n\tjmp __ZN5saver4tickEv");
__asm__(".text\n\t.p2align 2\n\t.globl _screen_clear_textbar\n_screen_clear_textbar:\n\tjmp __ZN6Screen13clear_textbarEv");
__asm__(".text\n\t.p2align 2\n\t.globl _screen_clear_view\n_screen_clear_view:\n\tjmp __ZN6Screen10clear_viewEv");
__asm__(".text\n\t.p2align 2\n\t.globl _screen_draw_icons\n_screen_draw_icons:\n\tjmp __ZN6Screen10draw_iconsEv");
__asm__(".text\n\t.p2align 2\n\t.globl _screen_draw_name\n_screen_draw_name:\n\tjmp __ZN6Screen9draw_nameEss");
__asm__(".text\n\t.p2align 2\n\t.globl _screen_draw_name_normal\n_screen_draw_name_normal:\n\tjmp __ZN6Screen16draw_name_normalEs");
__asm__(".text\n\t.p2align 2\n\t.globl _screen_draw_names\n_screen_draw_names:\n\tjmp __ZN6Screen10draw_namesEv");
__asm__(".text\n\t.p2align 2\n\t.globl _screen_mark_portrait\n_screen_mark_portrait:\n\tjmp __ZN6Screen13mark_portraitEss");
__asm__(".text\n\t.p2align 2\n\t.globl _screen_portraits\n_screen_portraits:\n\tjmp __ZN6Screen14draw_portraitsEv");
__asm__(".text\n\t.p2align 2\n\t.globl _screen_redraw\n_screen_redraw:\n\tjmp __ZN6Screen6redrawEv");
__asm__(".text\n\t.p2align 2\n\t.globl _screen_reset_icons\n_screen_reset_icons:\n\tjmp __ZN6Screen11reset_iconsEv");
__asm__(".text\n\t.p2align 2\n\t.globl _screen_short_name\n_screen_short_name:\n\tjmp __ZN6Screen10short_nameEPcS0_");
__asm__(".text\n\t.p2align 2\n\t.globl _screen_textbar\n_screen_textbar:\n\tjmp __ZN6Screen7textbarEPcs");
__asm__(".text\n\t.p2align 2\n\t.globl _shop_build_wares\n_shop_build_wares:\n\tjmp __ZN4Shop11build_waresEv");
__asm__(".text\n\t.p2align 2\n\t.globl _shop_format_money\n_shop_format_money:\n\tjmp __ZN4Shop12format_moneyEPcl");
__asm__(".text\n\t.p2align 2\n\t.globl _shop_leave\n_shop_leave:\n\tjmp __ZN4Shop5leaveEv");
__asm__(".text\n\t.p2align 2\n\t.globl _shop_trade\n_shop_trade:\n\tjmp __ZN4Shop5tradeEla");
__asm__(".text\n\t.p2align 2\n\t.globl _snd_obj_stop\n_snd_obj_stop:\n\tjmp __ZN5Sound4stopEv");
__asm__(".text\n\t.p2align 2\n\t.globl _speech_active\n_speech_active:\n\tjmp __ZN6Speech6activeEv");
__asm__(".text\n\t.p2align 2\n\t.globl _stash_add\n_stash_add:\n\tjmp __ZN5Stash3addEssh");
__asm__(".text\n\t.p2align 2\n\t.globl _stash_has_items\n_stash_has_items:\n\tjmp __ZN5Stash9has_itemsEv");
__asm__(".text\n\t.p2align 2\n\t.globl _stash_is\n_stash_is:\n\tjmp __ZN5Stash2isEssh");
__asm__(".text\n\t.p2align 2\n\t.globl _stash_set\n_stash_set:\n\tjmp __ZN5Stash3setEssh");
__asm__(".text\n\t.p2align 2\n\t.globl _stash_show\n_stash_show:\n\tjmp __ZN5Stash4showEv");
__asm__(".text\n\t.p2align 2\n\t.globl _tavern_add_bill\n_tavern_add_bill:\n\tjmp __ZN6Tavern8add_billEs");
__asm__(".text\n\t.p2align 2\n\t.globl _tavern_check_closing\n_tavern_check_closing:\n\tjmp __ZN6Tavern13check_closingEl");
__asm__(".text\n\t.p2align 2\n\t.globl _tavern_drink_test\n_tavern_drink_test:\n\tjmp __ZN6Tavern10drink_testEs");
__asm__(".text\n\t.p2align 2\n\t.globl _tavern_eat\n_tavern_eat:\n\tjmp __ZN6Tavern3eatEv");
__asm__(".text\n\t.p2align 2\n\t.globl _tavern_order\n_tavern_order:\n\tjmp __ZN6Tavern5orderEv");
__asm__(".text\n\t.p2align 2\n\t.globl _tavern_set_crowd\n_tavern_set_crowd:\n\tjmp __ZN6Tavern9set_crowdEl");
__asm__(".text\n\t.p2align 2\n\t.globl _tmap_draw\n_tmap_draw:\n\tjmp __ZN4TMap4drawEv");
__asm__(".text\n\t.p2align 2\n\t.globl _tmap_follow\n_tmap_follow:\n\tjmp __ZN4TMap6followEssah");
__asm__(".text\n\t.p2align 2\n\t.globl _tmap_free_pal\n_tmap_free_pal:\n\tjmp __ZN4TMap8free_palEv");
__asm__(".text\n\t.p2align 2\n\t.globl _tmap_load\n_tmap_load:\n\tjmp __ZN4TMap4loadEs");
__asm__(".text\n\t.p2align 2\n\t.globl _tmap_mark1_anim\n_tmap_mark1_anim:\n\tjmp __ZN4TMap10mark1_animEss");
__asm__(".text\n\t.p2align 2\n\t.globl _tmap_mark1_free\n_tmap_mark1_free:\n\tjmp __ZN4TMap10mark1_freeEv");
__asm__(".text\n\t.p2align 2\n\t.globl _tmap_mark2_anim\n_tmap_mark2_anim:\n\tjmp __ZN4TMap10mark2_animEss");
__asm__(".text\n\t.p2align 2\n\t.globl _tmap_mark2_free\n_tmap_mark2_free:\n\tjmp __ZN4TMap10mark2_freeEv");
__asm__(".text\n\t.p2align 2\n\t.globl _tmap_new_pal\n_tmap_new_pal:\n\tjmp __ZN4TMap7new_palEv");
__asm__(".text\n\t.p2align 2\n\t.globl _tmap_plot_points\n_tmap_plot_points:\n\tjmp __ZN4TMap11plot_pointsEPsh");
__asm__(".text\n\t.p2align 2\n\t.globl _tmap_screen_x\n_tmap_screen_x:\n\tjmp __ZN4TMap8screen_xEs");
__asm__(".text\n\t.p2align 2\n\t.globl _tmap_screen_y\n_tmap_screen_y:\n\tjmp __ZN4TMap8screen_yEs");
__asm__(".text\n\t.p2align 2\n\t.globl _tmap_set_pal\n_tmap_set_pal:\n\tjmp __ZN4TMap7set_palEv");
__asm__(".text\n\t.p2align 2\n\t.globl _tmap_set_pos\n_tmap_set_pos:\n\tjmp __ZN4TMap7set_posEss");
__asm__(".text\n\t.p2align 2\n\t.globl _town_enter\n_town_enter:\n\tjmp __ZN4Town5enterEsP9map_event");
__asm__(".text\n\t.p2align 2\n\t.globl _town_harbor_event\n_town_harbor_event:\n\tjmp __ZN4Town12harbor_eventEP9map_event");
__asm__(".text\n\t.p2align 2\n\t.globl _town_sites_event\n_town_sites_event:\n\tjmp __ZN4Town11sites_eventEP9map_event");
__asm__(".text\n\t.p2align 2\n\t.globl _town_swim_check\n_town_swim_check:\n\tjmp __ZN4Town10swim_checkEv");
__asm__(".text\n\t.p2align 2\n\t.globl _txt_build_index\n_txt_build_index:\n\tjmp __Z15txt_build_indexPPcS_l");
__asm__(".text\n\t.p2align 2\n\t.globl _txt_load_list\n_txt_load_list:\n\tjmp __ZN4Font9load_listEssPPvPPPc");
__asm__(".text\n\t.p2align 2\n\t.globl _txt_max_width\n_txt_max_width:\n\tjmp __ZN4Font9max_widthEPc");
__asm__(".text\n\t.p2align 2\n\t.globl _txt_split\n_txt_split:\n\tjmp __ZN4Font5splitEPPcS0_l");

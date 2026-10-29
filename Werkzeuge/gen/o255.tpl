void far f_0000(void)
{
	signed char l01, l02, l03, l04, l05, l06, l07, l08, l09, l0a, l0b, l0c, l0d, l0e, l0f, l10;
	signed char l11, l12, l13, l14, l15, l16, l17, l18, l19, l1a, l1b, l1c, l1d, l1e, l1f, l20, l21;
	register signed char rd, rb, rc;

	l01 = l02 = l03 = l04 = l05 = l06 = l07 = l08 = l09 = l0a = l0b = l0c = l0d = l0e = rd = rb = l0f = l10 = rc =
		l11 = l12 = l13 = l14 = l15 = l16 = l17 = l18 = l19 = l1a = l1b = l1c = l1d = l1e = l1f = l20 = l21 = 0;

	if (g_b515) { Z(b4a6 b4b0 b4ba) l01 = 1; }
	else if (g_b50b) { Z(b4a6 b4b0) }
	else if (g_b501) g_b4a6 = 0;

	if (g_b51e) { Z(b4ca b4cb b4cc) l09 = l02 = 1; }
	else if (g_b51d) { Z(b4ca b4cb) l02 = 1; }
	else if (g_b51c) g_b4ca = 0;

	if (g_b527) { Z(b4ee b4e6 b4de) l03 = 1; }
	else if (g_b52f) { Z(b4ee b4e6) }
	else if (g_b537) g_b4ee = 0;

	if (g_b528) { Z(b4f2 b4e9 b4e0) l0a = l04 = 1; }
	else if (g_b531) { Z(b4f2 b4e9) l04 = 1; }
	else if (g_b53a) g_b4f2 = 0;

	if (g_b529) { Z(b4f6 b4ec b4e2) l05 = 1; }
	else if (g_b533) { Z(b4f6 b4ec) }
	else if (g_b53d) g_b4f6 = 0;

	if (g_b520) { Z(b4d2 b4d1 b4d0) l0b = l06 = 1; }
	else if (g_b521) { Z(b4d2 b4d1) l06 = 1; }
	else if (g_b522) g_b4d2 = 0;

	if (g_b517) { Z(b4ae b4b6 b4be) l07 = 1; }
	else if (g_b50f) { Z(b4ae b4b6) }
	else if (g_b507) g_b4ae = 0;

	if (g_b516) { Z(b4aa b4b3 b4bc) l0c = l08 = 1; }
	else if (g_b50d) { Z(b4aa b4b3) l08 = 1; }
	else if (g_b504) g_b4aa = 0;

	if (l09 && l0a) {
		Z(b4e5 b4dc b4d3 b4dd b4d4 b4d5) l0e = 1;
		Z(b4ef b4f0 b4f1 b4e7 b4e8 b4df) rd = 1;
		Z(b4ee b4e6 b4de b4d6) l03 = 1;
	} else {
		if (l02 && l03) { Z(b4e5 b4dc b4d3 b4dd b4d4 b4d5) l0e = 1; }
		if (l03 && l04) { Z(b4ef b4f0 b4f1 b4e7 b4e8 b4df) rd = 1; }
	}
	if (l0a && l0b) {
		Z(b4f5 b4f4 b4f3 b4eb b4ea b4e1) rb = 1;
		Z(b4ed b4e4 b4db b4e3 b4da b4d9) l0f = 1;
		Z(b4f6 b4ec b4e2 b4d8) l05 = 1;
	} else {
		if (l04 && l05) { Z(b4f5 b4f4 b4f3 b4eb b4ea b4e1) rb = 1; }
		if (l05 && l06) { Z(b4ed b4e4 b4db b4e3 b4da b4d9) l0f = 1; }
	}
	if (l0b && l0c) {
		Z(b4b7 b4c0 b4c9 b4bf b4c8 b4c7) l10 = 1;
		Z(b4ad b4ac b4ab b4b5 b4b4 b4bd) rc = 1;
		Z(b4ae b4b6 b4be b4c6) l07 = 1;
	} else {
		if (l06 && l07) { Z(b4b7 b4c0 b4c9 b4bf b4c8 b4c7) l10 = 1; }
		if (l07 && l08) { Z(b4ad b4ac b4ab b4b5 b4b4 b4bd) rc = 1; }
	}
	if (l0c && l09) {
		Z(b4a7 b4a8 b4a9 b4b1 b4b2 b4bb) l11 = 1;
		Z(b4af b4b8 b4c1 b4b9 b4c2 b4c3) l0d = 1;
		Z(b4a6 b4b0 b4ba b4c4) l01 = 1;
	} else {
		if (l01 && l02) { Z(b4af b4b8 b4c1 b4b9 b4c2 b4c3) l0d = 1; }
		if (l08 && l01) { Z(b4a7 b4a8 b4a9 b4b1 b4b2 b4bb) l11 = 1; }
	}

	if (!l0d) {
		if ((g_b50b || g_b515) && (g_b513 || g_b514)) { Z(b4af b4b9 b4b8) l12 = 1; }
		if (g_b514 && g_b50c) {
			Z(b4af b4b9 b4b8 b4ba b4b0 b4a6) l12 = 1;
			if (!l11) { Z(b4a7 b4b1 b4a8) l21 = 1; }
		} else {
			if ((g_b514 || g_b513) && g_b515) g_b4b9 = 0;
			if (!l11 && (g_b503 || g_b50c) && g_b515) g_b4b1 = 0;
			if (g_b50a && g_b502) {
				Z(b4af b4b0 b4a6)
				if (!l11) g_b4a7 = 0;
			} else {
				if (g_b500 && g_b4f8) g_b4a6 = 0;
				if ((g_b501 || g_b50b || g_b515) && (g_b509 || g_b50a || g_b514) || g_b50b && g_b51e) g_b4af = 0;
				if (!l11 && ((g_b501 || g_b50b || g_b515) && (g_b4f9 || g_b502 || g_b50c) || g_b50b && g_b516)) g_b4a7 = 0;
			}
		}
		if (g_b514 && (g_b51e || g_b51d || g_b51c)) { Z(b4b8 b4c1 b4c2) l13 = 1; }
		else if (g_b513 && (g_b51e || g_b51d || g_b51c || g_b51b)) g_b4c1 = 0;
	}

	if (!l0e) {
		if ((g_b527 || g_b52f) && (g_b525 || g_b526)) { Z(b4e5 b4dd b4dc) l15 = 1; }
		if (g_b526 && g_b530) {
			Z(b4e5 b4dd b4dc b4de b4e6 b4ee) l15 = 1;
			if (!rd) { Z(b4ef b4e7 b4f0) l16 = 1; }
		} else {
			if ((g_b526 || g_b525) && g_b527) g_b4dd = 0;
			if (!rd && (g_b539 || g_b530) && g_b527) g_b4e7 = 0;
			if (g_b52e && g_b538) {
				Z(b4e5 b4e6 b4ee)
				if (!rd) g_b4ef = 0;
			} else {
				if (g_b536 && g_b540) g_b4ee = 0;
				if ((g_b537 || g_b52f || g_b527) && (g_b52d || g_b52e || g_b526) || g_b52f && g_b51e) g_b4e5 = 0;
				if (!rd && ((g_b537 || g_b52f || g_b527) && (g_b541 || g_b538 || g_b530) || g_b52f && g_b528)) g_b4ef = 0;
			}
		}
		if (g_b526 && (g_b51e || g_b51d || g_b51c)) { Z(b4dc b4d4 b4d3) l14 = 1; }
		else if (g_b525 && (g_b51e || g_b51d || g_b51c || g_b51b)) g_b4d3 = 0;
	}

	if (!l12 && !l13) {
		if (g_b514) g_b4b8 = 0;
		else if (g_b515 && g_b513) g_b4b8 = 0;
		else if (g_b50a && (g_b513 || g_b512)) g_b4b8 = 0;
	}
	if (!l14 && !l15) {
		if (g_b526) g_b4dc = 0;
		else if (g_b527 && g_b525) g_b4dc = 0;
		else if (g_b52e && (g_b525 || g_b524)) g_b4dc = 0;
	}

	if (!l0f) {
		if ((g_b533 || g_b529) && (g_b52b || g_b52a)) { Z(b4ed b4e3 b4e4) l1a = 1; }
		if (g_b52a && g_b532) {
			Z(b4ed b4e3 b4e4 b4e2 b4ec b4f6) l1a = 1;
			if (!rb) { Z(b4f5 b4eb b4f4) l19 = 1; }
		} else {
			if ((g_b52a || g_b52b) && g_b529) g_b4e3 = 0;
			if (!rb && (g_b53b || g_b532) && g_b529) g_b4eb = 0;
			if (g_b534 && g_b53c) {
				Z(b4ed b4ec b4f6)
				if (!rb) g_b4f5 = 0;
			} else {
				if (g_b53e && g_b546) g_b4f6 = 0;
				if ((g_b53d || g_b533 || g_b529) && (g_b535 || g_b534 || g_b52a) || g_b533 && g_b520) g_b4ed = 0;
				if (!rb && ((g_b53d || g_b533 || g_b529) && (g_b545 || g_b53c || g_b532) || g_b533 && g_b528)) g_b4f5 = 0;
			}
		}
		if (g_b52a && (g_b520 || g_b521 || g_b522)) { Z(b4e4 b4db b4da) l1b = 1; }
		else if (g_b52b && (g_b520 || g_b521 || g_b522 || g_b523)) g_b4db = 0;
	}

	if (!l10) {
		if ((g_b50f || g_b517) && (g_b519 || g_b518)) { Z(b4b7 b4bf b4c0) l1d = 1; }
		if (g_b518 && g_b50e) {
			Z(b4b7 b4bf b4c0 b4be b4b6 b4ae) l1d = 1;
			if (!rc) { Z(b4ad b4b5 b4ac) l1e = 1; }
		} else {
			if ((g_b518 || g_b519) && g_b517) g_b4bf = 0;
			if (!rc && (g_b505 || g_b50e) && g_b517) g_b4b5 = 0;
			if (g_b510 && g_b506) {
				Z(b4b7 b4b6 b4ae)
				if (!rc) g_b4ad = 0;
			} else {
				if (g_b508 && g_b4fe) g_b4ae = 0;
				if ((g_b507 || g_b50f || g_b517) && (g_b511 || g_b510 || g_b518) || g_b50f && g_b520) g_b4b7 = 0;
				if (!rc && ((g_b507 || g_b50f || g_b517) && (g_b4fd || g_b506 || g_b50e) || g_b50f && g_b516)) g_b4ad = 0;
			}
		}
		if (g_b518 && (g_b520 || g_b521 || g_b522)) { Z(b4c0 b4c9 b4c8) l1c = 1; }
		else if (g_b519 && (g_b520 || g_b521 || g_b522 || g_b523)) g_b4c9 = 0;
	}

	if (!l1a && !l1b) {
		if (g_b52a) g_b4e4 = 0;
		else if (g_b529 && g_b52b) g_b4e4 = 0;
		else if (g_b534 && (g_b52b || g_b52c)) g_b4e4 = 0;
	}
	if (!l1c && !l1d) {
		if (g_b518) g_b4c0 = 0;
		else if (g_b517 && g_b519) g_b4c0 = 0;
		else if (g_b510 && (g_b519 || g_b51a)) g_b4c0 = 0;
	}

	if (!l11) {
		if ((g_b515 || g_b50b) && (g_b503 || g_b50c)) { Z(b4a7 b4b1 b4a8) l21 = 1; }
		if (g_b50c && (g_b516 || g_b50d || g_b504)) { Z(b4a8 b4a9 b4b2) l20 = 1; }
		else if (g_b503 && (g_b516 || g_b50d || g_b504 || g_b4fb)) g_b4a9 = 0;
	}
	if (!rc) {
		if (g_b50e && (g_b516 || g_b50d || g_b504)) { Z(b4ac b4b4 b4ab) l1f = 1; }
		else if (g_b505 && (g_b516 || g_b50d || g_b504 || g_b4fb)) g_b4ab = 0;
		if ((g_b517 || g_b50f) && (g_b505 || g_b50e)) { Z(b4ad b4b5 b4ac) l1e = 1; }
		else {
			if ((g_b507 || g_b50f || g_b517) && (g_b4fd || g_b506)) g_b4ad = 0;
			if ((g_b505 || g_b50e) && g_b517) g_b4b5 = 0;
		}
	}

	if (!l20 && !l21) {
		if (g_b50c) g_b4a8 = 0;
		else if (g_b515 && g_b503) g_b4a8 = 0;
		else if (g_b502 && (g_b503 || g_b4fa)) g_b4a8 = 0;
	}
	if (!l1e && !l1f) {
		if (g_b50e) g_b4ac = 0;
		else if (g_b517 && g_b505) g_b4ac = 0;
		else if (g_b506 && (g_b505 || g_b4fc)) g_b4ac = 0;
	}

	if (!rd) {
		if ((g_b527 || g_b52f) && (g_b539 || g_b530)) { Z(b4e7 b4ef b4f0) l16 = 1; }
		if (g_b530 && (g_b528 || g_b531 || g_b53a)) { Z(b4f0 b4f1 b4e8) l17 = 1; }
		else if (g_b539 && (g_b528 || g_b531 || g_b53a || g_b543)) g_b4f1 = 0;
	}
	if (!rb) {
		if (g_b532 && (g_b528 || g_b531 || g_b53a)) { Z(b4f4 b4ea b4f3) l18 = 1; }
		else if (g_b53b && (g_b528 || g_b531 || g_b53a || g_b543)) g_b4f3 = 0;
		if ((g_b529 || g_b533) && (g_b53b || g_b532)) { Z(b4f5 b4eb b4f4) l19 = 1; }
	}

	if (!l16 && !l17) {
		if (g_b530) g_b4f0 = 0;
		else if (g_b527 && g_b539) g_b4f0 = 0;
		else if (g_b538 && (g_b539 || g_b542)) g_b4f0 = 0;
	}
	if (!l18 && !l19) {
		if (g_b532) g_b4f4 = 0;
		else if (g_b529 && g_b53b) g_b4f4 = 0;
		else if (g_b53c && (g_b53b || g_b544)) g_b4f4 = 0;
	}
}

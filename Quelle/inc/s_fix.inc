; s_fix.inc - Fehlerbehebungen per Schalter fuer TASM (erzeugt von tools/mkfix.py, siehe names/inc/s_fix.h)
ifndef FIX_BUGS
FIX_BUGS = 0
endif
ifndef FIX_EXPLOITS
if FIX_BUGS gt 0
FIX_EXPLOITS = 1
else
FIX_EXPLOITS = 0
endif
endif
ifndef FIX_0300
if FIX_BUGS ge 2
FIX_0300 = 1
else
FIX_0300 = 0
endif
endif
ifndef FIX_0301
if FIX_BUGS ge 2
FIX_0301 = 1
else
FIX_0301 = 0
endif
endif
ifndef FIX_0302
if FIX_BUGS ge 2
FIX_0302 = 1
else
FIX_0302 = 0
endif
endif
ifndef FIX_0304
if FIX_BUGS ge 2
FIX_0304 = 1
else
FIX_0304 = 0
endif
endif
ifndef FIX_0305
if FIX_BUGS ge 2
FIX_0305 = 1
else
FIX_0305 = 0
endif
endif
ifndef FIX_0306
if FIX_BUGS ge 2
FIX_0306 = 1
else
FIX_0306 = 0
endif
endif
ifndef FIX_0307
if FIX_BUGS ge 2
FIX_0307 = 1
else
FIX_0307 = 0
endif
endif
ifndef FIX_0350
if FIX_BUGS ge 2
FIX_0350 = 1
else
FIX_0350 = 0
endif
endif
ifndef FIX_0351
if FIX_BUGS ge 2
FIX_0351 = 1
else
FIX_0351 = 0
endif
endif
ifndef FIX_0352
if FIX_BUGS ge 1
FIX_0352 = 1
else
FIX_0352 = 0
endif
endif
ifndef FIX_0353
if FIX_BUGS ge 1 and FIX_EXPLOITS ne 0
FIX_0353 = 1
else
FIX_0353 = 0
endif
endif
ifndef FIX_0354
if FIX_BUGS ge 1 and FIX_EXPLOITS ne 0
FIX_0354 = 1
else
FIX_0354 = 0
endif
endif
ifndef FIX_0355
if FIX_BUGS ge 2
FIX_0355 = 1
else
FIX_0355 = 0
endif
endif
ifndef FIX_0356
if FIX_BUGS ge 1
FIX_0356 = 1
else
FIX_0356 = 0
endif
endif
ifndef FIX_0357
if FIX_BUGS ge 1 and FIX_EXPLOITS ne 0
FIX_0357 = 1
else
FIX_0357 = 0
endif
endif
ifndef FIX_0358
if FIX_BUGS ge 1 and FIX_EXPLOITS ne 0
FIX_0358 = 1
else
FIX_0358 = 0
endif
endif
ifndef FIX_0359
if FIX_BUGS ge 1
FIX_0359 = 1
else
FIX_0359 = 0
endif
endif
ifndef FIX_0360
if FIX_BUGS ge 1 and FIX_EXPLOITS ne 0
FIX_0360 = 1
else
FIX_0360 = 0
endif
endif
ifndef FIX_0361
if FIX_BUGS ge 1
FIX_0361 = 1
else
FIX_0361 = 0
endif
endif
ifndef FIX_0362
if FIX_BUGS ge 1
FIX_0362 = 1
else
FIX_0362 = 0
endif
endif
ifndef FIX_0363
if FIX_BUGS ge 1
FIX_0363 = 1
else
FIX_0363 = 0
endif
endif
ifndef FIX_0364
if FIX_BUGS ge 1
FIX_0364 = 1
else
FIX_0364 = 0
endif
endif
ifndef FIX_0365
if FIX_BUGS ge 2
FIX_0365 = 1
else
FIX_0365 = 0
endif
endif
ifndef FIX_0366
if FIX_BUGS ge 2
FIX_0366 = 1
else
FIX_0366 = 0
endif
endif
ifndef FIX_0367
if FIX_BUGS ge 1
FIX_0367 = 1
else
FIX_0367 = 0
endif
endif
ifndef FIX_0368
if FIX_BUGS ge 1 and FIX_EXPLOITS ne 0
FIX_0368 = 1
else
FIX_0368 = 0
endif
endif
ifndef FIX_0369
if FIX_BUGS ge 1 and FIX_EXPLOITS ne 0
FIX_0369 = 1
else
FIX_0369 = 0
endif
endif
ifndef FIX_0370
if FIX_BUGS ge 1 and FIX_EXPLOITS ne 0
FIX_0370 = 1
else
FIX_0370 = 0
endif
endif
ifndef FIX_0371
if FIX_BUGS ge 1
FIX_0371 = 1
else
FIX_0371 = 0
endif
endif
ifndef FIX_0372
if FIX_BUGS ge 2
FIX_0372 = 1
else
FIX_0372 = 0
endif
endif
ifndef FIX_0373
if FIX_BUGS ge 1
FIX_0373 = 1
else
FIX_0373 = 0
endif
endif
ifndef FIX_0374
if FIX_BUGS ge 1 and FIX_EXPLOITS ne 0
FIX_0374 = 1
else
FIX_0374 = 0
endif
endif
ifndef FIX_0375
if FIX_BUGS ge 1
FIX_0375 = 1
else
FIX_0375 = 0
endif
endif
ifndef FIX_0376
if FIX_BUGS ge 2
FIX_0376 = 1
else
FIX_0376 = 0
endif
endif
ifndef FIX_0377
if FIX_BUGS ge 1
FIX_0377 = 1
else
FIX_0377 = 0
endif
endif
ifndef FIX_0378
if FIX_BUGS ge 1
FIX_0378 = 1
else
FIX_0378 = 0
endif
endif

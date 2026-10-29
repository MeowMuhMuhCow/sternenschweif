import re
s=open('/root/sternenschweif/wovat/OVL257.HDR').read()
sw=open('/root/sternenschweif/wovat/gen/sw257.txt').read().rstrip('\n')
fl=sorted(set(re.findall(r'g_[0-9a-f]{4}',sw))|{'g_25a8','g_25aa'})
s=s.replace('@FLAGS@','\n'.join('extern signed char %s;'%f for f in fl))
s=s.replace('@SWITCH@',sw)
s=s.replace('#include <stdarg.h>','#include <stdarg.h>\n#include <stdio.h>')
open('/root/sternenschweif/wovat/OVL257.CPP','w').write(s)

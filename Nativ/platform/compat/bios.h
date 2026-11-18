#ifndef DSA_COMPAT_BIOS_H
#define DSA_COMPAT_BIOS_H

#ifdef __cplusplus
extern "C" {
#endif

int dsa_bioskey(int cmd);
#define bioskey(cmd) dsa_bioskey(cmd)

static inline int biosprint(int cmd, int abyte, int port) {
    (void)cmd; (void)abyte; (void)port;
    return 0x90; /* Drucker bereit */
}

#ifdef __cplusplus
}
#endif

#endif /* DSA_COMPAT_BIOS_H */

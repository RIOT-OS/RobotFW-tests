#ifndef SHELL_ARGS_H
#define SHELL_ARGS_H

#define CONVERT_ERROR   (-32768)

#define ARGS_OK     (0)
#define ARGS_ERROR  (-1)

int sc_args_check(int argc, char **argv, int c_min, int c_max, char *use);

int sc_arg2int(const char *arg, int *val);

int sc_arg2long(const char *arg, long *val);

int sc_arg2uint(const char *arg, unsigned int *val);

int sc_arg2ulong(const char *arg, unsigned long *val);

int sc_arg2u32(const char *arg, uint32_t *val);

int sc_arg2s32(const char *arg, int32_t *val);

int sc_arg2u16(const char *arg, uint16_t *val);

int sc_arg2u8(const char *arg, uint8_t *val);

int sc_arg2dev(const char *arg, unsigned maxdev);

#endif /* SHELL_ARGS_H */

/*
 * About:
 *   This is source code of "pushkey" utility for emulating pressing keypad buttons on Motorola EZX platform.
 *
 * Author:
 *   EXL
 *
 * License:
 *   Public Domain
 *
 * History:
 *   28-Nov-2021: Added and tested Motorola E680 (R51) platform.
 */

/* Linux */
#include <sys/ioctl.h>
#include <linux/keypad.h>

/* POSIX */
#include <fcntl.h>
#include <unistd.h>

/* C */
#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#if defined(EZX_E680)
	#define KEYPADI_DEVICENAME "/dev/keypad1"
	#define KEYPADB_DEVICENAME "/dev/keypad2"
#elif defined(EZX_E398_JUIX_P2)
	#define KEYPADI_DEVICENAME "/dev/keypadI"
	#define KEYPADB_DEVICENAME "/dev/keypadB"
#else
	#define KEYPADI_DEVICENAME "/dev/keypadI"
	#define KEYPADB_DEVICENAME "/dev/keypadB"
#endif

typedef uint16_t key_code_short_t;
typedef uint32_t key_code_long_t;

typedef enum {
	KEYMODE_DEFAULT,
	KEYMODE_SINGLE
} key_mode_t;

static int print_error(const char *format, const char *argument) {
	fprintf(stderr, format, argument);
	return 1;
}

static int ioctl_error(const char *argument) {
	print_error("pushkey: Error: ioctl: %s\n", strerror(errno));
	return print_error("pushkey: Error: Cannot insert event to the \"%s\" device!\n", argument);
}

static int print_usage(void) {
	fprintf(
		stderr,
		"|pushkey| by EXL, v1.0, 28-Nov-2021\n\n"
		"Usage:\n"
		"\tpushkey <keycode> <device> <mode>\n\n"
		"Examples:\n"
		"\tpushkey 0x8001         # push \"1\" button on \"" KEYPADI_DEVICENAME "\", default mode\n"
		"\tpushkey 0x0001         # release \"1\" button on \"" KEYPADI_DEVICENAME "\", default mode\n\n"
		"\tpushkey 0x8001 i       # push \"1\" button on \"" KEYPADI_DEVICENAME "\", default mode\n"
		"\tpushkey 0x0001 i       # release \"1\" button on \"" KEYPADI_DEVICENAME "\", default mode\n\n"
		"\tpushkey 0x8001 b       # push \"1\" button on \"" KEYPADB_DEVICENAME "\", default mode\n"
		"\tpushkey 0x0001 b       # release \"1\" button on \"" KEYPADB_DEVICENAME "\", default mode\n\n"
		"\tpushkey 0x0001 i s     # push \"1\" button on \"" KEYPADI_DEVICENAME "\", single mode\n"
		"\tpushkey 0x0001 b s     # push \"1\" button on \"" KEYPADB_DEVICENAME "\", single mode\n"
	);
	return 1;
}

static int send_event_to_keypad_driver(const int device_fd, const char *key, const key_mode_t mode) {
	int result = -1;
	const key_code_short_t keycode = strtoul(key, NULL, 0);
	fprintf(stderr, "Keycode is: 0x%04X, dec: %hu\n", keycode, keycode);
	if (mode == KEYMODE_DEFAULT)
		result = ioctl(device_fd, KEYPAD_IOC_INSERT_EVENT, keycode);
	else if (mode == KEYMODE_SINGLE) {
		result = ioctl(device_fd, KEYPAD_IOC_INSERT_EVENT, (keycode | KEYDOWN));
		result = ioctl(device_fd, KEYPAD_IOC_INSERT_EVENT, keycode);
	}
	return result;
}

static int send_event_to_driver(const char *key, const char *device, const key_mode_t mode) {
	int result = -1;

	const int device_fd = open(device, O_RDWR);
	if (device_fd < 0)
		return print_error("pushkey: Error: Cannot open keypad \"%s\" device!\n", device);

	result = send_event_to_keypad_driver(device_fd, key, mode);

	close(device_fd);
	return (result < 0) ? ioctl_error(device) : 0;
}

static int push_key_exec(int argc, char *argv[]) {
	key_mode_t mode = KEYMODE_DEFAULT;
	char *device = KEYPADI_DEVICENAME;
	if (argc > 2 && argv[2])
		switch (argv[2][0]) {
			case 'b':
				device = KEYPADB_DEVICENAME;
				break;
		}
	if (argc > 3 && argv[3])
		switch (argv[3][0]) {
			case 's':
				mode = KEYMODE_SINGLE;
				break;
		}
	return send_event_to_driver(argv[1], device, mode);
}

int main(int argc, char *argv[]) {
	if (argc < 2 || argc > 4)
		return print_usage();
	return push_key_exec(argc, argv);
}

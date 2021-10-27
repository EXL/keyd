/*
 * About:
 *   This is source code of "pushkey" utility for emulating pressing keypad buttons on Motorola MotoMAGX platform.
 *
 * Author:
 *   EXL
 *
 * License:
 *   Public Domain
 *
 * History:
 *   28-Oct-2021: Sending key events to driver works, move to C language.
 *   27-Oct-2021: Trying to send uint16_t, drop key server pipe.
 *   26-Oct-2021: First draft POC prototype version.
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

typedef uint16_t key_code_t;

typedef enum {
	KEYMODE_DEFAULT,
	KEYMODE_ASCII,
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
		"|pushkey| by EXL, v1.0, 28-Oct-2021\n\n"
		"Usage:\n"
		"\tpushkey <keycode> <device> <mode>\n\n"
		"Examples:\n"
		"\tpushkey 0x8001 # push \"1\" button on keypadI, default mode\n"
		"\tpushkey 0x0001 # release \"1\" button on keypadI, default mode\n\n"
		"\tpushkey 0x8001 i # push \"1\" button on keypadI, default mode\n"
		"\tpushkey 0x0001 i # release \"1\" button on keypadI, default mode\n\n"
		"\tpushkey 0x8001 b # push \"1\" button on keypadB, default mode\n"
		"\tpushkey 0x0001 b # release \"1\" button on keypadB, default mode\n\n"
		"\tpushkey 0x31 i a # push \"1\" button on keypadI, ascii mode\n"
		"\tpushkey 0x31 b a # push \"1\" button on keypadB, ascii mode\n\n"
		"\tpushkey 0x0001 i s # push \"1\" button on keypadI, single mode\n"
		"\tpushkey 0x0001 b s # push \"1\" button on keypadB, single mode\n"
	);
	return 1;
}

static int send_event_to_driver(const char *key, const char *device, const key_mode_t mode) {
	const key_code_t keycode = strtoul(key, NULL, 0);
	int result = -1;
	fprintf(stderr, "Keycode is: 0x%04X, dec: %hu\n", keycode, keycode);

	const int keypad = open(device, O_RDWR);
	if (keypad < 0)
		return print_error("pushkey: Error: Cannot open keypad \"%s\" device!\n", device);

	if (mode == KEYMODE_DEFAULT)
		result = ioctl(keypad, KEYPAD_IOC_INSERT_EVENT, keycode);
	else if (mode == KEYMODE_ASCII)
		result = ioctl(keypad, KEYPAD_IOC_INSERT_ASCII_EVENT, keycode);
	else if (mode == KEYMODE_SINGLE) {
		result = ioctl(keypad, KEYPAD_IOC_INSERT_EVENT, (keycode | 0x8000));
		result = ioctl(keypad, KEYPAD_IOC_INSERT_EVENT, keycode);
	}

	close(keypad);
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
			case 'a':
				mode = KEYMODE_ASCII;
				break;
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

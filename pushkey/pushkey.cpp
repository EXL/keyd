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
 *   27-Oct-2021: Trying to send uint16_t, drop key server pipe.
 *   26-Oct-2021: First draft POC prototype version.
 */

// Motorola
//#include <moto_input/vkm_ext.h>

// Linux
#include <sys/ioctl.h>
#include <linux/keypad.h>

// POSIX
#include <fcntl.h>
#include <unistd.h>

// C
#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef uint16_t KeyCode_t;

static int print_error(const char *a_error) {
	fprintf(stderr, a_error);
	return 1;
}

static int print_usage(void) {
	fprintf(
		stderr,
		"|pushkey| by EXL, v1.0, 27-Oct-2021\n\n"
		"Usage:\n"
		"\tpushkey i 0x8001 (push 1)\n"
		"\tpushkey i 0x0001 (release 1)\n"
		"\tpushkey b 0x8001 (push 1)\n"
		"\tpushkey b 0x0001 (release 1)\n\n"
		"\tpushkey i 0x31 ascii (push 1)\n"
		"\tpushkey b 0x31 ascii (push 1)\n\n"
	);
	return 1;
}

static int send_event_to_driver(const uint16_t a_keycode, const int a_ascii, const int a_keypad_b) {
	const int l_keypad_dev = open((a_keypad_b) ? KEYPADB_DEVICENAME : KEYPADI_DEVICENAME, O_RDWR);
	if (l_keypad_dev < 0)
		return print_error("pushkey: Error: Cannot open keypad \""KEYPADI_DEVICENAME"\" device!\n");
	if (ioctl(l_keypad_dev, (a_ascii) ? KEYPAD_IOC_INSERT_ASCII_EVENT : KEYPAD_IOC_INSERT_EVENT, a_keycode) < 0) {
		fprintf(stderr, "pushkey: Error: ioctl: %s\n", strerror(errno));
		return print_error("pushkey: Error: Cannot insert event to the \""KEYPADI_DEVICENAME"\" device!\n");
	}
	close(l_keypad_dev);
	return 0;
}

static int parse_keycode(const char *a_mode, const char *a_key, const int a_ascii) {
	KeyCode_t l_keycode = strtoul(a_key, NULL, 0);
	fprintf(stderr, "Keycode is: 0x%04X, dec: %hu\n", l_keycode, l_keycode);

//	MOTO_INPUT_EVENT_T l_event;
//	l_event.type = MOTO_INPUT_EVENT_TYPE_KEY;
//	l_event.event.key_event = l_keycode;

	return send_event_to_driver(l_keycode, a_ascii, (a_mode[0] == 'b'));
}

int main(int argc, char *argv[]) {
	if (argc < 3)
		return print_usage();
	return parse_keycode(argv[1], argv[2], (argc > 3));
}

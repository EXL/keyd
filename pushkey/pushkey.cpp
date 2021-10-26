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
 *   26-Oct-2021: First draft POC prototype version.
 */

// Motorola
#include <moto_input/vkm_ext.h>

// Linux
#include <sys/ioctl.h>
#include <linux/keypad.h>

// POSIX
#include <fcntl.h>
#include <unistd.h>

// C
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define KEY_SERVER_PIPE "/tmp/ui-framework-keytest-fifo"

typedef uint32_t KeyCode_t;

static int print_error(const char *a_error) {
	fprintf(stderr, a_error);
	return 1;
}

static int print_usage(void) {
	fprintf(
		stderr,
		"|pushkey| by EXL, v1.0, 26-Oct-2021\n\n"
		"Usage:\n\t"
		"pushkey 0x80000001 (push 1)\n\t"
		"pushkey 0x00000001 (release 1)\n\n\t"
		"pushkey 0x80000001 server (push 1)\n\t"
		"pushkey 0x00000001 server (release 1)\n"
	);
	return 1;
}

static int send_event_to_server(const MOTO_INPUT_EVENT_T &a_event) {
	static int l_server_fd = -1;
	if (l_server_fd == -1) {
		l_server_fd = open(KEY_SERVER_PIPE, O_WRONLY);
		if (l_server_fd < 0)
			return print_error("pushkey: Error: Cannot connect to the \""KEY_SERVER_PIPE"\" server!\n");
		return !(sizeof(a_event) == write(l_server_fd, &a_event, sizeof(a_event)));
	}
}

static int send_event_to_driver(const MOTO_INPUT_EVENT_T &a_event) {
	const int l_keypad_dev = open(KEYPADI_DEVICENAME, O_RDWR);
	if (l_keypad_dev < 0)
		return print_error("pushkey: Error: Cannot open keypad \""KEYPADI_DEVICENAME"\" device!\n");
	if (ioctl(l_keypad_dev, KEYPAD_IOC_INSERT_EVENT, &a_event) < 0)
		return print_error("pushkey: Error: Cannot insert event to the \""KEYPADI_DEVICENAME"\" device!\n");
	close(l_keypad_dev);
	return 0;
}

static int parse_keycode(const char *a_key, int a_server) {
	KeyCode_t l_keycode = strtoll(a_key, NULL, 0);
	fprintf(stderr, "Keycode is: 0x%08X\n", l_keycode);

	MOTO_INPUT_EVENT_T l_event;
	l_event.type = MOTO_INPUT_EVENT_TYPE_KEY;
	l_event.event.key_event = l_keycode;

	return (a_server) ? send_event_to_server(l_event) : send_event_to_driver(l_event);
}

int main(int argc, char *argv[]) {
	if (argc < 2)
		return print_usage();
	return parse_keycode(argv[1], (argc > 2));
}

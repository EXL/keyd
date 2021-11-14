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
 *   14-Nov-2021: First public release, fix help, typos, and bump version date.
 *   10-Nov-2021: Add "/dev/vkm" driver which is used on MotoMAGX emulator and probably modern MotoMAGX phones.
 *   28-Oct-2021: Move to the C language since sending key events to "/dev/keypadX" drivers now works.
 *   27-Oct-2021: Drop keys server pipe and trying to send uint16_t keycode.
 *   26-Oct-2021: First draft POC prototype version.
 */

/* Motorola */
#include <moto_input/vkm_ext.h>

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

typedef uint16_t key_code_short_t;
typedef uint32_t key_code_long_t;

typedef enum {
	KEYMODE_DEFAULT,
	KEYMODE_ASCII,
	KEYMODE_SINGLE
} key_mode_t;

typedef enum {
	DRIVER_KEYPAD,
	DRIVER_VKM
} driver_t;

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
		"|pushkey| by EXL, v1.0, 14-Nov-2021\n\n"
		"Usage:\n"
		"\tpushkey <keycode> <device> <mode>\n\n"
		"Examples:\n"
		"\tpushkey 0x8001         # push \"1\" button on \"/dev/keypadI\", default mode\n"
		"\tpushkey 0x0001         # release \"1\" button on \"/dev/keypadI\", default mode\n\n"
		"\tpushkey 0x8001 i       # push \"1\" button on \"/dev/keypadI\", default mode\n"
		"\tpushkey 0x0001 i       # release \"1\" button on \"/dev/keypadI\", default mode\n\n"
		"\tpushkey 0x8001 b       # push \"1\" button on \"/dev/keypadB\", default mode\n"
		"\tpushkey 0x0001 b       # release \"1\" button on \"/dev/keypadB\", default mode\n\n"
		"\tpushkey 0x31 i a       # push \"1\" button on \"/dev/keypadI\", ASCII mode\n"
		"\tpushkey 0x31 b a       # push \"1\" button on \"/dev/keypadB\", ASCII mode\n\n"
		"\tpushkey 0x0001 i s     # push \"1\" button on \"/dev/keypadI\", single mode\n"
		"\tpushkey 0x0001 b s     # push \"1\" button on \"/dev/keypadB\", single mode\n\n"
		"\tpushkey 0x80000004 v   # push \"4\" button on \"/dev/vkm\", default mode\n"
		"\tpushkey 0x00000004 v   # release \"4\" button on \"/dev/vkm\", default mode\n\n"
		"\tpushkey 0x00000004 v s # push \"4\" button on \"/dev/vkm\", single mode\n"
	);
	return 1;
}

static MOTO_INPUT_EVENT_T create_vkm_moto_input_event(const key_code_long_t keycode) {
	MOTO_INPUT_EVENT_T moto_input_event;
	moto_input_event.type = MOTO_INPUT_EVENT_TYPE_KEY;
	moto_input_event.event.key_event = keycode;
	return moto_input_event;
}

static int send_event_to_vkm_driver(const int device_fd, const char *key, const key_mode_t mode) {
	int result = -1;
	const key_code_long_t keycode = strtoul(key, NULL, 0);
	fprintf(stderr, "Keycode is: 0x%08X, dec: %u\n", keycode, keycode);
	if (mode == KEYMODE_ASCII)
		fprintf(stderr, "Mode \"KEYMODE_ASCII\" isn\'t supported on VKM driver.\n");
	else if (mode == KEYMODE_DEFAULT) {
		MOTO_INPUT_EVENT_T vkm_event = create_vkm_moto_input_event(keycode);
		result = ioctl(device_fd, MOTO_INPUT_IOC_INSERT_EVENT, &vkm_event);
	} else if (mode == KEYMODE_SINGLE) {
		MOTO_INPUT_EVENT_T vkm_event_press = create_vkm_moto_input_event(keycode | MOTO_INPUT_KEY_EVENT_PRESSED_MASK);
		MOTO_INPUT_EVENT_T vkm_event_release = create_vkm_moto_input_event(keycode);
		result = ioctl(device_fd, MOTO_INPUT_IOC_INSERT_EVENT, &vkm_event_press);
		result = ioctl(device_fd, MOTO_INPUT_IOC_INSERT_EVENT, &vkm_event_release);
	}
	return result;
}

static int send_event_to_keypad_driver(const int device_fd, const char *key, const key_mode_t mode) {
	int result = -1;
	const key_code_short_t keycode = strtoul(key, NULL, 0);
	fprintf(stderr, "Keycode is: 0x%04X, dec: %hu\n", keycode, keycode);
	if (mode == KEYMODE_DEFAULT)
		result = ioctl(device_fd, KEYPAD_IOC_INSERT_EVENT, keycode);
	else if (mode == KEYMODE_ASCII)
		result = ioctl(device_fd, KEYPAD_IOC_INSERT_ASCII_EVENT, keycode);
	else if (mode == KEYMODE_SINGLE) {
		result = ioctl(device_fd, KEYPAD_IOC_INSERT_EVENT, (keycode | KEYDOWN));
		result = ioctl(device_fd, KEYPAD_IOC_INSERT_EVENT, keycode);
	}
	return result;
}

static int send_event_to_driver(const char *key, const char *device, const key_mode_t mode, const driver_t driver) {
	int result = -1;

	const int device_fd = open(device, O_RDWR);
	if (device_fd < 0)
		return print_error("pushkey: Error: Cannot open keypad \"%s\" device!\n", device);

	result = (driver == DRIVER_VKM) ?
		send_event_to_vkm_driver(device_fd, key, mode) :
		send_event_to_keypad_driver(device_fd, key, mode);

	close(device_fd);
	return (result < 0) ? ioctl_error(device) : 0;
}

static int push_key_exec(int argc, char *argv[]) {
	key_mode_t mode = KEYMODE_DEFAULT;
	driver_t driver = DRIVER_KEYPAD;
	char *device = KEYPADI_DEVICENAME;
	if (argc > 2 && argv[2])
		switch (argv[2][0]) {
			case 'b':
				device = KEYPADB_DEVICENAME;
				break;
			case 'v':
				device = "/dev/" VKM_DEV_NAME;
				driver = DRIVER_VKM;
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
	return send_event_to_driver(argv[1], device, mode, driver);
}

int main(int argc, char *argv[]) {
	if (argc < 2 || argc > 4)
		return print_usage();
	return push_key_exec(argc, argv);
}

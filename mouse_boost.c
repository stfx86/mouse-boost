#define _GNU_SOURCE
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <libevdev-1.0/libevdev/libevdev.h>
#include <linux/uinput.h>
#include <string.h>
#include <stdlib.h>

#define BOOST 2.5   // ← change this (10, 50, 100…)

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s /dev/input/eventX\n", argv[0]);
        return 1;
    }

    int fd = open(argv[1], O_RDONLY|O_NONBLOCK);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    struct libevdev *dev = NULL;
    if (libevdev_new_from_fd(fd, &dev) < 0) {
        fprintf(stderr, "libevdev init failed\n");
        return 1;
    }

    printf("[+] Using device: %s\n", libevdev_get_name(dev));

    if (libevdev_grab(dev, LIBEVDEV_GRAB) != 0) {
        fprintf(stderr, "Failed to grab device\n");
        return 1;
    }

    int ufd = open("/dev/uinput", O_WRONLY|O_NONBLOCK);
    if (ufd < 0) {
        perror("uinput");
        return 1;
    }

    ioctl(ufd, UI_SET_EVBIT, EV_KEY);
    ioctl(ufd, UI_SET_EVBIT, EV_REL);
    ioctl(ufd, UI_SET_RELBIT, REL_X);
    ioctl(ufd, UI_SET_RELBIT, REL_Y);
    ioctl(ufd, UI_SET_RELBIT, REL_WHEEL);

    for (int i = BTN_LEFT; i <= BTN_TASK; i++)
        ioctl(ufd, UI_SET_KEYBIT, i);

    struct uinput_setup usetup = {0};
    snprintf(usetup.name, UINPUT_MAX_NAME_SIZE, "turbo-mouse");
    usetup.id.bustype = BUS_USB;
    usetup.id.vendor  = 0x1337;
    usetup.id.product = 0xBEEF;

    ioctl(ufd, UI_DEV_SETUP, &usetup);
    ioctl(ufd, UI_DEV_CREATE);

    sleep(1);
    printf("[+] Virtual mouse ready (BOOST=%d)\n", BOOST);

    struct input_event ev;
    while (1) {
        int rc = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_NORMAL, &ev);
        if (rc == LIBEVDEV_READ_STATUS_SUCCESS) {

            if (ev.type == EV_REL &&
               (ev.code == REL_X || ev.code == REL_Y)) {
                ev.value *= BOOST;
            }

            write(ufd, &ev, sizeof(ev));
        }
    }
}

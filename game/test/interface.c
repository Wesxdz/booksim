#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <limits.h>

#define MAX_EVENTS 1024
#define LEN_NAME 16
#define EVENT_SIZE (sizeof (struct inotify_event))
#define BUF_LEN ((MAX_EVENTS * (EVENT_SIZE + LEN_NAME)) + 1)

int main() {
    int length, i = 0, fd;
    int wd;
    char buffer[BUF_LEN];

    fd = inotify_init();

    if (fd < 0) {
        perror("inotify_init");
    }

    wd = inotify_add_watch(fd, ".", IN_CREATE);

    if (wd < 0) {
        printf("Couldn't add watch to %s\n", ".");
    } else {
        printf("Watching:: %s\n", ".");
    }

    while (1) {
        i = 0;
        length = read(fd, buffer, BUF_LEN);

        if (length < 0) {
            perror("read");
        }

        while (i < length) {
            struct inotify_event *event = (struct inotify_event *) &buffer[i];
            if (event->len) {
                if (event->mask & IN_CREATE) {
                    if (!(event->mask & IN_ISDIR)) {
                        printf("New file %s created.\n", event->name);
                        // Add your logic here to read the new file
                    }
                }
            }
            i += EVENT_SIZE + event->len;
        }
    }

    inotify_rm_watch(fd, wd);
    close(fd);

    return 0;
}

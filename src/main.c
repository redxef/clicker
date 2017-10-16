/**
 * @author      Manuel Federanko
 * @file        main.c
 * @version     0.0.0-r0
 * @since
 *
 * @brief       A brief documentation about the file.
 *
 * A detailed documentation.
 */


#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <stdbool.h>
#include <getopt.h>
#include <sys/time.h>
#include <Carbon/Carbon.h>
#include <ApplicationServices/ApplicationServices.h>



#define NUM_KEYS        8



enum mouse_button {
        mb_left,
        mb_right,
        mb_other
};

#define mbtn_get_evt_down(mbtn)         ((mbtn == mb_left) ? kCGEventLeftMouseDown : (mbtn == mb_right) ? kCGEventRightMouseDown : kCGEventOtherMouseDown)
#define mbtn_get_evt_up(mbtn)           ((mbtn == mb_left) ? kCGEventLeftMouseUp: (mbtn == mb_right) ? kCGEventRightMouseUp : kCGEventOtherMouseUp)
#define mbtn_get_kcgbtn(mbtn)           ((mbtn == mb_left) ? kCGMouseButtonLeft: (mbtn == mb_right) ? kCGMouseButtonRight : 0)

#define custom_CGEventPost(pid, tap, evt)    ((pid != 0)? CGEventPostToPid(pid, evt) : CGEventPost(tap, evt))


/* take care, preprocessor magic following */
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-pp-token"
#define single_quote                    '
#define _                               '               /* pls don't mess up the formatting */
#pragma clang diagnostic pop

#define concat_h(x, y, z)               z##y##z
#define concat(x, y, z)                 concat_h(x, y, z)
#define to_char(x)                      concat(single_quote, x, single_quote)
#define gen_ascii_char_add(c, add)      (ascii_to_keycode[(uint16_t) (to_char(c) + add)] = kVK_ANSI_##c)
#define gen_ascii_char(c)               (ascii_to_keycode[(uint16_t) to_char(c)] = kVK_ANSI_##c)

#define gen_ascii_char_num()            \
        gen_ascii_char(0),              \
        gen_ascii_char(1),              \
        gen_ascii_char(2),              \
        gen_ascii_char(3),              \
        gen_ascii_char(4),              \
        gen_ascii_char(5),              \
        gen_ascii_char(6),              \
        gen_ascii_char(7),              \
        gen_ascii_char(8),              \
        gen_ascii_char(9)               \

#define gen_ascii_char_lh(c)            gen_ascii_char(c), gen_ascii_char_add(c, 'a' - 'A')

uint16_t ascii_to_keycode[1024];

void do_click(pid_t pid, enum mouse_button btn, int times, useconds_t delay, useconds_t release_delay) {
        CGEventRef press, release;

        CGEventRef event = CGEventCreate(NULL);
        CGPoint cursor = CGEventGetLocation(event);
        CFRelease(event);

        press = CGEventCreateMouseEvent(
                NULL,
                mbtn_get_evt_down(btn),
                cursor,
                mbtn_get_kcgbtn(btn)
        );

        release = CGEventCreateMouseEvent(
                NULL,
                mbtn_get_evt_up(btn),
                cursor,
                mbtn_get_kcgbtn(btn)
        );

        while (times--) {
                custom_CGEventPost(pid, kCGHIDEventTap, press);
                usleep(release_delay);
                custom_CGEventPost(pid, kCGHIDEventTap, release);
                usleep(delay);
        }

        CFRelease(press);
        CFRelease(release);
}

void start_click(pid_t pid, enum mouse_button btn) {
        CGEventRef press;

        CGEventRef event = CGEventCreate(NULL);
        CGPoint cursor = CGEventGetLocation(event);
        CFRelease(event);

        press = CGEventCreateMouseEvent(
                NULL,
                mbtn_get_evt_down(btn),
                cursor,
                mbtn_get_kcgbtn(btn)
        );

        custom_CGEventPost(pid, kCGHIDEventTap, press);
        CFRelease(press);
}

void end_click(pid_t pid, enum mouse_button btn) {
        CGEventRef release;

        CGEventRef event = CGEventCreate(NULL);
        CGPoint cursor = CGEventGetLocation(event);
        CFRelease(event);

        release = CGEventCreateMouseEvent(
                NULL,
                mbtn_get_evt_up(btn),
                cursor,
                mbtn_get_kcgbtn(btn)
        );

        custom_CGEventPost(pid, kCGHIDEventTap, release);
        CFRelease(release);
}

void do_type(pid_t pid, uint16_t c, int times, useconds_t delay, useconds_t release_delay) {
        CGEventRef press;
        CGEventRef release;

        press = CGEventCreateKeyboardEvent(NULL, (CGKeyCode) ascii_to_keycode[c], TRUE);
        release = CGEventCreateKeyboardEvent(NULL, (CGKeyCode) ascii_to_keycode[c], FALSE);

        while(times--) {
                custom_CGEventPost(pid, kCGAnnotatedSessionEventTap, press);
                usleep(release_delay);
                custom_CGEventPost(pid, kCGAnnotatedSessionEventTap, release);
                usleep(delay);
        }

        CFRelease(press);
        CFRelease(release);
}

void init_keyconverter(void) {
        gen_ascii_char_num();
        gen_ascii_char_lh(A);
        gen_ascii_char_lh(B);
        gen_ascii_char_lh(C);
        gen_ascii_char_lh(D);
        gen_ascii_char_lh(E);
        gen_ascii_char_lh(F);
        gen_ascii_char_lh(G);
        gen_ascii_char_lh(H);
        gen_ascii_char_lh(I);
        gen_ascii_char_lh(J);
        gen_ascii_char_lh(K);
        gen_ascii_char_lh(L);
        gen_ascii_char_lh(M);
        gen_ascii_char_lh(N);
        gen_ascii_char_lh(O);
        gen_ascii_char_lh(P);
        gen_ascii_char_lh(Q);
        gen_ascii_char_lh(R);
        gen_ascii_char_lh(S);
        gen_ascii_char_lh(T);
        gen_ascii_char_lh(U);
        gen_ascii_char_lh(V);
        gen_ascii_char_lh(W);
        gen_ascii_char_lh(X);
        gen_ascii_char_lh(Y);
        gen_ascii_char_lh(Z);
}

uint64_t get_time(void) {
        struct timeval t;
        gettimeofday(&t, NULL);
        return (uint64_t)1000000 * t.tv_sec + t.tv_usec;
}

int main(int argc, char **argv) {
        init_keyconverter();

        char key[NUM_KEYS];             /* the keys to press */
        uint64_t keytd[NUM_KEYS];       /* target delay for key */
        uint64_t keylt[NUM_KEYS];       /* last time the key has been pressed */

        char mouse;
        uint64_t mousetd[2];
        uint64_t mouselt[2];

        CGPoint cursor, last_cursor;
        CGEventRef evt;
        int i;
        int ch;
        char *check;

        for (i = 0; i < NUM_KEYS; i++) {
                key[i] = 0;
                keytd[i] = 0;
                keylt[i] = 0;
        }
        mouse = 0;
        mousetd[0] = mousetd[1] = 0;
        mouselt[0] = mouselt[1] = 0;

        while ((ch = getopt(argc, argv, "k:l:r:LRh")) != -1) {
                switch(ch) {
                        case 'k':
                                i = 0;
                                while(key[i] != 0) i++;
                                key[i] = optarg[0];
                                keytd[i] = strtol(&optarg[1], &check, 0);
                                if (strcmp("\0", check) != 0) {
                                        fprintf(stderr, "failed to parse delay argument, assuming 1000ms\n");
                                        keytd[i] = 1000 * 1000;
                                } else {
                                        keytd[i] *= 1000;
                                }
                                break;
                        case 'l':
                                mouse |= 1 << 0;
                                mousetd[0] = strtol(&optarg[0], &check, 0);
                                if (strcmp("\0", check) != 0) {
                                        fprintf(stderr, "failed to parse delay argument, assuming 1000ms\n");
                                        mousetd[0] = 1000 * 1000;
                                } else {
                                        mousetd[0] *= 1000;
                                }
                                break;
                        case 'r':
                                mouse |= 1 << 1;
                                mousetd[1] = strtol(&optarg[0], &check, 0);
                                if (strcmp("\0", check) != 0) {
                                        fprintf(stderr, "failed to parse delay argument, assuming 1000ms\n");
                                        mousetd[1] = 1000 * 1000;
                                } else {
                                        mousetd[1] *= 1000;
                                }
                                break;
                        case 'L':
                                mouse |= 1 << 2;
                                break;
                        case 'R':
                                mouse |= 1 << 3;
                                break;
                        case 'h':
                        default:
                                printf("clicker [-k] [-l] [-r]\n");
                                printf("Click the mouse buttons or type keyboard keys\n");
                                printf("\n");
                                printf("arguments:\n");
                                printf("k:  specify a key to be typed periodicall\n");
                                printf("    the first character in the option is the key\n");
                                printf("    all following characters are interpreted as delay (integer)\n");
                                printf("l:  specify the left mouse button to be typed periodicall\n");
                                printf("    the argument represents the delay\n");
                                printf("r:  same as left mouse button, but the right mouse button\n");
                                printf("\n");
                                printf("all times are specified in milliseconds and can be specified in all 3 forms\n");
                                return 0;
                                break;

                }
        }

        evt = CGEventCreate(NULL);
        cursor = last_cursor = CGEventGetLocation(evt);
        CFRelease(evt);

        if (mouse & (1 << 2)) {
                /* left */
                start_click(0, mb_left);
        }
        if (mouse & (1 << 3)) {
                /* right */
                start_click(0, mb_right);
        }

        while (cursor.x == last_cursor.x && cursor.y == last_cursor.y) {
                for (i = 0; i < NUM_KEYS; i++) {
                        if (key[i] != 0 && keytd[i] < get_time() - keylt[i]) {
                                do_type(0, key[i], 1, 10, 1);
                                keylt[i] = get_time();
                        }
                }

                if ((mouse & (1 << 0)) && mousetd[0] < get_time() - mouselt[0]) {
                        /* left */
                        do_click(0, mb_left, 1, 10, 1);
                        mouselt[0] = get_time();
                }

                if ((mouse & (1 << 1)) && mousetd[1] < get_time() - mouselt[1]) {
                        /* right */
                        do_click(0, mb_right, 1, 10, 1);
                        mouselt[1] = get_time();
                }

                last_cursor = cursor;
                evt = CGEventCreate(NULL);
                cursor = CGEventGetLocation(evt);
                CFRelease(evt);
        }

        if (mouse & (1 << 2)) {
                /* left */
                end_click(0, mb_left);
        }
        if (mouse & (1 << 3)) {
                /* right */
                end_click(0, mb_right);
        }
}

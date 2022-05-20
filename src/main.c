#include <gba_console.h>
#include <gba_dma.h>
#include <gba_timers.h>
#include <gba_video.h>
#include <gba_input.h>
#include <stdio.h>

#define REG_WAITCNT *(vu16*) 0x04000204
#define GREEN (RGB8(25,  179, 25))
#define RED   (RGB8(237, 5,   5))

#define IDLES_1 \
    __asm__( \
        "mul r3, r8, r8 \n" \
    );

#define IDLES_2 \
    __asm__( \
        "mul r3, r8, r9 \n" \
    );

#define IDLES_3 \
    __asm__( \
        "mul r3, r8, r10 \n" \
    );

#define IDLES_4 \
    __asm__( \
        "mul r3, r8, r11 \n" \
    );

#define IDLES_5 \
    __asm__( \
        "mul r3, r8, r11 \n" \
        "mul r3, r8, r8 \n" \
    );

#define IDLES_6 \
    __asm__( \
        "mul r3, r8, r11 \n" \
        "mul r3, r8, r9 \n" \
    );

#define IDLES_7 \
    __asm__( \
        "mul r3, r8, r11 \n" \
        "mul r3, r8, r10 \n" \
    );

#define IDLES_8 \
    __asm__( \
        "mul r3, r8, r11 \n" \
        "mul r3, r8, r11 \n" \
    );

#define SETUP(idles) \
\
    /* push callee saved registers */ \
\
    __asm__( \
        "push {r4-r11}" \
    ); \
\
    /* r8 - r11 will be various registers that can be used */ \
    /* with the mul instruction to manipulate the amount of */ \
    /* idle cycles the precede the test. this puts the prefetch */ \
    /* buffer in a different state during each test. theoretically */ \
    /* this should cause timing differences. r7 will contain an */ \
    /* address to ROM */ \
\
    __asm__( \
        "ldr r7,  =0x08000000 \n" \
        "ldr r8,  =0x000000AA \n" \
        "ldr r9,  =0x0000AAAA \n" \
        "ldr r10, =0x00AAAAAA \n" \
        "ldr r11, =0xAAAAAAAA" \
    ); \
\
    /* now we start the timer */ \
\
    __asm__( \
        "ldr r0, =0x4000100 \n" \
        "ldr r1, =0x800000 \n" \
        "str r1, [r0]" \
    ); \
\
    /* manipulate the prefetch buffer a bunch */ \
\
    idles \

#define TEARDOWN \
    /* grab the timers value and stop it */ \
\
    __asm__( \
        "ldrh r3, [r0] \n" \
        "ldr r2, =#0x0 \n" \
        "str r2, [r0] \n" \
        "mov r0, r3" \
    ); \
\
    /* time to return */ \
\
    __asm__( \
        "pop {r4-r11} \n" \
        "bx lr" \
    ); \

#define CREATE_CALIBRATION(name, idles) \
    __attribute__((naked)) __attribute__((target("arm"))) int name() {\
\
        SETUP(idles) \
        /* run the test */ \
\
        TEARDOWN \
    }

#define CREATE_READ_TEST(name, idles) \
    __attribute__((naked)) __attribute__((target("arm"))) int name() {\
\
        SETUP(idles) \
        /* run the test */ \
\
        __asm__( \
            "ldr r6, [r7]" \
        ); \
\
        TEARDOWN \
    }

#define CREATE_WRITE_TEST(name, idles) \
    __attribute__((naked)) __attribute__((target("arm"))) int name() {\
\
        SETUP(idles) \
        /* run the test */ \
\
        __asm__( \
            "str r6, [r7]" \
        ); \
\
        TEARDOWN \
    }

CREATE_CALIBRATION(calibration1, IDLES_1);
CREATE_CALIBRATION(calibration2, IDLES_2);
CREATE_CALIBRATION(calibration3, IDLES_3);
CREATE_CALIBRATION(calibration4, IDLES_4);
CREATE_CALIBRATION(calibration5, IDLES_5);
CREATE_CALIBRATION(calibration6, IDLES_6);
CREATE_CALIBRATION(calibration7, IDLES_7);
CREATE_CALIBRATION(calibration8, IDLES_8);
CREATE_READ_TEST(read_test1, IDLES_1);
CREATE_READ_TEST(read_test2, IDLES_2);
CREATE_READ_TEST(read_test3, IDLES_3);
CREATE_READ_TEST(read_test4, IDLES_4);
CREATE_READ_TEST(read_test5, IDLES_5);
CREATE_READ_TEST(read_test6, IDLES_6);
CREATE_READ_TEST(read_test7, IDLES_7);
CREATE_READ_TEST(read_test8, IDLES_8);
CREATE_WRITE_TEST(write_test1, IDLES_1);
CREATE_WRITE_TEST(write_test2, IDLES_2);
CREATE_WRITE_TEST(write_test3, IDLES_3);
CREATE_WRITE_TEST(write_test4, IDLES_4);
CREATE_WRITE_TEST(write_test5, IDLES_5);
CREATE_WRITE_TEST(write_test6, IDLES_6);
CREATE_WRITE_TEST(write_test7, IDLES_7);
CREATE_WRITE_TEST(write_test8, IDLES_8);

#define NUM_TESTS 8
int (*calibration[8])()  = {calibration1, calibration2, calibration3, calibration4, calibration5, calibration6, calibration7, calibration8};
int (*read_tests[8])() = {read_test1, read_test2, read_test3, read_test4, read_test5, read_test6, read_test7, read_test8};
int (*write_tests[8])() = {write_test1, write_test2, write_test3, write_test4, write_test5, write_test6, write_test7, write_test8};

int waitstates[4] = {0x4000, 0x4004, 0x4010, 0x4014};

unsigned short read_expected[32] = {
    0x11, 0x0f, 0x0f, 0x0d,
    0x11, 0x0f, 0x0f, 0x0d,
    0x11, 0x0f, 0x0f, 0x0d,
    0x11, 0x0f, 0x10, 0x0e,
    0x11, 0x0f, 0x0f, 0x0d,
    0x11, 0x0f, 0x0f, 0x0d,
    0x11, 0x0f, 0x10, 0x0e,
    0x11, 0x0f, 0x0f, 0x0d,
};

unsigned short write_expected[32] = {
    0x10, 0x0e, 0x0e, 0x0c,
    0x10, 0x0e, 0x0e, 0x0c,
    0x10, 0x0e, 0x0e, 0x0c,
    0x10, 0x0e, 0x0f, 0x0d,
    0x10, 0x0e, 0x0e, 0x0c,
    0x10, 0x0e, 0x0e, 0x0c,
    0x10, 0x0e, 0x0f, 0x0d,
    0x10, 0x0e, 0x0e, 0x0c,
};

void wait_for_a_press() {
    while (!(REG_KEYINPUT & 1));
    while (REG_KEYINPUT & 1);
}

void do_read_test() {
    printf("  Wait:   .. N. .S NS (read)\n");
    printf("Expected:\n");
    for (int i = 0; i < NUM_TESTS; i++) {
        printf("  %d nops: ", i);
        for (int w = 0; w < 4; w++) {
            printf("%02x ", read_expected[i * 4 + w]);
        }
        printf("\n");
    }

    bool all_correct = true;
    printf("Actual: \n");
    for (int i = 0; i < NUM_TESTS; i++) {
        printf("  %d nops: ", i);
        for (int w = 0; w < 4; w++) {
            int waitstate = waitstates[w];
            REG_WAITCNT = waitstate;

            unsigned short result = read_tests[i]() - calibration[i]();

            all_correct &= result == read_expected[i * 4 + w];
            printf("%02x ", result);
        }

        printf("\n");
    }

    BG_PALETTE[0] = all_correct ? GREEN : RED;
}

void do_write_test() {
    printf("  Wait:   .. N. .S NS (write)\n");
    printf("Expected:\n");
    for (int i = 0; i < NUM_TESTS; i++) {
        printf("  %d nops: ", i);
        for (int w = 0; w < 4; w++) {
            printf("%02x ", write_expected[i * 4 + w]);
        }
        printf("\n");
    }

    bool all_correct = true;
    printf("Actual: \n");
    for (int i = 0; i < NUM_TESTS; i++) {
        printf("  %d nops: ", i);
        for (int w = 0; w < 4; w++) {
            int waitstate = waitstates[w];
            REG_WAITCNT = waitstate;

            unsigned short result = write_tests[i]() - calibration[i]();

            all_correct &= result == write_expected[i * 4 + w];
            printf("%02x ", result);
        }

        printf("\n");
    }

    BG_PALETTE[0] = all_correct ? GREEN : RED;
}

int main(void) {
	consoleDemoInit();

    while (1) {
        do_read_test();
        wait_for_a_press();
        do_write_test();
        wait_for_a_press();
    }
}
#include <gba_console.h>
#include <gba_dma.h>
#include <gba_timers.h>
#include <gba_video.h>
#include <stdio.h>

#define REG_WAITCNT *(vu16*) 0x04000204
#define GREEN (RGB8(25,  179, 25))
#define RED   (RGB8(237, 5,   5))

#define IDLES_SHORT \
    __asm__( \
        "mul r3, r8, r8 \n" \
    );

#define IDLES_MEDIUM \
    __asm__( \
        "mul r3, r8, r9 \n" \
    );

#define IDLES_LONG \
    __asm__( \
        "mul r3, r8, r10 \n" \
    );

#define IDLES_VERYLONG \
    __asm__( \
        "mul r3, r8, r11 \n" \
    );

#define IDLES_VERYVERYLONG \
    __asm__( \
        "mul r3, r8, r11 \n" \
        "mul r3, r8, r8 \n" \
    );

#define IDLES_VERYVERYVERYLONG \
    __asm__( \
        "mul r3, r8, r11 \n" \
        "mul r3, r8, r9 \n" \
    );

#define IDLES_VERYVERYVERYVERYLONG \
    __asm__( \
        "mul r3, r8, r11 \n" \
        "mul r3, r8, r10 \n" \
    );

#define IDLES_REEEEEEEEEEEEEEEEEEEEEEE \
    __asm__( \
        "mul r3, r8, r11 \n" \
        "mul r3, r8, r11 \n" \
    );

#define CREATE_TEST(name, idles) \
    __attribute__((naked)) __attribute__((target("arm"))) int name() {\
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
\
    /* run the test */ \
\
    __asm__( \
        "ldr r6, [r7]" \
    ); \
\
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
}

CREATE_TEST(test1, IDLES_SHORT);
CREATE_TEST(test2, IDLES_MEDIUM);
CREATE_TEST(test3, IDLES_LONG);
CREATE_TEST(test4, IDLES_VERYLONG);
CREATE_TEST(test5, IDLES_VERYVERYLONG);
CREATE_TEST(test6, IDLES_VERYVERYVERYLONG);
CREATE_TEST(test7, IDLES_VERYVERYVERYVERYLONG);
CREATE_TEST(test8, IDLES_REEEEEEEEEEEEEEEEEEEEEEE);

#define NUM_TESTS 8
int (*tests[8])() = {test1, test2, test3, test4, test5, test6, test7, test8};
int waitstates[4] = {0x4000, 0x4010, 0x4004, 0x4014};

unsigned short expected[32] = {
    0x1b, 0x15, 0x19, 0x13,
    0x1b, 0x15, 0x19, 0x13,
    0x1b, 0x15, 0x19, 0x13,
    0x1b, 0x17, 0x19, 0x15,
    0x21, 0x19, 0x1f, 0x17,
    0x21, 0x19, 0x1f, 0x17,
    0x21, 0x1b, 0x1f, 0x19,
    0x21, 0x1b, 0x1f, 0x19,
};

int main(void) {
	consoleDemoInit();

    printf(" expected: \n");
    for (int i = 0; i < NUM_TESTS; i++) {
        printf("     ");
        for (int w = 0; w < 4; w++) {
            printf("%04x ", expected[i * 4 + w]);
        }
        printf("\n");
    }
    printf("\n");

    bool all_correct = true;
    printf(" actual: \n");
    for (int i = 0; i < NUM_TESTS; i++) {
        printf("     ");
        for (int w = 0; w < 4; w++) {
            int waitstate = waitstates[w];
            REG_WAITCNT = waitstate;

            unsigned short result = tests[i]();
            all_correct &= result == expected[i * 4 + w];
            printf("%04x ", result);
        }

        printf("\n");
    }

    BG_PALETTE[0] = all_correct ? GREEN : RED;

    while (1);
}

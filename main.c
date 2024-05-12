#include <stdio.h>
#include <gsKit.h>
#include <libpad.h>
#include <loadfile.h>
#include <sbv_patches.h>
#include <sifrpc.h>
#include <ps2sdkapi.h>

#define GS_MODE_NTSC 0x02
#define GS_MODE_PAL  0x03
#define GS_MODE_DTV_480P  0x50
#define GS_NONINTERLACED 0x00
#define GS_INTERLACED 0x01
#define GS_FIELD 0x00
#define GS_FRAME 0x01
#define GS_PSM_CT24 0x01
#define GS_SETTING_OFF 0x00
#define GS_SETTING_ON 0x01
#define GS_CMODE_REPEAT 0x00
#define GS_CMODE_CLAMP 0x01
#define GS_ZTEST_OFF 0x01

#define gsKit_prim_box(gsGlobal,	x1, y1,		\
					x2, y2,		\
					z, color)	\
	gsKit_prim_quad_3d(gsGlobal,	x1, y1, z,	\
					x1, y2, z,	\
					x2, y1, z,	\
					x2, y2, z, color);

extern unsigned char sio2man_irx[] __attribute__((aligned(16)));
extern const unsigned int size_sio2man_irx;

extern unsigned char padman_irx[] __attribute__((aligned(16)));
extern const unsigned int size_padman_irx;

static char padBuf[256] __attribute__((aligned(64)));

static void init() {
	SifInitRpc(0);
	sbv_patch_enable_lmb();

    // Load sio2man & padman drivers (Better in someway than the ones in bios?)
    SifExecModuleBuffer(&sio2man_irx, size_sio2man_irx, 0, 0, 0);
    SifExecModuleBuffer(&padman_irx, size_padman_irx, 0, 0, 0);

    padInit(0);
    padPortOpen(0, 0, padBuf);
}

static void checkerboard(GSGLOBAL *gsGlobal, int pixels) {
    gsKit_clear(gsGlobal, GS_SETREG_RGBA(0, 0, 0, 0x80)); // Clear to a black background

    // Iterate through each pixel
    for (int y = 0; y < gsGlobal->Height; y += pixels) {
        for (int x = 0; x < gsGlobal->Width; x += pixels) {
            // Calculate whether the current pixel should be black or white
            int shouldColor = ((x / pixels + y / pixels) % 2 == 0);

            // Fill the pixels with black or white
            for (int dy = 0; dy < pixels; dy++) {
                for (int dx = 0; dx < pixels; dx++) {
                    if (!shouldColor) {
                        int newX = x + dx;
                        int newY = y + dy;
                        if (newX < gsGlobal->Width && newY < gsGlobal->Height) {
                            gsKit_prim_point(gsGlobal, newX, newY, 2, GS_SETREG_RGBA(0xFF, 0xFF, 0xFF, 0x80)); // White
                        }
                    }
                }
            }
        }
    }
}


static void colors(GSGLOBAL *gsGlobal) {
    float boxWidth = gsGlobal->Width * 0.04f;
    float boxHeight = gsGlobal->Height * 0.1f;

    for (int i = 0; i < 25; i++) {
        float xPos = i * boxWidth;

        int blackR = i * 10.625f;
        int blackG = i * 10.625f;
        int blackB = i * 10.625f;
        gsKit_prim_box(gsGlobal, xPos, 0.0f, xPos + boxWidth, boxHeight, 2, GS_SETREG_RGBA(blackR, blackG, blackB, 0x80));


        int redR = i * 10.625f;
        gsKit_prim_box(gsGlobal, xPos, boxHeight, xPos + boxWidth, boxHeight * 2, 2, GS_SETREG_RGBA(redR, 0, 0, 0x80));


        int orangeR = i * 10.625f;
        int orangeG = i * 6.f;
        gsKit_prim_box(gsGlobal, xPos, boxHeight * 2, xPos + boxWidth, boxHeight * 3, 2, GS_SETREG_RGBA(orangeR, orangeG, 0, 0x80));


        int yellowR = i * 10.625f;
        int yellowG = i * 10.625f;
        gsKit_prim_box(gsGlobal, xPos, boxHeight * 3, xPos + boxWidth, boxHeight * 4, 2, GS_SETREG_RGBA(yellowR, yellowG, 0, 0x80));

        
        int greenG = i * 10.625f;
        gsKit_prim_box(gsGlobal, xPos, boxHeight * 4, xPos + boxWidth, boxHeight * 5, 2, GS_SETREG_RGBA(0, greenG, 0, 0x80));


        int cyanG = i * 10.625f;
        int cyanB = i * 10.625f;
        gsKit_prim_box(gsGlobal, xPos, boxHeight * 5, xPos + boxWidth, boxHeight * 6, 2, GS_SETREG_RGBA(0, cyanG, cyanB, 0x80));


        int lblueG = i * 5.3333335f;
        int lblueB = i * 10.625f;
        gsKit_prim_box(gsGlobal, xPos, boxHeight * 6, xPos + boxWidth, boxHeight * 7, 2, GS_SETREG_RGBA(0, lblueG, lblueB, 0x80));


        int blueB = i * 10.625f;
        gsKit_prim_box(gsGlobal, xPos, boxHeight * 7, xPos + boxWidth, boxHeight * 8, 2, GS_SETREG_RGBA(0, 0, blueB, 0x80));


        int pinkR = i * 10.625f;
        int pinkB = i * 10.625f;
        gsKit_prim_box(gsGlobal, xPos, boxHeight * 8, xPos + boxWidth, boxHeight * 9, 2, GS_SETREG_RGBA(pinkR, 0, pinkB, 0x80));


        int lpinkR = i * 10.625f;
        int lpinkB = i * 5.3333335f;
        gsKit_prim_box(gsGlobal, xPos, boxHeight * 9, xPos + boxWidth, boxHeight * 10, 2, GS_SETREG_RGBA(lpinkR, 0, lpinkB, 0x80));
    }
}

static void changeMode(GSGLOBAL *gsGlobal, unsigned int *mode) {
    switch (*mode) {
        case 0:
            *mode = 3;
        case 3:
            gsGlobal->Mode = GS_MODE_DTV_480P;
	        gsGlobal->Interlace = GS_NONINTERLACED;
	        gsGlobal->Field = GS_FRAME;
	        gsGlobal->Width = 720;
	        gsGlobal->Height = 480;
            break;
        
        case 2:
            gsGlobal->Mode = GS_MODE_NTSC;
	        gsGlobal->Interlace = GS_INTERLACED;
	        gsGlobal->Field = GS_FIELD;
	        gsGlobal->Width = 640;
	        gsGlobal->Height = 448;
            break;

        default:
            *mode = 1;
        case 1:
            gsGlobal->Mode = GS_MODE_PAL;
	        gsGlobal->Interlace = GS_INTERLACED;
	        gsGlobal->Field = GS_FIELD;
	        gsGlobal->Width = 640;
	        gsGlobal->Height = 512;
            break;
    }

    gsGlobal->DoubleBuffering = GS_SETTING_OFF;
	gsGlobal->ZBuffering = GS_SETTING_OFF;

	gsKit_vram_clear(gsGlobal);
	gsKit_init_screen(gsGlobal);
	gsKit_set_display_offset(gsGlobal, -0.5f, -0.5f);
}

int main() {
    init();

    GSGLOBAL *gsGlobal = gsKit_init_global();
    gsGlobal->DoubleBuffering = GS_SETTING_OFF;
	gsGlobal->ZBuffering = GS_SETTING_OFF;
    gsKit_init_screen(gsGlobal);

    struct padButtonStatus buttons;
    unsigned int buttonState;
    unsigned int mode = gsGlobal->Mode == 0x03 ? 1 : 2;
    unsigned int updateNeeded = 1;
    int pixels = 1;
    int screen = 0;

    while(1) {
        if (updateNeeded) {
            switch (screen) {
                case 1:
                    pixels = (pixels < 1) ? 8 : (pixels > 8) ? (pixels == 9 ? 1 : 8 - (pixels - 8)) : pixels;
                    checkerboard(gsGlobal, pixels);
                    break;

                default:
                    colors(gsGlobal);
                    break;
            }
            
            gsKit_queue_exec(gsGlobal);
            updateNeeded = 0;
            usleep(500000);
        }

        if (padRead(0, 0, &buttons) != 0) {
            buttonState = 0xffff ^ buttons.btns;

            if (buttonState & PAD_SQUARE) {
                updateNeeded++;
                mode++;
                changeMode(gsGlobal, &mode);
            }

            if (buttonState & PAD_TRIANGLE) {
                updateNeeded++;
                mode--;
                changeMode(gsGlobal, &mode);
            }

            if (buttonState & PAD_CROSS) {
                updateNeeded++;
                screen = 0;
            }

            if (buttonState & PAD_CIRCLE) {
                updateNeeded++;
                screen = 1;
            }

            if (buttonState & PAD_UP) {
                (screen == 1) ? pixels++ : 0;
                (screen == 1) ? updateNeeded++ : 0;
            }

            if (buttonState & PAD_DOWN) {
                (screen == 1) ? pixels-- : 0;
                (screen == 1) ? updateNeeded++ : 0;
            }

            if (buttonState & PAD_START) {
                gsKit_clear(gsGlobal, GS_SETREG_RGBA(0x00, 0x00, 0x00, 0x80));
                gsKit_queue_exec(gsGlobal);
                break;
            }
        }

        // Sleeping for Vsync (Duration is in microseconds)
        usleep(gsGlobal->Mode == 0x03 ? 20000 : 16683); // 16683 (NTSC / 59.94fps) // 20000 (PAL / 50.00fps)
    }

    return 0;
}
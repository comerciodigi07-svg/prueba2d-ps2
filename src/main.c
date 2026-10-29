#include <tamtypes.h>
#include <kernel.h>
#include <gsKit.h>
#include <dmaKit.h>
#include <libpad.h>
#include <math.h>
#include <stdio.h>

#define SW 640.0f
#define SH 448.0f
#define PADDLE_W 12.0f
#define PADDLE_H 76.0f
#define BALL_SIZE 10.0f

static GSGLOBAL *gs;
static float player_y = 184.0f;
static float cpu_y = 184.0f;
static float ball_x = 315.0f, ball_y = 219.0f;
static float ball_vx = 3.2f, ball_vy = 2.0f;
static int player_score = 0, cpu_score = 0;
static int running = 1;

static u32 rgba(u8 r, u8 g, u8 b, u8 a) {
    return GS_SETREG_RGBAQ(r, g, b, a, 0x3F800000);
}

static void rect(float x1, float y1, float x2, float y2, u32 color) {
    gsKit_prim_sprite(gs, x1, y1, x2, y2, 1, color);
}

static void digit(int n, float x, float y, u32 color) {
    /* Dígitos tipo siete segmentos, sin fuentes externas. */
    static const unsigned char segs[10] = {
        0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f
    };
    static const float w = 16.0f, h = 4.0f, v = 14.0f;
    unsigned char s = segs[n % 10];
    if (s & 0x01) rect(x, y, x+w, y+h, color);
    if (s & 0x02) rect(x+w-h, y, x+w, y+v, color);
    if (s & 0x04) rect(x+w-h, y+v, x+w, y+v*2, color);
    if (s & 0x08) rect(x, y+v*2-h, x+w, y+v*2, color);
    if (s & 0x10) rect(x, y+v, x+h, y+v*2, color);
    if (s & 0x20) rect(x, y, x+h, y+v, color);
    if (s & 0x40) rect(x, y+v-h/2, x+w, y+v+h/2, color);
}

static void reset_ball(int direction) {
    ball_x = SW / 2.0f - BALL_SIZE / 2.0f;
    ball_y = SH / 2.0f - BALL_SIZE / 2.0f;
    ball_vx = 3.2f * (float)direction;
    ball_vy = (ball_y > SH/2) ? -2.0f : 2.0f;
}

static void init_pad(void) {
    padInit(0);
    padPortOpen(0, 0, NULL);
}

static int pad_buttons(void) {
    struct padButtonStatus buttons;
    int state = padGetState(0, 0);
    if (state != PAD_STATE_STABLE && state != PAD_STATE_FINDCTP1)
        return 0;
    if (padRead(0, 0, &buttons) == 0)
        return 0;
    return buttons.btns;
}

static void draw_scene(void) {
    int i;
    u32 white = rgba(240, 245, 255, 0x80);
    u32 cyan = rgba(60, 220, 235, 0x80);
    u32 gold = rgba(255, 195, 65, 0x80);

    gsKit_clear(gs, rgba(8, 12, 24, 0x80));

    /* Línea central punteada */
    for (i = 12; i < (int)SH; i += 24)
        rect(SW/2-2, (float)i, SW/2+2, (float)i+12, rgba(45, 58, 78, 0x80));

    /* Marcador */
    digit(player_score, SW/2-48, 34, cyan);
    digit(cpu_score, SW/2+32, 34, gold);

    /* Paletas y pelota */
    rect(28, player_y, 28+PADDLE_W, player_y+PADDLE_H, cyan);
    rect(SW-28-PADDLE_W, cpu_y, SW-28, cpu_y+PADDLE_H, gold);
    rect(ball_x, ball_y, ball_x+BALL_SIZE, ball_y+BALL_SIZE, white);

    gsKit_queue_exec(gs);
    gsKit_sync_flip(gs);
}

int main(int argc, char *argv[]) {
    (void)argc; (void)argv;
    gs = gsKit_init_global(GS_MODE_NTSC, GS_PSM_CT24);
    if (!gs) {
        SleepThread();
        return 1;
    }
    gsKit_init_screen(gs);
    dmaKit_init(D_CTRL_RELE_OFF, D_CTRL_MFD_OFF, D_CTRL_STS_UNSPEC,
                D_CTRL_STD_UNSPEC, D_CTRL_RCYC_8, 1 << DMA_CHANNEL_GIF);
    dmaKit_chan_init(DMA_CHANNEL_GIF);
    init_pad();

    while (running) {
        int btn = pad_buttons();

        if (!(btn & PAD_UP)) player_y -= 4.4f;
        if (!(btn & PAD_DOWN)) player_y += 4.4f;
        if (!(btn & PAD_START)) {
            player_score = cpu_score = 0;
            reset_ball((ball_vx >= 0) ? 1 : -1);
        }
        if (!(btn & PAD_SELECT)) running = 0;

        if (player_y < 8) player_y = 8;
        if (player_y > SH-PADDLE_H-8) player_y = SH-PADDLE_H-8;

        /* IA sencilla: persigue la pelota con un pequeño margen de error. */
        if (cpu_y + PADDLE_H/2 < ball_y + BALL_SIZE/2 - 5) cpu_y += 2.7f;
        if (cpu_y + PADDLE_H/2 > ball_y + BALL_SIZE/2 + 5) cpu_y -= 2.7f;
        if (cpu_y < 8) cpu_y = 8;
        if (cpu_y > SH-PADDLE_H-8) cpu_y = SH-PADDLE_H-8;

        ball_x += ball_vx;
        ball_y += ball_vy;

        if (ball_y <= 6 || ball_y+BALL_SIZE >= SH-6) ball_vy = -ball_vy;

        /* Rebote contra paleta del jugador */
        if (ball_vx < 0 && ball_x <= 28+PADDLE_W &&
            ball_x+BALL_SIZE >= 28 && ball_y+BALL_SIZE >= player_y &&
            ball_y <= player_y+PADDLE_H) {
            ball_x = 28+PADDLE_W;
            ball_vx = fabsf(ball_vx) + 0.12f;
            ball_vy += ((ball_y + BALL_SIZE/2) - (player_y+PADDLE_H/2)) * 0.045f;
        }

        /* Rebote contra paleta de la máquina */
        if (ball_vx > 0 && ball_x+BALL_SIZE >= SW-28-PADDLE_W &&
            ball_x <= SW-28 && ball_y+BALL_SIZE >= cpu_y &&
            ball_y <= cpu_y+PADDLE_H) {
            ball_x = SW-28-PADDLE_W-BALL_SIZE;
            ball_vx = -(fabsf(ball_vx) + 0.08f);
            ball_vy += ((ball_y + BALL_SIZE/2) - (cpu_y+PADDLE_H/2)) * 0.04f;
        }

        if (ball_x < -BALL_SIZE) {
            cpu_score++;
            reset_ball(1);
        } else if (ball_x > SW) {
            player_score++;
            reset_ball(-1);
        }

        draw_scene();
    }

    padPortClose(0, 0);
    padEnd();
    gsKit_deinit_global(gs);
    SleepThread();
    return 0;
}

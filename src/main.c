/*
 * O Ultimo Pudim - fatia vertical para Nintendo 64 (libdragon)
 * Mesmas regras do jogo em Godot: pulo duplo, dash com stamina, 3 armas,
 * pirata patrulheiro, moedas, loja. Sprites do jogo original (PNGs da pasta assets,
 * convertidos para .sprite pelo Makefile e carregados do ROM).
 * Resolucao 320x240, logica a 60 FPS fixos.
 */
#include <libdragon.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SW 320
#define SH 240
#define TS 16
#define COLS 120
#define ROWS 15
#define MAXE 16
#define MAXC 64
#define MAXB 48

/* ---- constantes de jogo (escala do original: 1280x720 -> 320x240) ---- */
#define RUN        2.1f
#define GRAV       0.12f
#define JUMP      -3.4f
#define MAXFALL    6.0f
#define DASH_SPD   5.5f
#define DASH_FR    10
#define DASH_COST  20.0f

enum { T_EMPTY = 0, T_SOLID = 1 };
static uint8_t map[ROWS][COLS];

typedef struct { float x, y, w, h, vx, vy; bool ground, wall; } Body;
typedef struct { Body b; int facing, jumps, dash_t, dash_dir, hp, invuln, cd, weapon, money; float stamina; bool owned[3]; } Player;
typedef struct { Body b; int dir, hp; bool alive; } Enemy;
typedef struct { float x, y; int val; bool alive; } Coin;
typedef struct { float x, y, vx, vy; int dmg, life; bool alive; } Bullet;
typedef struct { const char *name; int dmg, cd; float spd; int life, pellets, cost; } Weapon;

static const Weapon W[3] = {
    { "Pistola", 10, 24, 6.0f, 50, 1,   0 },
    { "Rifle",    7, 12, 7.0f, 60, 1, 100 },
    { "Shotgun", 15, 36, 6.0f, 22, 3, 200 },
};

typedef enum { ST_PLAY, ST_DEAD, ST_WIN } State;

static Player P;
static Enemy en[MAXE];
static Coin coins[MAXC];
static Bullet bl[MAXB];
static State state = ST_PLAY;
static float camx = 0;
static int stick_y = 0;
static unsigned fc = 0;   /* contador de frames (animacao) */

/* sprites */
static sprite_t *s_idle[4], *s_walk[6], *s_jump[2], *s_pir[6];
static sprite_t *s_coin, *s_gun[3], *s_bullet, *s_tile, *s_sign, *s_altar, *s_pudim;

static sprite_t *load_spr(const char *name) {
    char path[64];
    snprintf(path, sizeof path, "rom:/%s.sprite", name);
    return sprite_load(path);
}

static void load_assets(void) {
    char n[24];
    for (int i = 0; i < 4; i++) { snprintf(n, sizeof n, "p_idle%d", i); s_idle[i] = load_spr(n); }
    for (int i = 0; i < 6; i++) { snprintf(n, sizeof n, "p_walk%d", i); s_walk[i] = load_spr(n); }
    for (int i = 0; i < 2; i++) { snprintf(n, sizeof n, "p_jump%d", i); s_jump[i] = load_spr(n); }
    for (int i = 0; i < 6; i++) { snprintf(n, sizeof n, "e_walk%d", i); s_pir[i] = load_spr(n); }
    s_coin = load_spr("coin"); s_bullet = load_spr("bullet"); s_tile = load_spr("tile");
    s_gun[0] = load_spr("gun_pistol"); s_gun[1] = load_spr("gun_rifle"); s_gun[2] = load_spr("gun_shotgun");
    s_sign = load_spr("sign"); s_altar = load_spr("altar"); s_pudim = load_spr("pudim");
}

/* ------------------------------------------------------------ nivel --- */
static void fill(int c0, int r0, int c1, int r1, uint8_t t) {
    for (int r = r0; r <= r1; r++)
        for (int c = c0; c <= c1; c++)
            map[r][c] = t;
}

static void build_level(void) {
    memset(map, 0, sizeof map);
    fill(0, 13, COLS - 1, 14, T_SOLID);       /* chao */
    fill(30, 13, 33, 14, T_EMPTY);            /* buraco 1 (4 tiles) */
    fill(60, 13, 64, 14, T_EMPTY);            /* buraco 2 (5 tiles) */
    fill(80, 11, 81, 12, T_SOLID);            /* degrau (2 tiles de altura) */
    fill(92, 12, 93, 12, T_SOLID);            /* degrau baixo */
    fill(95, 9, 97, 12, T_SOLID);             /* parede de 4 tiles: exige pulo duplo */
}

static int tile_at(int c, int r) {
    if (c < 0 || c >= COLS) return T_SOLID;
    if (r < 0 || r >= ROWS) return T_EMPTY;
    return map[r][c];
}

static bool hits(float x, float y, float w, float h) {
    int c0 = (int)floorf(x / TS), c1 = (int)floorf((x + w - 0.01f) / TS);
    int r0 = (int)floorf(y / TS), r1 = (int)floorf((y + h - 0.01f) / TS);
    for (int r = r0; r <= r1; r++)
        for (int c = c0; c <= c1; c++)
            if (tile_at(c, r) == T_SOLID) return true;
    return false;
}

static void body_move(Body *b) {
    b->wall = false;
    b->x += b->vx;
    if (hits(b->x, b->y, b->w, b->h)) {
        if (b->vx > 0) b->x = floorf((b->x + b->w - 0.01f) / TS) * TS - b->w;
        else           b->x = (floorf(b->x / TS) + 1) * TS;
        b->vx = 0;
        b->wall = true;
    }
    b->y += b->vy;
    b->ground = false;
    if (hits(b->x, b->y, b->w, b->h)) {
        if (b->vy > 0) {
            b->y = floorf((b->y + b->h - 0.01f) / TS) * TS - b->h;
            b->ground = true;
        } else {
            b->y = (floorf(b->y / TS) + 1) * TS;
        }
        b->vy = 0;
    }
}

static bool overlap(float ax, float ay, float aw, float ah, float bx, float by, float bw, float bh) {
    return ax < bx + bw && ax + aw > bx && ay < by + bh && ay + ah > by;
}

/* --------------------------------------------------------- entidades --- */
static void add_enemy(int col) {
    for (int i = 0; i < MAXE; i++) if (!en[i].alive) {
        memset(&en[i], 0, sizeof en[i]);
        en[i].b.x = col * TS; en[i].b.y = 13 * TS - 26; en[i].b.w = 12; en[i].b.h = 26;
        en[i].dir = -1; en[i].hp = 30; en[i].alive = true;
        return;
    }
}

static void add_coin(float x, float y, int val) {
    for (int i = 0; i < MAXC; i++) if (!coins[i].alive) {
        coins[i].x = x; coins[i].y = y; coins[i].val = val; coins[i].alive = true;
        return;
    }
}

static void reset_level(void) {
    memset(en, 0, sizeof en);
    memset(coins, 0, sizeof coins);
    memset(bl, 0, sizeof bl);
    build_level();
    int ecols[] = { 14, 45, 70, 88, 104 };
    for (int i = 0; i < 5; i++) add_enemy(ecols[i]);
    int ccols[] = { 10, 22, 23, 42, 43, 31, 32, 62, 63, 76, 82, 93, 100, 108, 112 };
    int crows[] = { 12, 9, 9, 9, 9, 9, 9, 10, 10, 12, 10, 10, 12, 12, 12 };
    for (int i = 0; i < 15; i++) add_coin(ccols[i] * TS + 4, crows[i] * TS + 4, 10);

    int money = P.money; bool o1 = P.owned[1], o2 = P.owned[2]; int wp = P.weapon;
    memset(&P, 0, sizeof P);
    P.b.x = 2 * TS; P.b.y = 10 * TS; P.b.w = 12; P.b.h = 28;
    P.facing = 1; P.hp = 100; P.stamina = 100;
    P.owned[0] = true; P.owned[1] = o1; P.owned[2] = o2;
    P.money = money; P.weapon = wp;   /* dinheiro e armas persistem, como no original */
    state = ST_PLAY;
}

static void shoot(void) {
    const Weapon *w = &W[P.weapon];
    float ax = (float)P.facing, ay = 0;
    if (stick_y > 40) ay = -0.7f; else if (stick_y < -40) ay = 0.7f;
    float base = atan2f(ay, ax);
    for (int n = 0; n < w->pellets; n++) {
        float a = base + (w->pellets == 3 ? (n - 1) * 0.15f : 0.0f);
        for (int i = 0; i < MAXB; i++) if (!bl[i].alive) {
            bl[i].x = P.b.x + P.b.w / 2 + cosf(a) * 6;
            bl[i].y = P.b.y + 12 + sinf(a) * 6;
            bl[i].vx = cosf(a) * w->spd; bl[i].vy = sinf(a) * w->spd;
            bl[i].dmg = w->dmg; bl[i].life = w->life; bl[i].alive = true;
            break;
        }
    }
    P.cd = w->cd;
}

static void hurt_player(int dmg) {
    if (P.invuln > 0 || state != ST_PLAY) return;
    P.hp -= dmg; P.invuln = 60; P.b.vy = -2.0f;
    if (P.hp <= 0) { P.hp = 0; state = ST_DEAD; }
}

/* ------------------------------------------------------------ update --- */
static void update(joypad_buttons_t held, joypad_buttons_t pr, joypad_inputs_t inp) {
    if (state != ST_PLAY) {
        if (pr.start) reset_level();
        return;
    }
    fc++;
    stick_y = inp.stick_y;
    int dirx = 0;
    if (held.d_left  || inp.stick_x < -20) dirx = -1;
    if (held.d_right || inp.stick_x >  20) dirx =  1;
    if (dirx) P.facing = dirx;

    if (P.b.ground) P.jumps = 2;
    if ((pr.z || pr.r) && P.stamina >= DASH_COST && dirx != 0 && P.dash_t <= 0) {
        P.stamina -= DASH_COST; P.dash_t = DASH_FR; P.dash_dir = dirx;
    }
    if (P.dash_t > 0) {
        P.dash_t--;
        P.b.vx = P.dash_dir * DASH_SPD;
        P.b.vy = 0;
    } else {
        P.b.vx = dirx * RUN;
        P.b.vy += GRAV;
        if (P.b.vy > MAXFALL) P.b.vy = MAXFALL;
    }
    if (pr.a && P.jumps > 0) { P.b.vy = JUMP; P.jumps--; P.dash_t = 0; }
    body_move(&P.b);

    P.stamina += 0.3f; if (P.stamina > 100) P.stamina = 100;
    if (P.invuln > 0) P.invuln--;
    if (P.cd > 0) P.cd--;

    if (pr.c_right) for (int i = 1; i <= 3; i++) { int w = (P.weapon + i) % 3; if (P.owned[w]) { P.weapon = w; break; } }
    if (pr.c_left)  for (int i = 1; i <= 3; i++) { int w = (P.weapon + 3 - i) % 3; if (P.owned[w]) { P.weapon = w; break; } }
    if (held.b && P.cd <= 0) shoot();

    /* loja em col 8: C-cima compra a proxima arma */
    if (pr.c_up && fabsf(P.b.x - 8 * TS) < 32) {
        for (int w = 1; w < 3; w++) if (!P.owned[w]) {
            if (P.money >= W[w].cost) { P.money -= W[w].cost; P.owned[w] = true; P.weapon = w; }
            break;
        }
    }

    if (P.b.y > ROWS * TS + 40) { P.hp = 0; state = ST_DEAD; }
    if (overlap(P.b.x, P.b.y, P.b.w, P.b.h, 115 * TS, 11 * TS, TS, 2 * TS)) state = ST_WIN;

    for (int i = 0; i < MAXC; i++)
        if (coins[i].alive && overlap(P.b.x, P.b.y, P.b.w, P.b.h, coins[i].x, coins[i].y, 8, 8)) {
            coins[i].alive = false; P.money += coins[i].val;
        }

    for (int i = 0; i < MAXE; i++) if (en[i].alive) {
        Enemy *e = &en[i];
        e->b.vx = e->dir * 0.6f;
        e->b.vy += GRAV; if (e->b.vy > MAXFALL) e->b.vy = MAXFALL;
        body_move(&e->b);
        if (e->b.wall) e->dir = -e->dir;
        else if (e->b.ground) {
            float fx = e->dir > 0 ? e->b.x + e->b.w + 1 : e->b.x - 1;
            if (!hits(fx, e->b.y + e->b.h + 1, 1, 1)) e->dir = -e->dir;
        }
        if (e->b.y > ROWS * TS + 50) e->alive = false;
        if (overlap(P.b.x, P.b.y, P.b.w, P.b.h, e->b.x, e->b.y, e->b.w, e->b.h)) hurt_player(10);
    }

    for (int i = 0; i < MAXB; i++) if (bl[i].alive) {
        Bullet *b = &bl[i];
        b->x += b->vx; b->y += b->vy;
        if (--b->life <= 0 || tile_at((int)floorf(b->x / TS), (int)floorf(b->y / TS)) == T_SOLID) { b->alive = false; continue; }
        for (int j = 0; j < MAXE; j++) if (en[j].alive &&
            overlap(b->x - 1, b->y - 1, 3, 3, en[j].b.x, en[j].b.y, en[j].b.w, en[j].b.h)) {
            en[j].hp -= b->dmg; b->alive = false;
            if (en[j].hp <= 0) { en[j].alive = false; add_coin(en[j].b.x + 2, en[j].b.y + 4, 25); }
            break;
        }
    }

    camx = P.b.x + P.b.w / 2 - SW / 2;
    if (camx < 0) camx = 0;
    if (camx > COLS * TS - SW) camx = COLS * TS - SW;
}

/* -------------------------------------------------------------- draw --- */
static void rect(float x0, float y0, float x1, float y1, color_t c) {
    int ax = (int)x0, ay = (int)y0, bx = (int)x1, by = (int)y1;
    if (ax < 0) ax = 0;
    if (ay < 0) ay = 0;
    if (bx > SW) bx = SW;
    if (by > SH) by = SH;
    if (bx <= ax || by <= ay) return;
    rdpq_set_mode_fill(c);
    rdpq_fill_rectangle(ax, ay, bx, by);
}

static void spr(sprite_t *sp, float x, float y, bool flip) {
    rdpq_sprite_blit(sp, (int)floorf(x), (int)floorf(y), &(rdpq_blitparms_t){ .flip_x = flip });
}

static void draw(void) {
    int cx = (int)camx;
    rect(0, 0, SW, 140, RGBA32(110, 190, 255, 255));
    rect(0, 140, SW, SH, RGBA32(80, 150, 230, 255));

    /* tiles: textura enviada uma vez, retangulos texturizados */
    rdpq_set_mode_standard();
    rdpq_sprite_upload(TILE0, s_tile, NULL);
    int c0 = cx / TS, c1 = c0 + SW / TS + 1;
    for (int r = 0; r < ROWS; r++)
        for (int c = c0; c <= c1 && c < COLS; c++)
            if (map[r][c] == T_SOLID)
                rdpq_texture_rectangle(TILE0, c * TS - cx, r * TS, c * TS - cx + TS, r * TS + TS, 0, 0);

    rect(8 * TS + 7 - cx, 11 * TS + 2, 8 * TS + 9 - cx, 13 * TS, RGBA32(90, 55, 25, 255));   /* poste da loja */

    rdpq_set_mode_standard();
    rdpq_mode_alphacompare(1);
    spr(s_sign, 8 * TS - 8 - cx, 10 * TS - 4, false);
    spr(s_altar, 115 * TS - cx, 11 * TS, false);
    spr(s_pudim, 115 * TS - 4 - cx, 11 * TS - 22, false);

    for (int i = 0; i < MAXC; i++) if (coins[i].alive)
        spr(s_coin, coins[i].x - 2 - cx, coins[i].y - 2, false);
    for (int i = 0; i < MAXE; i++) if (en[i].alive) {
        Enemy *e = &en[i];
        spr(s_pir[(fc / 8 + i) % 6], e->b.x + e->b.w / 2 - 16 - cx, e->b.y + e->b.h - 32, e->dir < 0);
    }
    for (int i = 0; i < MAXB; i++) if (bl[i].alive)
        spr(s_bullet, bl[i].x - 3 - cx, bl[i].y - 3, false);

    if (state != ST_DEAD && !(P.invuln > 0 && (P.invuln / 4) % 2)) {
        sprite_t *ps;
        if (!P.b.ground)        ps = s_jump[P.b.vy < 0 ? 0 : 1];
        else if (P.b.vx != 0)   ps = s_walk[(fc / 5) % 6];
        else                    ps = s_idle[(fc / 16) % 4];
        float sx = P.b.x + P.b.w / 2 - 16 - cx, sy = P.b.y + P.b.h - 32;
        bool fl = P.facing < 0;
        if (P.dash_t > 0) spr(s_walk[0], sx - P.dash_dir * 12, sy, fl);   /* rastro do dash */
        spr(ps, sx, sy, fl);
        sprite_t *g = s_gun[P.weapon];
        float gx = P.facing > 0 ? P.b.x + P.b.w / 2 + 2 : P.b.x + P.b.w / 2 - 2 - g->width;
        spr(g, gx - cx, P.b.y + 12, fl);
    }

    /* HUD */
    spr(s_coin, 8, 30, false);
    rect(8, 8, 8 + 100, 14, RGBA32(40, 0, 0, 255));
    rect(8, 8, 8 + P.hp, 14, RGBA32(220, 40, 40, 255));
    rect(8, 16, 8 + 100, 20, RGBA32(0, 0, 40, 255));
    rect(8, 16, 8 + (int)P.stamina, 20, RGBA32(60, 140, 255, 255));
    rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, 24, 40, "%d  %s", P.money, W[P.weapon].name);
    if (fabsf(P.b.x - 8 * TS) < 32) {
        for (int w = 1; w < 3; w++) if (!P.owned[w]) {
            rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, 8, 56, "C-CIMA: comprar %s ($%d)", W[w].name, W[w].cost);
            break;
        }
    }
    if (state == ST_DEAD) rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, 90, 110, "VOCE MORREU - START");
    if (state == ST_WIN)  rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, 70, 110, "FIM DA DEMO - START");
}

/* ------------------------------------------------------------- main --- */
int main(void) {
    display_init(RESOLUTION_320x240, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE);
    dfs_init(DFS_DEFAULT_LOCATION);
    rdpq_init();
    joypad_init();
    load_assets();
    rdpq_font_t *font = rdpq_font_load_builtin(FONT_BUILTIN_DEBUG_MONO);
    rdpq_text_register_font(FONT_BUILTIN_DEBUG_MONO, font);

    memset(&P, 0, sizeof P);
    reset_level();

    while (1) {
        joypad_poll();
        joypad_buttons_t held = joypad_get_buttons_held(JOYPAD_PORT_1);
        joypad_buttons_t pr   = joypad_get_buttons_pressed(JOYPAD_PORT_1);
        joypad_inputs_t  inp  = joypad_get_inputs(JOYPAD_PORT_1);
        update(held, pr, inp);

        surface_t *disp = display_get();
        rdpq_attach_clear(disp, NULL);
        draw();
        rdpq_detach_show();
    }
}

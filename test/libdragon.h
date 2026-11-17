#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
typedef struct { uint8_t r,g,b,a; } color_t;
#define RGBA32(R,G,B,A) ((color_t){R,G,B,A})
typedef struct { int dummy; } surface_t;
typedef struct { int dummy; } rdpq_font_t;
typedef struct { bool a,b,z,r,l,start,d_left,d_right,d_up,d_down,c_left,c_right,c_up,c_down; } joypad_buttons_t;
typedef struct { int8_t stick_x, stick_y; } joypad_inputs_t;
#define RESOLUTION_320x240 0
#define DEPTH_16_BPP 0
#define GAMMA_NONE 0
#define FILTERS_RESAMPLE 0
#define JOYPAD_PORT_1 0
#define FONT_BUILTIN_DEBUG_MONO 1
void display_init(int,int,int,int,int); void rdpq_init(void); void joypad_init(void);
rdpq_font_t *rdpq_font_load_builtin(int); void rdpq_text_register_font(int, rdpq_font_t*);
void joypad_poll(void);
joypad_buttons_t joypad_get_buttons_held(int); joypad_buttons_t joypad_get_buttons_pressed(int); joypad_inputs_t joypad_get_inputs(int);
surface_t *display_get(void); void rdpq_attach_clear(surface_t*, void*); void rdpq_detach_show(void);
void rdpq_set_mode_fill(color_t); void rdpq_fill_rectangle(int,int,int,int);
void rdpq_text_printf(void*, int, int, int, const char*, ...);
typedef struct { int width, height; } sprite_t;
typedef struct { bool flip_x; } rdpq_blitparms_t;
#define DFS_DEFAULT_LOCATION 0
#define TILE0 0
int dfs_init(int); sprite_t *sprite_load(const char*);
void rdpq_sprite_blit(sprite_t*, int, int, const rdpq_blitparms_t*);
void rdpq_set_mode_standard(void); void rdpq_mode_alphacompare(int);
void rdpq_sprite_upload(int, sprite_t*, void*);
void rdpq_texture_rectangle(int,int,int,int,int,int,int);

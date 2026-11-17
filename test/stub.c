#include "libdragon.h"
#include <string.h>
#include <stdio.h>
static int frame=0; static surface_t s; static rdpq_font_t f;
void display_init(int a,int b,int c,int d,int e){} void rdpq_init(void){} void joypad_init(void){}
rdpq_font_t *rdpq_font_load_builtin(int a){return &f;} void rdpq_text_register_font(int a, rdpq_font_t*b){}
void joypad_poll(void){}
/* script: anda para a direita, pula e atira o tempo todo, dash de vez em quando */
joypad_buttons_t joypad_get_buttons_held(int p){ joypad_buttons_t h; memset(&h,0,sizeof h); h.d_right=1; h.b=1; return h; }
joypad_buttons_t joypad_get_buttons_pressed(int p){ joypad_buttons_t h; memset(&h,0,sizeof h); if(frame%50==0) h.a=1; if(frame%50==8) h.a=1; if(frame%120==30) h.z=1; if(frame%300==299) h.c_right=1; return h; }
joypad_inputs_t joypad_get_inputs(int p){ joypad_inputs_t i={0,0}; return i; }
surface_t *display_get(void){ return &s; }
void rdpq_attach_clear(surface_t*a, void*b){} 
long px=0; void rdpq_detach_show(void){ if(++frame>=3600){ printf("fim: 3600 frames sem crash\n"); exit(0);} }
void rdpq_set_mode_fill(color_t c){} 
void rdpq_fill_rectangle(int a,int b,int c,int d){ if(a<0||b<0||c>320||d>240||c<=a||d<=b){printf("RETANGULO INVALIDO %d %d %d %d\n",a,b,c,d); exit(1);} }
#include <stdarg.h>
void rdpq_text_printf(void*p,int f,int x,int y,const char*fmt,...){ if(frame%600==0){ va_list ap; va_start(ap,fmt); printf("[f%d] ",frame); vprintf(fmt,ap); printf("\n"); va_end(ap);} }
#include <stdlib.h>
static sprite_t pool[64]; static int np=0;
int dfs_init(int a){return 0;}
sprite_t *sprite_load(const char*p){
  char path[128]; snprintf(path,sizeof path,"assets/%s",p+5); char *dot=strstr(path,".sprite"); strcpy(dot,".png");
  FILE*f=fopen(path,"rb"); if(!f){printf("ASSET FALTANDO: %s\n",path); exit(2);} 
  fseek(f,16,SEEK_SET); unsigned char b[8]; fread(b,1,8,f); fclose(f);
  sprite_t*s=&pool[np++]; s->width=(b[0]<<24)|(b[1]<<16)|(b[2]<<8)|b[3]; s->height=(b[4]<<24)|(b[5]<<16)|(b[6]<<8)|b[7];
  if(s->width>32||s->height>32){printf("SPRITE GRANDE DEMAIS %s %dx%d\n",path,s->width,s->height);exit(3);} return s; }
void rdpq_sprite_blit(sprite_t*s,int x,int y,const rdpq_blitparms_t*p){ if(!s){printf("sprite nulo\n");exit(4);} }
void rdpq_set_mode_standard(void){} void rdpq_mode_alphacompare(int a){} void rdpq_sprite_upload(int t,sprite_t*s,void*p){ if(!s){printf("tile nulo\n");exit(4);} }
void rdpq_texture_rectangle(int t,int a,int b,int c,int d,int e,int f){ if(c<=a||d<=b){printf("texrect invalido\n");exit(5);} }

#define main game_main
#include "../src/main.c"
#undef main
#include <stdio.h>
int main(void){
  load_assets(); memset(&P,0,sizeof P); reset_level();
  int jumped=0;
  for(int f=0; f<8000 && state==ST_PLAY; f++){
    joypad_buttons_t h={0}, pr={0}; joypad_inputs_t in={0,0};
    h.d_right=1; h.b=1;
    int col=(int)((P.b.x+P.b.w+14)/TS), feet=(int)((P.b.y+P.b.h)/TS);
    int gap = tile_at(col,feet)==T_EMPTY && tile_at(col,feet+1)==T_EMPTY;
    int wall= tile_at(col,feet-1)==T_SOLID || tile_at((int)((P.b.x+P.b.w+4)/TS),feet-1)==T_SOLID;
    if(P.b.ground && (gap||wall)) { pr.a=1; jumped++; }
    else if(!P.b.ground && P.jumps>0 && P.b.vy>0.5f && (wall||gap) ) pr.a=1;
    if(P.b.y/TS>11 && !P.b.ground) { }
    update(h,pr,in);
  }
  printf("estado=%s x=%.0f/%d hp=%d dinheiro=%d pulos=%d\n", state==ST_WIN?"VITORIA":state==ST_DEAD?"MORTO":"JOGANDO", P.b.x, COLS*TS, P.hp, P.money, jumped);
  return 0; }

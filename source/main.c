#include <tonc.h>

#include "map1.h"

// This file is autogenerated from the file in the graphics folder
#include "back1.h"
#include "mapa1.h"
#include "sha.h"

#define UP 0
#define LEFT 3
#define DOWN 2
#define RIGHT 1
#define IDLE 4

#define JUMP_SPEED 8

#define GRAVITY 1

void load_back1(void);
void load_mapa1(void);
void update_sprite(void);
void update_physics(void);

s16 xOffset =0;
s16 yOffset =80;
u32 frame;
u8 anim;
s32 x=20;
s32 y=34;
s8 dx;
s8 dy;
bool isJumping;


OBJ_ATTR obj_buffer[128];
OBJ_AFFINE *obj_aff_buffer = (OBJ_AFFINE*)obj_buffer;

void load_back1(void){
	memcpy32(&tile_mem[1][0], back1Tiles, back1TilesLen / 4);
	memcpy32(&se_mem[31][0], back1Map, back1MapLen / 4);
}

void load_mapa1(void){
	memcpy32(&tile_mem[0][0], mapa1Tiles, mapa1TilesLen / 4);
	memcpy32(&se_mem[30][0], mapa1Map, mapa1MapLen / 4);
	memcpy32(pal_bg_mem, back1Pal, back1PalLen / 4);

}

void load_sprite(void)
{
	// Load tiles and palette of sprite into video and palete RAM. Note that
	// regular memcpy() won't work. To write to VRAM, it is required to write in
	// units of 16 or 32 bytes. memcpy() doesn't guarantee the size of the
	// writes, but mempcy32() always writes in chunks of 32 bytes.
	memcpy32(&tile_mem[4][0], shaTiles, shaTilesLen / 4);
	memcpy32(pal_obj_mem, shaPal, shaPalLen / 4);

	oam_init(obj_buffer, 128);

	OBJ_ATTR *sha = &obj_buffer[0];
	obj_set_attr(sha,
		ATTR0_SQUARE,  // Square, regular sprite
		ATTR1_SIZE_32, // 32x32 pixels,
		ATTR2_PALBANK(0) | 0); // palette index 0, tile index 0

	// Set position
	obj_set_pos(sha, 40, 117);

	oam_copy(oam_mem, obj_buffer, 1); // Update first OAM object
}

void update_sprite(void)
{
	OBJ_ATTR *sha = &obj_buffer[0];
	obj_set_attr(sha,
		ATTR0_SQUARE,  // Square, regular sprite
		ATTR1_SIZE_32, // 32x32 pixels,
		ATTR2_PALBANK(0) | ((anim*48)+(frame%3)*16)); // palette index 0, tile index (calculated)
	obj_set_pos(sha, x, y);

	oam_copy(oam_mem, obj_buffer, 1);
}


void update_physics(void){
	dx=key_tri_horz();
	y+=dy;
	dy+=GRAVITY;
	if(key_hit(KEY_A)){
		//if(!isJumping){
			isJumping=true;
			dy=-JUMP_SPEED;
		//}
	}
	
	s8 dtile=0;
	
	
	s16 tile_x=(x+xOffset/8);
	s16 tile_y=(y/8)+4;

	if(collision_map1[tile_y*40+tile_x]!=1){
		isJumping=false;
		dy=0;
	}
	
	if(dx>0){
		anim=RIGHT;
		if(x>140){
			xOffset++;
		}
	}else if(dx<0){
		anim=LEFT;
		if(x<20){
			xOffset--;
		}
	}else{
		anim=IDLE;
	}

	if(anim==RIGHT){
		dtile=3;
	}

	if(anim==LEFT){
		dtile=-1;
	}
	if(collision_map1[tile_y*40+tile_x+dtile]!=1){
		dx=0;
	}
	x+=dx;
}
int main(void)
{
	irq_init(NULL);
	irq_enable(II_VBLANK);
	anim=IDLE;
	frame=0;
	dx=0;
	dy=0;
	isJumping=false;
	load_back1();
	load_mapa1();
	REG_BG1CNT = BG_CBB(1) | BG_SBB(31) | BG_8BPP | BG_REG_32x32;
	REG_BG0CNT = BG_CBB(0) | BG_SBB(30) | BG_8BPP | BG_REG_32x32;
	load_sprite();

	REG_DISPCNT = DCNT_MODE0 | DCNT_BG0 | DCNT_BG1| DCNT_OBJ | DCNT_OBJ_1D;
	
	while (1) {
		VBlankIntrWait();
		key_poll();

		update_physics();
		REG_BG0HOFS = xOffset;
		REG_BG0VOFS = yOffset;
		
		update_sprite();
		frame++; 
	}
}

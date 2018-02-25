#ifndef _SPRITE_H
#define _SPRITE_H 1

#include <allegro.h>

class sprite
{
private:
public:
	int alive;
	int state;
	int objecttype;
	int direction;
	double x, y;
	int width, height;
	double velx, vely, speed;
	int xdelay, ydelay;
	int xcount, ycount;
	int curframe, totalframes, animdir;
	int framecount, framedelay;
	int animcolumns;
	int animstartx, animstarty;
	int faceAngle, moveAngle;
	BITMAP *image;


public:
	sprite();
	~sprite();
	int load(char *filename);
	void draw(BITMAP *dest);
	void drawframe(BITMAP *dest);
	void updatePosition();
	void updateAnimation();
	int inside(int x, int y, int left, int top, int right, int bottom);
	int pointInside(int px, int py);
	int collided(sprite *other, int shrink);
};
#endif

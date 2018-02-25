/* 
 Title: sprite.cpp
 Description: A class used to handle various sprite functions
 Date: May 22, 2015
 Author: Jason Bishop
 Student #: 3042012
 Version: 1.0
*/

/*
 DOCUMENTATION
 
  Purpose: The sprite class defined here allows for manipulation of sprites within the game.
           sprites can be updated in various ways, such as position and animation. 
     
 Notes: Based on code from the textbook Game Programming All In One (Chapter 9).
 
 Classes: 
     sprite - stores and updates information about individual sprites in game

 Variables: 
     image - used to draw the sprite image
     alive - indicates status of sprite within game
     direction - direction of sprite
     animcolumns, animstartx, animstarty, curframe, totalframes, framecount, framedelay, animdir
         - for controlling sprite animation
     x, y - position of sprite on screen
     width, height - sprite dimensions
     xdelay, ydelay, velx, vely, speed - used to control sprite movement
     faceAngle, moveAngle - for controlling angle of sprite
        
*/


#include <allegro.h>
#include "sprite.h"


// constructor
sprite::sprite()
{
	image = NULL;
	alive = 1;
	direction = 0;
	animcolumns = 0;
	animstartx = 0;
	animstarty = 0;
	x = 0.0f;
	y = 0.0f;
	width = 0;
	height = 0;
	xdelay = 0;
	ydelay = 0;
	xcount = 0;
	ycount = 0;
	velx = 0;
	vely = 0;
	speed = 0.0;
	curframe = 0;
	totalframes = 1;
	framecount = 0;
	framedelay = 10;
	animdir = 1;
	faceAngle = 0;
	moveAngle = 0;
}  // end of constructor

// destructor
sprite::~sprite()
{
	// remove bitmap from memory
	if (image != NULL)
	{
		destroy_bitmap(image);
	}
}  // end of destructor


// loads a bitmap file to be associated with the sprite
// returns 0 for failure, 1 for success
int sprite::load(char *filename)
{
	image = load_bitmap(filename, NULL);
	if (image == NULL)  return 0;
	width = image->w;
	height = image->h;

	return 1;
}  // end of load(char *)


// draws sprite to the given bitmap
void sprite::draw(BITMAP *dest)
{
	draw_sprite(dest, image, (int)x, (int)y);
}  // end of draw(BITMAP *)


// draws frame of animated sprite to given bitmap
void sprite::drawframe(BITMAP *dest)
{
	int fx = animstartx + (curframe % animcolumns) * width;
	int fy = animstarty + (curframe / animcolumns) * height;
	masked_blit(image, dest, fx, fy, (int)x, (int)y, width, height);
}  // end of drawframe(BITMAP *)


// updates the position of the sprite
void sprite::updatePosition()
{
	// update x position
	if (++xcount > xdelay)
	{
		xcount = 0;
		x += velx;
	}

	// update y position
	if (++ycount > ydelay)
	{
		ycount = 0;
		y += vely;
	}  // end of if
}  // end of updatePosition()

// **updated for explosion animation
// updates the animation sequence of the sprite
void sprite::updateAnimation()
{
	// update frame based on animdir
	if (++framecount > framedelay)
	{
		framecount = 0;
		curframe += animdir;

		if (curframe < 0)
		{
			if (alive == 0) {  // **updated for explosion animation
				curframe = 0;
				animdir = 1;
			}
			else { curframe = totalframes - 1; }
		}
		if (curframe > totalframes - 1)
		{
			if (alive == 0) {  // **updated for explosion animation
				curframe = totalframes - 1;
				animdir = -1;
			}
			else { curframe = 0; }
		}  // end of if
	}  // end of if
}  // end of updateAnimation()


// checks if a point is inside a given area
// returns 1 if within, 0 if not
int sprite::inside(int x, int y, int left, int top, int right, int bottom)
{
	if (x > left && x < right && y > top && y < bottom)
		return 1;
	else
		return 0;
}  // end of inside(int, int, int, int, int, int)


// checks if point is inside the sprite boundaries
int sprite::pointInside(int px, int py)
{
	return inside(px, py, (int)x, (int)y, (int)x + width, (int)y + height);
}  // end of pointInside(int, int)


// checks if sprite coordinates overlap (collision)
// uses shrink value to reduce collision detection boundaries
int sprite::collided(sprite *other, int shrink)
{
	int wa = (int)x + width;
	int ha = (int)y + height;
	int wb = (int)other->x + other->width;
	int hb = (int)other->y + other->height;

	if (inside((int)x, (int)y, (int)other->x + shrink,
		(int)other->y + shrink, wb - shrink, hb - shrink) ||
		inside((int)x, ha, (int)other->x + shrink,
		(int)other->y + shrink, wb - shrink, hb - shrink) ||
		inside(wa, (int)y, (int)other->x + shrink,
		(int)other->y + shrink, wb - shrink, hb - shrink) ||
		inside(wa, ha, (int)other->x + shrink,
		(int)other->y + shrink, wb - shrink, hb - shrink))
		return 1;
	else
		return 0;
}  // end of collided(sprite *, int

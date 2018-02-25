/* 
 Title: bullets.cpp
 Description: Classes to handle on-screen projectiles
 Date: May 23, 2015
 Author: Jason Bishop
 Student #: 3042012
 Version: 1.0
*/

/*
 DOCUMENTATION
 
  Purpose:
    The classes here are used to store the information for individual projectiles in the game.
    Parameters for position and veleocity, collision bounds and more are stored in individual
    Bullet classes.  Each bullet class is stored within an array used by the BulletHandler class.
    The BulletHandler class allows for creation, deletion, update, etc. of individual bullets.
     
 Notes: Angular velocity calculations from the textbook Game Programming All In One (Chapter 10).
        BulletHandler is an adaptation from the spriteHandler class of the textbook (Chapter 9).
 
 Classes: 
     Bullet - used to store individual bullet paramters used in the game
     BulletHandler - used to store and access a collection of Bullets

 Variables: 
     Bullet - x, y - coordinates of bullet
            - angle - angle of bullet movement
            - velx, vely - movement rate of bullet in x/y directions
            - active - determines if bullet is active on screen
            - colx, coly, colw, colh - bullet collision paramaters
            
     BulletHandler - count - number of bullets stored in the handler
                   - images[] - stores different bullet images
                   - bullets[] - stores individual Bullets
        
*/


#include "bullets.h"
#include <math.h>
#include <allegro.h>

#define PI 3.1415926535
#define WIDTH 640
#define HEIGHT 480
#define SIZE 25
#define WHITE makecol(255, 255, 255)


// constructor
Bullet::Bullet(double x, double y, double angle, int type) {
                      
	this->x = x;
	this->y = y;
	this->type = type;
	this->angle = angle;
	velx = calcAngleMoveX(angle);
	vely = calcAngleMoveY(angle);
	active = true;
	if (type == 1) {
             
		colx = 0;
		coly = 10;
		colw = 25;
		colh = 10;
		velx *= 10;
		vely *= 10;
	}
	
	else {
	
    	colx = 1;
		coly = 1;
		colw = 23;
		colh = 23;
		if (velx > 0) { velx += 2; }
		else { velx -=2; }
		if (vely > 0) { vely += 2; }
		else { vely -=2; }
	}  // end of else
}  // end of constructor


// adjust bullet position
void Bullet::move() {

	x += velx;
	y += vely;
}  // end of method move()


// stores collision paramaters in the array of the passed pointer
void Bullet::getCollisionParamaters(int *params) {

		params[0] = (int)x + colx;
		params[1] = (int)y - coly;
		params[2] = (int)x + (colx + colw);
		params[3] = (int)y - (coly + colh);
}  // end of getCollisionParamters(int *)


// constructor
bulletHandler::bulletHandler() {
	
	count = 0;
	for (int i = 0; i < MAXBULLETS; ++i) {

        bullets[i] = NULL;
    }

	BITMAP *sheet = load_bitmap("images/BulletHell.bmp", NULL);
	if (!sheet) {

        allegro_message("Error loading bullet sprites");
    }

    else {
		
		for (int i = 0; i < 3; ++i) {
            
			images[i] = create_bitmap(SIZE, SIZE);
			clear_bitmap(images[i]);
			blit(sheet, images[i], SIZE * i, 0, 0, 0, SIZE, SIZE);
		}

		//delete sheet;
	}  // end of else
}  // end of constructor


// destructor
bulletHandler::~bulletHandler() {

	int temp = 0;
	for (int i = 0; i < MAXBULLETS; ++i) {
		
		if (i < 3) {
			
            delete images[i];
		}

		if (!isNull(i) && temp <= count) {
			
			del(i);
			++temp;
		}  // end of if
	}  // end of for
}  // end of destructor


// creates and stores new Bullet class into array, returns position in array
int bulletHandler::create(double x, double y, double angle, int type) {
	
	for (int i = 0; i < MAXBULLETS; ++i) {

		if (bullets[i] == NULL) {

			bullets[i] = new Bullet(x, y, angle, type);
			++count;
			return i;
		}  // end of if
	}  // end of for

	return -1;
}  // end of create(double, double, double, int)


// checks if the passed position contains a bullet/NULL
bool bulletHandler::isNull(int index) {

	if (bullets[index] != NULL) {
		
		return false;
	}
	
	else {
		
		return true;
	}  // end of else
}  // end of isNull(int)


// deletes the Bullet stored at the given index
void bulletHandler::del(int index) {

	if (bullets[index] != NULL) {
		
        delete bullets[index];
		bullets[index] = NULL;
		--count;
	}  // end of if
}  // end of del(int)


// returns the Bullet stored at the given index
Bullet *bulletHandler::get(int index) {

	 return bullets[index]; 

}  // end of get(int)


// updates active bullets, removes inactive bullets, draws active bullets to the given bitmap
void bulletHandler::updateAll(BITMAP *buff) {
	
	int temp = 0;
	for (int i = 0; i < MAXBULLETS; ++i) {
		
		// update position and draw if active
		if (bullets[i] != NULL && temp <= count) {

			bullets[i]->move();
			if (bullets[i]->getActive()) {
                
                masked_blit(images[bullets[i]->getType()], buff, 0, 0,
                            (int)bullets[i]->getX(), (int)bullets[i]->getY(), SIZE, SIZE);
            }
            
            // delete if inactive
            else {
            
                 del(i);
            }
            
            // remove bullets if exceeding screen bounds
			if (bullets[i] != NULL && (bullets[i]->getX() > WIDTH
                || bullets[i]->getX() + SIZE < 160)) {
				
				del(i);
			}

			if (bullets[i] != NULL && (bullets[i]->getY() > HEIGHT
                || bullets[i]->getY() + SIZE < 0)) {
				
				del(i);
			}  // end of if
		}  // end of if
    }  // end of for
}  // end of updateAll(BITMAP *)


// calculate angular velocity in x direction
double calcAngleMoveX(double angle) {

	return (double) cos(angle * PI / 180);
}  // end of calcAngleMoveX(double)

// calculate angular velocity in x direction
double calcAngleMoveY(double angle) {

	return (double) sin(angle * PI / 180);
}  // end of calcAngleMoveY(double)

// calculates the angle of clockwise rotation of x-axis for 2 points
double calcAngle(int x1, int y1, int x2, int y2) {

	int xdiff = x2 - x1;
	int ydiff = y2 - y1;

	// angle of clockwise rotation of x-axis
	return atan2(ydiff, xdiff) * (180 / PI) + 180;
}  // end of calcAngle(int, int, int, int)

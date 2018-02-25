/* 
 Title: spritehandler.cpp
 Description: A class used to handle a set of sprites
 Date: May 22, 2015
 Author: Jason Bishop
 Student #: 3042012
 Version: 1.0
*/

/*
 DOCUMENTATION
 
  Purpose: The class defined here stores sprites in an array for easier access and manipulation. 
     
 Notes: Based on code from the textbook Game Programming All In One (Chapter 9).
 
 Classes: 
     spriteHandler - used to store and manipulate individual sprites

 Variables: 
     count - total number of sprites stored
     sprite *[] - array to hold individual sprites
        
*/


#include "spritehandler.h"


// constructor
spriteHandler::spriteHandler(void) {
	count = 0;
}  // end of constructor


// destructor
spriteHandler::~spriteHandler(void) {
	// delete the sprites
	for (int n = 0; n < count; n++) {

        delete sprites[n];

    }
}  // end of destructor


// adds passed sprite into array
void spriteHandler::add(sprite *spr) {
	if (spr != NULL) {
		sprites[count] = spr;
	}
	count++;
}  // end of add(sprite *)


// creates new sprite in array without having to create and pass in another function
void spriteHandler::create() {
	sprites[count] = new sprite();
	count++;
}  // end of create()


// returns pointer to sprite at given index
sprite *spriteHandler::get(int index) {
	return sprites[index];
}  // end of get(int)

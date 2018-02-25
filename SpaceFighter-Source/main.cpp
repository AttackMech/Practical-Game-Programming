/* 
 Title: main.cpp
 Description: A space based vertical shooter
 Date: May 22, 2015
 Author: Jason Bishop
 Student #: 3042012
 Version: 1.1
*/

/*
 DOCUMENTATION
 
 Program Purpose:
    This program creates a vertical shooting game.  It loads sprites for the player and each of
    the enemies.  A sprite and spritehandler class handle the movement and presentation of each
    sprite.  The user can move the player around the screen with the arrow keys and is limited
    in movement to the size of the screen, but cannot exceed a minimum height.  The user can fire
    projectiles from their sprite using the spacebar.  Each projectile is drawn and handled with
    a bullet and bullethandler class.  Enemies on the screen are destroyed when they collide with
    the player or an on screen projectile.  Enemy movement is relativle simple:  Depending on the
    enemy type, it will move in a horizontal/vertical line, or bounce from side to side while 
    moving down the screen.  Music plays in the background, and there are sound effects for
    projectiles, enemies, and enemy destruction.

    *UPDATE 1.1*

    The game now features a new game screen.  The play field has been shortened to a 480 x 480
    square.  This is on the right-hand portion of the screen and it features a scrolling
    background created in Mappy.  On the left portion of the screen (160 x 480), there is a
    bitmap featuring the player health and score.  Each UFO destroyed is worth 100 points, and
    each alien ship is worth 200 points.  The game will display "GAME OVER" when the player
    runs out of health, which occurs after 10 collisions with enemies.

    *UPDATE 1.2*
    
    The game now implements new AI technology in addition to randomness.  The original enemies use
    tracking to fire bullets at the player.  Additionally, a new bug enemy type is introduced that
    will track the player position and fire a beam weapon for 2 seconds.  The bug enemy needs 2
    hits to destroy, and being hit by the beam weapon depletes more of the player's health.  In
    addition to new AI, the program also features multi-threading.  A seperate thread is created
    and continuously executed to perform collision checking.

     
 Notes: The sprite and sprite handler classes are copied from those provided in the textbook Game
        Programming All In One (Chapter 9).  Other code is based on that provided in the same
        textbook.
        Changes from the previous version have been indicated by comments
 
 Classes: See individual .cpp files for descriptions of classes used in this program.  They are
          sprite.cpp, spritehandler.cpp and bullets.cpp.

 Variables: 
     
     fire, fireRate, bullet_time - used to limit the fire rate of the player
     bulletParams[] - used to store location of bullets used in collision detection
     spriteHandler - used to store and update sprites
     bulletHandler - used to story and update bullets
     samples[] - stores samples for music and sound effects
     voices[] - stores the voice allocated for each sample
     volume1, volume2, volume3 - used to control volume of individual voices
     alien_time, ufo_time - used to control when new enemies appear
     time_now - tracks the current time for use with other time variables

     *UPDATE 1.1*
     health - tracks player health, a value of 0 and the player loses a life
     lives - tracks the number of player lives, a value of 0 and the game is over
     score - tracks the player score
     mapoffset - map position variable allowing the background to scroll
     player_timer, flash_time - controls the time the player is dead and has no control over
                                their sprite

    *UPDATE 1.2*
    bug_timer - control when new bug enemy appears
    bug_fire_timer, bugfire-  controls the bugs firing time
    bughealth - controls how many hits it takes to destroy the bug enemy
    done, threadsafe - controls the execution of threads
    ufo_go, alien_go, bug_go - allows variable timings for enemies to reappear

        
*/

/*
 TEST PLAN
    See the attached files on the cover page for this assignment.
*/
 
 
#include <stdio.h>
#include <allegro.h>
#include <winalleg.h>  //**updated
#include <pthread.h>  //**updated
#include "sprite.h"
#include "spritehandler.h"
#include "bullets.h"
#include "mappyal.h"

#define MODE GFX_AUTODETECT_WINDOWED
#define LEFT 160
#define WIDTH 480
#define SCRWIDTH 640
#define HEIGHT 480
#define WHITE makecol(255, 255, 255)
#define BLACK makecol(0, 0, 0)


bool fire;
int bulletParams [4];
int fireRate = 300;
spriteHandler *sprites;
bulletHandler *bullets;
SAMPLE *samples[7];
int voices[7];
int volume1 = 64;
int volume2 = 128;
int volume3 = 255;
clock_t player_timer, flash_time, bullet_time, ufo_timer, alien_timer, bug_timer,
        bug_fire_timer, time_now;  //**updated
int health, lives, bughealth;  //**updated
long score, mapoffset;
bool bugfire, done;  //**updated
int ufo_go, alien_go, bug_go;  //**updated

// create a new thread mutex to protect variables
pthread_mutex_t threadsafe = PTHREAD_MUTEX_INITIALIZER;

// loads and checks samples, as well as allocating voices for each sample    
bool loadSamples() {
    
    samples[0] = load_sample("sounds/Yaataaru.wav");  // music
    samples[1] = load_sample("sounds/laser01.wav");  // sound effects
    samples[2] = load_sample("sounds/laser02.wav");
    samples[3] = load_sample("sounds/explosion.wav");
    samples[4] = load_sample("sounds/UFOsound.wav");
    samples[5] = load_sample("sounds/flyby.wav");
    samples[6] = load_sample("sounds/bump.wav");

    for (int i = 0; i < sizeof(samples)/sizeof(samples[0]); ++i) {
        
        // check that samples are loaded
        if (samples[i] == NULL) {
            return false;
        }
        
        // allocate voices and check
        voices[i] = allocate_voice(samples[i]);
        if (voices[i] == -1) {
            return false;
        }  // end of if

        voice_set_priority(voices[0], 7 - i);
    }  // end of for

    // set voice levels and begin music playback
    voice_set_volume(voices[0], volume1);
    voice_set_volume(voices[1], volume2);
    voice_set_volume(voices[2], volume3);
    voice_set_volume(voices[3], volume2);
    voice_set_volume(voices[4], volume2);
    voice_set_volume(voices[5], volume1);
    voice_set_volume(voices[6], volume3);
    
    voice_set_playmode(voices[0], PLAYMODE_LOOP);
    voice_set_playmode(voices[2], PLAYMODE_LOOP);
    voice_set_playmode(voices[4], PLAYMODE_LOOP);    

    return true;
} // end of function loadSamples()

// check input from the user and take appropriate action
// when there is no input it will control the time until control returns to the player
void checkInput(void) {
    
    sprite *player = sprites->get(0);

    if (player->alive) { 

        // move up
        if (key[KEY_UP]) {
            player->vely = -5;
            player->animstartx = 0;
        }
        
        // move down
        else if (key[KEY_DOWN]) {
            player->vely = 5;
            player->animstartx = 0;
        }
        
        // move left
        else if (key[KEY_LEFT]) {
            player->velx = -5;
            player->animstartx = player->width * 2;
        }
        
        // move right
        else if (key[KEY_RIGHT]) {
            player->velx = 5;
            player->animstartx = player->width * 4;
        }
        
        // set animation to default
        else {
            player->animstartx = 0;
        }
        
        // fire projectiles
        if (key[KEY_SPACE]) {
            
            // check to limit fire rate
            if (time_now - bullet_time >= fireRate) {
                fire = true;
                bullet_time = time_now;
            }
            
            // create projectile on screen and play sound
            if (fire) {
            
                sprite *player = sprites->get(0);
                bullets->create(player->x + 22.5, player->y, 270, 1);
                fire = false;
                voice_stop(voices[1]);
                voice_set_position(voices[1], 0);
                voice_start(voices[1]);
            }  // end of if
        }  // end of if
    }  // end of if
    else {
        if (time_now - player_timer > 4000) {
            
            player->alive = 1;
            health = 10;
        }
        if (time_now - player_timer > 2000) {
            
            player->x = WIDTH / 2 + LEFT - player->width / 2;
            player->y = HEIGHT - player->height;
            player->animstarty = 0;
            player->animstartx = 0;
            player->velx = 0;
            player->vely = 0;
            player->curframe = 0;
            player->framecount = 0;
            player->totalframes = 2;

        }  // end of if
    }  // end of else
}  // end of function checkInput()


// keeps player within screen bounds
void checkPlayerBounds() {
     
    sprite *player = sprites->get(0);
    
    // check horizontal
    if (player->x < LEFT) {
        player->x = LEFT;
    }
    
    if (player->x > SCRWIDTH - player->width) {
        player->x = SCRWIDTH - player->width;
    }
    
    // check vertical
    if (player->y < 70) {
        player->y = 70;
    }
    
    if (player->y > HEIGHT - player->height) {
        player->y = HEIGHT - player->height;
    }  // end of if

    // check if still alive
    if (!health) {
        player->alive = 0;
    }
}  // end of function checkPlayerBounds()


// checks for collisions between individal sprites and with projectiles
void collisionCheck() {

    Bullet *bullTemp;
    sprite *sprTemp;
    sprite *player = sprites->get(0);
    
    for (int i = 1; i < sprites->size(); ++i) {
    
        sprTemp = sprites->get(i);
        
        //**update
        // check if player collide with beam weapon
        if(sprTemp->objecttype == 3 && bugfire) {
           
            if ((int)player->x + player->width - 12 > (int)sprTemp->x + 35
                && (int)player->x + 12 < (int)sprTemp->x + sprTemp->width - 35)
            {                
                health -= 3;
                bug_fire_timer -= 3000;
                voice_stop(voices[2]);
                voice_set_position(voices[2], 0);
                voice_stop(voices[6]);
                voice_set_position(voices[6], 0);
                voice_start(voices[6]);
            }
        }

        if (sprTemp->alive == 1) {
            
            // check for enemy collision with player
            if (player->collided(sprTemp, 0)) {

                sprTemp->alive = 0;
                if (sprTemp->objecttype == 1) {

                    ufo_timer = time_now;
                    voice_stop(voices[4]);
                    voice_set_position(voices[4], 0);
                    sprTemp->animstarty = sprTemp->height + 1;
                    sprTemp->velx = 0;
                    sprTemp->vely = 4;
                    sprTemp->curframe = 0;
                    sprTemp->framecount = 0;
                    --health;
                }

                else if (sprTemp->objecttype == 2) {
                    alien_timer = time_now;
                    voice_stop(voices[5]);
                    voice_set_position(voices[5], 0);
                    sprTemp->animstarty = sprTemp->height + 1;
                    sprTemp->animstartx = 0;
                    sprTemp->velx = 0;
                    sprTemp->vely = 4;
                    sprTemp->curframe = 0;
                    sprTemp->framecount = 0;
                    sprTemp->totalframes = 4;
                    --health;
                }

                else if (sprTemp->objecttype == 3) {
                            
                    bug_timer = time_now;
                    bugfire = false;
                    voice_stop(voices[2]);
                    voice_set_position(voices[2], 0);
                    sprTemp->animstarty = sprTemp->height * 3 + 1;
                    sprTemp->animstartx = 0;
                    sprTemp->velx = 0;
                    sprTemp->vely = 4;
                    sprTemp->curframe = 0;
                    sprTemp->framecount = 0;
                    sprTemp->totalframes = 5;
                    sprTemp->animdir = 1;
                    health -= 2;
                }
                
                voice_stop(voices[3]);
                voice_set_position(voices[3], 0);
                voice_start(voices[3]);
            }

            // check for collisions with projectiles
            for (int j = 0; j < 100; ++j) {

                bullTemp = bullets->get(j);

                if (bullTemp != NULL  && bullTemp->getType() == 1) {

                    bullTemp->getCollisionParamaters(bulletParams);

                    if (sprTemp->pointInside(bulletParams[0], bulletParams[1]) 
                        || sprTemp->pointInside(bulletParams[0], bulletParams[3])
                        || sprTemp->pointInside(bulletParams[2], bulletParams[1])
                        || sprTemp->pointInside(bulletParams[2], bulletParams[3]))
                    {
                        bullets->del(j);
                        sprTemp->alive = 0;
                
                        if (sprTemp->objecttype == 1) {
                                                
                            ufo_timer = time_now;
                            voice_stop(voices[4]);
                            voice_set_position(voices[4], 0);
                            sprTemp->animstarty = sprTemp->height + 1;
                            sprTemp->velx = 0;
                            sprTemp->vely = 4;
                            sprTemp->curframe = 0;
                            sprTemp->framecount = 0;
                            score += 100;
                        }
                        
                        else if (sprTemp->objecttype == 2) {
                        
                            alien_timer = time_now;
                            voice_stop(voices[5]);
                            voice_set_position(voices[5], 0);
                            sprTemp->animstarty = sprTemp->height + 1;
                            sprTemp->animstartx = 0;
                            sprTemp->velx = 0;
                            sprTemp->vely = 4;
                            sprTemp->curframe = 0;
                            sprTemp->framecount = 0;
                            sprTemp->totalframes = 5;
                            score += 200;
                        }

                        //**updated for new enemy type
                        else if (sprTemp->objecttype == 3) {
                            
                            --bughealth;
                            
                            if(!bughealth) {
                                bug_timer = time_now;
                                bugfire = false;
                                voice_stop(voices[2]);
                                voice_set_position(voices[2], 0);
                                sprTemp->animstarty = sprTemp->height * 3;
                                sprTemp->animstartx = 0;
                                sprTemp->velx = 0;
                                sprTemp->vely = 4;
                                sprTemp->curframe = 0;
                                sprTemp->framecount = 0;
                                sprTemp->totalframes = 5;
                                score += 300;
                            }
                            else { sprTemp->alive = 1;}
                        }
                        
                        voice_stop(voices[3]);
                        voice_set_position(voices[3], 0);
                        voice_start(voices[3]);
                    }  // end of if
                }  // end of if

                //**update
                else if (bullTemp != NULL  && (bullTemp->getType() == 0 || bullTemp->getType() == 2)) {

                    bullTemp->getCollisionParamaters(bulletParams);

                    if (player->pointInside(bulletParams[0], bulletParams[1]) 
                        || player->pointInside(bulletParams[0], bulletParams[3])
                        || player->pointInside(bulletParams[2], bulletParams[1])
                        || player->pointInside(bulletParams[2], bulletParams[3]))
                    {
                        bullets->del(j);
                        --health;
                        voice_stop(voices[6]);
                        voice_set_position(voices[6], 0);
                        voice_start(voices[6]);
                    }  // end of if
                }  // end of else if
            }  // end of for
        }  // end of if
    }  //  end of for

    if (health <= 0 && player->alive) {

        player->alive = 0;
        player->animstarty = player->height + 1;
        player->animstartx = 0;
        player->velx = 0;
        player->vely = 4;
        player->curframe = 0;
        player->framecount = 0;
        player->totalframes = 5;
        --lives;
        player_timer = time_now;
        flash_time = time_now + 2000;
    }  // end of if
}  // end of function collisionCheck()


// creates new enemy with random paramaters
void newEnemy(sprite *enemy) {
    
    enemy->animstarty = 0;
    
    // create new ufo type
    if (enemy->objecttype == 1) {

        enemy->x = rand() % ((SCRWIDTH - enemy->width) - LEFT + 1) + LEFT;
        enemy->y = 1 - enemy->height;
        enemy->vely = rand() % 3 + 1;
        if (rand() % 2 == 1) {
                   
            enemy->velx = rand() % 3 + 1;
        }
        
        else {
             
            enemy->velx = -(rand() % 3 + 1);
        }
        voice_start(voices[4]);
    }
    
    // create new alien type in one of 3 directions
    else if (enemy->objecttype == 2) {
         
        int direction = rand() % 4 + 1;
        switch (direction) {
               
            case 1:  // horizontal left movement
                
                enemy->animstartx = enemy->width * 2;
                enemy->totalframes = 2;
                enemy->x = LEFT - enemy->width;
                enemy->y = rand() % ((3 * HEIGHT / 4 - enemy->height) - 30 + 1) + 30;
                enemy->velx = rand() % 3 + 4;
                enemy->vely = 0;
                break;

            case 2: // horizontal right movement

                enemy->animstartx = enemy->width * 4;
                enemy->totalframes = 2;
                enemy->x = SCRWIDTH - 1;
                enemy->y = rand() % ((3 * HEIGHT / 4 - enemy->height) - 30 + 1) + 30;
                enemy->velx = -(rand() % 3 + 4);
                enemy->vely = 0;
                break;

            default: // vertical movement
               
                enemy->animstartx = 0;
                enemy->totalframes = 2;
                enemy->x = rand() % ((SCRWIDTH - enemy->width) - LEFT + 1) + LEFT;
                enemy->y = 1 - enemy->height;
                enemy->velx = 0;
                enemy->vely = rand() % 2 + 4;
                break;
        }
        voice_stop(voices[5]);
        voice_set_position(voices[5], 0);
        voice_start(voices[5]);
    }

    //**updated to handle new enemy type
    else if (enemy->objecttype == 3) {

        enemy->animstartx = 0;
        enemy->totalframes = 1;
        enemy->x = 0;
        enemy->y = 0 - enemy->height;
        enemy->velx = 0;
        enemy->vely = 1;
        bughealth = 2;
    }

    enemy->alive = 1;
}  // end of function newEnemy()


// keeps enemies within position paramaters or deactivates enemies when no longer on screen
void updateEnemies() {
    
    int try_fire;
    sprite *temp;
    sprite *player;
    player = sprites->get(0);

    for (int i = 1; i < sprites->size(); ++i) {
    
        temp = sprites->get(i);
        
        if (temp->objecttype == 2) {
            
            if (temp->alive == 0) {
    
                if (time_now - alien_timer > 1000) {
    
                    newEnemy(temp);
                }  // end of if
            }  // end of if
            
            else {
            
                if (temp->x + temp->width < LEFT || temp->x > SCRWIDTH
                    || temp->y > HEIGHT)
                {
                    temp->alive = 0;
                    alien_timer = time_now;
                }  // end of if

                //**update
                try_fire = rand() % 100 + 1;
                if (try_fire == 1) {
                    double new_angle = calcAngle((int)player->x, (int)player->y, (int)temp->x, (int)temp->y);
                    bullets->create(temp->x + temp->width / 2, temp->y + temp->height / 2, new_angle, 2);
                }
            }  // end of else
        }  // end of if
        
        else if (temp->objecttype == 1) {
            
            if (temp->alive == 0) {
        
                if (time_now - ufo_timer > 1500) {
                 
                    newEnemy(temp);
                }  // end of fi
            }  // end of if
            
            else {
            
                if (temp->x < LEFT) {
            
                    temp->x = LEFT;
                    temp->velx = -(temp->velx);
                }
                
                if (temp->x + temp->width > SCRWIDTH) {
                
                    temp->x = SCRWIDTH - temp->width;
                    temp->velx = -(temp->velx);
                }
                
                if (temp->y > HEIGHT) {

                    temp->alive = 0;
                    voice_stop(voices[4]);
                    voice_set_position(voices[4], 0);
                    ufo_timer = time_now;
                }  // end of if

                //**update
                try_fire = rand() % 300 + 1;
                if (try_fire == 1) {
                    double new_angle = calcAngle((int)player->x, (int)player->y, (int)temp->x, (int)temp->y);
                    bullets->create(temp->x + temp->width / 2, temp->y + temp->height / 2, new_angle, 0);
                    }
            }  // end of else
        }  // end of else if

        //**updated to handle new enemy
        else if (temp->objecttype == 3) {
            
            if (temp->alive == 0) {

                if (time_now - bug_timer > 1000) {

                    newEnemy(temp);
                }
            }

            else {
                
                // movement when bug is not firing
                if (!bugfire) {

                    if (!temp->velx && temp->vely > 0) {  // track player position

                        temp->x = player->x;
                    }

                    else {

                        // kill bug when no longer on screen
                        if (temp->x > SCRWIDTH || temp->x + temp->width < LEFT) {
                            
                            temp->alive = 0;
                            bug_timer = time_now;
                        }
                    }
                }

                // actions when bug is firing
                else {

                    // stop firing after time and begin to move off screen
                    if (time_now - bug_fire_timer > 2000) {
                        
                        bugfire = false;
                        temp->vely = 1;
                        
                        if (temp->x > (SCRWIDTH + LEFT) / 2) {

                            temp->velx = 2;
                        }
                        else {
                            temp->velx = -2;
                        }

                        temp->animstartx = temp->width * 2 + 1;
                        temp->animstarty = temp->height * 2 + 1;
                        temp->totalframes = 1;
                        temp->curframe = 0;

                        voice_stop(voices[2]);
                        voice_set_position(voices[2], 0);

                    }

                }

                // check ig bug in position and animate before firing
                if (!temp->velx && temp->y == 0) {

                    temp->y = 1;
                    temp->vely = 0;
                    temp->animstartx = temp->width + 1;
                    temp->animstarty = 0;
                    temp->totalframes = 4;
                    temp->curframe = 0;
                    temp->animdir = 1;
                    temp->framedelay = 15;
                }

                // begin to fire after loading animation
                else if (!temp->velx && temp->y > 0 && temp->curframe == 3) {

                    temp->animstartx = 0;
                    temp->animstarty = temp->height * 2;
                    temp->totalframes = 2;
                    temp->curframe = 0;
                    bugfire = true;
                    bug_fire_timer = time_now;

                    voice_start(voices[2]);
                }

            }  // end of else
        }  // end of else if
    }  // end of for
}  // end of function updateEnemies()


// rest callback function to update time while resting
void rest1(void) {
     
     time_now = clock();
}  // end of function rest1()

//**updated
// this thread constantly does collision checking
void* thread0(void* data) {

    // get thread id
    int my_thread_id = *((int*)data);

    // thread main loop
    while (!done) {

        // lock mutex to protect variables
        if (pthread_mutex_lock(&threadsafe))
            allegro_message("error locking thread mutex");

        // check collisions and boundaries
        checkPlayerBounds();
        collisionCheck();

        // unlock the mutex
        if (pthread_mutex_unlock(&threadsafe))
            allegro_message("error unlocking mutex");

    }

    // terminate thread
    pthread_exit(NULL);
    
    return NULL;
}



// main
int main (void) {

    //**updated
    // set thread id
    int id;
    pthread_t pthread0;
    int threadid0 = 0;

    // initialize allegro routines 
    allegro_init();
    set_color_depth(24);
    set_gfx_mode(MODE, SCRWIDTH, HEIGHT, 0, 0);  // sets windowed mode for display
    install_keyboard();
    set_keyboard_rate(500, 500);
    install_timer();
    srand (time(NULL));  // random number seed
    
    // install and load sounds/music
    reserve_voices(7, 0);
    if (install_sound(DIGI_AUTODETECT, MIDI_NONE, "") != 0) {

        allegro_message("Error initializing sound system");
        return 1;
    }
    
    if (!loadSamples()) {
                        
        allegro_message("Error loading sound files");
        return 1;
    }
    

    // create back buffer
    BITMAP *buffer;
    buffer = create_bitmap(SCRWIDTH, HEIGHT);
    clear_bitmap(buffer);

    // load background
    BITMAP *scorebg;
    scorebg = load_bitmap("images/Gamescreen.bmp", NULL);
    if (!scorebg) {
        allegro_message("Error loading background");
        return 2;
    }   

    // load icon to represent player lives
    BITMAP *life;
    life = load_bitmap("images/miniguy.bmp", NULL);
    if (!life) {
        allegro_message("Error loading player life");
        return 2;
    } 
    
    // load Mappy file
    if (MapLoad("images/finalmap.FMP") != 0) {
       set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
       allegro_message("Can't find background image");
       return 2;
   }

    // load handler classes
    sprites = new spriteHandler();
    bullets = new bulletHandler();

    // player sprite
    sprite *player;
    player = new sprite();
    if (!player->load("images/Main-ex.bmp")) {
                                          
        allegro_message("Error loading sprite file.");
        return 2;
    }

    player->objecttype = 0;
    player->width = 70;
    player->height = 70;
    player->x = WIDTH / 2 + LEFT - player->width / 2;
    player->y = HEIGHT - player->height;
    player->animcolumns = 6;
    player->curframe = 0;
    player->totalframes = 2;
    player->animdir = 1;
    player->alive = 1;

    sprites->add(player);

    // enemy UFO sprite
    sprite *ufo;
    ufo = new sprite();
    if (!ufo->load("images/UFO-ex.bmp")) {
                                      
        allegro_message("Error loading sprite file.");
        return 2;
    }
    
    ufo->objecttype = 1;
    ufo->width = 50;
    ufo->height = 50;
    ufo->animcolumns = 4;
    ufo->curframe = 0;
    ufo->totalframes = 4;
    ufo->framedelay = 5;
    ufo->animdir = 1;
    ufo->alive = 0;

    sprites->add(ufo);

    // enemy alien ship sprite
    sprite *alien;
    alien = new sprite();
    if (!alien->load("images/Alien-ex.bmp")) {
                                          
        allegro_message("Error loading sprite file.");
        return 2;
    }
    
    alien->objecttype = 2;
    alien->width = 70;
    alien->height = 70;
    alien->animcolumns = 6;
    alien->curframe = 0;
    alien->totalframes = 2;
    alien->animdir = 1;
    alien->alive = 0;

    sprites->add(alien);

    //**updated new enemy type
    //enemy bug sprite
    sprite *bug;
    bug = new sprite();
    if (!bug->load("images/Bug-ex.bmp")) {
                                          
        allegro_message("Error loading sprite file.");
        return 2;
    }
    
    bug->objecttype = 3;
    bug->width = 88;
    bug->height = 88;
    bug->animcolumns = 3;
    bug->curframe = 0;
    bug->totalframes = 1;
    bug->animdir = 1;
    bug->alive = 0;

    sprites->add(bug);

    // load image for bug beam weapon
    BITMAP *beam;
    beam = load_bitmap("images/lazeboltr.bmp", NULL);
    if (!beam) {
        allegro_message("Error loading image");
        return 2;
    } 

    
    // set timers
    ufo_timer = clock();
    alien_timer = clock();
    bug_timer = clock();
    bullet_time = clock();
    fire = true;

    // set player and map variables
    score = 0;
    health = 10;
    lives = 3;
    mapoffset = 48000 - HEIGHT;

    //**updated
    // set enemy respawn rates
    ufo_go = 2000;
    alien_go = 1000;
    bug_go = 3000;

    // start music
    voice_start(voices[0]);

    //**updated
    // create the thread for collision checking
    id = pthread_create(&pthread0, NULL, thread0, (void*)&threadid0);


    // game loop
    while (!key[KEY_ESC]) {

        //**updated
        // lock mutex to protect variables
        pthread_mutex_lock(&threadsafe);

        // draw scrolling background        
        MapDrawBG(buffer, 0, mapoffset, LEFT, 0, 480, 480);

        // play when player still has health
        if (lives) {

            //update position of main sprite based on keyboard input
            checkInput();

            // update bullet positions
            bullets->updateAll(buffer);
            
            // update sprite animations and positions
            for (int i = 0; i < sprites->size(); ++i) {
                
                sprite *temp = sprites->get(i);
                temp->updateAnimation();
                temp->updatePosition();
            }  // end of for

            //**updated - this section has been moved to thread0
            /*// check collisions and boundaries
            checkPlayerBounds();
            collisionCheck();*/

            // draw sprites to buffer
            for (int i = sprites->size() - 1; i >= 0 ; --i) {

                sprite *temp = sprites->get(i);

                // flashes player before control returns
                if (i == 0 && temp->alive == 0) {

                    if (time_now - flash_time <= 200) {
                    
                        temp->drawframe(buffer);
                    }
                    else if (time_now - flash_time > 400) {
                    
                        flash_time = time_now;
                    } // end of else if
                }  // end of if
                else {

                    temp->drawframe(buffer);
                    
                    //**updated
                    // draws the beam weapon used in the game
                    if (temp->objecttype == 3 && bugfire) {
                    
                        masked_blit(beam, buffer, 0, 0, (int)temp->x + 35, (int)temp->y + 68, beam->w, beam->h);
                    }
                }  // end of else
            }  // end of for
            
            // check enemy bounds and validity
            updateEnemies();
            
            // reset player velocity to keep in position when user has no input
            player->vely = 0;
            player->velx = 0;
            if (health < 0) { health = 0;}
        }  //end of health check for play

        // display game over andn stop ufo sound when player lives are 0
        if(!lives) {
            textout_centre_ex(buffer, font, "GAME OVER", (SCRWIDTH + LEFT) / 2, HEIGHT / 2, WHITE, -1);
            voice_stop(voices[4]);

            //**updated
            // kill thread to stop collision checking
            done = true;
        }

        // update player info on screen
        blit(scorebg, buffer, 0, 0, 0, 0, LEFT, HEIGHT);
        textprintf_centre_ex(buffer, font, 80, 135, BLACK, -1, "%i", score);  // score
        textprintf_centre_ex(buffer, font, 78, 133, WHITE, -1, "%i", score);
        rectfill(buffer, 128, 217, 128 - 10 * (10 - health), 227, BLACK);  // health

        // lives
        switch (lives) {
            case 3:
                masked_blit(life, buffer, 0, 0, 20, 297, life->w, life->h);
                masked_blit(life, buffer, 0, 0, 60, 297, life->w, life->h);
                masked_blit(life, buffer, 0, 0, 102, 297, life->w, life->h);
                break;
            case 2:
                masked_blit(life, buffer, 0, 0, 20, 297, life->w, life->h);
                masked_blit(life, buffer, 0, 0, 60, 297, life->w, life->h);
                break;
            case 1:
                masked_blit(life, buffer, 0, 0, 20, 297, life->w, life->h);
                break;
        }
        
        // update screen
        acquire_screen();
            blit(buffer, screen, 0, 0, 0, 0, SCRWIDTH, HEIGHT);
        release_screen();
        
        // slow down game
        rest_callback(10, rest1);

        // update map offset to allow scrolling
        mapoffset -= 3;
        
        // stop map from scrolling past the top
        if(mapoffset < 0) {
            
            mapoffset = 0;
        }
        else if (mapoffset < 2 * (48000 - HEIGHT) / 3 && ufo_go > 1000) {
            ufo_go /= 2;
            alien_go /= 2;
            bug_go /=2;
        }
        else if (mapoffset < (48000 - HEIGHT) / 3 && ufo_go > 500) {
            ufo_go /= 2;
            alien_go /= 2;
            bug_go /=2;
        }

        //**updated
        // unlock the mutex
        pthread_mutex_unlock(&threadsafe);

    }  // end of game loop

    

    // release resources
    pthread_mutex_destroy(&threadsafe);
    
    destroy_bitmap(scorebg);
    destroy_bitmap(buffer);
    MapFreeMem();

    delete player;
    delete ufo;
    delete alien;
    delete bug;
    delete bullets;

    for (int i = 0; i < sizeof(samples) / sizeof(samples[0]); ++i){
        
        release_voice(voices[i]);
        destroy_sample(samples[i]);
    }
    
    remove_sound();
    allegro_exit();
    return 0;
}  // end of main
END_OF_MAIN()
     

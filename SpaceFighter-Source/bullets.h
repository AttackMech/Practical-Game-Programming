#ifndef _BULLETS_H_
#define _BULLETS_H_
#include <allegro.h>
#define MAXBULLETS 100

class Bullet {
	
	private:
		double x, y, angle, velx, vely;
		int type;
		bool active;
		int colx, coly, colw, colh;

	public:
		Bullet(double x, double y, double angle, int type);
		double getX() { return x; }
		double getY() { return y; }
		int getType() { return type; }
		bool getActive() { return active; }
		void deactivate() { active = false; }
		void move();
		void getCollisionParamaters(int *params);

};


class bulletHandler {
	private:
		int count;
		BITMAP *images[3];
		Bullet *bullets[MAXBULLETS];
	public:
		bulletHandler();
		~bulletHandler();
		//int size() { return count; }
		int create(double x, double y, double angle, int type);
		bool isNull(int index);
		void del(int index);
		Bullet *get(int index);
		void updateAll(BITMAP *buff);
};

double calcAngleMoveX(double angle);

double calcAngleMoveY(double angle);

double calcAngle(int x1, int y1, int x2, int y2);

#endif  // _BULLETS_H_

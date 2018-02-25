#pragma once
#include "sprite.h"

class spriteHandler {

	private:
		int count;
		sprite *sprites[100];

	public:
		spriteHandler(void);
		~spriteHandler(void);
		void add(sprite *spr);
		void create();
		sprite *get(int index);
		int size() { return count; }

};

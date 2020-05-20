#pragma once

#include "2DPhysics.h"
#include "GraphicsStructs.h"

double tickrate = 128.0;

float gravity = 9.81f/tickrate/8;
float jump_speed = 100.f/tickrate/1.5;
float move_speed = 20.f/tickrate;

float projectile_speed = 40.f/tickrate;
float projectile_lifetime = 3.f;

const int number_of_targets = 2;
unsigned initial_health = 20;

const unsigned amount_map_boxes = 9;//9;
const unsigned amount_map_boxes_projectiles = 12;
Box2D map_boxes[] = {
	{{0.000000, -1.000000}, {24.000000, 0.000000}},			//bottom left
	{{40.000000, -1.000000}, {64.000000, 0.000000}},		//bottom right
	{{12.000000, 9.000000}, {29.000000, 10.000000}},		//top left
	{{35.000000, 9.000000}, {52.000000, 10.000000}},		//top right
	{{28.000000, 3.500000}, {36.000000, 4.500000}},			//middle center
	{{7.000000, 3.500000}, {9.000000, 4.500000}},			//middle left
	{{55.000000, 3.500000}, {57.000000, 4.500000}},
	{{19.000000, 0.000000}, {20.000000, 2.500000}},
	{{44.000000, 0.000000}, {45.000000, 2.500000}},

	{{-1.000000, 0.000000}, {1.000000, 32.000000}},
	{{64.000000, 0.000000}, {65.000000, 32.000000}},
	{{0.000000, 32.000000}, {64.000000, 33.000000}},
};

bool f = false;
bool h = false;
bool t = false;

struct Game{
	Game(){}

	bool lost = false;

	struct Player{
		Player(){}

		Box2D box;
		C2 position, head;
		C2 speed;
		bool airborne;
		

		bool isCollidingAlready[amount_map_boxes] = {};





		bool bottom_collision = false;

		void collided(Box2D* object_collided){
			bool 
				TOP = this->box.topright.y == object_collided->bottomleft.y,
				BOTTOM = this->box.bottomleft.y == object_collided->topright.y,
				RIGHT = this->box.topright.x == object_collided->bottomleft.x,
				LEFT = this->box.bottomleft.x == object_collided->topright.x
			;

			if(BOTTOM)bottom_collision = true;

			//TODO: if it's corner, do not stop
			//if((TOP && LEFT) || (LEFT && BOTTOM) || (BOTTOM && RIGHT) || (RIGHT && TOP))return;

			if((LEFT && this->speed.x < 0) || (RIGHT && this->speed.x > 0))			{this->speed.x = 0.f; }
			else if((BOTTOM && this->speed.y < 0) || (TOP && this->speed.y > 0))	{this->speed.y = 0.f; }

		}



		bool Tick_CheckSpeedMultiplier(double m, Box2D& box){
			C2d newspeed_d = {this->speed};
			newspeed_d *= m;

			C2 newspeed = {(float)newspeed_d.x, (float)newspeed_d.y};


			Box2D newbox = this->box;
			newbox.topright += newspeed;
			newbox.bottomleft += newspeed;

			//printf("init bottomleft x: %.30f\n", this->box.bottomleft.x);
			//printf("init bottomleft y: %.30f\n", this->box.bottomleft.y);
			//printf("new bottomleft x:  %.30f\n", newbox.bottomleft.x);
			//printf("new bottomleft y:  %.30f\n", newbox.bottomleft.y);
			//printf("initial downwards: %.30f\n", this->speed.y);
			//printf("updated downwards: %.30f\n", newspeed.y);
			//printf("vecmul:            %.30f\n", (float)m);

			if(!newbox.collides(box))return false;
			//printf("collides\n");
			if(signbit((float)m))return false;
			//printf("sign bit check passed\n");

			return true;

		}

		void BeforeTick(){
			bottom_collision = false;
		}

		void AfterTick(){
			airborne = !bottom_collision;
		}

		void Tick(){

			/////////////////////////////////////////////////////////////////////////printf("(%f, %.40f), (%f, %.18f)\n", this->box.bottomleft.x, this->box.bottomleft.y, this->speed.x, this->speed.y);
			
			// // // // // // // // // // // // // // // // // // // // // if is airborne apply gravity 
			//if(this->airborne)	
				//speed.y -= 9.81f/tickrate/20.f;
			


			// // // // // // // // // // // // // // // // // // // // // check for collisions when applying the current speed. Register for each axis, and which map section it's touching on the respective axis.
			Box2D* collided_entity = 0;
			double distance_to_collision = 1.f;

			for (unsigned i = 0; i < amount_map_boxes; ++i){

				if(this->isCollidingAlready[i]){
					if(this->box.collides(map_boxes[i])){
						this->collided(&map_boxes[i]);
					} else {
						this->isCollidingAlready[i] = false;
						continue;
					}
				}

				bool h, v;
				C2d vecmul = WhenWillBoxesCollide(this->box, map_boxes[i], this->speed, {0, 0}, &h, &v);
				//printf("%d :: %llf, %llf\n", i, vecmul.x, vecmul.y);

				if(vecmul.x >= 0.0 && vecmul.x < 1.0 && vecmul.x < distance_to_collision) {
					if(Tick_CheckSpeedMultiplier(vecmul.x, map_boxes[i])){
						distance_to_collision = vecmul.x;
						collided_entity = &map_boxes[i];
					}
				}

				if(vecmul.y >= 0.0 && vecmul.y < 1.0 && vecmul.y < distance_to_collision) {
					if(Tick_CheckSpeedMultiplier(vecmul.y, map_boxes[i])){
						distance_to_collision = vecmul.y;
						collided_entity = &map_boxes[i];
					}
				}

			}


			// // // // // // // // // // // // // // // // // // // // // change the speed and update the positions, keep in mind that if there is no collision - no changes will be made to the speed
			if(collided_entity){
				//printf("stomped_stopped\n");

				C2d newspeed_d = {this->speed};
				//newspeed = this->speed;
				newspeed_d *= distance_to_collision;
				

				C2 newspeed = {(float)newspeed_d.x, (float)newspeed_d.y};
				

				this->position += newspeed;
				this->box.topright += newspeed;
				this->box.bottomleft += newspeed;
				this->head = this->position; this->head.y += 1.5;

				this->isCollidingAlready[collided_entity-map_boxes] = true;

				this->collided(collided_entity);
			}else{
				this->position += this->speed;
				this->box.topright += this->speed;
				this->box.bottomleft += this->speed;
				this->head = this->position; this->head.y += 1.5;
				//
				//this->didntCollide();
			}
		}
	};



	struct Projectile{
		Projectile(){}

		Projectile* prev = this;
		Projectile* next = 0;

		Box2D box;
		C2 position;
		C2 speed;
		
		float lifetime = 0.f;

		bool isCollidingAlready[amount_map_boxes] = {};



		void collided(Box2D* object_collided){
			bool 
				TOP = this->box.topright.y == object_collided->bottomleft.y,
				BOTTOM = this->box.bottomleft.y == object_collided->topright.y,
				RIGHT = this->box.topright.x == object_collided->bottomleft.x,
				LEFT = this->box.bottomleft.x == object_collided->topright.x
			;


			//TODO: if it's corner, do not stop
			//if((TOP && LEFT) || (LEFT && BOTTOM) || (BOTTOM && RIGHT) || (RIGHT && TOP))return;

			if((LEFT && this->speed.x < 0) || (RIGHT && this->speed.x > 0))			{this->speed.x = -this->speed.x; this->speed *= 0.9f;}
			else if((BOTTOM && this->speed.y < 0) || (TOP && this->speed.y > 0))	{this->speed.y = -this->speed.y; this->speed *= 0.9f;}

		}



		bool Tick_CheckSpeedMultiplier(double m, Box2D& box){
			C2d newspeed_d = {this->speed};
			newspeed_d *= m;

			C2 newspeed = {(float)newspeed_d.x, (float)newspeed_d.y};


			Box2D newbox = this->box;
			newbox.topright += newspeed;
			newbox.bottomleft += newspeed;

			//printf("init bottomleft x: %.30f\n", this->box.bottomleft.x);
			//printf("init bottomleft y: %.30f\n", this->box.bottomleft.y);
			//printf("new bottomleft x:  %.30f\n", newbox.bottomleft.x);
			//printf("new bottomleft y:  %.30f\n", newbox.bottomleft.y);
			//printf("initial downwards: %.30f\n", this->speed.y);
			//printf("updated downwards: %.30f\n", newspeed.y);
			//printf("vecmul:            %.30f\n", (float)m);

			if(!newbox.collides(box))return false;
			//printf("collides\n");
			if(signbit((float)m))return false;
			//printf("sign bit check passed\n");

			return true;

		}

		void BeforeTick(){
		}

		void AfterTick(){
		}

		void Tick(){

			/////////////////////////////////////////////////////////////////////////printf("(%f, %.40f), (%f, %.18f)\n", this->box.bottomleft.x, this->box.bottomleft.y, this->speed.x, this->speed.y);
			
			// // // // // // // // // // // // // // // // // // // // // if is airborne apply gravity 
			//if(this->airborne)	
				//speed.y -= 9.81f/tickrate/20.f;
			


			// // // // // // // // // // // // // // // // // // // // // check for collisions when applying the current speed. Register for each axis, and which map section it's touching on the respective axis.
			Box2D* collided_entity = 0;
			double distance_to_collision = 1.f;

			for (unsigned i = 0; i < amount_map_boxes_projectiles; ++i){

				if(this->isCollidingAlready[i]){
					if(this->box.collides(map_boxes[i])){
						this->collided(&map_boxes[i]);
					} else {
						this->isCollidingAlready[i] = false;
						continue;
					}
				}

				bool h, v;
				C2d vecmul = WhenWillBoxesCollide(this->box, map_boxes[i], this->speed, {0, 0}, &h, &v);
				//printf("%d :: %llf, %llf\n", i, vecmul.x, vecmul.y);

				if(vecmul.x >= 0.0 && vecmul.x < 1.0 && vecmul.x < distance_to_collision) {
					if(Tick_CheckSpeedMultiplier(vecmul.x, map_boxes[i])){
						distance_to_collision = vecmul.x;
						collided_entity = &map_boxes[i];
					}
				}

				if(vecmul.y >= 0.0 && vecmul.y < 1.0 && vecmul.y < distance_to_collision) {
					if(Tick_CheckSpeedMultiplier(vecmul.y, map_boxes[i])){
						distance_to_collision = vecmul.y;
						collided_entity = &map_boxes[i];
					}
				}

			}


			// // // // // // // // // // // // // // // // // // // // // change the speed and update the positions, keep in mind that if there is no collision - no changes will be made to the speed
			if(collided_entity){
				//printf("stomped_stopped\n");

				C2d newspeed_d = {this->speed};
				//newspeed = this->speed;
				newspeed_d *= distance_to_collision;
				

				C2 newspeed = {(float)newspeed_d.x, (float)newspeed_d.y};
				

				this->position += newspeed;
				this->box.topright += newspeed;
				this->box.bottomleft += newspeed;

				this->isCollidingAlready[collided_entity-map_boxes] = true;

				this->collided(collided_entity);
			}else{
				this->position += this->speed;
				this->box.topright += this->speed;
				this->box.bottomleft += this->speed;
				//
				//this->didntCollide();
			}
		}
	};


	struct MouseWorldVector{
		C2 vec;
		C2& calculate(Player* player, int screen_width, int screen_height, int m_x, int m_y){
			float distance_to_z0 = 40.f;

			float world_top    = player->head.y + distance_to_z0;
			float world_bottom = player->head.y - distance_to_z0;

			float ratio = float(screen_width)/float(screen_height);

			float world_left  = player->head.x - (distance_to_z0*ratio);
			float world_right = player->head.x + (distance_to_z0*ratio);

			//printf("(%f, %f):(%f, %f)\n", world_left, world_bottom, world_right, world_top);

			this->vec.x = float(m_x) * ((world_right-world_left)/float(screen_width))  + world_left;
			this->vec.y = float(screen_height-m_y) * ((world_top-world_bottom)/float(screen_height)) + world_bottom;

			this->vec.x -= player->head.x;
			this->vec.y -= player->head.y;

			this->vec.normalize();

			return this->vec;
		}
	};

	struct Target{
		float lifetime = 0.f;
		float time_to_live;
		C2 position;
		Box2D box;
	};



	bool TargetCollidesWithMap(Target* target){
		for (int i = 0; i < amount_map_boxes; ++i){
			if(target->box.collides(map_boxes[i]))
				return true;
		}
		return false;
	}

	void ReplaceTarget(Target* target){
		do{
			target->position.x = float(1+rand()%63);
			target->position.y = float(1+rand()%31);
			target->box = {
				{target->position.x-0.5f, target->position.y-0.5f}
				,
				{target->position.x+0.5f, target->position.y+0.5f}
			};

			//new time to live will be current difficulty plus the leftover time from before
		}while(TargetCollidesWithMap(target));
		target->time_to_live = this->difficulty;
		target->lifetime = 0.f;
	}

	void ResetTargets(){
		srand(this->seed);

		for (unsigned i = 0; i < number_of_targets; ++i){
			this->targets[i].lifetime = 0.f;
			this->targets[i].time_to_live = 0.f;
			ReplaceTarget(&this->targets[i]);
		}
	}


	void SpawnProjectile(){
		C2 spawn_position = this->player.head;
		//C2 spawn_position = this->mwp.vec;
		//spawn_position *= 3.f;
		//spawn_position += this->player.head;

		C2 spawn_speed = this->mwp.vec;
		spawn_speed *= projectile_speed;
		spawn_speed += this->player.speed;


		Projectile* newproj = new Projectile();

		newproj->position = spawn_position;
		newproj->box = {
			{newproj->position.x-0.25f, newproj->position.y-0.25f}
			, 
			{newproj->position.x+0.25f, newproj->position.y+0.25f}
		};
		newproj->speed = spawn_speed;

		if(!this->projectile_list){
			this->projectile_list = newproj;
		}else{
			newproj->prev = this->projectile_list->prev;
			this->projectile_list->prev->next = newproj;
			this->projectile_list->prev = newproj;
		}
	}

	void DespawnProjectile(Projectile* projectile){

		//replace prev's next (only if we're not the first, 
		//because if so our prev points to the last, and we 
		//don't want the last pointing to anything)
		if(projectile != this->projectile_list)
			projectile->prev->next = projectile->next;

		//replace next's prev
		if(projectile->next)
			projectile->next->prev = projectile->prev;

		//replace first node's prev if it's the last one
		if(this->projectile_list->prev == projectile){
			this->projectile_list->prev = projectile->prev;
		}

		//replace projectile_list if it's the first one
		if(this->projectile_list == projectile)
			this->projectile_list = projectile->next;

		delete projectile;
	}


	void DeleteAllProjectiles(){
		if(this->projectile_list){
			for (Projectile* projectile = this->projectile_list; projectile;){
				Projectile* old = projectile;
				projectile = projectile->next;
				delete old;
			}
		}
		this->projectile_list = 0;
	}

	void Reset(){
		DeleteAllProjectiles();
		ResetTargets();
		player.box = {{0, 0}, {2, 2}};
		player.box = {{31, 4.5}, {33, 6.5}};//(3.281250, 0.988575458526611330), (0.000000, -1.255390167236328100)
		player.position = {(player.box.bottomleft.x + player.box.topright.x) / 2.f, player.box.bottomleft.y};
		player.speed = {0, 0};//1.f/4.f};
		player.airborne = false;	
		difficulty = game_difficulty;
		score = 0;
		health = initial_health;
	}

	unsigned seed = 0xDEADBABE;

	Projectile* projectile_list;
	Player player;
	MouseWorldVector mwp;
	Target targets[number_of_targets] = {};

	float difficulty = 10.f;       //this is easiest difficulty, as this number gets lower the difficulty gets harder
	float game_difficulty = 10.f;  //the difficulty to which `difficulty` variable will be resset if the player dies
	unsigned score = 0;
	unsigned health = initial_health;
};

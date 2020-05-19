#pragma once

#include "2DPhysics.h"
#include "GraphicsStructs.h"

double tickrate = 128.0;

float gravity = 9.81f/tickrate/8;
float jump_speed = 100.f/tickrate/1.5;
float move_speed = 20.f/tickrate;

const unsigned amount_map_boxes = 9;//9;
Box2D map_boxes[] = {
	{{12.000000, 9.000000}, {29.000000, 10.000000}},		//top left
	{{35.000000, 9.000000}, {52.000000, 10.000000}},		//top right
	{{0.000000, -1.000000}, {24.000000, 0.000000}},			//bottom left
	{{40.000000, -1.000000}, {64.000000, 0.000000}},		//bottom right
	{{28.000000, 3.500000}, {36.000000, 4.500000}},			//middle center
	{{7.000000, 3.500000}, {9.000000, 4.500000}},			//middle left
	{{55.000000, 3.500000}, {57.000000, 4.500000}},
	{{19.000000, 0.000000}, {20.000000, 2.500000}},
	{{44.000000, 0.000000}, {45.000000, 2.500000}}
};

bool f = false;
bool h = false;
bool t = false;

struct Game{
	Game(){}
	

	struct Player{
		Player(){}

		Box2D box;
		C2 position;
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

			printf("(%f, %.40f), (%f, %.18f)\n", this->box.bottomleft.x, this->box.bottomleft.y, this->speed.x, this->speed.y);
			
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

				C2d vecmul = WhenWillBoxesCollide(this->box, map_boxes[i], this->speed, {0, 0});
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

	Player player;
};

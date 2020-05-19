#pragma once


#include "GraphicsStructs.h"

#include <cstdio>
#include <cmath>

struct Box2D{
	Box2D(){}
	Box2D(C2 bottomleft, C2 topright) : bottomleft(bottomleft), topright(topright){}
	Box2D(C2 bottomleft, C2 topright, float friction) : bottomleft(bottomleft), topright(topright), friction(friction){}
	
	C2 bottomleft, topright;
	float friction;



	bool isInsideRangeX(Box2D& o){
		float highest_x = (o.topright.x > this->topright.x)?o.topright.x:this->topright.x;
		float lowest_x = (o.bottomleft.x < this->bottomleft.x)?o.bottomleft.x:this->bottomleft.x;
		return (this->topright.x-this->bottomleft.x) + (o.topright.x-o.bottomleft.x) >= (highest_x - lowest_x);
	}


	bool isInsideRangeY(Box2D& o){
		float highest_y = (o.topright.y > this->topright.y)?o.topright.y:this->topright.y;
		float lowest_y = (o.bottomleft.y < this->bottomleft.y)?o.bottomleft.y:this->bottomleft.y;
		return (this->topright.y-this->bottomleft.y) + (o.topright.y-o.bottomleft.y) >= (highest_y - lowest_y);
	}

	bool collides(Box2D& o){
		return 
			this->isInsideRangeY(o) 
			&& 
			this->isInsideRangeX(o) 
			&& !(
				(this->topright.y == o.bottomleft.y && this->bottomleft.x == o.topright.x)
				||
				(this->bottomleft.y == o.topright.y && this->bottomleft.x == o.topright.x)
				||
				(this->bottomleft.y == o.topright.y && this->topright.x == o.bottomleft.x)
				||
				(this->topright.y == o.bottomleft.y && this->topright.x == o.bottomleft.x)
			)
		;

	}
};


//     (greens_left-reds_right) + vec_green*v - vec_red*v == 0
//     vec_green*v - vec_red*v == -(greens_left-reds_right)
//     -vec_green*v + vec_red*v == greens_left-reds_right
//     v(-vec_green + vec_red) == greens_left-reds_right
//     v == (greens_left-reds_right)/(-vec_green + vec_red)


C2d WhenWillBoxesCollide(Box2D box1, Box2D box2, C2 speed1, C2 speed2, bool* colliding_x, bool* colliding_y){			//assumes they're not colliding already

	C2 box1_middle = {(box1.topright.x+box1.bottomleft.x)/2, (box1.topright.y+box1.bottomleft.y)/2};
	C2 box2_middle = {(box2.topright.x+box2.bottomleft.x)/2, (box2.topright.y+box2.bottomleft.y)/2};

	double left_speed = 0;
	double right_speed = 0;
	double x_left = 0;
	double x_right = 0;
	if(box1_middle.x < box2_middle.x){
		x_left = box1.topright.x;
		x_right = box2.bottomleft.x;
		left_speed = speed1.x;
		right_speed = speed2.x;
	}else{
		x_left = box2.topright.x;
		x_right = box1.bottomleft.x;
		left_speed = speed2.x;
		right_speed = speed1.x;
	}

	/*printf("GREEN     X:     %.40f\n", x_right);
	printf("GREEN VEC X:     %.40f\n", right_speed);
	printf("RED       X:     %.40f\n", x_left);
	printf("RED   VEC X:     %.40f\n", left_speed);*/

	double top_speed = 0;
	double bottom_speed = 0;
	double y_top = 0;
	double y_bottom = 0;
	if(box1_middle.y < box2_middle.y){
		y_bottom = box1.topright.y;
		y_top = box2.bottomleft.y;
		bottom_speed = speed1.y;
		top_speed = speed2.y;
	}else{
		y_bottom = box2.topright.y;
		y_top = box1.bottomleft.y;
		bottom_speed = speed2.y;
		top_speed = speed1.y;
	}

//	Box2D& x_left  = (box1.bottomleft.x < box2.bottomleft.x)?box1:box2;			//will leave this for the compiler to optimize
//	Box2D& x_right = (box1.bottomleft.x < box2.bottomleft.x)?box2:box1;			//cuz I really wanna use references here
//
//	Box2D& y_bottom  = (box1.bottomleft.y < box2.bottomleft.y)?box1:box2;		//same
//	Box2D& y_top     = (box1.bottomleft.y < box2.bottomleft.y)?box2:box1;		//here

//	float x_need = (-speed1.x+speed2.x == 0.f)?INFINITY:(x_right.bottomleft.x - x_left.topright.x)/(-speed1.x+speed2.x);
//	float y_need = (-speed1.y+speed2.y == 0.f)?INFINITY:(y_top.bottomleft.y - y_bottom.topright.y)/(-speed1.y+speed2.y);

	double horizontal_diff = (x_right - x_left);
	double vertical_diff = (y_top - y_bottom);
	*colliding_x = !horizontal_diff;
	*colliding_y = !vertical_diff;


	double x_need = horizontal_diff/(-right_speed + left_speed);
	double y_need = vertical_diff/(-top_speed+bottom_speed);

	return {x_need, y_need};
}

struct InternalPhysicsObject;
struct PhysicsObject;
struct PhysicsEngine;





struct InternalPhysicsObject{
	Box2D box;
	C2 speed;
	unsigned identification_mask;
	unsigned collision_mask;
};




struct PhysicsObject{
	friend struct PhysicsEngine;

	PhysicsObject(PhysicsEngine* engine, unsigned identification_mask, unsigned collision_mask);
	void engine_register(Box2D box, C2 speed);
	void engine_unregister();
	void process_collisions();

	
	InternalPhysicsObject* internal_object;
	PhysicsEngine* engine;

	unsigned identification_mask;
	unsigned collision_mask;



	virtual void collided(PhysicsObject* o) = 0;
};




struct PhysicsEngine{
	PhysicsEngine();
	
	void reset_pointers(unsigned start_index, unsigned end_index);

	void realloc();

	void add(PhysicsObject* obj, Box2D box, C2 speed);

	void remove(PhysicsObject* obj);

	void process(PhysicsObject* obj);

	unsigned size;
	unsigned capacity;
	InternalPhysicsObject* array;
	PhysicsObject** ref_array;
};


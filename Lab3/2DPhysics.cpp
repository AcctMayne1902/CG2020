#include "2DPhysics.h"

#include <cstring>

PhysicsObject::PhysicsObject(PhysicsEngine* engine, unsigned identification_mask, unsigned collision_mask) : engine(engine), identification_mask(identification_mask), collision_mask(collision_mask){}
void PhysicsObject::engine_register(Box2D box, C2 speed){engine->add(this, box, speed);}
void PhysicsObject::engine_unregister(){engine->remove(this);}
void PhysicsObject::process_collisions(){engine->process(this);}




PhysicsEngine::PhysicsEngine() : size(0), capacity(8), array(new InternalPhysicsObject[8]), ref_array(new PhysicsObject*[8]) {}

void PhysicsEngine::reset_pointers(unsigned start_index, unsigned end_index){
	for (unsigned i = start_index; i < end_index; ++i){
		this->ref_array[i]->internal_object = &this->array[i];
	}
}

void PhysicsEngine::realloc(){
	InternalPhysicsObject* array     = new InternalPhysicsObject[this->capacity*2];
	PhysicsObject**        ref_array = new PhysicsObject*[this->capacity*2];
	
	memcpy(array,     this->array,     capacity*sizeof(InternalPhysicsObject));
	memcpy(ref_array, this->ref_array, capacity*sizeof(PhysicsObject*));
	
	delete[] this->array;
	delete[] this->ref_array;
	
	this->array     = array;
	this->ref_array = ref_array;

	this->capacity *= 2;

	this->reset_pointers(0, this->size);
}

void PhysicsEngine::add(PhysicsObject* obj, Box2D box, C2 speed){
	if(this->capacity == this->size)realloc();
	
	this->ref_array[this->size] = obj;
	this->array[this->size] = {
		box, 
		speed,
		obj->identification_mask, 
		obj->collision_mask
	};
	obj->internal_object = &this->array[this->size];

	this->size++;
}

void PhysicsEngine::remove(PhysicsObject* obj){
	unsigned index = obj->internal_object - this->array; //assuming it's in the array. If it's not, the person calling this function is an idiot.
	unsigned left_afterwards = this->size - index - 1;
	
	memmove(&this->ref_array[index], &this->ref_array[index+1], left_afterwards*sizeof(InternalPhysicsObject));
	memmove(&this->array[index],     &this->array[index+1],     left_afterwards*sizeof(PhysicsObject*)       );

	this->size--;

	this->reset_pointers(index, this->size);
}




bool CheckSpeedMultiplier(double m, Box2D a, Box2D b, C2d a_speed, C2d b_speed){
	a_speed *= m;
	b_speed *= m;

	C2 a_speed_m = {float(a_speed.x), float(a_speed.y)};
	C2 b_speed_m = {float(b_speed.x), float(b_speed.y)};

	a.topright   += a_speed_m;
	a.bottomleft += a_speed_m;

	b.topright   += b_speed_m;
	b.bottomleft += b_speed_m;

	//printf("init bottomleft x: %.30f\n", this->box.bottomleft.x);
	//printf("init bottomleft y: %.30f\n", this->box.bottomleft.y);
	//printf("new bottomleft x:  %.30f\n", newbox.bottomleft.x);
	//printf("new bottomleft y:  %.30f\n", newbox.bottomleft.y);
	//printf("initial downwards: %.30f\n", this->speed.y);
	//printf("updated downwards: %.30f\n", newspeed.y);
	//printf("vecmul:            %.30f\n", (float)m);

	if(!a.collides(b))return false;
	//printf("collides\n");
	//if(signbit((float)m))return false;
	//printf("sign bit check passed\n");

	return true;

}



void PhysicsEngine::process(PhysicsObject* obj){
	InternalPhysicsObject& iobj = *obj->internal_object;
	unsigned collision_mask = iobj.collision_mask;

	double distance_to_collision = 1.f;
	unsigned picked_index = 0;

	for (unsigned i = 0; i < this->size; ++i){

		InternalPhysicsObject& current = this->array[i];

		if(&current == &iobj)continue;	//if we're testing collision with ourselves (collision masks can contain their own type, i.e. grenades could collide with other grenades)
		if(!(collision_mask & current.identification_mask))continue;

		bool collides_x = false, collides_y = false;	//whether the boxes are already colliding (before testing the transform)
		C2d vecmul = WhenWillBoxesCollide(iobj.box, current.box, iobj.speed, current.speed, &collides_x, &collides_y);

		if(vecmul.x >= 0.0 && vecmul.x < 1.0 && vecmul.x < distance_to_collision && !signbit((float)vecmul.x)) {
			if(CheckSpeedMultiplier(vecmul.x, iobj.box, current.box, iobj.speed, current.speed)){
				if(collides_x){
					obj->collided(&current);
				}else{
					distance_to_collision = vecmul.x;
					collided_entity = i;
				}
			}
		}

		if(vecmul.y >= 0.0 && vecmul.y < 1.0 && vecmul.y < distance_to_collision && !signbit((float)vecmul.y)) {
			if(CheckSpeedMultiplier(vecmul.y, iobj.box, current.box, iobj.speed, current.speed)){
				if(collides_y){
					obj->collided(&current);
				}else{
					distance_to_collision = vecmul.y;
					collided_entity = i;
				}
			}
		}
	}
}
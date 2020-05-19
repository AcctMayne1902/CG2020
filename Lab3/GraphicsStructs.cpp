#include "GraphicsStructs.h"

#include <GL/glut.h>

////////////////////////////////////////////////////////////C2
C2::C2(){}
C2::C2(float a, float b) : x(a), y(b){}

float C2::dot(C2& o){
	return this->x*o.x + this->y*o.y;
}


C2& C2::operator+=(const C2& o){
	this->x += o.x;
	this->y += o.y;
	return *this;
}

C2& C2::operator*=(const C2& o){
	this->x *= o.x;
	this->y *= o.y;
	return *this;
}

C2& C2::operator*=(const float& o){
	this->x *= o;
	this->y *= o;
	return *this;
}
////////////////////////////////////////////////////////////C2d
C2d::C2d(){}
C2d::C2d(C2& v) : x(v.x), y(v.y){}
C2d::C2d(double a, double b) : x(a), y(b){}

double C2d::dot(C2d& o){
	return this->x*o.x + this->y*o.y;
}


C2d& C2d::operator+=(const C2d& o){
	this->x += o.x;
	this->y += o.y;
	return *this;
}

C2d& C2d::operator*=(const C2d& o){
	this->x *= o.x;
	this->y *= o.y;
	return *this;
}

C2d& C2d::operator*=(const double& o){
	this->x *= o;
	this->y *= o;
	return *this;
}

////////////////////////////////////////////////////////////C3

C3::C3(){}
C3::C3(float a){*this = a;}
C3::C3(float a, float b, float c) : x(a), y(b), z(c){}

C3& C3::operator+=(const C3& other){
	this->x += other.x;
	this->y += other.y;
	this->z += other.z;
	return *this;
}

C3& C3::operator-=(const C3& other){
	this->x -= other.x;
	this->y -= other.y;
	this->z -= other.z;
	return *this;	
}

C3& C3::operator/=(const float& div){
	this->x /= div;
	this->y /= div;
	this->z /= div;
	return *this;
}

C3& C3::operator*=(const float& div){
	this->x *= div;
	this->y *= div;
	this->z *= div;
	return *this;
}

C3 C3::operator*(const float& o){
	return {this->x * o, this->y * o, this->z * o,};
}
C3 C3::operator+(const C3& o){
	return {this->x + o.x, this->y + o.y, this->z + o.z,};
}

C3 C3::operator-(const C3& o){
	return {this->x - o.x, this->y - o.y, this->z - o.z,};
}

C3 C3::operator-(){
	return {-this->x, -this->y, -this->z};
}

bool C3::operator==(const C3& o){
	return this->x == o.x && this->y == o.y && this->z == o.z;
}
bool C3::operator!=(const C3& o){
	return !(*this == o);
}

C3& C3::operator=(const float& o){
	this->x = o;
	this->y = o;
	this->z = o;
	return *this;
}

C3& C3::operator=(const C3& o){
	this->x = o.x;
	this->y = o.y;
	this->z = o.z;
	return *this;
}


////////////////////////////////////////////////////////////Mesh

void Mesh::Center(){
	C3 center = {0, 0, 0};

	for (unsigned i = 0; i < this->point_amount; ++i)
		center += this->points[i];
	center /= float(this->point_amount);

	for (unsigned i = 0; i < this->point_amount; ++i)
		this->points[i] -= center;
	return;
}


////////////////////////////////////////////////////////////TextureData

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
int TextureData::Construct(const char* file_path){
    return !(this->data = stbi_load(
    	file_path, 
    	&this->width, 
    	&this->height, 
    	&this->channels, 
    	0
    ));
}

void TextureData::Destroy(){
	if(this->data)stbi_image_free(this->data);
	return;
}



////////////////////////////////////////////////////////////Texture

int Texture::Construct(TextureData* tex_data){
    glGenTextures(1, &this->gl_tex);  
    glBindTexture(GL_TEXTURE_2D, this->gl_tex);

    gluBuild2DMipmaps( 
    	GL_TEXTURE_2D, 
    	3, 
    	tex_data->width, 
    	tex_data->height, 
    	GL_RGB, 
    	GL_UNSIGNED_BYTE, 
    	tex_data->data 
    );


    /*glTexImage2D(
    	GL_TEXTURE_2D, 
    	0, 
    	3, 
    	tex_data->width, 
    	tex_data->height, 
    	0, 
    	GL_RGB, 
    	GL_UNSIGNED_BYTE, 
    	tex_data->data
    );*/
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR /*GL_NEAREST*/); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR /*GL_NEAREST*/); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);   
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    return true;
}

void Texture::Destroy(){
	glDeleteTextures(1, &this->gl_tex);
	return;
}



////////////////////////////////////////////////////////////Camera
int Camera::Construct(C3 position, C3 rot, float ratio, float fov){
	this->position = position;
	this->rot = rot;
	this->aspect_ratio = ratio;
	this->field_of_view = fov;
	return true;
}

void Camera::Load(){
	glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

	if(this->field_of_view == 0.f){
		glOrtho(-50.f, 50.f, -50.f, 50.f, 0.f, 1000.f);	//TODO: should have aspect ratio into account here as well
	}else{
		gluPerspective(this->field_of_view, this->aspect_ratio, 0.f, 1000.f);
	}

	glRotatef(this->rot.roll, 1.f, 0.f, 0.f);
	glRotatef(this->rot.pitch, 0.f, 1.f, 0.f);
	glRotatef(this->rot.yaw, 0.f, 0.f, 1.f);
	glTranslatef(this->position.x, this->position.y, this->position.z);
}

void Camera::ResetAspectRatio(float ratio){
	this->aspect_ratio = ratio;
	Camera::Load();
}



////////////////////////////////////////////////////////////CameraManager
int CameraManager::Construct(){
	this->cameras = new Camera[this->capacity = 2];
	this->current_camera = 0;
	this->camera_count = 0;
	return true;
}

void CameraManager::Destroy(){
	delete[] this->cameras;
}

void CameraManager::AddCamera(Camera cam){
	if(this->capacity == this->camera_count)
		this->realloc();
	memcpy(&this->cameras[this->camera_count++], &cam, sizeof(Camera));
}

void CameraManager::SetCamera(unsigned cam_num){
	(this->current_camera = this->cameras+cam_num)->Load();
}

void CameraManager::RotateCurrent(C3 rot){	//TODO: this code is probably optimizable
	C3& cam_rot = this->current_camera->rot;
	C3& cam_pos = this->current_camera->position;


	//translate into origin
	glTranslatef(-cam_pos.x, -cam_pos.y, -cam_pos.z);

	//rotate into origin				//TODO: this might be unecessary
	glRotatef(-cam_rot.roll, 1.f, 0.f, 0.f);
	glRotatef(-cam_rot.pitch, 0.f, 1.f, 0.f);
	glRotatef(-cam_rot.yaw, 0.f, 0.f, 1.f);

	//do roll pitch yaw rotation based on parameter
	glRotatef(rot.roll, 1.f, 0.f, 0.f);
	glRotatef(rot.pitch, 0.f, 1.f, 0.f);
	glRotatef(rot.yaw, 0.f, 0.f, 1.f);

	//rotate back into place			//TODO: this might be unecessary
	glRotatef(cam_rot.roll, 1.f, 0.f, 0.f);
	glRotatef(cam_rot.pitch, 0.f, 1.f, 0.f);
	glRotatef(cam_rot.yaw, 0.f, 0.f, 1.f);

	//translate back into place
	glTranslatef(cam_pos.x, cam_pos.y, cam_pos.z);

	cam_rot += rot;
}

void CameraManager::TranslateCurrent(C3 pos){
	C3& cam_rot = this->current_camera->rot;

	//rotate into origin
	glRotatef(-cam_rot.roll, 1.f, 0.f, 0.f);
	glRotatef(-cam_rot.pitch, 0.f, 1.f, 0.f);
	glRotatef(-cam_rot.yaw, 0.f, 0.f, 1.f);

	//do absolute translation
	glTranslatef(pos.x, pos.y, pos.z);

	//rotate back into place
	glRotatef(cam_rot.roll, 1.f, 0.f, 0.f);
	glRotatef(cam_rot.pitch, 0.f, 1.f, 0.f);
	glRotatef(cam_rot.yaw, 0.f, 0.f, 1.f);

	this->current_camera->position += pos;
}

#include <cstring>
void CameraManager::realloc(){
	uintptr_t curr_cam_index = this->current_camera - this->cameras;

	Camera* newmem = new Camera[this->capacity*=2];
	for (unsigned i = 0; i < this->camera_count; ++i)
		memcpy(&newmem[i], &this->cameras[i], sizeof(Camera));
	delete[] this->cameras;
	this->cameras = newmem;

	this->current_camera = newmem + curr_cam_index;
}
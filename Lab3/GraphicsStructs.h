#include <stdint.h>
#pragma once
//#include <glm/glm.hpp>

struct C2{
	C2();
	C2(float a, float b);
	union{
		struct{
			float x, y;
		};
		struct{
			float u, v;
		};
	};

	float dot(C2& o);
	float normalize();
	C2& operator+=(const C2& o);
	C2& operator*=(const C2& o);
	C2& operator*=(const float& o);
};

struct C2d{
	C2d();
	C2d(C2& v);
	C2d(double a, double b);
	union{
		struct{
			double x, y;
		};
		struct{
			double u, v;
		};
	};

	double dot(C2d& o);
	C2d& operator+=(const C2d& o);
	C2d& operator*=(const C2d& o);
	C2d& operator*=(const double& o);
};

struct C3{	//coordinate 3D
	C3();
	C3(float a);
	C3(float a, float b, float c);
	union{
		struct{
			float x, y, z;
		};
		struct{
			//roll: rotation around the x axis
			//pitch: rotation around the y axis
			//yaw: rotation around the z axis
			float roll, pitch, yaw;
		};
		struct{
			float r, g, b;
		};
	};

	C3& operator+=(const C3& o);
	C3& operator-=(const C3& o);
	C3& operator/=(const float& div);
	C3& operator*=(const float& div);

	C3& operator=(const float& o);
	C3& operator=(const C3& o);

	C3 operator+(const C3& o);
	C3 operator*(const float& o);
	C3 operator-(const C3& o);
	C3 operator-();

	bool operator==(const C3& o);
	bool operator!=(const C3& o);

};

struct TexCoord{float x, y;};

//every non-utility struct has a construct method that's a constructor but without the disadvantages 
//of a regular constructor. A bit more clumbersome to use but allows for more dynamic usage, example:
//	TextureData tex_data;
//	if(!tex_data.Construct("image_file.jpg")){puts("failed to load texture!!"); return 0;}
//	some_other_function(tex_data);
//	tex_data.Destroy();



struct Mesh{
	struct Section;
	struct Texture;

	void Center();	//average all the points in the mesh and translate them so it becomes (0, 0, 0)

	C3* points;
	unsigned point_amount;

	Mesh::Section* sections;
	unsigned section_amount;

	Mesh::Texture* texture;
};

struct Mesh::Section{
	int start_index, amount;
};

struct Mesh::Texture{
	enum Types{TEXTURE, COLORS, COLOR} type;
	union{
		struct{
			TexCoord* tex_coords;
			Texture* texture;
		};
		C3* colors;
		C3 color;
	};
};




//this structure gets and holds the pixels and dimensions for the texture
//"channels" is just how the pixels are layed, examples:
//	-RGB is 3
//	-RGBA (with alpha aka transparency) is 4
//	-Black and White is 1
struct TextureData{
	int Construct(const char* file_path);
	void Destroy();

	uint8_t* data;
	int width, height, channels;
};

//this structure builds and holds the openGL object (builds from a TextureData)
//also has a pointer to it's corresponding TextureData (in case that information is necessary)
struct Texture{
	int Construct(TextureData* tex_data);
	void Destroy();

	TextureData* tex_data;
	unsigned gl_tex;
};

struct Camera{
	Camera(){}

	int Construct(C3 position, C3 rot, float aspect_ratio, float fov = 0.f);
	void Load();
	void ResetAspectRatio(float ratio);
	
	C3 position;
	C3 rot;
	float field_of_view;
	float aspect_ratio;
};

struct CameraManager{
	int Construct();
	void Destroy();

	void AddCamera(Camera cam);
	void SetCamera(unsigned cam_num);

	void RotateCurrent(C3 rot);
	void TranslateCurrent(C3 pos);

	Camera* cameras;
	unsigned camera_count;

	Camera* current_camera;

	//you need not concern yourself with such matters
	private: 
	unsigned capacity;
	void realloc();

};
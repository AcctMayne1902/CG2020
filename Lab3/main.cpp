#include <GL/glut.h>
#include <GL/freeglut.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "GraphicsStructs.h"
#include "ObjParser.h"

#include "DriveHome.h"

#include "2DPhysics.h"
void renderBox2D(Box2D box, C3 color){
	glColor3f(color.r, color.g, color.b);
	glVertex3f(box.topright.x, box.bottomleft.y, 0.1f);
	glVertex3f(box.topright.x, box.topright.y, 0.1f);
	glVertex3f(box.bottomleft.x, box.bottomleft.y, 0.1f);


	glVertex3f(box.bottomleft.x, box.bottomleft.y, 0.1f);
	glVertex3f(box.topright.x, box.topright.y, 0.1f);
	glVertex3f(box.bottomleft.x, box.topright.y, 0.1f);
}
//Box2D collision_test_box = {{0, 0}, {5, 5}};

	Game gam = Game();
	bool axis = true;

#include <vector>


int width = 720, height = 720;																//  <<-global->> width and height of the window

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////struct for a "camera" object///////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct PlayerView{
	C3 position;
	C3 rotation;
	float FOV;
	float ratio;

	void Translate(C3 translation, bool rotation_relative){
		if(rotation_relative){
			translation = {
				translation.x,
				-translation.y*(float)cos(this->rotation.x * M_PI/180.f) + translation.z*(float)sin(this->rotation.x * M_PI/180.f),
				-translation.y*(float)sin(this->rotation.x * M_PI/180.f) - translation.z*(float)cos(this->rotation.x * M_PI/180.f)
			};
			translation = {
				+translation.x*(float)cos(this->rotation.y * M_PI/180.f) + translation.z*(float)sin(this->rotation.y * M_PI/180.f),
				translation.y,
				+translation.x*(float)sin(this->rotation.y * M_PI/180.f) - translation.z*(float)cos(this->rotation.y * M_PI/180.f)
			};
		}
		this->position += translation;
	}

	void Rotate(C3 rotation_vec){
		this->rotation += rotation_vec;
	}

	void ApplyGL(){
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity(); gluPerspective(this->FOV, this->ratio, 0.5, 100000);

		glRotatef(this->rotation.x, 1, 0, 0);
		glRotatef(this->rotation.y, 0, 1, 0);
		glRotatef(this->rotation.z, 0, 0, 1);
		glTranslatef(-this->position.x, -this->position.y, -this->position.z);
	}
};

PlayerView player_view = {{0.127537, 1.300000, 27.312429}, 		{0.f, 0.f, 0.000000}, 	70.f, 1.f};

PlayerView * curr_view = &player_view;



bool _w = false;																			//  <<-global->> bool for whether the w key is down
bool _a = false;																			//  <<-global->> bool for whether the a key is down
bool _s = false;																			//  <<-global->> bool for whether the s key is down
bool _d = false;																			//  <<-global->> bool for whether the d key is down
bool _q = false;																			//  <<-global->> bool for whether the q key is down
bool _e = false;																			//  <<-global->> bool for whether the e key is down
int mouse_move_x = 0, mouse_move_y = 0;														//  <<-global->> vars for the mouse offset since mouse movement was last processed
int mouse_x = 0; int mouse_y = 0;															//  <<-global->> vars for the mouse position in window coordinates




//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////struct for a 3D model (meshes and textures)///////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Model{

	C3 position = {};
	C3 rotation = {};
	C3 scale = 1.f;

	struct Section{
		OBJ* mesh;
		Texture* texture;
	};

	std::vector<Section> sections;

	//receives multidimensional array with sets of 2 `char*`s, first for the .obj, second for the texture. Last set is zeroed out. 
	int Construct(const char* * paths){
		while(*paths){
			OBJ* mesh = new OBJ();
			mesh->Construct(*paths++);

			TextureData* data = new TextureData();
			data->Construct(*paths++);
		    Texture* texture = new Texture();
		    texture->Construct(data);

			this->sections.push_back({mesh, texture}); 
		}
		return 1;
	}

	void Destroy(){
		for (int i = 0; i < this->sections.size(); ++i){
			this->sections[i].texture->tex_data->Destroy();
			delete this->sections[i].texture->tex_data;
			this->sections[i].texture->Destroy();
			delete this->sections[i].texture;
			delete this->sections[i].mesh;
		}
		this->sections.clear();
	}

	void RenderGL(){
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
		glTranslatef(this->position.x, this->position.y, this->position.z);
		glRotatef(this->rotation.x, 1, 0, 0);glRotatef(this->rotation.y, 0, 1, 0);glRotatef(this->rotation.z, 0, 0, 1);
		glScalef(this->scale.x, this->scale.y, this->scale.z);



		for (int i = 0; i < this->sections.size(); ++i){
			Section& sec = this->sections[i];

			glBindTexture(GL_TEXTURE_2D, sec.texture->gl_tex);

			glBegin(GL_TRIANGLES);
				for (unsigned i = 0; i < sec.mesh->map.size(); ++i){

					glTexCoord2f(
						+sec.mesh->UVs[sec.mesh->map[i].uv_index].u, 
						-sec.mesh->UVs[sec.mesh->map[i].uv_index].v 
					);
					glVertex3f(
						sec.mesh->vertexes[sec.mesh->map[i].vertex_index].x, 
						sec.mesh->vertexes[sec.mesh->map[i].vertex_index].y, 
						sec.mesh->vertexes[sec.mesh->map[i].vertex_index].z
					);
					glNormal3f(
						sec.mesh->normals[sec.mesh->map[i].normal_index].x, 
						sec.mesh->normals[sec.mesh->map[i].normal_index].y, 
						sec.mesh->normals[sec.mesh->map[i].normal_index].z
					);
					
				}
			glEnd();

		}

		glPopMatrix();
	}
};


Model* map = 0;
Model* skybox = 0;
Model* player[5] = {0};
#define PLAYER_WALK 0
#define PLAYER_FLY 1
#define PLAYER_TPOSE 2
#define PLAYER_UP 3
#define PLAYER_DOWN 4


//<-------------------------------------------------------------------------------------------------------------------------------------------------------------->
//<-----------------------------------------------------------idle function: called each frame------------------------------------------------------------------->
//<-------------------------------------------------------------------------------------------------------------------------------------------------------------->

void idle(void){
	glMatrixMode(GL_PROJECTION);

	C3 translation = {
		float(_d - _a),
		float(_e - _q),
		float(_s - _w)
	};

	curr_view->Rotate({float(mouse_move_y), float(mouse_move_x), 0});
	curr_view->Translate(translation*0.1, true);

	mouse_move_x = mouse_move_y = 0;

    glutPostRedisplay();
}



//<-------------------------------------------------------------------------------------------------------------------------------------------------------------->
//<-------------------------------------------------------keyboard function: called each key press--------------------------------------------------------------->
//<-------------------------------------------------------------------------------------------------------------------------------------------------------------->

void keyboard(unsigned char key, int x, int y){
	if(key == 0x1B){
		glutDestroyWindow ( glutGetWindow() );
	}
	switch(key){
		case 'w': _w = true; break;
		case 'a': _a = true; break;
		case 's': _s = true; break;
		case 'd': _d = true; break;
		case 'q': _q = true; break;
		case 'e': _e = true; break;

		case 't': /*gam.player.speed.y += jump_speed*/ t = true; break;
		case 'f': /*gam.player.speed.x -= move_speed;*/ f = true; break;
		case 'h': /*gam.player.speed.x += move_speed;*/ h = true; break;
	}
}


//<-------------------------------------------------------------------------------------------------------------------------------------------------------------->
//<------------------------------------------------------keyboard function: called each key release-------------------------------------------------------------->
//<-------------------------------------------------------------------------------------------------------------------------------------------------------------->

void keyboardup(unsigned char key, int x, int y){
	switch(key){
		case 'w': _w = false; break;
		case 'a': _a = false; break;
		case 's': _s = false; break;
		case 'd': _d = false; break;
		case 'q': _q = false; break;
		case 'e': _e = false; break;

		//case 'j': collision_test_box.bottomleft.x -= 1; collision_test_box.topright.x -= 1; break;
		//case 'k': collision_test_box.bottomleft.y -= 1; collision_test_box.topright.y -= 1; break;
		//case 'l': collision_test_box.bottomleft.x += 1; collision_test_box.topright.x += 1; break;
		//case 'i': collision_test_box.bottomleft.y += 1; collision_test_box.topright.y += 1; break;
		//case 'i': gam.player.Tick(); break;
		//case 'o': gam.player.Tick(); break;
		//case 'p': gam.player.Tick(); break;
		
		case 'f': if(f){/*gam.player.speed.x += move_speed;*/ f = false;} break;
		case 'h': if(h){/*gam.player.speed.x -= move_speed;*/ h = false;} break;
		case 'x': axis = !axis;
	}
}



//<-------------------------------------------------------------------------------------------------------------------------------------------------------------->
//<----------------------------------------------------keyboard function: called each special key press---------------------------------------------------------->
//<-------------------------------------------------------------------------------------------------------------------------------------------------------------->

void keyboard_special(int key, int x, int y){
}



//<-------------------------------------------------------------------------------------------------------------------------------------------------------------->
//<------------------------------------------------------mouse function: called each mouse button press---------------------------------------------------------->
//<-------------------------------------------------------------------------------------------------------------------------------------------------------------->

void mouse(int button, int state, int x, int y){

	mouse_x = x; 
	mouse_y = y;
}


//<-------------------------------------------------------------------------------------------------------------------------------------------------------------->
//<----------------------------------------------------------mouse function: called each mouse drag-------------------------------------------------------------->
//<-------------------------------------------------------------------------------------------------------------------------------------------------------------->

void mouse_move(int x, int y){
	mouse_move_x += (x - mouse_x)/2;
	mouse_move_y += (y - mouse_y)/2;
	mouse_x = x; 
	mouse_y = y;
}




//<-------------------------------------------------------------------------------------------------------------------------------------------------------------->
//<----------------------------------------------init function: loads models, textures, and initializes opengl--------------------------------------------------->
//<-------------------------------------------------------------------------------------------------------------------------------------------------------------->

void init(){

	const char* map_paths[][2] = {
		{"models/map.obj", "textures/map.png"},
		{0, 0}
	};
	map = new Model();
    map->Construct((const char**)map_paths); 

    const char* skybox_paths[][2] = {
		{"models/skybox.obj", "textures/skybox.png"},
		{0, 0}
	};

	skybox = new Model();
    skybox->Construct((const char**)skybox_paths); 
    skybox->scale = 64.f;
    skybox->rotation.y = -90.f;

    const char* player0_paths[][2] = {
		{"models/roll.obj", "textures/player.png"},
		{0, 0}
	};
	const char* player1_paths[][2] = {
		{"models/fly.obj", "textures/player.png"},
		{0, 0}
	};
	const char* player2_paths[][2] = {
		{"models/tpose.obj", "textures/player.png"},
		{0, 0}
	};
	const char* player3_paths[][2] = {
		{"models/straightup.obj", "textures/player.png"},
		{0, 0}
	};
	const char* player4_paths[][2] = {
		{"models/rko.obj", "textures/player.png"},
		{0, 0}
	};

	player[PLAYER_WALK ] = new Model();
    player[PLAYER_WALK ]->Construct((const char**)player0_paths); 
    player[PLAYER_FLY  ] = new Model();
    player[PLAYER_FLY  ]->Construct((const char**)player1_paths); 
    player[PLAYER_TPOSE] = new Model();
    player[PLAYER_TPOSE]->Construct((const char**)player2_paths); 
    player[PLAYER_UP   ] = new Model();
    player[PLAYER_UP   ]->Construct((const char**)player3_paths); 
    player[PLAYER_DOWN ] = new Model();
    player[PLAYER_DOWN ]->Construct((const char**)player4_paths); 
    //player->scale = 64.f;
    //player->rotation.y = -90.f;


    //-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/ "parameters" for the textures
    /*glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);   
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);*/

    glEnable(GL_TEXTURE_2D);

    //glClearColor (0.0705, 0.0156, 0.0901, 1.0);
    //glClearColor (1, 1, 1, 1.0);
    glClearColor (0.4, 0.4, 0.4, 1.0);


    //-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/ render setup (depth buffer, polygon filling, backface culling)
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glPolygonMode(GL_FRONT/*GL_FRONT_AND_BACK*/, GL_FILL);

	glEnable(GL_CULL_FACE);  
	glCullFace(GL_BACK);

	//-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/ lighting setup
	/*glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	float l_ambient[] = {0.f, 0.f, 0.f, 1.f};
	float l_diffuse[] = {1.f, 1.f, 1.f, 1.f};
	float l_specular[] = {1.f, 1.f, 1.f, 1.f};
	glLightfv(GL_LIGHT0, GL_AMBIENT, l_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, l_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, l_specular);
	glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 45.f);*/





	glShadeModel(GL_FLAT);
}







//<-------------------------------------------------------------------------------------------------------------------------------------------------------------->
//<------------------------------------------------------------display function: called each frame--------------------------------------------------------------->
//<-------------------------------------------------------------------------------------------------------------------------------------------------------------->
void display(void){

	glMatrixMode(GL_PROJECTION);

	curr_view->position = {gam.player.position.x, gam.player.position.y, 20};
	curr_view->ApplyGL();/////////////////////////////////////////////////////////////////////////////////////////////////VIEW MATRIX


	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glColor3f(1, 1, 1);	//reset colora



	glDisable(GL_TEXTURE_2D);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////DEBUG

    glEnable(GL_TEXTURE_2D);
	map->RenderGL();
	skybox->position= curr_view->position;
	skybox->RenderGL();
	
	Model* player_animation = gam.player.airborne
		? gam.player.speed.y >= 0.f
			? gam.player.speed.x == 0.f
				? player[PLAYER_UP]
				: player[PLAYER_FLY]
			: player[PLAYER_DOWN]
		:((f || h) 
			? player[PLAYER_WALK] 
			: player[PLAYER_TPOSE])
	;
	player_animation->scale = {f?-1.f:1.f, 1.f, 1.f};
	player_animation->position = {gam.player.position.x, gam.player.position.y, 1.5f};
	//player.position 
	player_animation->RenderGL();

	glDisable(GL_TEXTURE_2D);


	bool collides = false;

	glBegin(GL_TRIANGLES);		

		//for (int i = 0; i < amount_map_boxes; ++i)	{
		//	renderBox2D(map_boxes[i], {float(i)/8.f, 0, 1});
		////	collides = collides || collision_test_box.collides(map_boxes[i]);
		//}
		//
		//renderBox2D(collision_test_box, {collides?0.f:1.f, 1.f, 1.f});
		
		//printf("(%f, %f); Airborne: %s;\n", gam.player.position.x, gam.player.position.y, gam.player.airborne?"yes":"no");

		//renderBox2D(gam.player.newbox, {1.f, 0, 0});
		//renderBox2D(gam.player.box, {gam.player.airborne?0.f:1.f, 1.f, 1.f});
	glEnd();

	//gam.player.Tick();
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	glEnable(GL_TEXTURE_2D);

	
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////AXIS
	if(axis){
		glDisable(GL_TEXTURE_2D);

		glDisable(GL_LIGHTING);

		glLineWidth(4);
	    glDisable(GL_DEPTH_TEST);
		glBegin(GL_LINES);
			glColor3f(.4f, .1f, .1f);
			glVertex3f(0, 0, 0);glVertex3f(400, 0, 0);
			glColor3f(.1f, .4f, .1f);
			glVertex3f(0, 0, 0);glVertex3f(0, 400, 0);
			glColor3f(.1f, .1f, .4f);
			glVertex3f(0, 0, 0);glVertex3f(0, 0, 400);
		glEnd();
	    glEnable(GL_DEPTH_TEST);
		glBegin(GL_LINES);    
			glColor3f(1.f, 0.f, 0.f);
			glVertex3f(0, 0, 0);glVertex3f(400, 0, 0);
			glColor3f(0.f, 1.f, 0.f);
			glVertex3f(0, 0, 0);glVertex3f(0, 400, 0);
			glColor3f(0.f, 0.f, 1.f);
			glVertex3f(0, 0, 0);glVertex3f(0, 0, 400);
		glEnd();
		

		glEnable(GL_TEXTURE_2D);
	}

    glutSwapBuffers();
}


//<-------------------------------------------------------------------------------------------------------------------------------------------------------------->
//<-------------------------------------------------reshape function: called each time the window is resized----------------------------------------------------->
//<-------------------------------------------------------------------------------------------------------------------------------------------------------------->

void reshape(int w, int h){
    glViewport (0, 0, (GLsizei) w, (GLsizei) h);
    curr_view->ratio = (float)w/(float)h;
    curr_view->ApplyGL();


    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    width = w; height = h;
}



//<-------------------------------------------------------------------------------------------------------------------------------------------------------------->
//<---------------------------------------------------------------main: initializes glut------------------------------------------------------------------------->
//<-------------------------------------------------------------------------------------------------------------------------------------------------------------->



#include <Windows.h>

#include <chrono>


//double tickrate_ = 60.0;
double tick_time = 1000.0/tickrate;

bool gameruns = true;

DWORD WINAPI LogicThread(LPVOID param){
	std::chrono::system_clock::time_point tick_start = std::chrono::system_clock::now();
	std::chrono::system_clock::time_point after_sleep = std::chrono::system_clock::now();
	
	while(gameruns){
		tick_start = std::chrono::system_clock::now();
        std::chrono::duration<double, std::milli> work_time = tick_start - after_sleep;

        if (work_time.count() < tick_time)
        {
            std::chrono::duration<double, std::milli> delta_ms(tick_time - work_time.count());
            auto delta_ms_duration = std::chrono::duration_cast<std::chrono::milliseconds>(delta_ms);
            Sleep(delta_ms_duration.count());
        }

        after_sleep = std::chrono::system_clock::now();
        std::chrono::duration<double, std::milli> sleep_time = after_sleep - tick_start;

        //codaz
		gam.player.speed.y -= gravity;
        if(t){gam.player.speed.y += jump_speed; t = false;}
        
        if     (f) {gam.player.speed.x = -move_speed;}
        else if(h) {gam.player.speed.x = +move_speed;}
        else       {gam.player.speed.x = 0.f;}

        gam.player.BeforeTick();
		gam.player.Tick();
		gam.player.AfterTick();
		//gam.player.speed.x = 0;
        //printf("Time: %f - %f\n", (work_time + sleep_time).count(), (work_time).count());
	}
	return 0;
}



#include <cstdlib>
int main(int argc, char** argv){


//#define A 0.2063387632369995100000000000000f
//#define B 0.7307806611061096200000000000000f
//	
//
//	float a = A;
//	float b = B;
//
//	float floatie1 = A + (B * ((A)/(-B)));
//	float floatie2 = a + (b * ((a)/(-b)));
//	printf("%.40f  :  ", floatie1);
//	printfloat_bin(floatie1);
//	printf("%.40f", floatie2);
//	printfloat_bin(floatie2);
//
//	return 0;




	//float s1 = (a)/(-b);
	//float s2 = b*s1;
	//float floatie2 = a + s2;



//	float a = A;
//	float b = B;
//
//	//float floatie1 = A + (B * ((A)/(-B)));
//
//	float step1 = a/(-b);
//	//printf("%f\n", step1);
//	float step2 = b*step1;
//	//printf("%f\n", step2);
//
//	float floatie2 = a + step2;
//	printf("%.40f\n", floatie2);


	/*double a_ = A;
	double b_ = B;

	//float floatie1 = A + (B * ((A)/(-B)));

	double step1_ = a_/(-b_);
	printf("%f\n", step1_);
	double step2_ = b_*step1_;
	printf("%f\n", step2_);

	floatie2 = a_ + step2_;
	printf("%.40f\n", floatie2);*/
/*

	{
		unsigned number = *(unsigned*)(&floatie);
		for (int i = 32; i != 0; --i){
			printf("%d", (number >> i-1) & 1);
		}printf("\n\n");
	}
*/
//printf("\n\n\n\n\n\n\n\n\n\n\n\n");



	gam.player.box = {{0, 0}, {2, 2}};
//	gam.player.box = {{3.281250, 5.986723}, {2+3.281250, 2+5.986723}};
//	gam.player.box = {{3.281250, 0.206338763236999510}, {2+3.281250, 2+0.206338763236999510}};//(3.281250, 0.206338763236999510), (0.000000, -0.730780661106109620)
	gam.player.box = {{9.161062, 4.8924612998962402000000000000000000000000}, {2+9.161062, 2+4.8924612998962402000000000000000000000000}};//(3.281250, 0.988575458526611330), (0.000000, -1.255390167236328100)
	gam.player.position = {(gam.player.box.bottomleft.x + gam.player.box.topright.x) / 2.f, gam.player.box.bottomleft.y};
	gam.player.speed = {0, 0};//1.f/4.f};
//	gam.player.speed = {0.000000, -0.270937};
//	gam.player.speed = {0.000000, -0.730780661106109620};
//	gam.player.speed = {-0.468750, -0.421523422002792360};//(9.161062, 4.8924612998962402000000000000000000000000), (-0.468750, -0.421523422002792360)
	gam.player.airborne = false;
//
//	gam.player.Tick();gam.player.Tick();gam.player.Tick();
//
//	return 0;

	//printf("AE %f\n", WhenWillBoxesCollide({{-1,1.f/7.f}, {1,2}}, {{-1,-1}, {1,1.f/11.f}}, {0,-1.f/12.f}, {0,1.f/3.f}).y);

	glutInit(&argc, argv);
    glutSetOption(GLUT_MULTISAMPLE, 8);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_MULTISAMPLE);
    glutInitWindowSize(width, height);
    glutInitWindowPosition(100, 100);
    glutCreateWindow(argv[0]);
    init();
    glutIdleFunc(idle);
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutMotionFunc(mouse_move); 
    glutIgnoreKeyRepeat(true);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardup);
    glutSpecialFunc(keyboard_special);


    HANDLE logic_thread = CreateThread(0, 0, LogicThread, 0, 0, 0);


    glutMainLoop();


    gameruns = false;

    WaitForSingleObject(logic_thread, INFINITE);

    return 0;
}


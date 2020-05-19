#include <cstdio>
#include "ObjParser.h"

int main(int argc, char const *argv[]){
	OBJ obj = OBJ();
	obj.Construct("map_missing_triangles.obj");
	for (int t = 0; t < obj.map.size(); t+=3){
		C3 a = obj.vertexes[obj.map[t+0].vertex_index];
		C3 c = obj.vertexes[obj.map[t+1].vertex_index];
		C3 b = obj.vertexes[obj.map[t+2].vertex_index];
		if(a.x == 3.f && b.x == 3.f && c.x ==3.f)
			printf("[%f, %f] . [%f, %f]\n", -a.z, a.y, -c.z, c.y);
	}

	return 0;
}
#include <vector>
#include <cstdio>
#include "GraphicsStructs.h"

struct Point{
	unsigned vertex_index;
	unsigned normal_index;
	unsigned uv_index;
};

struct OBJ{
	OBJ(){
		this->vertexes = std::vector<C3> (); 
		this->normals  = std::vector<C3> ();	
		this->UVs = std::vector<C2> ();	
		this->map = std::vector<Point> ();	
	}

	std::vector<C3> vertexes;
	std::vector<C3> normals;
	std::vector<C2> UVs;
	std::vector<Point> map;
	
	int Construct(const char* filename){
		
		FILE * pFile;
		char buffer[300];
		pFile = fopen(filename,"r");	
		
		if(pFile == NULL) return 1;
		
		while(fgets(buffer,300,pFile)){

			if(buffer[0]=='v'){
				if(buffer[1]=='n'){
					//3
					C3 normal = {};
					sscanf(buffer+2,"%f %f %f",&normal.x,&normal.y,&normal.z);
					this->normals.push_back(normal);
				}else if(buffer[1]=='t'){
					//2
					C2 uv = {};
					sscanf(buffer+2,"%f %f",&uv.u,&uv.v);
					this->UVs.push_back(uv);
				}else {
					//3
					C3 vertice = {};
					sscanf(buffer+1,"%f %f %f",&vertice.x,&vertice.y,&vertice.z);
					this->vertexes.push_back(vertice);
				}
			}else if(buffer[0]=='f'){
				int currindex = 0;
				char* ints[3][3];
				char** ints2 = (char**)ints;
				char* buffer_ = buffer+1;

				while(buffer_[0]){
					if(buffer_[0] == ' ' || buffer_[0] == '/'){
						buffer_[0]=0;
						ints2[currindex++]=buffer_+1;
					}
					buffer_++;
				}

				for (int i = 0; i < 3; ++i){
					Point pinto = {};
					sscanf(ints[i][0],"%d",&pinto.vertex_index);
					sscanf(ints[i][1],"%d",&pinto.uv_index);
					sscanf(ints[i][2],"%d",&pinto.normal_index);
					pinto.vertex_index--;pinto.uv_index--;pinto.normal_index--;
					this->map.push_back(pinto);
				}
			}else{
				continue;
			}	

		}

		fclose(pFile);
		

		return 0;
	}

};




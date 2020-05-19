#include <stdio.h>

/*int main(int argc, char const *argv[])
{
	//float red_x = 1.f/11.f;
	//float green_x = 1.f/7.f;
	//float red_vec = 1.f/3.f;
	//float green_vec = 1.f/12.f;
	float red_x = 1.f/11.f;
	float green_x = 1.f/7.f;
	float red_vec = 1.f/3.f;
	float green_vec = -1.f/12.f;


	float val = (green_x-red_x)/(-green_vec+red_vec);

	float change_red = red_vec*val;
	float new_red_x = red_x + change_red;
	float change_green = green_vec*val;
	float new_green_x = green_x + change_green;

	printf("GREEN X:             %.40f\n", green_x);
	printf("NEW GREEN X:         %.40f\n", new_green_x);
	printf("RED X:               %.40f\n", red_x  );
	printf("NEW RED X:           %.40f\n", new_red_x);
	printf("VAL:                 %.40f\n", val);
	printf("CHANGE GREEN:        %.40f\n", change_green);
	printf("CHANGE RED:          %.40f\n", change_red);
	printf("SUBTRACTION:         %.40f\n", new_green_x - new_red_x);
	printf("SUBTRACTION IS ZERO: %s\n", ((new_green_x - new_red_x) == 0.f)?"yes":"no");


	return 0;
}*/

int main(int argc, char const *argv[]){
	#define A 0.2063387632369995100000000000000f
	#define B 0.7307806611061096200000000000000f
	

	float a = A;
	float b = B;
	float floatie1 = a + (b * ((a)/(-b)));



	float step1 = ((a)/(-b));
	float step2 = (b * step1);
	float floatie2 = a + step2;



	printf("%.40f\n", floatie1);
	printf("%.40f\n", floatie2);
	return 0;
}
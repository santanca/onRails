/* CS 3GC3 - Simple lighting example
 * by R. Teather
 */
#include <stdio.h>
#include <stdlib.h>

#ifdef __APPLE__
#  include <OpenGL/gl.h>
#  include <OpenGL/glu.h>
#  include <GLUT/glut.h>
#else
#  include <GL/gl.h>
#  include <GL/glu.h>
#  include <GL/freeglut.h>
#endif
//our includes
#include "basicMathLibrary.h" 		//for 3D points and Vectors
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <time.h>
#include "Target.h"						//Target Objects
#include <math.h> 	

using namespace std; 

#define PI 3.14159265		//used for hit detection calculations

//vars to save mouse x/y coord
int mouseX = 0, mouseY = 0;

float light_pos[] = {5,10,5,1};
/*** CAMERA VARIABLES***/
vector<point3D*> *cameraPos = new vector<point3D*>;
vector<point3D*> *lookAtPos = new vector<point3D*>;
vector<int> *stages = new vector<int>;
int stageNumber = 0;
int frameCounter = 0;
int cameraHeight = 1;
bool first = true;
int lookAtIndex = 0;
int cameraIndex = 0;
int cameraPosSize = 0;

/*** SCORE AND TIMER VARIABELS ***/
int health = 3;
int score = 0;
int elapsedTime;
int time1 = 0;
int timeIncr =0;


float pos[] = {0,1,0};
float camPos[] = {5, 5, 10};
float angle = 0.0f;


/*** HUD Images***/
int height = 0;
int width = 0;
int max2 = 0;
GLubyte *image_data;
GLubyte *images[4];

int side = 0;
int up = 0;
float ang = 0.0f;

/*** ENEMIES AND TARGET LISTS ***/
vector<Target> targetList;



//Initialize Target, and target positions
void createTargetList(){
	for (int i = 0; i < 30; i += 10){
		Target t(i,5,0,1,0.2);
		targetList.push_back(t);
	}
}


//insert a point into the cameraPos vector
void insertPoint3D(point3D *p){
	cameraPos->push_back(p);
}

/*Insert a point into the lookAt vector
  For each stage, there is a different point where the camera will be looking at	
*/
void insertPoint3DLookAt(point3D *p){
	lookAtPos->push_back(p);
}


vector<vec3D> getRay(){
	//construct Ray
	GLdouble R0[3], R1[3], Rd[3];
	GLdouble modelMat[16], projMat[16];
	GLint viewMat[4];

	//populate mpv matricies
	glGetDoublev(GL_MODELVIEW_MATRIX, modelMat);
	glGetDoublev(GL_PROJECTION_MATRIX, projMat);
	glGetIntegerv(GL_VIEWPORT, viewMat);

	//calculate near point
	gluUnProject(mouseX, mouseY, 0.0, modelMat, projMat, viewMat, &R0[0], &R0[1], &R0[2]);
	//calculate far point
	gluUnProject(mouseX, mouseY, 1.0, modelMat, projMat, viewMat, &R1[0], &R1[1], &R1[2]);

	//calcualte our ray from R0 and R1
	Rd[0] = R1[0] - R0[0];
	Rd[1] = R1[1] - R0[1];
	Rd[2] = R1[2] - R0[2];

	//turn ray Rd into unit ray 
	GLdouble m = sqrt(Rd[0]*Rd[0] + Rd[1]*Rd[1] + Rd[2]*Rd[2]);
	Rd[0] /= m;
	Rd[1] /= m;
	Rd[2] /= m;

	//printf("R0: %f, %f, %f | \n", R0[0], R0[1], R0[2]);
	//printf("R1: %f, %f, %f | \n", R1[0], R1[1], R1[2]);
	//printf("Rd: %f, %f, %f | \n", Rd[0], Rd[1], Rd[2]);

	vec3D vecRD(Rd[0],Rd[1],Rd[2]);		//the returning vector, vetor Rd
	vec3D vecR0(R0[0],R0[1],R0[2]);		//the returning vector, vetor Rd

	vector<vec3D> vector(2);
	vector[0] = vecR0;
	vector[1] = vecRD;

	return vector;
}

//Sphere Intersections
bool targetTest(vec3D Rd, vec3D R0, Target t){

	//At^2 + Bt + C = 0
	//A = Rd dot Rd
	//B = 2( (R0 - Pc) dot (Rd) )
	//C = ( (R0 -Pc) dot (R0 - Pc)) - r^2
	vec3D Pc(t.x,t.y,t.z);

	float A = Rd.dot(Rd);
	vec3D temp = (R0.subtract(Pc));
	float B = 2* (temp.dot(Rd));
	float C = (temp.dot(temp)) - (t.radius*t.radius);

	//check discriminant(ie d), discriminant = b^2 - 4ac
	float d = B*B - 4*A*C;

	//if d < 0 then no instersection.
	//if d = 0 ray is tangent to sphere
	//if d > 0 ray intersects sphere in two points

	if (d < 0 ){
		printf("You missed the sphere! \n");
		return false;
	}else if (d > 0){

		//if intersections, to find point of intersection
		//t = quadratic formula 
		// x = R0.x + t*Rd.x
		// y = R0.y + t*Rd.y
		// x = R0.z + t*Rd.z

		float t0 = (-1*B  + sqrt(d))/(2*A);
		//float t1 = (-1*B  - sqrt(d))/(2*A);

		//Point P
		float P[3];
		P[0] = R0.x + t0*Rd.x;
		P[1] = R0.y + t0*Rd.y;
		P[2] = R0.z + t0*Rd.z;

		//printf("Target hit! x: %f, y: %f , z: %f \n", t.x,t.y,t.z );
		printf("Hit at: x: %f , y: %f , z: %f \n", P[0], P[1], P[2]);
		return true;
	}
	return false;

}



//Checks Intersections with all Target Objects
void targetIntersections(vec3D Rd, vec3D R0){
	
	vector<int> targetHits;			//the int will be like the id of the targets that are hit
									//the id is the index in the TargetList Vector Array
	for (int i = 0; i < targetList.size(); i++){
		bool hit = targetTest(Rd,R0,targetList[i]);
		if (hit == true){
			targetHits.push_back(i);
		}
	}

	//delete hit objects
	for (int i = 0 ; i<targetHits.size(); i++){
		//printf("Target id: %i \n", targetHits[i]);
		targetList.erase( targetList.begin() + targetHits[i], targetList.begin() + targetHits[i]+1  );
		printf("Hit & Deleted Target id: %i \n", targetHits[i]);
	}

}

//Draw all the text onto the screen such as Score and time
void DrawText(){
	glPushMatrix();
		glLoadIdentity();
		//calculate time
		time1 =60-((elapsedTime)/250);
		string str = to_string(time1+timeIncr);
		glDisable(GL_LIGHTING);
		glColor3f(1,1,1);

		//Draw Time
		glTranslatef(370,730,0);
		glScalef(0.5,0.5,1);
		glutStrokeString(GLUT_STROKE_ROMAN, (unsigned char*)str.c_str());

		//Draw Score
		glLoadIdentity();
		str = "Score: 0";
		glTranslatef(10,775,0);
		glScalef(0.20,0.20,1);
		glutStrokeString(GLUT_STROKE_ROMAN, (unsigned char*)str.c_str());
		glEnable(GL_LIGHTING);

	glPopMatrix();

}

//Draw the HUD
void DrawHUD(){
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, 800, 0, 800);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glRasterPos2i(800,800-height);
	glPixelZoom(-1, 1);
	glDrawPixels(width,height,GL_RGB, GL_UNSIGNED_BYTE, image_data);
	glFlush(); 
}

//Draw Floor
void DrawFloor(){
	glPushMatrix();
		glScalef(100,0.5,100);
		glutSolidCube(1);
	glPopMatrix();
}

//Draw Targets
void drawTargets(){

	for (int i = 0; i < targetList.size(); i++){
		targetList[i].draw(cameraPos->at(cameraIndex)->x,cameraPos->at(cameraIndex)->z);
	}

}

//Draws the 3D scene
void Draw3DScene(){
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45, 1, 1, 100);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity(); 

	float origin[] = {0,0,0,1};
	float m_amb[] = {0.33, 0.22, 0.03, 1.0};
	float m_dif[] = {0.78, 0.57, 0.11, 1.0};
	float m_spec[] = {0.99, 0.91, 0.81, 1.0};
	float shiny = 27;

	/*float m_amb[] = {0.3, 0, 0.0, 1.0};
	float m_dif[] = {0.6, 0, 0, 1.0};
	float m_spec[] = {0.8, 0.6, 0.6, 1.0};
	float shiny = 32;*/

	gluLookAt(cameraPos->at(cameraIndex)->x, cameraHeight, cameraPos->at(cameraIndex)->z,
			  lookAtPos->at(lookAtIndex)->x,lookAtPos->at(lookAtIndex)->y,lookAtPos->at(lookAtIndex)->z ,
			  0,1,0);
	DrawFloor();

	/*glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, m_amb);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, m_dif);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, m_spec);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shiny);
	*/
	glPushMatrix();
		glTranslatef(side,up,0);
		glTranslatef(0,2,0);
		glRotatef(ang, 0,1,0);
		glutSolidTeapot(1);
		glColor3f(0,1,0);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-25,1,5);
	glutSolidCube(1);
	glPopMatrix();

	glPushMatrix();
		glColor3f(1,0,0);
		glTranslatef(7,1,-15);
		glutSolidSphere(1,50,50);

	glPopMatrix();

	drawTargets();
}

GLubyte* LoadPPM(char* file, int* width, int* height, int* max)
{
	GLubyte* img;
	FILE *fd;
	int n, m;
	int  k, nm;
	char c;
	int i;
	char b[100];
	float s;
	int red, green, blue;
	
	/* first open file and check if it's an ASCII PPM (indicated by P3 at the start) */
	fd = fopen(file, "r");
	fscanf(fd,"%[^\n] ",b);
	if(b[0]!='P'|| b[1] != '3')
	{
		printf("%s is not a PPM file!\n",file); 
		exit(0);
	}
	printf("%s is a PPM file\n", file);
	fscanf(fd, "%c",&c);

	/* next, skip past the comments - any line starting with #*/
	while(c == '#') 
	{
		fscanf(fd, "%[^\n] ", b);
		printf("%s\n",b);
		fscanf(fd, "%c",&c);
	}
	ungetc(c,fd); 

	/* now get the dimensions and max colour value from the image */
	fscanf(fd, "%d %d %d", &n, &m, &k);

	printf("%d rows  %d columns  max value= %d\n",n,m,k);

	/* calculate number of pixels and allocate storage for this */
	nm = n*m;
	img = (GLubyte*)malloc(3*sizeof(GLuint)*nm);
	s=255.0/k;

	/* for every pixel, grab the read green and blue values, storing them in the image data array */
	for(i=0;i<nm;i++) 
	{
		fscanf(fd,"%d %d %d",&red, &green, &blue );
		img[3*nm-3*i-3]=red*s;
		img[3*nm-3*i-2]=green*s;
		img[3*nm-3*i-1]=blue*s;
	}

	/* finally, set the "return parameters" (width, height, max) and return the image array */
	*width = n;
	*height = m;
	*max = k;

	return img;
}


void click(){

	//get the ray picking vector
	vector<vec3D> vector = getRay();
	vec3D Rd = vector[1];
	vec3D R0 = vector[0];
/*	
	printf("--------------- \n ");
	printf("Rd: %f, %f, %f |  \n", Rd.x, Rd.y, Rd.z);
	printf("R0: %f, %f, %f | \n", R0.x, R0.y, R0.z);
*/
	//check floor intersections
	//floorIntersection(Rd,R0);

	//check for target intersections
	targetIntersections(Rd,R0);

	//calculate if you hit an enemy



	/*
	-Search through list of objects in the Scene Graph and test Intersections with all of them
	-keep track of all the objects I hit, but only select the closest one
	*/


}

//handles the mouse events
void mouse(int btn, int state, int x, int y){

	mouseX = x;
	mouseY = 800 - y;

	//create a bump if the left mouse button is pressed
	if(btn == GLUT_LEFT_BUTTON){
		if(state == GLUT_DOWN){
			//printf("Left click %i , %i \n", mouseX, mouseY);
			click();
		}

	//create a dent if the right button is pressed
	}else if(btn == GLUT_RIGHT_BUTTON){
		if(state == GLUT_DOWN){
			printf("Right click %i , %i \n", mouseX, mouseY);
			click();
		}
	}

	glutPostRedisplay();

}

//Passive keyboard function
void passive(int x, int y){
	mouseX = x;
	mouseY = 800 - y;
}


void keyboard(unsigned char key, int x, int y)
{

	/* key presses move the cube, if it isn't at the extents (hard-coded here) */
	switch (key)
	{
		case 'q':
		case 27:
			exit (0);
			break;

		case 'a':
		case 'A':
			if(light_pos[0] > -15)
				light_pos[0]-=1;
			break;

		case 'w':
		case 'W':
			if(light_pos[2] > -15)
				light_pos[2] -= 1;
			break;

		case 'd':
		case 'D':
			if(light_pos[0] < 15)
				light_pos[0]+=1;
			break;

		case 's':
		case 'S':
			if(light_pos[2] < 15)
				light_pos[2] += 1;
			break;

		case 'r':
			if(light_pos[1] < 15)
				light_pos[1] += 1;
			break;

		case 'f':
			if(light_pos[1] > 0)
				light_pos[1] -= 1;
			break;
		case 'x':
			//printf("cameraIndex %i, Size: %i  \n",cameraIndex, cameraPos->size()-1 );
			if(cameraIndex < cameraPos->size()-1){
				cameraIndex++;
			}
			break;
		case 'z':
			if(first == true){
				first = false;
			}else{
				if(stageNumber < stages->size() -1){
					stageNumber++;
				}
			}
			break;
		case ' ':
			if(cameraHeight == 1){
				cameraHeight = 8;
			}else{
				cameraHeight = 1;
			}
			break;
		case 'v':
			if(lookAtIndex < lookAtPos->size()-1){
				lookAtIndex++;
			}else{
				lookAtIndex = 0;
			}
			break;
		case 'b':
			//image_data = LoadPPM("h2.ppm", &width, &height, &max2);
			health--;
			break;
		case 'n':
			//image_data = LoadPPM("h2.ppm", &width, &height, &max2);
			timeIncr +=10;
			break;


	}

	glutPostRedisplay();
}

void special(int key, int x, int y)
{
	/* arrow key presses move the camera */
	switch(key)
	{
		case GLUT_KEY_LEFT:
			camPos[0]-=0.1;
			break;

		case GLUT_KEY_RIGHT:
			camPos[0]+=0.1;
			break;

		case GLUT_KEY_UP:
			camPos[2] -= 0.1;
			break;

		case GLUT_KEY_DOWN:
			camPos[2] += 0.1;
			break;
		
		case GLUT_KEY_HOME:
			camPos[1] += 0.1;
			break;

		case GLUT_KEY_END:
			camPos[1] -= 0.1;
			break;

	}
	glutPostRedisplay();
}

/***
Given the start point and end point locations, This method
will interpolate steps number of location in between the start
and end locations. (Giving a smooth animation)
***/
void getSlopeVector(point3D *start, point3D *end, int steps){
	float x1 = start->x;
	float x2 = end->x;
	float y = start->y;

	float z1 = start->z;
	float z2 = end->z;

	float run = (x2 -x1)/ (float) steps;
	float rise = (z2 - z1)/ (float) steps;

	//add the camera position for each frame
	insertPoint3D(start);
	frameCounter++;

	//interpolate all the points between the start and end points
	for (int i = 0; i < steps; ++i)
	{

		point3D *p = new point3D(x1+run, y, z1+rise);
		insertPoint3D(p);
		x1 +=run;
		z1 += rise;
		frameCounter++;
	}

	insertPoint3D(end);
	frameCounter++;
	stages->push_back(frameCounter);

}

//Splits string by a delimiter
void split(const string &s, char delim, vector<string> &elems){
	stringstream ss;
	ss.str(s);
	string item;
	while(getline(ss, item, delim)){
		elems.push_back(item);
	}
}

//splits the string and returns as a vector
vector<string> split(const string &s, char delim){
	vector<string> elems;
	split(s,delim, elems);
	return elems;
}

//Load the positions of where the camera should be looking at
void loadLookAtPosition(){
	string line;
	ifstream myfile ("lookAtPositions.txt");
	if(myfile.is_open()){
		//iterate through the file line by line
		while(getline (myfile,line)){
			vector<string> sub = split(line, ' ');
			//insert each point into the vector of look at positions
			if(sub.size() > 1){
				point3D *p= new point3D(stoi(sub.at(0)),stoi(sub.at(1)),stoi(sub.at(2)));
				insertPoint3DLookAt(p);
			}
		}

	}else{
		cout << "Unable to open file.";
	}

}

//Load the camera positions from textfile
void loadCameraPoints(){
	string line;
	ifstream myfile ("cameraPositions.txt");
	if (myfile.is_open()){
		point3D *start;
		point3D *end;
		bool first = true;

		//iterate through the text file line by line
		while(getline (myfile,line)){
			//split the line into a vector
			vector<string> sub = split(line, ' ');

			if(sub.size() > 1){
				if(first == true){
					start = new point3D(stoi(sub.at(0)),stoi(sub.at(1)), stoi(sub.at(2)));
					cout<<"Start " <<sub.at(0) << " " + sub.at(1) << " "+ sub.at(2) << endl;
					first = false;
				}else{
					end = new point3D(stoi(sub.at(0)),stoi(sub.at(1)), stoi(sub.at(2)));
					getSlopeVector(start,end,15);
					start = new point3D(stoi(sub.at(0)),stoi(sub.at(1)), stoi(sub.at(2)));
				}		
			}
		}
		myfile.close();
	}else{
		cout << "Unable to open file";
	}
}

//Display the proper health bar status of the character
void ManageHealth(){
	if(health == 3){ //Full Health
		image_data = images[0];
	}else if (health ==2){ //Medium Health
		image_data = images[1];
	}else if (health ==1){	//Low Health
		image_data = images[2];
	}else if (health == 0){	//Empty Health
		image_data = images[3];
	}else{
		health = 3;
	}
}

//Initialize variables and Hud images
void init(void)
{
	images[0] = LoadPPM("HUD/h1.ppm", &width, &height, &max2);
	images[1] = LoadPPM("HUD/h2.ppm", &width, &height, &max2);
	images[2] = LoadPPM("HUD/h3.ppm", &width, &height, &max2);
	images[3] = LoadPPM("HUD/h4.ppm", &width, &height, &max2);
	image_data = images[0];
	glClearColor(0, 0, 0, 0);
	glColor3f(1, 1, 1);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	float position[4] = {1.5,0,0, 0};

	float amb[4] = {1.0, 1, 1, 1};
	float diff[4] = {1,0,0, 1};
	float spec[4] = {0,0,1, 1};


	glLightfv(GL_LIGHT1, GL_POSITION, position);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, diff);
	glLightfv(GL_LIGHT1, GL_AMBIENT, amb);
	glLightfv(GL_LIGHT1, GL_SPECULAR, spec);

	loadCameraPoints();
	loadLookAtPosition();
	cameraPosSize = 0;

	printf("Stage Size: %i\n", stages->size() );

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//glOrtho(-2, 2, -2, 2, -2, 2);
	//glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
	gluPerspective(45, 1, 1, 100);

	//Initialze Targets
	createTargetList();
	//elapsedTime = time (NULL);
		
}

//OpenGl function that handles the frames per second
void FPS(int val){
	glutPostRedisplay();
	//printf("Stage number: %i\n", stageNumber );
	if(first == false){
		if(cameraIndex < cameraPos->size()-1 && cameraIndex <=stages->at(stageNumber)){
			cameraIndex++;
			glutTimerFunc(100,FPS,100);
		}
	}
}



/* display function - GLUT display callback function
 *		clears the screen, sets the camera position, draws the ground plane and movable box
 */
void display(void)
{

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	elapsedTime = glutGet(GLUT_ELAPSED_TIME);
	DrawHUD();
	DrawText();
	Draw3DScene();
	ManageHealth();

	glutTimerFunc(100,FPS,0);
	glutSwapBuffers(); 


}

/* main function - program entry point */
int main(int argc, char** argv)
{
	glutInit(&argc, argv);		//starts up GLUT
	
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	
	
	glutInitWindowSize(800, 800);
	glutInitWindowPosition(100, 100);

	glutCreateWindow("On Rails");	//creates the window

	glutDisplayFunc(display);	//registers "display" as the display callback function
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutPassiveMotionFunc(passive);
	glutSpecialFunc(special);

	glEnable(GL_DEPTH_TEST);
	init();

	glutMainLoop();				//starts the event loop

	return(0);					//return may not be necessary on all compilers
}
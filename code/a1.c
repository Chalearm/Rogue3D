
/* Derived from scene.c in the The OpenGL Programming Guide */
/* Keyboard and mouse rotation taken from Swiftless Tutorials #23 Part 2 */
/* http://www.swiftless.com/tutorials/opengl/camera2.html */

/* Frames per second code taken from : */
/* http://www.lighthouse3d.com/opengl/glut/index.php?fps */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "graphics.h"

extern GLubyte  world[WORLDX][WORLDY][WORLDZ];

   /* mouse function called by GLUT when a button is pressed or released */
void mouse(int, int, int, int);

   /* initialize graphics library */
extern void graphicsInit(int *, char **);

   /* lighting control */
extern void setLightPosition(GLfloat, GLfloat, GLfloat);
extern GLfloat* getLightPosition();

   /* viewpoint control */
extern void setViewPosition(float, float, float);
extern void getViewPosition(float *, float *, float *);
extern void getOldViewPosition(float *, float *, float *);
extern void setOldViewPosition(float, float, float);
extern void setViewOrientation(float, float, float);
extern void getViewOrientation(float *, float *, float *);

   /* add cube to display list so it will be drawn */
extern void addDisplayList(int, int, int);

   /* mob controls */
extern void createMob(int, float, float, float, float);
extern void setMobPosition(int, float, float, float, float);
extern void hideMob(int);
extern void showMob(int);

   /* player controls */
extern void createPlayer(int, float, float, float, float);
extern void setPlayerPosition(int, float, float, float, float);
extern void hidePlayer(int);
extern void showPlayer(int);

   /* tube controls */
extern void createTube(int, float, float, float, float, float, float, int);
extern void hideTube(int);
extern void showTube(int);

   /* 2D drawing functions */
extern void  draw2Dline(int, int, int, int, int);
extern void  draw2Dbox(int, int, int, int);
extern void  draw2Dtriangle(int, int, int, int, int, int);
extern void  set2Dcolour(float []);


   /* flag which is set to 1 when flying behaviour is desired */
extern int flycontrol;
   /* flag used to indicate that the test world should be used */
extern int testWorld;
   /* flag to print out frames per second */
extern int fps;
   /* flag to indicate the space bar has been pressed */
extern int space;
   /* flag indicates the program is a client when set = 1 */
extern int netClient;
   /* flag indicates the program is a server when set = 1 */
extern int netServer; 
   /* size of the window in pixels */
extern int screenWidth, screenHeight;
   /* flag indicates if map is to be printed */
extern int displayMap;
   /* flag indicates use of a fixed viewpoint */
extern int fixedVP;

   /* frustum corner coordinates, used for visibility determination  */
extern float corners[4][3];

   /* determine which cubes are visible e.g. in view frustum */
extern void ExtractFrustum();
extern void tree(float, float, float, float, float, float, int);

   /* allows users to define colours */
extern int setUserColour(int, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat,
    GLfloat, GLfloat, GLfloat);
void unsetUserColour(int);
extern void getUserColour(int, GLfloat *, GLfloat *, GLfloat *, GLfloat *,
    GLfloat *, GLfloat *, GLfloat *, GLfloat *); 

/********* end of extern variable declarations **************/

struct Point
{
   int x;
   int y;
   int z;
};
struct Wall
{
   struct Point StartPoint;
   int height;
   int width;
   int XorZSide; //0 = X side, 1 = Y side
   int color;
};
struct Room
{
   struct Point StartPoint;        //count from ground point
   struct Wall Walls[4];          // ID=0(WEST).1(EAST),2(SOUTH),3(NORTH)
   struct Point DoorPosition[4];  // if point = {-1,-1,-1} = not defined, ID=0(WEST).1(EAST),2(SOUTH),3(NORTH)
   int doorHeight;
   int doorWidth;
   int haveRoof; // 0 = no, 1 = yes
   int haveGround; //0 = no, 1 = yes
   int Roofcolor;
   int Groundcolor;

};
/*
WestWall;
   struct Wall EastWall;
   struct Wall SouthWall;
   struct Wall NorthWall;
*/

#define WEST 0
#define EAST 1
#define SOUTH 2
#define NORTH 3
#define X_SIDE_WALL 0
#define Z_SIDE_WALL 1
#define NOT_HAVE_ROOF 0
#define NOT_HAVE_GROUND 0
#define HAVE_ROOF 1
#define HAVE_GROUND 1
void SetMAXandMINPoint(const int** MaxPoint,const int** MinPoint,const int* P1,const int* P2);
void BuildABox(const struct Point *StartPoint,const struct Point *Endpoint,int color);
void BuildAWall(const struct Wall *AWall);
struct Room BuildEasyRoom(const struct Point *StartPoint,int xLenght,int zLenght,int height,int haveRoof,int haveGround,int color);
void BuildARoom(const struct Room *ARoom);
void BuildDoorsWestVsEast(int roomID,struct Room *Room1,struct Room *Room2,int DirectionR1,int DirectionR2);
void BuildDoorsSouthVsNorth(int roomID,struct Room *Room1,struct Room *Room2,int DirectionR1,int DirectionR2);

#define AREA_XP 0
#define AREA_ZP 1
#define AREA_X_LENGHT 2
#define AREA_Z_LENGHT 3

      int areaAndDoorPosition[9][4]= {{0,0,33,33},
                        {33,0,34,33},
                        {67,0,33,33},
                        {0,33,33,34},
                        {33,33,34,34},
                        {67,33,33,34},
                        {0,67,33,33},
                        {33,67,34,33},
                        {67,67,33,33}};

int DoorDirections[9][4] = {{1,1,-1,3},
                            {0,2,-1,4},
                            {1,-1,-1,5},
                            {-1,4,0,6},
                            {3,5,1,7},
                            {4,-1,2,8},
                            {-1,7,3,-1},
                            {6,8,4,-1},
                            {7,-1,5,-1}};

   /*** collisionResponse() ***/
   /* -performs collision detection and response */
   /*  sets new xyz  to position of the viewpoint after collision */
   /* -can also be used to implement gravity by updating y position of vp*/
   /* note that the world coordinates returned from getViewPosition()
      will be the negative value of the array indices */
void collisionResponse() {

   /* your code for collisions goes here */


}


   /******* draw2D() *******/
   /* draws 2D shapes on screen */
   /* use the following functions:        */
   /* draw2Dline(int, int, int, int, int);      */
   /* draw2Dbox(int, int, int, int);         */
   /* draw2Dtriangle(int, int, int, int, int, int);   */
   /* set2Dcolour(float []);           */
   /* colour must be set before other functions are called  */
void draw2D() {

   if (testWorld) {
      /* draw some sample 2d shapes */
      if (displayMap == 1) {
         GLfloat green[] = {0.0, 0.5, 0.0, 0.5};
         set2Dcolour(green);
         draw2Dline(0, 0, 500, 500, 15);
         draw2Dtriangle(0, 0, 200, 200, 0, 200);

         GLfloat black[] = {0.0, 0.0, 0.0, 0.5};
         set2Dcolour(black);
         draw2Dbox(500, 380, 524, 388);
      }
   } else {

   /* your code goes here */

   }

}
void drawWallStacks(int x, int y, int z, int color) {
   int i;
   int ground = 21;
   int wallHeight = ground + y;


   for(i=ground; i<wallHeight; i++)
   {
      world[x][i][z] = color;
   }
}

   /*** update() ***/
   /* background process, it is called when there are no other events */
   /* -used to control animations and perform calculations while the  */
   /*  system is running */
   /* -gravity must also implemented here, duplicate collisionResponse */
void update() {
int i, j, k;
float *la;
float x, y, z;

   /* sample animation for the testworld, don't remove this code */
   /* demo of animating mobs */
   if (testWorld) {

   /* update old position so it contains the correct value */
   /* -otherwise view position is only correct after a key is */
   /*  pressed and keyboard() executes. */
#if 0
// Fire a ray in the direction of forward motion
float xx, yy, zz;
getViewPosition(&x, &y, &z);
getOldViewPosition(&xx, &yy, &zz);
printf("%f %f %f %f %f %f\n", xx, yy, zz, x, y, z);
printf("%f %f %f\n",  -xx+((x-xx)*25.0), -yy+((y-yy)*25.0), -zz+((z-zz)*25.0));
createTube(2, -xx, -yy, -zz, -xx-((x-xx)*25.0), -yy-((y-yy)*25.0), -zz-((z-zz)*25.0), 5);
#endif

      getViewPosition(&x, &y, &z);
      setOldViewPosition(x,y,z);

   /* sample of rotation and positioning of mob */
   /* coordinates for mob 0 */
      static float mob0x = 50.0, mob0y = 25.0, mob0z = 52.0;
      static float mob0ry = 0.0;
      static int increasingmob0 = 1;
   /* coordinates for mob 1 */
      static float mob1x = 50.0, mob1y = 25.0, mob1z = 52.0;
      static float mob1ry = 0.0;
      static int increasingmob1 = 1;
   /* counter for user defined colour changes */
      static int colourCount = 0;
      static GLfloat offset = 0.0;

   /* move mob 0 and rotate */
   /* set mob 0 position */
      setMobPosition(0, mob0x, mob0y, mob0z, mob0ry);

   /* move mob 0 in the x axis */
      if (increasingmob0 == 1)
         mob0x += 0.2;
      else 
         mob0x -= 0.2;
      if (mob0x > 50) increasingmob0 = 0;
      if (mob0x < 30) increasingmob0 = 1;

   /* rotate mob 0 around the y axis */
      mob0ry += 1.0;
      if (mob0ry > 360.0) mob0ry -= 360.0;

   /* move mob 1 and rotate */
      setMobPosition(1, mob1x, mob1y, mob1z, mob1ry);

   /* move mob 1 in the z axis */
   /* when mob is moving away it is visible, when moving back it */
   /* is hidden */
      if (increasingmob1 == 1) {
         mob1z += 0.2;
         showMob(1);
      } else {
         mob1z -= 0.2;
         hideMob(1);
      }
      if (mob1z > 72) increasingmob1 = 0;
      if (mob1z < 52) increasingmob1 = 1;

   /* rotate mob 1 around the y axis */
      mob1ry += 1.0;
      if (mob1ry > 360.0) mob1ry -= 360.0;

   /* change user defined colour over time */
      if (colourCount == 1) offset += 0.05;
      else offset -= 0.01;
      if (offset >= 0.5) colourCount = 0;
      if (offset <= 0.0) colourCount = 1;
      setUserColour(9, 0.7, 0.3 + offset, 0.7, 1.0, 0.3, 0.15 + offset, 0.3, 1.0);

   /* sample tube creation  */
   /* draws a purple tube above the other sample objects */
       createTube(1, 45.0, 30.0, 45.0, 50.0, 30.0, 50.0, 6);

    /* end testworld animation */


   } else {

   /* your code goes here */

   }
}


   /* called by GLUT when a mouse button is pressed or released */
   /* -button indicates which button was pressed or released */
   /* -state indicates a button down or button up event */
   /* -x,y are the screen coordinates when the mouse is pressed or */
   /*  released */ 
void mouse(int button, int state, int x, int y) {

   if (button == GLUT_LEFT_BUTTON)
      printf("left button - ");
   else if (button == GLUT_MIDDLE_BUTTON)
      printf("middle button - ");
   else
      printf("right button - ");

   if (state == GLUT_UP)
      printf("up - ");
   else
      printf("down - ");

   printf("%d %d\n", x, y);
}


void makeBorderRow1(int sidesLength, int endsLength, int xPosition, int zPosition, int color) {
   int i;

   sidesLength = xPosition + sidesLength;
   endsLength = zPosition + endsLength;

   printf("sidesLength = %i\n", sidesLength);

   for(i=xPosition;i<sidesLength;i++)
   {
      //left side
      drawWallStacks(i,3,0, color);
      //right side
      drawWallStacks(i,3,endsLength, color);
   }

   for(i=zPosition;i<endsLength;i++)
    {
      //back side
      drawWallStacks(0,3,i, color);
      //front side
      drawWallStacks(sidesLength-1,3,i, color);
   }
}



void makeWorld() {
   int i, j, k;
for(i=0; i<WORLDX; i++)
         for(j=0; j<WORLDY; j++)
            for(k=0; k<WORLDZ; k++)
               world[i][j][k] = 0;
}
void drawFloor()
{
   int i, j;

   for(i=0; i<WORLDX; i++) {
         for(j=0; j<WORLDZ; j++) {
            world[i][21][j] = 4;
         }
      }
}

void makeBorderRow2(int sidesLength, int endsLength, int xPosition, int zPosition, int color) {
   int i;

   sidesLength = xPosition + sidesLength;
   endsLength = zPosition + endsLength;

   for(i=xPosition;i<sidesLength;i++)
   {
      //left side
      drawWallStacks(i,3,33, color);
      //right side
      drawWallStacks(i,3,66, color);
   }
   for(i=zPosition;i<endsLength;i++)
    {
      //back side
      drawWallStacks(xPosition,3,i, color);
      //front side
      drawWallStacks(sidesLength-1,3,i, color);
   }
}

void drawBorderTest3(int sidesLength, int endsLength, int xPosition, int zPosition, int color) {
   int i;


   for(i=33;i<66;i++)
   {
      //left side
      drawWallStacks(i,3,33, color);
      //right side
      drawWallStacks(i,3,66, color);
   }

   for(i=33;i<66;i++)
    {
      //back side
      drawWallStacks(33,3,i, color);
      //front side
      drawWallStacks(66,3,i, color);
   }
}

void makeBorderRow3(int sidesLength, int endsLength, int xPosition, int zPosition, int color) {
   int i;

   sidesLength = xPosition + sidesLength;
   endsLength = zPosition + endsLength;

   for(i=xPosition;i<sidesLength;i++)
   {
      //left side
      drawWallStacks(i,3,zPosition, color);
      //right side
      drawWallStacks(i,3,endsLength, color);
   }
   for(i=zPosition;i<endsLength;i++)
    {
      //back side
      drawWallStacks(xPosition,3,i, color);
      //front side
      drawWallStacks(sidesLength-1,3,i, color);
   }
}

int getRandomNumber(int minimum, int maximum) {

   return minimum + rand() % (maximum+1 - minimum);
}
int main(int argc, char** argv)
{
int i, j, k;
   /* initialize the graphics system */
   graphicsInit(&argc, argv);
   srand(time(NULL));

   /* the first part of this if statement builds a sample */
   /* world which will be used for testing */
   /* DO NOT remove this code. */
   /* Put your code in the else statment below */
   /* The testworld is only guaranteed to work with a world of
      with dimensions of 100,50,100. */
   if (testWorld == 1) {
   /* initialize world to empty */
      for(i=0; i<WORLDX; i++)
         for(j=0; j<WORLDY; j++)
            for(k=0; k<WORLDZ; k++)
               world[i][j][k] = 0;

   /* some sample objects */
   /* build a red platform */
      for(i=0; i<WORLDX; i++) {
         for(j=0; j<WORLDZ; j++) {
            world[i][24][j] = 3;
         }
      }
   /* create some green and blue cubes */
      world[50][25][50] = 1;
      world[49][25][50] = 1;
      world[49][26][50] = 1;
      world[52][25][52] = 2;
      world[52][26][52] = 2;

   /* create user defined colour and draw cube */
      setUserColour(9, 0.7, 0.3, 0.7, 1.0, 0.3, 0.15, 0.3, 1.0);
      world[54][25][50] = 9;


   /* blue box shows xy bounds of the world */
      for(i=0; i<WORLDX-1; i++) {
         world[i][25][0] = 2;
         world[i][25][WORLDZ-1] = 2;
      }
      for(i=0; i<WORLDZ-1; i++) {
         world[0][25][i] = 2;
         world[WORLDX-1][25][i] = 2;
      }

   /* create two sample mobs */
   /* these are animated in the update() function */
      createMob(0, 50.0, 25.0, 52.0, 0.0);
      createMob(1, 50.0, 25.0, 52.0, 0.0);

   /* create sample player */
      createPlayer(0, 52.0, 27.0, 52.0, 0.0);
   } else {

   /* your code to build the world goes here */
      makeWorld();
    for(i=0; i<WORLDX; i++) {
         for(j=0; j<WORLDZ; j++) {
            //world[i][10][j] = 4;    // entire black
            world[i][10][j] = (((j%2==0)&&(i%2==1))||((j%2==1)&&(i%2==0)))?4:5;  // chessboard pattern
         }
      }

      int l,m,n,o,p;
      int xLenght = 0;
      int zLenght = 0;
      int roomX = 0;
      int roomZ = 0;
      int wallHeight = 3;
      int xViewP = 0;
      int zViewP = 0;
      int yStartP = 11;
      int GroundLv = 10;
      int numberofCubes = 2;
      int colorID1 = 3;
      const struct Point DoorInitialPoint = {-1,-1,-1};
      const int numRoom = 9;
      const int doorWidth = 5;
      const int wallColor = 1; // Green
      int ViewPointID = getRandomNumber(0,8); //which room the view point will be
      int doorHeight = 2;
      int oppositeRoomID = -1;
      int sparForCorridorsX = 4;
      int sparForCorridorsZ = 4;
      int sparForRoomSizeMax = 24;  // sparForRoomSize >= 3+doorWidth
      int sparForRoomSizeMin = 8;
      struct Room Rooms[9];
      
      // Initial value or set default value to Door position Point (X,Y,Z)
      for(k=0;k<numRoom;k++)
         for (i = WEST; i <=NORTH;i++)
            Rooms[k].DoorPosition[i] = DoorInitialPoint;
      // Build 9 Rooms
      for(k = 0;k<numRoom;k++)
      {
         // Find spar area between 2 rooms to build corridors and hallways
         sparForCorridorsX = (areaAndDoorPosition[k][AREA_X_LENGHT] - sparForRoomSizeMax)/2;
         sparForCorridorsZ = (areaAndDoorPosition[k][AREA_Z_LENGHT] - sparForRoomSizeMax)/2;
         // define door direction to Room attribute

         // random size of a room
         xLenght = getRandomNumber(sparForRoomSizeMin,sparForRoomSizeMax);
         zLenght = getRandomNumber(sparForRoomSizeMin,sparForRoomSizeMax);
         // Random location of a room
         roomX = sparForCorridorsX + areaAndDoorPosition[k][AREA_XP] + getRandomNumber(0,areaAndDoorPosition[k][AREA_X_LENGHT]-xLenght-sparForCorridorsX-sparForCorridorsX);
         roomZ=  sparForCorridorsZ + areaAndDoorPosition[k][AREA_ZP] + getRandomNumber(0,areaAndDoorPosition[k][AREA_Z_LENGHT]-zLenght-sparForCorridorsZ-sparForCorridorsZ);

         // Reference point to build a room
         const struct Point RoomStartPoint = {roomX,GroundLv,roomZ};
         // find view point
         if (k == ViewPointID)
         {
            xViewP = 2 + RoomStartPoint.x + getRandomNumber(0,xLenght-4);
            zViewP = 2 + RoomStartPoint.z + getRandomNumber(0,zLenght-4);
         }
         // create a few Cubes 1 high
         numberofCubes = getRandomNumber(1,2);
         for(i = 0; i < numberofCubes;i++)
            world[2 + roomX + getRandomNumber(0,xLenght-4)][yStartP][2 + roomZ  + getRandomNumber(0,zLenght -4)]= 8;
         
         // build A room
         Rooms[k]= BuildEasyRoom(&RoomStartPoint,xLenght,zLenght,wallHeight,HAVE_ROOF,NOT_HAVE_GROUND,wallColor);

         // Set and Build Doors
         Rooms[k].doorWidth = doorWidth;
         Rooms[k].doorHeight = doorHeight;

         // build door and corridor for 4 directions
         // east to west
         if(DoorDirections[k][EAST]> -1) 
            BuildDoorsWestVsEast(k,&Rooms[k],&Rooms[DoorDirections[k][EAST]],EAST,WEST);
         // west to east
         if(DoorDirections[k][WEST] > -1) 
            BuildDoorsWestVsEast(k,&Rooms[k],&Rooms[DoorDirections[k][WEST]],WEST,EAST);
         // south to north
         if(DoorDirections[k][SOUTH] > -1)
            BuildDoorsSouthVsNorth(k,&Rooms[k],&Rooms[DoorDirections[k][SOUTH]],SOUTH,NORTH);
         // north to south
         if(DoorDirections[k][NORTH] > -1)
            BuildDoorsSouthVsNorth(k,&Rooms[k],&Rooms[DoorDirections[k][NORTH]],NORTH,SOUTH);

      }

   setViewPosition(-1*xViewP, -48, -1*zViewP);
   }



   /* starts the graphics processing loop */
   /* code after this will not run until the program exits */
   glutMainLoop();
   return 0; 
}



void makeWall(int numOfBlocks, int color) {
   int i = 0;
   for (i = 0; i<numOfBlocks; i++) {

   }
}



void drawBorder(int borderMinRange, int borderMaxRange)
{
   int i;

   for(i=borderMinRange; i<borderMaxRange; i++) {
      drawWallStacks(i, 3, borderMinRange,6);
      drawWallStacks(i, 3, borderMaxRange,6);    
   }
   // for(i=borderMinRange; i<borderMaxRange; i++) {
   //    drawWallStacks(borderMinRange, 3, i,6);
   //    drawWallStacks(borderMaxRange, 3, i,6);
   // }
}





void drawBorderTest4(int sidesLength, int endsLength, int xPosition, int zPosition, int color) {
   int i;


   for(i=66;i<99;i++)
   {
      //left side
      drawWallStacks(i,3,33, color);
      //right side
      drawWallStacks(i,3,66, color);
   }

   for(i=33;i<66;i++)
    {
      //back side
      drawWallStacks(66,3,i, color);
      //front side
      drawWallStacks(99,3,i, color);
   }
}






void draw_wall_3high(int x, int z, int cubeStyle)
{
   world[x][21][z] = cubeStyle;
   world[x][22][z] = cubeStyle;
   world[x][23][z] = cubeStyle;
}

void drawPillars()
{
   int i, j;

   for(i=0; i<28; i=i+4) {  
      for(j=0; j<28; j=j+4) {
         draw_wall_3high(i,j,2);
         draw_wall_3high(j,i,2);
      }
   }
}




void drawTestBlocks()
{
      /* 1 high */
      world[1][25][1] = 1;

      /* 2 high */
      world[1][25][2] = 8;
      world[1][26][2] = 8;

      /* 3 high */
      draw_wall_3high(1,3,3);
}



void SetMAXandMINPoint(const int** MaxPoint,const int** MinPoint,const int* P1,const int* P2)
{
   if(*P1 > *P2)
   {
      *MaxPoint = P1;
      *MinPoint = P2;
   }
   else
   {
      *MaxPoint = P2;
      *MinPoint = P1;
   }
}

void BuildABox(const struct Point *StartPoint,const struct Point *Endpoint,int color)
{
   const int *MaxX;
   const int *MaxY;
   const int *MaxZ;
   const int *MinX;
   const int *MinY;
   const int *MinZ;
   int i,j,k;
   SetMAXandMINPoint(&MaxX,&MinX,&(StartPoint->x),&(Endpoint->x));
   SetMAXandMINPoint(&MaxY,&MinY,&(StartPoint->y),&(Endpoint->y));
   SetMAXandMINPoint(&MaxZ,&MinZ,&(StartPoint->z),&(Endpoint->z));
   for(k=*MinZ;k<=*MaxZ;k++)
      for(j=*MinY;j<=*MaxY;j++)
         for(i=*MinX;i<=*MaxX;i++)
            world[i][j][k] = color;
}
/*
struct Wall
{
   struct Point StartPoint;
   int height;
   int width;
   int XorZSide; //0 = X side, 1 = Y side
   int color;
};*/
void BuildAWall(const struct Wall *AWall)
{

   int EndXP = 0;
   int EndYP = 0;
   int EndZP = 0;
   struct Point EndPoint;
   if(AWall->XorZSide == 0) //0 = X side, 1 = Y side  // Z is not changed
   {
      EndPoint.x = AWall->StartPoint.x + AWall->width - 1; // for example start X=1 ,length =3 --> There are 3 points (X = 1, X=2, X=3) --> End X at value = 1 + 3 -1 =3
      EndPoint.z = AWall->StartPoint.z;
   }
   else
   {
      EndPoint.x = AWall->StartPoint.x;
      EndPoint.z = AWall->StartPoint.z + AWall->width - 1; //  same idea above (Xpoint )
   }
   EndPoint.y = AWall->StartPoint.y+AWall->height -1; //  same idea above (Xpoint )
   BuildABox(&(AWall->StartPoint),&EndPoint,AWall->color);
   
}
/*
struct Room
{
   struct Point StartPoint;
   struct Wall WestWall;
   struct Wall EastWall;
   struct Wall SouthWall;
   struct Wall NorthWall;
   struct Point DoorPosition[4];  // if point = {-1,-1,-1} = not defined
   int doorHeight;
   int doorWidth;
   int haveRoof; // 0 = no, 1 = yes
   int Roofcolor;
   int Groundcolor;

};
*/
void BuildARoom(const struct Room *ARoom)
{
   int i = WEST;
   struct Point StartPoint;
   struct Point EndPoint;
   for(i=WEST;i<=NORTH;i++)
   BuildAWall(&(ARoom->Walls[i]));

   // Build Roof
   if (ARoom->haveRoof == 1)
   {
      StartPoint.x = ARoom->StartPoint.x;
      StartPoint.y = ARoom->StartPoint.y + ARoom->Walls[WEST].height+1; // Assume that all rooms have the same height, not count ground LV.
      StartPoint.z = ARoom->StartPoint.z;

      EndPoint.x = ARoom->StartPoint.x + ARoom->Walls[SOUTH].width+1;
      EndPoint.y = ARoom->StartPoint.y + ARoom->Walls[WEST].height+1;// Assume that all rooms have the same height, not count ground LV.
      EndPoint.z = ARoom->StartPoint.z + ARoom->Walls[WEST].width-1;
      BuildABox(&StartPoint,&EndPoint,ARoom->Roofcolor);
   }
   // Build Ground
   if (ARoom->haveGround == 1)
   {
      EndPoint.x = ARoom->StartPoint.x + ARoom->Walls[SOUTH].width +1;
      EndPoint.y = ARoom->StartPoint.y;
      EndPoint.z = ARoom->StartPoint.z + ARoom->Walls[WEST].width -1;
      BuildABox(&(ARoom->StartPoint),&EndPoint,ARoom->Groundcolor);
   }
}

struct Room BuildEasyRoom(const struct Point *StartPoint,int xLenght,int zLenght,int height,int haveRoof,int haveGround,int color)
{


/*   Top View of Room

                     North Wall side


                          |--- X- Width        --|
                   ____ _______________________ ____
               |  |    |_______________________|    | 
                  |    |                       |    | 
               |  |    |                       |    | 
               _  |    |                       |    | 
               Z  |    |                       |    | 
                  |    |                       |    | 
               W  |    |                       |    | 
West Wall Side i  |    |                       |    |                EAST Wall Side
               d  |    |                       |    | 
               t  |    |                       |    | 
               h_ |    |                       |    | 
               |  |    |                       |    | 
                  |    |                       |    | 
               |  |    |                       |    | 
                  |    |                       |    | 
               |  |    |                       |    |          
                   ____                         ____
               |  |    |_______________________|    | 
                  _____ _______________________ _____


                           SOUTH  wall Side
*/

   struct Room ARoom;
   int x = StartPoint->x;
   int y = StartPoint->y;
   int z = StartPoint->z;


   // Set Attribute to Wall 
   ARoom.StartPoint            = *StartPoint;
   ARoom.Groundcolor           = color;
   ARoom.Roofcolor             = color;
   ARoom.haveRoof              = haveRoof;
   ARoom.haveGround            = haveGround;
   // the wall at West side
   ARoom.Walls[WEST].StartPoint.x = x;                            
   ARoom.Walls[WEST].StartPoint.y = y+1;
   ARoom.Walls[WEST].StartPoint.z = z;
   ARoom.Walls[WEST].height       = height;
   ARoom.Walls[WEST].width        = zLenght;
   ARoom.Walls[WEST].XorZSide     = Z_SIDE_WALL;
   ARoom.Walls[WEST].color        = color;


   // the wall at East side
   ARoom.Walls[EAST].StartPoint.x = x + xLenght - 1;
   ARoom.Walls[EAST].StartPoint.y = y+1;
   ARoom.Walls[EAST].StartPoint.z = z;
   ARoom.Walls[EAST].height       = height;
   ARoom.Walls[EAST].width        = zLenght;
   ARoom.Walls[EAST].XorZSide     = Z_SIDE_WALL;
   ARoom.Walls[EAST].color        = color;

   // the wall at South side
   ARoom.Walls[SOUTH].StartPoint.x = x + 1;
   ARoom.Walls[SOUTH].StartPoint.y = y+1;
   ARoom.Walls[SOUTH].StartPoint.z = z;
   ARoom.Walls[SOUTH].height       = height;
   ARoom.Walls[SOUTH].width        = xLenght - 2;
   ARoom.Walls[SOUTH].XorZSide     = X_SIDE_WALL;
   ARoom.Walls[SOUTH].color        = color;


   // the wall at North side
   ARoom.Walls[NORTH].StartPoint.x = x + 1;
   ARoom.Walls[NORTH].StartPoint.y = y+1;
   ARoom.Walls[NORTH].StartPoint.z = z + zLenght -1;
   ARoom.Walls[NORTH].height       = height;
   ARoom.Walls[NORTH].width        = xLenght - 2;
   ARoom.Walls[NORTH].XorZSide     = X_SIDE_WALL;
   ARoom.Walls[NORTH].color        = color;
   BuildARoom(&ARoom);
   return ARoom;
}


void BuildDoorsWestVsEast(int roomID, struct Room *Room1,struct Room *Room2,int DirectionR1,int DirectionR2)
{
         int x,z,y,i,j;
         int x2=-1,z2=-1;
         int doorWidth;
         int doorHeight;
         int color = 0;
         int oppositeRoomID = -1;
         if(Room1 != Room2)
         {
            color = Room1->Roofcolor;
            doorWidth = Room1->doorWidth;
            doorHeight = Room1->doorHeight;
            oppositeRoomID = DoorDirections[roomID][DirectionR1];
            x2 = Room2->DoorPosition[DirectionR2].x;
            z2 = Room2->DoorPosition[DirectionR2].z;
            Room1->DoorPosition[DirectionR1].x = Room1->Walls[DirectionR1].StartPoint.x;
            Room1->DoorPosition[DirectionR1].y = Room1->Walls[DirectionR1].StartPoint.y;
            i = 30; // Protect infinity loop
            do {Room1->DoorPosition[DirectionR1].z = Room1->Walls[DirectionR1].StartPoint.z + getRandomNumber(1,Room1->Walls[DirectionR1].width-Room1->doorWidth-1);}
            while((Room1->DoorPosition[DirectionR1].z  == z2) &&(i-- >=0) );

            x = Room1->DoorPosition[DirectionR1].x;
            z = Room1->DoorPosition[DirectionR1].z;
            y = Room1->DoorPosition[DirectionR1].y;
            for(j = 0; j< Room1->doorHeight;j++)
             for (i = 0; i < Room1->doorWidth; i++)
               world[x][y+j][z+i] = 0;

         if((x > -1)&&(x2> -1) &&(DirectionR1 < DirectionR2))
         {//AREA_X_LENGHT

            int LL = 0;
            int dXP = (areaAndDoorPosition[oppositeRoomID][AREA_XP] + areaAndDoorPosition[oppositeRoomID][AREA_X_LENGHT]) - (doorWidth/2) -1;
            int dXL = dXP - x2;
            int dZP = z2+doorWidth;



            int aXP = dXP + doorWidth;
            int aXL = dXL + doorWidth;
            int aZP = z2-1;

            int bXP = aXP+1;
            int bXL = x - bXP;
            int bZP = z-1;

            int fXP = dXP+1;
            int fXL = x - fXP;
            int fZP = z+ doorWidth;

            int cXP = fXP-1;
            int cZP = dZP+1;
            int cZL = fZP-dZP;

            int eXP = bXP;
            int eZP = aZP;
            int eZL = bZP-aZP;
            if (z < z2)
            {
               dZP = z2-1; 
               aZP = z2+doorWidth;
               bZP = z+ doorWidth;
               fZP = z-1;
               cZP = bZP;
               cZL = 1+aZP-bZP;
               eZP = fZP;
               eZL = 1+dZP-fZP;
               cXP = aXP+1;
               eXP = dXP;


            }

            // Build Roof for Hallway
            if (z2 > z)
            {
               for(j = 0;j < doorWidth+2;j++)
                  for(i=z-1;i<=z2+doorWidth;i++)
                     world[dXP+j][y +doorHeight][i] = color;
            }
            else
            {
               for(j = 0;j < doorWidth+2;j++)
                  for(i=z2-1;i<=z+doorWidth;i++)
                     world[dXP+j][y +doorHeight][i] = color;
            }
            LL = bXL;
            if (fXL < bXL ) LL = fXL;
            for(j = 1;j < LL;j++)
               for(i=0;i < doorWidth+2;i++)
                  world[bXP+j][y +doorHeight][z-1+i]= color; 
            LL = aXL;
            for(j = 0;j < LL-doorWidth-1;j++)
               for(i=0;i < doorWidth+2;i++)
                  world[x2+j+1][y +doorHeight][z2-1+i]= color;

            // Build hallway's walls
            for(j = 0;j < doorHeight;j++)
            {
               LL = bXL;
               if (fXL > bXL ) LL = fXL;
               for (i = 0; i < LL;i++)
               {
                  if(i <bXL)world[bXP+i][y +j][bZP] = color;
                  if(i <fXL)world[fXP+i][y +j][fZP] = color;
               }
               for (i = 0; i < aXL;i++)
               {
                   world[x2+i+1][y +j][aZP] = color;
                   if(i < dXL)world[x2+i+1][y+j][dZP] = color;
               }
               LL=cZL;
               if (cZL <eZL) LL =eZL;
               for(i = 0;i<LL;i++)
               {
                  if(i<cZL)world[cXP][y +j][cZP+i] = color;
                  if(i<eZL)world[eXP][y +j][eZP+i] = color;
               }


            }

          }
         }
}
void BuildDoorsSouthVsNorth(int roomId,struct Room *Room1,struct Room *Room2,int DirectionR1,int DirectionR2)
{
      int x,z,y,i,j;
      int x2=-1,z2=-1;
      int doorWidth;
      int doorHeight;
      int color = 0;
      int oppositeRoomID = -1;
      if(Room1 != Room2)
      {
            color = Room1->Roofcolor;
            doorWidth = Room1->doorWidth;
            doorHeight = Room1->doorHeight;
            oppositeRoomID = DoorDirections[roomId][DirectionR1];
            Room1->DoorPosition[DirectionR1].z = Room1->Walls[DirectionR1].StartPoint.z;
            Room1->DoorPosition[DirectionR1].y = Room1->Walls[DirectionR1].StartPoint.y;
            x2 = Room2->DoorPosition[DirectionR2].x;
            z2 = Room2->DoorPosition[DirectionR2].z;
            i = 30; // Protect infinity loop
            do {Room1->DoorPosition[DirectionR1].x = Room1->Walls[DirectionR1].StartPoint.x + getRandomNumber(0,Room1->Walls[DirectionR1].width-Room1->doorWidth-1);}
            while((Room1->DoorPosition[DirectionR1].x  == x2) &&(i-- >=0) );

            x = Room1->DoorPosition[DirectionR1].x;
            z = Room1->DoorPosition[DirectionR1].z;
            y = Room1->DoorPosition[DirectionR1].y;
            for(j = 0; j< doorHeight;j++)
             for (i = 0; i < doorWidth; i++)
               world[x+i][y+j][z] = 0;


         // south and north side 
            
         if((z > -1)&&(z2 > -1) &&(DirectionR1 < DirectionR2))
         {
            int LL = 0;
            int dZP = (areaAndDoorPosition[oppositeRoomID][AREA_ZP] + areaAndDoorPosition[oppositeRoomID][AREA_Z_LENGHT]) - (doorWidth/2)-1;
            int dZL = dZP - z2;
            int aZP = dZP + doorWidth;
            int aZL = dZL + doorWidth;
            int aXP = x2-1;
            int dXP = x2+doorWidth;
            int bZP = aZP+1;
            int bZL = z - bZP;
            int bXP = x-1;

            int fZP = dZP+1;
            int fZL = z - fZP;
            int fXP = x+ doorWidth;

            int cZP = fZP-1;
            int cXP = dXP+1;
            int cXL = fXP-dXP;

            int eZP = bZP;
            int eXP = aXP;
            int eXL = bXP-aXP;
            if (x < x2)
            {
               dXP = x2-1; 
               aXP = x2+doorWidth;
               bXP = x+ doorWidth;
               fXP = x-1;
               cXP = bXP;
               cXL = 1+aXP-bXP;
               eXP = fXP;
               eXL = 1+dXP-fXP;
               cZP = aZP+1;
               eZP = dZP;


            }
            // Build hallway's walls
            for(j = 0;j <doorHeight;j++)
            {
               LL = bZL;
               if (fZL > bZL ) LL = fZL;
               for (i = 0; i < LL;i++)
               {
                  if(i <bZL)world[bXP][y+j][bZP+i] = color;
                  if(i <fZL)world[fXP][y+j][fZP+i] = color;
               }

               for (i = 0; i < aZL;i++)
               {
                   world[aXP][y+j][z2+i+1] = color;
                   if(i < dZL)world[dXP][y+j][z2+i+1] = color;
               }
               LL=cXL;
               if (cXL <eXL) LL =eXL;
               for(i = 0;i<LL;i++)
               {
                  if(i<cXL)world[cXP+i][y+j][cZP] = color;
                  if(i<eXL)world[eXP+i][y+j][eZP] = color;
               }

            }
            // Build hallway's Roof 
            LL = bZL;
            if (fZL < bZL ) LL = fZL;
            for(j = 1;j < LL;j++)
               for(i=0;i < doorWidth+2;i++)
                  world[x-1+i][y+doorHeight][bZP+j]= color; 
               
            LL = aZL;
            for(j = 0;j < LL-doorWidth-1;j++)
               for(i=0;i < doorWidth+2;i++)
                  world[x2-1+i][y+doorHeight][z2+j+1]= color; 
            if (x2 > x)
            {
               for(j = 0;j < doorWidth+2;j++)
                  for(i=x-1;i<=x2+doorWidth;i++)
                     world[i][y+doorHeight][dZP+j] = color;
            }
            else
            {
               for(j = 0;j < doorWidth+2;j++)
                  for(i=x2-1;i<=x+doorWidth;i++)
                     world[i][y+doorHeight][dZP+j] = color;
            }
         }
      }
}


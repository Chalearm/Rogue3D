
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

	/* texture functions */
extern int setAssignedTexture(int, int);
extern void unsetAssignedTexture(int);
extern int getAssignedTexture(int);
extern void setTextureOffset(int, float, float);


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

	/* mesh creation, translatio, rotation functions */
extern void setMeshID(int, int, float, float, float);
extern void unsetMeshID(int);
extern void setTranslateMesh(int, float, float, float);
extern void setRotateMesh(int, float, float, float);
extern void setScaleMesh(int, float);
extern void drawMesh(int);
extern void hideMesh(int);

/********* end of extern variable declarations **************/

/********* end of extern variable declarations **************/

#define NUM_TILE_X 50
#define NUM_TILE_Z 50
#define MAX_NUM_LINE_IN_MAP 100
#define MAX_NUM_STAIR_IN_MAP 2


#define UP_STAIR 0
#define DOWN_STAIR 1
#define DEFAULT_NUM_ROOM 9
#define DEFAULT_UNDERGROUND_LV 10
#define DEFAULT_DOOR_HEIGHT 2
#define DEFAULT_DOOR_WIDTH 2
#define DEFAULT_ROOM_HEIGHT 3
#define DEFAULT_GRAVITY 21
#define DEFAULT_COLLISION_MARGIN 0.4
#define DEFAULT_SPARFORCORRIDORS_X 4
#define DEFAULT_SPARFORCORRIDORS_Z 4
#define DEFAULT_ROOM_SIZE_MAX 20
#define DEFAULT_ROOM_SIZE_MIN 5
#define DEFAULT_ROOM_COLOR 1 // Green 
#define DEFAULT_CUBE_COLOR 8

#define DEFAULT_VIEW_POINT_X 1
#define DEFAULT_VIEW_POINT_Y 48
#define DEFAULT_VIEW_POINT_Z 1

#define DEFAULT_HIGHEST_TERRAIN 36
#define DEFAULT_LOWEST_TERRAIN 7

#define DEFAULT_STAIR_STEP_NUM 4
#define DEFAULT_STAIR_WIDTH 3

#define DEFAULT_HIGHEST_CLOUD 48
#define DEFAULT_LOWEST_CLOUD 44
#define DEFAULT_CLOUD_LV_NUM DEFAULT_HIGHEST_CLOUD-DEFAULT_LOWEST_CLOUD

#define DEFAULT_MAX_WIDHTX_CLOUD_SIZE 10
#define DEFAULT_MAX_WIDHTZ_CLOUD_SIZE 10
#define DEFAULT_MAX_HIEGHT_CLOUD_SIZE DEFAULT_CLOUD_LV_NUM-1

#define DEFAULT_HIGHEST_TERRAIN_NUM 321
#define DEFAULT_LOWEST_TERRAIN_NUM 256

#define DEFAULT_MAP_2D_SCALE_VAL 7.0

struct Point2D
{
  int x;
  int z;
};

struct LineOrBox2D
{
  struct Point2D startP;
  struct Point2D stopP;
  int width; // 0 is solid Box, 1 ... n for line width
  GLfloat colorAttrib[4];
};

struct FogMap
{
  struct LineOrBox2D fogTiles[NUM_TILE_X][NUM_TILE_Z];
};

struct Map
{
  struct FogMap aFogMap;
  struct LineOrBox2D lines[MAX_NUM_LINE_IN_MAP];
  struct LineOrBox2D roomsPos[DEFAULT_NUM_ROOM];
  struct LineOrBox2D doorsPos[DEFAULT_NUM_ROOM][4];
  struct LineOrBox2D stairsPos[MAX_NUM_STAIR_IN_MAP];
  struct LineOrBox2D cubesPos[DEFAULT_NUM_ROOM][2];
  int numLines;
  int numStairs;
  int numCubes[DEFAULT_NUM_ROOM];
  int numDoors[DEFAULT_NUM_ROOM];
};

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
   int (*generateColorStyle)(int,int,int,int);
};

struct Room
{
   struct Point StartPoint;      //count from ground point
   struct Wall Walls[4];         // ID=0(WEST).1(EAST),2(SOUTH),3(NORTH)
   struct Point DoorPosition[4]; // if point = {-1,-1,-1} = not defined, ID=0(WEST).1(EAST),2(SOUTH),3(NORTH)
   int doorHeight;
   int doorWidth;
   int haveRoof;   // 0 = no, 1 = yes
   int haveGround; //0 = no, 1 = yes
   int Roofcolor;
   int Groundcolor;
   int unitCubeColor;
   int numUnitCubes; // 1 or 2
   struct Point unitCubePoint[2];

};

struct stair
{
   struct Point StartPoint; 
   struct Room aRoom;
   int color;
   int numberStair;
   int direction;
   int type; //0 = up, 1 = down
};


struct Underground
{
   // method interface

   // attribute
   // m_ = member of struct
   struct Map m_a2DMap;
   int m_groundLv;
   int m_downStairGroundLv;
   int m_doorHeight;
   int m_doorWidth;
   int m_roomWallHeight;
   float m_gravity;
   float m_collisionMargin;
   int m_sparForCorridorsX;
   int m_sparForCorridorsZ;
   int m_sparForRoomSizeMax;  // sparForRoomSize >= 3+doorWidth
   int m_sparForRoomSizeMin;
   int m_defaultRoomColor;
   int m_defaultUnitCubeColor;
   unsigned char m_stairOption; // 0 = no stair, 1 = only upstair, 2 = only down stair, 3 both up and down stairs
   struct stair m_upStair;
   struct stair m_downStair;
   int m_upStairRoomId;
   int m_downStairRoomId;
   struct Room m_rooms[DEFAULT_NUM_ROOM];
   struct Point m_upViewPoint;
   struct Point m_downViewPoint;
   int m_state;
   int m_visitedState;

};

struct OnGround
{
   unsigned char state; // 1 = already set (READY STATE), 2 = Terrain and Stair already create, otherwise = not ready
   unsigned char terrain[WORLDX][WORLDZ];
   unsigned char cloudSpace[WORLDX][DEFAULT_CLOUD_LV_NUM][WORLDZ];
   struct stair downStair;
   int highestLv;
   int lowestLv;

   int hightestLvOfCloud;
   int lowestLvOfCloud;

   int highestLvOfCubesNum;
   int lowestLvOfCubesNum;
};

#define NO_STAIR 0
#define ONLY_DOWN_STAIR 1
#define ONLY_UP_STAIR 2
#define BOTH_UP_DOWN_STAIR 3

#define NOT_VISITED 0
#define ALREADY_UP 1
#define ALREADY_DOWN 2

#define READY 1
#define NOT_READY 0
#define TERRAIN_AND_STAIR_IS_BUILT 2
#define GENERATED_UNDERGROUND_DONE 2
#define WEST 0
#define EAST 1
#define SOUTH 2
#define NORTH 3
#define NORTH_WEST 4
#define NORTH_EAST 5
#define SOUTH_EAST 6
#define SOUTH_WEST 7

#define X_SIDE_WALL 0
#define Z_SIDE_WALL 1
#define NOT_HAVE_ROOF 0
#define NOT_HAVE_GROUND 0
#define HAVE_ROOF 1
#define HAVE_GROUND 1
#define START_POINT 0
#define STOP_POINT 1

#define BUILT_NOW 0
#define BUILT_LATER 1

// convert 3D point to 2D point
struct Point2D convert3DPointTo2DPoint(const struct Point *obj);


void getAndConvertViewPos(struct Point *obj);
void getStairOfOutSideWorld(struct OnGround *obj,struct Point *startP,struct Point *stopP);
void getStairOfUndergroundWolrd(struct Underground *obj,int upOrDownStair,struct Point *startP,struct Point *stopP); // 0 = up, 1 = down
void SetMAXandMINPoint(const int **MaxPoint, const int **MinPoint, const int *P1, const int *P2);
void BuildABox(const struct Point *StartPoint, const struct Point *Endpoint, int color,int (*generateColorStyle)(int,int,int,int));
void BuildAWall(const struct Wall *AWall);
struct Room BuildEasyRoom(const struct Point *StartPoint, int xLenght, int zLenght, int height, int haveRoof, int haveGround, int color,int unitCubeColor,const unsigned char isbuiltNow);
void BuildARoom(const struct Room *ARoom);
void BuildDoorsWestVsEast(int roomID,struct Underground *obj);
void BuildDoorsSouthVsNorth(int roomID,struct Underground *obj);

int checkRoomAndOppositeRoomCanBuildCorridorAndHallway(int XorZSide,int roomID, struct Room *Rooms);

void setPrameterOfOnGround_defauleValue1(struct OnGround *obj);
void createOnGround(struct OnGround *obj);
int isOnDownStair(struct OnGround *obj);
// Underground functions
void setParameterOfUnderground_defaultValue1(struct Underground *obj,int hasDownStair);
void createUnderground(struct Underground *obj);
int findProperPositionToPlaceUpStairInRoomOfUnderground(struct Underground *obj,struct Point *stairPoint,int *stairDirection,const int stairWidth,const int stairNum, const int roomID);
int findProperPositionToPlaceDownStairInRoomOfUnderground(struct Underground *obj,struct Point *stairPoint,int *stairDirection,const int stairWidth,const int stairNum);
void listTheAreaProperforBuildDownStairInUnderground(struct Underground *obj,int listRoom[DEFAULT_NUM_ROOM],int *numEle,const int stairWidth,const int stairNum);
// Underground map setting functions
void updateUndergroundMap2D(struct Underground *obj,const int displayMode); // 1 = no map, 2 = fog, 0 = normal map
void setStairPositionInMap2D(struct Underground *obj);
void setRoomsPositionInMap2D(struct Underground *obj);
void setCubePositionInMap2D(struct Underground *obj);
int isOnUpStair(struct Underground *obj);

void generateTerrain(struct OnGround *obj,unsigned char terrain[WORLDX][WORLDZ], const struct Point *startBoundPoint,const struct Point *stopBoundPoint);
void updateBoundaryPointsForTerrainStyle2(struct Point *southWestP, struct Point *northWestP, struct Point *northEastP, struct Point *southEastP);

struct stair setStairAttribute(const struct Point StartPoint,const int frontOfStairDirection,const int stairWidth, const int numStair,const int type,const int color);
void locateAndBuildStairOnTerrain(struct OnGround *obj);
void BuildStair(const struct stair *obj);
struct Point getReferentStairPoint(const struct stair *obj,int startOrStop); // 0 = start, 1 = stop

int normalColorStyle(int originalColor,int x,int y,int z);
int pinkWhiteStyle(int originalColor,int x,int y,int z);
int darkAndLightBrownFloorStyle(int originalColor,int x,int y,int z);

int getMaxMinAtTerrainPoint(const unsigned char terrain[WORLDX][WORLDZ],const struct Point *refP,int direction,int *maxVal,int *minVal);
int readTerrain(const unsigned char terrain[WORLDX][WORLDZ],const int x,const int z);
int isIn3DBound(const struct Point *startP,const struct Point *stopP,const struct Point *ref); // yes = 1, otherwise = 0
int isIn2DBound(const struct Point *startP,const struct Point *stopP,const struct Point *ref); // yes = 1, otherwise = 0
int boundValue(int max,int min,int originValue);
int findMaxValue(int a,int b);
int findMinValue(int a,int b);
void plotWolrd(GLubyte world[WORLDX][WORLDY][WORLDZ],const struct Point aPoint,const unsigned char value);
void printPoint(const struct Point P1,const char *str);

int findDistanceBetweenPoint(const struct Point P1,const struct Point P2,const unsigned char type);

void createRandomShapeOfObjectInDefinedArea(unsigned char definedArea[WORLDX][WORLDZ],const struct Point *PStart,const struct Point *PStop);
void createCloudsInDefinedArea(struct OnGround *OutsideLand);
void moveCloudInOutsideLand(struct OnGround *obj);
// Map build
void initialMap2D(struct Map *obj);
void set3DLineOrBoxToMap2D(struct Map *obj,GLfloat color[4],int width,struct Point *startP,struct Point *stopP);
void addWallToMap2D(struct Map *obj,const struct Wall *aWall,GLfloat color[4],const int roomID);
void convertWallPositionToPoint2Ds(const struct Wall *obj,struct Point2D *P1,struct Point2D *P2);
void findStartAndStopPointOfARoom(struct Room *obj,struct Point *maxPoint,struct Point *minPoint);
void findStartAndStopPointOfARoom2D(struct Room *obj,struct Point2D *maxPoint,struct Point2D *minPoint);
int findViewPointIsWhichRoom2D(struct Map *obj);
void drawARoomInMap2D(struct Map *obj,const int roomID);

// Fog map functions
//jjjjj1
// fog map initial function
void setFogAreaInFogMap(struct FogMap *obj,GLfloat fogColor[4],struct LineOrBox2D (*transformationFn)(struct LineOrBox2D*));

// map opertion functions
void clearFogPosition(struct FogMap *obj,const int xView,const int zView);
void clearFogInArea(struct FogMap *obj,struct LineOrBox2D *area);
void clearFogPositionByIndices(struct FogMap *obj,const int i,const int j);
void drawFogInMap(struct FogMap *obj);
void getViewPointInMap(struct Point2D *obj);
void drawViewPoint(GLfloat color[4],struct LineOrBox2D (*transformationFn)(struct LineOrBox2D*));


// Line or Box functions


// map resolution 500x500 and scale: 5.0 , Invert Z-axis
struct LineOrBox2D mapTransformFuntion(struct LineOrBox2D *obj);
struct LineOrBox2D mapRetransformFuntion(struct LineOrBox2D *obj);
struct Point2D point2DTransformFuntionForMap(struct Point2D *obj);
struct Point2D point2DRetransformFuntionForMap(struct Point2D *obj);

void printLineOrBoxObj(struct LineOrBox2D *obj);
void setPointsAndColorOfLineOrBox(struct LineOrBox2D *obj,struct Point2D startAndStopPoint[2],const int width,const GLfloat colorAttrib[4]);
void drawLineMap2DWithTransFn(struct LineOrBox2D *obj,struct LineOrBox2D (*transformationFn)(struct LineOrBox2D*));
void drawBoxMap2DWithTransFn(struct LineOrBox2D *obj,struct LineOrBox2D (*transformationFn)(struct LineOrBox2D*));
void drawAStairInMap(struct Point *startP,struct Point *stopt,GLfloat color[4],struct LineOrBox2D (*transformationFn)(struct LineOrBox2D*));
void drawBoxMap2D(struct LineOrBox2D *obj);
void drawLineMap2D(struct LineOrBox2D *obj);
int visibilityOfLineOrBox(struct LineOrBox2D *obj);

// Fog map functions
void initialFogMap(struct FogMap *obj);

// Display map mode
#define NO_MAP 0
#define FOG_MAP 1
#define FULL_MAP 2


#define NOT_INVERT 0
#define X_INVERT 1
#define Z_INVERT 2
#define BOTH_INVERT 3

#define AREA_XP 0
#define AREA_ZP 1
#define AREA_X_LENGHT 2
#define AREA_Z_LENGHT 3

#define X_DISTANCE 0
#define Y_DISTANCE 1
#define Z_DISTANCE 2


float alphaVal = 0.9;
int stage_Lv = 0;
struct OnGround OutsideLand;
struct Underground AnUnderground;
// define the dimension of Grid 3X3
int dimensionOfGrid3x3[9][4] = {{0, 0, 33, 33},
                                {33, 0, 34, 33},
                                {67, 0, 33, 33},
                                {0, 33, 33, 34},
                                {33, 33, 34, 34},
                                {67, 33, 33, 34},
                                {0, 67, 33, 33},
                                {33, 67, 34, 33},
                                {67, 67, 33, 33}};

int DoorDirections[9][4] = {{-1, 1, -1, 3},
                            {0, 2, -1, 4},
                            {1, -1, -1, 5},
                            {-1, 4, 0, 6},
                            {3, 5, 1, 7},
                            {4, -1, 2, 8},
                            {-1, 7, 3, -1},
                            {6, 8, 4, -1},
                            {7, -1, 5, -1}};
/*
#define WEST 0
#define EAST 1
#define SOUTH 2
#define NORTH 3
#define NORTH_WEST 4
#define NORTH_EAST 5
#define SOUTH_EAST 6
#define SOUTH_WEST 7
*/
int directionOffset[8][2] = {{-1,0},
                              {1,0},
                              {0,-1},
                              {0,1},
                              {-1,1},
                              {1,1},
                              {1,-1},
                              {-1,-1}};


   float g_floorLv = (float)DEFAULT_UNDERGROUND_LV;

	/*** collisionResponse() ***/
	/* -performs collision detection and response */
	/*  sets new xyz  to position of the viewpoint after collision */
	/* -can also be used to implement gravity by updating y position of vp*/
	/* note that the world coordinates returned from getViewPosition()
	   will be the negative value of the array indices */
void collisionResponse()
{

   /* your code for collisions goes here */
   float tempX=0.0;
   float tempY=0.0;
   float tempZ=0.0;
   float margin = 0.2;
   float vxp = 0;
   float vyp = 0;
   float vzp = 0;
   float Xdirection = 0; // - , + or 0
   float Zdirection = 0; // - , + or 0
   int isHit = 0;
   int isAble2Jump = 0;
   int outOfSpace = 0;
   int SpaceOverHeadPos = 0;
   getViewPosition(&vxp, &vyp, &vzp);
   getOldViewPosition(&tempX, &tempY, &tempZ);
   Xdirection = -1 * (vxp > tempX) + (vxp < tempX);
   Zdirection = -1 * (vzp > tempZ) + (vzp < tempZ);
   int ValueAtCubePointDetect     = 0;
   int ValueAtMarginXOfCubeDetect = 0;
   int ValueAtMarginZOfCubeDetect = 0;
   int ValueAtMarginXZOfCubeDetect= 0;
   SpaceOverHeadPos = 1 + (-1) * (int)vyp;

   ValueAtCubePointDetect     = world[-1 * (int)vxp][-1 * (int)vyp][-1 * (int)vzp];
   ValueAtMarginXOfCubeDetect = world[-1 * (int)(vxp - margin * Xdirection)][-1 * (int)vyp][-1 * (int)vzp];
   ValueAtMarginZOfCubeDetect = world[-1 * (int)vxp][-1 * (int)vyp][-1 * (int)(vzp - margin * Zdirection)];
   ValueAtMarginXZOfCubeDetect= world[-1 * (int)(vxp - margin * Xdirection)][-1 * (int)vyp][-1 * (int)(vzp - margin * Zdirection)];
   isHit = ((ValueAtCubePointDetect + ValueAtMarginXOfCubeDetect + ValueAtMarginZOfCubeDetect + ValueAtMarginXZOfCubeDetect) > 0);
   isAble2Jump = world[-1 * (int)vxp][SpaceOverHeadPos][-1 * (int)vzp] +
                 world[-1 * (int)(vxp - margin * Xdirection)][SpaceOverHeadPos][-1 * (int)vzp] +
                 world[-1 * (int)vxp][SpaceOverHeadPos][-1 * (int)(vzp - margin * Zdirection)] +
                 world[-1 * (int)(vxp - margin * Xdirection)][SpaceOverHeadPos][-1 * (int)(vzp - margin * Zdirection)];
   outOfSpace = (((-1 * (int)vxp) > 99) || ((-1 * (int)vyp) > 49) || ((-1 * (int)vzp) > 99));
   outOfSpace = outOfSpace || (((-1 * (int)vxp) < 0) || ((-1 * (int)vyp) < (g_floorLv + 1)) || ((-1 * (int)vzp) < 0));
   // protect to pass through
   if ((outOfSpace == 1) || ((isHit != 0) && (isAble2Jump != 0)))
   {
      setViewPosition(tempX, tempY, tempZ);
   }
   // jump on the box
   else if ((isHit != 0) && (isAble2Jump == 0))
   {
      if (ValueAtCubePointDetect != 0)
      {
         tempX = (float)(int)vxp;
         tempZ = (float)(int)vzp;
      }
      else if (ValueAtMarginXOfCubeDetect != 0)
      {
         tempX = (float)(int)(vxp - margin * Xdirection);
         tempZ = (float)(int)vzp;
      }
      else if (ValueAtMarginZOfCubeDetect != 0)
      {
         tempX = (float)(int)vxp;
         tempZ = (float)(int)(vzp - margin * Zdirection);
      }
      else if (ValueAtMarginXZOfCubeDetect != 0)
      {
         tempX = (float)(int)(vxp - margin * Xdirection);
         tempZ = (float)(int)(vzp - margin * Zdirection);
      }
      setViewPosition(tempX, tempY - 1, tempZ);
   }
}


	/******* draw2D() *******/
	/* draws 2D shapes on screen */
	/* use the following functions: 			*/
	/*	draw2Dline(int, int, int, int, int);		*/
	/*	draw2Dbox(int, int, int, int);			*/
	/*	draw2Dtriangle(int, int, int, int, int, int);	*/
	/*	set2Dcolour(float []); 				*/
	/* colour must be set before other functions are called	*/
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
      struct Point stairP1;
      struct Point stairP2;          
      GLfloat black[] = {0.0, 0.0, 0.0, alphaVal};
      GLfloat green[] = {0.0, 0.5, 0.0, alphaVal};
      GLfloat grey[] = {0.6, 0.6, 0.6, alphaVal};

      if (displayMap != NO_MAP )
      {
         // Draw doors on outside world
           // Draw viewer position
           drawViewPoint(green,&mapTransformFuntion);
           if(stage_Lv == 0)
           {
              getStairOfOutSideWorld(&OutsideLand,&stairP1,&stairP2);
              drawAStairInMap(&stairP1,&stairP2,grey,&mapTransformFuntion);
           }
           else if (stage_Lv == -1)
           {
              updateUndergroundMap2D(&AnUnderground,displayMap);
           }
           // draw background
           set2Dcolour(black);
           draw2Dbox(0,0, 100*DEFAULT_MAP_2D_SCALE_VAL-1, 100*DEFAULT_MAP_2D_SCALE_VAL-1); 
      }

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

	/* offset counter for animated texture */
      static float textureOffset = 0.0;

	/* scaling values for fish mesh */
      static float fishScale = 1.0;
      static int scaleCount = 0;
      static GLfloat scaleOffset = 0.0;

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

	/* move texture for lava effect */
      textureOffset -= 0.01;
      setTextureOffset(18, 0.0, textureOffset);

	/* make fish grow and shrink (scaling) */
      if (scaleCount == 1) scaleOffset += 0.01;
      else scaleOffset -= 0.01;
      if (scaleOffset >= 0.5) scaleCount = 0;
      if (scaleOffset <= 0.0) scaleCount = 1;
      setScaleMesh(1, 0.5 + scaleOffset);

	/* make cow with id == 2 appear and disappear */
	/* use scaleCount as switch to flip draw/hide */
	/* rotate cow while it is visible */
      if (scaleCount == 0) {
         drawMesh(2);
         setRotateMesh(2, 0.0, 180.0 + scaleOffset * 100.0, 0.0);
      } else {
         hideMesh(2);
      }

    /* end testworld animation */


   } else {

	/* your code goes here */

      int valOfWorldAtBelowVP = 0;
      float vpx = 0;
      float vpy = 0;
      float vpz = 0;
      float gravityVal =DEFAULT_GRAVITY;
      float newY = 0;
      float floorLv = g_floorLv -1;
      static int fallState = 0;
      // yn = y(n-1) - 0.5*g*(tn^2-t(n-1)^2)
      static clock_t timeCount, clkRef;
      static float timePast, timeCurrent;
      getViewPosition(&vpx, &vpy, &vpz);
      floorLv = (floorLv>= 0.0)*floorLv;
      valOfWorldAtBelowVP = world[-1 * (int)vpx][(-1 * (int)vpy) - 1][-1 * (int)vpz];
      timeCount = clock();

      if (fallState == 0)
      {
         fallState = (valOfWorldAtBelowVP == 0);
      }
      else if (fallState == 1)
      {
         clkRef = clock();
         timePast = 0;
         timeCurrent = 0;
         fallState =2 ;
      }
      else if (fallState == 2)
      {

         timePast = timeCurrent;
         timeCurrent = (float)(clock() - clkRef)/CLOCKS_PER_SEC;
         newY = (-1.0)*vpy - gravityVal*(timeCurrent*timeCurrent-timePast*timePast);
         
         if (valOfWorldAtBelowVP != 0) 
         {
            floorLv = (-1*(int)vpy)-1;
         }

         if ((valOfWorldAtBelowVP != 0) && ((((float)((int)vpy)) - vpy) <= 0.0))
         {
            fallState = 0;
            newY = floorLv + 1.0;
         }
         if ((newY < (floorLv + 1)) || (newY > 49.0))
            newY = floorLv + 1.0;
         setViewPosition(vpx, newY * (-1.0), vpz);
      }
         if((isOnDownStair(&OutsideLand) == 1) && (stage_Lv == 0))
         {
            createUnderground(&AnUnderground);
            stage_Lv = -1;
         }
         if((isOnUpStair(&AnUnderground) == 1) &&(stage_Lv == -1))
         {
            createOnGround(&OutsideLand);
            stage_Lv = 0;
         }
         if(stage_Lv == 0)
            moveCloudInOutsideLand(&OutsideLand);

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
void makeWorld()
{
   int i, j, k;
   for (i = 0; i < WORLDX; i++)
      for (j = 0; j < WORLDY; j++)
         for (k = 0; k < WORLDZ; k++)
            world[i][j][k] = 0;
}

int getRandomNumber(int minimum, int maximum)
{
  int ret = minimum;
  if ((maximum + 1 - minimum) != 0)
  {
    ret = minimum + rand() % (maximum + 1 - minimum);
  }
  else
  {
    printf("cannot random - min:%d, max:%d",minimum,maximum);
  }
  return ret;
}
int getRandomNumberExceptValue(int minimum,int maximum,int exceptVal)
{
  int ret = minimum;
  if (minimum == exceptVal)
  {
    ret = getRandomNumber(1+minimum,maximum);
  }
  else if (maximum == exceptVal)
  {
    ret = getRandomNumber(minimum,maximum-1);
  }
  else if ((exceptVal > minimum)&&(maximum >exceptVal))
  {
    if (getRandomNumber(0,1) == 1)
    {
      ret = getRandomNumber(minimum,exceptVal-1);
    }
    else
    {
      ret = getRandomNumber(exceptVal+1,maximum);
    }
  }
  else
  {
    ret = getRandomNumber(minimum,maximum);
  }
  return ret;
}

int main(int argc, char** argv)
{
int i, j, k;
	/* initialize the graphics system */
   graphicsInit(&argc, argv);


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

	/* texture examples */

	/* create textured cube */
	/* create user defined colour with an id number of 11 */
      setUserColour(11, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0);
	/* attach texture 22 to colour id 11 */
      setAssignedTexture(11, 22);
	/* place a cube in the world using colour id 11 which is texture 22 */
      world[59][25][50] = 11;

	/* create textured cube */
      setUserColour(12, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0);
      setAssignedTexture(12, 27);
      world[61][25][50] = 12;

	/* create textured cube */
      setUserColour(10, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0);
      setAssignedTexture(10, 26);
      world[63][25][50] = 10;

	/* create textured floor */
      setUserColour(13, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0);
      setAssignedTexture(13, 8);
      for (i=57; i<67; i++)
         for (j=45; j<55; j++)
            world[i][24][j] = 13;

	/* create textured wall */
      setUserColour(14, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0);
      setAssignedTexture(14, 18);
      for (i=57; i<67; i++)
         for (j=0; j<4; j++)
            world[i][24+j][45] = 14;

	/* create textured wall */
      setUserColour(15, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0);
      setAssignedTexture(15, 42);
      for (i=45; i<55; i++)
         for (j=0; j<4; j++)
            world[57][24+j][i] = 15;

		// two cubes using the same texture but one is offset
		// cube with offset texture 33
      setUserColour(16, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0);
      setAssignedTexture(16, 33);
      world[65][25][50] = 16;
      setTextureOffset(16, 0.5, 0.5);
		// cube with non-offset texture 33
      setUserColour(17, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0);
      setAssignedTexture(17, 33);
      world[66][25][50] = 17;

		// create some lava textures that will be animated
      setUserColour(18, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0);
      setAssignedTexture(18, 24);
      world[62][24][55] = 18;
      world[63][24][55] = 18;
      world[64][24][55] = 18;
      world[62][24][56] = 18;
      world[63][24][56] = 18;
      world[64][24][56] = 18;

		// draw cow mesh and rotate 45 degrees around the y axis
		// game id = 0, cow mesh id == 0
      setMeshID(0, 0, 48.0, 26.0, 50.0);
      setRotateMesh(0, 0.0, 45.0, 0.0);

		// draw fish mesh and scale to half size (0.5)
		// game id = 1, fish mesh id == 1
      setMeshID(1, 1, 51.0, 28.0, 50.0);
      setScaleMesh(1, 0.5);

		// draw cow mesh and rotate 45 degrees around the y axis
		// game id = 2, cow mesh id == 0
      setMeshID(2, 0, 59.0, 26.0, 47.0);

		// draw bat
		// game id = 3, bat mesh id == 2
      setMeshID(3, 2, 61.0, 26.0, 47.0);
      setScaleMesh(3, 0.5);
		// draw cactus
		// game id = 4, cactus mesh id == 3
      setMeshID(4, 3, 63.0, 26.0, 47.0);
      setScaleMesh(4, 0.5);


   } else {

      srand(time(NULL));
	/* your code to build the world goes here */
      flycontrol = 0;
      makeWorld();

      setParameterOfUnderground_defaultValue1(&AnUnderground,BOTH_UP_DOWN_STAIR);
      setPrameterOfOnGround_defauleValue1(&OutsideLand);
      createOnGround(&OutsideLand);
      //　setViewPosition(-1 * xViewP, -1 * yStartP, -1 * zViewP);
     // setViewPosition(-1*10, -1*48, -1*10);

   }


	/* starts the graphics processing loop */
	/* code after this will not run until the program exits */
   glutMainLoop();
   return 0; 
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


void BuildABox(const struct Point *StartPoint,const struct Point *Endpoint,int color,int (*generateColorStyle)(int,int,int,int))
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
          if ((i <WORLDX) && (j <WORLDY) && (k <WORLDZ) && (i > -1) && (j > -1) && (k > -1))
          {
            world[i][j][k] = generateColorStyle(color,i,j,k);
          }
}

void BuildAWall(const struct Wall *AWall)
{

   struct Point EndPoint = {0,0,0};
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
   BuildABox(&(AWall->StartPoint),&EndPoint,AWall->color,AWall->generateColorStyle);
   
}

void BuildARoom(const struct Room *ARoom)
{
   int i = WEST;
   struct Point StartPoint = {0,0,0};
   struct Point EndPoint= {0,0,0};
   for(i=WEST;i<=NORTH;i++)
   BuildAWall(&(ARoom->Walls[i]));

   // Build Roof
   if (ARoom->haveRoof == HAVE_ROOF)
   {
      StartPoint.x = ARoom->StartPoint.x;
      StartPoint.y = ARoom->StartPoint.y + ARoom->Walls[WEST].height+1; // Assume that all rooms have the same height, not count ground LV.
      StartPoint.z = ARoom->StartPoint.z;

      EndPoint.x = ARoom->StartPoint.x + ARoom->Walls[SOUTH].width+1;
      EndPoint.y = ARoom->StartPoint.y + ARoom->Walls[WEST].height+1;// Assume that all rooms have the same height, not count ground LV.
      EndPoint.z = ARoom->StartPoint.z + ARoom->Walls[WEST].width-1;
      BuildABox(&StartPoint,&EndPoint,ARoom->Roofcolor,&normalColorStyle);
   }
   // Build Ground
   if (ARoom->haveGround == HAVE_GROUND)
   {
      EndPoint.x = ARoom->StartPoint.x + ARoom->Walls[SOUTH].width +1;
      EndPoint.y = ARoom->StartPoint.y;
      EndPoint.z = ARoom->StartPoint.z + ARoom->Walls[WEST].width -1;
      BuildABox(&(ARoom->StartPoint),&EndPoint,ARoom->Groundcolor,&normalColorStyle);
   }
}

struct Room BuildEasyRoom(const struct Point *StartPoint,int xLenght,int zLenght,int height,int haveRoof,int haveGround,int color,int unitCubeColor,const unsigned char isbuiltNow)
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
   int i = 0;
   int x = StartPoint->x;
   int y = StartPoint->y;
   int z = StartPoint->z;
   int onFloorLv = y + 1;
   // create a few Cubes 1 high
   ARoom.unitCubeColor = 0;
   ARoom.numUnitCubes = 0;
   memset(ARoom.unitCubePoint,0,sizeof(ARoom.unitCubePoint));
   if((zLenght > 5)&&(xLenght > 5))
   {
      ARoom.unitCubeColor = unitCubeColor;
      ARoom.numUnitCubes = getRandomNumber(1,2);
      for(i = 0; i < ARoom.numUnitCubes;i++)
      {
         ARoom.unitCubePoint[i] = (struct Point){
                                   x + 2 + getRandomNumber(0,xLenght-4),
                                   onFloorLv,
                                   z + 2 + getRandomNumber(0,zLenght -4)};
         BuildABox(&( ARoom.unitCubePoint[i]),&(ARoom.unitCubePoint[i]),unitCubeColor,&normalColorStyle);
      }
   }

      
   // Set Attribute to Wall 
   ARoom.StartPoint            = *StartPoint;
   ARoom.Groundcolor           = color;
   ARoom.Roofcolor             = color;
   ARoom.haveRoof              = haveRoof;
   ARoom.haveGround            = haveGround;
   // the wall at West side
   ARoom.Walls[WEST].StartPoint   = (struct Point){
                                     x,
                                     onFloorLv,
                                     z};
   ARoom.Walls[WEST].height       = height;
   ARoom.Walls[WEST].width        = zLenght;
   ARoom.Walls[WEST].XorZSide     = Z_SIDE_WALL;
   ARoom.Walls[WEST].color        = color;
   ARoom.Walls[WEST].generateColorStyle = &normalColorStyle;

   // the wall at East side
   ARoom.Walls[EAST].StartPoint   = (struct Point){
                                     x + xLenght - 1,
                                    onFloorLv,
                                     z};
   ARoom.Walls[EAST].height       = height;
   ARoom.Walls[EAST].width        = zLenght;
   ARoom.Walls[EAST].XorZSide     = Z_SIDE_WALL;
   ARoom.Walls[EAST].color        = color;
   ARoom.Walls[EAST].generateColorStyle = &normalColorStyle;

   // the wall at South side
   ARoom.Walls[SOUTH].StartPoint.x = x + 1;
   ARoom.Walls[SOUTH].StartPoint.y = onFloorLv;
   ARoom.Walls[SOUTH].StartPoint.z = z;
   ARoom.Walls[SOUTH].height       = height;
   ARoom.Walls[SOUTH].width        = xLenght - 2;
   ARoom.Walls[SOUTH].XorZSide     = X_SIDE_WALL;
   ARoom.Walls[SOUTH].color        = color;
   ARoom.Walls[SOUTH].generateColorStyle = &normalColorStyle;


   // the wall at North side
   ARoom.Walls[NORTH].StartPoint.x = x + 1;
   ARoom.Walls[NORTH].StartPoint.y = onFloorLv;
   ARoom.Walls[NORTH].StartPoint.z = z + zLenght -1;
   ARoom.Walls[NORTH].height       = height;
   ARoom.Walls[NORTH].width        = xLenght - 2;
   ARoom.Walls[NORTH].XorZSide     = X_SIDE_WALL;
   ARoom.Walls[NORTH].color        = color;
   ARoom.Walls[NORTH].generateColorStyle = &normalColorStyle;
   if(isbuiltNow == BUILT_NOW)
   {
      BuildARoom(&ARoom);
   }
   return ARoom;
}

void BuildDoorsWestVsEast(int roomID,struct Underground *obj)
{
      int i=0,j=0;
      struct Room *Rooms = obj->m_rooms;
      struct Point DoorOfAroomPoint = {-1,-1,-1};
      struct Point DoorOfOppositeRoomPoint = {-1,-1,-1};
      int doorWidth = 0;
      int doorHeight = 0;
      int color = obj->m_defaultRoomColor;
      struct Room *Aroom = NULL;
      struct Room *OpposisteRoom = NULL;
      int oppositeRoomID = -1;
      int AroomDirection = WEST;
      int OppositeRoomDirection = EAST;
      GLfloat pink[] = {1.0,0.0,1.0,alphaVal};
      GLfloat brown[] = {0.8, 0.45, 0.1, alphaVal};
      for(AroomDirection = WEST ; AroomDirection <= EAST;AroomDirection++)
      {
         if(DoorDirections[roomID][AroomDirection] >= 0)
         { 
            oppositeRoomID = DoorDirections[roomID][AroomDirection];
            OppositeRoomDirection = (AroomDirection+1)%2; // Becasue there are only 2 direnctions: WEST(0) and EAST(1)
            Aroom = &Rooms[roomID];
            OpposisteRoom = &Rooms[oppositeRoomID];
            doorWidth = Aroom->doorWidth;
            doorHeight = Aroom->doorHeight;
            oppositeRoomID = DoorDirections[roomID][AroomDirection];
            DoorOfOppositeRoomPoint = OpposisteRoom->DoorPosition[OppositeRoomDirection];

            if(obj->m_state == READY)
            {
                        Aroom->DoorPosition[AroomDirection]= Aroom->Walls[AroomDirection].StartPoint;
              i = 30; // Protect infinity loop
              do
              {
                 Aroom->DoorPosition[AroomDirection].z = Aroom->Walls[AroomDirection].StartPoint.z + getRandomNumber(1, Aroom->Walls[AroomDirection].width - Aroom->doorWidth - 1);
              } while ((Aroom->DoorPosition[AroomDirection].z == DoorOfOppositeRoomPoint.z) && (i-- >= 0));
    
            }

            DoorOfAroomPoint = Aroom->DoorPosition[AroomDirection];
            const struct Wall ADoor = {DoorOfAroomPoint,Aroom->doorHeight,Aroom->doorWidth,Z_SIDE_WALL,0,&normalColorStyle};
            BuildAWall(&ADoor);
            if(obj->m_state == READY)
            {
              addWallToMap2D(&(obj->m_a2DMap),&ADoor,pink,roomID);
            }
         }
         if((DoorOfAroomPoint.x > -1)&&(DoorOfOppositeRoomPoint.x> -1) &&(AroomDirection < OppositeRoomDirection))
         {
            int turningPointX = (dimensionOfGrid3x3[oppositeRoomID][AREA_XP] + dimensionOfGrid3x3[oppositeRoomID][AREA_X_LENGHT]) - (doorWidth / 2);

            struct Point AnOppositeCorridorRoofStartPoint = {turningPointX + doorWidth,DoorOfAroomPoint.y + doorHeight,DoorOfAroomPoint.z - 1};
            struct Point AnOppositeCorridorRoofEndPoint =   {DoorOfAroomPoint.x-1 ,DoorOfAroomPoint.y + doorHeight,DoorOfAroomPoint.z +doorWidth};
            struct Point ACorridorRoofStartPoint= {1+DoorOfOppositeRoomPoint.x,DoorOfAroomPoint.y + doorHeight,DoorOfOppositeRoomPoint.z-1};
            struct Point ACorridorRoofEndPoint  = {turningPointX-2,DoorOfAroomPoint.y + doorHeight,DoorOfOppositeRoomPoint.z+doorWidth};
            struct Point AMiddleCorridorRoofStartPoint = {turningPointX-1,DoorOfAroomPoint.y + doorHeight,DoorOfOppositeRoomPoint.z - 1};
            struct Point AMiddleCorridorRoofEndPoint = {turningPointX+doorWidth,DoorOfAroomPoint.y + doorHeight,DoorOfAroomPoint.z+doorWidth};



            struct Wall ALongCorridorWall ={{turningPointX,
                                             DoorOfAroomPoint.y,
                                             DoorOfAroomPoint.z+ doorWidth},
                                             doorHeight,
                                             DoorOfAroomPoint.x - turningPointX,
                                             X_SIDE_WALL,
                                             color,
                                             &pinkWhiteStyle};
            struct Wall AShortOppositeCorridorWall ={{DoorOfOppositeRoomPoint.x+ 1,
                                                      DoorOfOppositeRoomPoint.y,
                                                      DoorOfOppositeRoomPoint.z + doorWidth},
                                                      doorHeight,
                                                      turningPointX - 1 - DoorOfOppositeRoomPoint.x,
                                                      X_SIDE_WALL,
                                                      color,
                                                      &pinkWhiteStyle};
            struct Wall ALongOppositeCorridorWall = {{DoorOfOppositeRoomPoint.x+ 1,
                                                      DoorOfOppositeRoomPoint.y,
                                                      DoorOfOppositeRoomPoint.z-1},
                                                      doorHeight,
                                                      doorWidth + AShortOppositeCorridorWall.width,
                                                      X_SIDE_WALL,
                                                      color,
                                                      &pinkWhiteStyle};

            struct Wall AShortCorridorWall = {{turningPointX + doorWidth,
                                               DoorOfAroomPoint.y,
                                               DoorOfAroomPoint.z-1},
                                               doorHeight,
                                               DoorOfAroomPoint.x - turningPointX - doorWidth,
                                               X_SIDE_WALL,
                                               color,
                                               &pinkWhiteStyle};


            struct Wall AnOppositeZSIDECorridorWall= {{turningPointX - 1,
                                                       DoorOfOppositeRoomPoint.y,
                                                       DoorOfOppositeRoomPoint.z + doorWidth+1},
                                                       doorHeight,
                                                       DoorOfAroomPoint.z - DoorOfOppositeRoomPoint.z,
                                                       Z_SIDE_WALL,
                                                       color,
                                                       &pinkWhiteStyle};


            struct Wall AZSIDECorridorWall = {{turningPointX + doorWidth,
                                               DoorOfAroomPoint.y,
                                               DoorOfOppositeRoomPoint.z-1},
                                               doorHeight,
                                               DoorOfAroomPoint.z - DoorOfOppositeRoomPoint.z,
                                               Z_SIDE_WALL,
                                               color,
                                               &pinkWhiteStyle};

            if (DoorOfAroomPoint.z < DoorOfOppositeRoomPoint.z)
            {
               ALongCorridorWall.StartPoint.z = DoorOfAroomPoint.z - 1;
               AShortOppositeCorridorWall.StartPoint.z = DoorOfOppositeRoomPoint.z- 1;
               ALongOppositeCorridorWall.StartPoint.z = DoorOfOppositeRoomPoint.z + doorWidth; 
               AShortCorridorWall.StartPoint.z = DoorOfAroomPoint.z + doorWidth;
               AnOppositeZSIDECorridorWall.StartPoint.x = turningPointX  + doorWidth; 
               AnOppositeZSIDECorridorWall.StartPoint.z = DoorOfAroomPoint.z + doorWidth;
               AnOppositeZSIDECorridorWall.width =  1 + DoorOfOppositeRoomPoint.z - DoorOfAroomPoint.z;
               AZSIDECorridorWall.StartPoint.x = turningPointX -1; 
               AZSIDECorridorWall.StartPoint.z = DoorOfAroomPoint.z - 1;
               AZSIDECorridorWall.width = AnOppositeZSIDECorridorWall.width;
            }

            // Build Roof for Hallway
            if (DoorOfOppositeRoomPoint.z > DoorOfAroomPoint.z)
            {
               AMiddleCorridorRoofEndPoint.z  =  DoorOfOppositeRoomPoint.z +doorWidth;
               AMiddleCorridorRoofStartPoint.z = DoorOfAroomPoint.z -1;
            }

            BuildAWall(&AnOppositeZSIDECorridorWall);
            BuildAWall(&AZSIDECorridorWall);
            BuildAWall(&AShortCorridorWall);
            BuildAWall(&ALongCorridorWall);
            BuildAWall(&AShortOppositeCorridorWall);
            BuildAWall(&ALongOppositeCorridorWall);
            
            // Build Roofs of Corridors
            BuildABox(&AMiddleCorridorRoofStartPoint,&AMiddleCorridorRoofEndPoint,color,&pinkWhiteStyle);
            BuildABox(&ACorridorRoofStartPoint,&ACorridorRoofEndPoint,color,&pinkWhiteStyle);
            BuildABox(&AnOppositeCorridorRoofStartPoint,&AnOppositeCorridorRoofEndPoint,color,&pinkWhiteStyle);

            // Create map 
            if(obj->m_state == READY)
            {
              addWallToMap2D(&(obj->m_a2DMap),&AnOppositeZSIDECorridorWall,brown,-1);
              addWallToMap2D(&(obj->m_a2DMap),&AZSIDECorridorWall,brown,-1);
              addWallToMap2D(&(obj->m_a2DMap),&AShortCorridorWall,brown,-1);
              addWallToMap2D(&(obj->m_a2DMap),&ALongCorridorWall,brown,-1);
              addWallToMap2D(&(obj->m_a2DMap),&AShortOppositeCorridorWall,brown,-1);
              addWallToMap2D(&(obj->m_a2DMap),&ALongOppositeCorridorWall,brown,-1);
            }
      } 
   }
}
void BuildDoorsSouthVsNorth(int roomID,struct Underground *obj)
{

      int i=0,j=0;
      struct Room *Rooms = obj->m_rooms;
      struct Point DoorOfAroomPoint = {-1,-1,-1};
      struct Point DoorOfOppositeRoomPoint = {-1,-1,-1};
      int doorWidth = 0;
      int doorHeight = 0;
      int color = obj->m_defaultRoomColor;
      struct Room *Aroom = NULL;
      struct Room *OpposisteRoom = NULL;
      int oppositeRoomID = -1;
      int AroomDirection = SOUTH;
      int OppositeRoomDirection = NORTH;
      GLfloat pink[] = {1.0,0.0,1.0,alphaVal};
      GLfloat brown[] = {0.8, 0.45, 0.1, alphaVal};
      for(AroomDirection = SOUTH ; AroomDirection <= NORTH;AroomDirection++)
   {
      if(DoorDirections[roomID][AroomDirection] >= 0)
      { 
         oppositeRoomID = DoorDirections[roomID][AroomDirection];
         OppositeRoomDirection = 2+(AroomDirection-1)%2; // Becasue there are only 2 direnctions: SOUTH(2) and NORTH(3)
         Aroom = &Rooms[roomID];
         OpposisteRoom = &Rooms[oppositeRoomID];
         doorWidth = Aroom->doorWidth;
         doorHeight = Aroom->doorHeight;
         oppositeRoomID = DoorDirections[roomID][AroomDirection];
         DoorOfOppositeRoomPoint = OpposisteRoom->DoorPosition[OppositeRoomDirection];
         if(obj->m_state == READY)
         {
           Aroom->DoorPosition[AroomDirection] = Aroom->Walls[AroomDirection].StartPoint;
           i = 30; // Protect infinity loop
           do
           {
              Aroom->DoorPosition[AroomDirection].x = Aroom->Walls[AroomDirection].StartPoint.x + getRandomNumber(0, Aroom->Walls[AroomDirection].width - Aroom->doorWidth - 1);
           } while ((Aroom->DoorPosition[AroomDirection].x == DoorOfOppositeRoomPoint.x) && (i-- >= 0));
         }

         DoorOfAroomPoint = Aroom->DoorPosition[AroomDirection];
         const struct Wall ADoor = {DoorOfAroomPoint,Aroom->doorHeight,Aroom->doorWidth,X_SIDE_WALL,0,&normalColorStyle};
         BuildAWall(&ADoor);
        if(obj->m_state == READY)
        {
          addWallToMap2D(&(obj->m_a2DMap),&ADoor,pink,roomID);
        }
      }
      // south and north side

      if((DoorOfAroomPoint.z > -1)&&(DoorOfOppositeRoomPoint.z > -1) &&(AroomDirection < OppositeRoomDirection))
      {

            int turningPointZ = (dimensionOfGrid3x3[oppositeRoomID][AREA_ZP] + dimensionOfGrid3x3[oppositeRoomID][AREA_Z_LENGHT]) - (doorWidth / 2);

            struct Point AnOppositeCorridorRoofStartPoint = {DoorOfAroomPoint.x - 1,DoorOfAroomPoint.y + doorHeight,turningPointZ + doorWidth};
            struct Point AnOppositeCorridorRoofEndPoint =   {DoorOfAroomPoint.x +doorWidth ,DoorOfAroomPoint.y + doorHeight,DoorOfAroomPoint.z-1};
            struct Point ACorridorRoofStartPoint= {DoorOfOppositeRoomPoint.x-1,DoorOfAroomPoint.y + doorHeight,1+DoorOfOppositeRoomPoint.z};
            struct Point ACorridorRoofEndPoint  = {DoorOfOppositeRoomPoint.x+doorWidth,DoorOfAroomPoint.y + doorHeight,turningPointZ-2};
            struct Point AMiddleCorridorRoofStartPoint = {DoorOfOppositeRoomPoint.x - 1,DoorOfAroomPoint.y + doorHeight,turningPointZ-1};
            struct Point AMiddleCorridorRoofEndPoint = {DoorOfAroomPoint.x+doorWidth,DoorOfAroomPoint.y + doorHeight,turningPointZ+doorWidth};



            struct Wall ALongCorridorWall ={{DoorOfAroomPoint.x+ doorWidth,
                                             DoorOfAroomPoint.y,
                                             turningPointZ},
                                             doorHeight,
                                             DoorOfAroomPoint.z - turningPointZ,
                                             Z_SIDE_WALL,
                                             color,
                                             &pinkWhiteStyle};
            struct Wall AShortOppositeCorridorWall ={{DoorOfOppositeRoomPoint.x + doorWidth,
                                                      DoorOfOppositeRoomPoint.y,
                                                      DoorOfOppositeRoomPoint.z+ 1},
                                                      doorHeight,
                                                      turningPointZ - 1 - DoorOfOppositeRoomPoint.z,
                                                      Z_SIDE_WALL,
                                                      color,
                                                      &pinkWhiteStyle};
            struct Wall ALongOppositeCorridorWall = {{DoorOfOppositeRoomPoint.x-1,
                                                      DoorOfOppositeRoomPoint.y,
                                                      DoorOfOppositeRoomPoint.z+ 1},
                                                      doorHeight,
                                                      doorWidth + AShortOppositeCorridorWall.width,
                                                      Z_SIDE_WALL,
                                                      color,
                                                      &pinkWhiteStyle};

            struct Wall AShortCorridorWall = {{DoorOfAroomPoint.x-1,
                                               DoorOfAroomPoint.y,
                                               turningPointZ + doorWidth},
                                               doorHeight,
                                               DoorOfAroomPoint.z - turningPointZ - doorWidth,
                                               Z_SIDE_WALL,
                                               color,
                                               &pinkWhiteStyle};


            struct Wall AnOppositeZSIDECorridorWall= {{DoorOfOppositeRoomPoint.x + doorWidth+1,
                                                       DoorOfOppositeRoomPoint.y,
                                                       turningPointZ - 1},
                                                       doorHeight,
                                                       DoorOfAroomPoint.x - DoorOfOppositeRoomPoint.x,
                                                       X_SIDE_WALL,
                                                       color,
                                                       &pinkWhiteStyle};


            struct Wall AZSIDECorridorWall = {{DoorOfOppositeRoomPoint.x-1,
                                               DoorOfAroomPoint.y,
                                               turningPointZ + doorWidth},
                                               doorHeight,
                                               DoorOfAroomPoint.x - DoorOfOppositeRoomPoint.x,
                                               X_SIDE_WALL,
                                               color,
                                               &pinkWhiteStyle};

            if (DoorOfAroomPoint.x < DoorOfOppositeRoomPoint.x)
            {
               ALongCorridorWall.StartPoint.x = DoorOfAroomPoint.x - 1;
               AShortOppositeCorridorWall.StartPoint.x = DoorOfOppositeRoomPoint.x- 1;
               ALongOppositeCorridorWall.StartPoint.x = DoorOfOppositeRoomPoint.x + doorWidth; 
               AShortCorridorWall.StartPoint.x = DoorOfAroomPoint.x + doorWidth;
               AnOppositeZSIDECorridorWall.StartPoint.z = turningPointZ  + doorWidth; 
               AnOppositeZSIDECorridorWall.StartPoint.x = DoorOfAroomPoint.x + doorWidth;
               AnOppositeZSIDECorridorWall.width =  1 + DoorOfOppositeRoomPoint.x - DoorOfAroomPoint.x;
               AZSIDECorridorWall.StartPoint.z = turningPointZ -1; 
               AZSIDECorridorWall.StartPoint.x = DoorOfAroomPoint.x - 1;
               AZSIDECorridorWall.width = AnOppositeZSIDECorridorWall.width;
            }

            // Build Roof for Hallway
            if (DoorOfOppositeRoomPoint.x > DoorOfAroomPoint.x)
            {
               AMiddleCorridorRoofEndPoint.x  =  DoorOfOppositeRoomPoint.x +doorWidth;
               AMiddleCorridorRoofStartPoint.x = DoorOfAroomPoint.x -1;
            }

            BuildAWall(&AnOppositeZSIDECorridorWall);
            BuildAWall(&AZSIDECorridorWall);
            BuildAWall(&AShortCorridorWall);
            BuildAWall(&ALongCorridorWall);
            BuildAWall(&AShortOppositeCorridorWall);
            BuildAWall(&ALongOppositeCorridorWall);
            
            // Build Roofs of Corridors
            BuildABox(&AMiddleCorridorRoofStartPoint,&AMiddleCorridorRoofEndPoint,color,&pinkWhiteStyle);
            BuildABox(&ACorridorRoofStartPoint,&ACorridorRoofEndPoint,color,&pinkWhiteStyle);
            BuildABox(&AnOppositeCorridorRoofStartPoint,&AnOppositeCorridorRoofEndPoint,color,&pinkWhiteStyle);
            if(obj->m_state == READY)
            {
              addWallToMap2D(&(obj->m_a2DMap),&AnOppositeZSIDECorridorWall,brown,-1);
              addWallToMap2D(&(obj->m_a2DMap),&AZSIDECorridorWall,brown,-1);
              addWallToMap2D(&(obj->m_a2DMap),&AShortCorridorWall,brown,-1);
              addWallToMap2D(&(obj->m_a2DMap),&ALongCorridorWall,brown,-1);
              addWallToMap2D(&(obj->m_a2DMap),&AShortOppositeCorridorWall,brown,-1);
              addWallToMap2D(&(obj->m_a2DMap),&ALongOppositeCorridorWall,brown,-1);
            }

      }
   }
}

int normalColorStyle(int originalColor,int x,int y,int z)
{
   return originalColor;
}

int pinkWhiteStyle(int originalColor,int x,int y,int z)
{
   return (((y%2==1)&&(x%2==0)&&(z%2==1))||((y%2==0)&&(x%2==1)&&(z%2==0)))?5:6;  
}


int darkAndLightBrownFloorStyle(int originalColor,int x,int y,int z)
{
   return (((x%2==0)&&(z%2==1))||((x%2==1)&&(z%2==0)))?20:21;  // chessboard pattern
}


int findMaxValue(int a,int b)
{
   int ret = a;
   if (a < b)ret = b;
   return ret;
}
int findMinValue(int a,int b)
{
   int ret = a;
   if (a > b)ret = b;
   return ret;
}

int boundValue(int max,int min,int originValue)
{
   int ret = originValue;
   if(originValue > max)
      ret = max;
   if(originValue < min)
      ret = min;
   return ret;
}


void setParameterOfUnderground_defaultValue1(struct Underground *obj,int hasDownStair)
{
   obj->m_visitedState = NOT_VISITED;
   obj->m_downStairGroundLv = DEFAULT_UNDERGROUND_LV;
   obj->m_groundLv   = DEFAULT_UNDERGROUND_LV;
   obj->m_doorHeight = DEFAULT_DOOR_HEIGHT;
   obj->m_doorWidth = DEFAULT_DOOR_WIDTH;
   obj->m_roomWallHeight = DEFAULT_ROOM_HEIGHT;
   obj->m_gravity = DEFAULT_GRAVITY;
   obj->m_collisionMargin = DEFAULT_COLLISION_MARGIN;
   obj->m_sparForCorridorsX = DEFAULT_SPARFORCORRIDORS_X;
   obj->m_sparForCorridorsZ = DEFAULT_SPARFORCORRIDORS_Z;
   obj->m_sparForRoomSizeMin = DEFAULT_ROOM_SIZE_MIN;
   obj->m_sparForRoomSizeMax = DEFAULT_ROOM_SIZE_MAX;
   obj->m_defaultRoomColor = DEFAULT_ROOM_COLOR;
   obj->m_defaultUnitCubeColor = DEFAULT_CUBE_COLOR;
   obj->m_stairOption = hasDownStair;
   obj->m_upViewPoint = (struct Point){-1,-1,-1};
   obj->m_downViewPoint = (struct Point){-1,-1,-1};
   obj->m_upStairRoomId = -1;
   obj->m_downStairRoomId = -1;
   obj->m_state = READY;

   
}
void createUnderground(struct Underground *obj)
{
   int indexOfRoom = 0;
   int numRoom = DEFAULT_NUM_ROOM;
   const struct Point DoorInitialPoint = {-1, -1, -1};
   int ViewPointID = getRandomNumber(0, DEFAULT_NUM_ROOM-1); //which room the view point will be
   int downStairID = getRandomNumber(0,DEFAULT_NUM_ROOM-1);

   int stairDirection = EAST;
   int foundGoodPlaceForUpStair = 0;
   int foundGoodPlaceForDownStair = 0;
   int foundPlaceForUpStairResult = 0;
   struct Point stairPoint;
   struct Point goodStairPoint;
   int goodStairDirection;
   // Room's attributes
   int xLenght = 0;
   int zLenght = 0;
   int roomX = 0;
   int roomZ = 0;
   int i =0;
   int protectInfinityLoopVal = 30;

   flycontrol = 0;
   makeWorld();
   //ground custom colors (brown and light brown)
   //light brown color
   setUserColour(20, 0.724, 0.404, 0.116, 1.0, 0.2, 0.2, 0.2, 1.0);
   //dark brown
   setUserColour(21, 0.404, 0.268, 0.132, 1.0, 0.2, 0.2, 0.2, 1.0);
   struct Point FloorStartPoint = {0,obj->m_groundLv,0};
   struct Point FloorEndPoint = {WORLDX-1,obj->m_groundLv,WORLDZ-1};
   // Build Roofs of Corridors
   //printf("create underground state: -2 \n");
   while((protectInfinityLoopVal-- > 0) && ((foundGoodPlaceForUpStair == 0 )||(foundGoodPlaceForDownStair == 0)))
   {
      foundGoodPlaceForUpStair =0;
      foundGoodPlaceForDownStair = 0;
      makeWorld();
      BuildABox(&FloorStartPoint,&FloorEndPoint,0,&darkAndLightBrownFloorStyle);

      // initi map parameter
      if(obj->m_state == READY)
      { 
        initialMap2D(&(obj->m_a2DMap));
      }
         //printf("create underground  state:-1 \n");
      for (indexOfRoom = 0; indexOfRoom < DEFAULT_NUM_ROOM; indexOfRoom++)
      {
         if(obj->m_state == READY)
         {      

            // initial Room parameter    
            for (i = WEST; i <=NORTH;i++)
               obj->m_rooms[indexOfRoom].DoorPosition[i] = DoorInitialPoint;
            // Find spar area between 2 rooms to build corridors and hallways
            obj->m_sparForCorridorsX = (dimensionOfGrid3x3[indexOfRoom][AREA_X_LENGHT] - obj->m_sparForRoomSizeMax)/2;
            obj->m_sparForCorridorsZ = (dimensionOfGrid3x3[indexOfRoom][AREA_Z_LENGHT] - obj->m_sparForRoomSizeMax)/2;
            // define door direction to Room attribute

            // random size of a room
            xLenght = getRandomNumber(obj->m_sparForRoomSizeMin,obj->m_sparForRoomSizeMax);
            zLenght = getRandomNumber(obj->m_sparForRoomSizeMin,obj->m_sparForRoomSizeMax);
            // Random location of a room
            roomX = obj->m_sparForCorridorsX + dimensionOfGrid3x3[indexOfRoom][AREA_XP] + getRandomNumber(0,dimensionOfGrid3x3[indexOfRoom][AREA_X_LENGHT]-xLenght-obj->m_sparForCorridorsX-obj->m_sparForCorridorsX);
            roomZ=  obj->m_sparForCorridorsZ + dimensionOfGrid3x3[indexOfRoom][AREA_ZP] + getRandomNumber(0,dimensionOfGrid3x3[indexOfRoom][AREA_Z_LENGHT]-zLenght-obj->m_sparForCorridorsZ-obj->m_sparForCorridorsZ);

            // Reference point to build a room
            const struct Point RoomStartPoint = {roomX,obj->m_groundLv ,roomZ};
   
            // create a few Cubes 1 high
            // build A room
            obj->m_rooms[indexOfRoom]= BuildEasyRoom(&RoomStartPoint,xLenght,zLenght,obj->m_roomWallHeight,HAVE_ROOF,NOT_HAVE_GROUND,obj->m_defaultRoomColor,obj->m_defaultUnitCubeColor,BUILT_NOW);
            obj->m_rooms[indexOfRoom].doorWidth = obj->m_doorWidth;
            obj->m_rooms[indexOfRoom].doorHeight = obj->m_doorHeight;

            BuildDoorsWestVsEast(indexOfRoom,obj);
            BuildDoorsSouthVsNorth(indexOfRoom,obj);

            foundPlaceForUpStairResult = findProperPositionToPlaceUpStairInRoomOfUnderground(obj,&stairPoint,&stairDirection,DEFAULT_STAIR_WIDTH,DEFAULT_STAIR_STEP_NUM,indexOfRoom);
            if((foundPlaceForUpStairResult == 1) && (foundPlaceForUpStairResult !=3))
            {
               goodStairPoint = stairPoint;
               goodStairDirection = stairDirection;
               foundGoodPlaceForUpStair = 1;
               obj->m_upStairRoomId  = indexOfRoom;
            }
            if (ViewPointID == indexOfRoom)
            {  
              // printf("indexOfRoom : %d \n",indexOfRoom);
               if (foundPlaceForUpStairResult == 1)
               {            
                  obj->m_upStair = setStairAttribute(goodStairPoint,stairDirection,DEFAULT_STAIR_WIDTH,DEFAULT_STAIR_STEP_NUM,UP_STAIR,5); // white
                  foundPlaceForUpStairResult = 3;
                  obj->m_upStairRoomId  = indexOfRoom;

                  // find start position of View point
                  struct Point startP;
                  struct Point stopP;
                  findStartAndStopPointOfARoom(&(obj->m_rooms[obj->m_upStairRoomId]),&stopP,&startP);
                  obj->m_downViewPoint.x = startP.x+1;
                  obj->m_downViewPoint.z = startP.z+1;
                  obj->m_downViewPoint.y = 1+obj->m_groundLv;
                  if(world[obj->m_downViewPoint.x][obj->m_downViewPoint.y][obj->m_downViewPoint.z] != 0) obj->m_downViewPoint.y++; // has a cube


               }

            }
         }
         else if (obj->m_state == GENERATED_UNDERGROUND_DONE)
         {
            // no need to loop for finding a good position of stair
            protectInfinityLoopVal = 0;
            BuildARoom(&(obj->m_rooms[indexOfRoom]));
            for(i = 0; i < obj->m_rooms[indexOfRoom].numUnitCubes;i++)
            {
               plotWolrd(world,obj->m_rooms[indexOfRoom].unitCubePoint[i],obj->m_rooms[indexOfRoom].unitCubeColor);
            }
            BuildDoorsWestVsEast(indexOfRoom,obj);
            BuildDoorsSouthVsNorth(indexOfRoom,obj);
         }
      }


      // locate up stair
      if ((foundGoodPlaceForUpStair == 1) &&(obj->m_state == READY))
      {
        obj->m_upStair = setStairAttribute(goodStairPoint,stairDirection,DEFAULT_STAIR_WIDTH,DEFAULT_STAIR_STEP_NUM,UP_STAIR,5); // white

        // find start position of View point
        struct Point startP;
        struct Point stopP;
        findStartAndStopPointOfARoom(&(obj->m_rooms[obj->m_upStairRoomId]),&stopP,&startP);
        obj->m_downViewPoint.x = startP.x+1;
        obj->m_downViewPoint.z = startP.z+1;
        obj->m_downViewPoint.y = 1+obj->m_groundLv;
        if(world[obj->m_downViewPoint.x][obj->m_downViewPoint.y][obj->m_downViewPoint.z] != 0) obj->m_downViewPoint.y++; // has a cube
      }
       // locate down stair
      if((obj->m_state == READY)&&((obj->m_stairOption == BOTH_UP_DOWN_STAIR) || (obj->m_stairOption == ONLY_DOWN_STAIR)))
      {
        if(findProperPositionToPlaceDownStairInRoomOfUnderground(obj,&stairPoint,&stairDirection,DEFAULT_STAIR_WIDTH,DEFAULT_STAIR_STEP_NUM) == 0) //ok
        {
          foundGoodPlaceForDownStair = 1;    
          int stairColor = 23;//Grey   
          setUserColour(stairColor, 0.604, 0.604, 0.604, 1.0, 0.2, 0.2, 0.2, 1.0); //Grey 
          obj->m_downStair = setStairAttribute(stairPoint,stairDirection,DEFAULT_STAIR_WIDTH,DEFAULT_STAIR_STEP_NUM,DOWN_STAIR,stairColor); // Grey  


          struct Point startP;
          struct Point stopP;
          findStartAndStopPointOfARoom(&(obj->m_rooms[obj->m_downStairRoomId]),&stopP,&startP);
          obj->m_upViewPoint.x = startP.x+1;
          obj->m_upViewPoint.z = startP.z+1;
          obj->m_upViewPoint.y = 1+obj->m_groundLv;
          if(world[obj->m_upViewPoint.x][obj->m_upViewPoint.y][obj->m_upViewPoint.z] != 0) obj->m_upViewPoint.y++; // has a cube
        }
        else
        {
          foundGoodPlaceForDownStair = 0;
        }
      }
        

      if ((obj->m_state == GENERATED_UNDERGROUND_DONE) || (foundGoodPlaceForUpStair == 1) || (foundPlaceForUpStairResult ==3))
      {
        BuildStair(&(obj->m_upStair));
      }
      if ((foundGoodPlaceForDownStair == 1) || ((obj->m_state == GENERATED_UNDERGROUND_DONE)&&((obj->m_stairOption == BOTH_UP_DOWN_STAIR) || (obj->m_stairOption == ONLY_DOWN_STAIR))))
      {   
        int stairColor = 23;//Grey  
        setUserColour(stairColor, 0.604, 0.604, 0.604, 1.0, 0.2, 0.2, 0.2, 1.0); //Grey 
        BuildStair(&(obj->m_downStair));
      }

   }
   if (obj->m_state == READY)
   {
      setStairPositionInMap2D(obj);
      setRoomsPositionInMap2D(obj);
      setCubePositionInMap2D(obj);   
      //printf("create underground state:-1.1 \n");
      obj->m_state = GENERATED_UNDERGROUND_DONE;
   }
//printf("Never found : %d , proloop:%d\n ",foundGoodPlaceForUpStair,protectInfinityLoopVal);
   g_floorLv =findMinValue(obj->m_groundLv,obj->m_downStairGroundLv);
   if ((obj->m_visitedState == ALREADY_DOWN) || (obj->m_visitedState == NOT_VISITED))
   {
      setViewPosition(-1 * (obj->m_downViewPoint.x), -1 * (obj->m_downViewPoint.y), -1 * (obj->m_downViewPoint.z));
      //obj->m_visitedState = ALREADY_DOWN;
   }
   else if (obj->m_visitedState == ALREADY_UP)
   {
      setViewPosition(-1 * (obj->m_upViewPoint.x), -1 * (obj->m_upViewPoint.y), -1 * (obj->m_upViewPoint.z));
   }
   //printf("create underground state:0 \n");
  // setViewPosition(-1 * (obj->m_currentViewPoint.x), -1 * (40), -1 * (obj->m_currentViewPoint.z));
}


struct stair setStairAttribute(const struct Point StartPoint,const int frontOfStairDirection,const int stairWidth, const int numStair,const int type,const int color)
{
   struct stair obj;
   obj.numberStair = numStair;
   obj.direction = frontOfStairDirection;
   obj.color = color;
   obj.type = type;
   obj.StartPoint = StartPoint;
   if(frontOfStairDirection <= EAST)
   {

      obj.aRoom = BuildEasyRoom(&StartPoint,numStair+2,stairWidth+2,numStair,HAVE_ROOF,HAVE_GROUND,color,0,BUILT_LATER);
   }
   else
   {
      obj.aRoom = BuildEasyRoom(&StartPoint,stairWidth+2,numStair+2,numStair,HAVE_ROOF,HAVE_GROUND,color,0,BUILT_LATER);
   }
   return obj;
}
void locateAndBuildStairOnTerrain(struct OnGround *obj)
{
      setUserColour(20, 0.724, 0.404, 0.116, 1.0, 0.2, 0.2, 0.2, 1.0);
      setUserColour(21, 0.404, 0.268, 0.132, 1.0, 0.2, 0.2, 0.2, 1.0); 

      int i,j;
      long int k = 5;
      struct Point startStairPoint =  getReferentStairPoint(&(obj->downStair),START_POINT);
      struct Point stopStairPoint =  getReferentStairPoint(&(obj->downStair),STOP_POINT);
      int xMin = findMinValue(startStairPoint.x,stopStairPoint.x);
      int xMax = findMaxValue(startStairPoint.x,stopStairPoint.x);
      int zMin = findMinValue(startStairPoint.z,stopStairPoint.z);
      int zMax = findMaxValue(startStairPoint.z,stopStairPoint.z);
      int highestLvOfCubesNum = 0;
      int lowestLvOfCubesNum = 0;
      clock_t timeRef = clock();
      if(obj->state == READY)
      {
         createCloudsInDefinedArea(obj);
         timeRef = clock();
         while(((obj->highestLvOfCubesNum >= highestLvOfCubesNum)||(obj->lowestLvOfCubesNum >= lowestLvOfCubesNum))&&(k-- > 0))
         {  
            highestLvOfCubesNum = 0;  
            lowestLvOfCubesNum = 0;
            makeWorld();        
            memset(obj->terrain,0,sizeof(obj->terrain));
            for (i=xMin;i<=xMax;i++)
               for(j=zMin;j<=zMax;j++)
                  obj->terrain[i][j] = startStairPoint.y;
            generateTerrain(obj,obj->terrain,&startStairPoint,&stopStairPoint);
            for(i=0;i<WORLDX;i++)
               for(j=0;j<WORLDZ;j++)
                  if (obj->terrain[i][j] == obj->highestLv) 
                  {
                     world[i][obj->highestLv][j] = 5;
                     highestLvOfCubesNum++;

                  }
                  else if (obj->terrain[i][j] == obj->lowestLv) 
                  {
                     world[i][obj->lowestLv][j] = 21;
                     lowestLvOfCubesNum++;
                  }
         }
         /*
         world[0][obj->terrain[0][0]][0] = 3;
         world[99][obj->terrain[99][0]][0] = 6;
         world[99][obj->terrain[99][99]][99] = 7;
         world[0][obj->terrain[0][99]][99] = 2;
         */
         timeRef = clock()-timeRef;
         //printf("HCN:%d, LCN:%d , k:%ld , time:%f second\n",highestLvOfCubesNum,lowestLvOfCubesNum,k,((float)timeRef)/CLOCKS_PER_SEC);
         obj->highestLvOfCubesNum = highestLvOfCubesNum;
         obj->lowestLvOfCubesNum = lowestLvOfCubesNum;


      }
      else if (obj->state == TERRAIN_AND_STAIR_IS_BUILT)
      {
         for(i=0;i<WORLDX;i++)
            for(j=0;j<WORLDZ;j++)
               if (obj->terrain[i][j] == obj->highestLv) 
               {
                  world[i][obj->highestLv][j] = 5;
               }
               else if (obj->terrain[i][j] == obj->lowestLv) 
               {
                  world[i][obj->lowestLv][j] = 21;
               }
               else
               {
                  world[i][obj->terrain[i][j]][j] = 1;
               }
      }
      g_floorLv = obj->lowestLv; 
      setViewPosition(obj->downStair.StartPoint.x*(-1),(-1)*(startStairPoint.y+1),obj->downStair.StartPoint.z*(-1));

      BuildStair(&(obj->downStair));

}
void BuildStair(const struct stair *obj)
{
   int i,j,k;  
   const struct Room *aRoom = &(obj->aRoom);
   const struct Wall *Walls = aRoom->Walls;
   const int color = obj->color;
   const int direction = obj->direction;

   struct Point topStairPoint1;
   struct Point topStairPoint2;
   struct Point downSideStartPoint;
   struct Point downSideStopPoint;

   struct Point upSideStartPoint;
   struct Point upSideStopPoint;
   if (direction <= EAST)
   {
         downSideStartPoint = (struct Point){Walls[direction].StartPoint.x,
                                     Walls[direction].StartPoint.y,
                                     Walls[direction].StartPoint.z + 1};
         downSideStopPoint  = (struct Point){downSideStartPoint.x,
                                     downSideStartPoint.y+Walls[direction].width-3,
                                     downSideStartPoint.z+Walls[direction].width-3}; 
         upSideStartPoint = (struct Point){Walls[NORTH].StartPoint.x ,
                                     Walls[NORTH].StartPoint.y + Walls[NORTH].height,
                                     Walls[NORTH].StartPoint.z- Walls[direction].width+2};
         upSideStopPoint  = (struct Point){Walls[NORTH].StartPoint.x + Walls[direction].width-3,
                                     upSideStartPoint.y,
                                     Walls[NORTH].StartPoint.z-1}; 

         if(direction == WEST)
         {
            upSideStartPoint = (struct Point){Walls[NORTH].StartPoint.x +Walls[NORTH].width-1,
                                        Walls[NORTH].StartPoint.y + Walls[NORTH].height,
                                        Walls[NORTH].StartPoint.z- Walls[direction].width+2};
            upSideStopPoint  = (struct Point){Walls[NORTH].StartPoint.x+Walls[NORTH].width - Walls[direction].width+2,
                                        upSideStartPoint.y,
                                        Walls[NORTH].StartPoint.z-1};
         }  

   }
   else
   {
         downSideStartPoint = (struct Point){Walls[direction].StartPoint.x,
                                     Walls[direction].StartPoint.y,
                                     Walls[direction].StartPoint.z};
         downSideStopPoint  = (struct Point){downSideStartPoint.x+Walls[direction].width-1,
                                     downSideStartPoint.y+Walls[direction].width-1,
                                     downSideStartPoint.z};  
         upSideStartPoint = (struct Point){Walls[NORTH].StartPoint.x,
                                     Walls[NORTH].StartPoint.y + Walls[NORTH].height,
                                     Walls[NORTH].StartPoint.z-Walls[EAST].width +Walls[direction].width+1};
         upSideStopPoint  = (struct Point){Walls[NORTH].StartPoint.x +Walls[NORTH].width-1,
                                     upSideStartPoint.y,
                                     Walls[NORTH].StartPoint.z-Walls[EAST].width+2}; 
         if(direction == SOUTH)
         {
            upSideStartPoint = (struct Point){Walls[NORTH].StartPoint.x,
                                        Walls[NORTH].StartPoint.y + Walls[NORTH].height,
                                        Walls[NORTH].StartPoint.z -Walls[direction].width };
            upSideStopPoint  = (struct Point){Walls[NORTH].StartPoint.x+Walls[direction].width-1,
                                        upSideStartPoint.y,
                                        Walls[NORTH].StartPoint.z-1}; 
         }
   }
   topStairPoint1 = getReferentStairPoint(obj,0);
   topStairPoint1.y = upSideStopPoint.y;
   topStairPoint2 = getReferentStairPoint(obj,1);
   topStairPoint2.y = downSideStartPoint.y;

   // Clear top roof before creating stair
   BuildABox(&topStairPoint1,&topStairPoint2,0,&normalColorStyle);
   if(direction == WEST)
   {
      for(j=0;j<Walls[direction].height;j++ )
         for(k = 0; k <Walls[direction].width-2;k++)
            world[Walls[NORTH].StartPoint.x+j][Walls[NORTH].StartPoint.y+j][Walls[NORTH].StartPoint.z+2- Walls[WEST].width+k] = color;
   }
   else if(direction == EAST)
   {
      for(j=0;j<Walls[direction].height;j++ )
         for(k = 0; k <Walls[direction].width-2;k++)
            world[Walls[EAST].StartPoint.x-j-1][Walls[NORTH].StartPoint.y+j][1+Walls[EAST].StartPoint.z+k] = color;
      
   }
   else if(direction == SOUTH)
   {
      for(j=0;j<Walls[direction].height;j++ )
         for(k = 0; k <Walls[direction].width;k++)
            world[Walls[SOUTH].StartPoint.x+k][Walls[NORTH].StartPoint.y+j][1+Walls[SOUTH].StartPoint.z+j] = color;
      
   }
   else if (direction == NORTH)
   {
      for(j=0;j<Walls[direction].height;j++ )
         for(k = 0; k <Walls[direction].width;k++)
            world[Walls[NORTH].StartPoint.x+k][Walls[NORTH].StartPoint.y+j][1+Walls[NORTH].StartPoint.z-2-j] = color;
      
   }

   // create stair
   BuildARoom(&(obj->aRoom));
   BuildABox(&downSideStartPoint,&downSideStopPoint,0,&normalColorStyle);
   BuildABox(&upSideStartPoint,&upSideStopPoint,0,&normalColorStyle);
}

struct Point getReferentStairPoint(const struct stair *obj,int startOrStop) // 0 = start, 1 = stop
{
   struct Point startPoint;
   struct Point stopPoint;
   struct Point referentPoint;
   const struct Room *aRoom = &(obj->aRoom);
   const struct Wall *Walls = aRoom->Walls;
   const int direction = obj->direction;
   switch (direction)
   {
      case WEST:
            startPoint = (struct Point){Walls[EAST].StartPoint.x, Walls[EAST].StartPoint.y+Walls[EAST].height,Walls[EAST].StartPoint.z};
            stopPoint  = (struct Point){Walls[WEST].StartPoint.x, Walls[EAST].StartPoint.y+Walls[EAST].height, Walls[WEST].StartPoint.z + Walls[WEST].width -1};
      break;
      case EAST:
            startPoint = (struct Point){Walls[WEST].StartPoint.x, Walls[WEST].StartPoint.y+Walls[WEST].height,Walls[WEST].width-1 +Walls[WEST].StartPoint.z};
            stopPoint  = (struct Point){Walls[EAST].StartPoint.x, Walls[EAST].StartPoint.y+Walls[EAST].height, Walls[EAST].StartPoint.z};
      break;
      case SOUTH:
            startPoint = (struct Point){Walls[NORTH].width +Walls[NORTH].StartPoint.x,Walls[NORTH].StartPoint.y+Walls[NORTH].height,Walls[NORTH].StartPoint.z};
            stopPoint  = (struct Point){Walls[SOUTH].StartPoint.x-1, Walls[SOUTH].StartPoint.y+Walls[SOUTH].height, Walls[SOUTH].StartPoint.z};
      break;
      case NORTH:
            startPoint = (struct Point){Walls[SOUTH].width +Walls[SOUTH].StartPoint.x,Walls[SOUTH].StartPoint.y+Walls[SOUTH].height,Walls[SOUTH].StartPoint.z}; 
            stopPoint  = (struct Point){Walls[NORTH].StartPoint.x-1, Walls[NORTH].StartPoint.y+Walls[NORTH].height, Walls[NORTH].StartPoint.z};
      break;
      default:
            referentPoint = (struct Point){0,0,0};
      break;
   }
   referentPoint = stopPoint;
   if (startOrStop == 0)
   {
      referentPoint = startPoint;
   }
   return referentPoint;
}

void generateTerrain(struct OnGround *obj,unsigned char terrain[WORLDX][WORLDZ],const struct Point *startBoundPoint,const struct Point *stopBoundPoint)
{
      int protectInfinityLoopVal =20000;
      int i =0;
      int j= 0;
      int currentDirection = WEST;
      int minRangVal = WORLDY;
      int maxRangVal = -1;
      int currentHeight = 0;
      int maxHeight = 0;
      int minHeight = 100;
      int maxX = findMaxValue(startBoundPoint->x,stopBoundPoint->x);
      int minX = findMinValue(startBoundPoint->x,stopBoundPoint->x);
      int maxZ = findMaxValue(startBoundPoint->z,stopBoundPoint->z);
      int minZ = findMinValue(startBoundPoint->z,stopBoundPoint->z);
      struct Point southWestP = {findMaxValue(minX+directionOffset[SOUTH_WEST][0],0),startBoundPoint->y,findMaxValue(minZ+directionOffset[SOUTH_WEST][1],0)};
      struct Point northWestP = {findMaxValue(minX+directionOffset[NORTH_WEST][0],0),startBoundPoint->y,findMinValue(maxZ+directionOffset[NORTH_WEST][1],WORLDZ-1)};
      struct Point northEastP = {findMinValue(maxX+directionOffset[NORTH_EAST][0],WORLDX-1),startBoundPoint->y,findMinValue(maxZ+directionOffset[NORTH_EAST][1],WORLDZ-1)};
      struct Point southEastP = {findMinValue(maxX+directionOffset[SOUTH_EAST][0],WORLDX-1),startBoundPoint->y,findMaxValue(minZ+directionOffset[SOUTH_EAST][1],0)};


      int fullfillNorthEastArea = (northEastP.x == WORLDX-1)&&(northEastP.z == WORLDZ-1)&&(readTerrain(terrain,northEastP.x-1,northEastP.z) >0) &&  (readTerrain(terrain,northEastP.x,northEastP.z-1) >0);
      int fullfillSouthEastArea = (southEastP.x == WORLDX-1)&&(southEastP.z == 0)&&(readTerrain(terrain,southEastP.x,southEastP.z+1) >0) &&  (readTerrain(terrain,southEastP.x-1,southEastP.z) >0);
      int fullfillNorthWestArea = (northWestP.x == 0)&&(northWestP.z == WORLDZ-1)&&(readTerrain(terrain,northWestP.x+1,northWestP.z) >0) &&  (readTerrain(terrain,northWestP.x,northWestP.z-1) >0);
      int fullfillSouthWestArea = (southWestP.x == 0)&&(southWestP.z == 0)&&(readTerrain(terrain,southWestP.x+1,southWestP.z) >0) &&  (readTerrain(terrain,southWestP.x,southWestP.z+1) >0);
      int fullfillArea = fullfillNorthEastArea && fullfillSouthEastArea && fullfillNorthWestArea && fullfillSouthWestArea;

      struct Point p1 = southWestP;
      struct Point p2 = southWestP;
      while ((fullfillArea == 0) && (protectInfinityLoopVal-- > 0))
      {

         p2.x =  boundValue(WORLDX-1,0,p1.x);
         p2.z =  boundValue(WORLDZ-1,0,p1.z);
         if((readTerrain(terrain,p2.x,p2.z) == 0))
         {
            maxRangVal = -1;
            minRangVal = 100;
            for(j=0;j<8;j++)
               getMaxMinAtTerrainPoint(terrain,&p2,j,&maxRangVal,&minRangVal);

            if (maxRangVal == minRangVal)
            {

               currentHeight = minRangVal + getRandomNumber(1,3)-2;

            }
            else if (maxRangVal == (minRangVal+1))
            {
               currentHeight =  getRandomNumber(minRangVal,maxRangVal);
            }
            else
            {
               currentHeight =  maxRangVal-1;
            }
            currentHeight = boundValue(DEFAULT_HIGHEST_TERRAIN,DEFAULT_LOWEST_TERRAIN,currentHeight);

            maxHeight = findMaxValue(maxHeight,currentHeight);
            minHeight = findMinValue(minHeight,currentHeight);
            p2.y = currentHeight;
            struct Point startWorldP = {0,0,0};
            struct Point stopWorldP = {99,49,99};
            if ((isIn2DBound(startBoundPoint,stopBoundPoint,&p1) == 0) && (isIn3DBound(&startWorldP,&stopWorldP,&p2) == 1))
            {  
                  terrain[p2.x][p2.z] = currentHeight;
                  world[p2.x][currentHeight][p2.z] = 1;
            }
         }

         // search all direction
         if ((((southWestP.x < 0) && (southWestP.z < 0)) == 0) && (p1.z < northWestP.z) && (p1.x == northWestP.x))
         {
            p1.z++;
            currentDirection = NORTH;
           //printf("it is true(%d), p1:(%d,%d),northWestP.z:%d \n",(((southWestP.x == 0) && (southWestP.z == 0)) == 0),p1.x,p1.z,northWestP.z);
         }
         else if ((((northWestP.x < 0) && (northWestP.z > WORLDZ-1)) == 0) && (p1.x < northEastP.x) && (p1.z == northEastP.z))
         {

            p1.x++;
            currentDirection =  EAST;
           //printf("it is false1(%d), p1:(%d,%d), northEastP.x:%d \n",(((northWestP.x == 0) && (northWestP.z == WORLDZ-1)) == 0),p1.x,p1.z,northEastP.x);
         }
         else if ((((northEastP.x> WORLDX-1) && (northEastP.z > WORLDZ-1)) == 0) && (p1.z > southEastP.z) && (p1.x == southEastP.x))
         {
            p1.z--;
            currentDirection = SOUTH;
          // printf("it is false2(%d), p1:(%d,%d), southEastP.z:%d \n",(((northEastP.x == WORLDX-1) && (northEastP.z == WORLDZ-1)) == 0),p1.x,p1.z,southEastP.z);
         }
         else if ((((southEastP.x > WORLDX-1) && (southEastP.z <0)) == 0) && (p1.x > southWestP.x) && (p1.z == southWestP.z))
         {
            p1.x--;
            currentDirection = WEST;
          //  printf("it is false3(%d), p1:(%d,%d), southEastP.z:%d \n",(((northEastP.x == WORLDX-1) && (northEastP.z == WORLDZ-1)) == 0),p1.x,p1.z,southEastP.z);
         }


          if ((southWestP.x == p1.x) && (southWestP.z == p1.z) )
         {
            //printf("Set1 terrain(%d,%d) : %d, kkkk:%d  ,fullfillNorthEastArea:%d , SouthEastArea:%d,  NorthWestArea:%d, SouthWestArea:%d\n",p1.x,p1.z,readTerrain(terrain,p1.x,p1.z),protectInfinityLoopVal,fullfillNorthEastArea,fullfillSouthEastArea,fullfillNorthWestArea,fullfillSouthWestArea);
            updateBoundaryPointsForTerrainStyle2(&southWestP,&northWestP,&northEastP,&southEastP);
            p1 = southWestP;
            currentDirection = NORTH;
         }
         else if ((fullfillNorthEastArea == 1) && (fullfillSouthEastArea == 0)&& (fullfillNorthWestArea == 0)&& (currentDirection == EAST))
         {
            //printf("Reset direction 2 terrain(%d,%d) : %d, kkkk:%d \n",p1.x,p1.z,readTerrain(terrain,p1.x,p1.z),protectInfinityLoopVal);
            p1 = southEastP;
         }
         else if ((fullfillNorthWestArea == 1) && (fullfillNorthEastArea == 0) && (fullfillSouthWestArea == 0) && (currentDirection == NORTH))
         {
            //printf("Reset direction 3 terrain(%d,%d) : %d, kkkk:%d \n",p1.x,p1.z,readTerrain(terrain,p1.x,p1.z),protectInfinityLoopVal);
            p1 = northEastP;
         }
         
         else if ((fullfillSouthWestArea == 1)&&(fullfillNorthWestArea == 0) && (fullfillSouthEastArea == 0) && (currentDirection == WEST))
         {
            //printf("Reset direction 4 terrain(%d,%d) : %d, kkkk:%d \n",p1.x,p1.z,readTerrain(terrain,p1.x,p1.z),protectInfinityLoopVal);
            updateBoundaryPointsForTerrainStyle2(&southWestP,&northWestP,&northEastP,&southEastP);
            p1 = northWestP;

         }
         else if ((fullfillSouthEastArea == 1)&&(fullfillSouthWestArea == 0)&& (fullfillNorthEastArea == 0) && (currentDirection == SOUTH))
         {
            //printf("Reset direction 5 terrain(%d,%d) : %d, kkkk:%d \n",p1.x,p1.z,readTerrain(terrain,p1.x,p1.z),protectInfinityLoopVal);
            updateBoundaryPointsForTerrainStyle2(&southWestP,&northWestP,&northEastP,&southEastP);
            p1 = southWestP;
         }
         else if ((fullfillSouthEastArea == 0)&&(fullfillSouthWestArea == 0)  && (southWestP.z == 0)&&(readTerrain(terrain,southWestP.x+1,southWestP.z) > 0) && (readTerrain(terrain,southEastP.x-1,southEastP.z) > 0)&& (currentDirection == WEST))
         {
            //printf("Reset direction 6 terrain(%d,%d) : %d, kkkk:%d \n",p1.x,p1.z,readTerrain(terrain,p1.x,p1.z),protectInfinityLoopVal);
            updateBoundaryPointsForTerrainStyle2(&southWestP,&northWestP,&northEastP,&southEastP);
            p1 = southWestP;
         } 
         else if ((fullfillNorthWestArea == 0) && (fullfillSouthWestArea == 0) && (southWestP.x == 0)&& (readTerrain(terrain,southWestP.x,southWestP.z+1) > 0) && (readTerrain(terrain,northWestP.x,northWestP.z-1) > 0)&& (currentDirection == NORTH))
         {
            //printf("Reset direction 7 terrain(%d,%d) : %d, kkkk:%d \n",p1.x,p1.z,readTerrain(terrain,p1.x,p1.z),protectInfinityLoopVal);
            p1 = northWestP;
         }      
         else if ((fullfillNorthEastArea == 0) && (fullfillSouthEastArea == 0) && (northEastP.x == WORLDX-1) && (readTerrain(terrain,southEastP.x,southEastP.z+1) > 0) && (readTerrain(terrain,northEastP.x,northEastP.z-1) > 0) && (currentDirection == SOUTH))
         {
            //printf("Reset direction 8 terrain(%d,%d) : %d, kkkk:%d \n",p1.x,p1.z,readTerrain(terrain,p1.x,p1.z),protectInfinityLoopVal);
            p1 = southEastP;
         }  
         else if ((fullfillNorthEastArea == 0) && (fullfillNorthWestArea == 0) && (northEastP.z == WORLDZ-1)&& (readTerrain(terrain,northEastP.x-1,northEastP.z) > 0) && (readTerrain(terrain,northWestP.x+1,northWestP.z) > 0) && (currentDirection == EAST))
         {
            //printf("Reset direction 9 terrain(%d,%d) : %d, kkkk:%d \n",p1.x,p1.z,readTerrain(terrain,p1.x,p1.z),protectInfinityLoopVal);
            p1 = northEastP;
         }
         else if ((fullfillNorthWestArea == fullfillSouthWestArea) && (fullfillSouthWestArea == 1) && (fullfillNorthEastArea == 0) && (currentDirection == WEST))
         {

            //printf("Reset direction 10 terrain(%d,%d) : %d, kkkk:%d \n",p1.x,p1.z,readTerrain(terrain,p1.x,p1.z),protectInfinityLoopVal);
            updateBoundaryPointsForTerrainStyle2(&southWestP,&northWestP,&northEastP,&southEastP);
             p1 = northEastP;
         }
         else if ((fullfillNorthWestArea == fullfillNorthEastArea) && (fullfillNorthEastArea == 1) && (fullfillSouthWestArea == 0) && (currentDirection == NORTH))
         {
            //printf("Reset direction 11 terrain(%d,%d) : %d, kkkk:%d \n",p1.x,p1.z,readTerrain(terrain,p1.x,p1.z),protectInfinityLoopVal);
             p1 = southEastP;
         }
         else if ((fullfillSouthEastArea == fullfillNorthEastArea) && (fullfillNorthEastArea == 1) && (fullfillSouthWestArea == 0) && (currentDirection == EAST))
         {
            //printf("Reset direction 12 terrain(%d,%d) : %d, kkkk:%d \n",p1.x,p1.z,readTerrain(terrain,p1.x,p1.z),protectInfinityLoopVal);
            updateBoundaryPointsForTerrainStyle2(&southWestP,&northWestP,&northEastP,&southEastP);
             p1 = southWestP;
         }
         else if ((fullfillSouthEastArea == fullfillSouthWestArea) && (fullfillSouthWestArea == 1) && (fullfillNorthEastArea == 0) && (currentDirection == SOUTH))
         {
            //printf("Reset direction 13 terrain(%d,%d) : %d, kkkk:%d \n",p1.x,p1.z,readTerrain(terrain,p1.x,p1.z),protectInfinityLoopVal);
            updateBoundaryPointsForTerrainStyle2(&southWestP,&northWestP,&northEastP,&southEastP);
             p1 = northWestP;
         }

         fullfillNorthEastArea = (northEastP.x == WORLDX-1)&&(northEastP.z == WORLDZ-1)&&(readTerrain(terrain,northEastP.x,northEastP.z) >0) && (readTerrain(terrain,northEastP.x-1,northEastP.z) >0) &&  (readTerrain(terrain,northEastP.x,northEastP.z-1) >0);
         fullfillSouthEastArea = (southEastP.x == WORLDX-1)&&(southEastP.z == 0)&&(readTerrain(terrain,southEastP.x,southEastP.z) >0)&&(readTerrain(terrain,southEastP.x,southEastP.z+1) >0) &&  (readTerrain(terrain,southEastP.x-1,southEastP.z) >0);
         fullfillNorthWestArea = (northWestP.x == 0)&&(northWestP.z == WORLDZ-1)&&(readTerrain(terrain,northWestP.x,northWestP.z) >0) &&(readTerrain(terrain,northWestP.x+1,northWestP.z) >0) &&  (readTerrain(terrain,northWestP.x,northWestP.z-1) >0);
         fullfillSouthWestArea = (southWestP.x == 0)&&(southWestP.z == 0)&&(readTerrain(terrain,southWestP.x,southWestP.z) >0) &&(readTerrain(terrain,southWestP.x+1,southWestP.z) >0) &&  (readTerrain(terrain,southWestP.x,southWestP.z+1) >0);
         fullfillArea = fullfillNorthEastArea && fullfillSouthEastArea && fullfillNorthWestArea && fullfillSouthWestArea;
      }
      obj->highestLv = maxHeight;
      obj->lowestLv = minHeight;
}

int isIn3DBound(const struct Point *startP,const struct Point *stopP,const struct Point *ref) // yes = 1, otherwise = 
{
   int isInX = (ref->x >= findMinValue(startP->x,stopP->x)) && (ref->x <= findMaxValue(startP->x,stopP->x));
   int isInY = (ref->y >= findMinValue(startP->y,stopP->y)) && (ref->y <= findMaxValue(startP->y,stopP->y));
   int isInZ = (ref->z >= findMinValue(startP->z,stopP->z)) && (ref->z <= findMaxValue(startP->z,stopP->z));
   return ((isInX+isInY+isInZ) == 3);
}

int readTerrain(const unsigned char terrain[WORLDX][WORLDZ],const int x,const int z)
{
   int canAccess = (x > -1) && (x <WORLDX) && (z > -1) && (z <WORLDZ);
   int ret = -1;
   if (canAccess == 1)
   {
      ret = terrain[x][z];
   }
   return ret;
}

int getMaxMinAtTerrainPoint(const unsigned char terrain[WORLDX][WORLDZ],const struct Point *refP,int direction,int *maxVal,int *minVal)
{
   int heightVal = readTerrain(terrain,refP->x+directionOffset[direction][0],refP->z+directionOffset[direction][1]);
   int ret = 0;
   if (heightVal > 0)
   {
      *maxVal = findMaxValue(*maxVal,heightVal);
      *minVal = findMinValue(*minVal,heightVal);
      ret = 1;
   }
   return ret;
}

int isIn2DBound(const struct Point *startP,const struct Point *stopP,const struct Point *ref)
{
   int isInX = (ref->x >= findMinValue(startP->x,stopP->x)) && (ref->x <= findMaxValue(startP->x,stopP->x));
   int isInZ = (ref->z >= findMinValue(startP->z,stopP->z)) && (ref->z <= findMaxValue(startP->z,stopP->z));
   return ((isInX+isInZ) == 2);
}

void updateBoundaryPointsForTerrainStyle2(struct Point *southWestP, struct Point *northWestP, struct Point *northEastP, struct Point *southEastP)
{
   int maxVal = WORLDX-1;
   int minVal = 0;
   *southWestP = (struct Point){findMaxValue(southWestP->x+directionOffset[SOUTH_WEST][0],minVal),0,findMaxValue(southWestP->z+directionOffset[SOUTH_WEST][1],minVal)};
   *northWestP = (struct Point){findMaxValue(northWestP->x+directionOffset[NORTH_WEST][0],minVal),0,findMinValue(northWestP->z+directionOffset[NORTH_WEST][1],maxVal)};
   *northEastP = (struct Point){findMinValue(northEastP->x+directionOffset[NORTH_EAST][0],maxVal),0,findMinValue(northEastP->z+directionOffset[NORTH_EAST][1],maxVal)};
   *southEastP = (struct Point){findMinValue(southEastP->x+directionOffset[SOUTH_EAST][0],maxVal),0,findMaxValue(southEastP->z+directionOffset[SOUTH_EAST][1],minVal)};  
}

void setPrameterOfOnGround_defauleValue1(struct OnGround *obj)
{   
   int numStairSteps = DEFAULT_STAIR_STEP_NUM;
   int StairStepWidth = DEFAULT_STAIR_WIDTH;
   struct Point stairRefPoint = {10+getRandomNumber(0,80),
                                 DEFAULT_LOWEST_TERRAIN+4+getRandomNumber(0,DEFAULT_HIGHEST_TERRAIN-DEFAULT_LOWEST_TERRAIN-numStairSteps-10),
                                 10+getRandomNumber(0,80)};

   int stairColor = 23;
   int stairType = DOWN_STAIR;
   setUserColour(stairColor, 0.604, 0.604, 0.604, 1.0, 0.2, 0.2, 0.2, 1.0); //Grey
   memset(obj->terrain,0,sizeof(obj->terrain));
   obj->downStair = setStairAttribute(stairRefPoint,getRandomNumber(WEST,NORTH),StairStepWidth,numStairSteps,stairType,stairColor);
   obj->highestLv = -1;
   obj->lowestLv = WORLDY;
   obj->highestLvOfCubesNum = DEFAULT_HIGHEST_TERRAIN_NUM;
   obj->lowestLvOfCubesNum = DEFAULT_LOWEST_TERRAIN_NUM;
   obj->state = READY;
}
void createOnGround(struct OnGround *obj)
{
   makeWorld();
   locateAndBuildStairOnTerrain(obj);
   obj->state = TERRAIN_AND_STAIR_IS_BUILT;
}
int isOnDownStair(struct OnGround *obj)
{
   float vxp,vyp,vzp;
   struct Point viewPosition = {-1,-1,-1};
   int ret = 0;
   if ((obj->state) == READY || obj->state == TERRAIN_AND_STAIR_IS_BUILT)
   {      
      struct Point startStairPoint =  getReferentStairPoint(&(obj->downStair),START_POINT);
      struct Point stopStairPoint =  getReferentStairPoint(&(obj->downStair),STOP_POINT);
      getViewPosition(&vxp,&vyp,&vzp);
      startStairPoint.y = obj->downStair.StartPoint.y+1;
      stopStairPoint.y = obj->downStair.StartPoint.y+1;
      viewPosition.x = (-1)*((int)vxp);
      viewPosition.y = (-1)*((int)vyp);
      viewPosition.z = (-1)*((int)vzp);
      //obj->lowestLv
      //printf("VP(%d,%d,%d)\n",viewPosition.x,viewPosition.y,viewPosition.z);
      ret = isIn3DBound(&startStairPoint,&stopStairPoint,&viewPosition);

      startStairPoint.y = obj->lowestLv;
      stopStairPoint.y = obj->lowestLv;
      ret = ret || isIn3DBound(&startStairPoint,&stopStairPoint,&viewPosition);
   } 
   return ret;

}

void printPoint(const struct Point P1,const char *str)
{
   printf("(%d,%d,%d)%s",P1.x,P1.y,P1.z,str);
}

void plotWolrd(GLubyte world[WORLDX][WORLDY][WORLDZ],const struct Point aPoint,const unsigned char value)
{
  if ((aPoint.x <WORLDX) && (aPoint.y <WORLDY) && (aPoint.z <WORLDZ) && (aPoint.x > -1) && (aPoint.y > -1) && (aPoint.z > -1))
  {
    world[aPoint.x][aPoint.y][aPoint.z] = value;
  }
}


int findDistanceBetweenPoint(const struct Point P1,const struct Point P2,const unsigned char type)
{
   int ret = 0;
   switch(type)
   {
      case X_DISTANCE:
         ret = 1 + findMaxValue(P1.x,P2.x) - findMinValue(P1.x,P2.x); 
      break;
      case Z_DISTANCE:
         ret = 1 + findMaxValue(P1.z,P2.z) - findMinValue(P1.z,P2.z); 
      break;
      case Y_DISTANCE:
         ret = 1 + findMaxValue(P1.y,P2.y) - findMinValue(P1.y,P2.y); 
      break;
   }
   return ret;
}
    

void listTheAreaProperforBuildDownStairInUnderground(struct Underground *obj,int listRoom[DEFAULT_NUM_ROOM],int *numEle,const int stairWidth,const int stairNum)
{
   const int stairMaxWidth = findMaxValue(stairWidth+2,stairNum+2);
   const int stairMinWidth = findMinValue(stairWidth+2,stairNum+2);
   int roomMaxWidth = 0;
   int roomMinWidth = 0;
   int index = 0;
   int indexList = 0;
   struct Point startP;
   struct Point stopP;
   struct Room *rooms = obj->m_rooms;
   for (index =0 ;index <DEFAULT_NUM_ROOM;index++)
   {
      findStartAndStopPointOfARoom(&(rooms[index]),&stopP,&startP);
      roomMaxWidth = findMaxValue(stopP.x-startP.x-1,stopP.z-startP.z-1);
      roomMinWidth = findMinValue(stopP.x-startP.x-1,stopP.z-startP.z-1);
      if ((roomMinWidth > stairMaxWidth) && (obj->m_upStairRoomId != index))
      {
     // printf("id:%d Room(%d,%d) , starir(%d,%d) upstairid:%d\n",index,roomMaxWidth,roomMinWidth,stairMaxWidth,stairMinWidth,obj->m_upStairRoomId);
        listRoom[indexList] = index;
        indexList++;
        *numEle = indexList;
      }
   }
}
int findProperPositionToPlaceDownStairInRoomOfUnderground(struct Underground *obj,struct Point *stairPoint,int *stairDirection,const int stairWidth,const int stairNum)
{
    const int stairMaxWidth = findMaxValue(stairWidth+2,stairNum+2);
    const int stairMinWidth = findMinValue(stairWidth+2,stairNum+2);
    int index = 0;
    int numProperSpaceRoom = 0;
    int listRoomOK[DEFAULT_NUM_ROOM];
    struct Point startP;
    struct Point stopP;
    struct Room *rooms = obj->m_rooms;
    listTheAreaProperforBuildDownStairInUnderground(obj,listRoomOK,&numProperSpaceRoom,stairWidth,stairNum);
    if (numProperSpaceRoom > 0)
    {
      obj->m_downStairRoomId = listRoomOK[getRandomNumber(0,numProperSpaceRoom-1)];
      /*
      for (index = 0;index < numProperSpaceRoom;index++)
        printf("RoomdID OK:%d \n",listRoomOK[index]); */
     
      *stairDirection = getRandomNumber(WEST,NORTH);

      findStartAndStopPointOfARoom(&(rooms[obj->m_downStairRoomId]),&stopP,&startP);
      if ((stopP.x-1-stairMaxWidth) > (startP.x+1))
      {
        stairPoint->x = getRandomNumber(startP.x+1,stopP.x-1-stairMaxWidth);
      }
      else
      {
        stairPoint->x = startP.x+1;
      }
      if ((stopP.z-1-stairMaxWidth) > (startP.z+1))
      {
        stairPoint->z = getRandomNumber(startP.z+1,stopP.z-1-stairMaxWidth);
      }
      else
      {
        stairPoint->z = startP.z+1;
      }

     // printf("choose : %d direct;%d RommStart:(%d,%d),stop:(%d,%d)  Stair:(%d,%d)\n",obj->m_downStairRoomId,*stairDirection,startP.x,startP.z,stopP.x,stopP.z,stairPoint->x,stairPoint->z);
      stairPoint->y = startP.y-stairMaxWidth;
      obj->m_downStairGroundLv = stairPoint->y;
    }

    return (numProperSpaceRoom==0); 
  
}
int findProperPositionToPlaceUpStairInRoomOfUnderground(struct Underground *obj,struct Point *stairPoint,int *stairDirection,const int stairWidth,const int stairNum, const int roomID)
{
   int i =0;
   int ret = 0;
   int directionId = 0;
   int canBuidInRoom = 0;
   int canBuildOutSide = 0;
   int widthX = 0;
   int widthZ = 0;
   int wallWithoutDoorNum = 0;
   int wallWithoutDoorId[3];
   int stairMaxWidth = findMaxValue(stairWidth+2,stairNum+2);
   int stairMinWidth = findMinValue(stairWidth+2,stairNum+2);
   struct Room *aRoom = &(obj->m_rooms[roomID]);
   struct Wall *walls = aRoom->Walls;
   struct Point startPointInTheRoom = walls[WEST].StartPoint;
   struct Point stopPointInTheRoom = walls[EAST].StartPoint;
   startPointInTheRoom.x += 1;
   startPointInTheRoom.z += 1;
   stopPointInTheRoom.x -= 1;
   stopPointInTheRoom.z += aRoom->Walls[EAST].width - 2;
   widthX = findDistanceBetweenPoint(startPointInTheRoom,stopPointInTheRoom,X_DISTANCE);
   widthZ = findDistanceBetweenPoint(startPointInTheRoom,stopPointInTheRoom,Z_DISTANCE);
   for (i = WEST ; i <= NORTH;i++)
      if(DoorDirections[roomID][i] == -1)
         wallWithoutDoorId[wallWithoutDoorNum++] = i;

   // Check size of area is suitable
   // 8 is spar space in the room
   canBuidInRoom = (stairMaxWidth <= (findMaxValue(widthX,widthZ) -8-4)) && (stairMinWidth <= (findMinValue(widthX,widthZ) -8-4));

   for (i = 0 ; i <wallWithoutDoorNum;i++)
   {  
      directionId = wallWithoutDoorId[i];
      if ((directionId == EAST) || (directionId == WEST))
      {
         canBuildOutSide = ((walls[directionId].width-(stairWidth+2+4)) > 2);
         canBuildOutSide = canBuildOutSide && ((dimensionOfGrid3x3[roomID][AREA_X_LENGHT]+ dimensionOfGrid3x3[roomID][AREA_XP] - walls[directionId].StartPoint.x-1) > (stairNum+2)); 
         if(directionId == WEST)
         {
            canBuildOutSide = canBuildOutSide && ((walls[directionId].StartPoint.x - dimensionOfGrid3x3[roomID][AREA_XP]) > (stairNum+2));
         }
         i = wallWithoutDoorNum;
      }
      else
      {
         canBuildOutSide = ((walls[directionId].width-(stairWidth+2+6)) > 2);
         canBuildOutSide = canBuildOutSide && ((dimensionOfGrid3x3[roomID][AREA_Z_LENGHT]+ dimensionOfGrid3x3[roomID][AREA_ZP] - walls[directionId].StartPoint.z-1) > (stairNum+2)); 
         if(directionId == SOUTH)
         {
            canBuildOutSide = canBuildOutSide && ((walls[directionId].StartPoint.z - dimensionOfGrid3x3[roomID][AREA_ZP]) > (stairNum+2));
         }
         i = wallWithoutDoorNum;
      }
   }
   ret = canBuidInRoom || canBuildOutSide;

   if (canBuildOutSide == 1)
   {

      stairPoint->y = startPointInTheRoom.y;
      if (directionId <= EAST)
      {
        // printf("o oooo : %d \n",walls[directionId].width-(stairWidth+2+4));
         stairPoint->x = walls[directionId].StartPoint.x + (-1)*(directionId== WEST)*(stairNum+1);
         stairPoint->z = walls[directionId].StartPoint.z +  getRandomNumber(2,walls[directionId].width-(stairWidth+2+4));
         *stairDirection = (1+directionId)%2;
         /*
         printf("WEST %d P: ",directionId);printPoint(walls[WEST].StartPoint,"\n");
         printf("Find OutEW  P: ");printPoint(*stairPoint,"\n");
         */
      }
      else
      {

       //  printf("2o oooo : %d \n",walls[directionId].width-(stairWidth+2+6));
         stairPoint->z = walls[directionId].StartPoint.z + (-1)*(directionId== SOUTH)*(stairNum+1);
         stairPoint->x = walls[directionId].StartPoint.x +  getRandomNumber(2,walls[directionId].width-(stairWidth+2+6));
         *stairDirection = 2+(1+directionId)%2;
         /*
         printf("Direct :%d\n",directionId);
         printf("South  P: ");printPoint(walls[SOUTH].StartPoint,"\n");
         printf("XXXXXXXXXFind OutSN  P:  ");printPoint(*stairPoint,"\n");
         */
      }
   }   // can make in side
   else if (canBuidInRoom == 1)
   {
      if (widthX > widthZ)
      {
         stairPoint->x = walls[WEST].StartPoint.x +  getRandomNumber(4,widthX -8 - stairMaxWidth);
         stairPoint->z = walls[SOUTH].StartPoint.z +  getRandomNumber(4,widthZ -8 - stairMinWidth);
         *stairDirection = getRandomNumber(WEST,EAST);
        // printf("Find insideEW  P: ");printPoint(*stairPoint,"\n");
      }
      else
      {
         stairPoint->x = walls[WEST].StartPoint.x +  getRandomNumber(4,widthX -8 - stairMinWidth);
         stairPoint->z = walls[SOUTH].StartPoint.z+  getRandomNumber(4,widthZ -8 - stairMaxWidth);
         *stairDirection = getRandomNumber(SOUTH,NORTH);


       //  printf("Find insideNS  P: ");printPoint(*stairPoint,"\n");
      }
   }
   stairPoint->y = startPointInTheRoom.y -1;
   return ret;

}

int isOnUpStair(struct Underground *obj)
{
   float vxp,vyp,vzp;
   struct Point viewPosition = {-1,-1,-1};
   int ret = 0;
   if (obj->m_state == GENERATED_UNDERGROUND_DONE)
   {      
      struct Point startStairPoint =  getReferentStairPoint(&(obj->m_upStair),START_POINT);
      struct Point stopStairPoint =  getReferentStairPoint(&(obj->m_upStair),STOP_POINT);
      stopStairPoint.y +=1;
      getAndConvertViewPos(&viewPosition);
      //obj->lowestLv
      //printf("VP(%d,%d,%d)\n",viewPosition.x,viewPosition.y,viewPosition.z);
      ret = isIn3DBound(&startStairPoint,&stopStairPoint,&viewPosition); 
   } 
   return ret;
}

void createRandomShapeOfObjectInDefinedArea(unsigned char definedArea[WORLDX][WORLDZ],const struct Point *PStart,const struct Point *PStop)
{
   int i,j,k;
   const int *MaxX;
   const int *MaxY;
   const int *MaxZ;
   const int *MinX;
   const int *MinY;
   const int *MinZ;
   int baseX  = 0;
   int baseZ  = 0;
   int baseY = 0;
   int maxLv = -1;
   int minLv = 100;
   SetMAXandMINPoint(&MaxX,&MinX,&(PStart->x),&(PStop->x));
   SetMAXandMINPoint(&MaxY,&MinY,&(PStart->y),&(PStop->y));
   SetMAXandMINPoint(&MaxZ,&MinZ,&(PStart->z),&(PStop->z));
   baseX = *MinX;
   baseZ = *MinZ;
   baseY = getRandomNumber(*MinY,*MaxY);
   if ((*MinX < WORLDX) && (*MinZ < WORLDZ))
   definedArea[*MinX][*MinZ] = baseY;
   maxLv =baseY;
   minLv = baseY;

   for(k=*MinZ;k<=*MaxZ;k++)
      for(i=*MinX;i<=*MaxX;i++)
      {
         // find max and min LV
         // WEST
         if ( (i-1 >= *MinX) && ((i-1) < WORLDX) && (k < WORLDZ))
         {
            maxLv = findMaxValue(maxLv,definedArea[i-1][k]);
            minLv = findMinValue(minLv,definedArea[i-1][k]);
         }
         // SOUTH
         if ((k -1 >= *MinZ) && ((k-1) < WORLDZ) && (i < WORLDX))
         {
            maxLv = findMaxValue(maxLv,definedArea[i][k-1]);
            minLv = findMinValue(minLv,definedArea[i][k-1]);
         }
         // SOUTH EAST
         if ((k -1 >= *MinZ) && ((k-1) < WORLDZ) && ((i+1)< WORLDX))
         {
            maxLv = findMaxValue(maxLv,definedArea[i+1][k-1]);
            minLv = findMinValue(minLv,definedArea[i+1][k-1]);
         }
         // SOUTH WEST
         if ((k -1 >= *MinZ) && (i-1 >= *MinX) && ((i-1) < WORLDX) && ((k -1) >= *MinZ) )
         {
            maxLv = findMaxValue(maxLv,definedArea[i-1][k-1]);
            minLv = findMinValue(minLv,definedArea[i-1][k-1]);
         }
         if ((maxLv <= 0) || (minLv <=0))
         {
            maxLv = baseY;
            minLv = baseY;
         }
         if ((i < WORLDX) && (k < WORLDZ))
         {           
            if (maxLv == minLv)
            {
               definedArea[i][k] = minLv + (1 - getRandomNumber(0,2));
            }
            else if (maxLv == minLv+1)
            {
               definedArea[i][k] = getRandomNumber(minLv,maxLv);
            }
            else if (maxLv == minLv+2)
            {
               definedArea[i][k] = minLv+1;
            }
            else
            {

            }
            
            //definedArea[i][k] = boundValue(*MaxY,*MinY,definedArea[i][k]);
         }

      }
}

void createCloudsInDefinedArea(struct OnGround *OutsideLand)
{
   int i,j;
   struct Point Apoint;
   struct Point Bpoint;
   unsigned char cloudTerrain[WORLDZ][WORLDX];
   memset(cloudTerrain,0,sizeof(cloudTerrain));
   //cloudSpace
   for (j = 0;j < WORLDZ;j+= DEFAULT_MAX_WIDHTZ_CLOUD_SIZE)
      for (i = 0;i < WORLDX;i+=DEFAULT_MAX_WIDHTX_CLOUD_SIZE)
      {

         // create or not by randomly
         if(getRandomNumber(0,100) > 96)
         {
               Apoint.x = boundValue(WORLDX-1,0,i + getRandomNumber(0,4));
               Bpoint.x = boundValue(WORLDX-1,0,i +DEFAULT_MAX_WIDHTX_CLOUD_SIZE - getRandomNumber(1,4));
               Apoint.z = boundValue(WORLDZ-1,0,j+ getRandomNumber(0,4));
               Bpoint.z = boundValue(WORLDZ-1,0,j +DEFAULT_MAX_WIDHTX_CLOUD_SIZE - getRandomNumber(1,4));
               Apoint.y = getRandomNumber(DEFAULT_LOWEST_CLOUD,DEFAULT_HIGHEST_CLOUD);
               Bpoint.y = getRandomNumber(DEFAULT_LOWEST_CLOUD,DEFAULT_HIGHEST_CLOUD);
               createRandomShapeOfObjectInDefinedArea(cloudTerrain,&Apoint,&Bpoint);
         }
      }
   for(i = 0; i < WORLDZ;i++)
      for(j=0; j <WORLDX;j++)
         if ((cloudTerrain[j][i] != 0) && (cloudTerrain[j][i] < DEFAULT_HIGHEST_CLOUD))
         {
          OutsideLand->cloudSpace[j][cloudTerrain[j][i]-DEFAULT_LOWEST_CLOUD][i] = 5;
         }

}
void moveCloudInOutsideLand(struct OnGround *obj)
{
         int i,k,j;
      static int moveId = 0;
      static clock_t cloudMovingRefTime = 0;
      static float timeUpdate = 0.02;
      static float currentCloudTime = 0;
      currentCloudTime = ((float)(clock()-cloudMovingRefTime))/CLOCKS_PER_SEC;
      if (currentCloudTime > timeUpdate)
      {

         cloudMovingRefTime = clock();
         for(i = 0;i<100;i++)
            for(j = 0;j<DEFAULT_CLOUD_LV_NUM;j++)
               for(k=0;k<100;k++)
                  world[i][DEFAULT_LOWEST_CLOUD+j][k] = obj->cloudSpace[(i+moveId)%100][j][(k+moveId)%100];
               moveId = (1+moveId)%100;
               

      }
}

void getAndConvertViewPos(struct Point *obj)
{
   float xvf,yvf,zvf;
   getViewPosition(&xvf,&yvf,&zvf);
   obj->x = (-1)*(int)(xvf);
   obj->y = (-1)*(int)(yvf);
   obj->z = (-1)*(int)(zvf);
}

void getStairOfOutSideWorld(struct OnGround *obj,struct Point *startP,struct Point *stopP)
{
   struct Point P1,P2;
   if (obj->state == TERRAIN_AND_STAIR_IS_BUILT)
   {
      P1 = getReferentStairPoint(&(obj->downStair),0); // 0 = start, 1 = stop
      P2 = getReferentStairPoint(&(obj->downStair),1); // 0 = start, 1 = stop
      stopP->x = findMaxValue(P1.x,P2.x);
      stopP->y = findMaxValue(P1.y,P2.y);
      stopP->z = findMaxValue(P1.z,P2.z);
      startP->x = findMinValue(P1.x,P2.x);
      startP->y = findMinValue(P1.y,P2.y);
      startP->z = findMinValue(P1.z,P2.z);
   }
   else
   {
      startP->x = 0;
      startP->y = 0;
      startP->z = 0;
      *stopP = *startP;
   }
}


void getStairOfUndergroundWolrd(struct Underground *obj,int upOrDownStair,struct Point *startP,struct Point *stopP) // 0 = up, 1 = down
{   
   if (obj->m_state == READY)
   {
      if(upOrDownStair == 0)// up stair
      {  
            *startP = getReferentStairPoint(&(obj->m_upStair),0); // 0 = start, 1 = stop
            *stopP = getReferentStairPoint(&(obj->m_upStair),1); // 0 = start, 1 = stop
      }
      else if (upOrDownStair == 1) // down stair
      {            
            *startP = getReferentStairPoint(&(obj->m_downStair),0); // 0 = start, 1 = stop
            *stopP = getReferentStairPoint(&(obj->m_downStair),1); // 0 = start, 1 = stop
      }
   }
   else
   {
      startP->x = 0;
      startP->y = 0;
      startP->z = 0;
      *stopP = *startP;
   }
}
//oooooo

void addWallToMap2D(struct Map *obj,const struct Wall *aWall,GLfloat color[4],const int roomID)
{
    struct Point2D point2Ds[2];
    struct LineOrBox2D aLine;
    convertWallPositionToPoint2Ds(aWall,&point2Ds[0],&point2Ds[1]);
    setPointsAndColorOfLineOrBox(&aLine,point2Ds,1,color);
    if(roomID == -1) // wall
    {
        obj->lines[obj->numLines] = mapTransformFuntion(&aLine);
        //printf("line start (%d,%d), stop(%d,%d) \n",obj->lines[obj->numLines].startP.x,obj->lines[obj->numLines].startP.z,obj->lines[obj->numLines].stopP.x,obj->lines[obj->numLines].stopP.z);
        obj->numLines++;  
    }
    else
    {
        obj->doorsPos[roomID][obj->numDoors[roomID]] = mapTransformFuntion(&aLine);
        //printf("line start (%d,%d), stop(%d,%d) \n",obj->lines[obj->numLines].startP.x,obj->lines[obj->numLines].startP.z,obj->lines[obj->numLines].stopP.x,obj->lines[obj->numLines].stopP.z);
        obj->numDoors[roomID]++;  
    }

}
void convertWallPositionToPoint2Ds(const struct Wall *obj,struct Point2D *P1,struct Point2D *P2)
{
  *P1 = convert3DPointTo2DPoint(&(obj->StartPoint));
  *P2 = *P1;
  if (obj->XorZSide == Z_SIDE_WALL)
  {
    P2->z += obj->width - 1;
  }
  else if(obj->XorZSide == X_SIDE_WALL)
  {
    P2->x += obj->width - 1;
  }
}
void findStartAndStopPointOfARoom(struct Room *obj,struct Point *maxPoint,struct Point *minPoint)
{
   int index = 0;
   maxPoint->x = -1;
   maxPoint->y = -1;
   maxPoint->z = -1;
   minPoint->x = WORLDX;
   minPoint->y = WORLDY;
   minPoint->z = WORLDZ;
   for (index = 0 ;index < 4;index++)
   {
      maxPoint->x = findMaxValue(obj->Walls[index].StartPoint.x,maxPoint->x);
      maxPoint->y = findMaxValue(obj->Walls[index].StartPoint.y,maxPoint->y);
      maxPoint->z = findMaxValue(obj->Walls[index].StartPoint.z,maxPoint->z);
      minPoint->x = findMinValue(obj->Walls[index].StartPoint.x,minPoint->x);
      minPoint->y = findMinValue(obj->Walls[index].StartPoint.y,minPoint->y);
      minPoint->z = findMinValue(obj->Walls[index].StartPoint.z,minPoint->z);
   }
}

void findStartAndStopPointOfARoom2D(struct Room *obj,struct Point2D *maxPoint,struct Point2D *minPoint)
{
  struct Point  roomP[2];
  findStartAndStopPointOfARoom(obj,&roomP[0],&roomP[1]);
  *maxPoint = convert3DPointTo2DPoint(&roomP[0]);
  *minPoint = convert3DPointTo2DPoint(&roomP[1]);
}
int findViewPointIsWhichRoom2D(struct Map *obj)
{
  struct Point2D vPoint2D;
  struct Point vPoint;
  int indexRoom=0;
  int indexRet = -1;
  struct LineOrBox2D *roomPos = obj->roomsPos;
  getAndConvertViewPos(&vPoint);
  vPoint2D = convert3DPointTo2DPoint(&vPoint);
  vPoint2D = point2DTransformFuntionForMap(&vPoint2D);
  for(indexRoom = 0; indexRoom<DEFAULT_NUM_ROOM;indexRoom++)
  {
      if((roomPos[indexRoom].startP.x <= vPoint2D.x) && (roomPos[indexRoom].startP.z <= vPoint2D.z) && (roomPos[indexRoom].stopP.x >= vPoint2D.x) && (roomPos[indexRoom].stopP.z >= vPoint2D.z))
      {
        indexRet = indexRoom;
        indexRoom = DEFAULT_NUM_ROOM;
      }

  //printf("vPoint2D: (%d,%d) Room:",vPoint2D.x,vPoint2D.z);printLineOrBoxObj(&roomPos[indexRoom]);
  }
  return indexRet;
}

void drawARoomInMap2D(struct Map *obj,const int roomID)
{
  int i = 0;
  if((roomID > -1) && (roomID < DEFAULT_NUM_ROOM))
  {
    for(i=0;i<obj->numDoors[roomID];i++)
    {
      drawBoxMap2D(&(obj->doorsPos[roomID][i]));
    }
    drawBoxMap2D(&(obj->roomsPos[roomID]));
    for(i=0;i<obj->numCubes[roomID];i++)
    {
      drawBoxMap2D(&(obj->cubesPos[roomID][i])); 
    } 
  }
}
void updateUndergroundMap2D(struct Underground *obj,const int displayMode)  // 1 = no map, 2 = fog, 0 = normal map
{
   int indexRoom = 0;
   int i = 0;
   struct Map *a2DMapP = &(obj->m_a2DMap);
   struct Point vPoint;
   if (obj->m_state == GENERATED_UNDERGROUND_DONE)
   {

      // drae up stair position in a map
      // Always shows the up-stair
      if ((obj->m_stairOption == ONLY_UP_STAIR) || (obj->m_stairOption == BOTH_UP_DOWN_STAIR))
      {
        drawBoxMap2D(&(a2DMapP->stairsPos[0]));
      }
      indexRoom =  findViewPointIsWhichRoom2D(a2DMapP);
      if (displayMode == FOG_MAP)
      { 
        drawARoomInMap2D(a2DMapP,indexRoom);
        drawFogInMap(&(a2DMapP->aFogMap));
      }
      getAndConvertViewPos(&vPoint);
      clearFogPosition(&(a2DMapP->aFogMap),vPoint.x,vPoint.z);
      clearFogInArea(&(a2DMapP->aFogMap),&(a2DMapP->roomsPos[indexRoom]));


      // doors and hallways
      for(i = 0;i<a2DMapP->numLines;i++ )
      {
        drawLineMap2D(&(a2DMapP->lines[i]));
      }
      //down room
      for(indexRoom = 0;indexRoom < DEFAULT_NUM_ROOM;indexRoom++)
      {
        drawARoomInMap2D(a2DMapP,indexRoom);
      }
      // drae down stair position in a map
      if (((obj->m_stairOption == ONLY_DOWN_STAIR) || (obj->m_stairOption == BOTH_UP_DOWN_STAIR)) && (a2DMapP->numStairs == 2))
      {
        drawBoxMap2D(&(a2DMapP->stairsPos[1]));
      } 

   }
}
void drawFogInMap(struct FogMap *obj)
{
  int i,j;
  for(i=0;i<NUM_TILE_X;i++)
    for(j=0;j<NUM_TILE_Z;j++)
    {
        if (visibilityOfLineOrBox(&(obj->fogTiles[i][j])) == 1)
        {
         // printf("draw i:%d,j:%d\n",i,j);
          drawBoxMap2D(&obj->fogTiles[i][j]);
        }
    }

}
void clearFogPosition(struct FogMap *obj,const int xView,const int zView)
{
  int i = (NUM_TILE_X*xView)/WORLDX;
  int j = (NUM_TILE_Z*zView)/WORLDZ;

  clearFogPositionByIndices(obj,i,j);
  clearFogPositionByIndices(obj,i-1,j-1);
  clearFogPositionByIndices(obj,i-1,j);
  clearFogPositionByIndices(obj,i,j-1);

  clearFogPositionByIndices(obj,i-1,j+1);
  clearFogPositionByIndices(obj,i+1,j-1);


  clearFogPositionByIndices(obj,i+1,j+1);
  clearFogPositionByIndices(obj,i+1,j);
  clearFogPositionByIndices(obj,i,j+1);

}
void clearFogInArea(struct FogMap *obj,struct LineOrBox2D *area)
{
  static struct LineOrBox2D oldDataOfArea;
  struct LineOrBox2D aArea;
  int i,j;
  if ( ((area->startP.x ==oldDataOfArea.startP.x)&&(area->startP.z ==oldDataOfArea.startP.z) && (area->stopP.x ==oldDataOfArea.stopP.x)&&(area->stopP.z ==oldDataOfArea.stopP.z)) == 0)
  {
    // revert 
    aArea = mapRetransformFuntion(area);
    for(i=aArea.startP.x;i<=aArea.stopP.x;i++)
    {
      for(j=aArea.startP.z;j<=aArea.stopP.z;j++)
        clearFogPositionByIndices(obj,(NUM_TILE_X*i)/WORLDX,(NUM_TILE_Z*j)/WORLDZ); 
    }
    //aArea 
    oldDataOfArea = *area;
  }

}

void clearFogPositionByIndices(struct FogMap *obj,const int i,const int j)
{
    if((i < NUM_TILE_X) && (j <NUM_TILE_Z) && (i > -1) && (j > -1))
  {
    if (visibilityOfLineOrBox(&(obj->fogTiles[i][j])) == 1)
    {
      obj->fogTiles[i][j].colorAttrib[3] = 0.0;
    }
  }
}
void setFogAreaInFogMap(struct FogMap *obj,GLfloat fogColor[4],struct LineOrBox2D (*transformationFn)(struct LineOrBox2D*))
{
  int i,j;
  int xVal = 0;
  int zVal = 0;
  int xWidth = 0;
  int zWidth = 0;
  struct Point2D point2Ds[2];
  int worldXScale = WORLDX*DEFAULT_MAP_2D_SCALE_VAL;
  int worldZScale = WORLDZ*DEFAULT_MAP_2D_SCALE_VAL;
  if((NUM_TILE_X > 0) && (NUM_TILE_Z > 0))
  {
    xWidth = worldXScale/NUM_TILE_X;
    zWidth = worldZScale/NUM_TILE_Z;  
  }
  for(i=0;i<NUM_TILE_X;i++)
  {
    zVal = 0;
    for(j=0;j<NUM_TILE_Z;j++)
    {
      point2Ds[0] = (struct Point2D){xVal,worldZScale-DEFAULT_MAP_2D_SCALE_VAL-zVal};
      point2Ds[1] = (struct Point2D){xVal+xWidth,(worldZScale-DEFAULT_MAP_2D_SCALE_VAL-(zVal+zWidth))};
      setPointsAndColorOfLineOrBox(&(obj->fogTiles[i][j]),point2Ds,0,fogColor);
     // obj->fogTiles[i][j] = transformationFn(&(obj->fogTiles[i][j]));
      zVal += zWidth;
    }
    xVal += xWidth;
  }
 // printf("xval:%d \n",xVal);
}

void setPointsAndColorOfLineOrBox(struct LineOrBox2D *obj,struct Point2D startAndStopPoint[2],const int width,const GLfloat colorAttrib[4])
{
  obj->startP.x = findMinValue(startAndStopPoint[0].x,startAndStopPoint[1].x);
  obj->startP.z = findMinValue(startAndStopPoint[0].z,startAndStopPoint[1].z);
  obj->stopP.x = findMaxValue(startAndStopPoint[0].x,startAndStopPoint[1].x);
  obj->stopP.z = findMaxValue(startAndStopPoint[0].z,startAndStopPoint[1].z);
  obj->width = width;
  memcpy(obj->colorAttrib,colorAttrib,sizeof(GLfloat)*4);
}

int visibilityOfLineOrBox(struct LineOrBox2D *obj)
{
  return (obj->colorAttrib[3] != 0.0);
}

void drawLineMap2D(struct LineOrBox2D *obj)
{  
  int i = 0;
  int isSetColor = 0;
  static GLfloat oldColor[] = {0.0,0.0,0.0,0.0};
  for (i = 0;i < 4;i++)
    isSetColor = (isSetColor ==1) && (oldColor[i] == obj->colorAttrib[i]);
  if (isSetColor == 0)
  {
    set2Dcolour(obj->colorAttrib);
    for (i = 0;i < 4;i++)
      oldColor[i] = obj->colorAttrib[i];
  }
  draw2Dline(obj->startP.x,obj->startP.z,obj->stopP.x,obj->stopP.z, obj->width);
}

void drawLineMap2DWithTransFn(struct LineOrBox2D *obj,struct LineOrBox2D (*transformationFn)(struct LineOrBox2D*))
{
  struct LineOrBox2D aLine = transformationFn(obj);
  drawLineMap2D(&aLine);
}

void drawBoxMap2D(struct LineOrBox2D *obj)
{
  int i = 0;
  int isSetColor = 0;
  static GLfloat oldColor[] = {0.0,0.0,0.0,0.0};
  for (i = 0;i < 4;i++)
    isSetColor = (isSetColor ==1) && (oldColor[i] == obj->colorAttrib[i]);
  if (isSetColor == 0)
  {
    set2Dcolour(obj->colorAttrib);
    for (i = 0;i < 4;i++)
      oldColor[i] = obj->colorAttrib[i];
  }
  if (obj->width == 0)
  {
    draw2Dbox(obj->startP.x,obj->startP.z,obj->stopP.x,obj->stopP.z); 
  }
  else
  {
    draw2Dline(obj->startP.x,obj->startP.z,obj->startP.x,obj->stopP.z, obj->width);
    draw2Dline(obj->startP.x,obj->stopP.z,obj->stopP.x,obj->stopP.z, obj->width);
    draw2Dline(obj->stopP.x,obj->stopP.z,obj->stopP.x,obj->startP.z, obj->width);
    draw2Dline(obj->stopP.x,obj->startP.z,obj->startP.x,obj->startP.z, obj->width);
  }
}

void drawBoxMap2DWithTransFn(struct LineOrBox2D *obj,struct LineOrBox2D (*transformationFn)(struct LineOrBox2D*))
{
  struct LineOrBox2D aLine = transformationFn(obj);
  drawBoxMap2D(&aLine);
}

// map resolution 500x500 and scale: 5.0 , Invert Z-axis
struct LineOrBox2D mapTransformFuntion(struct LineOrBox2D *obj)
{
  const float scaleVal = DEFAULT_MAP_2D_SCALE_VAL;
  struct LineOrBox2D aLine;
  struct Point2D point2Ds[2];
  memcpy(aLine.colorAttrib,obj->colorAttrib,sizeof(GLfloat)*4);
  point2Ds[0] = point2DTransformFuntionForMap(&(obj->startP));
  point2Ds[1] = point2DTransformFuntionForMap(&(obj->stopP));

  aLine.startP.x = findMinValue(point2Ds[0].x,point2Ds[1].x);
  aLine.startP.z = findMinValue(point2Ds[0].z,point2Ds[1].z);
  aLine.stopP.x = findMaxValue(point2Ds[0].x,point2Ds[1].x);
  aLine.stopP.z = findMaxValue(point2Ds[0].z,point2Ds[1].z);

  aLine.width = (int)((float)obj->width*DEFAULT_MAP_2D_SCALE_VAL);
  //printf("transform (%d,%d)(%d,%d) --> (%d,%d)(%d,%d)\n",obj->startP.x,obj->startP.z,obj->stopP.x,obj->stopP.x,aLine.startP.x,aLine.startP.z,aLine.stopP.x,aLine.stopP.z);
  return aLine;
}

struct LineOrBox2D mapRetransformFuntion(struct LineOrBox2D *obj)
{
    const float scaleVal = DEFAULT_MAP_2D_SCALE_VAL;
  struct LineOrBox2D aLine = *obj;
  struct Point2D point2Ds[2];
  if(scaleVal != 0.0)
  {
    point2Ds[0] = point2DRetransformFuntionForMap(&(obj->startP));
    point2Ds[1] = point2DRetransformFuntionForMap(&(obj->stopP));
    aLine.startP.x = findMinValue(point2Ds[0].x,point2Ds[1].x);
    aLine.startP.z = findMinValue(point2Ds[0].z,point2Ds[1].z);
    aLine.stopP.x = findMaxValue(point2Ds[0].x,point2Ds[1].x);
    aLine.stopP.z = findMaxValue(point2Ds[0].z,point2Ds[1].z);
    aLine.width = (int)((float)obj->width/scaleVal);
  }
  //printf("transform (%d,%d)(%d,%d) --> (%d,%d)(%d,%d)\n",obj->startP.x,obj->startP.z,obj->stopP.x,obj->stopP.x,aLine.startP.x,aLine.startP.z,aLine.stopP.x,aLine.stopP.z);
  return aLine;
}

struct Point2D convert3DPointTo2DPoint(const struct Point *obj)
{
  struct  Point2D aPoint;
  aPoint.x = obj->x;
  aPoint.z = obj->z;
  return aPoint;
}

struct Point2D point2DTransformFuntionForMap(struct Point2D *obj)
{
    const float scaleVal = DEFAULT_MAP_2D_SCALE_VAL;
    struct Point2D aPoint;
    aPoint.x = (int)(scaleVal*(float)obj->x);
    aPoint.z = (int)(scaleVal*((float)(WORLDZ-1) - (float)(obj->z)) + 0.5);
    return aPoint;
}

struct Point2D point2DRetransformFuntionForMap(struct Point2D *obj)
{

    const float scaleVal = DEFAULT_MAP_2D_SCALE_VAL;
    struct Point2D aPoint = *obj;
    if (scaleVal != 0.0)
    {
      aPoint.x = (int)((float)obj->x/scaleVal);
      aPoint.z = (int)(((float)(WORLDZ-1)*scaleVal - (float)(obj->z))/scaleVal + 0.5);
    }
    return aPoint;
}
void getViewPointInMap(struct Point2D *obj)
{
  struct Point aPoint;
  getAndConvertViewPos(&aPoint);
  *obj = convert3DPointTo2DPoint(&aPoint); 
}
//
//void drawBoxMap2D(struct LineOrBox2D *obj,struct LineOrBox2D (*transformationFn)(struct LineOrBox2D*))
void drawViewPoint(GLfloat color[4],struct LineOrBox2D (*transformationFn)(struct LineOrBox2D*))
{
  struct Point2D aPoint2D[2];
  struct LineOrBox2D aBox;
  getViewPointInMap(&(aPoint2D[0]));
  aPoint2D[1]= aPoint2D[0];
  aPoint2D[0].x = boundValue(WORLDX-1,0,aPoint2D[0].x - 1);
  aPoint2D[0].z = boundValue(WORLDZ-1,0,aPoint2D[0].z - 1);
  aPoint2D[1].x = boundValue(WORLDX-1,0,aPoint2D[1].x + 1);
  aPoint2D[1].z = boundValue(WORLDZ-1,0,aPoint2D[1].z + 1);
  setPointsAndColorOfLineOrBox(&aBox,aPoint2D,0,color);
  drawBoxMap2DWithTransFn(&aBox,transformationFn); 
}


void drawAStairInMap(struct Point *startP,struct Point *stopt,GLfloat color[4],struct LineOrBox2D (*transformationFn)(struct LineOrBox2D*))
{
  struct Point2D aPoint2D[2];
  struct LineOrBox2D aBox;
  aPoint2D[0] = convert3DPointTo2DPoint(startP);
  aPoint2D[1] = convert3DPointTo2DPoint(stopt);
  setPointsAndColorOfLineOrBox(&aBox,aPoint2D,0,color);
  drawBoxMap2DWithTransFn(&aBox,transformationFn); 
}

void setCubePositionInMap2D(struct Underground *obj)
{
   struct Map *a2DMapP = &(obj->m_a2DMap);

   int indexRoom = 0;
   int i = 0;
   GLfloat yellow[] = {1.0, 1.0, 0.0, alphaVal};
   if (obj->m_state == READY)
   {
      for(indexRoom = 0;indexRoom < DEFAULT_NUM_ROOM;indexRoom++)
      {
        a2DMapP->numCubes[indexRoom] = 0;
        for(i = 0; i < obj->m_rooms[indexRoom].numUnitCubes;i++)
        {   
            struct Point2D point2Ds[2];
            struct LineOrBox2D aLine;
            point2Ds[0] = convert3DPointTo2DPoint(&(obj->m_rooms[indexRoom].unitCubePoint[i]));
            point2Ds[1].x = point2Ds[0].x+1;
            point2Ds[1].z = point2Ds[0].z+1;
            setPointsAndColorOfLineOrBox(&aLine,point2Ds,0,yellow);
            a2DMapP->cubesPos[indexRoom][i] = mapTransformFuntion(&aLine);
            a2DMapP->numCubes[indexRoom]++;
            //printf("numCube %d,%d\n", obj->m_rooms[indexRoom].numUnitCubes,a2DMapP->numCubes[indexRoom]); 
        }
      }
   }
}
void setRoomsPositionInMap2D(struct Underground *obj)
{
   struct Map *a2DMapP = &(obj->m_a2DMap);
   struct Point2D point2Ds[2];
   struct LineOrBox2D aLine;
   int indexRoom = 0;
   GLfloat brown[] = {0.8, 0.45, 0.1, alphaVal};
   if ((obj->m_state == READY) || (obj->m_state == GENERATED_UNDERGROUND_DONE))
   {
      //find position of room
      for(indexRoom = 0;indexRoom < DEFAULT_NUM_ROOM;indexRoom++)
      {
         findStartAndStopPointOfARoom2D(&(obj->m_rooms[indexRoom]),&point2Ds[0],&point2Ds[1]);
         setPointsAndColorOfLineOrBox(&aLine,point2Ds,1,brown);
         a2DMapP->roomsPos[indexRoom] = mapTransformFuntion(&aLine);
         //printLineOrBoxObj(&(a2DMapP->roomsPos[indexRoom]));
      }
   }
}
void setStairPositionInMap2D(struct Underground *obj)
{
    struct Point stairP1;
    struct Point stairP2;
    struct Point2D aPoint2D[2];
    struct Map *a2DMapP = &(obj->m_a2DMap);
    int indexRoom = 0;
    GLfloat white[] = {1.0, 1.0, 1.0, alphaVal};
    GLfloat grey[] = {0.4, 0.4, 0.4, alphaVal};
   if (obj->m_state == READY)
   {
      // drae up stair position in a map
      if ((obj->m_stairOption == ONLY_UP_STAIR) || (obj->m_stairOption == BOTH_UP_DOWN_STAIR))
      {
        getStairOfUndergroundWolrd(obj,0,&stairP1,&stairP2); // 0 = up, 1 = down
        aPoint2D[0] = convert3DPointTo2DPoint(&stairP1);
        aPoint2D[1] = convert3DPointTo2DPoint(&stairP2);
        setPointsAndColorOfLineOrBox(&(a2DMapP->stairsPos[a2DMapP->numStairs]),aPoint2D,0,white);
        a2DMapP->stairsPos[a2DMapP->numStairs] = mapTransformFuntion(&(a2DMapP->stairsPos[a2DMapP->numStairs]));
        a2DMapP->numStairs++;
      }
      // drae down stair position in a map
       if ((obj->m_stairOption == ONLY_DOWN_STAIR) || (obj->m_stairOption == BOTH_UP_DOWN_STAIR))
      {
        getStairOfUndergroundWolrd(obj,1,&stairP1,&stairP2); // 0 = up, 1 = down
        aPoint2D[0] = convert3DPointTo2DPoint(&stairP1);
        aPoint2D[1] = convert3DPointTo2DPoint(&stairP2);
        setPointsAndColorOfLineOrBox(&(a2DMapP->stairsPos[a2DMapP->numStairs]),aPoint2D,0,grey);
        a2DMapP->stairsPos[a2DMapP->numStairs] = mapTransformFuntion(&(a2DMapP->stairsPos[a2DMapP->numStairs]));
        a2DMapP->numStairs++;
      }
    }
}


void initialMap2D(struct Map *obj)
{
  obj->numLines = 0;
  obj->numStairs = 0;
  memset(obj->numCubes,0,sizeof(obj->numCubes));
  memset(obj->numDoors,0,sizeof(obj->numDoors));
  GLfloat black[] = {0.0, 0.0, 0.0, alphaVal};
  setFogAreaInFogMap(&(obj->aFogMap),black,&mapTransformFuntion);
}

void set3DLineOrBoxToMap2D(struct Map *obj,GLfloat color[4],int width,struct Point *startP,struct Point *stopP)
{
  struct LineOrBox2D aLine;
  struct Point2D P2D[2];
  P2D[0] = convert3DPointTo2DPoint(startP);
  P2D[1] = convert3DPointTo2DPoint(stopP);
  setPointsAndColorOfLineOrBox(&aLine,P2D,width,color);
  obj->lines[obj->numLines] = mapTransformFuntion(&aLine);
  obj->numLines++;
}

void printLineOrBoxObj(struct LineOrBox2D *obj)
{
    printf("LineOrBox (%d,%d), (%d,%d) w:%d\n",obj->startP.x,obj->startP.z,obj->stopP.x,obj->stopP.z,obj->width);
}


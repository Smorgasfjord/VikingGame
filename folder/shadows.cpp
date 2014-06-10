// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <iostream>
#include "math.h"
#include <time.h>

// Include GLEW
#include </home/nclarke/Desktop/deps/glew/include/GL/glew.h>

// Include GLFW
#include </home/nclarke/Desktop/deps/glfw/include/GLFW/glfw3.h>
GLFWwindow* window;

#define CLOUDS 8


// Include GLM
#include </home/nclarke/Desktop/deps/glm/include/glm/glm.hpp>
#include </home/nclarke/Desktop/deps/glm/include/glm/gtc/matrix_transform.hpp>
#include </home/nclarke/Desktop/deps/glm/include/glm/gtc/type_ptr.hpp> //value_ptr
#include </home/nclarke/Desktop/deps/glm/include/glm/gtc/matrix_transform.hpp>
#include </home/nclarke/Desktop/deps/glm/include/glm/gtx/transform.hpp>
using namespace glm;

#include </home/nclarke/Desktop/Particles/particles.hpp>
#include </home/nclarke/Desktop/Graphics/common/shader.hpp>
#include </home/nclarke/Desktop/Graphics/common/texture.hpp>
#include </home/nclarke/Desktop/Graphics/common/controls.hpp>
#include </home/nclarke/Desktop/Graphics/common/objloader.hpp>
#include </home/nclarke/Desktop/Graphics/common/vboindexer.hpp>


//Components
/*
#include "Components/GameObject.h"
#include "Components/Platform.h"
#include "Components/Mountain.h"
#include "Components/Hammer.h"
#include "Components/Bjorn.h"
#include "Utils/World.h"
*/

typedef struct Image {
  unsigned long sizeX;
  unsigned long sizeY;
  char *data;
} Image;

Image *TextureImage;

typedef struct RGB {
  GLubyte r;
  GLubyte g; 
  GLubyte b;
} RGB;



RGB myimage[64][64];
RGB* g_pixel;
int ImageLoad(char *filename, Image *image);
GLvoid LoadTexture(char* image_file, int tex_id);

/* BMP file loader loads a 24-bit bmp file only */

/*
* getint and getshort are help functions to load the bitmap byte by byte
*/
static unsigned int getint(FILE *fp) {
  int c, c1, c2, c3;
  
  /*  get 4 bytes */ 
  c = getc(fp);  
  c1 = getc(fp);  
  c2 = getc(fp);  
  c3 = getc(fp);
  
  return ((unsigned int) c) +   
    (((unsigned int) c1) << 8) + 
    (((unsigned int) c2) << 16) +
    (((unsigned int) c3) << 24);
}

static unsigned int getshort(FILE *fp){
  int c, c1;
  
  /* get 2 bytes*/
  c = getc(fp);  
  c1 = getc(fp);
  
  return ((unsigned int) c) + (((unsigned int) c1) << 8);
}


glm::vec3 eye = glm::vec3(0, 0, 15);
glm::vec3 lightInvDir = glm::vec3(0.5f,2,2);

//GLuint MatrixID, ViewMatrixID, ModelMatrixID, DepthBiasID, ShadowMapID;

GLuint vertexbuffer;
GLuint uvbuffer;
GLuint elementbuffer;
GLuint normalbuffer;
GLuint depthTexture;
GLuint FramebufferName;

std::vector<glm::vec3> vertices;
std::vector<glm::vec2> uvs;
std::vector<glm::vec3> normals;

std::vector<unsigned short> indices;
std::vector<glm::vec3> indexed_vertices;
std::vector<glm::vec2> indexed_uvs;
std::vector<glm::vec3> indexed_normals;

//GLuint vertexbuffer2;
GLuint uvbuffer2;
//GLuint elementbuffer2;
GLuint normalbuffer2;
GLuint FramebufferName2;

std::vector<glm::vec3> vertices2;
std::vector<glm::vec2> uvs2;
std::vector<glm::vec3> normals2;

std::vector<unsigned short> indices2;
std::vector<glm::vec3> indexed_vertices2;
std::vector<glm::vec2> indexed_uvs2;
std::vector<glm::vec3> indexed_normals2;
bool twin_lights = false, simple_test = true;

glm::mat4 biasMatrix(
	0.5, 0.0, 0.0, 0.0, 
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 0.5, 0.0,
	0.5, 0.5, 0.5, 1.0
);


glm::vec3 aorBox = glm::vec3(1.0);
glm::mat4 rotateBox;
glm::mat4 scaleBox;
glm::mat4 transBox;
glm::mat4 MVP;
float boxAngle = 0;
float bscale = 1;
/*
GLint h_aPosition;
GLint h_uColor;
GLint h_uModelMatrix;
GLint h_uViewMatrix;
GLint h_uProjMatrix;
*/

GLuint CubeBuffObj, CIndxBuffObj, TexBuffObj;
int g_CiboLen;
bool shadow_lights = false;


void keyCalling()
{

        if (glfwGetKey( window, GLFW_KEY_Z ) == GLFW_PRESS)
            lightInvDir = glm::vec3(lightInvDir.x, lightInvDir.y, lightInvDir.z+.1);

        if (glfwGetKey( window, GLFW_KEY_X ) == GLFW_PRESS)
            lightInvDir = glm::vec3(lightInvDir.x, lightInvDir.y, lightInvDir.z-.1);

        if (glfwGetKey( window, GLFW_KEY_W ) == GLFW_PRESS)
            lightInvDir = glm::vec3(lightInvDir.x, lightInvDir.y+.1, lightInvDir.z);

        if (glfwGetKey( window, GLFW_KEY_S ) == GLFW_PRESS)
            lightInvDir = glm::vec3(lightInvDir.x, lightInvDir.y-.1, lightInvDir.z);

        if (glfwGetKey( window, GLFW_KEY_A ) == GLFW_PRESS)
            lightInvDir = glm::vec3(lightInvDir.x+.1, lightInvDir.y, lightInvDir.z);

        if (glfwGetKey( window, GLFW_KEY_D ) == GLFW_PRESS)
            lightInvDir = glm::vec3(lightInvDir.x-.1, lightInvDir.y, lightInvDir.z);

        if (glfwGetKey( window, GLFW_KEY_L ) == GLFW_PRESS)
            twin_lights = !twin_lights;

        if (glfwGetKey( window, GLFW_KEY_U ) == GLFW_PRESS)
            shadow_lights = !shadow_lights;

        if (glfwGetKey( window, GLFW_KEY_P ) == GLFW_PRESS)
         {            
            simple_test = !simple_test;
            printf("simple is on? (%s)\n", simple_test ? "yes" : "no");
         }

        if (glfwGetKey( window, GLFW_KEY_R ) == GLFW_PRESS)
         {
            glfwSetTime(0);
//            printf("timer is reset\n");
            initDustParticles();
         }

        if (glfwGetKey( window, GLFW_KEY_T ) == GLFW_PRESS)
         {
            glfwSetTime(0);
            initSnowParticles();
         }

        if (glfwGetMouseButton( window, GLFW_MOUSE_BUTTON_LEFT ) == GLFW_PRESS)
         {
            double xPos, yPos;
            glfwGetCursorPos( window, &xPos, &yPos);

            glfwSetTime(0);
//            printf("left button pressed at (%lf, %lf).\n timer is reset\n", xPos, yPos);
            initDustParticles();
         }

}



static void initCube() {


   printf("starting initing cube ... ");

  float CubePos[] = {
    -1.5, -1.5, -1.5, //back face 5 verts :0 
    -1.5, 1.5, -1.5,
    1.5, 1.5, -1.5,
    1.5, -1.5, -1.5,
    1.5, -1.5, 1.5, //right face 5 verts :4
    1.5, 1.5, 1.5,
    1.5, 1.5, -1.5,
    1.5, -1.5, -1.5,
    -1.5, -1.5, 1.5, //front face 4 verts :8
    -1.5, 1.5, 1.5,
    1.5, 1.5, 1.5,
    1.5, -1.5, 1.5,
    -1.5, -1.5, -1.5, //left face 4 verts :12
    -1.5, 1.5, -1.5,
    -1.5, 1.5, 1.5,
    -1.5, -1.5, 1.5
  };

   static GLfloat CubeTex[] = {
      .25, 0, // back 
      .25, 1,
      0, 1,
      0, 0,

      .75, 0, // left 
      .75, 1,
      1, 1,
      1, 0,


      .5, 0, //front 
      .5, 1,
      .75, 1,
      .75, 0,

      .25, 0, //right 
      .25, 1,
      .5, 1,
      .5, 0
    }; 
   
    unsigned short idx[] = {0, 1, 2,  2, 3, 0,  4, 5, 6, 
     6, 7, 4,  8, 9, 10, 10, 11, 8,  12, 13, 14, 14, 15, 12, 
     9, 10, 1};

    g_CiboLen = 27;
    glGenBuffers(1, &CubeBuffObj); //vertices
    glBindBuffer(GL_ARRAY_BUFFER, CubeBuffObj);
    glBufferData(GL_ARRAY_BUFFER, sizeof(CubePos), CubePos, GL_STATIC_DRAW);

    glGenBuffers(1, &CIndxBuffObj); //indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, CIndxBuffObj);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);

    glGenBuffers(1, &TexBuffObj);
    glBindBuffer(GL_ARRAY_BUFFER, TexBuffObj);
    glBufferData(GL_ARRAY_BUFFER, sizeof(CubeTex), CubeTex, GL_STATIC_DRAW);

   printf("finished initing cube\n");

}

GLuint cubeShaderID, h_aPosition, h_uTexUnit, h_uProjMatrix, h_uViewMatrix, h_uModelMatrix;
GLuint TextureID, MatrixID, ViewMatrixID, ModelMatrixID;
GLuint DepthBiasID, ShadowMapID, h_simple, h_aTexCoord,  h_dust;
GLuint Texture, depthProgramID, depthMatrixID;
bool res;
GLuint VertexArrayID;




GLuint lightInvDirID;


glm::mat4 renderToFrame(GLuint vertBuff, GLuint indexBuff, unsigned int indicesSize, glm::mat4 dModelMatrix)
{

   glm::mat4 dProjectionMatrix;
   glm::mat4 dViewMatrix;


		// Compute the MVP matrix from the light's point of view
		dProjectionMatrix = glm::ortho<float>(-10,10,-10,10,-10,20);
		dViewMatrix = glm::lookAt(lightInvDir, glm::vec3(0,0,0), glm::vec3(0,1,0));

		glm::mat4 depthMVPtemp = dProjectionMatrix * dViewMatrix * dModelMatrix;

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(depthMatrixID, 1, GL_FALSE, &depthMVPtemp[0][0]);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertBuff);
		glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,(void*)0);

		// Index buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuff);

		// Draw the shadow onto the frame 
		glDrawElements(GL_TRIANGLES,indicesSize,GL_UNSIGNED_SHORT,(void*)0);

		glDisableVertexAttribArray(0);

      return depthMVPtemp;

}


void renderRoomToScreen(glm::mat4 depthMVPtemp)
{


		// Compute the MVP matrix from keyboard and mouse input
		computeMatricesFromInputs();
		glm::mat4 ProjectionMatrix2 = getProjectionMatrix();
		glm::mat4 ViewMatrix2 = getViewMatrix();
		//ViewMatrix = glm::lookAt(glm::vec3(14,6,4), glm::vec3(0,1,0), glm::vec3(0,1,0));
//		glm::mat4 ModelMatrix2 = glm::mat4(1.0);
		glm::mat4 ModelMatrix2 = glm::translate( glm::mat4(1.0f), glm::vec3(2.0f, 2.0, 6.0));
		MVP = ProjectionMatrix2 * ViewMatrix2 * ModelMatrix2;


		glm::mat4 depthBiasMVP2 = biasMatrix*depthMVPtemp;

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix2[0][0]);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix2[0][0]);
		glUniformMatrix4fv(DepthBiasID, 1, GL_FALSE, &depthBiasMVP2[0][0]);

		glUniform3f(lightInvDirID, lightInvDir.x, lightInvDir.y, lightInvDir.z);

		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		// Set our "myTextureSampler" sampler to user Texture Unit 0
		glUniform1i(TextureID, 0);

//      if(counter != 1)
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, depthTexture);
		glUniform1i(ShadowMapID, 1);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(0, 3,GL_FLOAT,GL_FALSE,0,(void*)0);
      //attibute, size, type, normalized, stride, array buffer offset

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer2);
		glVertexAttribPointer(1,2,GL_FLOAT, GL_FALSE, 0, (void*)0);

		// 3rd attribute buffer : normals
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, normalbuffer2);
		glVertexAttribPointer(2,3, GL_FLOAT, GL_FALSE, 0, (void*)0 );

		// Index buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

		// Draw the object (can't have shadows without an object)
		glDrawElements(GL_TRIANGLES,indices2.size(),GL_UNSIGNED_SHORT,(void*)0);



}




//glm::mat4 ProjectionMatrix3, ViewMatrix3, ModelMatrix3, depthBiasMVP3;

void renderCubeToScreen(glm::mat4 depthMVPtemp)
{

		// Compute the MVP matrix from keyboard and mouse input
		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();
		glm::mat4 ModelMatrix = translate( mat4(1.0f), vec3(1.0, 1.0, 10.0)) * scale( mat4(1.0f), vec3(2));
		MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

		glm::mat4 depthBiasMVP = biasMatrix*depthMVPtemp;

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);
		glUniformMatrix4fv(DepthBiasID, 1, GL_FALSE, &depthBiasMVP[0][0]);

		glUniform3f(lightInvDirID, lightInvDir.x, lightInvDir.y, lightInvDir.z);

		// Bind our texture in Texture Unit 0
      glEnable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, CLOUDS);
		// Set our "myTextureSampler" sampler to user Texture Unit 0
		glUniform1i(TextureID, 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, depthTexture);
		glUniform1i(ShadowMapID, 1);

      //The Texture mapping
       glEnableVertexAttribArray(h_aTexCoord);
       glBindBuffer(GL_ARRAY_BUFFER, TexBuffObj);
       glVertexAttribPointer(h_aTexCoord, 2, GL_FLOAT, GL_FALSE, 0, 0); 

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, CubeBuffObj);
		glVertexAttribPointer(0, 3,GL_FLOAT,GL_FALSE,0,(void*)0);
      //attibute, size, type, normalized, stride, array buffer offset


		// Index buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, CIndxBuffObj);

		// Draw the object (can't have shadows without an object)
		glDrawElements(GL_TRIANGLES,g_CiboLen,GL_UNSIGNED_SHORT,(void*)0);

}



void renderParticlesToScreen(glm::mat4 depthMVPtemp)
{


		// Compute the MVP matrix from keyboard and mouse input
		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();
		glm::mat4 ModelMatrix = scale(mat4(1.0f), vec3(5.0));
		glm::mat4 ParticleMVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

		glm::mat4 depthBiasMVP = biasMatrix*depthMVPtemp;

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &ParticleMVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);
		glUniformMatrix4fv(DepthBiasID, 1, GL_FALSE, &depthBiasMVP[0][0]);

		glUniform3f(lightInvDirID, lightInvDir.x, lightInvDir.y, lightInvDir.z);

   float sizeScale = 10.0;
    glPointSize(sizeScale);

		// Bind our texture in Texture Unit 0
      glEnable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE0);
//		glBindTexture(GL_TEXTURE_2D, CLOUDS);
		// Set our "myTextureSampler" sampler to user Texture Unit 0
		glUniform1i(TextureID, 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, depthTexture);
		glUniform1i(ShadowMapID, 1);


		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, getDustBuff());
		glVertexAttribPointer(0, 3,GL_FLOAT,GL_FALSE,0,(void*)0);
      //attibute, size, type, normalized, stride, array buffer offset


		// Index buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, getDustInxBuff());


		glDrawElements(GL_POINTS,NUMBER_OF_DUST_PARTICLES,GL_UNSIGNED_SHORT,(void*)0);
    glPointSize(1/sizeScale);


   sizeScale = 2;
    glPointSize(sizeScale);


		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, getSnowBuff());
		glVertexAttribPointer(0, 3,GL_FLOAT,GL_FALSE,0,(void*)0);
      //attibute, size, type, normalized, stride, array buffer offset

		// Index buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, getSnowInxBuff());


		glDrawElements(GL_POINTS,NUMBER_OF_SNOW_PARTICLES,GL_UNSIGNED_SHORT,(void*)0);



    glPointSize(1/sizeScale);


}




static const float g_groundY = -.10;
static const float g_groundSize = 100.0;

GLuint GrndBuffObj, GIndxBuffObj;
int g_GiboLen = 0;

GLuint GNBuffObj;


static void initGround() {

    float GrndPos[] = { 
    -g_groundSize, g_groundY, -g_groundSize, 
    -g_groundSize, g_groundY,  g_groundSize, 
     g_groundSize, g_groundY,  g_groundSize, 
     g_groundSize, g_groundY, -g_groundSize
    };
    unsigned short idx[] = {0, 1, 2, 0, 2, 3};

    float GrndNorm[] = { 
     0, 1, 0, 
     0, 1, 0, 
     0, 1, 0, 
     0, 1, 0, 
     0, 1, 0, 
     0, 1, 0
    };

   static GLfloat GrndTex[] = {
      1, 0, // back 
      1, 1,
      0, 1,
      0, 0,
    }; 
  
    g_GiboLen = 6;
    glGenBuffers(1, &GrndBuffObj);
    glBindBuffer(GL_ARRAY_BUFFER, GrndBuffObj);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GrndPos), GrndPos, GL_STATIC_DRAW);

    glGenBuffers(1, &GIndxBuffObj);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GIndxBuffObj);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);

    glGenBuffers(1, &GNBuffObj);
    glBindBuffer(GL_ARRAY_BUFFER, GNBuffObj);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GrndNorm), GrndNorm, GL_STATIC_DRAW);

    glGenBuffers(1, &TexBuffObj);
    glBindBuffer(GL_ARRAY_BUFFER, TexBuffObj);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GrndTex), GrndTex, GL_STATIC_DRAW);

}


int main( void )
{

//   int counter = 0;
	
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		return -1;
	}
   glfwSetTime(0);

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow( 1024, 768, "Nick trying to impress The Teacher with shadows", NULL, NULL);
	if( window == NULL ){
		fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible.\n" );
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, GL_TRUE);
	glfwSetCursorPos(window, 1024/2, 768/2);

   initCube();
   srand(time(NULL));

  initGround();
  initDustParticles();
  initSnowParticles();


	//Background color
	glClearColor(0.9f, 0.7f, 0.9f, 0.0f);
	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS); 
	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);
   //^^Typical preparation stuff


	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// the depth shaders
	depthProgramID = LoadShaders( "DepthRTT.vertexshader", "DepthRTT.fragmentshader" );
	// Get a handle for our "MVP" uniform
	depthMatrixID = glGetUniformLocation(depthProgramID, "depthMVP");
	// Load the texture for the ball&stick
	Texture = loadDDS("uvmap.DDS");

	// Read our .obj file (The second twin)
	res = loadOBJ("room_thickwalls.obj", vertices2, uvs2, normals2);

	indexVBO(vertices2, uvs2, normals2, indices2, indexed_vertices2, indexed_uvs2, indexed_normals2);

	// Load it into a VBO
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_vertices2.size() * sizeof(glm::vec3), &indexed_vertices2[0], GL_STATIC_DRAW);

	glGenBuffers(1, &uvbuffer2);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer2);
	glBufferData(GL_ARRAY_BUFFER, indexed_uvs2.size() * sizeof(glm::vec2), &indexed_uvs2[0], GL_STATIC_DRAW);

	glGenBuffers(1, &normalbuffer2);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer2);
	glBufferData(GL_ARRAY_BUFFER, indexed_normals2.size() * sizeof(glm::vec3), &indexed_normals2[0], GL_STATIC_DRAW);

	// Generate a buffer for the indices as well

	glGenBuffers(1, &elementbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices2.size() * sizeof(unsigned short), &indices2[0], GL_STATIC_DRAW);

   //FRAME_BUFFER_STUFF  (The Second)
	FramebufferName2 = 0;
	glGenFramebuffers(1, &FramebufferName2);
	glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName2);

	// Depth texture
	glGenTextures(1, &depthTexture);
	glBindTexture(GL_TEXTURE_2D, depthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0,GL_DEPTH_COMPONENT16, 1024, 1024, 0,GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
		 
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTexture, 0);

	glDrawBuffer(GL_NONE);

	// Always check that our framebuffer is ok
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
   {
      printf("BAD\n\n");
		return false;
   }
   //The cube shader. for cubes.
	cubeShaderID = LoadShaders( "tex_vert.glsl", "tex_frag.glsl" );
   h_aPosition = glGetAttribLocation(cubeShaderID, "aPosition");
   h_uTexUnit = glGetUniformLocation(cubeShaderID, "uTexUnit");
   h_uProjMatrix = glGetUniformLocation(cubeShaderID, "uProjMatrix");
   h_uViewMatrix = glGetUniformLocation(cubeShaderID, "uViewMatrix");
   h_uModelMatrix = glGetUniformLocation(cubeShaderID, "uModelMatrix");

	// the actuial drawing shaders
	GLuint programID = LoadShaders( "ShadowMapping.vertexshader", "ShadowMapping.fragmentshader" );

	// Get some handles
	TextureID  = glGetUniformLocation(programID, "myTextureSampler");
	MatrixID = glGetUniformLocation(programID, "MVP");
	ViewMatrixID = glGetUniformLocation(programID, "V");
	ModelMatrixID = glGetUniformLocation(programID, "M");
	DepthBiasID = glGetUniformLocation(programID, "DepthBiasMVP");
	ShadowMapID = glGetUniformLocation(programID, "shadowMap");
   h_simple = glGetUniformLocation(programID, "simple");
   h_aTexCoord = glGetUniformLocation(programID, "aTexCoord");
   h_dust = glGetUniformLocation(programID, "vertDust");

//	GLuint boxShaderID = LoadShaders( "boxvert.glsl", "boxfrag.glsl" );
	
	// Get a handle for our "LightPosition" uniform
	lightInvDirID = glGetUniformLocation(programID, "LightInvDirection_worldspace");

// --------------------------------------------------------------

   glm::mat4 depthRoomMVP, depthCubeMVP, ModelMatrix; //not a global since its passed to the methods	

	do{


      //see if any keys have been triggered to change where the light is
      keyCalling();

//------------------------------------- First render for the shadows
//vv This is the important part

      //Basic setting up of Frame variables
		// Render to our framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName2);
   if(shadow_lights)
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0,0,1024,1024); // Render on the whole framebuffer

		// We don't use bias in the shader, but instead we draw back faces, 
		// which are already separated from the front faces by a small distance 
		// (if your geometry is made this way)
		glDisable(GL_CULL_FACE);
		// glCullFace(GL_BACK); // Cull back-facing triangles -> draw only front-facing triangles

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use our shader
		glUseProgram(depthProgramID);


		ModelMatrix = mat4(1.0f);
      depthRoomMVP = renderToFrame(vertexbuffer, elementbuffer, indices2.size(), ModelMatrix);

		ModelMatrix = translate( mat4(1.0f), vec3(1.0, 1.0, 10.0)) * scale( mat4(1.0f), vec3(2));
      depthCubeMVP = renderToFrame(CubeBuffObj, CIndxBuffObj, g_CiboLen, ModelMatrix);





// ------------------------------------ Now draw the actual things

   if(!shadow_lights)
   {
      //Basic setting up of window variables
		// Render to the screen
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0,0,1024,768); // Render on the whole framebuffer
		glDisable(GL_CULL_FACE);
//		glCullFace(GL_BACK); // Cull back-facing triangles -> draw only front-facing triangles

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// Use our shader
		glUseProgram(programID);

      renderRoomToScreen(depthRoomMVP);

      //The simple test is if to use the simple shader
      //only to be used for the simple geometry fo the cube
      if(simple_test)
   		glUniform1i(h_simple, 1);

      renderCubeToScreen(depthCubeMVP);

      if(simple_test)
   		glUniform1i(h_simple, 0);

   }
   glUniform1f(h_dust, 1.0);

      //HERES THE dust STUFF
//   if(glfwGetTime() < 4.2)
//   {
      renderParticlesToScreen(depthCubeMVP);
//   }


   if( glfwGetTime() > .1)
   {
      moveParticles();
//      glfwSetTime(0);
   }

   glUniform1f(h_dust, 0.0);


		// Swap buffers (cleaner this way)
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 );

//---------------------------------------------

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}




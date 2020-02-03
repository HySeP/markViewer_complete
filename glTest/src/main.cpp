#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>

#include <GLFW/glfw3.h>
GLFWwindow* window;

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

//#include "../../../ogl/common/shader.hpp"
//Using Opengl tutorial git.
#include "shader.hpp"

#include "hsCamera.h"
#include "hsGL.h"

#include <vector>
#include <iostream>
#include <ctime>

using namespace std;
using namespace cv;
/*
bool CleanVBO(unsigned int vertexbuffer, unsigned int VertexArrayID, unsigned int programID) {
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteVertexArrays(1, &VertexArrayID);

    glDeleteProgram(programID);

}

bool makeTri(unsigned int vertexbuffer) {

        // 1rst attribute buffer : vertices
        glClear(GL_COLOR_BUFFER_BIT);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glVertexAttribPointer(
                0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
                3,                  // size
                GL_FLOAT,           // type
                GL_FALSE,           // normalized?
                0,                  // stride
                (void*)0            // array buffer offset
                );

        // Draw the triangle !
        glDrawArrays(GL_TRIANGLES, 0, 3); // 3 indices starting at 0 -> 1 triangle

        glDisableVertexAttribArray(0);

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
}


bool makeGL(void) {
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_RESIZABLE,GL_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    window = glfwCreateWindow( 1024, 768, "Playground", NULL, NULL);
    if( window == NULL ){
        fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
        getchar();
        glfwTerminate();
        return 0;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        return 0;
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
}
*/



int main( void ) {
	// Initialise GLFW
	if( !glfwInit() ) {
		fprintf( stderr, "Failed to initialize GLFW\n" );
		getchar();
		return 0;
	}

	if(!makeGL()) return 0;

	HsCamera *pUsbCam = new HsCamera();
	HsGL *glCam = new HsGL();


    if(!glCam->makeGL()) return 0;


	// Dark blue background
	glClearColor(0.0f, 0.40f, 0.0f, 0.0f);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders
	printf("11111111111\n");
    GLuint programID = LoadShaders( "../shader/SimpleVertexShader.vertexshader", "../shader/SimpleFragmentShader.fragmentshader" );
    //GLuint programID = LoadShaders( "../src/SimpleVertexShader.vertexshader", "../src/SimpleFragmentShader.fragmentshader" );
	printf("22222222222\n");

	// Get a handle for our "MVP" uniform
    GLuint MatrixID = glGetUniformLocation(programID, "MVP");

    // Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
    glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);
    // Or, for an ortho camera :
    //glm::mat4 Projection = glm::ortho(-10.0f,10.0f,-10.0f,10.0f,0.0f,100.0f); // In world coordinates

   	//std::cout << "View matrix = " << std::endl << " "  << View << std::endl << std::endl;


	float l = 1.0f;

    static const GLfloat g_vertex_buffer_data[] = {
        -l, -l, 0.0f,
         l, -l, 0.0f,
         0.0f,  l, 0.0f,
    };

    GLuint vertexbuffer;
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
	

		//Use our shader
	glUseProgram(programID);

	if(!pUsbCam->initCamera("../../asdf.yml")) {
		printf("Parameter file error.\n");
		return 0;
	}

	// Postion data 추출...
	vector<cv::Point3f> markerCorners3d;
	markerCorners3d.push_back(cv::Point3f(-0.5f, 0.5f, 0));
	markerCorners3d.push_back(cv::Point3f(0.5f, 0.5f, 0));
	markerCorners3d.push_back(cv::Point3f(0.5f, -0.5f, 0));
	markerCorners3d.push_back(cv::Point3f(-0.5f, -0.5f, 0));


	cv::Mat src;
	glm::mat4 myCam = glm::mat4(0.0f);

	do {
		/// OpenCV parts  ///

		// Image grab.
		if(!pUsbCam->grab(src))  continue;
		if(!pUsbCam->getCameraImage(src, myCam, markerCorners3d))  continue;

		/// OpenGL parts ///


		// Model matrix : an identity matrix (model will be at the origin)
		glm::mat4 Model      = glm::mat4(1.0f);
		glm::mat4 MVP        = Projection * myCam * Model; // Remember, matrix multiplication is the other way around


		// Send our transformation to the currently bound shader,
		// in the "MVP" uniform

		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

		//make triangle
		if(!glCam->makeTri(vertexbuffer)) continue;

	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
			glfwWindowShouldClose(window) == 0 );



	pUsbCam->releaseCamera();

	// Cleanup VBO

	if(!glCam->CleanVBO(vertexbuffer, VertexArrayID, programID)) return 0;

	delete pUsbCam;
	printf("deleted pUsbCam object.\n");
	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

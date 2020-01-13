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


#include <opencv2/highgui.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/aruco/charuco.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>
#include <iostream>
#include <ctime>
#include "opencv2/calib3d/calib3d.hpp"
#include <opencv2/aruco.hpp>

using namespace std;
using namespace cv;



//cv::VideoCapture vc;

    int squaresX = 5;//인쇄한 보드의 가로방향 마커 갯수
    int squaresY = 7;//인쇄한 보드의 세로방향 마커 갯수
    float squareLength = 36;//검은색 테두리 포함한 정사각형의 한변 길이, mm단위로 입력
    float markerLength = 18;//인쇄물에서의 마커 한변의 길이, mm단위로 입력
    int dictionaryId = 10;//DICT_6X6_250=10
    string outputFile = "output.txt";

    int calibrationFlags = 0;
    float aspectRatio = 1;


    Ptr<aruco::DetectorParameters> detectorParams = aruco::DetectorParameters::create();

    bool refindStrategy =true;
    int camId = 2;

    VideoCapture inputVideo;
	//inputVideo.open(camId);
    int waitTime = 10;

    Ptr<aruco::Dictionary> dictionary =
        aruco::getPredefinedDictionary(aruco::PREDEFINED_DICTIONARY_NAME(dictionaryId));

    // create charuco board object
    Ptr<aruco::CharucoBoard> charucoboard =
        aruco::CharucoBoard::create(squaresX, squaresY, squareLength, markerLength, dictionary);
    Ptr<aruco::Board> board = charucoboard.staticCast<aruco::Board>();

    // collect data from each frame
    vector< vector< vector< Point2f > > > allCorners;
    vector< vector< int > > allIds;
    vector< Mat > allImgs;
    Size imgSize;

    vector< Mat > rvecs, tvecs;




Mat camMatrix, distCoeffs;

bool initCamera(char *paramFile) {
	//FileStorage fs("../../markViewer/build/output.txt",FileStorage::READ);
	FileStorage fs("./output.txt",FileStorage::READ);
	if(!fs.isOpened()) {
			return false;
	}
    fs["camera_matrix"] >> camMatrix;
    fs["distortion_coefficients"] >> distCoeffs;

    inputVideo.open(camId);
	if(!inputVideo.isOpened()) {
		printf("Video open error...\n");
	} else {
		printf("Video open Success...\n");
	}
    waitTime = 10;

	printf("[%s] is OK!!!\n", __func__);
	return true;
}

bool getCameraImage(cv::Mat &src) {



	return false;
}

// releaseCamera 는 지금은 안쓰도록
void releaseCamera() {
	if(inputVideo.isOpened()) {
		inputVideo.release();
		printf("Released VideoCapture.\n");
	}

	//release VideoCapture
	// delete pVideoCapture;
}


int main( void ) {
	// Initialise GLFW
	if( !glfwInit() ) {
		fprintf( stderr, "Failed to initialize GLFW\n" );
		getchar();
		return -1;
	}

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
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	cv::Mat newImg;
	cv::Mat tvec, rvec;
	HsCamera *pUsbCam = new HsCamera();



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





    static const GLfloat g_vertex_buffer_data[] = {
        -1.0f, -1.0f, 0.0f,
         1.0f, -1.0f, 0.0f,
         0.0f,  1.0f, 0.0f,
    };

    GLuint vertexbuffer;
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
	

		//Use our shader
	glUseProgram(programID);


	if(!initCamera("../../asdf.yml")) {
		printf("Parameter file error.\n");
		return 0;
	}


	do {
		glClear(GL_COLOR_BUFFER_BIT);


		// Camera matrix
		/*
		glm::mat4 View      = glm::lookAt(
				glm::vec3(4,3,3), // Camera is at (4,3,3), in World Space
				glm::vec3(0,0,0), // and looks at the origin
				glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
				);
		*/
		//std::cout << "Default " << glm::to_string(View) << std::endl;


		// Image grab.
		if(!inputVideo.grab()) {
			continue;
		} else {
			inputVideo.retrieve(newImg);
			cv::imshow("형석이 소스", newImg);
			cv::waitKey(1);
		}
		// Postion data 추출...
		

		// CV vector -> glm Vector converting.







		glm::mat4 myView = glm::mat4(1.0f);
		glm::vec3 axis = glm::vec3(0, 1, 0);

		//transfer
		//myView = glm::translate(myView, glm::vec3(4, 3, 3));
		myView = glm::translate(myView, glm::vec3(0, 0, -3));
		//myView = glm::translate(myView, glm::vec3(-3, 0, -5));

		//rotation.
		myView = glm::rotate(myView, glm::radians(00.0f), glm::vec3(1, 0, 0));	// X axis rotation.
		myView = glm::rotate(myView, glm::radians( 0.0f), glm::vec3(0, 1, 0));	// y axis rotation.
		myView = glm::rotate(myView, glm::radians(45.0f), glm::vec3(0, 0, 1));	// z axis rotation.
		

		//glm::mat4 scale_m = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
		//glm::mat4 scale_m = glm::scale(myView, glm::vec3(1.0f, 1.0f, 1.0f));

		//std::cout << "Default " << glm::to_string(myView) << std::endl;

		// Model matrix : an identity matrix (model will be at the origin)
		glm::mat4 Model      = glm::mat4(1.0f);
		// Our ModelViewProjection : multiplication of our 3 matrices
		//glm::mat4 MVP        = Projection * View * Model; // Remember, matrix multiplication is the other way around
		glm::mat4 MVP        = Projection * myView * Model; // Remember, matrix multiplication is the other way around




		//if(pUsbCam->getNewImage() && pUsbCam->getCameraPose(tvec, rvec)) {
		//OpenGL camera position setting.
		//}

		//printf("drawing triangle.\n");

        // Send our transformation to the currently bound shader,
        // in the "MVP" uniform
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);



		// Draw nothing, see you in tutorial 2 !

        // 1rst attribute buffer : vertices
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

	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 );


	releaseCamera();

    // Cleanup VBO
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteVertexArrays(1, &VertexArrayID);


	glDeleteProgram(programID);


	delete pUsbCam;
	printf("deleted pUsbCam object.\n");
	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}


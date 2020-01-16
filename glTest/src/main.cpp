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

#include <vector>
#include <iostream>
#include <ctime>

#include <opencv2/highgui.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/aruco/charuco.hpp>
#include <opencv2/imgproc.hpp>
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
Ptr<aruco::CharucoBoard> charucoboard = aruco::CharucoBoard::create(squaresX, squaresY, squareLength, markerLength, dictionary);
Ptr<aruco::Board> board = charucoboard.staticCast<aruco::Board>();

// collect data from each frame
vector< vector< vector< Point2f > > > allCorners;
vector< vector< int > > allIds;
vector< Mat > allImgs;
Size imgSize;

vector< Mat > rvecs, tvecs;


HsCamera hsCam;

//Mat camMatrix, distCoeffs;

/*
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
 */


/*
bool getCameraImage(cv::Mat &src) {


	return false;
}
*/


/*
// releaseCamera 는 지금은 안쓰도록
void releaseCamera() {
	if(inputVideo.isOpened()) {
		inputVideo.release();
		printf("Released VideoCapture.\n");
	}

	//release VideoCapture
	// delete pVideoCapture;
}
*/


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
	//cv::Mat tvec, rvec;
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
	do {

		///////////////////////////////////////////////////////////////////
		// OpenCV parts
		///////////////////////////////////////////////////////////////////

		// Image grab.
		//if(!inputVideo.grab()) continue;

		if(!pUsbCam->grab(src))  continue;
		if(!pUsbCam->getCameraImage(src, markerCorners3d))  continue;




		//	inputVideo.retrieve(newImg);
/*
		Mat image, imageCopy;
		Mat rvec(3, 1, CV_64F), tvec(3, 1, CV_64F);

		vector< int > ids;
		vector< vector< Point2f > > corners, rejected;

		// detect markers
		aruco::detectMarkers(image, dictionary, corners, ids, detectorParams, rejected);

		// refind strategy to detect more markers
		//if (refindStrategy) aruco::refineDetectedMarkers(image, board, corners, ids, rejected);
		aruco::refineDetectedMarkers(image, board, corners, ids, rejected);

		// interpolate charuco corners
		Mat currentCharucoCorners, currentCharucoIds;
		if (ids.size() > 0) aruco::interpolateCornersCharuco(corners, ids, image, charucoboard, currentCharucoCorners, currentCharucoIds);

		// draw results
		image.copyTo(imageCopy);
		if (ids.size() > 0) {
			aruco::drawDetectedMarkers(imageCopy, corners);
			int seq=0;
			vector<Point2f> mark8Points;

			for(vector<int>::iterator itr = ids.begin(); itr != ids.end(); itr++) {
				if(*itr == 8) {
					printf("----[%d]\n", *itr);

					mark8Points = corners[seq]; // = m;

					//Mat input_image = imread("test.jpg", IMREAD_COLOR);
					//Mat input_image = imread("test.jpg", IMREAD_COLOR);

					solvePnP(markerCorners3d, mark8Points, camMatrix, distCoeffs, rvec, tvec);
					cout << "markerID" << *itr << endl;
					cout << "rotation_vector" << endl << rvec << endl;
					cout << "translation_vector" << endl << tvec << endl;


					//for(vector<Point2f>::iterator iter=mark8Points.begin(); iter!=mark8Points.end();iter++) printf("[%f,%f]\n", iter[0].x, iter[0].y);

					aruco::drawAxis(imageCopy, camMatrix, distCoeffs, rvec, tvec, 1.0);


				}
				seq++;
			}
		}

		if (currentCharucoCorners.total() > 0)
			aruco::drawDetectedCornersCharuco(imageCopy, currentCharucoCorners, currentCharucoIds);

		putText(imageCopy, "Press 'c' to add current frame. 'ESC' to finish and calibrate",
				Point(10, 20), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(255, 0, 0), 2);

		imshow("out", imageCopy);
		char key = (char)waitKey(waitTime);
		if (key == 27) break;
		if (key == 'c' && ids.size() > 0) {
			cout << "Frame captured" << endl;
			allCorners.push_back(corners);
			allIds.push_back(ids);
			allImgs.push_back(image);
			imgSize = image.size();
		}

		Mat matR;
		Rodrigues(rvec, matR);
		cv::Rodrigues(rvec, matR);
		cv::Mat T = cv::Mat::eye(4, 4, matR.type()); // T is 4x4 unit matrix.
		for(unsigned int row=0; row<3; ++row) {
			for(unsigned int col=0; col<3; ++col) {
				T.at<double>(row, col) = matR.at<double>(row, col);
			}
			T.at<double>(row, 3) = tvec.at<double>(row, 0);
		}

		///////////////////////////////////////////////////////////////////
		//Converting parts
		///////////////////////////////////////////////////////////////////

		//Convert CV to GL
		cv::Mat cvToGl = cv::Mat::zeros(4, 4, CV_64F);
		cvToGl.at<double>(0, 0) =  1.0f;
		cvToGl.at<double>(1, 1) = -1.0f; // Invert the y axis
		cvToGl.at<double>(2, 2) = -1.0f; // invert the z axis
		cvToGl.at<double>(3, 3) =  1.0f;
		T = cvToGl * T;

		//Convert to cv::Mat to glm::mat4.
		glm::mat4 camPose = glm::mat4(0.0f);
		for(int i=0; i < T.cols; i++) {
			for(int j=0; j < T.rows; j++) {
				camPose[j][i] = *T.ptr<double>(i, j);
			}
		}
*/


		///////////////////////////////////////////////////////////////////
		// OpenGL parts
		///////////////////////////////////////////////////////////////////
/*
		        //TODO: delete;

*/
		glm::mat4 camPose = glm::mat4(0.0f);

		// CV vector -> glm Vector converting.
		glm::mat4 myView = glm::mat4(1.0f);
		glm::vec3 axis = glm::vec3(0, 1, 0);

		// Model matrix : an identity matrix (model will be at the origin)
		glm::mat4 Model      = glm::mat4(1.0f);
		glm::mat4 MVP        = Projection * camPose * Model; // Remember, matrix multiplication is the other way around


		//if(pUsbCam->getNewImage() && pUsbCam->getCameraPose(tvec, rvec)) {
		//OpenGL camera position setting.
		//}

		//printf("drawing triangle.\n");

		// Send our transformation to the currently bound shader,
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

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

	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
			glfwWindowShouldClose(window) == 0 );

	pUsbCam->releaseCamera();

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

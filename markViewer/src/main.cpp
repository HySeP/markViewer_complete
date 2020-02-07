/*
#include <opencv2/highgui.hpp>
#include <opencv2/aruco/charuco.hpp>

using namespace cv;

namespace {
const char* about = "Create a ChArUco board image";
const char* keys =
"{@outfile |<none> | Output image }"
"{w        |       | Number of squares in X direction }"
"{h        |       | Number of squares in Y direction }"
"{sl       |       | Square side length (in pixels) }"
"{ml       |       | Marker side length (in pixels) }"
"{d        |       | dictionary: DICT_4X4_50=0, DICT_4X4_100=1, DICT_4X4_250=2,"
"DICT_4X4_1000=3, DICT_5X5_50=4, DICT_5X5_100=5, DICT_5X5_250=6, DICT_5X5_1000=7, "
"DICT_6X6_50=8, DICT_6X6_100=9, DICT_6X6_250=10, DICT_6X6_1000=11, DICT_7X7_50=12,"
"DICT_7X7_100=13, DICT_7X7_250=14, DICT_7X7_1000=15, DICT_ARUCO_ORIGINAL = 16}"
"{m        |       | Margins size (in pixels). Default is (squareLength-markerLength) }"
"{bb       | 1     | Number of bits in marker borders }"
"{si       | false | show generated image }";
}

int main(int argc, char *argv[]) {

int squaresX = 5;//가로방향 마커 갯수
int squaresY = 7;//세로방향 마터 갯수
int squareLength = 80; //검은색 테두리 포함한 정사각형의 한변 길이 , 픽셀단위
int markerLength = 40;//마커 한 변의 길이, 픽셀단위
int dictionaryId = 10; //DICT_6X6_250=10
int margins = 10;//ChArUco board와 A4용지 사이의 흰색 여백 크기, 픽셀단위
int borderBits = 1;//검은색 테두리 크기
bool showImage = false;

String out = "board.jpg";


Ptr<aruco::Dictionary> dictionary =
aruco::getPredefinedDictionary(aruco::PREDEFINED_DICTIONARY_NAME(dictionaryId));

Size imageSize;
imageSize.width = squaresX * squareLength + 2 * margins;
imageSize.height = squaresY * squareLength + 2 * margins;

Ptr<aruco::CharucoBoard> board = aruco::CharucoBoard::create(squaresX, squaresY, (float)squareLength,
(float)markerLength, dictionary);

// show created board
Mat boardImage;
board->draw(imageSize, boardImage, margins, borderBits);

if (showImage) {
imshow("board", boardImage);
waitKey(0);
}

imwrite(out, boardImage);

return 0;
}
 */

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

/**
 */
static bool saveCameraParams(const string &filename, Size imageSize, float aspectRatio, int flags,
		const Mat &cameraMatrix, const Mat &distCoeffs, double totalAvgErr) {
	FileStorage fs(filename, FileStorage::WRITE);
	if (!fs.isOpened())
		return false;

	time_t tt;
	time(&tt);
	struct tm *t2 = localtime(&tt);
	char buf[1024];
	strftime(buf, sizeof(buf) - 1, "%c", t2);

	fs << "calibration_time" << buf;

	fs << "image_width" << imageSize.width;
	fs << "image_height" << imageSize.height;

	if (flags & CALIB_FIX_ASPECT_RATIO) fs << "aspectRatio" << aspectRatio;

	if (flags != 0) {
		sprintf(buf, "flags: %s%s%s%s",
				flags & CALIB_USE_INTRINSIC_GUESS ? "+use_intrinsic_guess" : "",
				flags & CALIB_FIX_ASPECT_RATIO ? "+fix_aspectRatio" : "",
				flags & CALIB_FIX_PRINCIPAL_POINT ? "+fix_principal_point" : "",
				flags & CALIB_ZERO_TANGENT_DIST ? "+zero_tangent_dist" : "");
	}

	fs << "flags" << flags;

	fs << "camera_matrix" << cameraMatrix;
	fs << "distortion_coefficients" << distCoeffs;

	fs << "avg_reprojection_error" << totalAvgErr;

	return true;
}

/**
 */
int main(int argc, char *argv[]) {

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
	int waitTime;

	inputVideo.open(camId);
	waitTime = 10;


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
	//Get calibration datas.
	Mat camMatrix, distCoeffs;

	FileStorage fs("../output.txt", FileStorage::READ);
	if (!fs.isOpened())
		return false;
	fs["camera_matrix"] >> camMatrix;
	fs["distortion_coefficients"] >> distCoeffs;

	vector<cv::Point3f> markerCorners3d;
	markerCorners3d.push_back(cv::Point3f(-0.5f, 0.5f, 0));
	markerCorners3d.push_back(cv::Point3f(0.5f, 0.5f, 0));
	markerCorners3d.push_back(cv::Point3f(0.5f, -0.5f, 0));
	markerCorners3d.push_back(cv::Point3f(-0.5f, -0.5f, 0));

	while (inputVideo.grab()) {
		Mat image, imageCopy;
		inputVideo.retrieve(image);

		vector< int > ids;
		vector< vector< Point2f > > corners, rejected;

		// detect markers
		aruco::detectMarkers(image, dictionary, corners, ids, detectorParams, rejected);

		// refind strategy to detect more markers
		if (refindStrategy) aruco::refineDetectedMarkers(image, board, corners, ids, rejected);

		// interpolate charuco corners
		Mat currentCharucoCorners, currentCharucoIds;
		if (ids.size() > 0) aruco::interpolateCornersCharuco(corners, ids, image, charucoboard, currentCharucoCorners, currentCharucoIds);

		// draw results
		image.copyTo(imageCopy);
		if (ids.size() > 0) {
			aruco::drawDetectedMarkers(imageCopy, corners);
			int seq=0;
			vector<Point2f> mark8Points;
			//vector<Point2f> *pMark8Points;

			for(vector<int>::iterator itr = ids.begin(); itr != ids.end(); itr++) {
				if(*itr == 8) {
					printf("----[%d]\n", *itr);

					mark8Points = corners[seq]; // = m;
					
					Mat rotation_vector, translation_vector;
					//Mat input_image = imread("test.jpg", IMREAD_COLOR);
					//Mat input_image = imread("test.jpg", IMREAD_COLOR);
					
					solvePnP(markerCorners3d, mark8Points, camMatrix, distCoeffs, rotation_vector, translation_vector);
					cout << "markerID" << *itr << endl;
					cout << "rotation_vector" << endl << rotation_vector << endl;
					cout << "translation_vector" << endl << translation_vector << endl;

					//for(vector<Point2f>::iterator iter=mark8Points.begin(); iter!=mark8Points.end();iter++) printf("[%f,%f]\n", iter[0].x, iter[0].y);

					aruco::drawAxis(imageCopy, camMatrix, distCoeffs, rotation_vector, translation_vector, 1.0);

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
	}

	if (allIds.size() < 1) {
		cerr << "Not enough captures for calibration" << endl;
		return 0;
	}


	/*
	   double repError;
	   if (calibrationFlags & CALIB_FIX_ASPECT_RATIO) {
	   cameraMatrix = Mat::eye(3, 3, CV_64F);
	   cameraMatrix.at< double >(0, 0) = aspectRatio;
	   }

	// prepare data for calibration
	vector< vector< Point2f > > allCornersConcatenated;
	vector< int > allIdsConcatenated;
	vector< int > markerCounterPerFrame;
	markerCounterPerFrame.reserve(allCorners.size());
	for (unsigned int i = 0; i < allCorners.size(); i++) {
	markerCounterPerFrame.push_back((int)allCorners[i].size());
	for (unsigned int j = 0; j < allCorners[i].size(); j++) {
	allCornersConcatenated.push_back(allCorners[i][j]);
	allIdsConcatenated.push_back(allIds[i][j]);
	}
	}

	// calibrate camera using aruco markers
	double arucoRepErr;
	arucoRepErr = aruco::calibrateCameraAruco(allCornersConcatenated, allIdsConcatenated,
	markerCounterPerFrame, board, imgSize, cameraMatrix,
	distCoeffs, noArray(), noArray(), calibrationFlags);

	// prepare data for charuco calibration
	int nFrames = (int)allCorners.size();
	vector< Mat > allCharucoCorners;
	vector< Mat > allCharucoIds;
	vector< Mat > filteredImages;
	allCharucoCorners.reserve(nFrames);
	allCharucoIds.reserve(nFrames);

	for (int i = 0; i < nFrames; i++) {
// interpolate using camera parameters
Mat currentCharucoCorners, currentCharucoIds;
aruco::interpolateCornersCharuco(allCorners[i], allIds[i], allImgs[i], charucoboard,
currentCharucoCorners, currentCharucoIds, cameraMatrix,
distCoeffs);

allCharucoCorners.push_back(currentCharucoCorners);
allCharucoIds.push_back(currentCharucoIds);
filteredImages.push_back(allImgs[i]);
}

if (allCharucoCorners.size() < 4) {
cerr << "Not enough corners for calibration" << endl;
return 0;
}

// calibrate camera using charuco
repError =
aruco::calibrateCameraCharuco(allCharucoCorners, allCharucoIds, charucoboard, imgSize,
cameraMatrix, distCoeffs, rvecs, tvecs, calibrationFlags);

bool saveOk = saveCameraParams(outputFile, imgSize, aspectRatio, calibrationFlags,
cameraMatrix, distCoeffs, repError);
if (!saveOk) {
cerr << "Cannot save output file" << endl;
return 0;
}

cout << "Rep Error: " << repError << endl;
cout << "Rep Error Aruco: " << arucoRepErr << endl;
cout << "Calibration saved to " << outputFile << endl;
	 */


return 0;
}


#ifndef HSCAMERA_H
#define HSCAMERA_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>
#include <opencv2/aruco/charuco.hpp>


#include <glm/glm.hpp>

using namespace std;
using namespace cv;

class HsCamera {
	private:
 	   	int squaresX = 5;//인쇄한 보드의 가로방향 마커 갯수
   	 	int squaresY = 7;//인쇄한 보드의 세로방향 마커 갯수
    	float squareLength = 36;//검은색 테두리 포함한 정사각형의 한변 길이, mm단위로 입력
  		float markerLength = 18;//인쇄물에서의 마커 한변의 길이, mm단위로 입력
    	int dictionaryId = 10;//DICT_6X6_250=10
    	string outputFile = "output.txt";

    	Ptr<aruco::DetectorParameters> detectorParams;
    	Ptr<aruco::Dictionary> dictionary;
		Ptr<aruco::CharucoBoard> charucoboard;

		Ptr<aruco::Board> board;

		int camId = 0;
		VideoCapture inputVideo;

		vector< vector< vector< Point2f > > > allCorners;
		vector< vector< int > > allIds;
		vector< Mat > allImgs;
		Size imgSize;

vector< Mat > rvecs, tvecs;


	protected:
		int  waitTime = 10;
		Mat camMatrix, distCoeffs;
	public:
		HsCamera();
		virtual ~HsCamera();

		bool initCamera(const char * paramFile);
		bool grab(cv::Mat &rtn);
		bool getCameraImage(cv::Mat& src, vector<cv::Point3f> mc3d);
		bool getPose(cv::Mat& src, cv::Mat T);
		bool releaseCamera();
};

#endif

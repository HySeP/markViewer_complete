#include "hsCamera.h"

HsCamera::HsCamera() {
   detectorParams = aruco::DetectorParameters::create();		
   dictionary = aruco::getPredefinedDictionary(aruco::PREDEFINED_DICTIONARY_NAME(dictionaryId));
}

HsCamera::~HsCamera() {
}

bool HsCamera::initCamera(const char * paramFile) {
	Mat camMatrix, distCoeffs;

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
   int  waitTime = 10;

    printf("[%s] is OK!!!\n", __func__);

    return true;
}

bool HsCamera::releaseCamera() {
    if(inputVideo.isOpened()) {
        inputVideo.release();
        printf("Released VideoCapture.\n");
		return true;
    } else {
		return false;
	}
}

bool HsCamera::getCameraImage(cv::Mat & src){
    do {
        glClear(GL_COLOR_BUFFER_BIT);

        // Image grab.
        if(!inputVideo.grab()) {
            continue;
        }
	 
	vector<cv::Point3f> markerCorners3d;
    markerCorners3d.push_back(cv::Point3f(-0.5f, 0.5f, 0));
    markerCorners3d.push_back(cv::Point3f(0.5f, 0.5f, 0));
    markerCorners3d.push_back(cv::Point3f(0.5f, -0.5f, 0));
    markerCorners3d.push_back(cv::Point3f(-0.5f, -0.5f, 0));


        Mat image, imageCopy;
        Mat rvec(3, 1, CV_64F), tvec(3, 1, CV_64F);
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
        glm::mat4 camPose = glm::mat4(0.0f);
        Rodrigues(rvec, matR);

        cv::Rodrigues(rvec, matR);
        cv::Mat T = cv::Mat::eye(4, 4, matR.type()); // T is 4x4 unit matrix.
        for(unsigned int row=0; row<3; ++row) {
            for(unsigned int col=0; col<3; ++col) {
                T.at<double>(row, col) = matR.at<double>(row, col);
            }
            T.at<double>(row, 3) = tvec.at<double>(row, 0);
        }

        //Convert CV to GL
        cv::Mat cvToGl = cv::Mat::zeros(4, 4, CV_64F);
        cvToGl.at<double>(0, 0) =  1.0f;
        cvToGl.at<double>(1, 1) = -1.0f; // Invert the y axis
        cvToGl.at<double>(2, 2) = -1.0f; // invert the z axis
        cvToGl.at<double>(3, 3) =  1.0f;
        T = cvToGl * T;

        //Convert to cv::Mat to glm::mat4.
        for(int i=0; i < T.cols; i++) {
            for(int j=0; j < T.rows; j++) {
                camPose[j][i] = *T.ptr<double>(i, j);
            }
        }


	return true;
}

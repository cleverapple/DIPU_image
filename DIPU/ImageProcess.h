#pragma once
#include <vector>
#include <string>
#include <stdio.h>
#include <math.h>
#include <iostream>

//#define RNG rng(12345);

//
#include "use_opencv.h"

//// openCV 관련 헤더
//#include <core.hpp>
//
//#include "opencv2/core/utility.hpp"
//#include "opencv2/imgproc.hpp"
//#include "opencv2/imgcodecs.hpp"
//#include "opencv2/highgui.hpp"

#define FILE "color.png"
#define LimitDistanceEQU2PT 0.5
#define LimitDistancePT2PT 5

#define MinimumContourPixel 3

#define CannyThresh 100
#define CannyImageBrightness 7
#define CannyImageBrightnessTerm 1
#define A4Y double(210)
#define A4X double(297)
#define A4RATIO double(297/210)

#define XPoint (105)
#define YPoint (-148.5)

#define B 0
#define G 1
#define R 2

#define ColorNum 4


using namespace cv;
using namespace std;

class DIPU
{

public:
	int ImageProcess();
	int ImageProcess_video();

	void myDrawContours(InputOutputArray image, vector<vector<Point>> contours, bool Dot=0);



	vector<vector<Point>> ContourApproximation(Mat src);
	vector<vector<Point2d>> ContoursTransform(Mat src, vector<vector<Point>> contours);




	Mat ColorTransform(Mat src);


	int test();
	Point2f FindLinearEquation(Point Pt_A, Point Pt_B);
	float FindDistance_equ2pt(Point2f equ, Point Pt);
	float FindDistance_pt2pt(Point Pt_A, Point Pt_B);

private:
	;
	
};
#include "ImageProcess.h"
Mat A_drawing;
unsigned char PenColor[ColorNum][3] = { { 0,0,0 },{ 60,60,255 },{ 200,220,250 },{ 255,255,255 } };// 검 빨 살 흰


int DIPU::ImageProcess()
{
	cout << "Image Process";

	char* filename = FILE;

	Mat src = imread(filename, 0);		//원본 이미지
	Mat src_3c = imread(filename, 3);	//원본 이미지(3c)
	Mat edge = imread(filename, 0);		//edge 이미지
	Mat edge_3c = imread(filename, 3);	//edge 이미지(3c)
	Mat color_3c = imread(filename, 3);	//색깔 이미지

	Mat dst = imread(filename, 3);		//최종 이미지

	int edgeThresh = CannyThresh;

	//imshow("sourc", src);

	if (src.empty()) {
		cout << "can not open" << endl;
		return 1;
	}

	GaussianBlur(src, src, Size_<int>(5, 5), 1);

	///외곽선 추출
	// 밝기 값에 맞추어 canny edge detect
	while (1) {
		Canny(src, edge, edgeThresh, edgeThresh * 3, 3);
		Scalar a = ((mean(edge)));
		cout << a[0] << "     " << edgeThresh << endl;
		if (a[0] < CannyImageBrightness - CannyImageBrightnessTerm) edgeThresh--;
		else if (a[0] > CannyImageBrightness + CannyImageBrightnessTerm) edgeThresh++;
		else {
			cout << edgeThresh << endl;
			break;
		}
	}

	// Contour approximation and Conversion 
	vector<vector<Point>> contours;
	vector<vector<Point2d>> TransformContours;
	contours=ContourApproximation(edge);
	//TransformContours=ContoursTransform(edge, contours);

	///색상 추출
	//pyrMeanShiftFiltering(src_3c, color_3c, 25, 70, 2);
	color_3c=ColorTransform(src_3c);
	cvtColor(edge, edge_3c, CV_GRAY2BGR);
	//subtract(color_3c, edge_3c, dst);
	subtract(color_3c, A_drawing, dst);

	imshow("sourc", src_3c);
	imshow("edge", edge);
	imshow("color", color_3c);
	imshow("dst", dst);

	return 0;
}


int DIPU::test()
{
	Mat image= imread(FILE, 3);

	Mat color = ColorTransform(image);

	imshow("dst", color);
	return 0;
}

vector<vector<Point>> DIPU::ContourApproximation(Mat src)
{
	RNG rng(12345);
	Mat dst = src.clone();

	//imshow("sourc", src);

	if (src.empty()) {
		//cout << "can not open" << endl;
		//return 1;
	}

	//RNG rng(12345);
	// Mat output;
	vector<vector<Point>> contours;
	vector<vector<Point>> DividedContours;
	vector<vector<Point>> ApproximatedContours;
	vector<Vec4i> hierarchy;

	findContours(src, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE, Point(0, 0));

	// Draw contours
	cout << "Draw contours**********\n";
	int CountourNum = 0;
	Mat drawing = Mat::zeros(src.size(), CV_8UC3);
	for (int i = 0; i < contours.size(); i++)
	{
		Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
		drawContours(drawing, contours, i, color, 1, 8, hierarchy, 0, Point());
		CountourNum += contours[i].size();
		
	//	cout << "contour " << i + 1 << "   size : " << contours[i].size() << endl;
	}
	cout << "Contour point : " << CountourNum << endl;
	cout << "countour number : " << contours.size() << endl << endl;



	// Check Overlap
	for (int i = 0; i < contours.size(); i++) {
		for (int j = 0; j < contours[i].size(); j++) {
			bool overlap = 0;
			for (int k = 0; k < j; k++) {
				if ((contours[i][j].x == contours[i][k].x) && (contours[i][j].y == contours[i][k].y)) overlap = 1;
			}
			if (overlap == 1) {
				overlap = 0;
				contours[i][j].x = contours[i][j].y = 0;
			}
		}
	}


	// Make Contours (DividedContours)
	for (int i = 0; i < contours.size(); i++) {
		for (int j = 0; j < contours[i].size(); j++) {
			vector<Point> it1;

			for (j; j < contours[i].size(); j++) {
				if ((contours[i][j].x == 0) && (contours[i][j].y == 0)) break;
				else {
					Point point(contours[i][j].x, contours[i][j].y);
					it1.push_back(point);
				}
			}
			if (it1.size() >MinimumContourPixel) DividedContours.push_back(it1);
		}
	}
	int DividedCountourNum = 0;
	// Check contours(remove overlap)
	cout << endl << "Divided Contour **********" << endl;
	for (int i = 0; i < DividedContours.size(); i++) {
		//cout << "contour " << i + 1 << "   size : " << DividedContours[i].size()<<endl ;
		DividedCountourNum += DividedContours[i].size();
		for (int j = 0; j < DividedContours[i].size(); j++) {
			//cout << DividedContours[i][j] << endl;
		}
	}
	cout << "Divided Contour point : " << DividedCountourNum << endl;
	cout << "countour number : " << DividedContours.size() << endl << endl;
	//cout << endl;

	// Polygun Approximation 
	for (int i = 0; i < DividedContours.size(); i++) {
		vector<Point> it1;
		Point2f EquationPT2PT;
		float DistanceEQU2PT = 0, DistancePT2PT;
		int num = 0;
		for (int j = 0; j < DividedContours[i].size(); j++) {
			Point point(DividedContours[i][j].x, DividedContours[i][j].y);

			// 첫 번째 포인트, 마지막 포인트 저장
			if (j == 0) {
				it1.push_back(point);
				continue;
			}
			else if (j == DividedContours[i].size() - 1) {
				it1.push_back(point);
				continue;
			}

			// 아닌 경우 직선의 방정식을 구하고 이전 점과 현재 점 사이의 거리를 구한다.
			else{
				EquationPT2PT = FindLinearEquation(it1[it1.size() - 1], point);
				DistancePT2PT = FindDistance_pt2pt(it1[it1.size() - 1], point);
				if (DistancePT2PT > LimitDistancePT2PT) goto save;
				//cout << "Points : " << it1[it1.size() - 1] << point << EquationPT2PT << endl;
			}
			for (int k = num; k < j; k++) {
				DistanceEQU2PT = FindDistance_equ2pt(EquationPT2PT, DividedContours[i][k]);
				if (DistanceEQU2PT > LimitDistanceEQU2PT) goto save;
				//cout <<"Distance : "<< DividedContours[i][k] << DistanceEQU2PT <<endl;
			}
			continue;

		save:
			//cout << "save Point" << point << endl << endl;
			num = j;
			it1.push_back(point);
			continue;
		}
		if (it1.size() !=0 ) ApproximatedContours.push_back(it1);
	}

	// Check contours(approximation)
	cout <<endl<< "Approximated Contour ********" << endl;
	int  ApproximatedCountourNum = 0;
	for (int i = 0; i < ApproximatedContours.size(); i++) {
		//cout << "contour " << i + 1 << "   size : " << ApproximatedContours[i].size()<<endl;
		ApproximatedCountourNum += ApproximatedContours[i].size();
		for (int j = 0; j < ApproximatedContours[i].size(); j++) {
			//cout << ApproximatedContours[i][j] << endl;
		}
	}
	cout << "Approximated Contour point : " << ApproximatedCountourNum << endl;
	cout << "countour number : " << ApproximatedContours.size() << endl << endl;
	//cout << endl;

	//DividedContours
	Mat T_drawing = Mat::zeros(src.size(), CV_8UC3);
	for (int i = 0; i < DividedContours.size()-1; i++)
	{
		myDrawContours(T_drawing, DividedContours);
	}

	//ApproximatedContours
	A_drawing = Mat::zeros(src.size(), CV_8UC3);
	for (int i = 0; i < ApproximatedContours.size()-1; i++)
	{
		myDrawContours(A_drawing, ApproximatedContours, 1);
	}
	return ApproximatedContours;

}


vector<vector<Point2d>> DIPU::ContoursTransform(Mat src , vector<vector<Point>> contours)
{
	// Convert form pixel to mm
	int TransformMode = 0;
	double ImageRatio = src.cols / src.rows;
	if (ImageRatio > A4RATIO) { cout << "가로길이 맞추기*************\n"; } //가로길이에 맞추기
	else { cout << "세로길이 맞추기***********\n"; } //세로길이에 맞추기

	vector<vector<Point2d>> TransformContour;
	for (int i = 0; i < contours.size(); i++) {
		cout << "\ncontour " << i + 1 << endl;

		for (int j = 0; j < contours[i].size(); j++) {
			vector<Point2d> it1;
			if (ImageRatio > A4RATIO)//가로길이에 맞추기
			{
				Point2d point(contours[i][j].x *(A4X/src.cols)+XPoint, contours[i][j].y*(A4X/src.cols)+(A4Y)*(1-A4X/src.cols)/2 +YPoint);
				cout << point<<endl;
				it1.push_back(point);
			}
			else //세로길이에 맞추기
			{
				Point2d point(contours[i][j].x * (A4Y/src.rows)+(A4X)*(1-A4Y/src.cols)/2 + XPoint, contours[i][j].y*(A4Y/src.rows)+YPoint);
				cout << point<<endl;
				it1.push_back(point);
			}
			if (it1.size() != 0) TransformContour.push_back(it1);
		}
		cout << endl;
	}

	return TransformContour;
}

Point2f DIPU::FindLinearEquation(Point Pt_A, Point Pt_B)
{
	Point2f equ;
	float m = 500;
	if ((Pt_A.x) != Pt_B.x) m = ((float(Pt_A.y) - Pt_B.y) / (float(Pt_A.x) - Pt_B.x)); //기울기
	float n = Pt_A.y - m*Pt_A.x; //절편
	equ.x = m;
	equ.y = n;
	return equ;
}

float DIPU::FindDistance_equ2pt(Point2f equ, Point Pt)
{
	return(abs(equ.x*Pt.x - Pt.y + equ.y) / sqrt(equ.x*equ.x + 1));
}

float DIPU::FindDistance_pt2pt(Point Pt_A, Point Pt_B)
{
	return sqrt((Pt_A.x-Pt_B.x)*(Pt_A.x - Pt_B.x)+ (Pt_A.y - Pt_B.y)*(Pt_A.y - Pt_B.y));
}



void DIPU::myDrawContours(InputOutputArray image, vector<vector<Point>> contours, bool Dot)
{
	RNG rng(12345);
	for (int i = 0; i < contours.size(); i++) {
		Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
		for (int j = 1; j < contours[i].size(); j++) {
			Point pt1(contours[i][j - 1].x, contours[i][j - 1].y), pt2(contours[i][j].x, contours[i][j].y);
			line(image, pt1, pt2, color);
			if (Dot == 1) {
				line(image, pt1, pt1, Scalar(255, 255, 255), 2);
				line(image, pt2, pt2, Scalar(255, 255, 255), 2);
			}
		}
	}
}


int DIPU::ImageProcess_video()
{
	cout << "Image Process for Video";

	CvCapture* capture = NULL;
	IplImage* grabImage;

	cvGrabFrame(capture);
	grabImage = cvRetrieveFrame(capture);

	// else if (grabImage == NULL) {
	// }

	return 0;


}


//////////////////////////////////////////////////
//color
Mat DIPU::ColorTransform(Mat src)
{
	Mat color = src.clone();
	pyrMeanShiftFiltering(src, color, 25, 70, 2);

	TypedMat<unsigned char> tmSrc = src;
	TypedMat<unsigned char> tmColor = color;

	unsigned int distance = 0xffffffff;
	int ColorIndex = 0;

	for (int i = 0; i < src.cols; i++) {
		for (int j = 0; j < src.rows; j++) {
			for (int k = 0; k < ColorNum; k++) {
				int blue = (tmSrc(j, i, B) - PenColor[k][B]);
				int green = (tmSrc(j, i, G) - PenColor[k][G]);
				int red = (tmSrc(j, i, R) - PenColor[k][R]);

				unsigned int squre = (blue*blue + green*green + red*red);

				if (distance > squre) {
					distance = squre;
					ColorIndex = k;
				}
			}
			tmColor(j, i, B)= PenColor[ColorIndex][B];
			tmColor(j, i, G) = PenColor[ColorIndex][G];
			tmColor(j, i, R) = PenColor[ColorIndex][R];
			distance = 0xffffffff;
		}
	}
	return color;
}

//TypedMat<unsigned char> tm = image;    // 연결방법 1

									   // image가 1채널 grayscale 이미지일 경우
									   //tm[y][x] = 100;    // (x,y)의 픽셀값을 100으로 설정

									   // image가 3채널 color 이미지일 경우
									   //tm(y, x, 0) = 100;    // (x,y)의 픽셀의 blue값을 100으로 설정
									   //tm(y, x, 1) = 200;    // (x,y)의 픽셀의 green값을 200으로 설정
									   //tm(y, x, 2) = 50;    // (x,y)의 픽셀의 red값을 50으로 설정
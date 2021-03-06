#include "stdafx.h"

/*
##########
動作テスト
##########
一番下のキャプチャ･画像加工が動かないときに
テストとして使ってみてね
*/

/*
int main()
{
	//width220, height150の画像を作成
	Mat src = Mat::zeros(150, 220, CV_8UC3);

	//赤色で画像にHello Worldを描く
	putText(src, "Hello World", Point(5, 50), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 200), 2, CV_AA);

	//緑色で画像に線を描く
	line(src, Point(190, 25), Point(190, 45), Scalar(0, 200, 0), 3);

	//要素を直接操作し画像に点を描く
	for (int x = 188; x < 192; x++) {
		for (int y = 53; y < 57; y++) { // y座標
			for (int i = 0; i < 2; i++) { // i < 3にするとBGRすべての要素を200にする
				src.at<uchar>(Point(x * 3 + i, y)) = saturate_cast<uchar>(200);
			}
		}
	}

	//画像を表示
	imshow("", src);

	//ウインドウの表示時間(0はキーが押されるまで閉じない)
	waitKey(0);

	return 0;
}
*/

/*
#################################
エッジ検出(Sobel, Laplasian, もろもろ)
#################################
これもテスト用
*/
/*
#include <opencv/cv.h>
#include <opencv/highgui.h>


int
main (int argc, char **argv)
{
	IplImage *src_img, *sobel_img, *laplaian_img, *canny_img;
	IplImage *tmp_img;
	char *imagename;

	// (1)load a specified file as a grayscale image
	//	and allocate destination images
	imagename = argc > 1 ? imagename = argv[1] : imagename = "C:/Users/masaki/Documents/Visual Studio 2015/Projects/opencvsample/x64/lena.png";
	src_img = cvLoadImage(imagename, CV_LOAD_IMAGE_GRAYSCALE);
	if (src_img == 0) {
		printf("NULLLLLLLLLLLLLLLLLL\n");
		return -1;
	}
	tmp_img = cvCreateImage(cvGetSize(src_img), IPL_DEPTH_32F, 1);
	sobel_img = cvCreateImage(cvGetSize(src_img), IPL_DEPTH_8U, 1);
	laplaian_img = cvCreateImage(cvGetSize(src_img), IPL_DEPTH_8U, 1);
	canny_img = cvCreateImage(cvGetSize(src_img), IPL_DEPTH_8U, 1);

	// (2)calculate the first image derivatives using an Sobel operator
	cvSobel(src_img, tmp_img, 1, 1, 3);
	cvConvertScaleAbs(tmp_img, sobel_img, 1, 0);

	// (3)calculate the Laplacian of an image
	cvLaplace(src_img, tmp_img, 3);
	cvConvertScaleAbs(tmp_img, laplaian_img, 1, 0);

	// (4)implement the Canny algorithm for edge detection
	cvCanny(src_img, canny_img, 50.0, 200.0, 3);

	// (5)show original gray and their edge images respectively,
	//	and quit when any key pressed
	cvNamedWindow("Original(GrayScale)", CV_WINDOW_AUTOSIZE);
	cvNamedWindow("Sobel", CV_WINDOW_AUTOSIZE);
	cvNamedWindow("Laplacian", CV_WINDOW_AUTOSIZE);
	cvNamedWindow("Canny", CV_WINDOW_AUTOSIZE);
	cvShowImage("Original(GrayScale)", src_img);
	cvShowImage("Sobel", sobel_img);
	cvShowImage("Laplacian", laplaian_img);
	cvShowImage("Canny", canny_img);
	cvWaitKey(0);

	cvDestroyWindow("Original");
	cvDestroyWindow("Sobel");
	cvDestroyWindow("Laplace");
	cvDestroyWindow("Canny");
	cvReleaseImage(&src_img);
	cvReleaseImage(&sobel_img);
	cvReleaseImage(&laplaian_img);
	cvReleaseImage(&canny_img);
	cvReleaseImage(&tmp_img);

	return 0;
}
*/

/*
######################
カメラキャプチャ & 加工
######################
基本的にこっち
操作方法：
Dキー:デフォルト
Cキー:コーナー検出
	Cキー押してから･･･
	1キー:EigenValue法(エッジ候補上位数十位のものをプロットする適当なやつなので、実用的でない)
	2キー:Harris法(上に同じ)
	3キー:FAST法(THE コーナー検出、四角形の角とかそこそこ検出できる、上2つと比べて検出数がかなり少ない)
Eキー:エッジ検出
	Eキー押してから･･･
	1キー:Sobel(1階微分)
	2キー:Laplacian(2階微分)
	3キー:Canny(結構正確、ノイズに強い)
Gキー:グレースケール表示
Oキー:オプティカルフロー表示(実 験 中、遅すぎ、何やってるかわからん、今のところ使う意味ナシ)
Qキー:終了

ウインドウが例外無しに一瞬で消える時は [Ctrl+F5] で実行してみよう！

*/


#define WINNAME_DEF	"capture"
#define WINNAME_GRA "gray_scale"
#define WINNAME_SOB	"[Edge]Sobel"
#define WINNAME_LAP	"[Edge]Laplacian"
#define WINNAME_CAN	"[Edge]Canny"
#define WINNAME_EIG	"[Corner]EigenValue"
#define WINNAME_HAR	"[Corner]Harris"
#define WINNAME_FAS	"[Corner]FAST"
#define GRAY		1
#define SOBEL		2
#define LAPLACE		3
#define CANNY		4
#define EIGEN		5
#define HARRIS		6
#define FAST_		7
#define FLOW		8


#include<opencv/cvaux.h>
#include<opencv2/superres/optical_flow.hpp>

using namespace cv;
using namespace cv::superres;

int main() {
	VideoCapture cap(0);
	Mat frame, prev, temp, gray, sobel, laplacian, canny, eigen, harris, fast, hsv, flowBgr;
	Mat flowX, flowY, magnitude, angle, hsvPlanes[3];
	Ptr<DenseOpticalFlowExt> opticalFlow = superres::createOptFlow_DualTVL1();
	vector<Point2f> corners;
	vector<Point2f>::iterator it_corner;
	vector<KeyPoint> keypoints;
	vector<KeyPoint>::iterator it_kp;
	int threshold = 100;
	bool nonmax = true;
	int filenumber = 0;
	char filename[32];
	int flag = 0;
	int key = 0;
	clock_t start = 0, end = 0, diff = 0;
	bool flash = false;

	start = clock();

	if (!cap.isOpened()) {
		fprintf(stderr, "[ERROR]: any camera not detected\nCheck the connection of it.\n");
		return EXIT_FAILURE;
	}

	do cap >> prev; while (prev.empty());
	if (!prev.data) {
		fprintf(stderr, "EMPTY: (Mat)prev\n");
		return EXIT_FAILURE;
	}
	end = clock();
	fprintf(stdout, "|captured(prv):\t%g [s]\n", (float)(end - start) / CLOCKS_PER_SEC);
	while (1) {
		start = clock();
		do cap >> frame; while (frame.empty());
		diff = end;
		end = clock();
		fprintf(stdout, "\n|captured:\t%g [s]\n", (float)(end - diff) / CLOCKS_PER_SEC, (float)(end - start) / CLOCKS_PER_SEC);

		if (!frame.data) {
			fprintf(stderr, "EMPTY: (Mat)frame\n");
			return EXIT_FAILURE;
		}
		eigen = frame.clone();
		harris = frame.clone();
		fast = frame.clone();
		diff = end;
		end = clock();
		fprintf(stdout, "|cloned:\t%g [s]\n", (float)(end - diff) / CLOCKS_PER_SEC, (float)(end - start) / CLOCKS_PER_SEC);
		switch (flag) {
		case 0:
			imshow(WINNAME_DEF, frame);
			diff = end;
			end = clock();
			fprintf(stdout, "|disp[def]:\t%g [s]\n", (float)(end - diff) / CLOCKS_PER_SEC, (float)(end - start) / CLOCKS_PER_SEC);
			break;
		case GRAY:
			cvtColor(frame, gray, CV_BGR2GRAY);
			normalize(gray, gray, 0, 255, NORM_MINMAX);
			imshow(WINNAME_DEF, gray);
			diff = end;
			end = clock();
			fprintf(stdout, "|disp[gry]:\t%g [s]\n", (float)(end - diff) / CLOCKS_PER_SEC, (float)(end - start) / CLOCKS_PER_SEC);
			break;
		case SOBEL:
			Sobel(frame, temp, CV_32F, 1, 1);
			convertScaleAbs(temp, sobel, 1, 0);
			imshow(WINNAME_DEF, sobel);
			diff = end;
			end = clock();
			fprintf(stdout, "|disp[sbl]:\t%g [s]\n", (float)(end - diff) / CLOCKS_PER_SEC, (float)(end - start) / CLOCKS_PER_SEC);
			break;
		case LAPLACE:
			Laplacian(frame, temp, CV_32F, 3);
			convertScaleAbs(temp, laplacian, 1, 0);
			imshow(WINNAME_DEF, laplacian);
			diff = end;
			end = clock();
			fprintf(stdout, "|disp[lpl]:\t%g [s]\n", (float)(end - diff) / CLOCKS_PER_SEC, (float)(end - start) / CLOCKS_PER_SEC);
			break;
		case CANNY:
			Canny(frame, canny, 50, 200);
			imshow(WINNAME_DEF, canny);
			diff = end;
			end = clock();
			fprintf(stdout, "|disp[cny]:\t%g [s]\n", (float)(end - diff) / CLOCKS_PER_SEC, (float)(end - start) / CLOCKS_PER_SEC);
			break;
		case EIGEN:
			cvtColor(frame, gray, CV_BGR2GRAY);
			normalize(gray, gray, 0, 255, NORM_MINMAX);
			goodFeaturesToTrack(gray, corners, 80, 0.01, 5);
			it_corner = corners.begin();
			for (; it_corner != corners.end(); ++it_corner) {
				circle(eigen, Point(it_corner->x, it_corner->y), 1, Scalar(0, 200, 255), -1);
				circle(eigen, Point(it_corner->x, it_corner->y), 8, Scalar(0, 200, 255));
			}
			imshow(WINNAME_DEF, eigen);
			diff = end;
			end = clock();
			fprintf(stdout, "|disp[egn]:\t%g [s]\n", (float)(end - diff) / CLOCKS_PER_SEC, (float)(end - start) / CLOCKS_PER_SEC);
			break;
		case HARRIS:
			cvtColor(frame, gray, CV_BGR2GRAY);
			normalize(gray, gray, 0, 255, NORM_MINMAX);
			goodFeaturesToTrack(gray, corners, 80, 0.01, 3, Mat(), 3, true);
			it_corner = corners.begin();
			for (; it_corner != corners.end(); ++it_corner) {
				circle(harris, Point(it_corner->x, it_corner->y), 1, Scalar(0, 255, 0), -1);
				circle(harris, Point(it_corner->x, it_corner->y), 8, Scalar(0, 255, 0));
			}
			imshow(WINNAME_DEF, harris);
			diff = end;
			end = clock();
			fprintf(stdout, "|disp[hrs]:\t%g [s]\n", (float)(end - diff) / CLOCKS_PER_SEC, (float)(end - start) / CLOCKS_PER_SEC);
			break;
		case FAST_:
			cvtColor(frame, gray, CV_BGR2GRAY);
			normalize(gray, gray, 0, 255, NORM_MINMAX);
			FAST(gray, keypoints, threshold, nonmax);
			it_kp = keypoints.begin();
			for (; it_kp != keypoints.end(); ++it_kp) {
				circle(fast, Point(it_kp->pt.x, it_kp->pt.y), 1, Scalar(50, 0, 255), -1);
				circle(fast, Point(it_kp->pt.x, it_kp->pt.y), 8, Scalar(50, 0, 255));
			}
			imshow(WINNAME_DEF, fast);
			diff = end;
			end = clock();
			fprintf(stdout, "|disp[fst]:\t%g [s]\n", (float)(end - diff) / CLOCKS_PER_SEC, (float)(end - start) / CLOCKS_PER_SEC);
			break;
		case FLOW:
			opticalFlow->calc(prev, frame, flowX, flowY);
			cartToPolar(flowX, flowY, magnitude, angle, true);
			hsvPlanes[0] = angle;
			normalize(magnitude, magnitude, 0, 1, NORM_MINMAX);
			hsvPlanes[1] = magnitude;
			hsvPlanes[2] = Mat::ones(magnitude.size(), CV_32F);
			merge(hsvPlanes, 3, hsv);
			cvtColor(hsv, flowBgr, cv::COLOR_HSV2BGR);
			imshow(WINNAME_DEF, frame);
			imshow(WINNAME_DEF, flowBgr);
			diff = end;
			end = clock();
			fprintf(stdout, "|disp[flw]:\t%g [s]\n", (float)(end - diff) / CLOCKS_PER_SEC, (float)(end - start) / CLOCKS_PER_SEC);
			break;
		default:
			fprintf(stderr, "????????\n");
			return EXIT_FAILURE;
		}
		prev = frame;
		key = waitKey(1);
		if (isalpha(key))key = toupper(key);
		switch (key) {
		case 'D':
			if (flag != 0) {
				flag = 0;
				printf("[SET]default #%d\n", flag);
			}
			continue;
		case 'G':
			if (flag != GRAY) {
				flag = GRAY;
				printf("[SET]gray scale #%d\n", flag);
			}
			continue;
		case 'E':
			if (flag != SOBEL) {
				flag = SOBEL;
				printf("[SET]Edge mode : Sobel #%d\n", flag);
			}
			continue;
		case 'C':
			if (flag != EIGEN) {
				flag = EIGEN;
				printf("[SET]Corner mode : EigenValue #%d\n", flag);
			}
			continue;
		case 'O':
			if (flag != FLOW) {
				flag = FLOW;
				printf("[SET]Optical Flow mode #%d\n", flag);
			}
			continue;
		case '1':
			if (flag == LAPLACE || flag == CANNY) {
				flag = SOBEL;
				printf("[SET]Edge mode : Sobel #%d\n", flag);
			}
			else if (flag == HARRIS || flag == FAST_) {
				flag = EIGEN;
				printf("[SET]Corner mode : EigenValue #%d\n", flag);
			}
			continue;
		case '2':
			if (flag == SOBEL || flag == CANNY) {
				flag = LAPLACE;
				printf("[SET]Edge mode : Laplasian #%d\n", flag);
			}
			else if (flag == EIGEN || flag == FAST_) {
				flag = HARRIS;
				printf("[SET]Corner mode : Harris #%d\n", flag);
			}
			continue;
		case '3':
			if (flag == SOBEL || flag == LAPLACE) {
				flag = CANNY;
				printf("[SET]Edge mode : Canny #%d\n", flag);
			}
			else if (flag == EIGEN || flag == HARRIS) {
				flag = FAST_;
				printf("[SET]Corner mode : FAST #%d\n", flag);
			}
			continue;
		case 'S':
			switch (flag) {
			case 0:
				sprintf_s(filename, sizeof(filename), "image%d.png", filenumber++);
				cv::imwrite(filename, frame);
				break;
			case GRAY:
				sprintf_s(filename, sizeof(filename), "image%d_g.png", filenumber++);
				imwrite(filename, gray);
				break;
			case SOBEL:
				sprintf_s(filename, sizeof(filename), "image%d_s.png", filenumber++);
				imwrite(filename, sobel);
				break;
			case LAPLACE:
				sprintf_s(filename, sizeof(filename), "image%d_l.png", filenumber++);
				imwrite(filename, laplacian);
				break;
			case CANNY:
				sprintf_s(filename, sizeof(filename), "image%d_c.png", filenumber++);
				imwrite(filename, canny);
				break;
			case EIGEN:
				sprintf_s(filename, sizeof(filename), "image%d_e.png", filenumber++);
				imwrite(filename, eigen);
				break;
			case HARRIS:
				sprintf_s(filename, sizeof(filename), "image%d_h.png", filenumber++);
				imwrite(filename, harris);
				break;
			case FAST_:
				sprintf_s(filename, sizeof(filename), "image%d_f.png", filenumber++);
				imwrite(filename, fast);
				break;
			case FLOW:
				sprintf_s(filename, sizeof(filename), "image%d_o.png", filenumber++);
				imwrite(filename, flowBgr);
				break;
			}
			printf("SAVED the image as %s.\n", filename);
			end = clock();
			fprintf(stdout, "|saved:\t%g [s]\n", (float)(end - diff) / CLOCKS_PER_SEC, (float)(end - start) / CLOCKS_PER_SEC);
			continue;
		case 'Q':
			printf("QUIT...\n");
			destroyAllWindows();
			return EXIT_SUCCESS;
		case -1: continue;
		default:
			if (!(key < -1 || key>200)) {
				printf("undefined key:[%c]\n", key);
				continue;
			}
			fprintf(stderr, "unknown key detected:[%d]\n", key);
			return -1;
		}
	}
}

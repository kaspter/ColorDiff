#ifndef FINDSQUARES_H_
#define FINDSQUARES_H_

#include "opencv2/highgui.hpp"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgproc/imgproc_c.h>

using namespace cv;
using namespace std;

typedef struct {
    cv::Rect rect;
    int countInRange;
} rectPointType;

void findSquares(const Mat &image, vector<vector<Point>> &squares, float minArea, float maxArea);

int findRects(const Mat &image, vector<rectPointType> &Rects, int imgType);

void drawSquares(Mat &image, const vector<vector<Point>> &squares);

void drawRects(Mat &image, vector<rectPointType> &vecRect);

void drawAllCenter(Mat &image, vector<rectPointType> &vecRect);

void drawRectsCenter(Mat &image, rectPointType &rectPoint);

void sortSquares(vector<vector<Point>> &squares, vector<rectPointType> &vecRect, int imgType);

int findAllRectCenter(Mat &image, vector<rectPointType> &vecRect, vector<Point> &Points);

#endif /* GPICK_COLOR_H_ */

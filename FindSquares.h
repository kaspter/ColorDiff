#ifndef FINDSQUARES_H_
#define FINDSQUARES_H_


#include "opencv2/highgui.hpp"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgproc/imgproc_c.h>

using namespace cv;
using namespace std;



typedef struct
{
    cv::Rect rect;
    int countInRange;
} rectPointType;


void findSquares( const Mat& image, vector<vector<Point> >& squares );

void drawSquares( Mat& image, const vector<vector<Point> >& squares);

void drawRects( Mat& image, vector<rectPointType>& vecRect);

void sortSquares(vector<vector<Point> >& squares, vector<rectPointType>& vecRect);

#endif /* GPICK_COLOR_H_ */

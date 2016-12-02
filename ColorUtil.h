#ifndef COLORUTIL_H_
#define COLORUTIL_H_


#include "opencv2/highgui.hpp"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgproc/imgproc_c.h>

using namespace cv;
using namespace std;


int color_pick(Mat &src, int x, int y);

int color_pick_left(Mat &src, int x, int y);

int color_pick_right(Mat &src, int x, int y);

#endif /* GPICK_COLOR_H_ */

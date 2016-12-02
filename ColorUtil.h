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

double ColourDiff(const Color *a, const Color *b);

float DeltaE2000(const Color *Lab1, const Color *Lab2);

#endif /* GPICK_COLOR_H_ */

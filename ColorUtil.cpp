#include "opencv2/xphoto.hpp"
#include "opencv2/highgui.hpp"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgproc/imgproc_c.h>


#include "Color.h"



using namespace cv;
using namespace std;





int color_pick(Mat &src, int x, int y)
{
    int cPointR,cPointG,cPointB;

    //注意坐标和像素点存放位置关系
    cPointB = src.at<Vec3b>(y,x)[0];
    cPointG = src.at<Vec3b>(y,x)[1];
    cPointR = src.at<Vec3b>(y,x)[2];

    Color rgb, hsv, hsl;
    color_set(&rgb, cPointR, cPointG, cPointB);
    color_rgb_to_hsv(&rgb, &hsv);
    color_rgb_to_hsl(&rgb, &hsl);

    printf("x=%d, y=%d, #%02X%02X%02X ", x, y, cPointR,cPointG,cPointB);
    printf("h=%f, s=%f, v=%f ", hsv.hsv.hue, hsv.hsv.saturation, hsv.hsv.value);
    printf("h=%f, s=%f, l=%f\n", hsl.hsl.hue, hsl.hsl.saturation, hsl.hsl.lightness);
    return 0;
} 


int color_pick_left(Mat &src, int x, int y)
{
    int cPointR,cPointG,cPointB;

    char label[128];

    cPointB = src.at<Vec3b>(y,x)[0];
    cPointG = src.at<Vec3b>(y,x)[1];
    cPointR = src.at<Vec3b>(y,x)[2];

    Color rgb, hsv;
    color_set(&rgb, cPointR, cPointG, cPointB);
    color_rgb_to_hsv(&rgb, &hsv);

    sprintf(label,"[#%X%X%X]-[%d,%d,%d]",cPointR,cPointG,cPointB, (int)(hsv.hsv.hue *100), (int)(hsv.hsv.saturation*100), (int)(hsv.hsv.value*100));

    putText(src, label, cvPoint(x,y), FONT_HERSHEY_PLAIN, 1.0, Scalar(255, 255, 255, 0), 2);

    char mg[64];

    sprintf(mg,"[%f]",hsv.hsv.hue);

    putText(src, mg, cvPoint(x + 12,y + 12), FONT_HERSHEY_PLAIN, 1.0,Scalar(255, 255, 255, 0), 2);

    return 0;
}

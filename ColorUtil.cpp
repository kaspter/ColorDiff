#include "opencv2/xphoto.hpp"
#include "opencv2/highgui.hpp"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgproc/imgproc_c.h>


#include "Color.h"
#include "ColorUtil.h"


using namespace cv;
using namespace std;



Color rgb1, rgb2;
Color lab1, lab2;


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

    //printf("x=%d, y=%d, #%02X%02X%02X ", x, y, cPointR,cPointG,cPointB);
    //printf("h=%f, s=%f, v=%f ", hsv.hsv.hue, hsv.hsv.saturation, hsv.hsv.value);
    //printf("h=%f, s=%f, l=%f\n", hsl.hsl.hue, hsl.hsl.saturation, hsl.hsl.lightness);
    return 0;
} 


int color_pick_left(Mat &src, int x, int y)
{
    int cPointR,cPointG,cPointB;

    char label[128];

    cPointB = src.at<Vec3b>(y,x)[0];
    cPointG = src.at<Vec3b>(y,x)[1];
    cPointR = src.at<Vec3b>(y,x)[2];

    Color hsv;
    color_set(&rgb1, cPointR, cPointG, cPointB);

    color_rgb_to_lab_d50(&rgb1,&lab1);

    //color_rgb_to_hsv(&rgb1, &hsv);
    //sprintf(label,"[#%X%X%X]-[%d,%d,%d]",cPointR,cPointG,cPointB,
    //        (int)(hsv.hsv.hue *100), (int)(hsv.hsv.saturation*100), (int)(hsv.hsv.value*100));

    //putText(src, label, cvPoint(x,y), FONT_HERSHEY_PLAIN, 1.0, Scalar(255, 255, 255, 0), 2);

    //char mg[64];

    //sprintf(mg,"[%f]",hsv.hsv.hue);

    //putText(src, mg, cvPoint(x + 12,y + 12), FONT_HERSHEY_PLAIN, 1.0,Scalar(255, 255, 255, 0), 2);

    return 0;
}


int color_pick_right(Mat &src, int x, int y)
{
    int cPointR,cPointG,cPointB;

    char label[128];

    cPointB = src.at<Vec3b>(y,x)[0];
    cPointG = src.at<Vec3b>(y,x)[1];
    cPointR = src.at<Vec3b>(y,x)[2];

    Color hsv;
    color_set(&rgb2, cPointR, cPointG, cPointB);

    float diff_rgb = color_distance(&rgb1, &rgb2);
    double diff_rgb2 = ColourDiff(&rgb1, &rgb2);

    printf("[%f-%f]-", diff_rgb,diff_rgb2);


    color_rgb_to_lab_d50(&rgb2,&lab2);

    float diff_lab = color_distance_lch(&lab1, &lab2);
    float diff_lab2k = DeltaE2000(&lab1, &lab2);
    printf("[%f-%f]\n", diff_lab,diff_lab2k);

    //color_rgb_to_hsv(&rgb2, &hsv);
    //sprintf(label,"[#%X%X%X]-[%d,%d,%d]",cPointR,cPointG,cPointB,
    //        (int)(hsv.hsv.hue *100), (int)(hsv.hsv.saturation*100), (int)(hsv.hsv.value*100));
    //putText(src, label, cvPoint(x,y), FONT_HERSHEY_PLAIN, 1.0, Scalar(255, 255, 255, 0), 2);

    char mg[64];

    sprintf(mg,"[%.1f]",diff_lab2k);

    //putText(src, mg, cvPoint(x,y), FONT_HERSHEY_PLAIN, 1.0,Scalar(255, 255, 255, 0), 2);
    //putText(src, mg, cvPoint(x + 12,y + 12), FONT_HERSHEY_PLAIN, 1.0,Scalar(255, 255, 255, 0), 2);

    return 0;
}


//http://www.compuphase.com/cmetric.htm
double ColourDiff(const Color *a, const Color *b)
{
  long rmean = ( (long)a->rgb.red + (long)b->rgb.red ) / 2;
  long r = (long)a->rgb.red - (long)b->rgb.red;
  long g = (long)a->rgb.green - (long)b->rgb.green;
  long db = (long)a->rgb.blue - (long)b->rgb.blue;
  return sqrt((((512+rmean)*r*r)>>8) + 4*g*g + (((767-rmean)*db*db)>>8));
}



float color_diff(Mat &src, Point & pt1, Point & pt2)
{
    Color rgb1, rgb2;
    Color lab1, lab2;

    int cPointR,cPointG,cPointB;

    int x1 = pt1.x;
    int y1 = pt1.y;
    int x2 = pt2.x;
    int y2 = pt2.y;

    cPointB = src.at<Vec3b>(y1,x1)[0];
    cPointG = src.at<Vec3b>(y1,x1)[1];
    cPointR = src.at<Vec3b>(y1,x1)[2];
    color_set(&rgb1, cPointR, cPointG, cPointB);

    cPointB = src.at<Vec3b>(y2,x2)[0];
    cPointG = src.at<Vec3b>(y2,x2)[1];
    cPointR = src.at<Vec3b>(y2,x2)[2];
    color_set(&rgb2, cPointR, cPointG, cPointB);

    color_rgb_to_lab_d50(&rgb1,&lab1);
    color_rgb_to_lab_d50(&rgb2,&lab2);

    float cie94 = color_distance_lch(&lab1, &lab2);
    float cie2k = DeltaE2000(&lab1, &lab2);

    printf("[%f-%f]\n", cie94,cie2k);

    return cie2k;
}



 //
 //  deltae2000.c
 //
 //  Translated by Dr Cube on 10/1/16.
 //  Translated to C from this javascript code written by Bruce LindBloom:
 //    http://www.brucelindbloom.com/index.html?Eqn_DeltaE_CIE2000.html
 //    http://www.brucelindbloom.com/javascript/ColorDiff.js

// function expects Lab where: 0 >= L <=100.0 , -100 >=a <= 100.0  and  -100 >= b <= 100.0

float DeltaE2000(const Color *Lab1, const Color *Lab2)
{
    float dhPrime; // not initialized on purpose

    float kL = 1.0;
    float kC = 1.0;
    float kH = 1.0;

    float lBarPrime = 0.5 * (Lab1->lab.L + Lab2->lab.L);

    float c1 = sqrtf(Lab1->lab.a * Lab1->lab.a + Lab1->lab.b * Lab1->lab.b);
    float c2 = sqrtf(Lab2->lab.a * Lab2->lab.a + Lab2->lab.b * Lab2->lab.b);
    float cBar = 0.5 * (c1 + c2);

    float cBar7 = cBar * cBar * cBar * cBar * cBar * cBar * cBar;
    float g = 0.5 * (1.0 - sqrtf(cBar7 / (cBar7 + 6103515625.0)));  /* 6103515625 = 25^7 */

    float a1Prime = Lab1->lab.a * (1.0 + g);
    float a2Prime = Lab2->lab.a * (1.0 + g);

    float c1Prime = sqrtf(a1Prime * a1Prime + Lab1->lab.b * Lab1->lab.b);
    float c2Prime = sqrtf(a2Prime * a2Prime + Lab2->lab.b * Lab2->lab.b);
    float cBarPrime = 0.5 * (c1Prime + c2Prime);

    float h1Prime = (atan2f(Lab1->lab.b, a1Prime) * 180.0) / M_PI;


    if (h1Prime < 0.0)
       h1Prime += 360.0;
    float h2Prime = (atan2f(Lab2->lab.b, a2Prime) * 180.0) / M_PI;
    if (h2Prime < 0.0)
       h2Prime += 360.0;
    float hBarPrime = (fabsf(h1Prime - h2Prime) > 180.0) ? (0.5 * (h1Prime + h2Prime + 360.0)) : (0.5 * (h1Prime + h2Prime));
    float t = 1.0 -
                    0.17 * cosf(M_PI * (      hBarPrime - 30.0) / 180.0) +
                    0.24 * cosf(M_PI * (2.0 * hBarPrime       ) / 180.0) +
                    0.32 * cosf(M_PI * (3.0 * hBarPrime +  6.0) / 180.0) -
                    0.20 * cosf(M_PI * (4.0 * hBarPrime - 63.0) / 180.0);
    if (fabsf(h2Prime - h1Prime) <= 180.0)
       dhPrime = h2Prime - h1Prime;
    else
       dhPrime = (h2Prime <= h1Prime) ? (h2Prime - h1Prime + 360.0) : (h2Prime - h1Prime - 360.0);

    float dLPrime = Lab2->lab.L - Lab1->lab.L;
    float dCPrime = c2Prime - c1Prime;
    float dHPrime = 2.0 * sqrtf(c1Prime * c2Prime) * sinf(M_PI * (0.5 * dhPrime) / 180.0);

    float sL = 1.0 + ((0.015 * (lBarPrime - 50.0) * (lBarPrime - 50.0)) / sqrtf(20.0 + (lBarPrime - 50.0) * (lBarPrime - 50.0)));
    float sC = 1.0 + 0.045 * cBarPrime;
    float sH = 1.0 + 0.015 * cBarPrime * t;

    float dTheta = 30.0 * expf(-((hBarPrime - 275.0) / 25.0) * ((hBarPrime - 275.0) / 25.0));
    float cBarPrime7 = cBarPrime * cBarPrime * cBarPrime * cBarPrime * cBarPrime * cBarPrime * cBarPrime;

    float rC = sqrtf(cBarPrime7 / (cBarPrime7 + 6103515625.0));
    float rT = -2.0 * rC * sinf(M_PI * (2.0 * dTheta) / 180.0);

    return  (sqrtf((dLPrime / (kL * sL)) * (dLPrime / (kL * sL)) +
                   (dCPrime / (kC * sC)) * (dCPrime / (kC * sC)) +
                   (dHPrime / (kH * sH)) * (dHPrime / (kH * sH)) +
                   (dCPrime / (kC * sC)) * (dHPrime / (kH * sH)) * rT));
}

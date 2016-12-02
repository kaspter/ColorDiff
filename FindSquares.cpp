#include "opencv2/xphoto.hpp"
#include "opencv2/highgui.hpp"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgproc/imgproc_c.h>


#include "Color.h"
#include "ColorUtil.h"
#include "FindSquares.h"

using namespace cv;
using namespace std;






// finds a cosine of angle between vectors from pt0->pt1 and from pt0->pt2
double angle( Point pt1, Point pt2, Point pt0 ) {
    double dx1 = pt1.x - pt0.x;
    double dy1 = pt1.y - pt0.y;
    double dx2 = pt2.x - pt0.x;
    double dy2 = pt2.y - pt0.y;

    double ratio;//边长平方的比

    ratio=(dx1*dx1+dy1*dy1)/(dx2*dx2+dy2*dy2);
    //根据边长平方的比过小或过大提前淘汰这个四边形，如果淘汰过多，调整此比例数字
    if (ratio<0.8 || 1.2<ratio) {
        return 1.0;
    }

    return (dx1*dx2 + dy1*dy2)/sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
}


void findSquares( const Mat& image, vector<vector<Point> >& squares )
{
    squares.clear();

    //二值化
    Mat thresh;
    threshold(image, thresh, 175, 255, CV_THRESH_BINARY);

    // blur will enhance edge detection
    Mat blurred(thresh);
    medianBlur(thresh, blurred, 9);

    Mat gray0(blurred.size(), CV_8U), gray;

    vector<vector<Point> > contours;

    // find squares in every color plane of the image
    for (int c = 0; c < 3; c++) {
        int ch[] = {c, 0};
        mixChannels(&blurred, 1, &gray0, 1, ch, 1);

        // try several threshold levels
        const int threshold_level = 2;
        for (int l = 0; l < threshold_level; l++) {
            // Use Canny instead of zero threshold level!
            // Canny helps to catch squares with gradient shading
            if (l == 0) {
                Canny(gray0, gray, 10, 20, 3); //
                //Canny(gray0, gray, 0, 50, 5); //
                // Dilate helps to remove potential holes between edge segments
                dilate(gray, gray, Mat(), Point(-1,-1));
            } else {
                    gray = gray0 >= (l+1) * 255 / threshold_level;
            }

            // Find contours and store them in a list
            findContours(gray, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);

            // Test contours
            vector<Point> approx;
            for (size_t i = 0; i < contours.size(); i++) {
                    // approximate contour with accuracy proportional
                    // to the contour perimeter
                    approxPolyDP(Mat(contours[i]), approx, arcLength(Mat(contours[i]), true)*0.02, true);

                    // square contours should have 4 vertices after approximation
                    // relatively large area (to filter out noisy contours)
                    // and be convex.
                    // Note: absolute value of an area is used because
                    // area may be positive or negative - in accordance with the
                    // contour orientation
                    if (approx.size() == 4 && isContourConvex(Mat(approx))) {

                            double area;
                            area = fabs(contourArea(Mat(approx)));
                            //printf("area = %lg\n", area);
                            ////当正方形面积在此范围内……，如果有因面积过大或过小漏检正方形问题，调整此范围。
                            if (6000.0 < area && area < 443000.0) {

                                printf("find area = %lg\n", area);

                                double maxCosine = 0;

                                for (int j = 2; j < 5; j++) {
                                    // find the maximum cosine of the angle between joint edges
                                    double cosine = fabs(angle(approx[j%4], approx[j-2], approx[j-1]));
                                    maxCosine = MAX(maxCosine, cosine);
                                }

                                //四个角和直角相比的最大误差，可根据实际情况略作调整，越小越严格
                                if (maxCosine < 0.3)
                                    squares.push_back(approx);
                            }
                    }
            }
        }
    }
}

// the function draws all the squares in the image
void drawSquares( Mat& image, const vector<vector<Point> >& squares )
{
    for( size_t i = 0; i < squares.size(); i++ ) {
        const Point* p = &squares[i][0];
        int n = (int)squares[i].size();
        polylines(image, &p, &n, 1, true, Scalar(0,255,0), 1, CV_AA);
    }
}

// the function draws all the squares in the image
void drawRects( Mat& image, vector<rectPointType>& vecRect )
{
    for( size_t i = 0; i < vecRect.size(); i++ ) {
        rectPointType rectPoint = vecRect.at(i);
        Rect rect = rectPoint.rect;

        rectangle(image,rect,Scalar(0,0,255),1,8,0);//用矩形画矩形窗

        printf("rect: (%d-%d-%d-%d)\n", rect.x, rect.y, rect.width, rect.height);
        //取正方形中心
        Point center(rect.x + rect.width/2, rect.y + rect.height/2);
        circle(image, center, 1, Scalar(0, 255, 0), -1, 8, 0);
    }
}


bool sortFun(const rectPointType& r1, const rectPointType& r2)
{
    return (r1.countInRange > r2.countInRange);
}


void sortSquares(vector<vector<Point> >& squares, vector<rectPointType>& vecRect)
{
    //取正方形中心
    for( size_t i = 0; i < squares.size(); i++ ) {

        Rect rect = boundingRect(Mat(squares[i]));

        rectPointType rectPoint;
        rectPoint.rect = rect;
        vecRect.push_back(rectPoint);
    }


    for (int index = 0; index < vecRect.size(); index++) {
        rectPointType now_rect = vecRect.at(index);
        int countInRange = 0;

        double now_area = now_rect.rect.width * now_rect.rect.height;

        for (int i = 0; i < vecRect.size(); i++) {

            double area = vecRect.at(i).rect.width * vecRect.at(i).rect.height;
            //printf("index =%d, area = %.2f\n",index, area);
            if (now_area > area) {
                countInRange++;
            }
        }
        vecRect.at(index).countInRange = countInRange;
    }


    sort(vecRect.begin(), vecRect.end(), sortFun);
}

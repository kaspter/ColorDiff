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


void findSquares( const Mat& image, vector<vector<Point> >& squares, float minArea, float maxArea)
{
    squares.clear();

    //二值化
    //Mat thresh;
    //threshold(image, thresh, 138, 255, CV_THRESH_BINARY);
    //namedWindow("thresh", 1);
    //imshow("thresh", thresh);


    // blur will enhance edge detection
    Mat blurred(image);
    medianBlur(image, blurred, 9);

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
                Canny(gray0, gray, 180, 250, 5); //
                //Canny(gray0, gray, 0, 50, 5); //

                //namedWindow("canny", 1);
                //imshow("canny", gray);



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
                            if (minArea < area && area < maxArea) {



                                double maxCosine = 0;

                                for (int j = 2; j < 5; j++) {
                                    // find the maximum cosine of the angle between joint edges
                                    double cosine = fabs(angle(approx[j%4], approx[j-2], approx[j-1]));
                                    maxCosine = MAX(maxCosine, cosine);
                                }


                                printf("find area = %lg\t%f\n", area, maxCosine);

                                //四个角和直角相比的最大误差，可根据实际情况略作调整，越小越严格
                                if (maxCosine < 0.2)
                                    squares.push_back(approx);
                            }
                    }
            }
        }
    }
}


int findRects( const Mat& image, vector<rectPointType>& Rects, int imgType)
{
    Rects.clear();
    vector<vector<Point> > squares;

    //自动拍照
    if (imgType > 0) {
        findSquares(image, squares, 7500.0, 10000.0);
    } else {
        //手动拍照
        findSquares(image, squares, 14000.0, 900000.0);
    }

    //构造最大巨型并排序
    sortSquares(squares, Rects, imgType);

    //判定矩形面积
    if (Rects.size() > 0) {
        rectPointType rectPoint = Rects.at(0);
        Rect maxRect = rectPoint.rect;
        if (maxRect.width * maxRect.height < 960 * 960 / 2) {
            printf("--------- maxRect: area failed\n");
            return -1;
        }
    }

    return Rects.size();
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
        circle(image, center, 1, Scalar(0, 0, 255), -1, 8, 0);
    }
}




void drawRectCenter( Mat& image, Rect & maxRect)
{
    for (size_t i = 0; i < 14; i++) {
        Point center;
        center.x = maxRect.x + i * maxRect.width / 14 +  maxRect.width / 28;
        for (size_t j = 0; j < 14; j++) {
            center.y = maxRect.y + j * maxRect.height / 14 +  maxRect.height / 28;
            circle(image, center, 1, Scalar(0, 255, 0), -1, 8, 0);
        }
    }
}


void drawAllCenter( Mat& image, vector<rectPointType>& vecRect )
{
    for( size_t index = 0; index < vecRect.size(); index++ ) {

        rectPointType rectPoint = vecRect.at(index);

        Rect maxRect = rectPoint.rect;
        //用矩形画最大矩形
        rectangle(image, maxRect,Scalar(0,0,255),1,8,0);
        printf("maxRect: (%d-%d-%d-%d)\n", maxRect.x, maxRect.y, maxRect.width, maxRect.height);
        //
        drawRectCenter(image, maxRect);
        if (index == 0)
            break;
    }
}



int findRectCenter( Mat& image, Rect & maxRect, vector<Point>& Points)
{
    for (size_t i = 0; i < 14; i++) {
        Point center;
        center.x = maxRect.x + i * maxRect.width / 14 +  maxRect.width / 28;
        for (size_t j = 0; j < 14; j++) {
            center.y = maxRect.y + j * maxRect.height / 14 +  maxRect.height / 28;
            Points.push_back(center);
        }
    }

    return Points.size();
}



int findAllRectCenter( Mat& image, vector<rectPointType>& vecRect, vector<Point>& Points)
{
    int num = 0;
    for( size_t index = 0; index < vecRect.size(); index++ ) {

        rectPointType rectPoint = vecRect.at(index);

        Rect maxRect = rectPoint.rect;

        printf("maxRect: (%d-%d-%d-%d)\n", maxRect.x, maxRect.y, maxRect.width, maxRect.height);
        //
        num = findRectCenter(image, maxRect, Points);
        if (index == 0)
            break;
    }
    return num;
}






bool sortFun(const rectPointType& r1, const rectPointType& r2)
{
    return (r1.countInRange > r2.countInRange);
}


void sortSquares(vector<vector<Point> >& squares, vector<rectPointType>& vecRect, int imgType)
{

    rectPointType MaxRectPoint;
    vector<Point> points;

    //将点转换成矩形
    for( size_t index = 0; index < squares.size(); index++ ) {

        Rect rect = boundingRect(Mat(squares[index]));

        if (imgType > 0) {
            printf("XXXXXXXXX Rect: (%d-%d-%d-%d)\n", rect.x, rect.y, rect.width, rect.height);
            Point pt1(cvRound(rect.x), cvRound(rect.y));
            points.push_back(pt1);

            Point pt2(cvRound(rect.x + rect.width), rect.y);
            points.push_back(pt2);

            Point pt3(rect.x, rect.y + rect.height);
            points.push_back(pt3);
        }

        rectPointType rectPoint;
        rectPoint.rect = rect;
        vecRect.push_back(rectPoint);
    }

    //最大外包矩形
    if (imgType > 0) {
        RotatedRect box = minAreaRect(Mat(points));
        MaxRectPoint.rect = box.boundingRect();
        vecRect.push_back(MaxRectPoint);
    }

    //计算正方形面积
    for (size_t i = 0; i < vecRect.size(); i++) {
        rectPointType now_rect = vecRect.at(i);
        int countInRange = 0;

        double now_area = now_rect.rect.width * now_rect.rect.height;

        for (size_t j = 0; j < vecRect.size(); j++) {

            double area = vecRect.at(j).rect.width * vecRect.at(j).rect.height;
            //printf("index =%d, area = %.2f\n",index, area);
            if (now_area > area) {
                countInRange++;
            }
        }
        vecRect.at(i).countInRange = countInRange;
    }

    if (vecRect.size() > 0)
        sort(vecRect.begin(), vecRect.end(), sortFun);
}

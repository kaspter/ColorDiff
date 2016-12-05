#include "opencv2/xphoto.hpp"
#include "opencv2/highgui.hpp"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgproc/imgproc_c.h>


#include "Color.h"
#include "ColorUtil.h"
#include "HoughCircle.h"
#include "FindSquares.h"

using namespace cv;
using namespace std;

const char *keys = { "{help h usage ? |         | print this message}"
                     "{i              |         | input image name  }"
                     "{a              |grayworld| color balance algorithm (simple, grayworld or learning_based)}"
                     "{m              |         | path to the model for the learning-based algorithm (optional) }" };





static const char * hcoName = "final CircleDetector";




void onMouseMove(int event, int x, int y, int flags, void *ustc)
{
    Mat &src = *(Mat*)ustc;

    color_pick(src, x, y);

    switch (event) {
    case CV_EVENT_LBUTTONDOWN:
        color_pick_left(src, x, y);
        imshow(hcoName, src);
        break;
    case CV_EVENT_LBUTTONUP:
        break;
    case CV_EVENT_RBUTTONDOWN:
        color_pick_right(src, x, y);
        imshow(hcoName, src);
        break;
    }
}


int main(int argc, const char **argv)
{
    CommandLineParser parser(argc, argv, keys);
    parser.about("OpenCV color balance demonstration sample");
    if (parser.has("help") || argc < 2) {
        parser.printMessage();
        return 0;
    }

    string inFilename = parser.get<string>("i");
    string algorithm = parser.get<string>("a");
    string modelFilename = parser.get<string>("m");

    if (!parser.check()) {
        parser.printErrors();
        return -1;
    }

    Mat src = imread(inFilename, 1);
    if (src.empty()) {
        printf("Cannot read image file: %s\n", inFilename.c_str());
        return -1;
    }

    //namedWindow("before color balance", 1);
    //imshow("before color balance", src);

    Mat wbImg;
    Ptr<xphoto::WhiteBalancer> wb;
    if (algorithm == "simple")
        wb = xphoto::createSimpleWB();
    else if (algorithm == "grayworld")
        wb = xphoto::createGrayworldWB();
    else if (algorithm == "learning_based")
        wb = xphoto::createLearningBasedWB(modelFilename);
    else {
        printf("Unsupported algorithm: %s\n", algorithm.c_str());
        return -1;
    }

    //cvResize()，cvPyrDown(),cvPyrSegmentation()
    printf("col = %d\n", src.cols);
    while (src.cols > 1000) {
        Mat small;
        pyrDown(src, small);
        src = small;
    }

    wb->balanceWhite(src, wbImg);

    //namedWindow("after color balance", 1);
    //imshow("after color balance", wbImg);

    color_init();



#if 1

    Mat BigImg;
    Mat LitImg;

#if 0
    CircleDetector bigger(wbImg);
    //2,103,40,20,40,98,10,170,260
    struct EggsDetectorAlgorithmSettings mBigSettings(2,103,40,20,40,98,10,148,240);
    BigImg = bigger.findCircles(mBigSettings);

    namedWindow("first CircleDetector", 1);
    setMouseCallback("first CircleDetector", onMouseMove, &BigImg);
    imshow("first CircleDetector", BigImg);

    struct EggsDetectorAlgorithmSettings mSettings(2,103,7,20,26,35,12,/*86*/87,92);

    CircleDetector smaller(BigImg);
    LitImg = smaller.findCircles(mSettings);
#else

    vector<Vec3f> bCircles;
    struct EggsDetectorAlgorithmSettings mBigSettings(2,103,40,20,40,98,10,148,240);
    int ret = findCircles(wbImg, bCircles, mBigSettings);

    printf("big = %d\n", ret);
    BigImg = drawCircles(wbImg,bCircles);


    vector<Vec3f> lCircles;
    struct EggsDetectorAlgorithmSettings mLitSettings(2,103,7,20,26,35,12,/*86*/87,92);
    ret = findCircles(BigImg, lCircles, mLitSettings);

    printf("lit = %d\n", ret);
    LitImg = drawCircles(BigImg,lCircles);

#endif


    vector<vector<Point> > squares;
    vector<rectPointType> vecRect;

    findSquares(wbImg, squares);

    sortSquares(squares,vecRect);

    //drawSquares(LitImg, squares);
    //drawRects(LitImg, vecRect);
    //drawAllCenter(LitImg, vecRect );

    //int ColorIndex[26] = {  3,   8,   9,  17,  22,  23,  42,  43,  44,  57,  84,  97,  98,
    //                      110, 111, 112, 126, 140, 152, 166, 167, 171, 175, 176, 177, 185};


    int ColorIndex[26] = {   3,   6,   7,   8,   9,  17,  18,  31,  42, 43,  54,   55, 110,
                           112,  113, 124, 126, 127, 138, 175, 177, 178, 179, 188,189, 193};

    vector<Point> vecHPoints;   //HCHO Points
    vector<Point> vecCPoints;   //Color Points


    int num = findAllRectCenter(LitImg, vecRect, vecHPoints);

    for (int index = 0; index < 26; index++) {
        Point center = vecHPoints.at(ColorIndex[index]);
        circle(LitImg, center, 1, Scalar(0, 255, 0), -1, 8, 0);
    }



    CvRNG rng;
    rng= cvRNG(cvGetTickCount());
    printf("=======================\n");
    for (int i= 0;i<20;i++) {
        printf("%d\n",cvRandInt(&rng)%20);//如果%6出来的将会是0~5的正整数
    }
    printf("=======================\n");

    namedWindow(hcoName, 1);
    setMouseCallback(hcoName, onMouseMove, &LitImg);
    imshow(hcoName, LitImg);






    int key = 0;

    while(key != 'q' && key != 'Q') {
	// get user key
	key = waitKey(10);
    }


#else
    EggsDetectorBind bind(wbImg);
    bind.run();
#endif

    return 0;
}

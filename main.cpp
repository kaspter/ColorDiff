#include "opencv2/xphoto.hpp"
#include "opencv2/highgui.hpp"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgproc/imgproc_c.h>


#include "Color.h"
#include "ColorUtil.h"
#include "HoughCircle.h"

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
        imshow("after color balance", src);
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

    Mat res;
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
    while (src.cols > 1000) {
        Mat small;
        pyrDown(src, small);
        src = small;
    }

    wb->balanceWhite(src, res);

    //namedWindow("after color balance", 1);
    //imshow("after color balance", res);

    color_init();

#if 1

    Mat dst;

    CircleDetector bigger(res);
    struct EggsDetectorAlgorithmSettings mBigSettings(2,103,40,20,40,98,10,170,260);
    dst = bigger.findCircles(mBigSettings);

    namedWindow("first CircleDetector", 1);
    setMouseCallback("first CircleDetector", onMouseMove, &dst);
    imshow("first CircleDetector", dst);

    struct EggsDetectorAlgorithmSettings mSettings(2,103,7,20,26,35,16,/*86*/87,92);

    CircleDetector smaller(dst);
    dst = smaller.findCircles(mSettings);

    namedWindow(hcoName, 1);
    setMouseCallback(hcoName, onMouseMove, &dst);
    imshow(hcoName, dst);

    int key = 0;

    while(key != 'q' && key != 'Q') {
	// get user key
	key = waitKey(10);
    }


#else
    EggsDetectorBind bind(res);
    bind.run();
#endif

    return 0;
}

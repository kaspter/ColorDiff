#include "opencv2/xphoto.hpp"
#include "opencv2/highgui.hpp"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgproc/imgproc_c.h>


#include "Color.h"

using namespace cv;
using namespace std;

const char *keys = { "{help h usage ? |         | print this message}"
                     "{i              |         | input image name  }"
                     "{a              |grayworld| color balance algorithm (simple, grayworld or learning_based)}"
                     "{m              |         | path to the model for the learning-based algorithm (optional) }" };



static const char * windowName = "Hough Circle Detection Demo";

static const char * hcoName = "final CircleDetector";


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

    imshow(hcoName, src);
    return 0;
}

void onMouseMove(int event, int x, int y, int flags, void *ustc)
{
    Mat &src = *(Mat*)ustc;

    color_pick(src, x, y);

    switch (event) {
    case CV_EVENT_LBUTTONDOWN:
        color_pick_left(src, x, y);
        break;
    case CV_EVENT_LBUTTONUP:
        break;
    case CV_EVENT_RBUTTONDOWN:
        imshow("after color balance", src);
        break;
    }
}


/**
 * These settings can be adjusted in runtime with OpenCV sliders.
 * Due to slider implementatio (lack of floating-point support) so values
 * are stored scaled by x10 or x100.
 */
struct EggsDetectorAlgorithmSettings
{
    int mSpatialWindowRadius;
    int mColorWindowRadius;
    int mSharpeningWeight; // x100
    int mLaplaccianScale;  // x100
    int mCannyThreshold;
    int mAccumulatorThreshold;
    
    int mHoughResolution;  // x10
    int mMinRadius;
    int mMaxRadius;
    
    EggsDetectorAlgorithmSettings()
        : mSpatialWindowRadius(2)
        , mColorWindowRadius(103)
        , mSharpeningWeight(7)
        , mLaplaccianScale(20)
        , mCannyThreshold(26)
        , mAccumulatorThreshold(35)
        , mHoughResolution(16)
        , mMinRadius(66)
        , mMaxRadius(75)
    {      
    }

    EggsDetectorAlgorithmSettings(int spatialWindowRadius,
				    int colorWindowRadius,
				    int sharpeningWeigh, // x100
				    int laplaccianScale,  // x100
				    int cannyThreshold,
				    int accumulatorThreshold,

				    int houghResolution,  // x10
				    int minRadius,
				    int maxRadius)
        : mSpatialWindowRadius(spatialWindowRadius)
        , mColorWindowRadius(colorWindowRadius)
        , mSharpeningWeight(sharpeningWeigh)
        , mLaplaccianScale(laplaccianScale)
        , mCannyThreshold(cannyThreshold)
        , mAccumulatorThreshold(accumulatorThreshold)
        , mHoughResolution(houghResolution)
        , mMinRadius(minRadius)
        , mMaxRadius(maxRadius)
    {
    }



};

 
class EggsDetectorAlgorithm
{
public:
    
    void process(const Mat_<Vec3b>& inputRgbImage, const EggsDetectorAlgorithmSettings& settings)
    {
        //Step 1 - Filter image
        pyrMeanShiftFiltering(inputRgbImage, filtered, settings.mSpatialWindowRadius, settings.mColorWindowRadius, 1);
        
        //Step 2 - Increase sharpness
        cvtColor(filtered, grayImg, COLOR_BGR2GRAY);
        grayImg.convertTo(grayImgf, CV_32F);
        
        GaussianBlur(grayImgf, blurredf, Size(5,5), 0);
        
        Laplacian(blurredf, laplaccian, CV_32F);
        
        float weight = 0.01f * settings.mSharpeningWeight;
        float scale  = 0.01f * settings.mLaplaccianScale;
 
        Mat_<float> sharpenedf = 1.5f * grayImgf
                                   - 0.5f * blurredf
                                   - weight * grayImgf.mul(scale * laplaccian);
        
        sharpenedf.convertTo(sharpened, CV_8U);
        
        // Step 3 - Detect circles
        HoughCircles(sharpened,
                         circles,
                         CV_HOUGH_GRADIENT,
                         0.1f * settings.mHoughResolution,
                         settings.mMinRadius,
                         settings.mCannyThreshold,
                         settings.mAccumulatorThreshold,
                         settings.mMinRadius,
                         settings.mMaxRadius );

        //Step 4 - Validate eggs
        //TODO

        //
        
    }
    
    Mat display(const Mat& colorImg) const
    {       
        // clone the colour, input image for displaying purposes
        Mat display = colorImg.clone();
        for( size_t i = 0; i < circles.size(); i++ )
        {
            Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
            int radius = cvRound(circles[i][2]);

	    color_pick(display, cvRound(circles[i][0]),cvRound(circles[i][1]));

            printf("center: (%d,%d-%d)\n", cvRound(circles[i][0]),cvRound(circles[i][1]),radius);
            // circle center
            circle( display, center, 3, Scalar(0,255,0), -1, 8, 0 );
            // circle outline
            circle( display, center, radius, Scalar(0,0,255), 3, 8, 0 );
        }
 
        return display;
    }
    
private:
    Mat filtered;
    Mat grayImg;
    Mat grayImgf;
    Mat_<float> blurredf;
    Mat_<float> laplaccian;
    Mat sharpened;
    
    std::vector<Vec3f> circles;
};
 

 
class EggsDetectorBind
{
public:
    
    EggsDetectorBind(const Mat_<Vec3b>& inputRgbImage)
        : mImage(inputRgbImage)
    {
        
    }
    
    void run()
    {
 
        // create the main window, and attach the trackbars
        namedWindow( windowName, WINDOW_AUTOSIZE );
#if 0
        createTrackbar("Spatial window radius", windowName, &mSettings.mSpatialWindowRadius,  100, trackbarPropertyChanged, this);
        createTrackbar("Color window radius",   windowName, &mSettings.mColorWindowRadius,    2000, trackbarPropertyChanged, this);
        createTrackbar("Sharpening weight",     windowName, &mSettings.mSharpeningWeight,     100, trackbarPropertyChanged, this);
        createTrackbar("Laplaccian scale",      windowName, &mSettings.mLaplaccianScale,      100, trackbarPropertyChanged, this);
        createTrackbar("Canny threshold",       windowName, &mSettings.mCannyThreshold,       255, trackbarPropertyChanged, this);
        createTrackbar("Accumulator Threshold", windowName, &mSettings.mAccumulatorThreshold, 100, trackbarPropertyChanged, this);
        createTrackbar("Hough resolution",      windowName, &mSettings.mHoughResolution,      100, trackbarPropertyChanged, this);
        createTrackbar("Min Radius",            windowName, &mSettings.mMinRadius,            1500, trackbarPropertyChanged, this);
        createTrackbar("Max Radius",            windowName, &mSettings.mMaxRadius,            2000, trackbarPropertyChanged, this);
#endif
        display();
 
        // infinite loop to display
        // and refresh the content of the output image
        // until the user presses q or Q
        int key = 0;
        
        while(key != 'q' && key != 'Q')
        {
            // those paramaters cannot be =0 so we must check here
            mSettings.mSpatialWindowRadius  = std::max(mSettings.mSpatialWindowRadius, 1);
            mSettings.mColorWindowRadius    = std::max(mSettings.mColorWindowRadius, 1);
 
            mSettings.mCannyThreshold       = std::max(mSettings.mCannyThreshold, 1);
            mSettings.mAccumulatorThreshold = std::max(mSettings.mAccumulatorThreshold, 1);
 
            mSettings.mHoughResolution      = std::max(mSettings.mHoughResolution, 1);
            mSettings.mMinRadius            = std::max(mSettings.mMinRadius, 1);
            mSettings.mMaxRadius            = std::max(mSettings.mMaxRadius, 1);
 
            // get user key
            key = waitKey(10);
        }
    }
    
protected:
    
    void display()
    {
        mAlgorithm.process(mImage, mSettings);
        imshow(windowName, mAlgorithm.display(mImage));
    }
 
    static void trackbarPropertyChanged(int, void* userdata)
    {
        EggsDetectorBind * self = (EggsDetectorBind *)userdata;
        self->display();
    }
    
private:
    Mat                       mImage;
    EggsDetectorAlgorithmSettings mSettings;
    EggsDetectorAlgorithm         mAlgorithm;
};


#if 1
class CircleDetector
{
public:

    CircleDetector(const Mat_<Vec3b>& inputRgbImage)
        : mImage(inputRgbImage)
    {

    }

    Mat findCircles(const EggsDetectorAlgorithmSettings& settings) {

        mAlgorithm.process(mImage, settings);
	return mAlgorithm.display(mImage);
    }

private:
    Mat                       mImage;
    EggsDetectorAlgorithm         mAlgorithm;
};

#endif


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
    struct EggsDetectorAlgorithmSettings mBigSettings(2,103,40,20,40,98,10,60,153);
    dst = bigger.findCircles(mBigSettings);

    namedWindow("first CircleDetector", 1);
    setMouseCallback("first CircleDetector", onMouseMove, &dst);
    imshow("first CircleDetector", dst);

    struct EggsDetectorAlgorithmSettings mSettings;

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

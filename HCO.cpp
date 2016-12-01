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
                     "{o              |         | output image name }"
                     "{a              |grayworld| color balance algorithm (simple, grayworld or learning_based)}"
                     "{m              |         | path to the model for the learning-based algorithm (optional) }" };






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

    //putText(src, mg, cvPoint(x + 12,y + 12), FONT_HERSHEY_PLAIN, 1.0,Scalar(255, 255, 255, 0), 2);

    imshow("after color balance", src);
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
    int spatialWindowRadius; 
    int colorWindowRadius;
    int sharpeningWeight; // x100
    int laplaccianScale;  // x100
    int cannyThreshold;
    int accumulatorThreshold;
    
    int houghResolution;  // x10
    int minRadius;
    int maxRadius;
    
    EggsDetectorAlgorithmSettings()
        : spatialWindowRadius(6)
        , colorWindowRadius(200)
        , sharpeningWeight(40)
        , laplaccianScale(20)
        , cannyThreshold(40)
        , accumulatorThreshold(70)
        , houghResolution(10)
        , minRadius(60)
        , maxRadius(140)
    {      
    }
};
 
 
class EggsDetectorAlgorithm
{
public:
    
    void process(const Mat_<Vec3b>& inputRgbImage, const EggsDetectorAlgorithmSettings& settings)
    {
        //Step 1 - Filter image
        pyrMeanShiftFiltering(inputRgbImage, filtered, settings.spatialWindowRadius, settings.colorWindowRadius, 1);
        
        //Step 2 - Increase sharpness
        cvtColor(filtered, grayImg, COLOR_BGR2GRAY);
        grayImg.convertTo(grayImgf, CV_32F);
        
        GaussianBlur(grayImgf, blurredf, Size(5,5), 0);
        
        Laplacian(blurredf, laplaccian, CV_32F);
        
        float weight = 0.01f * settings.sharpeningWeight;
        float scale  = 0.01f * settings.laplaccianScale;
 
        Mat_<float> sharpenedf = 1.5f * grayImgf
                                   - 0.5f * blurredf
                                   - weight * grayImgf.mul(scale * laplaccian);
        
        sharpenedf.convertTo(sharpened, CV_8U);
        
        // Step 3 - Detect circles
        HoughCircles(sharpened,
                         circles,
                         CV_HOUGH_GRADIENT,
                         0.1f * settings.houghResolution,
                         settings.minRadius,
                         settings.cannyThreshold,
                         settings.accumulatorThreshold,
                         settings.minRadius,
                         settings.maxRadius );

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
 
static const char * windowName = "Hough Circle Detection Demo";
 
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
        createTrackbar("Spatial window radius", windowName, &mSettings.spatialWindowRadius,  100, trackbarPropertyChanged, this);
        createTrackbar("Color window radius",   windowName, &mSettings.colorWindowRadius,    2000, trackbarPropertyChanged, this);
        createTrackbar("Sharpening weight",     windowName, &mSettings.sharpeningWeight,     100, trackbarPropertyChanged, this);
        createTrackbar("Laplaccian scale",      windowName, &mSettings.laplaccianScale,      100, trackbarPropertyChanged, this);
        createTrackbar("Canny threshold",       windowName, &mSettings.cannyThreshold,       255, trackbarPropertyChanged, this);
        createTrackbar("Accumulator Threshold", windowName, &mSettings.accumulatorThreshold, 100, trackbarPropertyChanged, this);
        createTrackbar("Hough resolution",      windowName, &mSettings.houghResolution,      100, trackbarPropertyChanged, this);
        createTrackbar("Min Radius",            windowName, &mSettings.minRadius,            1500, trackbarPropertyChanged, this);
        createTrackbar("Max Radius",            windowName, &mSettings.maxRadius,            2000, trackbarPropertyChanged, this);
#endif
        display();
 
        // infinite loop to display
        // and refresh the content of the output image
        // until the user presses q or Q
        int key = 0;
        
        while(key != 'q' && key != 'Q')
        {
            // those paramaters cannot be =0 so we must check here
            mSettings.spatialWindowRadius  = std::max(mSettings.spatialWindowRadius, 1);
            mSettings.colorWindowRadius    = std::max(mSettings.colorWindowRadius, 1);
 
            mSettings.cannyThreshold       = std::max(mSettings.cannyThreshold, 1);
            mSettings.accumulatorThreshold = std::max(mSettings.accumulatorThreshold, 1);
 
            mSettings.houghResolution      = std::max(mSettings.houghResolution, 1);
            mSettings.minRadius            = std::max(mSettings.minRadius, 1);
            mSettings.maxRadius            = std::max(mSettings.maxRadius, 1);
 
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

    while (src.cols > 1000) {
        Mat small;
        pyrDown(src, small);
        src = small;
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
    else
    {
        printf("Unsupported algorithm: %s\n", algorithm.c_str());
        return -1;
    }

    wb->balanceWhite(src, res);

    //namedWindow("after color balance", 1);
    //imshow("after color balance", res);

    //setMouseCallback("after color balance", onMouseMove, &res);


    EggsDetectorBind bind(res);
    bind.run();

    return 0;
}

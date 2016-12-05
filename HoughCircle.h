#ifndef HOUGHCIRCLE_H_
#define HOUGHCIRCLE_H_

using namespace cv;
using namespace std;

/**
 * These settings can be adjusted in runtime with OpenCV sliders.
 * Due to slider implementatio (lack of floating-point support) so values
 * are stored scaled by x10 or x100.
 */
struct EggsDetectorAlgorithmSettings {
    int mSpatialWindowRadius;
    int mColorWindowRadius;
    int mSharpeningWeight; // x100
    int mLaplaccianScale;  // x100
    int mCannyThreshold;
    int mAccumulatorThreshold;

    int mHoughResolution; // x10
    int mMinRadius;
    int mMaxRadius;

    EggsDetectorAlgorithmSettings()
        : mSpatialWindowRadius(2)
        , mColorWindowRadius(103)
        , mSharpeningWeight(40)
        , mLaplaccianScale(20)
        , mCannyThreshold(40)
        , mAccumulatorThreshold(98)
        , mHoughResolution(10)
        , mMinRadius(148) //89
        , mMaxRadius(215) //94
    {
    }

    EggsDetectorAlgorithmSettings(int spatialWindowRadius,
                                  int colorWindowRadius,
                                  int sharpeningWeigh, // x100
                                  int laplaccianScale, // x100
                                  int cannyThreshold,
                                  int accumulatorThreshold,

                                  int houghResolution, // x10
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

class EggsDetectorAlgorithm {
public:
    void process(const Mat_<Vec3b>& inputRgbImage, const EggsDetectorAlgorithmSettings& settings)
    {
        //Step 1 - Filter image
        pyrMeanShiftFiltering(inputRgbImage, filtered, settings.mSpatialWindowRadius, settings.mColorWindowRadius, 1);

        //Step 2 - convert_to_gray
        cvtColor(filtered, grayImg, COLOR_BGR2GRAY);
        grayImg.convertTo(grayImgf, CV_32F);

        //Step 3
        GaussianBlur(grayImgf, blurredf, Size(5, 5), 0);

        //Step 4
        Laplacian(blurredf, laplaccian, CV_32F);

        float weight = 0.01f * settings.mSharpeningWeight;
        float scale  = 0.01f * settings.mLaplaccianScale;

        Mat_<float> sharpenedf = 1.5f * grayImgf
            - 0.5f * blurredf
            - weight * grayImgf.mul(scale * laplaccian);

        sharpenedf.convertTo(sharpened, CV_8U);

        // Step 6 - Detect circles
        HoughCircles(sharpened,
                     circles,
                     CV_HOUGH_GRADIENT,
                     0.1f * settings.mHoughResolution, //
                     settings.mMinRadius,              //两个圆之间的最小距离，
                     settings.mCannyThreshold,         //
                     settings.mAccumulatorThreshold,   //圆上像素点的范围，超过则成圆，否则丢弃
                     settings.mMinRadius,              //最小圆的半径
                     settings.mMaxRadius);             //最大圆的半径

        //Step 7 - Validate circles
        //TODO

        //
    }

    Mat display(const Mat& colorImg) const
    {
        // clone the colour, input image for displaying purposes
        Mat display = colorImg.clone();
        for (size_t i = 0; i < circles.size(); i++) {
            Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
            int   radius = cvRound(circles[i][2]);

            //color_pick(display, cvRound(circles[i][0]),cvRound(circles[i][1]));

            printf("circle center: (%d,%d-%d)\n", cvRound(circles[i][0]), cvRound(circles[i][1]), radius);
            // circle center
            circle(display, center, 3, Scalar(0, 255, 0), -1, 8, 0);
            // circle outline
            circle(display, center, radius, Scalar(0, 0, 255), 3, 8, 0);
        }

        return display;
    }

private:
    Mat         filtered;
    Mat         grayImg;
    Mat         grayImgf;
    Mat_<float> blurredf;
    Mat_<float> laplaccian;
    Mat         sharpened;

    std::vector<Vec3f> circles;
};

class CircleDetector {
public:
    CircleDetector(const Mat_<Vec3b>& inputRgbImage)
        : mImage(inputRgbImage)
    {
    }

    Mat findCircles(const EggsDetectorAlgorithmSettings& settings)
    {

        mAlgorithm.process(mImage, settings);
        return mAlgorithm.display(mImage);
    }

private:
    Mat                   mImage;
    EggsDetectorAlgorithm mAlgorithm;
};

class EggsDetectorBind {
public:
    EggsDetectorBind(const Mat_<Vec3b>& inputRgbImage);

    void run();

protected:
    void display();

    static void trackbarPropertyChanged(int, void* userdata)
    {
        EggsDetectorBind* self = (EggsDetectorBind*)userdata;
        self->display();
    }

private:
    Mat                           mImage;
    EggsDetectorAlgorithmSettings mSettings;
    EggsDetectorAlgorithm         mAlgorithm;
};

int findCircles(const Mat_<Vec3b>& inputRgbImage, vector<Vec3f>& circles, const EggsDetectorAlgorithmSettings& settings);
Mat drawCircles(const Mat& colorImg, vector<Vec3f>& circles);
void sortCircles(vector<Vec3f>& circles);

#endif /* GPICK_COLOR_H_ */

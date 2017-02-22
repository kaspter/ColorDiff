#include "opencv2/highgui.hpp"
#include "opencv2/xphoto.hpp"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgproc/imgproc_c.h>

#include "Color.h"
#include "ColorUtil.h"
#include "FindSquares.h"
#include "HoughCircle.h"

#include <stdlib.h>
#include <unistd.h>

//#define USE_FCGI 1

#ifdef USE_FCGI
extern char** environ;

#include "fcgi_config.h"
#include "fcgio.h"
#include <fcgi_stdio.h>
#endif

using namespace cv;
using namespace std;

const char* keys = { "{help h usage ? |         | print this message}"
                     "{i              |         | input image name  }" };

static const char* hcoName = "final CircleDetector";

static const int ColorIndex[26] = {3,   6,   7,   8,   9,   17,  18,  31,  42,  43,  54,  55,  110,
                      112, 113, 124, 126, 127, 138, 175, 177, 178, 179, 188, 189, 193};

static const float ColorHcho[26] = {0.35, 0.09, 0.02, 0.45, 0.13, 0.05, 0.18, 0.12, 0.25,
                       0.06, 0.07, 0.19, 0.10, 0.30, 0.14, 0.16, 0.15, 0.03,
                       0.11, 0.01, 0.08, 0.17, 0.04, 0.40, 0.20, 0.50};


void onMouseMove(int event, int x, int y, int flags, void* ustc)
{
    Mat& src = *(Mat*)ustc;

    (void)flags;

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

#define MAX_HCHO_POINTS 52  //甲醛取点个数
#define MAX_COLOR_POINTS 32 //色卡取点个数
#define MAX_CHECK_COUNTS 5  //检测次数

//在点Center 附近scope 范围内的4个象县随机生成
int genRandPoints(Point& Center, int scope, size_t num, vector<Vec3f>& oCircles)
{
    CvRNG rng;
    rng = cvRNG(cvGetTickCount());

    for (size_t i = 0; i < num; i++) {
        Vec3f Circle;
        int   rand1 = cvRandInt(&rng) % scope; //如果%6出来的将会是0~5的正整数
        int   rand2 = cvRandInt(&rng) % scope; //如果%6出来的将会是0~5的正整数

        int rand3 = i % 4;

        //四个象限各取几个点
        if (rand3 == 0) {
            Circle[0] = Center.x + rand1;
            Circle[1] = Center.y + rand2;
        } else if (rand3 == 1) {
            Circle[0] = Center.x - rand1;
            Circle[1] = Center.y - rand2;
        } else if (rand3 == 2) {
            Circle[0] = Center.x + rand1;
            Circle[1] = Center.y - rand2;
        } else {
            Circle[0] = Center.x - rand1;
            Circle[1] = Center.y + rand2;
        }

        Circle[2] = 0;

        // printf("%d-%d-%d-%d-%d\n",cvRound(Circle[0]), cvRound(Circle[1]), rand1, rand2, rand3);
        oCircles.push_back(Circle);
    }

    return 0;
}

//色差相差最大的两个点
// TODO: 删除色差 > Delta的点
int eraseRandPoints(Mat& image, vector<Vec3f>& iCircles, float Delta)
{
    float colorDelta = 0;
    int   iIndex     = -1;
    int   jIndex     = -1;

    for (size_t i = 0; i < iCircles.size(); i++) {

        Point iCenter(cvRound(iCircles[i][0]), cvRound(iCircles[i][1]));

        for (size_t j = 0; j < iCircles.size(); j++) {

            Point jCenter(cvRound(iCircles[j][0]), cvRound(iCircles[j][1]));

            float _delta = color_diff(image, iCenter, jCenter);

            // printf("colorDelta=%.2f\n", _delta);

            if (_delta > colorDelta) {
                colorDelta = _delta;
                iIndex     = i;
                jIndex     = j;
            } else {
                // TODO
            }
        }
    }

    // printf("size = %lu, max colorDelta=%f\n", oCircles.size(), colorDelta);
    if (Delta > 0 && jIndex >= 0 && iIndex >= 0) {
        vector<Vec3f>::iterator iIt = iCircles.begin() + iIndex;
        vector<Vec3f>::iterator jIt = iCircles.begin() + jIndex;
        iCircles.erase(iIt);
        iCircles.erase(jIt);
    }
    return 0;
}

int calcCirclePoints(Mat& image, vector<Vec3f>& bCircles,/* vector<Vec3f>& lCircles,*/
                     vector<Vec3f>& oCircles)
{
    oCircles.clear();

    Point MaxCircleCenter(cvRound(bCircles[0][0]), cvRound(bCircles[0][1]));
    int   MaxCircleRadius = cvRound(bCircles[0][2]);

    //生成随即点
    genRandPoints(MaxCircleCenter, MaxCircleRadius * 2 / 5, MAX_HCHO_POINTS, oCircles);

    //TODO: 排除在小圆内的点,不足MAX_HCHO_POINTS个点重新取点
#if 0
    for (size_t i = 0; i < lCircles.size(); i++) {

        Point iCenter(cvRound(lCircles[i][0]), cvRound(lCircles[i][1]));
        int   CircleRadius = cvRound(lCircles[i][2]);

        for (size_t j = 0; j < oCircles.size(); j++) {
            Point jCenter(cvRound(oCircles[j][0]), cvRound(oCircles[j][1]));
            // TODO
            int _delta = (iCenter.x - jCenter.x) * (iCenter.x - jCenter.x) + (iCenter.y - jCenter.y) * (iCenter.y - jCenter.y);
        }
    }
#endif

    //排除色差相差最大的两个点
    eraseRandPoints(image, oCircles, 3.0);

    return 0;
}

int calcColorPoints(Mat& image, vector<rectPointType>& vecRect, vector<Point>& Points,
                    vector<Vec3f>& oCircles /*vector<vector<Point> >& colorPoints*/)
{
    Rect maxRect;

    oCircles.clear();

    if (vecRect.size() > 0) {
        rectPointType rectPoint = vecRect.at(0);
        maxRect                 = rectPoint.rect;
    } else {
        return -1;
    }

    int MaxScope = maxRect.width / 56;

    for (size_t i = 0; i < Points.size(); i++) {

        Vec3f         Circle;
        vector<Vec3f> cPoint;

        Point Center = Points.at(i);

        //生成随即点
        genRandPoints(Center, MaxScope, MAX_COLOR_POINTS, oCircles);

        //排除色差相差最大的两个点 TODO refactor
        float colorDelta = 0.0;
        int   iIndex     = -1;
        int   jIndex     = -1;

        size_t f = i * (MAX_COLOR_POINTS - 2);

        for (size_t w = f; w < f + MAX_COLOR_POINTS; w++) {

            Point iCenter(cvRound(oCircles[w][0]), cvRound(oCircles[w][1]));

            for (size_t h = f; h < f + MAX_COLOR_POINTS; h++) {

                Point jCenter(cvRound(oCircles[h][0]), cvRound(oCircles[h][1]));

                // printf("(%lu-%lu)-(%d-%d)-(%d-%d)\n", w, h, iCenter.x,
                // iCenter.y,jCenter.x,jCenter.y);

                float _delta = color_diff(image, iCenter, jCenter);
                // printf("=============== (%lu-%lu)-(%d-%d)-(%d-%d)-%.3f\n", w, h, iCenter.x,
                // iCenter.y,jCenter.x,jCenter.y, _delta);
                if (_delta > colorDelta) {
                    colorDelta = _delta;
                    iIndex     = w;
                    jIndex     = h;
                } else {
                    // TODO
                }
            }
        }

        //printf("RectIdx %lu, max colorDelta=%f\n", i, colorDelta);
        if (jIndex >= 0 && iIndex >= 0) {
            vector<Vec3f>::iterator iIt = oCircles.begin() + iIndex;
            vector<Vec3f>::iterator jIt = oCircles.begin() + jIndex;
            oCircles.erase(iIt);
            oCircles.erase(jIt);
        }
    }

    return 0;
}

typedef struct {
    int   index;
    float delta;
} DeltaColorScore;

typedef struct {
    int index;
    int wins; //得分最高的次数
} WinColorScore;

//根据delta 从小到大排序
static bool sortDeltaScoreFun(const DeltaColorScore& c1, const DeltaColorScore& c2)
{
    return (c1.delta < c2.delta);
}

void sortDeltaColorScore(vector<DeltaColorScore>& scores)
{
    if (scores.size() > 0)
        sort(scores.begin(), scores.end(), sortDeltaScoreFun);
}

//根据得分 从大到小排序
static bool sortWinScoreFun(const WinColorScore& c1, const WinColorScore& c2)
{
    return (c1.wins > c2.wins);
}

void sortWinColorScore(vector<WinColorScore>& scores)
{
    if (scores.size() > 0)
        sort(scores.begin(), scores.end(), sortWinScoreFun);
}

int findMaxScore(Mat& image, vector<Vec3f>& ColorCircles, vector<Vec3f>& HchoCircles,
                 vector<WinColorScore>& winScores)
{
    winScores.clear();

    for (size_t i = 0; i < 26; i++) {
        WinColorScore wScores;
        wScores.index = i;
        wScores.wins  = 0;
        winScores.push_back(wScores);
    }

    for (size_t i = 0; i < HchoCircles.size(); i++) {

        Point HchoPoint(cvRound(HchoCircles[i][0]), cvRound(HchoCircles[i][1]));

        vector<DeltaColorScore> deltaScores;

        for (size_t z = 0; z < (MAX_COLOR_POINTS - 2); z++) {

            for (size_t j = 0; j < 26; j++) {

                DeltaColorScore iscore;

                Vec3f Circles = ColorCircles.at(j * (MAX_COLOR_POINTS - 2) + z);

                Point ColorPoint(cvRound(Circles[0]), cvRound(Circles[1]));

                iscore.delta = color_diff(image, HchoPoint, ColorPoint);
                iscore.index = j;
                deltaScores.push_back(iscore);
            }

            sortDeltaColorScore(deltaScores);

            // printf("HCHO point index = %lu, winGroup = %d\n", i, deltaScores.at(0).index);

            int            winIndex = deltaScores.at(0).index;
            WinColorScore& wScore   = winScores.at(winIndex);
            wScore.wins++;
        }
    }

    sortWinColorScore(winScores);
    return 0;
}

static float hcho_main(string inFilename, Mat& out)
{
    Mat src = imread(inFilename, 1);
    if (src.empty()) {
        printf("Cannot read image file: %s\n", inFilename.c_str());
        return -1;
    }

    Mat                        wbImg;
    Ptr<xphoto::WhiteBalancer> wb;
    wb = xphoto::createSimpleWB(); //createGrayworldWB()/createLearningBasedWB(modelFilename);

    // cvResize()，cvPyrDown(),cvPyrSegmentation()
    printf("col = %d\n", src.cols);
    while (src.cols > 1000) {
        Mat small;
        pyrDown(src, small);
        src = small;
    }

    wb->balanceWhite(src, wbImg);

    int ret;
    vector<Vec3f>                        bCircles;
    struct EggsDetectorAlgorithmSettings mBigSettings(2, 103, 40, 20, 40, 98, 10, 148, 240);
    ret = findCircles(wbImg, bCircles, mBigSettings);
    printf("big = %d\n", ret);

#if 0
    Mat BigImg = drawCircles(wbImg, bCircles);

    vector<Vec3f>                        lCircles;
    struct EggsDetectorAlgorithmSettings mLitSettings(2, 103, 7, 20, 26, 35, 12, /*86*/ 87, 92);
    ret = findCircles(BigImg, lCircles, mLitSettings);
    printf("lit = %d\n", ret);
    // fassthough if we do find little circles,
    Mat LitImg = drawCircles(BigImg, lCircles);
#else

    Mat LitImg = wbImg.clone();
#endif


    //识别矩形
    vector<rectPointType> vecRect;
    ret = findRects(wbImg, vecRect);

    printf("rects = %d\n", ret);
    if (ret <= 0) {
        return -1;
    }

    //色卡定位
    vector<Point> vecHPoints; //所有色块中心点
    vector<Point> CPoints;    //有效色块中心点（26个）

    int num = findAllRectCenter(LitImg, vecRect, vecHPoints);

    printf("num of rect center %d\n", num);
    for (int index = 0; index < 26 && index < num; index++) {
        Point center = vecHPoints.at(ColorIndex[index]);
        // circle(LitImg, center, 1, Scalar(0, 255, 0), -1, 8, 0);
        CPoints.push_back(center);
    }

    vector<int> ColorPPM;

    vector<Vec3f>         colorPoints;
    vector<Vec3f>         hcoPoints;
    vector<WinColorScore> winScores;

    for (int i = 0; i < MAX_CHECK_COUNTS; i++) {
        //色卡取点
        ret = calcColorPoints(LitImg, vecRect, CPoints, colorPoints);
        if (ret != 0 ) {
            printf("errorrrrrrrrrr...\n");
        }
        //甲醛取点
        ret = calcCirclePoints(LitImg, bCircles, /*lCircles,*/ hcoPoints);

        //投票
        ret = findMaxScore(LitImg, colorPoints, hcoPoints, winScores);

        //记录结果
        printf("===== ColorIndex = %d, ppm = %.2f, wins = %d\n", winScores.at(0).index,
               ColorHcho[winScores.at(0).index], winScores.at(0).wins);
        ColorPPM.push_back(winScores.at(0).index);
    }

#if 0
    //标出选定色卡
    for (size_t i = 0; i < ColorPPM.size(); i++) {
        int blockIdx = ColorIndex[ColorPPM[i]];
        Point block = vecHPoints.at(blockIdx);
        circle(LitImg, block, 6, Scalar(0, 0, 255), -1, 8, 0);
    }

    //标出所有巨型
    drawRects(LitImg, vecRect);

    //标出色卡取点
    for (size_t i = 0; i < colorPoints.size(); i++) {
        Point center(cvRound(colorPoints[i][0]), cvRound(colorPoints[i][1]));
        circle(LitImg, center, 1, Scalar(0, 255, 0), -1, 8, 0);
    }
    //标出甲醛取点
    for (size_t i = 0; i < hcoPoints.size(); i++) {
        Point center(cvRound(hcoPoints[i][0]), cvRound(hcoPoints[i][1]));
        circle(LitImg, center, 1, Scalar(0, 255, 0), -1, 8, 0);
    }
#endif

    float ppm = 0.0;
    for (size_t i = 0; i < ColorPPM.size(); i++) {
        ppm += ColorHcho[ColorPPM[i]];
        printf("===== final ColorPPM = %.2f\n", ppm / (i + 1));
    }

    out = LitImg.clone();
    return (ppm / MAX_CHECK_COUNTS);
}

// Maximum number of bytes allowed to be read from stdin
static const unsigned long STDIN_MAX = 1000000;

//转译

static unsigned char hexchars[] = "0123456789ABCDEF";

static int php_htoi(char* s)
{
    int value;
    int c;

    c = ((unsigned char*)s)[0];
    if (isupper(c))
        c = tolower(c);
    value = (c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10) * 16;

    c = ((unsigned char*)s)[1];
    if (isupper(c))
        c = tolower(c);
    value += c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10;

    return (value);
}

char* php_url_encode(char const* s, int len, int* new_length)
{
    register unsigned char c;
    unsigned char *        to, *start;
    unsigned char const *  from, *end;

    from  = (unsigned char*)s;
    end   = (unsigned char*)s + len;
    start = to = (unsigned char*)calloc(1, 3 * len + 1);

    while (from < end) {
        c = *from++;

        if (c == ' ') {
            *to++ = '+';
        } else if ((c < '0' && c != '-' && c != '.') || (c < 'A' && c > '9') || (c > 'Z' && c < 'a' && c != '_') || (c > 'z')) {
            to[0] = '%';
            to[1] = hexchars[c >> 4];
            to[2] = hexchars[c & 15];
            to += 3;
        } else {
            *to++ = c;
        }
    }
    *to = 0;
    if (new_length) {
        *new_length = to - start;
    }
    return (char*)start;
}

int php_url_decode(char* str, int len)
{
    char* dest = str;
    char* data = str;

    while (len--) {
        if (*data == '+') {
            *dest = ' ';
        } else if (*data == '%' && len >= 2 && isxdigit((int)*(data + 1)) && isxdigit((int)*(data + 2))) {
            *dest = (char)php_htoi(data + 1);
            data += 2;
            len -= 2;
        } else {
            *dest = *data;
        }
        data++;
        dest++;
    }
    *dest = '\0';
    return dest - str;
}

#ifdef USE_FCGI
static long parseFilePath(FCGX_Request* request, char** content)
{
    char* clenstr = FCGX_GetParam("REQUEST_URI", request->envp);

    unsigned long clen = STDIN_MAX;

    if (clenstr) {
        char* equal = strstr(clenstr, "fastcgi.cgi?path=");
        char* dollr = strstr(equal, "&");

        if (!equal) {
            return -1;
        }

        if (!dollr) {
            clen = strlen(equal);
        } else {
            clen = dollr - equal - strlen("fastcgi.cgi?path=");
        }

        *content = new char[clen + 1];
        memset(*content, 0, clen + 1);
        memcpy(*content, equal + strlen("fastcgi.cgi?path="), clen);

    } else {
        // *never* read stdin when CONTENT_LENGTH is missing or unparsable
        *content = 0;
        clen     = 0;
    }

    return clen;
}

int fastcgi_main(int argc, const char** argv)
{

    int  count = 0;
    long pid   = getpid();

    streambuf* cin_streambuf  = cin.rdbuf();
    streambuf* cout_streambuf = cout.rdbuf();
    streambuf* cerr_streambuf = cerr.rdbuf();

    FCGX_Request request;

    FCGX_Init();
    FCGX_InitRequest(&request, 0, 0);

    //_r 调用多线程安全版本
    while (FCGX_Accept_r(&request) == 0) {

        // Note that the default bufsize (0) will cause the use of iostream
        // methods that require positioning (such as peek(), seek(),
        // unget() and putback()) to fail (in favour of more efficient IO).
        fcgi_streambuf cin_fcgi_streambuf(request.in);
        fcgi_streambuf cout_fcgi_streambuf(request.out);
        fcgi_streambuf cerr_fcgi_streambuf(request.err);

        cin.rdbuf(&cin_fcgi_streambuf);
        cout.rdbuf(&cout_fcgi_streambuf);
        cerr.rdbuf(&cerr_fcgi_streambuf);

        //获取图片路径
        char*         content;
        unsigned long clen = parseFilePath(&request, &content);

        cout << "Content-type: text/html\r\n"
                "\r\n"
                "<TITLE>echo-cpp</TITLE>\n"
                "<H1>echo-cpp</H1>\n"
                "<H4>PID: "
             << pid << "</H4>\n"
                       "<H4>Request Number: "
             << ++count << "</H4>\n";

        if (clen > 0) {

            //字符串转译
            php_url_decode(content, clen);

            cout << "ImagePath: <H2>" << content << "</H2>\n";

            // string imgfile = "/srv/22b64d15-3183-423e-8813-961d921f6f1c.jpg";
            string imgfile(content);

            Mat out;
            float ppm = hcho_main(imgfile, out);

            cout << "ImagePPM:<span>" << ppm << "</span>\n";
        }

        if (content)
            delete[] content;
    }

    cin.rdbuf(cin_streambuf);
    cout.rdbuf(cout_streambuf);
    cerr.rdbuf(cerr_streambuf);

    return 0;
}
#endif

int main(int argc, const char** argv)
{

    color_init();

#if 1
    CommandLineParser parser(argc, argv, keys);

    parser.about("OpenCV color diff sample");

    if (parser.has("help") || argc < 2) {
        parser.printMessage();
        return 0;
    }

    string inFilename = parser.get<string>("i");

    if (!parser.check()) {
        parser.printErrors();
        return -1;
    }

    Mat out;
    printf("==================================\n");
    float ppm = hcho_main(inFilename, out);
    printf("xxx ppm: %f\n",ppm);
    
#if 0
    if (ppm > 0) {
        namedWindow(hcoName, 1);
        setMouseCallback(hcoName, onMouseMove, &out);
        imshow(hcoName, out);

        int key = 0;
        while (key != 'q' && key != 'Q') {
            key = waitKey(10);
        }
    }
#endif

    return 0;
#else
    return fastcgi_main(argc,argv);
#endif
}



#include "opencv2/highgui.hpp"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgproc/imgproc_c.h>

#include "HoughCircle.h"

using namespace cv;
using namespace std;

static const char* windowName = "Hough Circle Detection Demo";


//http://b217dgy.blog.51cto.com/5704306/1320360

EggsDetectorBind::EggsDetectorBind(const Mat_<Vec3b>& inputRgbImage)
    : mImage(inputRgbImage)
{
}

void EggsDetectorBind::run(void)
{

    // create the main window, and attach the trackbars
    namedWindow(windowName, WINDOW_AUTOSIZE);
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

    while (key != 'q' && key != 'Q') {
        // those paramaters cannot be =0 so we must check here
        mSettings.mSpatialWindowRadius = std::max(mSettings.mSpatialWindowRadius, 1);
        mSettings.mColorWindowRadius   = std::max(mSettings.mColorWindowRadius, 1);

        mSettings.mCannyThreshold       = std::max(mSettings.mCannyThreshold, 1);
        mSettings.mAccumulatorThreshold = std::max(mSettings.mAccumulatorThreshold, 1);

        mSettings.mHoughResolution = std::max(mSettings.mHoughResolution, 1);
        mSettings.mMinRadius       = std::max(mSettings.mMinRadius, 1);
        mSettings.mMaxRadius       = std::max(mSettings.mMaxRadius, 1);

        // get user key
        key = waitKey(10);
    }
}

void EggsDetectorBind::display(void)
{
    mAlgorithm.process(mImage, mSettings);
    imshow(windowName, mAlgorithm.display(mImage));
}

#if 0
static void icvFindCirclesGradient( CvMat* img, Mat &contour_img, float dp, float min_dist,int min_radius, int max_radius,
	int low_threshold, int high_threshold,int acc_threshold,CvSeq* circles, int circles_max )
{
	const int SHIFT = 10, ONE = 1 << SHIFT;
	cv::Ptr<CvMat> dx, dy;
	cv::Ptr<CvMat> edges, accum, dist_buf;
	std::vector<int> sort_buf;
	cv::Ptr<CvMemStorage> storage;

	int x, y, i, j, k, center_count, nz_count;
	float min_radius2 = (float)min_radius*min_radius;
	float max_radius2 = (float)max_radius*max_radius;
	int rows, cols, arows, acols;
	int astep, *adata;
	float* ddata;
	CvSeq *nz, *centers;
	float idp, dr;
	CvSeqReader reader;

	//如果输入的轮廓图尺寸与输入的源图完全相同，则使用输入的轮廓图，否则cvCanny提取轮廓图，可以更灵活的预处理图像
	if ( contour_img.cols==img->cols && contour_img.rows==img->rows )
	{
		edges = cvCloneMat(&CvMat(contour_img));
	}else
	{
		edges = cvCreateMat( img->rows, img->cols, CV_8UC1 );
		cvCanny( img, edges, low_threshold, high_threshold, 3 );	//添加cvCanny的低阈值low_threshold，参数控制可以更灵活
		contour_img=Mat(edges)+0;
	}

	dx = cvCreateMat( img->rows, img->cols, CV_16SC1 );
	dy = cvCreateMat( img->rows, img->cols, CV_16SC1 );
	///////////////////////////////////计算方向导数的Sobel核大小，重要参数////////////////////////////////////////
	cvSobel( img, dx, 1, 0, cvSobel_Core );
	cvSobel( img, dy, 0, 1, cvSobel_Core );

	if( dp < 1.f )
		dp = 1.f;
	idp = 1.f/dp;
	accum = cvCreateMat( cvCeil(img->rows*idp)+2, cvCeil(img->cols*idp)+2, CV_32SC1 );
	cvZero(accum);

	storage = cvCreateMemStorage();
	nz = cvCreateSeq( CV_32SC2, sizeof(CvSeq), sizeof(CvPoint), storage );
	centers = cvCreateSeq( CV_32SC1, sizeof(CvSeq), sizeof(int), storage );

	rows = img->rows;
	cols = img->cols;
	arows = accum->rows - 2;
	acols = accum->cols - 2;
	adata = accum->data.i;
	astep = accum->step/sizeof(adata[0]);
	// Accumulate circle evidence for each edge pixel
	for( y = 0; y < rows; y++ )
	{
		const uchar* edges_row = edges->data.ptr + y*edges->step;
		const short* dx_row = (const short*)(dx->data.ptr + y*dx->step);
		const short* dy_row = (const short*)(dy->data.ptr + y*dy->step);

		for( x = 0; x < cols; x++ )
		{
			float vx, vy;
			int sx, sy, x0, y0, x1, y1, r;
			CvPoint pt;

			vx = dx_row[x];
			vy = dy_row[x];

			if( !edges_row[x] || (vx == 0 && vy == 0) )
				continue;

			float mag = sqrt(vx*vx+vy*vy);
			assert( mag >= 1 );
			sx = cvRound((vx*idp)*ONE/mag);
			sy = cvRound((vy*idp)*ONE/mag);

			x0 = cvRound((x*idp)*ONE);
			y0 = cvRound((y*idp)*ONE);
			// Step from min_radius to max_radius in both directions of the gradient
			for(int k1 = 0; k1 < 2; k1++ )
			{
				x1 = x0 + min_radius * sx;
				y1 = y0 + min_radius * sy;

				for( r = min_radius; r <= max_radius; x1 += sx, y1 += sy, r++ )
				{
					int x2 = x1 >> SHIFT, y2 = y1 >> SHIFT;
					if( (unsigned)x2 >= (unsigned)acols ||
						(unsigned)y2 >= (unsigned)arows )
						break;
					adata[y2*astep + x2]++;
				}

				sx = -sx; sy = -sy;
			}

			pt.x = x; pt.y = y;
			cvSeqPush( nz, &pt );
		}
	}

	nz_count = nz->total;
	if( !nz_count )
		return;
	//Find possible circle centers
	for( y = 1; y < arows - 1; y++ )
	{
		for( x = 1; x < acols - 1; x++ )
		{
			int base = y*(acols+2) + x;
			if( adata[base] > acc_threshold &&
				adata[base] > adata[base-1] && adata[base] > adata[base+1] &&
				adata[base] > adata[base-acols-2] && adata[base] > adata[base+acols+2] )
				cvSeqPush(centers, &base);
		}
	}

	center_count = centers->total;
	if( !center_count )
		return;

	sort_buf.resize( MAX(center_count,nz_count) );
	cvCvtSeqToArray( centers, &sort_buf[0] );

	icvHoughSortDescent32s( &sort_buf[0], center_count, adata );
	cvClearSeq( centers );
	cvSeqPushMulti( centers, &sort_buf[0], center_count );

	dist_buf = cvCreateMat( 1, nz_count, CV_32FC1 );
	ddata = dist_buf->data.fl;

	dr = dp;
	min_dist = MAX( min_dist, dp );
	min_dist *= min_dist;
	// For each found possible center
	// Estimate radius and check support
	for( i = 0; i < centers->total; i++ )
	{
		int ofs = *(int*)cvGetSeqElem( centers, i );
		y = ofs/(acols+2);
		x = ofs - (y)*(acols+2);
		//Calculate circle's center in pixels
		float cx = (float)((x + 0.5f)*dp), cy = (float)(( y + 0.5f )*dp);
		float start_dist, dist_sum;
		float r_best = 0;
		int max_count = 0;
		// Check distance with previously detected circles
		for( j = 0; j < circles->total; j++ )
		{
			float* c = (float*)cvGetSeqElem( circles, j );
			if( (c[0] - cx)*(c[0] - cx) + (c[1] - cy)*(c[1] - cy) < min_dist )
				break;
		}

		if( j < circles->total )
			continue;
		// Estimate best radius
		cvStartReadSeq( nz, &reader );
		for( j = k = 0; j < nz_count; j++ )
		{
			CvPoint pt;
			float _dx, _dy, _r2;
			CV_READ_SEQ_ELEM( pt, reader );
			_dx = cx - pt.x; _dy = cy - pt.y;
			_r2 = _dx*_dx + _dy*_dy;
			if(min_radius2 <= _r2 && _r2 <= max_radius2 )
			{
				ddata[k] = _r2;
				sort_buf[k] = k;
				k++;
			}
		}

		int nz_count1 = k, start_idx = nz_count1 - 1;
		if( nz_count1 == 0 )
			continue;
		dist_buf->cols = nz_count1;
		cvPow( dist_buf, dist_buf, 0.5 );
		icvHoughSortDescent32s( &sort_buf[0], nz_count1, (int*)ddata );

		dist_sum = start_dist = ddata[sort_buf[nz_count1-1]];
		for( j = nz_count1 - 2; j >= 0; j-- )
		{
			float d = ddata[sort_buf[j]];

			if( d > max_radius )
				break;

			if( d - start_dist > dr )
			{
				float r_cur = ddata[sort_buf[(j + start_idx)/2]];
				if( (start_idx - j)*r_best >= max_count*r_cur ||
					(r_best < FLT_EPSILON && start_idx - j >= max_count) )
				{
					r_best = r_cur;
					max_count = start_idx - j;
				}
				start_dist = d;
				start_idx = j;
				dist_sum = 0;
			}
			dist_sum += d;
		}
		// Check if the circle has enough support
		if( max_count > acc_threshold )
		{
			float c[3];
			c[0] = cx;
			c[1] = cy;
			c[2] = (float)r_best;
			cvSeqPush( circles, c );
			if( circles->total > circles_max )
				return;
		}
	}
}




CV_IMPL CvSeq* cvFindCircles( CvArr* src_image, Mat &contour_image, void* circle_storage,float dp, int min_dist,
	int low_threshold, int high_threshold,int acc_threshold,int min_radius, int max_radius)
{
	CvSeq* result = 0;

	CvMat stub, *img = (CvMat*)src_image;

	CvMat* mat = 0;
	CvSeq* circles = 0;
	CvSeq circles_header;
	CvSeqBlock circles_block;
	int circles_max = INT_MAX;

	img = cvGetMat( img, &stub );

	if( !CV_IS_MASK_ARR(img))
		CV_Error( CV_StsBadArg, "The source image must be 8-bit, single-channel" );

	if ( contour_image.cols==img->cols && contour_image.rows==img->rows )
	{
		if( contour_image.type()!=CV_8UC1)
			CV_Error( CV_StsBadArg, "The contour image must be 8-bit, single-channel" );
	}

	if( !circle_storage )
		CV_Error( CV_StsNullPtr, "NULL destination" );

	if( dp <= 0 || min_dist <= 0 || low_threshold <= 0 || high_threshold <= 0 || acc_threshold <= 0 )
		CV_Error( CV_StsOutOfRange, "dp, min_dist, canny_threshold and acc_threshold must be all positive numbers" );

	min_radius = MAX( min_radius, 0 );
	if( max_radius <= 0 )
		max_radius = MAX( img->rows, img->cols );
	else if( max_radius <= min_radius )
		max_radius = min_radius + 2;

	if( CV_IS_STORAGE( circle_storage ))
	{
		circles = cvCreateSeq( CV_32FC3, sizeof(CvSeq),
			sizeof(float)*3, (CvMemStorage*)circle_storage );
	}
	else if( CV_IS_MAT( circle_storage ))
	{
		mat = (CvMat*)circle_storage;

		if( !CV_IS_MAT_CONT( mat->type ) || (mat->rows != 1 && mat->cols != 1) ||
			CV_MAT_TYPE(mat->type) != CV_32FC3 )
			CV_Error( CV_StsBadArg,
			"The destination matrix should be continuous and have a single row or a single column" );

		circles = cvMakeSeqHeaderForArray( CV_32FC3, sizeof(CvSeq), sizeof(float)*3,
			mat->data.ptr, mat->rows + mat->cols - 1, &circles_header, &circles_block );
		circles_max = circles->total;
		cvClearSeq( circles );
	}
	else
		CV_Error( CV_StsBadArg, "Destination is not CvMemStorage* nor CvMat*" );

	icvFindCirclesGradient( img, contour_image, (float)dp, (float)min_dist,
		min_radius, max_radius, low_threshold,high_threshold,
		acc_threshold, circles, circles_max );

	if( mat )
	{
		if( mat->cols > mat->rows )
			mat->cols = circles->total;
		else
			mat->rows = circles->total;
	}
	else
		result = circles;

	return result;
}

#endif

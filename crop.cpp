#include <opencv2/opencv.hpp>

typedef struct overflow
{
	int top;
	int bottom;
	int left;
	int right;
} overflow_t;

void
calc_overflow(cv::Mat img, cv::Rect roi, overflow_t *roi_overflow_px)
{
	roi_overflow_px->top = roi.y < 0 ? -roi.y : 0;
	roi_overflow_px->bottom = roi.y + roi.height > img.size().height ? roi.y + roi.height - img.size().height : 0;
	roi_overflow_px->left = roi.x < 0 ? -roi.x : 0;
	roi_overflow_px->right = roi.x + roi.width > img.size().width ? roi.x + roi.width - img.size().width : 0;

	DLOG("overflows - top:%d, buttom:%d, left:%d, right:%d", roi_overflow_px->top, roi_overflow_px->bottom, roi_overflow_px->left, roi_overflow_px->right);
}

void
crop_roi(cv::Rect *roi, overflow_t *roi_overflow_px)
{
	roi->x += roi_overflow_px->left;
	roi->y += roi_overflow_px->top;
	roi->width -= roi_overflow_px->right + roi_overflow_px->left;
	roi->height -= roi_overflow_px->bottom + roi_overflow_px->top;

	DLOG("roi - x:%d, y:%d, width:%d, height:%d", roi->x, roi->y, roi->width, roi->height);
}

void
crop (cv::Mat img, cv::Rect roi, cv::Mat *cropped_img)
{
	overflow_t roi_overflow_px;
	calc_overflow(img, roi, &roi_overflow_px);

	crop_roi(&roi, &roi_overflow_px);

	// Crop the original image to the defined ROI
	cv::Mat crop = img(roi);

	cvtColor(crop, crop, cv::COLOR_RGB2RGBA);

	// Put the cropped image onto the background
	cv::Rect overflow(roi_overflow_px.left, roi_overflow_px.top, crop.size().width, crop.size().height);
	crop.copyTo((*cropped_img)(overflow));
}

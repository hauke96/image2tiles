#include <opencv2/opencv.hpp>

/**
 * The overflow is the area that is larger then e.g. the image. For example
 * when a rectangle has a negative X/Y coordinate, this is not in the image
 * and is therefore called overflow.
 *
 * An overflow has four values and is written as four-tuple:
 *
 *    (left, right, top, bottom)
 *
 * Example:
 *
 *     * * * * * * * * * * * +  -+-
 *     * rect with overflow  *   | 20
 *     *                     *   |
 *     *   +-----------------+  -+-
 *     *   | image           |
 *     *   |                 |
 *     *   |                 |
 *     *   |                 |
 *     *   |                 |
 *     * * +-----------------+
 *
 *     |---|
 *      10
 *
 * The overflow of this example is (10, 0, 20, 0).
 */
typedef struct overflow
{
	int left;
	int right;
	int top;
	int bottom;
} overflow_t;

/**
 * Calculates the amount of pixel the given rectangle is larger than the image.
 *
 * For exmaple: The image is 100x100 large and the rectangle is (-10, 0, 110, 90) the overflow would be (10, 0, 10, 0). Notice that only the pixel larger than the image are set.
 *
 * @param img The image using to calculate the overflow.
 * @param roi The region of interest that migh has overflow regarding the given image.
 * @param roi_overflow_px The output parameter containing the overflow.
 */
void
calc_overflow(cv::Mat img, cv::Rect roi, overflow_t *roi_overflow_px)
{
	roi_overflow_px->top = roi.y < 0 ? -roi.y : 0;
	roi_overflow_px->bottom = roi.y + roi.height > img.size().height ? roi.y + roi.height - img.size().height : 0;
	roi_overflow_px->left = roi.x < 0 ? -roi.x : 0;
	roi_overflow_px->right = roi.x + roi.width > img.size().width ? roi.x + roi.width - img.size().width : 0;

	DLOG("overflows - top:%d, buttom:%d, left:%d, right:%d", roi_overflow_px->top, roi_overflow_px->bottom, roi_overflow_px->left, roi_overflow_px->right);
}

/**
 * Removes the given overflow from the rectangle.
 *
 * For example: The rectangle is (-50,10,100,100), the overflow is (50,0,0,20) (with the values (left,right,top,bottom)) then the result is (0,10,50,80).
 *
 * @param roi Region (rectangle) of interest. This will be changed and contains the area without the given overflow).
 * @param roi_overflow_px The overflow that should be removed from the rectangle.
 */
void
crop_roi(cv::Rect *roi, overflow_t *roi_overflow_px)
{
	roi->x += roi_overflow_px->left;
	roi->y += roi_overflow_px->top;
	roi->width -= roi_overflow_px->right + roi_overflow_px->left;
	roi->height -= roi_overflow_px->bottom + roi_overflow_px->top;

	/*
	 *     * * * * * * * * * *  -+-
	 *     * old rect        *   | 20
	 *     *                 *   |
	 *     *   +-------------+  -+-
	 *     *   |new rect     |
	 *     *   |             |
	 *     *   |             |
	 *     *   |             |
	 *     *   |             |
	 *     * * +-------------+
	 *
	 *     |---|
	 *      10
	 *
	 * The difference between the old and the new rectangle is the overflow. In
	 * this example, the overflow would be (10, 0, 20, 0).
	 */

	DLOG("roi - x:%d, y:%d, width:%d, height:%d", roi->x, roi->y, roi->width, roi->height);
}

/**
 * Cuts the given rectangle out of the given image.
 *
 * @param img The orginal image.
 * @param roi The region of interest that should be cu out.
 * @param cropped_img The output image, which contains the region of interest.
 */
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

#include <stdio.h>
#include <experimental/filesystem>

#include <opencv2/opencv.hpp>

int DEBUG = 1;

#define dlog(fmt, ...) \
	if (DEBUG) \
		printf("%s - " fmt "\n", __func__, ##__VA_ARGS__); \

#define log(fmt, ...) \
	printf(fmt "\n", ##__VA_ARGS__); \

typedef struct overflow
{
	int top;
	int bottom;
	int left;
	int right;
} overflow_t;

void
calc_overflow(cv::Mat img, cv::Rect roi, overflow_t *flow)
{
	flow->top = roi.y < 0 ? -roi.y : 0;
	flow->bottom = roi.y + roi.height > img.size().height ? roi.y + roi.height - img.size().height : 0;
	flow->left = roi.x < 0 ? -roi.x : 0;
	flow->right = roi.x + roi.width > img.size().width ? roi.x + roi.width - img.size().width : 0;

	dlog("overflows - top:%d, buttom:%d, left:%d, right:%d", flow->top, flow->bottom, flow->left, flow->right);
}

void
crop_roi(cv::Rect *roi, overflow_t *flow)
{
	roi->x += flow->left;
	roi->y += flow->top;
	roi->width -= flow->right + flow->left;
	roi->height -= flow->bottom + flow->top;

	dlog("roi - x:%d, y:%d, width:%d, height:%d", roi->x, roi->y, roi->width, roi->height);
}

void
crop (cv::Mat img, cv::Rect roi, cv::Mat *cropped_img)
{
	overflow_t flow;
	calc_overflow(img, roi, &flow);

	crop_roi(&roi, &flow);

	// Crop the original image to the defined ROI
	cv::Mat crop = img(roi);

	cvtColor(crop, crop, cv::COLOR_RGB2RGBA);

	// Put the cropped image onto the background
	cv::Rect overflow(flow.left, flow.top, crop.size().width, crop.size().height);
	crop.copyTo((*cropped_img)(overflow));
}

void
save_image(cv::Mat img, int x, int y, int z)
{
	// Ensure that folder exist
	std::string folderName = std::to_string(z) + "/" + std::to_string(x);
	std::experimental::filesystem::create_directories(folderName);

	// Write final image to disk
	cv::imwrite(std::to_string(z) + "/" + std::to_string(x) + "/" + std::to_string(y) + ".png", img);
}

int
main ()
{
	printf("Read image ...\n");
	cv::Mat img = cv::imread("img.jpg", cv::IMREAD_UNCHANGED);

	if (img.empty())
	{
	    std::cout << "!!! imread() failed to open target image" << std::endl;
	    return -1;        
	}

	printf("Start cuttig it ...\n");

	/* Set Region of Interest */

	int offset_x = 45;
	int offset_y = -195;

	cv::Rect roi;
	roi.x = offset_x;
	roi.y = offset_y;
	roi.width = 255;
	roi.height = 255;

	// TODO Extract values into variables or parse from arguments
	for (int x = 3638; roi.x <= img.size().width; x++)
	{
		printf("Cut column Z:%d, X:%d\n", 13, x);
		for (int y = 2178; roi.y <= img.size().height; y++)
		{
			cv::Mat cropped_img(roi.width, roi.height, CV_8UC4, cv::Scalar(0, 0, 0, 0));
			crop(img, roi, &cropped_img);
			resize(cropped_img, cropped_img, cv::Size(256, 256), 0, 0, cv::INTER_LINEAR_EXACT);
			save_image(cropped_img, x, y, 13);
			roi.y += roi.height;
		}

		roi.x += roi.width;
		roi.y = offset_y;
	}	

	printf("Done!\n");

	return 0;
}

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
crop (cv::Mat img, cv::Rect roi, int x, int y, int z)
{
	static int i = 0;

	dlog("Crop x:%d, y:%d ...", x, y);

	cv::Mat background(roi.width, roi.height, CV_8UC4, cv::Scalar(0, 0, 0, 0));

	overflow_t flow;
	flow.top = roi.y < 0 ? -roi.y : 0;
	flow.bottom = roi.y + roi.height > img.size().height ? roi.y + roi.height - img.size().height : 0;
	flow.left = roi.x < 0 ? -roi.x : 0;
	flow.right = roi.x + roi.width > img.size().width ? roi.x + roi.width - img.size().width : 0;

	dlog("overflows - top:%d, buttom:%d, left:%d, right:%d", flow.top, flow.bottom, flow.left, flow.right);

	roi.x += flow.left;
	roi.y += flow.top;
	roi.width -= flow.right + flow.left;
	roi.height -= flow.bottom + flow.top;

	dlog("roi - x:%d, y:%d, width:%d, height:%d", roi.x, roi.y, roi.width, roi.height);

	// Crop the original image to the defined ROI
	cv::Mat crop = img(roi);

	cvtColor(crop, crop, cv::COLOR_RGB2RGBA);

	// Put the cropped image onto the background
	cv::Rect overflow(flow.left, flow.top, crop.size().width, crop.size().height);
	crop.copyTo(background(overflow));

	// Ensure that folder exist
	std::string folderName = std::to_string(z) + "/" + std::to_string(x);
	std::experimental::filesystem::create_directories(folderName);

	// Write final image to disk
	cv::imwrite(std::to_string(z) + "/" + std::to_string(x) + "/" + std::to_string(y) + ".png", background);

	dlog("DONE");

	i++;
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
			crop(img, roi, x, y, 13);
			roi.y += roi.height;
		}

		roi.x += roi.width;
		roi.y = offset_y;
	}	

	printf("Done!\n");

	return 0;
}

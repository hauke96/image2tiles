#include <stdio.h>
#include <algorithm>
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
	std::string folderName = "out/" + std::to_string(z) + "/" + std::to_string(x);
	std::experimental::filesystem::create_directories(folderName);

	// Write final image to disk
	cv::imwrite("out/" + std::to_string(z) + "/" + std::to_string(x) + "/" + std::to_string(y) + ".png", img);
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

	// Set Region of Interest
	int zoom_level = 13;

	int output_tile_size_px = 256;

	int first_tile_x = 45;
	int first_tile_y = -195;

	cv::Rect roi;
	roi.x = first_tile_x;
	roi.y = first_tile_y;
	roi.width = 255;
	roi.height = 255;

	int start_coord_x = 3638;
	int start_coord_y = 2178;

	for (int z = zoom_level; z >= 0; z--)
	{
		dlog("y:%d, y:%d, w:%d, h:%d", roi.x, roi.y, roi.width, roi.height);
		dlog("Top left tile: https://a.tile.openstreetmap.org/%d/%d/%d.png", z, start_coord_x, start_coord_y);

		for (int x = start_coord_x; roi.x <= img.size().width; x++)
		{
			printf("Cut column Z:%d, X:%d\n", z, x);

			for (int y = start_coord_y; roi.y <= img.size().height; y++)
			{
				cv::Mat cropped_img(roi.width, roi.height, CV_8UC4, cv::Scalar(0, 0, 0, 0));
				crop(img, roi, &cropped_img);

				resize(cropped_img, cropped_img, cv::Size(output_tile_size_px, output_tile_size_px), 0, 0, cv::INTER_LINEAR_EXACT);

				save_image(cropped_img, x, y, z);

				roi.y += roi.height;
			}

			roi.x += roi.width;
			roi.y = first_tile_y;
		}

		roi.x = first_tile_x;

		/*
		 * When the current coordinate is odd, the corner of the upper (upper
		 * = one z layer less) tile is shifted
		 * 
		 *    |x:100   'x':101  |x:102
		 *    |x':50   '        |x':51
		 * ---+-----------------+---
		 *    |        '        |
		 *    |        '        |
		 *    |        '        |
		 *  - + - - - - - - - - + - 
		 *    |        '        |
		 *    |        '        |
		 *    |        '        |
		 * ---+-----------------+---
		 *    |        ,        |
		 * 
		 * x is the position on e.g. zoom level 13 and x' would then be the
		 * position on zoom level 12. If a feature is on tile, which has the
		 * position x:101, it'll appear on the tile x':50 whose origin is
		 * shifted to the left. This is only the case when the original tile
		 * has an odd coordinate.
		 */
		if (start_coord_x % 2 != 0)
		{
			roi.x -= roi.width; 
			start_coord_x--;
		}
		if (start_coord_y % 2 != 0)
		{
			roi.y -= roi.height; 
			start_coord_y--;
		}

		resize(img, img, cv::Size(std::max(1, img.size().width / 2), std::max(1, img.size().height / 2)), 0, 0, cv::INTER_AREA);

		// When the image is half as large, all distances and offsets have to
		// be as well
		roi.x /= 2;
		roi.y /= 2;

		// Because the loops are resetting the positions
		first_tile_x = roi.x;
		first_tile_y = roi.y;

		// Actually update the x and y coordinates
		start_coord_x /= 2;
		start_coord_y /= 2;
	}

	printf("Done!\n");

	return 0;
}

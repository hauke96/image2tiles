#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <experimental/filesystem>
#include <regex>

#include <opencv2/opencv.hpp>

#include "math.cpp"
#include "logging.cpp"
#include "crop.cpp"
#include "settings.cpp"

/**
 * Saved the image (tile). The folder structure is:
 *
 *    ./{settings.output_folder}/{z}/{x}/{y}.png.
 *
 * @param img The image (tile) to store.
 * @param settings The settings used to determine the output directory.
 * @param x_coord The x coordinate of the tile.
 * @param y_coord The y coordinate of the tile.
 * @param z The zoom level of this tile.
 */
void
save_image(cv::Mat img, settings_t settings, int x_coord, int y_coord, int z)
{
	// Ensure that folder exist
	std::string folderName = settings.output_folder + "/" + std::to_string(z) + "/" + std::to_string(x_coord);
	std::experimental::filesystem::create_directories(folderName);

	// Write final image to disk
	cv::imwrite(folderName + "/" + std::to_string(y_coord) + ".png", img);
}

int
main (int argc, char** argv)
{
	settings_t settings;

	parse_args(argc, argv, &settings);

	fill_tile_settings(&settings);

	verify_settings(&settings);

	LOG("Read image ...");
	cv::Mat img = cv::imread(settings.file, cv::IMREAD_UNCHANGED);

	if (img.empty())
	{
		ELOG("Could not open image '%s'", settings.file.c_str());
	    return EIO;
	}

	LOG("Start cuttig image ...");

	// Set Region of Interest
	cv::Rect roi;
	roi.x = settings.first_tile_x_px;
	roi.y = settings.first_tile_y_px;
	roi.width = settings.tile_size_px;
	roi.height = settings.tile_size_px;

	for (int z = settings.zoom_level; z >= 0; z--)
	{
		DLOG("y:%d, y:%d, w:%d, h:%d", roi.x, roi.y, roi.width, roi.height);

		for (int x_coord = settings.start_x_coord; roi.x <= img.size().width; x_coord++)
		{
			VLOG("Cut column Z:%d, X:%d", z, x_coord);

			for (int y_coord = settings.start_y_coord; roi.y <= img.size().height; y_coord++)
			{
				cv::Mat cropped_img(roi.width, roi.height, CV_8UC4, cv::Scalar(0, 0, 0, 0));
				crop(img, roi, &cropped_img);

				resize(cropped_img, cropped_img, cv::Size(settings.output_tile_size, settings.output_tile_size), 0, 0, cv::INTER_LINEAR_EXACT);

				save_image(cropped_img, settings, x_coord, y_coord, z);

				roi.y += roi.height;
			}

			roi.x += roi.width;
			roi.y = settings.first_tile_y_px;
		}

		roi.x = settings.first_tile_x_px;

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
		if (settings.start_x_coord % 2 != 0)
		{
			roi.x -= roi.width; 
			settings.start_x_coord--;
		}
		if (settings.start_y_coord % 2 != 0)
		{
			roi.y -= roi.height; 
			settings.start_y_coord--;
		}

		resize(img, img, cv::Size(std::max(1, img.size().width / 2), std::max(1, img.size().height / 2)), 0, 0, cv::INTER_AREA);

		// When the image is half as large, all distances and offsets have to
		// be as well
		roi.x /= 2;
		roi.y /= 2;

		// Because the loops are resetting the positions
		settings.first_tile_x_px = roi.x;
		settings.first_tile_y_px = roi.y;

		// Actually update the x and y coordinates
		settings.start_x_coord /= 2;
		settings.start_y_coord /= 2;
	}

	LOG("Done!");

	return 0;
}

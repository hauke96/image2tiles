#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <experimental/filesystem>
#include <regex>

#include <opencv2/opencv.hpp>

#include "math.cpp"
#include "logging.cpp"
#include "crop.cpp"

void
save_image(cv::Mat img, int x_coord, int y_coord, int z)
{
	// Ensure that folder exist
	std::string folderName = "out/" + std::to_string(z) + "/" + std::to_string(x_coord);
	std::experimental::filesystem::create_directories(folderName);

	// Write final image to disk
	cv::imwrite("out/" + std::to_string(z) + "/" + std::to_string(x_coord) + "/" + std::to_string(y_coord) + ".png", img);
}

typedef struct
{
	int x;
	int y;
	double lon;
	double lat;
} img_point_t;

typedef struct settings
{
	// Given from user via command line arguments
	img_point_t p1;
	img_point_t p2;
	int output_tile_size;
	std::string file;
	std::string output_folder;
	int zoom_level;

	// Calculated based on the arguments above
	int first_tile_x_px;
	int first_tile_y_px;
	int start_x_coord;
	int start_y_coord;
	float tile_size_px;
} settings_t;

void
parse_args(int argc, char** argv, settings_t *settings)
{
	// Default settings
	settings->output_tile_size = 256;
	settings->output_folder = "./out";

	// Regex for parsing the points
	std::string float_regex_str = "[+-]?[\\d]*\\.?[\\d]+";
	std::string int_regex_str = "[-+]?\\d+";
	// For example: --p1=100,20.123,100,64.123
	std::regex point_regex("(" + int_regex_str + "),(" + float_regex_str + "),(" + int_regex_str + "),(" + float_regex_str + ")");

	static struct option long_options[] = {
		{"zoom-level", required_argument, 0, 'z' },
		{"tile-size",  required_argument, 0, 't' },
		{"p1",         required_argument, 0, '1' },
		{"p2",         required_argument, 0, '2' },
		{"file",       required_argument, 0, 'f' },
		{"output",     required_argument, 0, 'o' },
		{"verbose",    no_argument,       0, 'v' },
		{"version",    no_argument,       0,  0  },
		{"debug",      no_argument,       0, 'd' },
		{0,            0,                 0,  0  }
	};
	
	while (1)
	{
		int option_index = 0;
		int c = getopt_long(argc, argv, "vdo:f:z:1:2:t:",
			long_options, &option_index);
		if (c == -1)
		{
			break;
		}

		switch (c)
		{
			case 0:
			{
				std::string opt = long_options[option_index].name;

				if (opt == "version")
				{
					LOG(VERSION);
					exit(0);
				}

				break;
			}
			case '1': // fall through
			case '2':
			{
				std::smatch matches;
				std::string arg_str(optarg);
				if (std::regex_search(arg_str, matches, point_regex))
				{
					img_point_t *p;
					if (c == '1')
					{
						p = &settings->p1;
					}
					else
					{
						p = &settings->p2;
					}
					p->x = atoi(matches.str(1).c_str());
					p->lon = std::stof(matches.str(2).c_str());
					p->y = atoi(matches.str(3).c_str());
					p->lat = std::stof(matches.str(4).c_str());
					DLOG("> %d", p->x);
					DLOG("> %f", p->lon);
					DLOG("> %d", p->y);
					DLOG("> %f", p->lat);
				}
				else
				{
					ELOG("Cannot parse point '%s'", optarg);
					LOG("A correct point option would be: --p1=1,-23,45,+6.78");
					LOG("(The + is optional and make sure there are no spaces)");
					exit(EINVAL);
				}

				break;
			}
			case 't':
				settings->output_tile_size = atoi(optarg);
				break;
			case 'v':
				VERBOSE = 1;
				break;
			case 'd':
				DEBUG = 1;
				VERBOSE = 1;
				break;
			case 'f':
				settings->file = optarg;
				break;
			case 'o':
				settings->output_folder = optarg;
				break;
			case 'z':
				settings->zoom_level = atoi(optarg);
				break;
			case '?':
				ELOG("Unrecognized option '%s'", optarg);
				exit(EINVAL);
		}
	}
}

void
fill_tile_settings(settings_t *settings)
{
	int zoom_level = settings->zoom_level;

	// W/E
	double p1_long = settings->p1.lon;
	int p1_x = settings->p1.x;
	// N/S
	double p1_lat = settings->p1.lat;
	int p1_y = settings->p1.y;

	// W/E
	double p2_long = settings->p2.lon;
	int p2_x = settings->p2.x;
	// N/S
	double p2_lat = settings->p2.lat;
	int p2_y = settings->p2.y;

	double pixel_per_long = abs(p1_x - p2_x) / abs(p1_long - p2_long);
	double pixel_per_lat = abs(p1_y - p2_y) / abs(p1_lat - p2_lat);

	DLOG("pixel per long: %f", pixel_per_long);
	DLOG("pixel per lat: %f", pixel_per_lat);
	
	double long_per_tile = 360 / pow(2, zoom_level);

	settings->tile_size_px = pixel_per_long * long_per_tile;

	DLOG("tile size: %f", settings->tile_size_px);

	// Determine most upper left given point. This is then used to calculate the tile and image offset within the tile of the origin.
	int x_px = std::min(p1_x, p2_x);
	double x_long;
	if (x_px == p1_x)
	{
		x_long = p1_long;
	}
	else
	{
		x_long = p2_long;
	}

	int y_px = std::min(p1_y, p2_y);
	double y_lat;
	if (y_px == p1_y)
	{
		y_lat = p1_lat;
	}
	else
	{
		y_lat = p2_lat;
	}

	double origin_long =  x_long - x_px / pixel_per_long;
	double origin_lat =  y_lat + y_px / pixel_per_lat;

	DLOG("x_long: %f", x_long);
	DLOG("y_lat: %f", y_lat);

	DLOG("origin_long: %f", origin_long);
	DLOG("origin_lat: %f", origin_lat);

	settings->start_x_coord = long_to_tile_x(origin_long, zoom_level);
	settings->start_y_coord = lat_to_tile_y(origin_lat, zoom_level);

	DLOG("tile x: %d", settings->start_x_coord);
	DLOG("tile y: %d", settings->start_y_coord);

	double origin_tile_long = tile_x_to_long(long_to_tile_x(origin_long, zoom_level), zoom_level);
	double origin_tile_lat =  tile_y_to_lat(lat_to_tile_y(origin_lat, zoom_level), zoom_level);

	DLOG("back to long: %f", origin_tile_long);
	DLOG("back to lat: %f", origin_tile_lat);

	settings->first_tile_x_px = (origin_tile_long - origin_long) * pixel_per_long;
	settings->first_tile_y_px = (origin_lat - origin_tile_lat) * pixel_per_lat;

	DLOG("x offset for first tile: %d", settings->first_tile_x_px);
	DLOG("y offset for first tile: %d", settings->first_tile_y_px);
}

int
main (int argc, char** argv)
{
	settings_t settings;

	parse_args(argc, argv, &settings);

	fill_tile_settings(&settings);

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

				save_image(cropped_img, x_coord, y_coord, z);

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

int DEBUG = 0;
int VERBOSE = 0;

#define VERSION "v0.1.0"

/**
 * Prints debuggin information. The DEBUG flag (setting with "--debug") must be set.
 */
#define DLOG(fmt, ...) \
	if (DEBUG) \
		printf("DEBUG: %s - " fmt "\n", __func__, ##__VA_ARGS__); \

/**
 * Printing normal information that might not be usable for most users. This is only done when the VERBOSE flag is set (using "--verbose").
 */
#define VLOG(fmt, ...) \
	if (VERBOSE) \
		printf(fmt "\n", ##__VA_ARGS__); \

/**
 * Printing text which is always visible.
 */
#define LOG(fmt, ...) \
	printf(fmt "\n", ##__VA_ARGS__); \

/**
 * Printing warning text which is always visible.
 */
#define WLOG(fmt, ...) \
	printf("WARNING: " fmt "\n", ##__VA_ARGS__); \

/**
 * Printing text to stderr.
 */
#define ELOG(fmt, ...) \
	fprintf(stderr, "ERROR: " fmt "\n", ##__VA_ARGS__); \

bool
req_confirm()
{
	char choice;
	bool success = false;

    while(true)
    {
        std::cout << "Proceed? [Y/n]" << std::endl;
        std::cin >> choice;

        if (choice == 'N' || choice == 'n')
		{
			success = false;
            break;
        }
		else if (choice == 'Y')
		{
			success = true;
			break;
		}
    }

	return success;
}

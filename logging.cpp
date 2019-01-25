int DEBUG = 0;
int VERBOSE = 0;

#define VERSION "v0.1.0"

#define DLOG(fmt, ...) \
	if (DEBUG) \
		printf("%s - " fmt "\n", __func__, ##__VA_ARGS__); \

#define VLOG(fmt, ...) \
	if (VERBOSE) \
		printf(fmt "\n", ##__VA_ARGS__); \

#define LOG(fmt, ...) \
	printf(fmt "\n", ##__VA_ARGS__); \

#define ELOG(fmt, ...) \
	fprintf(stderr, fmt "\n", ##__VA_ARGS__); \


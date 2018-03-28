#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "neven.h"

#define LOGE printf
#define doThrow(...)

struct neven_env *neven_create(const char *descfile, int w, int h, int maxFaces) {
	const int MAX_FILE_SIZE = 65536;
	void* initData = malloc( MAX_FILE_SIZE ); /* enough to fit entire file */
	int filedesc = open(descfile, O_RDONLY);
	if (filedesc == -1) {
		LOGE("ERROR: unable to load describe file\n");
		return NULL;
	}
	int initDataSize = read(filedesc, initData, MAX_FILE_SIZE);
	close(filedesc);

	btk_HSDK sdk = NULL;
	btk_SDKCreateParam sdkParam = btk_SDK_defaultParam();
	sdkParam.fpMalloc = malloc;
	sdkParam.fpFree = free;
	sdkParam.maxImageWidth = w;
	sdkParam.maxImageHeight = h;

	btk_Status status = btk_SDK_create(&sdkParam, &sdk);
	// make sure everything went well
	if (status != btk_STATUS_OK) {
		// XXX: be more precise about what went wrong
		doThrow(_env, "java/lang/OutOfMemoryError", NULL);
		return 0;
	}

	btk_HDCR dcr = NULL;
	btk_DCRCreateParam dcrParam = btk_DCR_defaultParam();
	btk_DCR_create( sdk, &dcrParam, &dcr );

	btk_HFaceFinder fd = NULL;
	btk_FaceFinderCreateParam fdParam = btk_FaceFinder_defaultParam();
	fdParam.pModuleParam = initData;
	fdParam.moduleParamSize = initDataSize;
	fdParam.maxDetectableFaces = maxFaces;
	status = btk_FaceFinder_create( sdk, &fdParam, &fd );
	btk_FaceFinder_setRange(fd, 1, w/2); /* set eye distance range */

	// make sure everything went well
	if (status != btk_STATUS_OK) {
		// XXX: be more precise about what went wrong
		doThrow(_env, "java/lang/OutOfMemoryError", NULL);
		return 0;
	}

	// free the configuration file
	free(initData);

	struct neven_env *env = malloc(sizeof(struct neven_env));
	env->fd = fd;
	env->sdk = sdk;
	env->dcr = dcr;
	env->width = w;
	env->height = h;

	return env;
}

void neven_destroy(struct neven_env *env) {
	btk_FaceFinder_close(env->fd);
	btk_DCR_close(env->dcr);
	btk_SDK_close(env->sdk);
	free(env);
}

int neven_detect(struct neven_env *env, void *bwbuffer) {
	// get the fields we need
	btk_HDCR hdcr = env->dcr;
	btk_HFaceFinder hfd = env->fd;
	int width = env->width;
	int height = env->height;

	// run detection
	btk_DCR_assignGrayByteImage(hdcr, bwbuffer, width, height);

	int numberOfFaces = 0;
	if (btk_FaceFinder_putDCR(hfd, hdcr) == btk_STATUS_OK) {
		numberOfFaces = btk_FaceFinder_faces(hfd);
	} else {
		LOGE("ERROR: Return 0 faces because error exists in btk_FaceFinder_putDCR.\n");
	}

	return numberOfFaces;
}

static void getFaceData(btk_HDCR hdcr, struct neven_face *fdata)
{
	btk_Node leftEye, rightEye;

	btk_DCR_getNode(hdcr, 0, &leftEye);
	btk_DCR_getNode(hdcr, 1, &rightEye);

	fdata->eyedist = (float)(rightEye.x - leftEye.x) / (1 << 16);
	fdata->midpointx = (float)(rightEye.x + leftEye.x) / (1 << 17);
	fdata->midpointy = (float)(rightEye.y + leftEye.y) / (1 << 17);
	fdata->confidence = (float)btk_DCR_confidence(hdcr) / (1 << 24);
}

void neven_get_face(struct neven_env *env, struct neven_face *face, int index) {
	btk_HDCR hdcr = env->dcr;
	btk_HFaceFinder hfd = env->fd;

	btk_FaceFinder_getDCR(hfd, hdcr);
	getFaceData(hdcr, face);
}


#include "../neven.h"
#include <stdio.h>
#include <cv.h>
#include <highgui.h>

static IplImage *image;

void load_image(char *file, void **buffer, int *width, int *height)
{
	image = cvLoadImage(file, CV_LOAD_IMAGE_GRAYSCALE);
	*width = image->width;
	*height = image->height;
	*buffer = image->imageData;
}

void draw_face(struct neven_face* face)
{
	CvPoint pt1, pt2;
	pt1.x = (int)(face->midpointx - face->eyedist/2.0);
	pt1.y = (int)(face->midpointy - face->eyedist/2.0);
	pt2.x = (int)(face->midpointx + face->eyedist/2.0);
	pt2.y = (int)(face->midpointy + face->eyedist/2.0);
	cvRectangle(image, pt1, pt2, cvScalar(255, 255, 0, 0), 3, 8, 0);
}

int main(int argc, char * argv[])
{
	void *buffer;
	int width, height, max_faces;

	if(argc < 3)
	{
		printf("Usage: ./test img_file desc_file\n");
		return 1;
	}
	max_faces = 150;
	load_image(argv[1], &buffer, &width, &height);

	struct neven_env *env = neven_create(argv[2], width, height, max_faces);
	int num_faces = neven_detect(env, buffer);
	int i;
	for (i = 0; i < num_faces; i++)
	{
		struct neven_face face;
		neven_get_face(env, &face, i);
		printf("face %d is at (%f, %f)\n", i, face.midpointx, face.midpointy);
		draw_face(&face);
	}
	neven_destroy(env);

	cvShowImage("result", image);
	cvWaitKey(0);
	cvSaveImage("result.jpg", image, 0);
	cvReleaseImage(&image);
	return 0;
}


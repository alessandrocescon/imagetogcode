//gcc imagetogcode.c -lgd -ljpeg -o imagetogcode.o

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "jpeglib.h"
#include "gd.h"

//params for G-CODE
float thickness = 2.50;
float pitch = 1.0;
int spindle = 0;
int safez = 10;
int movespeed = 50;
int workspeed = 50;
//float ystart = 0.0f;
/* we will be using this uninitialized pointer later to store raw, uncompressd image */

unsigned char *raw_image = NULL;
unsigned char *raw_image_tmp = NULL;

/* dimensions of the image we want to write */
int width = 0;
int height = 0;
int bytes_per_pixel = 3;   /* or 1 for GRACYSCALE images */
int color_space = JCS_RGB; /* or JCS_GRAYSCALE for grayscale images */ 
//int bytes_per_pixel = 1;   /* or 1 for GRACYSCALE images */
//int color_space = 1;
/**
 * read_jpeg_file Reads from a jpeg file on disk specified by filename and saves into the 
 * raw_image buffer in an uncompressed format.
* 
* \returns positive integer if successful, -1 otherwise
* \param *filename char string specifying the file name to read from
*
*/

int read_jpeg_file( char *filename,char *fileout,char *filescaled , char *widthmm, char *pitch){

/* these are standard libjpeg structures for reading(decompression) */
struct jpeg_decompress_struct cinfo;
struct jpeg_error_mgr jerr;
/* libjpeg data structure for storing one row, that is, scanline of an image */
JSAMPROW row_pointer[1];

FILE *infile = fopen( filescaled, "rb" );
FILE *outfile = fopen(fileout, "w");


//scale JPEG
  FILE *fp;
  gdImagePtr in, out;
  int ls = (int)(atof(widthmm)/atof(pitch));
  int ws,hs;
  fp = fopen(filename, "rb");
  if (!fp) {
    printf( "JPEG File read error GD \n" );
  }
  in = gdImageCreateFromJpeg(fp);
  fclose(fp);
  if (!in) {
    printf( "gdImageCreateFromJpeg error GD \n" );
  }
  int inw = in->sx;
  int inh = in->sy;
  if (inw < inh) {
    //portrait
    float ratio = (float)ls/(float)inh;
    hs=ls;
    ws=round(inw*ratio);
    //char dims[2048];
    //sprintf(dims,"%d-%d-%d,%d\n",inh,inw,hs,ws);
    //e.debug=add(e.debug,dims);
  }
  //landscape
  else {
    float ratio = (float)ls/(float)inw;
    ws=ls;
    hs=round(inh*ratio);
    //char dims[2048];
    //sprintf(dims,"%d-%d-%d,%d\n",inw,inh,ws,hs);
    //e.debug=add(e.debug,dims);
  }
  gdImageSetInterpolationMethod(in, GD_BILINEAR_FIXED);
  gdImageGrayScale(in);
  out = gdImageScale(in, ws, hs);
  if (!out) {
    printf("Can't scale image\n");
  }
  fp = fopen(filescaled, "wb");
  if (!fp) {
    printf("Failed opening image for save\n");
  }
  gdImageJpeg(out, fp, 100);
  fclose(fp);
  gdImageDestroy(in);
  gdImageDestroy(out);
  
 //end scale image 
  
  
//write headers

fprintf(outfile, "%%\n");
//go to safe Z
fprintf(outfile, "M3 S%d\nG21\nG00 Z%d F%d\n",spindle,safez,movespeed);
//go to start position
//fprintf(outfile, "G00 X%0.2f Y%0.2f F%d\n",xstart,ystart,movespeed);


unsigned long location = 0;
int i = 0;

if ( !infile )
{
    printf("Error opening jpeg file %s\n!", filename );
    return -1;
}
/* here we set up the standard libjpeg error handler */
cinfo.err = jpeg_std_error( &jerr );
/* setup decompression process and source, then read JPEG header */
jpeg_create_decompress( &cinfo );
/* this makes the library read from infile */
jpeg_stdio_src( &cinfo, infile );
/* reading the image header which contains image information */
jpeg_read_header( &cinfo, TRUE );
/* Uncomment the following to output image information, if needed. */
printf( "JPEG File Information: \n" );
printf( "Image width and height: %d pixels and %d pixels.\n", cinfo.image_width, cinfo.image_height );
printf( "Color components per pixel: %d.\n", cinfo.num_components );
printf( "Color space: %d.\n", cinfo.jpeg_color_space );
width = cinfo.image_width;
height = cinfo.image_height; 


/* Start decompression jpeg here */
jpeg_start_decompress( &cinfo );

/* allocate memory to hold the uncompressed image */
raw_image = (unsigned char*)malloc( cinfo.output_width*cinfo.output_height*cinfo.num_components );
/* now actually read the jpeg into the raw buffer */
row_pointer[0] = (unsigned char *)malloc( cinfo.output_width*cinfo.num_components );
/* read one scan line at a time */
while( cinfo.output_scanline < cinfo.image_height )
{
    jpeg_read_scanlines( &cinfo, row_pointer, 1 );
    for( i=0; i<cinfo.image_width*cinfo.num_components;i++) 
        raw_image[location++] = row_pointer[0][i];
}
int ip;
int jp; 
int impos=0;
int px=0;
int ypos=0;
for (ip=0; ip < cinfo.image_width; ip++){
 for (jp=0; jp < cinfo.image_height; jp++ ){
   ypos=jp;
   int totcomp=cinfo.image_width*cinfo.image_height*cinfo.num_components;
   int recim=totcomp-(jp*cinfo.image_width*cinfo.num_components)+(ip*cinfo.num_components);
   if (ip % 2) {
     ypos = cinfo.image_height-1-jp; 
     recim=(jp*cinfo.image_width*cinfo.num_components)+(ip*cinfo.num_components);
   }

   float zval = (thickness/(float)255)*(float)raw_image[recim];
   //penetrate
   if (px == 0) {
     fprintf(outfile, "G01 Z-%.2f F%d\n",zval,workspeed);
   }
   fprintf(outfile, "G01 X%.2f Y%.2f Z-%.2f F%d\n",(float)ip*atof(pitch),(float)ypos*atof(pitch),zval,workspeed);
   //G01 X62.849015 Y48.561147 Z-5.4 F95
   //printf("x:%.2f y:%.2f p:%d\n",(float)yloop/(float)pxmm,(float)ip/(float)pxmm,raw_image[px]);
   //printf("spessore: %.2f\n",zval);
   //printf("pixloop: %d  px: %d yloop: %d jp:%d\n\n\n",pixloop,px,yloop,jp);
   px++;
   // write to file


 }
}
//go to Z safe position and stop
fprintf(outfile, "G00 Z%d F%d\nM5\nG00 X0.00 Y0.00 F%d\nM2\n",safez,movespeed,movespeed);


/* wrap up decompression, destroy objects, free pointers and close open files */
jpeg_finish_decompress( &cinfo );
jpeg_destroy_decompress( &cinfo );
free( row_pointer[0] );
fclose( infile );
//closing G-CODE FILE
fprintf(outfile, "%%\n");
fclose( outfile );
/* yup, we succeeded! */
return 1;
}

int main(int argc, char** argv) {
if(argc !=10) {
printf("[executable] [grayscale jpeg image] [tool diam mm] [G-CODE out file name] [spindle motor speed] [safe Z mm] [move speed] [work speed]\n");
printf("imagetogcode.o gray.jpg 20 gray.gcode 2000 10 400 50 10.10 20.20\n");
exit(1);
}

spindle = atoi(argv[4]);
safez = atoi(argv[5]);
movespeed = atoi(argv[6]);
workspeed = atoi(argv[7]);
//ystart =  atof(argv[9]);
read_jpeg_file(argv[1],argv[3],argv[8],argv[9],argv[2]);

}

// FDFFile.m
//
// Copyright (c) 2011-12 
// Robarts Research Institute and 
// The University of Western Ontario
//
// Author: Andrew Curtis, Martyn Klassen


//
//
// This file is part of QLPluginFDF.
//
// QLPluginFDF is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// QLPluginFDF is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with QLPluginFDF.  If not, see <http://www.gnu.org/licenses/>.

// FDFFile provides an interface betweeen ObjectiveC and the low level FDF i/o
//  routines.  Generates images for previews.


#import "FDFFile.h"
#import "fdf_helpers.h"
#import "complex.h"

uint16_t byteswap2(uint16_t a)
{
   a = ((a & 0x00FF) << 8) | ((a & 0xFF00) >> 8);
   return a;
}

uint32_t byteswap4(uint32_t a)
{
   a = ((a & 0x000000FF) << 24) |
   ((a & 0x0000FF00) <<  8) |
   ((a & 0x00FF0000) >>  8) |
   ((a & 0xFF000000) >> 24);
   return a;
}

uint64_t byteswap8(uint64_t a)
{
   a = ((a & 0x00000000000000FFULL) << 56) | 
   ((a & 0x000000000000FF00ULL) << 40) | 
   ((a & 0x0000000000FF0000ULL) << 24) | 
   ((a & 0x00000000FF000000ULL) <<  8) | 
   ((a & 0x000000FF00000000ULL) >>  8) | 
   ((a & 0x0000FF0000000000ULL) >> 24) | 
   ((a & 0x00FF000000000000ULL) >> 40) | 
   ((a & 0xFF00000000000000ULL) >> 56);
   return a;
}

@implementation FDFFile



+ (id)createImage:(NSData *)fdfDataPtr 
	width:(long)w height:(long)h
{
	NSBitmapImageRep	*newBitmap;
	NSImage			*theImage;
	unsigned char		*bitmapData;
	long			size;

	size = w * h;
	newBitmap = [[NSBitmapImageRep alloc]
		initWithBitmapDataPlanes:0 
		pixelsWide:w pixelsHigh:h bitsPerSample:8
		samplesPerPixel:1 hasAlpha:FALSE isPlanar:FALSE
		colorSpaceName:NSDeviceWhiteColorSpace
		bytesPerRow:w bitsPerPixel:0];
	[newBitmap retain];
	bitmapData = (unsigned char *)[newBitmap bitmapData];
   

   memcpy((unsigned char *)bitmapData, (unsigned char *)[fdfDataPtr bytes], size);
	
	theImage = [[NSImage alloc] initWithSize:[newBitmap size]];
	[theImage addRepresentation:newBitmap];

	return theImage;
}




- (void)loadFDFFile:(FILE *)fp fdfHeader:(FDF_HEADER *)fh limitOne:(BOOL)onlyOne
{
	unsigned long	length;
	
   NSImage	*theImage = nil;
   NSMutableData *imageData = nil;
   NSMutableData *fdfData = nil;
   
   void *data_buffer;
   
   if(myImages != nil)
      [myImages release];
	
   length = 1;
   if (fh->rank == 3) {
      length = fh->matrix[2];
   }
   
   long width, height, dsize, dpoints;
   width = fh->matrix[0];
   height = fh->matrix[1];

   dpoints = width*height*length;
   dsize = dpoints*(fh->bits/8);
   if (fh->type == FDF_TYPE_COMPLEX) dsize*=2;

   fdfData = [[NSMutableData alloc] initWithLength:dsize];
   data_buffer = (void *)[fdfData mutableBytes];
   
   if (fdfReadData(fp, data_buffer) == EXIT_SUCCESS) {
      long convertPoints=dpoints;
      long termBits=fh->bits;
      if (fh->type == FDF_TYPE_COMPLEX) convertPoints *= 2;
      
      // Byte swapping check
      short const i=1;
      if ((*(char *)&i != 0) && fh->bigendian) {
         if (termBits == 16) {
            uint16_t *format = (uint16_t *)data_buffer;
            for (int i=0; i<dpoints; i++) {
               format[i] = byteswap2(format[i]);
            }
         }
         else if (termBits == 32) {
            uint32_t *format = (uint32_t *)data_buffer;
            for (int i=0; i<dpoints; i++) {
               format[i] = byteswap4(format[i]);
            }
         }
         else if (termBits == 64) {
            uint64_t *format = (uint64_t *)data_buffer;
            for (int i=0; i<dpoints; i++) {
               format[i] = byteswap8(format[i]);
            }
         }
         else if (termBits == 8) {
            // Do nothing
         }
         else {
            [fdfData release];
            return;
         }
      }
         
      // Convert to floats
      float *p = (float *) [fdfData mutableBytes];
      if (fh->storage == FDF_STORAGE_INTEGER) {
         if (termBits == 8) {
            int8_t *oldFormat = (int8_t *)data_buffer;
            for (int i=convertPoints-1; i>=0; i--) {
               p[i] = oldFormat[i];
            }
         }
         else if (termBits == 16) {
            int16_t *oldFormat = (int16_t *)data_buffer;
            for (int i=convertPoints-1; i>=0; i--) {
               p[i] = oldFormat[i];
            }
         }
         else if (termBits == 32) {
            int32_t *oldFormat = (int32_t *)data_buffer;
            for (int i=0; i<convertPoints; i++) {
               p[i] = oldFormat[i];
            }
         }
         else if (termBits == 64) {
            int64_t *oldFormat = (int64_t *)data_buffer;
            for (int i=0; i<convertPoints; i++) {
               p[i] = oldFormat[i];
            }
         }
         else {
            // Cannot process
            [fdfData release];
            return;
         }
      }
      else {
         if (termBits == 64) {
            // There is potential truncation that occurs here
            // This generally should not be an issue since raw data only 
            // coome in float format
            double *oldFormat = (double *)data_buffer;
            for (int i=0; i<convertPoints; i++) {
               p[i] = oldFormat[i];
            }
         }
         else if (termBits == 32) {
            // already floating point
            // convert to this because usually no conversion will be required
            // so this is the fastest
         }
         else {
            // Cannot process
            [fdfData release];
            return;
         }
      }
      
      // Take the magnitude of complex data
      if (fh->type == FDF_TYPE_COMPLEX) {
         complex float *oldFormat = (complex float *)data_buffer;
         for (int i=0; i < dpoints; i++) {
            p[i] = cabsf(oldFormat[i]);
         }
      }
  
      // Get max and min
      float max=FLT_MIN, min=FLT_MAX;
      for (int i = 0; i<dpoints; i++) {
         if (max < p[i]) {
            max = p[i];
         }
         if (min > p[i]) {
            min = p[i];
         }
      }
      
      // Alloc space for u16bit representation of float fdf data 
      imageData = [[NSMutableData alloc] initWithLength:dpoints*sizeof(char)];
      unsigned char *c = (unsigned char *)[imageData mutableBytes];
      for (int i = 0; i<dpoints; i++) {
         c[i] = (unsigned char)(((p[i]-min)/(max-min))*255);
      }
      
      // Thumbnails only show middle of volume
      if (onlyOne) {

         myImages = [[[NSMutableArray alloc] initWithCapacity:1] retain];
         dsize = width*height;
         
         int i=floor(length/2);
         NSRange drange = {dsize*i,dsize};
         
         theImage = [FDFFile createImage:[imageData subdataWithRange:drange]
                                   width:width height:height];
         
         
         [myImages insertObject:theImage atIndex:0];
         
      } else
      {


         myImages = [[[NSMutableArray alloc] initWithCapacity:length] retain];
         dsize = width*height;
         
         for (int i = 0; i<length; i++) {
            NSRange drange = {dsize*i,dsize};
            
            theImage = [FDFFile createImage:[imageData subdataWithRange:drange]
                                      width:width height:height];
            
            
            [myImages insertObject:theImage atIndex:i];
         }
      }
      
      
      [imageData release];
      
   }
   
   [fdfData release];
	
}


- (bool)parseFDFFile:(NSString *)thePath limitOne:(BOOL)onlyOne
{

   FDF_HEADER fdfHeader;
   char fname[1024];
	
	[thePath getCString:fname maxLength:1024 encoding:1];
	
   FILE *fp;
   fp = fopen(fname, "r");
   
   if(fdfReadHeader(fp, &fdfHeader) == EXIT_SUCCESS)
   {
      if ((fdfHeader.rank <= 3) && (fdfHeader.rank > 1)) {
         [self loadFDFFile:fp fdfHeader:&fdfHeader limitOne:onlyOne];
      }
   }
   
   fclose(fp);
   
	return YES;
}

- (NSArray *)getImages
{
	return myImages;
}

- (int)numFDFs
{
	return [myImages count];
}

- (id)getPath
{
	return filePath;
}

@end

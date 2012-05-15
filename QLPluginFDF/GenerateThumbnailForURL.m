// GenerateThumbnailForURL.m
//
// Copyright (c) 2006-12 
// Robarts Research Institute and 
// The University of Western Ontario
//
// Author: Andrew Curtis, Martyn Klassen
//

// Portions of this file based on BrushViewQL by Laura Dickey,
// brushview.sourceforge.net

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




#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#include <QuickLook/QuickLook.h>
#include <Foundation/Foundation.h>
#include "FDFFile.h"

#define DIMENSIONS_HEIGHT	20.0
#define SPACING_WIDTH		4.0
#define SPACING_HEIGHT		4.0




void
DrawOneFDF(FDFFile *FDFFile, NSGraphicsContext *context, float fdfSize,
        NSSize canvasSize);


/* -----------------------------------------------------------------------------
    Generate a thumbnail for file

   This function's job is to create thumbnail for designated file as fast as possible
   ----------------------------------------------------------------------------- */

OSStatus GenerateThumbnailForURL(void *thisInterface, QLThumbnailRequestRef thumbnail, CFURLRef url, CFStringRef contentTypeUTI, CFDictionaryRef options, CGSize maxSize)
{
   NSString	  *fileName;
	FDFFile	*theFDF;
	NSSize		canvasSize;
	int		numFDFs;
	float		fdfSize;
	CGContextRef	cgContext;

   NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
   
	
	NSGraphicsContext	*context;

	fileName = [(NSURL *)url path];
	theFDF = [[FDFFile alloc] init];
	if (![theFDF parseFDFFile:fileName limitOne:true]) {
      [theFDF release];
      [pool release];
		return noErr; // wrong, but live with it for now
	}
	fdfSize = 128.0;
	numFDFs = [theFDF numFDFs];
	if (numFDFs == 0) {
      // avoid a divide by 0 problem later
		return noErr;
	}

	canvasSize = NSMakeSize((fdfSize ) , 
                           (fdfSize ) );
   
	cgContext = QLThumbnailRequestCreateContext(
                                             thumbnail, *(CGSize *)&canvasSize, true, NULL);
	if (cgContext) {
		context = [NSGraphicsContext graphicsContextWithGraphicsPort:
                 (void *)cgContext flipped:NO];
		if (context) {
			DrawOneFDF(theFDF, context, fdfSize, canvasSize);
		}
		QLThumbnailRequestFlushContext(thumbnail, cgContext);
		CFRelease(cgContext);
	}
   [pool release];
	return noErr;

}

void CancelThumbnailGeneration(void* thisInterface, QLThumbnailRequestRef thumbnail)
{
    // implement only if supported
}


void
DrawOneFDF(FDFFile *FDFFile, NSGraphicsContext *context, float fdfSize,
        NSSize canvasSize)
{
   // Drawing code here.
	NSImage	*myImage;
	NSArray	*images;
	NSDictionary	*strAttribs;
	float	totalWidth, totalHeight, maxHeight, maxWidth, maxLineHeight;
	float	currPosX, currPosY, scaleFactor, scaleX, scaleY;
	NSSize	mySize;
	int	numFDFs;
	int	cols, rows;
	
	//NSRect	myFrame, myBounds;
   
	images = [FDFFile getImages];
	if (images == nil) {
		return;
	}
   
	[NSGraphicsContext saveGraphicsState];
	[NSGraphicsContext setCurrentContext:context];
   
	totalWidth = 0.0;
   totalHeight = 0.0;
	maxHeight = 0.0;
	maxWidth = 0.0;
	mySize = canvasSize;
   
	strAttribs = [NSDictionary dictionaryWithObjectsAndKeys:
                 [NSFont systemFontOfSize:16], NSFontAttributeName,
                 [NSColor whiteColor], NSForegroundColorAttributeName,
                 nil];
	
	if (fdfSize > mySize.width) {
		cols = 1;
	} else {
		cols = (int)(mySize.width / fdfSize);
	}
	if (cols == 0) {
		cols = 1;
	}
	numFDFs = [images count];
	rows = numFDFs / cols;
	if ((numFDFs % cols) != 0) {
		++rows;
	}
   
	for (myImage in images) {
		totalWidth += [myImage size].width;
		totalHeight += [myImage size].height;
		if ([myImage size].height > maxHeight) {
			maxHeight = [myImage size].height;
		}
		if ([myImage size].width > maxWidth) {
			maxWidth = [myImage size].width;
		}
	}
	
   
	currPosX = 0;
	currPosY = mySize.height;
	
	totalWidth = 0.0;
	maxLineHeight = 0.0;
	for (myImage in images) {
		NSSize	imgSize = [myImage size];
		NSRect	centerRect;
		
		scaleX = 1.0;
		scaleY = 1.0;
		scaleFactor = 1.0;
		
		if ((currPosX + fdfSize) > mySize.width) {
			currPosX = 0;
			currPosY -= (fdfSize +
                      + SPACING_HEIGHT);
		}
		//if (imgSize.width > fdfSize) {
      scaleX = fdfSize / imgSize.width;
		//}
		//if (imgSize.height > fdfSize) {
      scaleY = fdfSize / imgSize.height;
		//}
		if (scaleX < scaleY) {
			scaleFactor = scaleX;
		} else {
			scaleFactor = scaleY;
		}
      
		[[NSColor blackColor] set];
      // Debug: (or if you like borders) draw bounding rect.
#if 0
		[NSBezierPath strokeRect:NSMakeRect(currPosX, (currPosY - fdfSize),
                                          fdfSize, fdfSize)]; 
#endif
		
		
      
      
		/* center the image inside the rectangle  */
		centerRect.origin.x = currPosX;
		centerRect.origin.x += 
      (fdfSize - (scaleFactor * imgSize.width)) / 2.0;
		centerRect.origin.y = currPosY - fdfSize;
		centerRect.origin.y += 
      (fdfSize - (scaleFactor * imgSize.height)) / 2.0;
		centerRect.size.width = scaleFactor * imgSize.width;
		centerRect.size.height = scaleFactor * imgSize.height;
		
		[myImage drawInRect:centerRect
                 fromRect:NSZeroRect 
                operation:NSCompositeCopy fraction:1.0];
      
		totalWidth += [myImage size].width;
		currPosX += (fdfSize + SPACING_WIDTH);
	}
   /* // Optional: draw info string
    sizeStr = [NSString stringWithFormat:@"%d x %d x %d",
    (int)imgSize.width, (int)imgSize.height];
    strSize = [sizeStr sizeWithAttributes:strAttribs];
    strLoc.x = ((fdfSize - strSize.width) / 2.0) +
    currPosX;
    strLoc.y = (currPosY - fdfSize) ;
    [[NSColor whiteColor] set];
    */
   //[sizeStr drawAtPoint:strLoc withAttributes:strAttribs];
	
   
   
	[NSGraphicsContext restoreGraphicsState];
   
}

// FDFFile.h
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



#import <Cocoa/Cocoa.h>
#import "fdf_helpers.h"

@interface FDFFile : NSObject {

	NSMutableDictionary	*sections;
	NSString		*filePath;
	NSMutableArray		*myImages;	
}

+ (id)createImage:(NSData *)fdfDataPtr
            width:(long)w height:(long)h;
- (void)loadFDFFile:(FILE *)fp fdfHeader:(FDF_HEADER *)fh limitOne:(BOOL)onlyOne;
- (bool)parseFDFFile:(NSString *)thePath limitOne:(BOOL)onlyOne;
- (NSArray *)getImages;
- (id)getPath;
- (int)numFDFs;

@end

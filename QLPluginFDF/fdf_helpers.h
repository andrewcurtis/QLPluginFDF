// fdf_helpers.h
//
// Copyright (c) 2006-12 
// Robarts Research Institute and 
// The University of Western Ontario
//
// Author: Martyn Klassen, Andrew Curtis

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

// Provides basic fdf i/o support.


// Data type definitions for groking fdf file and headers.

#include <stdio.h>

#ifndef _fdf_helpers_H
#define	_fdf_helpers_H

#ifdef	__cplusplus
extern "C" {
#endif


typedef enum
{
   FDF_RANK_NONE  = 0,
   FDF_RANK_VOXEL = 1,
   FDF_RANK_1D    = 2,
   FDF_RANK_2D    = 3,
   FDF_RANK_3D    = 4
}  FDF_SPATIAL_RANK;

typedef enum
{
   FDF_STORAGE_INTEGER  = 0,
   FDF_STORAGE_FLOAT    = 1
} FDF_STORAGE;

typedef enum
{
   FDF_TYPE_REAL        = 0,
   FDF_TYPE_IMAGINARY   = 1,
   FDF_TYPE_ABSVAL      = 2,
   FDF_TYPE_COMPLEX     = 3,
   FDF_TYPE_PHASE       = 4
} FDF_TYPE;

typedef enum
{
   FDF_ORDINATE_INTENSITY  = 0,
   FDF_ORDINATE_TIME       = 1,
   FDF_ORDINATE_ANGLE      = 2
} FDF_ORDINATE;

typedef enum
{
   FDF_ABSCISSA_HZ   = 0,
   FDF_ABSCISSA_S    = 1,
   FDF_ABSCISSA_CM   = 2,
   FDF_ABSCISSA_CMS  = 3,
   FDF_ABSCISSA_CMS2 = 4,
   FDF_ABSCISSA_DEG  = 5,
   FDF_ABSCISSA_PPM1 = 6,
   FDF_ABSCISSA_PPM2 = 7,
   FDF_ABSCISSA_PPM3 = 8
} FDF_ABSCISSA;

typedef struct
{
   long              rank;
   FDF_SPATIAL_RANK  spatial_rank;
   FDF_STORAGE       storage;
   long              bits;
   FDF_TYPE          type;
   long              matrix[3];
   FDF_ABSCISSA      abscissa[3];
   FDF_ORDINATE      ordinate;
   float             span[3];
   float             origin[3];
   char              nucleus[3][8];
   float             nucfreq[3];
   long              numberNuclei;
   float             location[3];
   float             roi[3];
   float             orientation[9];
   int               bigendian;
   long              checksum;
   long              slice_no;
   long              slices;
   long              echo_no;
   long              echoes;
   long              coil;
   long              coils;
   float              array_index;
   long              array_dim;
   long              display_order;
} FDF_HEADER;

int fdfReadHeader(FILE *fp, FDF_HEADER *fdf_header);
int fdfReadData(FILE *fp, void *data);


#ifdef	__cplusplus
}
#endif

#endif	/* _fdf_helpers_H */


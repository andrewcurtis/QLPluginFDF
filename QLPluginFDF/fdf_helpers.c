// fdf_helpers.c
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "fdf_helpers.h"


/*
 * Check for file existance
 */
static int fileExist(char *filename) {
   struct stat buf;
   int i = stat(filename, &buf);
   if (i == 0)
      return(1);
   else
      return(0);
}  

static int copyfile(char *fromfile, char *tofile)
{
	FILE *ifp, *ofp;
	size_t r;
   char buf[1024];

	if((ifp = fopen(fromfile, "r")) == NULL) {
      return EXIT_FAILURE;
   }
	if((ofp = fopen(tofile, "w")) == NULL) {
      fclose(ifp);
      return EXIT_FAILURE;
   }

   while((r = fread(buf, 1, sizeof(buf), ifp)) > 0) {
		if (fwrite(buf, 1, r, ofp) != r) {
         fprintf( stderr, "File %s: Function %s: Line %d: Unable to copy %s to %s\n",
               __FILE__, __FUNCTION__, __LINE__, fromfile, tofile);
         fclose(ifp);
         fclose(ofp);
         remove(tofile);
         return(EXIT_FAILURE);
      }
   }
   
	fclose(ifp);
	fclose(ofp);

	return(EXIT_SUCCESS);
}


int fdfReadData(FILE *fp, void *data)
{
   // NOTE: Varian may buffer the space between the header and data
   // with multiple null character which would make this routine fail
   char  res = '\1';
   long  pos;

   fseek(fp, 0, SEEK_END);
   pos = ftell(fp);
   fseek(fp, 0, SEEK_SET);

   while (res != '\0')
      fscanf(fp, "%c", &res);

   pos -= ftell(fp);

   size_t readData;
   if ((readData = fread(data, 1, (size_t) pos, fp)) != (size_t) pos) {
      fprintf( stderr, "File %s: Function %s: Line %d: Unable to read %ld bytes, read %ld\n",
            __FILE__, __FUNCTION__, __LINE__, pos, readData);
      return(EXIT_FAILURE);
   }

   return(EXIT_SUCCESS);
}


int fdfReadHeader(FILE *fp, FDF_HEADER *fdf_header)
{
   int i;
   char test[5000];
   char name[5000];

   // Default is bigendian
   fdf_header->bigendian = 1;

   // Go to the beginning of the file
   fseek(fp, 0, SEEK_SET);
   
   // Read in teh magic number line
   fscanf(fp, "#!/usr/local/fdf/startup");

   fscanf(fp, "%c", test);

   while (test[0] != '\0') {
      while ((test[0] != (char) 105) && (test[0] != (char) 99) && (test[0] != (char) 102)) {
         fscanf(fp, "%c", test);
         if (test[0] == '\0') {
            return(EXIT_SUCCESS);
         }
      }
      fseek(fp, -1, SEEK_CUR);

      // The type is the first string
      fscanf(fp, "%[^* ]", test);

      // Eat any *
      fscanf(fp, "%[* ]", test);
   
      // Get the field name
      fscanf(fp, "%[^[ =]", name);

      // Eat everything until an equal sign is reached
      fscanf(fp, "%[^=]", test);

      // If an area is expected, remove all data up to {
      if (strchr(test, '[') != NULL)
         fscanf(fp, "%[^{]", test);

      // Remove the starting { or =
      fscanf(fp, "%c", test);

      if (strcmp(name, "rank") == 0) {
         fscanf(fp, "%ld", &(fdf_header->rank));
      }
      else if (strcmp(name, "spatial_rank") == 0) {
         fscanf(fp, "%[^\"]", test);
         fscanf(fp, "\"%[^\"]\"", test);
         if (strcmp(test, "none") == 0)
            fdf_header->spatial_rank = FDF_RANK_NONE;
         else if (strcmp(test, "voxel") == 0)
            fdf_header->spatial_rank = FDF_RANK_VOXEL;
         else if (strcmp(test, "1dfov") == 0)
            fdf_header->spatial_rank = FDF_RANK_1D;
         else if (strcmp(test, "2dfov") == 0)
            fdf_header->spatial_rank = FDF_RANK_2D;
         else if (strcmp(test, "3dfov") == 0)
            fdf_header->spatial_rank = FDF_RANK_3D;
         else {
            fprintf( stderr, "File %s: Function %s: Line %d: Unrecognized spatial rank = \"%s\"\n", __FILE__,
                  __FUNCTION__, __LINE__, test);
            return(EXIT_FAILURE);
         }
      }
      else if (strcmp(name, "storage") == 0) {
         fscanf(fp, "%[^\"]", test);
         fscanf(fp, "\"%[^\"]\"", test);
         if (strcmp(test, "integer") == 0)
            fdf_header->storage = FDF_STORAGE_INTEGER;
         else if (strcmp(test, "float") == 0)
            fdf_header->storage = FDF_STORAGE_FLOAT;
         else { 
            fprintf( stderr, "File %s: Function %s: Line %d: Unrecognized storage type = \"%s\"\n", __FILE__,
                  __FUNCTION__, __LINE__, test);
            return(EXIT_FAILURE);
         }
      }
      else if (strcmp(name, "bits") == 0) {
         fscanf(fp, "%ld", &(fdf_header->bits));
      }
      else if (strcmp(name, "type") == 0) {
         fscanf(fp, "%[^\"]", test);
         fscanf(fp, "\"%[^\"]\"", test);
         if (strcmp(test, "real") == 0)
            fdf_header->type = FDF_TYPE_REAL;
         else if (strcmp(test, "imag") == 0)
            fdf_header->type = FDF_TYPE_IMAGINARY;
         else if (strcmp(test, "absval") == 0)
            fdf_header->type = FDF_TYPE_ABSVAL;
         else if (strcmp(test, "complex") == 0)
            fdf_header->type = FDF_TYPE_COMPLEX;
         else if (strcmp(test, "phase") == 0)
            fdf_header->type = FDF_TYPE_PHASE;
         else {
            fprintf( stderr, "File %s: Function %s: Line %d: Unrecognized image type = \"%s\"\n", __FILE__,
                  __FUNCTION__, __LINE__, test);
            return(EXIT_FAILURE);
         }
      }
      else if (strcmp(name, "matrix") == 0) {
         for (i=0; i<(int)fdf_header->rank; i++)
            fscanf(fp, "%ld,", fdf_header->matrix+i);
      }
      else if (strcmp(name, "abscissa") == 0) {
         for (i=0; i<(int)fdf_header->rank; i++) {
            fscanf(fp, "%[^\"]", test);
            fscanf(fp, "\"%[^\"]\"", test);
            if (strcmp(test, "hz") == 0)
               fdf_header->abscissa[i] = FDF_ABSCISSA_HZ;
            else if (strcmp(test, "s") == 0)
               fdf_header->abscissa[i] = FDF_ABSCISSA_S;
            else if (strcmp(test, "cm") == 0)
               fdf_header->abscissa[i] = FDF_ABSCISSA_CM;
            else if (strcmp(test, "cm/s2") == 0)
               fdf_header->abscissa[i] = FDF_ABSCISSA_CMS2;
            else if (strcmp(test, "cm/s") == 0)
               fdf_header->abscissa[i] = FDF_ABSCISSA_CMS;
            else if (strcmp(test, "deg") == 0)
               fdf_header->abscissa[i] = FDF_ABSCISSA_DEG;
            else if (strcmp(test, "ppm1") == 0)
               fdf_header->abscissa[i] = FDF_ABSCISSA_PPM1;
            else if (strcmp(test, "ppm2") == 0)
               fdf_header->abscissa[i] = FDF_ABSCISSA_PPM2;
            else if (strcmp(test, "ppm3") == 0)
               fdf_header->abscissa[i] = FDF_ABSCISSA_PPM3;
            else { 
               fprintf( stderr, "File %s: Function %s: Line %d: Unrecognized abscissa[%d] = \"%s\"\n", __FILE__,
                     __FUNCTION__, __LINE__, i, test);
               return(EXIT_FAILURE);
            }
         }
      }
      else if (strcmp(name, "ordinate") == 0) {
         fscanf(fp, "%[^\"]", test);
         fscanf(fp, "\"%[^\"]\"", test);
         if (strcmp(test, "intensity") == 0)
            fdf_header->ordinate = FDF_ORDINATE_INTENSITY;
         else if (strcmp(test, "s") == 0)
            fdf_header->ordinate = FDF_ORDINATE_TIME;
         else if (strcmp(test, "deg") == 0)
            fdf_header->ordinate = FDF_ORDINATE_ANGLE;
         else {
            fprintf( stderr, "File %s: Function %s: Line %d: Unrecognized ordinate = \"%s\"\n", __FILE__,
                  __FUNCTION__, __LINE__, test);
            return(EXIT_FAILURE);
         }
      }
      else if (strcmp(name, "span") == 0) {
         for (i=0; i<(int)fdf_header->rank; i++)
            fscanf(fp, "%e,", fdf_header->span+i);
      }
      else if (strcmp(name, "origin") == 0) {
         for (i=0; i<(int)fdf_header->rank; i++)
            fscanf(fp, "%e,", fdf_header->origin+i);
      }
      else if (strcmp(name, "nucleus") == 0) {
         fscanf(fp, "%[^\"]", test);
         test[0]=',';
         fdf_header->numberNuclei = 0;
         i = 0;
         while ((test[0] == ',') && (i<3)) {
            fdf_header->numberNuclei++;
            fscanf(fp, "\"%[^\"]\"", fdf_header->nucleus[i]);
            i++;
            fscanf(fp, "%c", test);
         }
      }
      else if (strcmp(name, "nucfreq") == 0) {
         test[0] = ',';
         fdf_header->numberNuclei = 0;
         i = 0;
         while ((test[0] == ',') && (i<3)) {
            fdf_header->numberNuclei++;
            fscanf(fp, "%e", &(fdf_header->nucfreq[i]));
            i++;
            fscanf(fp, "%c", test);
         }
      }
      else if (strcmp(name, "location") == 0) {
         for (i=0; i < 3; i++)
            fscanf(fp, "%e,", fdf_header->location+i);
      }
      else if (strcmp(name, "roi") == 0) {
         for (i=0; i < 3; i++)
            fscanf(fp, "%e,", fdf_header->roi+i);
      }
      else if (strcmp(name, "orientation") == 0) {
         for (i=0; i<9; i++)
            fscanf(fp, "%e,", fdf_header->orientation+i);
      }
      else if (strcmp(name, "bigendian") == 0) {
         fscanf(fp, "%d", &(fdf_header->bigendian));
      }

      fscanf(fp, "%c", test);

      while (test[0] != ';')
         fscanf(fp, "%c", test);
   }

   return(EXIT_SUCCESS);
}



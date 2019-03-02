// $Id$
/* mkhexgrid -- generates hex grids
 * Copyright (C) 2006 Joel Uckelman
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <exception>
#include <stdexcept>
#include <string>
#include <sstream>
using namespace std;

#include <gd.h>

#include "grid.h"

// globals for PNG drawing
gdImagePtr im;
double cx,        // current x 
       cy;        // current y
int bc,           // background color
    gc,           // grid color
    tc,           // text color
    cc;           // center color

void Grid::draw_png()
{
   // setup the image for GD
   im = gdImageCreateTrueColor(int(round(iw)), int(round(ih)));
   
   // allocate colors
   unsigned int c;
   istringstream s;

   // background color
   s.str(bg_color);
   s >> hex >> c;
   if (s.fail() || !s.eof()) throw runtime_error("bad background color");
   bc = gdImageColorExactAlpha(im, (c >> 16) & 0xff, 
                                   (c >>  8) & 0xff,
                                    c        & 0xff,
                                   (unsigned int)bg_opacity);
   // grid color
   s.clear();
   s.str(grid_color);
   s >> hex >> c;
   if (s.fail() || !s.eof()) throw runtime_error("bad grid color");
   gc = gdImageColorExactAlpha(im, (c >> 16) & 0xff, 
                                   (c >>  8) & 0xff,
                                    c        & 0xff,
                                   (unsigned int)grid_opacity);
   // text color
   s.clear();
   s.str(coord_color);
   s >> hex >> c;
   if (s.fail() || !s.eof()) throw runtime_error("bad coordinate color");
   tc = gdImageColorExactAlpha(im, (c >> 16) & 0xff,
                                   (c >>  8) & 0xff,
                                    c        & 0xff,
                                   (unsigned int)coord_opacity);

   // center color
   s.clear();
   s.str(center_color);
   s >> hex >> c;
   if (s.fail() || !s.eof()) throw runtime_error("bad center color");
   cc = gdImageColorExactAlpha(im, (c >> 16) & 0xff,
                                   (c >>  8) & 0xff,
                                    c        & 0xff,
                                   (unsigned int)center_opacity);

   // anti-alias to the alpha channel if our background is transparent
   if (bg_opacity == 127) {
      gdImageSaveAlpha(im, 1);
      gdImageAlphaBlending(im, 0);
   }

   // fill background
   if (matte) {
      gdImageFilledRectangle(im, 0, 0, gdImageSX(im)-1, gdImageSY(im)-1,
                             gdImageColorExactAlpha(im, 255, 255, 255 ,0));
      gdImageFilledRectangle(im, int(mleft), int(mtop),
                                 int(gdImageSX(im)-1-mright),
                                 int(gdImageSY(im)-1-mbottom),
                             bc);
   }
   else 
      gdImageFilledRectangle(im, 0, 0, gdImageSX(im)-1, gdImageSY(im)-1, bc);
 
   // NB: apparently thickness does nothing when AA is on
   // FIXME: new AA line drawing doesn't respect thickness
   gdImageSetThickness(im, (unsigned int)grid_thickness);

   if (lowfirstcol) {
      cx = mleft+0.25*hw, 
      cy = mtop+0.5*hh;

      // outline
      for (int n = 1; n <= 2*cols-cols%2; ++n)
         side_png(n);
      for (int n = cols%2; n <= 2*rows-2+cols%2; ++n)
         edge_png(n);
      for (int n = 3+(cols%2); n <= 2*cols+1+2*(cols%2); ++n)
         side_reverse_png(n);
      for (int n = 0; n <= 2*rows-1; ++n)
         edge_reverse_png(n);

      double oy = cy;

      for (int r = 0; r < rows - 1; ++r) {
         // bottoms
         cx = mleft+0.75*hw;
         cy = oy;
         for (int n = 0; n <= 2*cols-3; ++n) side_skip_png(n);

         // tops
         cx = mleft+0.25*hw;
         cy = oy += hh;
         for (int n = 1; n <= 2*cols - 1; ++n) side_png(n);
      }

      cx = mleft+0.75*hw;
      cy = oy;
      for (int n = 0; n <= 2*cols-3; ++n) side_skip_png(n);
   }
   else {
      cx = mleft;
      cy = mtop+0.5*hh;    

      // outline
      for (int n = 2; n <= 2*cols+2-(cols+1)%2; ++n)
         side_png(n);
      for (int n = 1-cols%2; n <= 2*rows-1-cols%2; ++n)
         edge_png(n);
      for (int n = 3*(cols%2); n <= 2*cols-1+2*(cols%2); ++n)
         side_reverse_png(n);
      for (int n = 0; n <= 2*rows-2; ++n)
         edge_reverse_png(n);

      double oy = cy = mtop+hh;
      
      for (int r = 0; r < rows - 1; ++r) {
         // bottoms
         cx = mleft+0.75*hw;
         for (int n = 2; n <= 2*cols-1; ++n) side_skip_png(n);

         // tops
         cx = mleft+0.25*hw;
         cy = oy;
         for (int n = 3; n <= 2*cols + 1; ++n) side_png(n);

         cy = oy += hh;
      }
      
      cx = mleft+0.75*hw;
      cy = oy;
      for (int n = 2; n <= 2*cols-1; ++n) side_skip_png(n);
   } 

   // draw centers
   if (center_style != Centerless) {
      switch (center_style) {
      case Cross:
         for (int r = 0; r < rows; ++r)
            for (int c = 0; c < cols; ++c)
               cross_png(c, r);
         break;
      case Dot:
         for (int r = 0; r < rows; ++r)
            for (int c = 0; c < cols; ++c)
               dot_png(c, r);
         break;
      default:
         break;
      }
   }

   if (coord_display) {
      // draw coordinates
      if (!antialiased) tc = -tc;  // text is AA unless the color is negative

#ifdef WIN32
      // our precompiled GD DLL doesn't have the right font path for
      // Windows so we set the environment variable it uses here
      putenv("GDFONTPATH=C:\\Windows\\Fonts");
#endif

      gdFTUseFontConfig(1);

      char fn[coord_font.length()+1];
      memset(fn, '\0', coord_font.length()+1);
      coord_font.copy(fn, coord_font.length());

      double bcos = cos(coord_bearing*rad),
             bsin = sin(coord_bearing*rad);

      for (int r = 0; r < rows; ++r) {
         if ((r+coord_rstart) % coord_rskip) continue;
         for (int c = 0; c < cols; ++c) {
            if ((c+coord_cstart) % coord_cskip) continue;

            int cc = 0, cr = 0;

            switch (coord_origin) {
            case UpperLeft:
               cc = c+coord_cstart;
               cr = r+coord_rstart;
               break;
            case UpperRight:
               cc = cols-c-1+coord_cstart;
               cr = r+coord_rstart;
               break;
            case LowerLeft:
               cc = c+coord_cstart;
               cr = rows-r-1+coord_rstart;
               break;
            case LowerRight:
               cc = cols-c-1+coord_cstart;
               cr = rows-r-1+coord_rstart;
               break;
            }

            int cnum = cc, rnum = cr;
            if (coord_order == RowsFirst) swap(cnum, rnum);

            ostringstream s;
            s << coord_fmt_pre;

            switch (coord_first_style) {
            case NoCoord:
               break;
            case Number:
               if (coord_first_width) s << setw(coord_first_width);
               if (coord_first_fill) s << setfill('0');
               s << cnum;
               break;
            case Alpha:
               s << alpha(cnum);
               break;
            case AlphaTally:
               s << alpha_tally(cnum);
               break;
            }

            s << coord_fmt_inter;
   
            switch (coord_second_style) {
            case NoCoord:
               break;
            case Number:
               if (coord_second_width) s << setw(coord_second_width);
               if (coord_second_fill) s << setfill('0');
               s << rnum;
               break;
            case Alpha:
               s << alpha(rnum);
               break;
            case AlphaTally:
               s << alpha_tally(rnum);
               break;
            }

            double x = (c*0.75 + 0.5)*hw+coord_dist*bcos;
            double y = (r + 0.5*(1 + (c+lowfirstcol)%2))*hh+coord_dist*bsin;

//            gdImageLine(im, round(x), round(y), round(x-coord_dist*bcos), round(y-coord_dist*bsin), 0);

            char ct[s.str().length()+1];
            memset(ct, '\0', s.str().length()+1);
            s.str().copy(ct, s.str().length());
   
            char *err;
            int br[8];        
  
            err = gdImageStringFT(NULL, br, tc, fn, coord_size,
                                  coord_tilt*rad, 0, 0, ct);
            if (err) throw runtime_error(err);
   
            int l = min(br[0], min(br[2], min(br[4], br[6]))),
                r = max(br[0], max(br[2], max(br[4], br[6]))),
                t = min(br[1], min(br[3], min(br[5], br[7]))),
                b = max(br[1], max(br[3], max(br[5], br[7])));

            int w = r-l+1,
                h = b-t+1;

            // we do this to avoid a bounding box bug in gd-2.0.33 
            gdImagePtr tmp = gdImageCreateTrueColor(w, h);
            gdImageSaveAlpha(tmp, 1);
            gdImageAlphaBlending(tmp, 0);
            gdImageFilledRectangle(tmp, 0, 0, w-1, h-1, 
               gdImageColorExactAlpha(tmp, 0, 0, 0, 127));

            /*
             * works, but the min/max stuff above is simpler
             * 
            // find the offset of the upper left corner
            // of the (orthogonal) bounding box
            int bx = 0, by = 0;
            if (0 <= coord_tilt && coord_tilt < 90) {
               bx = -br[6];
               by = -br[5];
            }
            else if (90 <= coord_tilt && coord_tilt < 180) {
               bx = -br[4];
               by = -br[3];
            }
            else if (180 <= coord_tilt && coord_tilt < 270) {
               bx = -br[2];
               by = -br[1];
            }
            else if (270 <= coord_tilt && coord_tilt < 360) {
               bx = -br[0];
               by = -br[7];
            }
            */

            /*
            // draw box around coordinates for debugging
            gdPoint p[4];
            p[0].x = bx+br[0];
            p[0].y = by+br[1];
            p[1].x = bx+br[2];
            p[1].y = by+br[3];
            p[2].x = bx+br[4];
            p[2].y = by+br[5];
            p[3].x = bx+br[6];
            p[3].y = by+br[7];

            cerr << p[0].x << ',' << p[0].y << ' '
                 << p[1].x << ',' << p[1].y << ' '
                 << p[2].x << ',' << p[2].y << ' '
                 << p[3].x << ',' << p[3].y << ' '
                 << bx << ',' << by << endl;

            gdImagePolygon(tmp, p, 4, gdImageColorExactAlpha(tmp, 0, 0, 0, 0)); 
            */

            err = gdImageStringFT(tmp, NULL, tc, fn, coord_size,
                                  coord_tilt*rad, br[0]-l, br[1]-t, ct);
            if (err) throw runtime_error(err);
   
            // clip left of text to eliminate unused pixel columns
            for (int i = 0; i < w; ++i) {
               for (int j = 0; j < h; ++j) { 
                  if (gdImageAlpha(im, gdImageGetPixel(tmp, i, j)) < 127) {
                     l = i;
                     goto LCLIP;
                  }            
               }
            }
            LCLIP:
   
            // clip right of text to eliminate unused pixel columns
            for (int i = w-1; i >= 0; --i) {
               for (int j = 0; j < h; ++j) { 
                  if (gdImageAlpha(im, gdImageGetPixel(tmp, i, j)) < 127) {
                     r = i;
                     goto RCLIP;
                  }            
               }
            }
            RCLIP:
   
            // clip top of text to eliminate unused pixel rows
            for (int i = 0; i < h; ++i) {
               for (int j = 0; j < w; ++j) { 
                  if (gdImageAlpha(im, gdImageGetPixel(tmp, j, i)) < 127) {
                     t = i;
                     goto TCLIP;
                  }            
               }
            }
            TCLIP:

            // clip bottom of text to eliminate unused pixel rows 
            for (int i = h-1; i >= 0; --i) {
               for (int j = 0; j < w; ++j) { 
                  if (gdImageAlpha(im, gdImageGetPixel(tmp, j, i)) < 127) {
                     b = i;
                     goto BCLIP;
                  }            
               }
            }
            BCLIP:

            w = r-l+1;
            h = b-t+1;

            gdImageCopy(im, tmp, int(x-(w/2)+1+mleft),
                                 int(y-(h/2)+1+mtop), l, t, w, h);
            gdImageDestroy(tmp);

            if (err) throw runtime_error(err);
         }
      }
   }   

   if (grain == Horizontal) {
      int sx = gdImageSX(im), sy = gdImageSY(im);
      gdImagePtr rot = gdImageCreateTrueColor(sy, sx);
         
      // anti-alias to the alpha channel if our background is transparent
      if (bg_opacity == 127) {
         gdImageSaveAlpha(rot, 1);
         gdImageAlphaBlending(rot, 0);
      }

      // rotate 90 degrees
      for (int i = 0; i < sx; ++i)
         for (int j = 0; j < sy; ++j)
            gdImageSetPixel(rot, sy-1-j, i, gdImageGetPixel(im, i, j));

      gdImageDestroy(im);
      im = rot;
   }

   FILE *out;
   if (outfile.empty() || outfile == "-") out = stdout;
   else {
      out = fopen(outfile.c_str(), "wb");
      if (!out) throw runtime_error("cannot write to " + outfile);
   }
   
   gdImagePngEx(im, out, 9);
   if (out != stdout) fclose(out);
   gdImageDestroy(im);
}


void Grid::side_png(int n)
{
   double dx = 0,
          dy = 0;

   switch (n % 4) {
   case 0:
      dx = 0.25*hw;
      dy = 0.5*hh;
      break;
   case 2:
      dx = 0.25*hw;
      dy = -0.5*hh;
      break;
   case 1:
   case 3:
      dx = 0.5*hw;
      break;
   }

   line_png(int(round(cx)), int(round(cy)),
            int(round(cx+dx)), int(round(cy+dy)), gc);

   cx += dx;
   cy += dy;
}


void Grid::side_reverse_png(int n)
{
   double dx = 0,
          dy = 0;

   switch (n % 4) {
   case 0:
      dx = -0.25*hw;
      dy = 0.5*hh;
      break;
   case 2:
      dx = -0.25*hw;
      dy = -0.5*hh;
      break;
   case 1:
   case 3:
      dx = -0.5*hw;
      break;
   }

   line_png(int(round(cx)), int(round(cy)),
            int(round(cx+dx)), int(round(cy+dy)), gc);

   cx += dx;
   cy += dy;
}


void Grid::side_skip_png(int n)
{
   double dx = 0,
          dy = 0;

   switch (n % 4) {
   case 0:
      dx = 0.25*hw;
      dy = 0.5*hh;
      line_png(int(round(cx)), int(round(cy)),
               int(round(cx+dx)), int(round(cy+dy)), gc);
      break;
   case 2:
      dx = 0.25*hw;
      dy = -0.5*hh;
      line_png(int(round(cx)), int(round(cy)),
               int(round(cx+dx)), int(round(cy+dy)), gc);
      break;
   case 1:
   case 3:
      dx = 0.5*hw;
      break;
   }

   cx += dx;
   cy += dy;
}


void Grid::edge_png(int n)
{
   double dx = (n % 2 ? 0.25 : -0.25)*hw,
          dy = 0.5*hh;

   line_png(int(round(cx)), int(round(cy)),
            int(round(cx+dx)), int(round(cy+dy)), gc);
   cx += dx;
   cy += dy;            
}


void Grid::edge_reverse_png(int n)
{
   double dx = (n % 2 ? 0.25 : -0.25)*hw,
          dy = -0.5*hh;

   line_png(int(round(cx)), int(round(cy)),
            int(round(cx+dx)), int(round(cy+dy)), gc);
   cx += dx;
   cy += dy;     
}


void Grid::cross_png(int c, int r)
{
   line_png(int(round((c*0.75 + 0.5)*hw-center_size+mleft)),
            int(round((0.5*(1+(c+lowfirstcol)%2)+r)*hh+mtop)),
            int(round((c*0.75 + 0.5)*hw+center_size+mleft)),
            int(round((0.5*(1+(c+lowfirstcol)%2)+r)*hh+mtop)), cc);
   line_png(int(round((c*0.75 + 0.5)*hw+mleft)),
            int(round((0.5*(1+(c+lowfirstcol)%2)+r)*hh-center_size+mtop)),
            int(round((c*0.75 + 0.5)*hw+mleft)),
            int(round((0.5*(1+(c+lowfirstcol)%2)+r)*hh+center_size+mtop)), cc);
}


void Grid::dot_png(int c, int r)
{
   cx = (c*0.75 + 0.5)*hw+mleft;
   cy = (0.5*(1+(c+lowfirstcol)%2)+r)*hh+mtop;

   if (antialiased) {
      double o2 = (center_size+1)*(center_size+1),
             i2 = center_size*center_size;

      for (double x = -center_size; x <= center_size; ++x) {
         for (double y = -center_size; y <= center_size; ++y) {
            double x2y2 = x*x + y*y;
            if (x2y2 < i2) {        // interior pixel
               pixel_png(int(round(cx+x)), int(round(cy+y)), cc);
            }
            else if (x2y2 < o2) {   // edge pixel
               double d = sqrt(x2y2)-center_size;
               int b = gdImageColorExactAlpha(im, gdImageRed(im, cc),
                                                  gdImageGreen(im, cc),
                                                  gdImageBlue(im, cc),
                                                  (unsigned int)round(d*127));
               pixel_png(int(round(cx+x)), int(round(cy+y)), b);
            }
         }
      }
   }
   else gdImageFilledEllipse(im, int(round(cx)), int(round(cy)),
                                 int(round(center_size)),
                                 int(round(center_size)), cc);
}


void Grid::line_png(int x1, int y1, int x2, int y2, int c)
{
   if (antialiased) {
      // adapted from gdImageSetAALine() in gd-2.0.33

   	/* keep them as 32bits */
   	long x, y, inc;
   	long dx, dy,tmp;

     	dx = x2 - x1;
   	dy = y2 - y1;

   	if (dx == 0 && dy == 0) {
   		/* TBB: allow setting points */
   		pixel_png(x1, y1, c);
   		return;
   	}

   	if (abs(dx) > abs(dy)) {
   		if (dx < 0) {
   			tmp = x1;
   			x1 = x2;
   			x2 = tmp;
   			tmp = y1;
   			y1 = y2;
   			y2 = tmp;
   			dx = x2 - x1;
   			dy = y2 - y1;
   		}
   		x = x1 << 16;
   		y = y1 << 16;
   		inc = (dy * 65536) / dx;
   		/* TBB: set the last pixel for consistency (<=) */
   		while ((x >> 16) <= x2) {
            int a = gdImageColorExactAlpha(im, gdImageRed(im, c),
                                               gdImageGreen(im, c),
                                               gdImageBlue(im, c),
                                               ((y >> 8) & 0xFF)/2);
            int b = gdImageColorExactAlpha(im, gdImageRed(im, c),
                                               gdImageGreen(im, c),
                                               gdImageBlue(im, c),
                                               ((~y >> 8) & 0xFF)/2);
	   		pixel_png(x >> 16, y >> 16, a);
   			pixel_png(x >> 16, (y >> 16) + 1, b);
   			x += (1 << 16);
   			y += inc;
   		}
   	} 
      else {
   		if (dy < 0) {
   			tmp = x1;
   			x1 = x2;
   			x2 = tmp;
   			tmp = y1;
   			y1 = y2;
   			y2 = tmp;
   			dx = x2 - x1;
   			dy = y2 - y1;
   		}
   		x = x1 << 16;
   		y = y1 << 16;
   		inc = (dx * 65536) / dy;
   		/* TBB: set the last pixel for consistency (<=) */
   		while ((y >> 16) <= y2) {
            int a = gdImageColorExactAlpha(im, gdImageRed(im, c),
                                               gdImageGreen(im, c),
                                               gdImageBlue(im, c),
                                               ((x >> 8) & 0xFF)/2);
            int b = gdImageColorExactAlpha(im, gdImageRed(im, c),
                                               gdImageGreen(im, c),
                                               gdImageBlue(im, c),
                                               ((~x >> 8) & 0xFF)/2);
   			pixel_png(x >> 16, y >> 16, a);
   			pixel_png((x >> 16) + 1, (y >> 16), b);
   			x += inc;
   			y += (1<<16);
   		}
   	}
   }
   else gdImageLine(im, x1, y1, x2, y2, c);
}


void Grid::pixel_png(int x, int y, int c)
{
   if (bg_opacity == 127) {
      if (gdImageAlpha(im, gdImageGetPixel(im, x, y)) > gdImageAlpha(im, c)) {
         gdImageAlphaBlending(im, 0);
      	gdImageSetPixel(im, x, y, c);     
         gdImageAlphaBlending(im, 1);
      }
   }
   else gdImageSetPixel(im, x, y, c);
}

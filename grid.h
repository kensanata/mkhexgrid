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

#ifndef __GRID_H_
#define __GRID_H_

#include <map>
#include <string>
using namespace std;

class Grid {
   public:
      Grid(const map<string, string> &opt);
      void draw();

   private:
      // PNG-specific functions
      void draw_png();
      void side_png(int n);
      void side_reverse_png(int n);
      void side_skip_png(int n);
      void edge_png(int n);
      void edge_reverse_png(int n);
      void cross_png(int c, int r);
      void dot_png(int c, int r);
      void line_png(int x1, int y1, int x2, int y2, int c);
      void pixel_png(int x, int y, int c);

      // PS-specific functions
      void draw_ps();

      // SVG-specific functions
      void draw_svg();
      void side_path_svg(int n);
      void side_path_reverse_svg(int n);
      void side_skip_path_svg(int n);
      void edge_path_svg(int n);
      void edge_path_reverse_svg(int n);

      // parse functions
      void parse_length(const char *o, const string &str, double &d);
      void parse_color(const char *o, const string &str, string &c);
      void parse_opacity(const char *o, const string &str, double &op);
      void parse_format(const string &str);

      // utilty functions
      string alpha(int m);
      string alpha_tally(int m);

      // image parameters
      enum OutputType { PNG, PS, SVG } output;  // output type
      string outfile;   // output filename

      double bg_opacity;        // background opacity

      double grid_thickness,    // hex grid line width
             grid_opacity;      // hex grid opacity

      bool antialiased,  // antiailiasing
           lowfirstcol,  // first column is high or low
           matte;        // matte around background

      enum Grain { Vertical, Horizontal } grain;

      bool coord_display;        // display coordinates if true

      unsigned int coord_rskip,  // number every multiple of ith row
                   coord_cskip,  // number every multiple of ith column
                   coord_rstart, // coordinate of first row
                   coord_cstart; // coordinate of first column

      enum CoordOrigin { UpperLeft, UpperRight, LowerLeft, LowerRight }
         coord_origin;

      string coord_font,         // coordinate font
             coord_fmt_pre,      // pre-format text
             coord_fmt_inter,    // inter-format text
             coord_fmt_post;     // post-format text

      unsigned int coord_first_width,   // field width, first coordinate
                   coord_second_width;  // field width, second coordinate

      enum CoordStyle { NoCoord, Number, Alpha, AlphaTally }
         coord_first_style,
         coord_second_style;

      enum CoordOrder { OrderUndetermined, ColumnsFirst, RowsFirst }
         coord_order;

      bool coord_first_fill,    // 0-fill, first coordinate
           coord_second_fill;   // 0-fill, second coordinate

      double coord_size,         // coordinate font size in points
             coord_bearing,      // coordinate bearing from hex center
             coord_dist,         // coordinate distance from hex center
             coord_tilt,         // tilt of coordinate text from horizontal
             coord_opacity;      // coordinate opacity

      enum CenterStyle { Centerless, Dot, Cross } center_style;

      double center_size,       // size of center marker
             center_opacity;    // center marker opacity

      double mleft,    // left marign
             mright,   // right margin
             mtop,     // top margin
             mbottom;  // bottom margin

      string grid_color,      // grid color
             coord_color,     // coordinate color
             center_color,    // center color
             bg_color;        // background color

      double hw,          // hex width
             hh,          // hex height
             hs,          // hex side 
             iw,          // image width
             ih;          // image height

      int rows,
          cols;

      // useful constants
      static const double rad;    // radians per degree
};

#endif /* __GRID_H_ */

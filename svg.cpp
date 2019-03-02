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
#include <iostream>
#include <iomanip>
#include <exception>
#include <fstream>
#include <stdexcept>
#include <string>
#include <sstream>
using namespace std;

#include "grid.h"

ofstream out;

void Grid::draw_svg()
{
   if (outfile.empty() || outfile == "-") out.ostream::rdbuf(cout.rdbuf());
   else {
      filebuf *buf = new filebuf;
      buf->open(outfile.c_str(), ios::out | ios::binary);
      out.ostream::rdbuf(buf);
      if (!out) throw runtime_error("cannot write to " + outfile);
   }

   // write header
   out << "<?xml version=\"1.0\" standalone=\"no\"?>\n"
          "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\"\n" 
          "   \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n"
          "<svg width=\"" << iw
       << "\" height=\"" << ih
       << "\" version=\"1.1\"\n"
          "   xmlns=\"http://www.w3.org/2000/svg\"\n"
          "   xmlns:xlink=\"http://www.w3.org/1999/xlink\">\n";
   
   // write definitions for repeated elements
   out << "<defs>\n";

   // center definition
   if (center_style != Centerless) {
      switch (center_style) {
      case Cross:
         out << "<path id=\"cross\" d=\""
             << "M 0 0 m " << -center_size << " 0"
             << " h " << 2*center_size
             << " M 0 0 m 0 " << -center_size
             << " v " << 2*center_size
             << "\" />\n";  

         out << "<g id=\"c-row\">\n";
         for (int c = 0; c < cols; ++c) {
            out << "<use x=\"" << (c*0.75 + 0.5)*hw << "\" "
                   "y=\"" << 0.5*hh*((c+lowfirstcol)%2) << "\" "
                   "xlink:href=\"#cross\" />\n";
         }
         out << "</g>\n";
         break;
      case Dot:
         out << "<g id=\"c-row\">\n";
         for (int c = 0; c < cols; ++c) {
            out << "<circle cx=\"" << (c*0.75 + 0.5)*hw
                << "\" cy=\"" << 0.5*hh*((c+lowfirstcol)%2)
                << "\" r=\"" << center_size << "\" />\n";
         }
         out << "</g>\n";
         break;
      default:
         break;
      }
   }
 
   // grid definition
   if (lowfirstcol == true) {
      out << "<path id=\"bottoms\" d=\"M 0 0"; 
      for (int n = 0; n <= 2*cols-3; ++n) side_skip_path_svg(n);
      out << "\" />\n";

      out << "<path id=\"tops\" d=\"M 0 0 l";
      for (int n = 1; n <= 2*cols - 1; ++n) side_path_svg(n);
      out << "\" />\n";

      out << "<path id=\"outline\" d=\"M 0 0 l";
      for (int n = 1; n <= 2*cols-cols%2; ++n)
         side_path_svg(n);
      for (int n = cols%2; n <= 2*rows-2+cols%2; ++n)
         edge_path_svg(n);
      for (int n = 3+(cols%2); n <= 2*cols+1+2*(cols%2); ++n)
         side_path_reverse_svg(n);
      for (int n = 0; n <= 2*rows-2; ++n)
         edge_path_reverse_svg(n);
      out << " z\" />\n";
   }
   else {
      out << "<path id=\"bottoms\" d=\"M 0 0";
      for (int n = 2; n <= 2*cols-1; ++n) side_skip_path_svg(n);
      out << "\" />\n";

      out << "<path id=\"tops\" d=\"M 0 0 l";
      for (int n = 3; n <= 2*cols + 1; ++n) side_path_svg(n);
      out << "\" />\n";

      out << "<path id=\"outline\" d=\"M 0 0 l";
      for (int n = 2; n <= 2*cols+2-(cols+1)%2; ++n)
         side_path_svg(n);
      for (int n = 1-cols%2; n <= 2*rows-1-cols%2; ++n)
         edge_path_svg(n);
      for (int n = 3*(cols%2); n <= 2*cols-1+2*(cols%2); ++n)
         side_path_reverse_svg(n);
      for (int n = 0; n <= 2*rows-3; ++n)
         edge_path_reverse_svg(n);
      out << " z\" />\n";
   }

   out << "</defs>\n";

   // draw background
   if (!bg_color.empty()) {
      out << "<rect id=\"background\" "
             "x=\""      << (matte ? mleft-grid_thickness/2 : 0) << "\" "
             "y=\""      << (matte ? mtop-grid_thickness/2  : 0) << "\" "
             "width=\""  << (matte ?
                              iw-mleft-mright+grid_thickness : iw) << "\" "
             "height=\"" << (matte ?
                              ih-mtop-mbottom+grid_thickness : ih) << "\" "
             "style=\""
             "fill: #"        << bg_color   << "; "
             "fill-opacity: " << bg_opacity << "; "
             "stroke: none;\" />\n";
   }
   
   out << "<g transform=\"translate(";
   if (grain == Horizontal) {
      out << iw-mright << ','
          << mtop+grid_thickness/2 << ") rotate(90)\">\n";
   }
   else {
      out << mleft+grid_thickness/2 << ',' 
          << mtop+grid_thickness/2 << ")\">\n";
   }

   // draw grid
   out << "<g id=\"grid\" style=\""
          "fill: none; "
          "stroke: #"        << grid_color << "; "
          "stroke-opacity: " << grid_opacity << "; "
          "stroke-width: "   << grid_thickness << "; "
          "\">\n";

   if (lowfirstcol == true) {
      double x = 0.25*hw, 
             y = 0.5*hh;

      out << "<use x=\"" << x << "\" y=\"" << y
          << "\" xlink:href=\"#outline\" />\n";

      for (int r = 0; r < rows - 1; ++r) {
         x = 0.75*hw;
         out << "<use x=\"" << x << "\" y=\"" << y
             << "\" xlink:href=\"#bottoms\" />\n";

         x = 0.25*hw;
         y += hh;

         out << "<use x=\"" << x << "\" y=\"" << y
             << "\" xlink:href=\"#tops\" />\n";
      }

      x = 0.75*hw;
      out << "<use x=\"" << x << "\" y=\"" << y
          << "\" xlink:href=\"#bottoms\" />\n";
   }
   else {
      double x = 0,
             y = 0.5*hh;

      out << "<use x=\"" << x << "\" y=\"" << y
          << "\" xlink:href=\"#outline\" />\n";
      
      y = hh;

      for (int r = 0; r < rows - 1; ++r) {
         x = 0.75*hw;
         out << "<use x=\"" << x << "\" y=\"" << y
             << "\" xlink:href=\"#bottoms\" />\n";

         x = 0.25*hw;

         out << "<use x=\"" << x << "\" y=\"" << y
             << "\" xlink:href=\"#tops\" />\n";

         y += hh;
      }

      x = 0.75*hw;
      out << "<use x=\"" << x << "\" y=\"" << y
          << "\" xlink:href=\"#bottoms\" />\n";
   }

   out << "</g>\n";

   // draw centers
   if (center_style != Centerless) {
      out << "<g id=\"centers\" ";

      switch (center_style) {
      case Cross:
         out << "style=\""
                "stroke: #" << center_color << "; "
                "stroke-opacity: " << center_opacity << "; "
                "\"";
         break;
      case Dot:
         out << "style=\""
                "fill: #" << center_color << "; "
                "fill-opacity: " << center_opacity << "; "
                "\"";
         break;
      default:
         break;
      }

      out << ">\n";

      for (int r = 0; r < rows; ++r)
         out << "<use x=\"0" << "\" "
             << "y=\"" << (r + 0.5)*hh
             << "\" xlink:href=\"#c-row\" />\n"; 
      out << "</g>\n";
   }

   if (coord_display) {
      // draw coordinates
      out << "<g id=\"coordinates\" "
             "style=\""
             "fill: #" << coord_color << "; "
             "fill-opacity: " << coord_opacity << "; "
             "font-family: " << coord_font << "; "
             "font-size: " << coord_size << "px; "
             "text-anchor: middle; "
             "\">\n";
      // NB: CSS requires that font-size have some unit
   
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

            int a = cc, b = cr;
            if (coord_order == RowsFirst) swap(a, b);

            ostringstream s;
            s << coord_fmt_pre;

            switch (coord_first_style) {
            case NoCoord:
               break;
            case Number:
               if (coord_first_width) s << setw(coord_first_width);
               if (coord_first_fill) s << setfill('0');
               s << a;
               break;
            case Alpha:
               s << alpha(a);
               break;
            case AlphaTally:
               s << alpha_tally(a);
               break;
            }

            s << coord_fmt_inter;

            switch (coord_second_style) {
            case NoCoord:
               break;
            case Number:
               if (coord_second_width) s << setw(coord_second_width);
               if (coord_second_fill) s << setfill('0');
               s << b;
               break;
            case Alpha:
               s << alpha(b);
               break;
            case AlphaTally:
               s << alpha_tally(b);
               break;
            }

            double x = (c*0.75 + 0.5)*hw+coord_dist*bcos;
            double y = (r + 0.5*(1 + (c+lowfirstcol)%2))*hh+coord_dist*bsin;

            out << "<text x=\"" << x
                << "\" y=\"" << y << "\"";
            if (coord_tilt)
               out << " transform=\"rotate(" << coord_tilt
                   << ' ' << x << ' ' << y << ")\"";
            out << '>' << s.str() << "</text>\n";
         }
      }
      out << "</g>\n";
   }   

   out << "</g>\n";
   out << "</svg>" << endl;

   if (!outfile.empty()) out.close();
}


void Grid::side_path_svg(int n)
{
   switch (n % 4) {
   case 0:
      out << ' ' << 0.25*hw << ' ' << 0.5*hh;
      break;
   case 2:
      out << ' ' << 0.25*hw << ' ' << -0.5*hh;
      break;
   case 1:
   case 3:
      out << ' ' << 0.5*hw << " 0";
      break;
   }
}


void Grid::side_path_reverse_svg(int n)
{
   switch (n % 4) {
   case 0:
      out << ' ' << -0.25*hw << ' ' << 0.5*hh;
      break;
      break;
   case 2:
      out << ' ' << -0.25*hw << ' ' << -0.5*hh;
      break;
   case 1:
   case 3:
      out << ' ' << -0.5*hw << " 0";
      break;
   }
}


void Grid::side_skip_path_svg(int n)
{
   switch (n % 4) {
   case 0:
      out << " l " << 0.25*hw << ' ' << 0.5*hh;
      break;
   case 2:
      out << " l " << 0.25*hw << ' ' << -0.5*hh;
      break;
   case 1:
   case 3:
      out << " m " << 0.5*hw << " 0";
      break;
   }
}


void Grid::edge_path_svg(int n)
{
   out << ' ' << (n % 2 ? 0.25 : -0.25)*hw
        << ' ' << 0.5*hh;
}


void Grid::edge_path_reverse_svg(int n)
{
   out << ' ' << (n % 2 ? 0.25 : -0.25)*hw
        << ' ' << -0.5*hh;
}

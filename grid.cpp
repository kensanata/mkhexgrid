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
#include <map>
#include <string>
#include <exception>
#include <stdexcept>
using namespace std;

#include <boost/lexical_cast.hpp>
using namespace boost;

#include "grid.h"

const double Grid::rad = M_PI/180.0;

Grid::Grid(const map<string, string> &opt)
{
   map<string, string>::const_iterator i;

   // 
   // Output Parameters
   //
   i = opt.find("output");
   if (i == opt.end())          output = PNG;
   else if (i->second == "png") output = PNG;
   else if (i->second == "ps")  output = PS;
   else if (i->second == "svg") output = SVG;
   else throw runtime_error("unrecognized output type `" + i->second + "'");

   i = opt.find("outfile");
   if (i != opt.end()) outfile = i->second;

   antialiased = (opt.find("antialias") != opt.end());
   if (antialiased && output != PNG)
      cerr << "PostScript and SVG output is always antialiased" << endl;

   // NB: this works because 0 is opaque in PNG, 1 is opaque in SVG, and
   // opacity is ignored in PostScript.
   bg_opacity = grid_opacity =
      coord_opacity = center_opacity = (output == SVG);

   if (output == PS) 
        grid_color = coord_color = center_color = "0.5 0.5 0.5";
   else grid_color = coord_color = center_color = "808080";

   if (output == PNG) bg_color = "ffffff";
 
   //
   // Grid Parameters
   //
   i = opt.find("grid-grain");
   if (i == opt.end())        grain = Vertical;
   else if (i->second == "h") grain = Horizontal;
   else if (i->second == "v") grain = Vertical;
   else
      throw runtime_error("unrecognized grain direction `" + i->second + "'");

   i = opt.find("grid-start");
   if (i == opt.end())        lowfirstcol = false;
   else if (i->second == "i") lowfirstcol = true;
   else if (i->second == "o") lowfirstcol = false;
   else throw runtime_error("unrecognized grid start `" + i->second + "'");

   i = opt.find("grid-thickness");
   if (i == opt.end()) grid_thickness = 1;
   else parse_length("grid thickness", i->second, grid_thickness);
   if (grid_thickness < 0)
      throw range_error("grid thickness is negative");
   if (output == PNG && grid_thickness != floor(grid_thickness))
      throw runtime_error("grid thickness is not an integer");

   i = opt.find("grid-color");
   if (i != opt.end()) parse_color("grid color", i->second, grid_color);

   i = opt.find("grid-opacity");
   if (i != opt.end()) parse_opacity("grid opactiy", i->second, grid_opacity);

   //
   // Coordinate Parameters
   //
#ifdef WIN32
   coord_font = "Arial";
#else
   coord_font = "sans";
#endif
   coord_bearing = 90;
   coord_tilt = coord_dist = 0;
   coord_size = 8; 

   i = opt.find("coord-font");
   if (i != opt.end()) coord_font = i->second;

   i = opt.find("coord-bearing");
   if (i != opt.end()) {
      try {
         coord_bearing = lexical_cast<double>(i->second);
      }
      catch (bad_lexical_cast &) {
         throw runtime_error("coord-bearing is not a number");
      }
   }

   i = opt.find("coord-tilt");
   if (i != opt.end()) {
      try {
         coord_tilt = lexical_cast<double>(i->second);
      }
      catch (bad_lexical_cast &) {
         throw runtime_error("coord-tilt is not a number");
      }
   }

   i = opt.find("coord-distance");
   if (i != opt.end())
      parse_length("coordinate distance", i->second, coord_dist);

   i = opt.find("coord-size");
   if (i != opt.end())
      parse_length("coordinate size", i->second, coord_size);
   if (coord_size <= 0) throw range_error("coordinate size is not positive");
      
   i = opt.find("coord-origin");
   if (i == opt.end())         coord_origin = UpperLeft;
   else if (i->second == "ul") coord_origin = UpperLeft;
   else if (i->second == "ll") coord_origin = LowerLeft;
   else if (i->second == "ur") coord_origin = UpperRight;
   else if (i->second == "lr") coord_origin = LowerRight;
   else throw
      runtime_error("unrecognized coordinate origin `" + i->second + "'"); 

   coord_cskip = coord_rskip = coord_cstart = coord_rstart = 1;

   i = opt.find("coord-column-skip");
   if (i != opt.end()) {
      try {
         coord_cskip = lexical_cast<unsigned int>(i->second);
      }
      catch (bad_lexical_cast &) {
         throw runtime_error("coord-column-skip is not an integer");
      }
      
      if (coord_cskip == 0)
         throw range_error("coord-column-skip is not positive");
   }

   i = opt.find("coord-row-skip");
   if (i != opt.end()) {
      try {
         coord_rskip = lexical_cast<unsigned int>(i->second);
      }
      catch (bad_lexical_cast &) {
         throw runtime_error("coord-row-skip is not an integer");
      }

      if (coord_rskip == 0)
         throw range_error("coord-row-skip is not positive");
   }

   i = opt.find("coord-column-start");
   if (i != opt.end()) {
      try {
         coord_cstart = lexical_cast<unsigned int>(i->second);
      }
      catch (bad_lexical_cast &) {
         throw runtime_error("coord-column-start is not a nonnegative integer");
      }
   }

   i = opt.find("coord-row-start");
   if (i != opt.end()) {
      try {
         coord_rstart = lexical_cast<unsigned int>(i->second);
      }
      catch (bad_lexical_cast &) {
         throw runtime_error("coord-row-start is not a nonnegative integer");
      }
   }

   // default format is equivalent to "%02c%02r"
   coord_display = true;
   coord_first_width = 2;
   coord_second_width= 2;
   coord_first_style = Number;
   coord_second_style = Number;
   coord_order = ColumnsFirst;
   coord_first_fill = true;
   coord_second_fill = true;
   
   i = opt.find("coord-format");
   if (i != opt.end()) {
      if (i->second.empty()) coord_display = false;
      else parse_format(i->second);
   }

   i = opt.find("coord-color");
   if (i != opt.end()) parse_color("coordinate color", i->second, coord_color);

   i = opt.find("coord-opacity");
   if (i != opt.end())
      parse_opacity("coordinate opacity", i->second, coord_opacity);
  
   switch (output) {
   case PNG:
      // 90 degrees is down in GD, except for text
      // for which 90 degrees is up!
      coord_bearing = -coord_bearing;
      break;
   case PS:
      coord_bearing -= 90;
      break;
   case SVG:
      // 90 degrees is down in SVG
      coord_bearing = -coord_bearing;
      coord_tilt = -coord_tilt;
      break;
   }

   //
   // Center Parameters
   //
   center_size = 3;

   i = opt.find("center-style");
   if (i == opt.end())        center_style = Centerless;
   else if (i->second == "c") center_style = Cross;
   else if (i->second == "d") center_style = Dot;
   else if (i->second == "n") center_style = Centerless;
   else throw runtime_error("unrecognized center style `" + i->second + "'");

   i = opt.find("center-size");
   if (i != opt.end())
      parse_length("center size", i->second, center_size);
   if (center_size < 0)
      throw range_error("center size is negative");
   if (output == PNG && center_size != floor(center_size))
      throw runtime_error("center size is not an integer");

   i = opt.find("center-color");
   if (i != opt.end()) parse_color("center color", i->second, center_color);

   i = opt.find("center-opacity");
   if (i != opt.end())
      parse_opacity("center opacity", i->second, center_opacity);

   //
   // Background Parameters
   // 
   i = opt.find("bg-color");
   if (i != opt.end()) parse_color("background color", i->second, bg_color);

   i = opt.find("bg-opacity");
   if (i != opt.end())
      parse_opacity("background opacity", i->second, bg_opacity);

   matte = (opt.find("matte") != opt.end());
   if (matte && bg_color.empty())
      cerr << "matte is useless without background color" << endl;

   //
   // Size Parameters
   //
   i = opt.find("image-margin");
   if (i == opt.end()) mtop = mright = mbottom = mleft = 0;
   else {
      string str;
      istringstream s(i->second);

      getline(s, str, ',');
      parse_length("image margin", str.c_str(), mtop);

      // equal margins if only one length given
      if (s.eof()) mright = mbottom = mleft = mtop;
      else {
         getline(s, str, ',');
         parse_length("image margin", str.c_str(), mright);

         getline(s, str, ',');
         parse_length("image margin", str.c_str(), mbottom);

         getline(s, str);
         parse_length("image margin", str.c_str(), mleft);
      }

      if (s.fail() || !s.eof()) throw runtime_error(
         "image margins must be given as a single "
         "value or as four (t,r,b,l)");
   }

   hw = hh = hs = iw = ih = 0;
   cols = rows = 0;

   i = opt.find("hex-width");
   if (i != opt.end()) parse_length("hex width", i->second, hw);

   i = opt.find("hex-height");
   if (i != opt.end()) parse_length("hex height", i->second, hh);
   
   i = opt.find("hex-side");
   if (i != opt.end()) parse_length("hex side", i->second, hs);

   i = opt.find("columns");
   if (i != opt.end()) {
      try {
         cols = lexical_cast<unsigned int>(i->second);
      }
      catch (bad_lexical_cast &) {
         throw runtime_error("number of columns is not an integer");
      }
      if (cols == 0) throw range_error("number of columns is not positive");
   }
 
   i = opt.find("rows");
   if (i != opt.end()) {
      try {
         rows = lexical_cast<unsigned int>(i->second);
      }
      catch (bad_lexical_cast &) {
         throw runtime_error("number of rows is not an integer");
      }
      if (rows == 0) throw range_error("number of rows is not positive");
   }

   i = opt.find("image-width");
   if (i != opt.end()) parse_length("image width", i->second, iw);

   i = opt.find("image-height");
   if (i != opt.end()) parse_length("image height", i->second, ih);

   if (grain == Horizontal) {
      swap(hw, hh);
      swap(cols, rows);

      // rotate margins
      double tmp = mright;
      mright = mtop;
      mtop = mleft;
      mleft = mbottom;
      mbottom = tmp;
   }

   while (!hs || !hw || !hh || !cols || !rows || !iw || !ih) {
      bool more = false;

      if (!hs) {                       // calculate hs
         if (hw) {
            hs = hw/2;
            more = true;
         }
         else if (hh) {                
            hs = hh/(2*sin(60*rad));
            more = true;
         }
      }
   
      if (!hw) {                       // calculate hw
         if (hs) {
            hw = 2*hs;
            more = true;
         }
         else if (hh) {
            hw = hh/sin(60*rad);
            more = true;
         }
      }
   
      if (!hh) {                       // calculate hh
         if (hs) {
            hh = 2*hs*sin(60*rad);
            more = true;
         }
         else if (hw) {
            hh = hw*sin(60*rad);
            more = true;
         }
      }
   
      if (hw && cols && !iw) {         // calculate iw
         iw = mleft + (0.25+0.75*cols)*hw+grid_thickness + mright;
         more = true;
      }
      else if (hw && !cols && iw) {    // calculate cols
         cols = (int)floor(((iw-mleft-grid_thickness-mright)/hw - 0.25)/0.75); 
         more = true;
      }
      else if (!hw && cols && iw) {    // calculate hw
         hw = (iw-mleft-grid_thickness-mright)/(0.25+0.75*cols);
         more = true;
      }
   
      if (hh && rows && cols && !ih) {         // calculate ih
         if (cols > 1) ih = mtop + (0.5+rows)*hh+grid_thickness + mbottom;
         else ih = mtop + rows*hh + grid_thickness + mbottom;
         more = true; 
      }
      else if (hh && !rows && cols && ih) {    // calculate rows
         if (cols > 1)
            rows = (int)floor((ih-mtop-grid_thickness-mbottom)/hh - 0.5);
         else rows = (int)floor((ih-mtop-grid_thickness-mbottom)/hh);
         more = true; 
      }
      else if (!hh && rows && cols && ih) {    // calculate hh
         if (cols > 1) hh = (ih-mtop-grid_thickness-mbottom)/(0.5+rows);
         else hh = (ih-mtop-grid_thickness-mbottom)/rows;
         more = true; 
      }

      if (!more) break;
   }
   
   if (!hs)
      throw runtime_error("unable to determine hex side from given values");
   if (!hw)
      throw runtime_error("unable to determine hex width from given values");
   if (!hh)
      throw runtime_error("unable to determine hex height from given values");
   if (!iw)
      throw runtime_error("unable to determine image width from given values");
   if (!ih)
      throw runtime_error("unable to determine image height from given values");
   if (!rows)
      throw runtime_error("unable to determine rows from given values");
   if (!cols)
      throw runtime_error("unable to determine columns from given values");

   // adjust for centering
   if (opt.find("centered") != opt.end()) {
      // calculate actual grid width, height
      double aw = (0.25+0.75*cols)*hw+grid_thickness,
             ah = (0.5+rows)*hh+grid_thickness;
      
      // margin adjustment
      double h = ((iw - mleft - mright) - aw)/2,
             v = ((ih - mtop - mbottom) - ah)/2; 

      mleft   += h;
      mright  += h;
      mtop    += v;
      mbottom += v;
   }

   if (grain == Horizontal) {
      // unrotate margins
      double tmp = mright;
      mright = mbottom;
      mbottom = mleft;
      mleft = mtop;
      mtop = tmp;

      swap(iw, ih);

      switch (output) {
      case PS:
         coord_bearing += 90;
         coord_tilt += 90;
         break;
      case PNG:
         coord_bearing -= 90;
         coord_tilt += 90;
         break;
      case SVG:
         coord_bearing -= 90;
         coord_tilt -= 90;
         break;
      }
      
      switch (coord_origin) {
      case UpperLeft:
         coord_origin = LowerLeft;
         lowfirstcol = !lowfirstcol;
         break;
      case LowerLeft:
         coord_origin = LowerRight;
         if (cols%2) lowfirstcol = !lowfirstcol;
         break;
      case UpperRight:
         coord_origin = UpperLeft;
         break;
      case LowerRight:
         coord_origin = UpperRight;
         break;
      }
   }
 
   // adjust lowfirstcol if even cols and coordinate origin on the right
   if ((coord_origin == UpperRight || coord_origin == LowerRight) && !(cols%2))
      lowfirstcol = !lowfirstcol;

   // there is no wave with only one column
   if (cols == 1) lowfirstcol = false;

   // horizontal grain adjustments
   if (grain == Horizontal) {
      if (output == PS) swap(rows, cols);
      else if (output == PNG) {
         swap(iw, ih);
      
         // rotate margins
         double tmp = mright;
         mright = mtop;
         mtop = mleft;
         mleft = mbottom;
         mbottom = tmp;

         swap(mtop, mbottom);
         swap(mleft, mright);
      }
   }

   // clip angles to [0,360) for PNG output
   if (output == PNG) {
      coord_bearing = fmod(coord_bearing, 360);
      if (coord_bearing < 0) coord_bearing += 360;
      coord_tilt = fmod(coord_tilt, 360);
      if (coord_tilt < 0) coord_tilt += 360;  
   }
}


void Grid::draw()
{
   switch (output) {
   case SVG:   draw_svg(); break;
   case PNG:   draw_png(); break;
   case PS:    draw_ps();  break;
   }
}


string Grid::alpha(int m)
{
   // A ... Z AA AB ...
   if (m < 27) return string(1, m > 0 ? m+64 : 'Z');
   else return alpha((m-1)/26) + string(1, m % 26 ? (m % 26) + 64 : 'Z');
}


string Grid::alpha_tally(int m)
{
   // A ... Z AA BB ...
   return string(int(ceil(m/26.0)), (m-1) % 26 + 65);
}


void Grid::parse_color(const char *o, const string &str, string &c)
{
   stringstream s(str);

   if (output == PS) {
      double r, g, b;
      char c1, c2;
      
      s >> r >> c1 >> g >> c2 >> b;
      if (s.fail() || !s.eof()  || c1 != ',' || c2 != ',') 
         throw runtime_error("invalid color format for " + string(o));
      if (r < 0 || r > 1) throw range_error("red value for " + string(o) +
         "is not in the range [0,1]");
      if (g < 0 || g > 1) throw range_error("green value for " + string(o) +
         "is not in the range [0,1]");
      if (b < 0 || b > 1) throw range_error("blue value for " + string(o) +
         "is not in the range [0,1]");

      s.clear();
      s << r << ' ' << g << ' ' << b;
   }
   else {
      unsigned int h;
      
      s >> hex >> h;
      if (s.fail() || !s.eof())
         throw runtime_error("invalid color format for " + string(o));
    
      if (h > 0xFFFFFF) throw range_error(string(o) +
         " is not in the range [000000,FFFFFF]");
 
      s.clear(); 
      s << setfill('0') << setw(6) << hex << h;
   }

   c = s.str();
}


void Grid::parse_opacity(const char *o, const string &str, double &op)
{
   if (output == PS) {
      cerr << "opacity ignored for PostScript output" << endl; 
      return;
   }

   try {
      op = lexical_cast<double>(str);
   }
   catch (bad_lexical_cast &) {
      throw runtime_error(string(o) + " is not a number");
   }

   if (output == PNG) {
      if (op < 0 || op > 127) throw range_error(string(o) +
         " is not in the allowable range [0,127] for PNG output");
      if (op != floor(op)) throw runtime_error(string(o) +
         " is not an integer");
   }
   else if (output == SVG) {
      if (op < 0 || op > 1) throw range_error(string(o) +
         " is not in the allowable range [0,1] for SVG output");
   }
}


void Grid::parse_length(const char *o, const string &str, double &d)
{
   string tmp(str);
   istringstream s(tmp);
   s >> d;
   if (s.fail()) throw runtime_error(string(o) + " is not a number");

   if (!s.eof()) {
      string u;
      s >> u;

      switch (output) {
      case PNG:
         if (strcmp(o, "coordinate size") && u != "pt")
            throw runtime_error(string(o) + " is not in pt");
         else if (u != "px")
            throw runtime_error(string(o) + " is not in px");
         break;
      case SVG:
         if (u != "px") throw runtime_error(string(o) + " is not in px");
         break;
      case PS:
         if      (u == "pt") ;               // 1 point per point :)
         else if (u == "cm") d *= 72/2.54;   // ~28.35 points per cm
         else if (u == "mm") d *= 72/25.4;   // ~2.835 points per mm
         else if (u == "in") d *= 72;        // 72 points per inch
         else throw runtime_error(string(o) +
            " has unrecognized unit `" + u + "'");
         break;
      }
   }
}


void Grid::parse_format(const string &str)
{
   coord_first_style = coord_second_style = Grid::NoCoord;
   coord_first_fill = coord_second_fill = false;
   coord_first_width = coord_second_width = 0;

   istringstream i(str);

   // get coord_fmt_pre
   while (!i.eof()) {
      string s;
      getline(i, s, '%');
      coord_fmt_pre += s;
      if (i.peek() == '%') coord_fmt_pre += i.get();
      else break;
   }

   // get first coord format
   if (!i.eof()) {
      if (i.peek() == '0') { coord_first_fill = true; i.get(); }
      i >> coord_first_width;
      i.clear();
      bool tally = false;
      if (i.peek() == 't') { tally = true; i.get(); }
      char c = i.get();
      if (c == 'c' || c == 'C') coord_order = ColumnsFirst;
      else if (c == 'r' || c == 'R') coord_order = RowsFirst;
      else throw runtime_error("bad coordinate format string");

      if (c == 'c' || c == 'r') coord_first_style = Number;
      else coord_first_style = tally ? AlphaTally : Alpha;

      if ((c == 'C' || c == 'R') &&
          (coord_first_width || coord_first_fill))
         throw runtime_error("bad coordinate format string");
   }

   // get coord_fmt_inter
   while (!i.eof()) {
      string s;
      getline(i, s, '%');
      coord_fmt_inter += s;
      if (i.peek() == '%') coord_fmt_inter += i.get();
      else break;
   }

   // get second coord format
   if (!i.eof()) {
      if (i.peek() == '0') { coord_second_fill = true; i.get(); }
      i >> coord_second_width;
      i.clear();
      bool tally = false;
      if (i.peek() == 't') { tally = true; i.get(); }
      char c = i.get();
      if (((c == 'c' || c == 'C') && coord_order == ColumnsFirst) ||
          ((c == 'r' || c == 'R') && coord_order == RowsFirst))
         throw runtime_error("bad coordinate format string");

      if (c == 'c' || c == 'r') coord_second_style = Number;
      else coord_second_style = tally ? AlphaTally : Alpha;

      if ((c == 'C' || c == 'R') &&
          (coord_second_width || coord_second_fill))
         throw runtime_error("bad coordinate format string");
   }

   // get coord_fmt_post
   while (!i.eof()) {
      string s;
      getline(i, s, '%');
      coord_fmt_post += s;
      if (i.peek() == '%') coord_fmt_post += i.get();
      else break;
   }

   if (!i.eof()) throw runtime_error("bad coordinate format string");   
}

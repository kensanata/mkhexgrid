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
#include <fstream>
#include <string>
#include <sstream>
#include <exception>
#include <stdexcept>
using namespace std;

#include "grid.h"

void Grid::draw_ps()
{
   ofstream out;
   if (outfile.empty() || outfile == "-") out.ostream::rdbuf(cout.rdbuf());
   else {
      filebuf *buf = new filebuf;
      buf->open(outfile.c_str(), ios::out | ios::binary);
      out.ostream::rdbuf(buf);
      if (!out) throw runtime_error("cannot write to " + outfile);
   }
  
   // header
   out <<
"%!PS-Adobe-3.0 EPSF-3.0\n"
"%%Title: \n"
"%%Creator: mkhexgrid " << VERSION << "\n"
"%%Pages: 1\n"
"%%BoundingBox: 0 0 "
      << int(ceil(iw)) << ' '
      << int(ceil(ih)) << "\n"
"%%EndComments\n";

   // parameters
   out <<
"%%BeginProlog\n"
"save countdictstack mark newpath /showpage {} def /setpagedevice {pop} def\n"
"\n"
"%\n"
"% Parameters\n"
"%\n"
"/#copies 1 def\n"
"\n"
"/mleft "   << mleft   << " def\n"
"/mright "  << mright  << " def\n"
"/mtop "    << mtop    << " def\n"
"/mbottom " << mbottom << " def\n"
"\n";
   
   if (!bg_color.empty()) out <<
"/bg_color {" << bg_color << "} def\n";

   out <<
"/hex_side   " << hs << " def\n"
"/hex_height { 2 hex_side mul 60 sin mul} bind def\n"
"/hex_width  { 2 hex_side mul } bind def\n"
"/hex_color  {"   << grid_color << "} def\n"
"/hex_linewidth " << grid_thickness << " def\n"
"\n"
"/rows "        << rows << " def\n"
"/cols "        << cols << " def\n"
"/lowfirstcol " << lowfirstcol << " def\n"
"\n"
"/center_size "   << center_size << " def\n"
"/center_color {" << center_color << "} def\n"
"\n"
"/coord_font /"  << coord_font << " def\n"
"/coord_size "   << coord_size << " def\n"
"/coord_digits 2 def\n"
"/coord_color {" << coord_color << "} def\n"
"/coord_cskip "  << coord_cskip << " def\n"
"/coord_rskip "  << coord_rskip << " def\n"
"/coord_cstart " << coord_cstart << " def\n"
"/coord_rstart " << coord_rstart << " def\n"
"/coord_cstart_right "
   << (coord_origin == UpperRight || coord_origin == LowerRight) << " def\n"
"/coord_rstart_top "
   << (coord_origin == UpperLeft || coord_origin == UpperRight) << " def\n"
"/coord_bearing "  << coord_bearing << " def\n"
"/coord_distance " << coord_dist << " def\n"
"/coord_tilt " << coord_tilt << " def\n"
"\n";

   // definitions
   out <<
"%\n"
"% Definitions\n"
"%\n"
"/line {hex_side 0 rlineto} def\n"
"/no_line {hex_side 0 rmoveto} def\n"
"\n"
"%\n"
"%             1\n"
"%          ________\n"
"%         /        \\  \n"
"%     0  /          \\ 2\n"
"%       /            \\________\n"
"%                        3\n"
"%\n"
"\n"
"/side\n"
"{\n"
"   dup 0 eq {\n"
"      % 0\n"
"      60 rotate\n"
"      line\n"
"      -60 rotate \n"
"   } {\n"
"      dup 1 eq {\n"
"         % 1\n"
"         line\n"
"      } {\n"
"         dup 2 eq {\n"
"            % 2\n"
"            -60 rotate\n"
"            line\n"
"            60 rotate\n"
"         } {\n"
"            % 3\n"
"            line\n"
"         } ifelse\n"
"      } ifelse\n"
"   } ifelse \n"
"\n"
"   pop\n"
"} bind def\n"
"\n"
"/side_skip\n"
"{\n"
"   dup 0 eq {\n"
"      % 0\n"
"      60 rotate\n"
"      line\n"
"      -60 rotate\n"
"   } {\n"
"      dup 1 eq {\n"
"         % 1\n"
"         no_line\n"
"      } {\n"
"         dup 2 eq {\n"
"            % 2\n"
"            -60 rotate\n"
"            line\n"
"            60 rotate\n"
"         } {\n"
"            % 3\n"
"            no_line\n"
"         } ifelse\n"
"      } ifelse\n"
"   } ifelse \n"
"\n"
"   pop\n"
"} bind def\n"
"\n"
"/edge\n"
"{\n"
"   0 eq {\n"
"      % 0\n"
"      -60 rotate\n"
"      line\n"
"   } {\n"
"      % 1\n"
"      60 rotate\n"
"      line\n"
"   } ifelse\n"
"} bind def\n"
"\n"
"%%EndProlog\n"
"%%Page: 1 1\n"
"\n";

   // draw background
   if (!bg_color.empty()) {
      out <<
"bg_color setrgbcolor\n";
      if (matte) out << mleft-grid_thickness/2 << ' '
                     << mtop-grid_thickness/2  << ' '
                     << iw-mleft-mright << ' '
                     << ih-mtop-mbottom << " rectfill\n";
      else out << 
"0 0 " << int(ceil(iw)) << ' ' << int(ceil(ih)) << " rectfill\n";
   }

   // draw grid
   out <<
"newpath\n"
"\n";

   if (grain == Horizontal) out <<
"%\n"
"% adjust for horizontal grain\n"
"%\n"
"/cols rows /rows cols def def\n"
"\n"
"-90 rotate\n"
"0.25 hex_width 0.75 hex_width cols 1 sub mul mul add\n"
"  mtop mbottom add add neg 1 translate\n"
"\n"
"/mright mbottom /mbottom mleft /mleft mtop /mtop mright def def def def\n"
"\n";

   out <<
"%\n"
"% draw the hex grid\n"
"%\n"
"hex_color setrgbcolor\n"
"hex_linewidth setlinewidth\n"
"\n"
"mleft mbottom moveto\n"
"\n"
"lowfirstcol 0 eq {\n"
"   60 rotate\n"
"   no_line\n"
"   -60 rotate\n"
"\n"
"   1 1 2 cols mul cols 2 mod sub { 4 mod side } for\n"
"\n"
"   1 cols 2 mod add 60 mul rotate\n"
"   1 cols 2 mod sub 1 2 rows mul 1 sub cols 2 mod sub { 2 mod edge } for\n"
"\n"
"   1 cols 2 mod add 60 mul rotate\n"
"   cols 2 mod 1 add 1 2 cols mul 2 cols 2 mod mul add { 4 mod side } for\n"
"\n"
"   60 rotate \n"
"   1 1 2 rows mul 1 sub { 2 mod edge } for\n"
"   closepath\n"
"\n"
"   60 rotate\n"
"   rows 1 sub {\n"
"      currentpoint\n"
"\n"
"      3 1 2 cols mul 1 add { 4 mod side_skip } for\n"
"      moveto\n"
"\n"
"      0 hex_height rmoveto\n"
"      currentpoint\n"
"\n"
"      1 1 2 cols mul 1 sub { 4 mod side } for\n"
"      moveto\n"
"   } repeat\n"
"\n"
"   3 1 2 cols mul 1 add { 4 mod side_skip } for\n"
"} {\n"
"   0 0.5 hex_height mul rmoveto\n"
"\n"
"   2 1 2 cols mul 2 add cols 1 add 2 mod sub { 4 mod side } for\n"
"\n"
"   1 cols 1 add 2 mod add 60 mul rotate\n"
"   cols 2 mod 1 2 rows mul 1 sub cols 1 add 2 mod sub { 2 mod edge } for\n"
"\n"
"   1 cols 1 add 2 mod add 60 mul rotate\n"
"   2 cols 2 mod sub 1 2 cols mul 1 add 2 cols 2 mod mul sub { 4 mod side } for\n"
"\n"
"   120 rotate\n"
"   0 1 2 rows mul 2 sub { 2 mod edge } for\n"
"   closepath\n"
"\n"
"   120 rotate\n"
"   0.25 hex_width mul 0.5 hex_height mul rmoveto\n"
"   rows 1 sub {\n"
"      currentpoint\n"
"      currentpoint\n"
"      1 1 2 cols mul 2 sub { 4 mod side_skip } for\n"
"      moveto\n"
"\n"
"      3 1 2 cols mul 1 add { 4 mod side } for\n"
"      moveto\n"
"\n"
"      0 hex_height rmoveto\n"
"   } repeat\n"
"\n"
"   1 1 2 cols mul 2 sub { 4 mod side_skip } for\n"
"} ifelse\n"
"\n"
"stroke\n"
"\n";

   // print the centers
   if (center_style != Centerless) {
      out <<
"%\n"
"% draw the centers\n"
"%\n"
"center_color setrgbcolor\n"
"\n"
"mleft mbottom moveto\n"
"\n"
"hex_width 2 div 2 lowfirstcol sub 0.5 hex_height mul mul rmoveto\n"
"\n"
"lowfirstcol 1 eq { -1 } { 1 } ifelse\n"
"\n"
"cols {\n"
"   currentpoint\n"
"   rows {\n";
      switch (center_style) {
      case Dot:
         out <<
"      currentpoint\n"
"      currentpoint center_size 0 360 arc\n"
"      fill\n"
"      moveto\n"
"      0 hex_height rmoveto\n";
         break;
      case Cross:
         out <<
"      currentpoint\n"
"      currentpoint\n"
"      center_size neg 0 rmoveto\n"
"      2 center_size mul 0 rlineto\n"
"      moveto\n"
"      0 center_size neg rmoveto\n"
"      0 2 center_size mul rlineto\n"
"      moveto\n" 
"      0 hex_height rmoveto\n";
         break;
      default:
         break;
      }

      out <<
"   } repeat\n"
"   moveto\n"
"\n"
"   neg\n"
"   dup\n"
"\n"
"   0.5 hex_height mul mul\n"
"   0.75 hex_width mul\n"
"   exch\n"
"\n"
"   rmoveto\n"
"} repeat \n"
"\n";
   }

   if (coord_display) {
      // print the coordinates
      out <<
"%\n"
"% print the coordinates\n"
"%\n"
"coord_color setrgbcolor\n"
"coord_font findfont\n"
"coord_size scalefont\n"
"setfont\n"
"\n"
"/coord_text [\n";

      if (grain == Horizontal) swap(rows, cols);
   
      for (int c = 0; c < cols; ++c) {
         for (int r = 0; r < rows; ++r) {
            if ((c+coord_cstart) % coord_cskip || 
                (r+coord_rstart) % coord_rskip) {
               out << "() ";
               continue;
            }

            int cc = 0, cr = 0;
   
            switch (coord_origin) {
            case UpperLeft:
               cc = c+coord_cstart;
               cr = rows-r-1+coord_rstart;
               break;
            case UpperRight:
               cc = cols-c-1+coord_cstart;
               cr = rows-r-1+coord_rstart;
               break;
            case LowerLeft:
               cc = c+coord_cstart;
               cr = r+coord_rstart;
               break;
            case LowerRight:
               cc = cols-c-1+coord_cstart;
               cr = r+coord_rstart;
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
   
            out << '(' << s.str() << ") ";
         }
         out << '\n';
      }

      out <<
"] def\n" 
"\n"
"/i 0 def\n"
"\n"
"mleft mbottom moveto\n"
"hex_width 2 div 2 lowfirstcol sub 0.5 hex_height mul mul rmoveto\n"
"lowfirstcol 1 eq { -1 } { 1 } ifelse \n"
"\n"
"cols {\n"
"   currentpoint\n"
"   rows {\n"
"      currentpoint\n"
"\n"
"      coord_text i get stringwidth pop\n"
"      coord_bearing rotate\n"
"      0 coord_distance rmoveto\n"
"      coord_bearing neg rotate\n"
"      coord_tilt rotate\n"
"      2 div neg 0 rmoveto\n"
"      coord_text i get show\n"
"      coord_tilt neg rotate\n"
"\n"
"      moveto\n"
"      0 hex_height rmoveto\n"
"\n"
"      /i i 1 add def\n"
"   } repeat\n"
"   moveto\n"
"   \n"
"   neg\n"
"   dup\n"
"   \n"
"   0.5 hex_height mul mul\n"
"   0.75 hex_width mul\n"
"   exch\n"
"\n"
"   rmoveto\n"
"} repeat\n";
   }

   out <<
"\n"
"stroke\n"
"\n"
"showpage\n"
"\n"
"%%Trailer\n"
"cleartomark countdictstack exch sub { end } repeat restore\n"
"%%EOF\n"
      << endl;

   if (!outfile.empty()) out.close();
}

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

#include <cstdlib>
#include <exception>
#include <iostream>
#include <fstream>
#include <map>
#include <stdexcept>
#include <string>
using namespace std;

#include <boost/lexical_cast.hpp>
using namespace boost;

#include <getopt.h>

#include "grid.h"

void parse_spec(istream &in, map<string, string> &opt);
void print_help();

struct option long_options[] = {
   { "hex-width",          1, 0, 0 },
   { "hex-height",         1, 0, 0 },
   { "hex-side",           1, 0, 0 },
   { "image-width",        1, 0, 0 },
   { "image-height",       1, 0, 0 },
   { "image-margin",       1, 0, 0 },
   { "rows",               1, 0, 0 },
   { "columns",            1, 0, 0 },
   { "grid-color",         1, 0, 0 },
   { "grid-opacity",       1, 0, 0 },
   { "grid-thickness",     1, 0, 0 },
   { "grid-grain",         1, 0, 0 },
   { "grid-start",         1, 0, 0 },
   { "coord-color",        1, 0, 0 },
   { "coord-opacity",      1, 0, 0 },
   { "coord-format",       1, 0, 0 },
   { "coord-font",         1, 0, 0 },
   { "coord-size",         1, 0, 0 },
   { "coord-bearing",      1, 0, 0 },
   { "coord-distance",     1, 0, 0 },
   { "coord-tilt",         1, 0, 0 },
   { "coord-column-skip",  1, 0, 0 },
   { "coord-row-skip",     1, 0, 0 },
   { "coord-column-start", 1, 0, 0 },
   { "coord-row-start",    1, 0, 0 },
   { "coord-origin",       1, 0, 0 },
   { "bg-color",           1, 0, 0 },
   { "bg-opacity",         1, 0, 0 },
   { "matte",              0, 0, 0 },
   { "center-style",       1, 0, 0 },
   { "center-color",       1, 0, 0 },
   { "center-opacity",     1, 0, 0 },
   { "center-size",        1, 0, 0 },
   { "antialias",          0, 0, 0 },
   { "centered",           0, 0, 0 },
   { "output",             1, 0, 0 },
   { "outfile",            1, 0, 'o' },
   { "help",               0, 0, 'h' },
   { "version",            0, 0, 'v' },
   { 0, 0, 0, 0 }
};


int main(int argc, char **argv)
{
   map<string, string> opt;

   int c;
   int option_index = 0;

   while (1) {
      c = getopt_long(argc, argv, "i:o:", long_options, &option_index);
      if (c == -1) break;  // end of options

      switch (c) {
      case 0:
         opt[long_options[option_index].name] = optarg ? optarg : "";
         break;
      case 'i':
         opt["infile"] = optarg;
         break;
      case 'o':
         opt["outfile"] = optarg;
         break;
      case 'v':
         cout << "mkhexgrid version " << VERSION << endl;
         exit(0);
      case 'h':
         print_help();
         exit(0);
      default:
         exit(1);    // getopt produces an error message
      }
   }

   try {
      // get name of specfile, if given as a positional parameter
      if (optind < argc) {
         if (opt.find("infile") != opt.end())
            throw runtime_error("specfile specified twice on command line");
         
         opt["infile"] = argv[optind++];

         // get name of outfile, if given as a positional parameter
         if (optind < argc) {
            if (opt.find("outfile") != opt.end()) 
             throw runtime_error("output file specified twice on command line");
            opt["outfile"] = argv[optind++];

            if (optind < argc) throw runtime_error("too many arguments");
         }
      }

      // read and parse specfile
      map<string,string>::const_iterator i = opt.find("infile");
      if (i != opt.end()) {
         if (i->second == "-") parse_spec(cin, opt); 
         else {
            ifstream in;
            in.open(i->second.c_str(), ios::in | ios::binary);
            if (!in)
             throw runtime_error("cannot read spec file `" + i->second + "'");
            parse_spec(in, opt);
            in.close();
         }
      } 
   
      // draw hex grid
      Grid g(opt);
      g.draw();
   }
   catch (exception &e) {
      cerr << argv[0] << ": " << e.what() << endl;
      exit(1);
   }

   return 0;
}


void parse_spec(istream &in, map<string, string> &opt)
{
   string key, val;
   unsigned int line = 1, col = 0;
   char c;
   enum { START, KEY, PRE_EQ, POST_EQ, VAL, QVAL, GUARD, END }
      state = START;
   
   while (in.get(c)) {
      ++col;
      switch (state) {
      case START:
         switch (c) {
         case '\n':
            ++line;
            col = 0;
         case ' ':
         case '\t':
         case '\r':
            break;
         case '#':
            do { 
               in.get(c);
               if (c == '\n') {
                  ++line;
                  col = 0;
                  break;
               }
            } while (in);
            break;
         default:
            key += c;
            state = KEY;
            break;
         }
         break;
      case KEY:
         switch (c) {
         case ' ':
         case '\t':
         {
            unsigned int i = 0;
            for ( ; long_options[i].name &&
                    long_options[i].name != key; ++i);
            if (!long_options[i].name)
               throw runtime_error("unrecognized option `" + key +
                  "' in specfile");
            state = PRE_EQ;
            break;
         }
         case '\r':
         case '\n':
         case '#':
            throw runtime_error("malformed spec file at"
               " line " + lexical_cast<string>(line) + ","
               " column " + lexical_cast<string>(col));
            break;
         case '=':
         {
            unsigned int i = 0;
            for ( ; long_options[i].name &&
                    long_options[i].name != key; ++i);
            if (!long_options[i].name)
               throw runtime_error("unrecognized option `" + key +
                  "' in specfile");
            state = POST_EQ;
            break;
         }
         default:
            key += c;
            break;
         }
         break;
      case PRE_EQ:
         switch (c) {
         case ' ':
         case '\t':
            break;
         case '=':
            state = POST_EQ;
            break;
         default:
            throw runtime_error("malformed spec file at"
               " line " + lexical_cast<string>(line) + ","
               " column " + lexical_cast<string>(col));
         }
         break;
      case POST_EQ:
         switch (c) {
         case ' ':
         case '\t':
            break;
         case '\r':
         case '\n':
         case '#':
            throw runtime_error("malformed spec file at"
               " line " + lexical_cast<string>(line) + ","
               " column " + lexical_cast<string>(col));
         case '"':
            state = QVAL;
            break;
         default:
            val += c;
            state = VAL;
            break;
         }
         break;
      case QVAL:
         switch (c) {
         case '"':
            if (opt.find(key) == opt.end()) opt[key] = val;
            key.clear();
            val.clear();
            state = END;
            break;
         case '\\':
            state = GUARD;
            break;
         case '\n':
            throw runtime_error("malformed spec file at"
               " line " + lexical_cast<string>(line) + ","
               " column " + lexical_cast<string>(col));
         default:
            val += c;
            break;
         }
         break;
      case GUARD:
         switch (c) {
         case '\\':
            val += "\\\\";
            break;
         case '"':
            val += '"';
            break;
         case '\n':
            throw runtime_error("malformed spec file at"
               " line " + lexical_cast<string>(line) + ","
               " column " + lexical_cast<string>(col));
         default:
            val += "\\" + c;
            break;
         }
         state = QVAL;
         break;
      case VAL:
         switch (c) {
         case '=':
         case '#':
            throw runtime_error("malformed spec file at"
               " line " + lexical_cast<string>(line) + ","
               " column " + lexical_cast<string>(col));
         case '\n':
            ++line;
            col = 0;
            if (opt.find(key) == opt.end()) opt[key] = val;
            key.clear();
            val.clear();
            state = START;
            break;
         case '\r':
         case '\t':
         case ' ':
            if (opt.find(key) == opt.end()) opt[key] = val;
            key.clear();
            val.clear();
            state = END;
            break;
         default:
            val += c;
            break;
         }
         break;
      case END:
         switch (c) {
         case '\n':
            ++line;
            col = 0;
            state = START;
            break;
         case ' ':
         case '\t':
         case '\r':
            break;
         case '#':
            do { 
               in.get(c);
               if (c == '\n') {
                  ++line;
                  col = 0;
                  break;
               }
            } while (in);
            state = START;
            break;
         default:
            throw runtime_error("malformed spec file at"
               " line " + lexical_cast<string>(line) + ","
               " column " + lexical_cast<string>(col));
         }
         break;
      }
   }
   
   switch (state) {
   case START:
   case END:
      break;
   case QVAL:
   case GUARD:
   case KEY:
   case PRE_EQ:
   case POST_EQ:
      throw runtime_error("malformed spec file at"
         " line " + lexical_cast<string>(line) + ","
         " column " + lexical_cast<string>(col));
   case VAL:
      if (opt.find(key) == opt.end()) opt[key] = val;
      break; 
   }
}


void print_help()
{
   cout <<
"Usage: mkhexgrid [OPTION]... [SPECFILE [OUTFILE]]\n"
"Make hexagonal grids.\n"
"\n"
"Options:\n"
"   --hex-side=LENGTH        set hex sdie to LENGTH\n"
"   --hex-width=LENGTH       set hex width to LENGTH\n"
"   --hex-height=LENGTH      set hex height to LENGTH\n"
"   --columns=N              set number of hex columns to N\n"
"   --rows=N                 set number of hex rows to N\n"
"   --image-width=LENGTH     set image width to LENGTH\n"
"   --image-height=LENGTH    set image height to LENGTH\n"
"   --margin=LENGTH          set image margins to LENGTH\n"
"   --margin=T,R,B,L         set image margins independently\n"
"   --grid-color=COLOR       set grid color to COLOR\n"
"   --grid-opacity=OPACITY   set grid opacity to OPACITY\n"
"   --grid-width=SIZE        set width of grid lines to SIZE\n" 
"   --grid-grain=G           set grid grain, G = h, v\n"    
"   --grid-start=S           set first column/row to start in (i) or out (o)\n"
"   --coord-color=COLOR      set coordinates color to COLOR\n"
"   --coord-opacity=OPACITY  set coordinates opacity to OPACITY\n"
"   --coord-format=FORMAT    set coordinates format to FORMAT\n"
"   --coord-font=FONT        set coordinates font to FONT\n"
"   --corod-size=SIZE        set coordinates font size to SIZE\n"
"   --coord-bearing=ANGLE    place coordinates at ANGLE from hex centers\n"
"   --coord-distance=LENGTH  place coordinates LENGTH from hex centers\n"
"   --coord-tilt=ANGLE       tilt coordinates by ANGLE\n"
"   --coord-column-skip=N    number columns divisible by N\n"
"   --coord-row-skip=N       number rows divisible by N\n"
"   --coord-column-start=N   number columns starting from N\n"
"   --coord-row-start=N      number rows starting from N\n"
"   --coord-origin=O         set coordinate origin O = ul, ur, ll, lr\n"
"   --bg-color=COLOR         set background color to COLOR\n"
"   --bg-opacity=OPACITY     set background opacity to OPACITY\n"
"   --matte                  clip background at margins\n"               
"   --center-style=STYLE     set hex center style STYLE = n, d, c\n"
"   --center-color=COLOR     set hex center color to COLOR\n"
"   --center-size=SIZE       set hex center size to SIZE\n"
"   --antialias              turn on antialiased output\n"
"   --centered               center grid within the image margins\n"
"   --output=TYPE            set output TYPE = png, ps, svg\n"
"-o --outfile=FILE           set output filename to FILE\n"
"   --help                   display this help and exit\n"
"   --version                display version information and exit\n"
"\n"
"Please visit http:://www.nomic.net/~uckelman/mkhexgrid for updates and bug\n"
"reports.\n";
}

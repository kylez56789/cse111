// $Id: interp.cpp,v 1.3 2019-03-19 16:18:22-07 - - $

#include <memory>
#include <string>
#include <vector>
using namespace std;

#include <GL/freeglut.h>

#include "debug.h"
#include "interp.h"
#include "shape.h"
#include "util.h"

static unordered_map<void*,string> fontname {
   {GLUT_BITMAP_8_BY_13       , "Fixed-8x13"    },
   {GLUT_BITMAP_9_BY_15       , "Fixed-9x15"    },
   {GLUT_BITMAP_HELVETICA_10  , "Helvetica-10"  },
   {GLUT_BITMAP_HELVETICA_12  , "Helvetica-12"  },
   {GLUT_BITMAP_HELVETICA_18  , "Helvetica-18"  },
   {GLUT_BITMAP_TIMES_ROMAN_10, "Times-Roman-10"},
   {GLUT_BITMAP_TIMES_ROMAN_24, "Times-Roman-24"},
};

unordered_map<string,interpreter::interpreterfn>
interpreter::interp_map {
   {"define" , &interpreter::do_define },
   {"draw"   , &interpreter::do_draw   },
};

unordered_map<string,interpreter::factoryfn>
interpreter::factory_map {
   {"text"     , &interpreter::make_text     },
   {"ellipse"  , &interpreter::make_ellipse  },
   {"circle"   , &interpreter::make_circle   },
   {"polygon"  , &interpreter::make_polygon  },
   {"rectangle", &interpreter::make_rectangle},
   {"square"   , &interpreter::make_square   },
	{"diamond"	, &interpreter::make_diamond	},
	{"triangle" , &interpreter::make_triangle },
	{"equilateral", &interpreter::make_equilateral },
};

interpreter::shape_map interpreter::objmap;
static rgbcolor color_border;

interpreter::~interpreter() {
   for (const auto& itor: objmap) {
      cout << "objmap[" << itor.first << "] = "
           << *itor.second << endl;
   }
}

void interpreter::interpret (const parameters& params) {
   DEBUGF ('i', params);
   param begin = params.cbegin();
   string command = *begin;
   auto itor = interp_map.find (command);
   if (itor == interp_map.end()) throw runtime_error ("syntax error");
   interpreterfn func = itor->second;
   func (++begin, params.cend());
	   if (command == "moveby") {
      ++begin;
      window::pixels_to_move_by = strtof((*begin).c_str(), 0);
      return;
   }  
   if (command == "border") {
      rgbcolor new_color {begin[1]};
		window_dimensions::border_color = new_color;
		window::border_width = strtof((begin[2]).c_str(), 0);
      return;
   }  
}

void interpreter::do_define (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   string name = *begin;
   objmap.emplace (name, make_shape (++begin, end));
}


void interpreter::do_draw (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   if (end - begin != 4) throw runtime_error ("syntax error");
   string name = begin[1];
   shape_map::const_iterator itor = objmap.find (name);
   if (itor == objmap.end()) {
      throw runtime_error (name + ": no such shape");
   }
   rgbcolor color {begin[0]};
   vertex where {from_string<GLfloat> (begin[2]),
                 from_string<GLfloat> (begin[3])};
   window::push_back (object (itor->second, where, color));
}

shape_ptr interpreter::make_shape (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   string type = *begin++;
   auto itor = factory_map.find(type);
   if (itor == factory_map.end()) {
      throw runtime_error (type + ": no such shape");
   }
   factoryfn func = itor->second;
   return func (begin, end);
}

shape_ptr interpreter::make_text (param begin, param end) {
   DEBUGF ('f', range (begin, end));
	string text = "";
	string font = *begin;
	++begin;
	if (fontname.find(font) == fontname.end())
      throw runtime_error 
         ("Define failed: font not found in list of fonts");	
	while (begin != end) {
		text = text + (*begin);
		++begin;
		text += " ";
	}
   return make_shared<text> (font, text);
}

shape_ptr interpreter::make_ellipse (param begin, param end) {
   DEBUGF ('f', range (begin, end));
	GLfloat width = strtof((*begin++).c_str(), 0);
	GLfloat height = strtof((*begin).c_str(), 0);
   return make_shared<ellipse> (width, height);
}

shape_ptr interpreter::make_circle (param begin, param end) {
   DEBUGF ('f', range (begin, end));
	GLfloat diameter = strtof((*begin).c_str(), 0);
   return make_shared<circle> (diameter);
}

shape_ptr interpreter::make_polygon (param begin, param end) {
   DEBUGF ('f', range (begin, end));
	vertex_list vertexes;
	GLfloat xcoord;
	GLfloat ycoord;
	while (begin != end) {
		xcoord = strtof((*begin++).c_str(), 0);
		ycoord = strtof((*begin++).c_str(), 0);
		vertexes.push_back({xcoord, ycoord});
	}
   return make_shared<polygon> (vertexes);
}

shape_ptr interpreter::make_rectangle (param begin, param end) {
   DEBUGF ('f', range (begin, end));
	GLfloat width = strtof((*begin++).c_str(), 0);
	GLfloat height = strtof((*begin).c_str(), 0);
   return make_shared<rectangle> (width, height);
}

shape_ptr interpreter::make_square (param begin, param end) {
   DEBUGF ('f', range (begin, end));
	GLfloat side = strtof((*begin).c_str(), 0);
   return make_shared<square> (side);
}

shape_ptr interpreter::make_diamond(param begin, param end) {
	DEBUGF ('f', range (begin, end));
   GLfloat width = strtof((*begin++).c_str(), 0);
   GLfloat height = strtof((*begin).c_str(), 0);
   return make_shared<diamond> (width, height);

}

shape_ptr interpreter::make_triangle(param begin, param end) {
	DEBUGF ('f', range (begin, end));
   vertex_list vertexes;
   GLfloat xcoord;
   GLfloat ycoord;
   while (begin != end) {
      xcoord = strtof((*begin++).c_str(), 0);
      ycoord = strtof((*begin++).c_str(), 0);
      vertexes.push_back({xcoord, ycoord});
   }
   return make_shared<triangle>(vertexes);
}

shape_ptr interpreter::make_equilateral(param begin, param end) {
	DEBUGF ('f', range (begin, end));
	GLfloat width = strtof((*begin).c_str(), 0);	
	return make_shared<equilateral>(width);
}

// $Id: shape.cpp,v 1.2 2019-02-28 15:24:20-08 - - $

#include <typeinfo>
#include <unordered_map>
#include <cmath>
using namespace std;

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

static unordered_map<string,void*> fontcode {
   {"Fixed-8x13"    , GLUT_BITMAP_8_BY_13       },
   {"Fixed-9x15"    , GLUT_BITMAP_9_BY_15       },
   {"Helvetica-10"  , GLUT_BITMAP_HELVETICA_10  },
   {"Helvetica-12"  , GLUT_BITMAP_HELVETICA_12  },
   {"Helvetica-18"  , GLUT_BITMAP_HELVETICA_18  },
   {"Times-Roman-10", GLUT_BITMAP_TIMES_ROMAN_10},
   {"Times-Roman-24", GLUT_BITMAP_TIMES_ROMAN_24},
};

ostream& operator<< (ostream& out, const vertex& where) {
   out << "(" << where.xpos << "," << where.ypos << ")";
   return out;
}

shape::shape() {
   DEBUGF ('c', this);
}

text::text (void* glut_bitmap_font_, const string& textdata_):
      glut_bitmap_font(glut_bitmap_font_), textdata(textdata_) {
   DEBUGF ('c', this);
}

ellipse::ellipse (GLfloat width, GLfloat height):
dimension ({width, height}) {
   DEBUGF ('c', this);
}

circle::circle (GLfloat diameter): ellipse (diameter, diameter) {
   DEBUGF ('c', this);
}


polygon::polygon (const vertex_list& vertices_): vertices(vertices_) {
   DEBUGF ('c', this);
}

rectangle::rectangle (GLfloat width, GLfloat height):
            polygon({{-width,-height},{-width,height},
                     {width,-height},{width,height}}) {
   DEBUGF ('c', this << "(" << width << "," << height << ")");
}

square::square (GLfloat width): rectangle (width, width) {
   DEBUGF ('c', this);
}

diamond::diamond (GLfloat width, GLfloat height):
               polygon({{width/2,0},{-width/2,0},
                        {0,height/2},{0,-height/2}}) {

}
triangle::triangle(const vertex_list& vertices_): polygon(vertices_) {
//Made decision if triangle vertices > 3, then we only take the first 3
}

equilateral::equilateral(GLfloat width): 
            triangle({{width/2,0},{-width/2,0},{0,width}}) {

}
void text::draw (const vertex& center, const rgbcolor& color) const {
   static const string print = textdata;
   void* font = glut_bitmap_font;
   auto u_str = reinterpret_cast<const GLubyte*>(print.c_str());
   //int str_width = glutBitmapLength (font, u_str);
   //int str_height = glutBitmapHeight (font);
   glColor3ubv (color.ubvec);
   GLfloat xpos = center.xpos;//Left side of string
   GLfloat ypos = center.ypos;//Bottom of string
   glRasterPos2f(xpos,ypos);
   glutBitmapString(font,u_str);
   
   DEBUGF ('d', this << "(" << center << "," << color << ")");
}

void text::draw_border (const vertex& center, const rgbcolor& color) {
   glBegin(GL_LINE_LOOP);
   glColor3ubv(color.ubvec);
   static const string print = textdata;
   void* font = glut_bitmap_font;
   auto u_str = reinterpret_cast<const GLubyte*>(print.c_str());
   int str_width = glutBitmapLength (font, u_str);
   int str_height = glutBitmapHeight (font);
   vertex_list v_list;
   v_list.push_back({center.xpos,center.ypos});
   v_list.push_back({center.xpos+str_width,center.ypos});
   v_list.push_back({center.xpos,center.ypos+str_height});
   v_list.push_back({center.xpos+str_width,center.ypos+str_height});
   //glLineWidth eventually
   for(GLfloat num = 0; num < v_list.size(); ++num) {
      const GLfloat x_pos = v_list[num].xpos;
      const GLfloat y_pos = v_list[num].ypos;
      glVertex2f(x_pos,y_pos);
   }
   glEnd();
}

void ellipse::draw (const vertex& center, const rgbcolor& color) const {
   const GLfloat delta = 2 * M_PI / 64;
   glBegin(GL_POLYGON);
   cout << endl;
   glColor3ubv(color.ubvec);//colors.h
   for (GLfloat theta = 0; theta < 2 * M_PI; theta += delta) {
         const GLfloat x_pos = this->dimension.xpos * cos (theta) + center.xpos;
         const GLfloat y_pos = this->dimension.ypos * sin (theta) + center.ypos;
         glVertex2f (x_pos, y_pos);
   }
   glEnd();
   DEBUGF ('d', this << "(" << center << "," << color << ")");
}

void ellipse::draw_border (const vertex& center, const rgbcolor& color) {
   const GLfloat delta = 2 * M_PI / 64;
   glBegin(GL_LINE_LOOP);
   glColor3ubv(color.ubvec);//colors.h
   for (GLfloat theta = 0; theta < 2 * M_PI; theta += delta) {
         const GLfloat x_pos = this->dimension.xpos * cos (theta) + center.xpos;
         const GLfloat y_pos = this->dimension.ypos * sin (theta) + center.ypos;
         glVertex2f (x_pos, y_pos);
   }
   glEnd();
   DEBUGF ('d', this << "(" << center << "," << color << ")");
}
void polygon::draw (const vertex& center, const rgbcolor& color) const{
   glBegin(GL_POLYGON);
   glColor3ubv(color.ubvec);
   GLfloat x_total = 0;
   GLfloat y_total = 0;
   GLfloat vec_size = vertices.size();
   for(GLfloat num = 0; num < vec_size; ++num) {
      x_total += vertices[num].xpos;
      y_total += vertices[num].ypos;
   }
   GLfloat x_avg = x_total/vec_size;
   GLfloat y_avg = y_total/vec_size;
   for(GLfloat num_vertex = 0; num_vertex < vec_size; ++num_vertex) {
      const GLfloat x_pos = (vertices[num_vertex].xpos - x_avg) + center.xpos;
      const GLfloat y_pos = (vertices[num_vertex].ypos - y_avg) + center.ypos;
      glVertex2f(x_pos,y_pos);
   }
   glEnd();
   
   DEBUGF ('d', this << "(" << center << "," << color << ")");
}

void polygon::draw_border (const vertex& center, const rgbcolor& color) {
   glBegin(GL_LINE_LOOP);
   glColor3ubv(color.ubvec);
   GLfloat x_total = 0;
   GLfloat y_total = 0;
   GLfloat vec_size = vertices.size();
   for(GLfloat num = 0; num < vec_size; ++num) {
      x_total += vertices[num].xpos;
      y_total += vertices[num].ypos;
   }
   GLfloat x_avg = x_total/vec_size;
   GLfloat y_avg = y_total/vec_size;
   for(GLfloat num_vertex = 0; num_vertex < vec_size; ++num_vertex) {
      const GLfloat x_pos = (vertices[num_vertex].xpos - x_avg) + center.xpos;
      const GLfloat y_pos = (vertices[num_vertex].ypos - y_avg) + center.ypos;
      glVertex2f(x_pos,y_pos);
   }
   glEnd();
}


void shape::show (ostream& out) const {
   out << this << "->" << demangle (*this) << ": ";
}

void text::show (ostream& out) const {
   shape::show (out);
   out << glut_bitmap_font << "(" << fontname[glut_bitmap_font]
       << ") \"" << textdata << "\"";
}

void ellipse::show (ostream& out) const {
   shape::show (out);
   out << "{" << dimension << "}";
}

void polygon::show (ostream& out) const {
   shape::show (out);
   out << "{" << vertices << "}";
}

ostream& operator<< (ostream& out, const shape& obj) {
   obj.show (out);
   return out;
}


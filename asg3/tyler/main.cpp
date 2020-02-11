// $Id: main.cpp,v 1.11 2018-01-25 14:19:29-08 - - $

#include <cstdlib>
#include <exception>
#include <fstream>
#include <iostream>
#include <string>
#include <unistd.h>
#include <cassert>
#include <regex>


using namespace std;

#include "listmap.h"
#include "xpair.h"
#include "util.h"

using str_str_map = listmap<string,string>;
using str_str_pair = str_str_map::value_type;

const string cin_name = "-";

void scan_options (int argc, char** argv) {
   opterr = 0;
   for (;;) {
      int option = getopt (argc, argv, "@:");
      if (option == EOF) break;
      switch (option) {
         case '@':
            debugflags::setflags (optarg);
            break;
         default:
            complain() << "-" << char (optopt) << ": invalid option"
                       << endl;
            break;
      }
   }
}

void process_file( istream& infile, const string& filename) {
   regex comment_regex {R"(^\s*(#.*)?$)"};
   regex key_value_regex {R"(^\s*(.*?)\s*=\s*(.*?)\s*$)"};
   regex trimmed_regex {R"(^\s*([^=]+?)\s*$)"};
   for(;;) {
      string line;
      getline(infile, line);
      if (infile.eof()) {
         break;
      }
      smatch result;
      if (regex_search (line, result, comment_regex)) {
         cout << "Comment or empty line." << endl;
         continue;
      }
      if (regex_search (line, result, key_value_regex)) {
         cout << "key  : \"" << result[1] << "\"" << endl;
         cout << "value: \"" << result[2] << "\"" << endl;
      }else if (regex_search (line, result, trimmed_regex)) {
         cout << "query: \"" << result[1] << "\"" << endl;
      }else {
         assert (false and "This can not happen.");
      }
      cout << endl;
   }   
}

int main (int argc, char** argv) {
   sys_info::execname (argv[0]);
   scan_options (argc, argv);
   
   vector<string> filenames;  
   str_str_map test;
   for (char** argp = &argv[optind]; argp != &argv[argc]; ++argp) {
      str_str_pair pair (*argp, to_string<int> (argp - argv));
      filenames.push_back(*argp);
      cout << "Before insert: " << pair << endl;
      test.insert (pair);
   }


   if(filenames.size() == 0) {
      filenames.push_back(cin_name);
   }
   for(int a = 0; a < filenames.size(); a++) {
      cout << "filenames " << a << ":" << filenames[a] << endl;
   }
   for(const auto& filename: filenames) {
      if(filename == cin_name) {
         cout << "Reading from cin" << endl;
         process_file(cin, filename); 
      }
      else {
         ifstream infile (filename);
         if(infile.fail()) {
            cerr << "No such file or directory" << endl;
         }
         else {
            cout << "Read from file " << filename << endl;
            process_file(infile, filename);
            infile.close();
         }
      }
   }




   for (str_str_map::iterator itor = test.begin();
        itor != test.end(); ++itor) {
      cout << "During iteration: " << *itor << endl;
   }
   
   str_str_map::iterator itor = test.begin();
   test.erase (itor);

   cout << "EXIT_SUCCESS" << endl;
   return EXIT_SUCCESS;
}


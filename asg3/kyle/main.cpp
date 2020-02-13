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

//using key_map_map = listmap<key_t, mapped_t>;

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

void process_file(istream& infile, const string& filename,str_str_map& list) {
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
         if(result[1] == "" && result[2] == "") {
            for (str_str_map::iterator itor = list.begin();
                 itor != list.end(); ++itor) {
               cout << "= " << *itor << endl;
            }
         }
         else if (result[1] != "" && result[2] == "") {
            str_str_map::iterator itor = list.find(result[1]);
            if (itor != list.end()) {
               list.erase(itor);
            }
            else {
               cout << "Error msg for deleting node not in map?\n";
            }   
         }
         else if (result[1] == "" && result[2] != "") {
            for (str_str_map::iterator itor = list.begin();
                 itor != list.end(); ++itor) {
               if((*itor).second == result[2]) {
                  cout << "= " << *itor << endl;
               }
            }
         }
         else {
            cout << "Insert based on key or replace value" << endl;
            str_str_pair to_be_inserted (result[1],result[2]);
            list.insert(to_be_inserted);
         }
      }else if (regex_search (line, result, trimmed_regex)) {
         str_str_map::iterator itor = list.find(result[1]);
         if (itor == list.end()) {
            cerr << result[1] << ": key not found" << endl;
         }
         else {
            cout << *itor << endl;
         }   
         
      }else {
         assert (false and "This can not happen.");
      }
      cout << endl;
   }   
}

int main (int argc, char** argv) {
   sys_info::execname (argv[0]);
   scan_options (argc, argv);
   str_str_map list {};
   vector<string> filenames;  
   str_str_map test {};
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
			process_file(cin, filename, list);
      }
      else {
         ifstream infile (filename);
         if(infile.fail()) {
            cerr << "No such file or directory" << endl;
         }
         else {
            cout << "Read from file " << filename << endl;
            process_file(infile, filename, list);
            infile.close();
         }
      }
   }
   /*
   for (str_str_map::iterator itor = list.begin();
		//itor != test.end(); ++itor) {
		itor != list.end(); ++itor) {
      cout << "During iteration: " << *itor << endl;
   }
   */
   str_str_map::iterator itor = list.begin();
   list.erase (itor);

   cout << "EXIT_SUCCESS" << endl;
   return EXIT_SUCCESS;
}


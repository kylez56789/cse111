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

void process_file(istream& infile, const string& filename,
                                       str_str_map& list) {
   regex comment_regex {R"(^\s*(#.*)?$)"};
   regex key_value_regex {R"(^\s*(.*?)\s*=\s*(.*?)\s*$)"};
   regex trimmed_regex {R"(^\s*([^=]+?)\s*$)"};
   int linenum = 1;
   for(;;) {
      string line;
      if (infile.eof()) {
         break;
      }
      cout << filename << ": " << linenum << ": ";
      getline(infile, line);
      cout << line << endl;
      //if (infile.eof()) {
      //   break;
      //}
      smatch result;
      if (regex_search (line, result, comment_regex)) {
         linenum++;
         //cout << endl;
         continue;
      }
      if (regex_search (line, result, key_value_regex)) {
         if(result[1] == "" && result[2] == "") {
            bool printed = false;
            for (str_str_map::iterator itor = list.begin();
               itor != list.end(); ++itor) {
               cout<<itor->first<<" = "<<itor->second<<endl;
               printed = true;
            }
            if(printed == false) {
               //cout << endl;
            }
         }
         else if (result[1] != "" && result[2] == "") {
            str_str_map::iterator itor = list.find(result[1]);
            if (itor != list.end()) {
               list.erase(itor);
            } 
            //cout << endl;  
         }
         else if (result[1] == "" && result[2] != "") {
            bool printed = false;
            for (str_str_map::iterator itor = list.begin();
                 itor != list.end(); ++itor) {
               if((*itor).second == result[2]) {
                  cout<<itor->first<<" = "<<itor->second<<endl;
                  printed = true;
               }
            }
            if(printed == false) {
               //cout << endl;
            }
         }
         else {
            str_str_pair to_be_inserted (result[1],result[2]);
            list.insert(to_be_inserted);
            cout << result[1] << " = " << result[2]<<endl;
         }
      }else if (regex_search (line, result, trimmed_regex)) {
         str_str_map::iterator itor = list.find(result[1]);
         if (itor == list.end()) {
            cerr << result[1] << ": key not found"<<endl;
         }
         else {
             cout<<itor->first<<" = "<<itor->second<<endl;
         }   
         
      }else {
         assert (false and "This can not happen.");
      }
      linenum++;
   }   
}

int main (int argc, char** argv) {
   sys_info::execname (argv[0]);
   scan_options (argc, argv);
   str_str_map list {};
   vector<string> filenames;  
   for (char** argp = &argv[optind]; argp != &argv[argc]; ++argp) {
      str_str_pair pair (*argp, to_string<int> (argp - argv));
      filenames.push_back(*argp);
   }

   if(filenames.size() == 0) {
      filenames.push_back(cin_name);
   }
   for(const auto& filename: filenames) {
      if(filename == cin_name) {
         process_file(cin, filename, list);
      }
   else {
         ifstream infile (filename);
         if(infile.fail()) {
            complain() << filename << 
               ": No such file or directory" << endl;
            sys_info::exit_status(1);
         }
         else {
            process_file(infile, filename, list);
            infile.close();
         }
      }
   }
   str_str_map::iterator itor = list.begin();
   list.erase (itor);
   return sys_info::exit_status();
}


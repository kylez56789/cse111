// $Id: cix.cpp,v 1.9 2019-04-05 15:04:28-07 - - $

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
using namespace std;

#include <libgen.h>
#include <sys/types.h>
#include <unistd.h>

#include "protocol.h"
#include "logstream.h"
#include "sockets.h"

logstream outlog (cout);
struct cix_exit: public exception {};

unordered_map<string,cix_command> command_map {
   {"exit", cix_command::EXIT},
   {"help", cix_command::HELP},
   {"ls"  , cix_command::LS  },
   {"put" , cix_command::PUT },
};
using wordvec = vector<string>;
wordvec split (const string& line, const string& delimiters) {
   wordvec words;
   size_t end = 0;

   // Loop over the string, splitting out words, and for each word
   //    // thus found, append it to the output wordvec.
   for (;;) {
      size_t start = line.find_first_not_of (delimiters, end);
      if (start == string::npos) break;
         end = line.find_first_of (delimiters, start);
         words.push_back (line.substr (start, end - start));
      }
      //DEBUGF ('u', words);
      return words;
   }

static const char help[] = R"||(
exit         - Exit the program.  Equivalent to EOF.
get filename - Copy remote file to local host.
help         - Print help summary.
ls           - List names of files on remote server.
put filename - Copy local file to remote host.
rm filename  - Remove file from remote server.
)||";

bool valid_filename(string file_name) {
   bool valid = true;
   int name_len = static_cast<int>(file_name.length());
   for(int iter = 0; iter < name_len; ++iter) {
      if(file_name.at(iter) == '/') {
         cout << "problem char at index " << iter << endl;
         valid = false;
         return valid;
      }
      if(iter == name_len && file_name.at(iter) != '\0') {
         valid = false;
         return valid;
      }
   }
   return valid;
}


void cix_help() {
   cout << help;
}

void cix_ls (client_socket& server) {
   cix_header header;
   header.command = cix_command::LS;
   outlog << "sending header " << header << endl;
   send_packet (server, &header, sizeof header);
   recv_packet (server, &header, sizeof header);
   outlog << "received header " << header << endl;
   if (header.command != cix_command::LSOUT) {
      outlog << "sent LS, server did not return LSOUT" << endl;
      outlog << "server returned " << header << endl;
   }else {
      auto buffer = make_unique<char[]> (header.nbytes + 1);
      recv_packet (server, buffer.get(), header.nbytes);
      outlog << "received " << header.nbytes << " bytes" << endl;
      buffer[header.nbytes] = '\0';
      cout << buffer.get();
   }
}

void cix_put (client_socket& server, string file_name) {
   cix_header header;
   header.command = cix_command::PUT;
   if(not valid_filename(file_name)) {
      cout << "Error: Invalid file name " << file_name << endl;
   }
   else {
      cout << "Valid File Name " << file_name << endl;
      int size = sizeof(header.filename);
      strncpy(header.filename, file_name.c_str(), size);
      //TODO(tystran): Set file name
      outlog << "sending header " << header << endl;
      send_packet (server, &header, sizeof header);
      //Probably need to send another packet including the file
      //Algorithm:
      //Send packet with name
      //Send packet(s) with data(maybe split up)
      //Send packet with eof to end
      recv_packet (server, &header, sizeof header);
      outlog << "received header " << header << endl;
      if(header.command != cix_command::ACK) {
         outlog << "Sent put, server didn't return ACK" << endl;
         outlog << "Server returned " << header << endl;
      }
      else {
         outlog <<" Put command successful" << endl;
      }
   }
}

void usage() {
   cerr << "Usage: " << outlog.execname() << " [host] [port]" << endl;
   throw cix_exit();
}

int main (int argc, char** argv) {
   outlog.execname (basename (argv[0]));
   outlog << "starting" << endl;
   vector<string> args (&argv[1], &argv[argc]);
   if (args.size() > 2) usage();
   string host = get_cix_server_host (args, 0);
   in_port_t port = get_cix_server_port (args, 1);
   outlog << to_string (hostinfo()) << endl;
   try {
      outlog << "connecting to " << host << " port " << port << endl;
      client_socket server (host, port);
      outlog << "connected to " << to_string (server) << endl;
      for (;;) {
         string line;
         getline (cin, line);
         if (cin.eof()) throw cix_exit();
         wordvec words = split(line, " \t");
         if (words.size() == 0) {
            continue;//Handles empty lines
         }
         outlog << "command " << line << endl;
         const auto& itor = command_map.find (words[0]);
         cix_command cmd = itor == command_map.end()
                         ? cix_command::ERROR : itor->second;
         string filename;
         if(cmd == cix_command::PUT || cmd == cix_command::GET) {
            //TODO(tystran): add more to this statement rm
            //Get file name if there, if not error to outlog
            if (words.size() < 2) {
               outlog << words[0] << ":requires filename" << endl;
               continue;
            }
            if (words.size() > 2) {
               outlog << words[0] << ": too many args, " 
                      << "only taking first" << endl;
               continue;
            }
            filename = words[1];
         }
         switch (cmd) {
            case cix_command::EXIT:
               throw cix_exit();
               break;
            case cix_command::HELP:
               cix_help();
               break;
            case cix_command::LS:
               cix_ls (server);
               break;
            case cix_command::PUT:
               //TODO(tystran): file names

               cix_put (server, filename);
               break;
            default:
               outlog << words[0] << ": invalid command" << endl;
               break;
         }
      }
   }catch (socket_error& error) {
      outlog << error.what() << endl;
   }catch (cix_exit& error) {
      outlog << "caught cix_exit" << endl;
   }
   outlog << "finishing" << endl;
   return 0;
}


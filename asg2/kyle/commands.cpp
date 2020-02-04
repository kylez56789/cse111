// $Id: commands.cpp,v 1.18 2019-10-08 13:55:31-07 - - $
// Tyler Tran tystran
// Kyle Zhang kmzhang
#include "commands.h"
#include "debug.h"

command_hash cmd_hash {
   {"cat"   , fn_cat   },
   {"cd"    , fn_cd    },
   {"echo"  , fn_echo  },
   {"exit"  , fn_exit  },
   {"ls"    , fn_ls    },
   {"lsr"   , fn_lsr   },
   {"make"  , fn_make  },
   {"mkdir" , fn_mkdir },
   {"prompt", fn_prompt},
   {"pwd"   , fn_pwd   },
   {"rm"    , fn_rm    },
   {"rmr"   , fn_rmr   },
};

command_fn find_command_fn (const string& cmd) {
   // Note: value_type is pair<const key_type, mapped_type>
   // So: iterator->first is key_type (string)
   // So: iterator->second is mapped_type (command_fn)
   DEBUGF ('c', "[" << cmd << "]");
   const auto result = cmd_hash.find (cmd);
   if (result == cmd_hash.end()) {
      cerr << cmd << ": no such function" << endl;
      return NULL; 
   } else {
      return result->second;
   }
}

command_error::command_error (const string& what):
            runtime_error (what) {
}

inode_ptr checkpath(inode_state &state, wordvec path, bool fromroot) {
   inode_ptr checkedpath;
   int iter = 0;
   if (fromroot) {
      checkedpath = state.get_root();
   } else {
      checkedpath = state.current_dir();
   }
   for (iter = 0; iter < static_cast<int>(path.size()); iter++){
      try {
         checkedpath = checkedpath->get_child(path[iter]);
      }
      catch (...) {
         return nullptr;
      }
   }
   return checkedpath;
}

void rremove(inode_ptr node) {
   int iter = 2;
   inode_ptr parent = node->get_parent();
   if (node -> get_type() == file_type::DIRECTORY_TYPE) {
      wordvec children = node -> get_child_names();
      int children_size = static_cast<int>(children.size());
      for (iter = 2; iter < children_size; iter++) {
         inode_ptr child = node->get_child(children.at(iter));
         rremove(child);
      }
   }
   parent->remove(node->get_name());
}

void rprint(inode_ptr node, inode_state& state) {
   int iter = 2;
   wordvec path;
   inode_ptr pathname = node;
   while (pathname != state.get_root()) {
      path.push_back(pathname->get_name());
      pathname = pathname->get_parent();
   }
   for (iter = static_cast<int>(path.size()) - 1; iter >= 0; iter--) {
      cout << "/" << path.at(iter);
   }
   if (static_cast<int>(path.size()) == 0) {
      cout << "/";
   }
   cout << ":" << endl;
   cout << node << endl;
   wordvec children = node -> get_child_names();
   for (iter = 2; iter < static_cast<int>(children.size()); iter++) {
      inode_ptr child = node -> get_child(children.at(iter));
      if (child -> get_type() == file_type::DIRECTORY_TYPE) {
         rprint(child, state);
      }
   }
}

int exit_status_message() {
   int status = exec::get_status();
   cout << exec::execname() << ": exit(" << status << ")" << endl;
   return status;
}

void fn_cat (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   inode_ptr dir {nullptr};
   int iter = 1;
   // check to see if enough arguments
   if (words.size() < 2) cerr << "cat: too few operands" << endl;
   // find path
   for (iter = 1; iter < static_cast<int>(words.size()); iter++) {
      wordvec path = split(words[iter], "/");
      dir = checkpath(state, path, (words[1][0] == '/'));
      if (dir == nullptr) {
         cerr << words.at(0) << ": " << path.back(); 
         cerr << ": No such file or directory" << endl;
         break;
      }
      if (dir->get_type() == file_type::PLAIN_TYPE) {
         cout << dir;
      } else cerr << "cat: can't cat a directory!" << endl;
   }
   
}

void fn_cd (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   wordvec path;
   inode_ptr dir {nullptr};
   int iter = 1;
   // check to see if too many arguments
   if (static_cast<int>(words.size()) > 2) {
      cerr << "cd: too many operands" << endl;
   }
   // check to see if path was given
   if (static_cast<int>(words.size()) == 2) {
      //find path
      for (iter = 1; iter < static_cast<int>(words.size()); iter++) {
         path = split(words[iter], "/");
      }
      dir = checkpath(state, path, ((words.at(1)).at(0) == '/'));
      if (dir == nullptr) {
         cerr << words.at(0) << ": " << path.back(); 
         cerr << ": No such file or directory" << endl;
      } else {
         state.set_dir(dir);
      }
   }
   // if no path given
   else {
      state.set_dir(state.get_root());
   }
}

void fn_echo (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   cout << word_range (words.cbegin() + 1, words.cend()) << endl;
}


void fn_exit (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   int status = 0;
   if (words.size() > 1) {
      string exitnum = words.at(1);
      for (uint i = 0; i < exitnum.size(); i++) {
         if (exitnum.at(i) < '0' or exitnum.at(i) > '9') {
            status = 127;
            break;
         }
      }
      if (status != 127) status = stoi(exitnum);
   }
   exec::set_status(status);
   rremove(state.get_root());
   throw ysh_exit();
}

void fn_ls (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   inode_ptr dir;
   wordvec path;
   int iter = 1;
   // check to see if too few arguments
   if (static_cast<int>(words.size()) > 2) {
      cerr << "cd: too many operands" << endl;
   }
   // check whether path was given
   if (static_cast<int>(words.size()) == 2) {
      //find path
      for (iter = 1; iter < static_cast<int>(words.size()); iter++) {
         path = split(words[iter], "/");
         dir = checkpath(state, path, ((words.at(1)).at(0) == '/'));
         if (dir == nullptr) {
            cerr << words.at(0) << ": " << path.back(); 
            cerr << ": No such file or directory" << endl;
            break;
         }
         cout << words.at(1) << ":" << endl;
         cout << dir << endl;
      }
   } else {
      cout << state.current_dir()->get_name() << ":" << endl;
      cout << state.current_dir() << endl;
   }
}

void fn_lsr (inode_state& state, const wordvec& words){
   if (words.size() >= 2) {
      int iter = 1;
      int word_size = static_cast<int>(words.size());
      for (iter = 1; iter < word_size; iter++) {
         wordvec path = split(words.at(iter), "/");
         inode_ptr dir = checkpath(state,
                         path, (words.at(iter).at(0) == '/'));
         rprint(dir, state);
      }
   }
   else {
      rprint(state.current_dir(), state);
   }   
}

void fn_make (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if (static_cast<int>(words.size()) <= 1) {
      cerr << "make: too few operands" << endl;
   }
   inode_ptr dir, new_file;
   wordvec path, check_path, contents;
   int iter = 2;
   path = split(words.at(1), "/");
   check_path = path;
   check_path.erase(check_path.end());
   dir = checkpath(state, check_path, ((words.at(1)).at(0) == '/'));
   if (dir == nullptr) {
      cerr << words.at(0) << ": " << path.back(); 
      cerr << ": No such file or directory" << endl;
   } else {
      for (; iter < static_cast<int>(words.size()); ++iter) {
         contents.push_back(words[iter]);
      }
      dir->make_file(path.back(), contents);
   }
}

void fn_mkdir (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if (static_cast<int>(words.size()) <= 1) {
      cerr << "make: too few operands" << endl;
   }
   if (static_cast<int>(words.size()) > 2) {
      cerr << "make: too many operands" << endl;
   }
   inode_ptr dir, new_file;
   wordvec path, check_path, contents;
   path = split(words.at(1), "/");
   check_path = path;
   check_path.erase(check_path.end());
   dir = checkpath(state, check_path, ((words.at(1)).at(0) == '/'));
   if (dir == nullptr) {
      cerr << words.at(0) << ": " << path.back();
      cerr << ": No such file or directory" << endl;
   } else {
      dir->make_dir(path.back(), dir);
   }
}

void fn_prompt (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   int iter = 0;
   string prompt("");
   if (static_cast<int>(words.size() < 2)) {
      cerr << "cd: too few operands" << endl;
   }
   for (iter = 1; iter < static_cast<int>(words.size()); iter++) {
      prompt = prompt + words.at(iter) + " ";
   }
   state.set_prompt(prompt);
}

void fn_pwd (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   wordvec path;
   int iter = 0;
   inode_ptr currentdir = state.current_dir();
   while (currentdir != state.get_root()) {
      path.push_back(currentdir->get_name());
      currentdir = currentdir->get_parent();
   }
   for (iter = static_cast<int>(path.size()) - 1; iter >= 0; iter--) {
      cout << "/" << path.at(iter);
   }
   if (static_cast<int>(path.size()) == 0) {
      cout << "/" << endl;
   }
   else {
      cout << endl;
   }
}

void fn_rm (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if (static_cast<int>(words.size()) < 2) {
      cerr << "make: too few operands" << endl;
   }
   inode_ptr dir, new_file;
   wordvec path, check_path, contents;
   path = split(words.at(1), "/");
   check_path = path;
   check_path.erase(check_path.end());
   dir = checkpath(state, check_path, ((words.at(1)).at(0) == '/'));
   if (dir == nullptr) {
      cerr << words.at(0) << ": " << path.back();
      cerr << ": No such file or directory" << endl;
   } else {
      dir->remove(path.back());
   }
}

void fn_rmr (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if (static_cast<int>(words.size()) < 2){
      cerr << "rmr: too few operands" << endl;
   } else if (words.at(1) == "/"){
      cerr << "rmr: cannot remove root directory" << endl;
   } else {
      inode_ptr dir, new_file;
      wordvec path, check_path, contents;
      path = split(words.at(1), "/");
      check_path = path;
      dir = checkpath(state, check_path, ((words.at(1)).at(0) == '/'));
      if (dir == nullptr) {
         cerr << words.at(0) << ": " << path.back(); 
         cerr << ": No such file or directory" << endl;
      } else {
         rremove(dir);
      }
   }
}


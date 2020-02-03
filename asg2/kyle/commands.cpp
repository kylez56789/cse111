// $Id: commands.cpp,v 1.18 2019-10-08 13:55:31-07 - - $

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
      throw command_error (cmd + ": no such function");
   }
   return result->second;
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
   cout << "Passed initial conditions in checkpath\n";
   for (iter = 0; iter < static_cast<int>(path.size()); iter++){
      try {
         cout << "Just got in the for loop\n";
         checkedpath = checkedpath->get_child(path[iter]);
         cout << "Inside the for loop\n";
      }
      catch (...) {
         throw command_error ("path does not exist");
      }
      cout << "Outside try catch\n";
      checkedpath = checkedpath->get_child(path[iter]);
      cout << "Passed a thing\n";
   }
   cout << "Right before return\n";
   return checkedpath;
}

int exit_status_message() {
   int status = exec::status();
   cout << exec::execname() << ": exit(" << status << ")" << endl;
   return status;
}

void fn_cat (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   inode_ptr dir {nullptr};
   int iter = 1;
   // check to see if enough arguments
   if (words.size() < 2) throw command_error ("cat: too few operands");
   // find path
   for (iter = 1; iter < static_cast<int>(words.size()); iter++) {
      wordvec path = split(words[iter], "/");
      dir = checkpath(state, path, ((words.at(1)).at(0) == '/'));
      if (dir->get_type() == file_type::PLAIN_TYPE) {
         cout << dir << endl;
      } else throw command_error ("cat: can't cat a directory!");
   }
}

void fn_cd (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   wordvec path;
   inode_ptr dir {nullptr};
   int iter = 1;
   // check to see if too many arguments
   if (static_cast<int>(words.size()) > 2) throw command_error("cd: too many operands");
   // check to see if path was given
   if (static_cast<int>(words.size()) == 2) {
      //find path
      for (iter = 1; iter < static_cast<int>(words.size()); iter++) {
         path = split(words[iter], "/");
      }
      dir = checkpath(state, path, ((words.at(1)).at(0) == '/'));
      state.set_dir(dir);
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
   throw ysh_exit();
}

void fn_ls (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   inode_ptr dir;
   wordvec path;
   int iter = 1;
   // check to see if too few arguments
   if (static_cast<int>(words.size()) > 2) throw command_error("cd: too many operands");
   // check whether path was given
   if (static_cast<int>(words.size()) == 2) {
      //find path
      for (iter = 1; iter < static_cast<int>(words.size()); iter++) {
         path = split(words[iter], "/");
         dir = checkpath(state, path, ((words.at(1)).at(0) == '/'));
         cout << dir << endl;
      }
   } else {
      inode_ptr current_dir = state.current_dir();
      cout << current_dir << endl;
   }
}

void fn_lsr (inode_state& state, const wordvec& words){
   
}

void fn_make (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if (static_cast<int>(words.size()) <= 1) throw command_error("make: too few operands");
   inode_ptr dir, new_file;
   wordvec path, check_path, contents;
   int iter = 2;
   path = split(words.at(1), "/");
   check_path = path;
   check_path.erase(check_path.end());
   dir = checkpath(state, check_path, ((words.at(1)).at(0) == '/'));
   for (; iter < static_cast<int>(words.size()); ++iter) {
      contents.push_back(words[iter]);
   }
   new_file = dir->make_file(path.back(), contents);
}

void fn_mkdir (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if (static_cast<int>(words.size()) <= 1) throw command_error("make: too few operands");
   if (static_cast<int>(words.size()) > 2) throw command_error("make: too many operands");
   inode_ptr dir, new_file;
   wordvec path, check_path, contents;
   path = split(words.at(1), "/");
   check_path = path;
   check_path.erase(check_path.end());
   dir = checkpath(state, check_path, ((words.at(1)).at(0) == '/'));
   new_file = dir->make_dir(path.back(), dir);
}

void fn_prompt (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   int iter = 0;
   string prompt("");
   if (static_cast<int>(words.size() < 2)) throw command_error("cd: too few operands");
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
   for (iter = static_cast<int>(path.size()) - 1; iter >= 0; iter++) {
      cout << "/" << path.at(iter);
   }
   if (static_cast<int>(path.size()) == 0) {
      cout << "/" << endl;
   }
}

void fn_rm (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if (static_cast<int>(words.size()) < 2) throw command_error("make: too few operands");
   inode_ptr dir, new_file;
   wordvec path, check_path, contents;
   path = split(words.at(1), "/");
   check_path = path;
   check_path.erase(check_path.end());
   dir = checkpath(state, check_path, ((words.at(1)).at(0) == '/'));
   dir->remove(path.back());
}

void fn_rmr (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   inode_ptr dir, new_file;
   wordvec path, check_path, contents;
   path = split(words.at(1), "/");
   check_path = path;
   check_path.erase(check_path.end());
   dir = checkpath(state, check_path, ((words.at(1)).at(0) == '/'));
   dir->rremove(path.back());
}


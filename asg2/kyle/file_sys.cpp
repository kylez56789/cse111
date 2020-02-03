// $Id: file_sys.cpp,v 1.7 2019-07-09 14:05:44-07 - - $

#include <iostream>
#include <stdexcept>
#include <unordered_map>

using namespace std;

#include "debug.h"
#include "file_sys.h"

int inode::next_inode_nr {1};

struct file_type_hash {
   size_t operator() (file_type type) const {
      return static_cast<size_t> (type);
   }
};

ostream& operator<< (ostream& out, file_type type) {
   static unordered_map<file_type,string,file_type_hash> hash {
      {file_type::PLAIN_TYPE, "PLAIN_TYPE"},
      {file_type::DIRECTORY_TYPE, "DIRECTORY_TYPE"},
   };
   return out << hash[type];
}

// inode_state

inode_state::inode_state() {
   root = make_shared<inode>(file_type::DIRECTORY_TYPE);
   cwd = root;
   directory_ptr direc = dynamic_pointer_cast<directory>(root->contents);
   direc->set_dir(".",root);
   direc->set_dir("..",root);
   //cout << direc->get_dirents().size() << endl;
   //cout << *direc;
   DEBUGF ('i', "root = " << root << ", cwd = " << cwd
          << ", prompt = \"" << prompt() << "\"");
}

const string& inode_state::prompt() const { return prompt_; }

inode_ptr inode_state::current_dir() {
   return cwd;
}

inode_ptr inode_state::get_root() {
   return root;
}

void inode_state::set_prompt(string ppt) {
   prompt_ = ppt;
}

void inode_state::set_dir(inode_ptr dir) {
   cwd = dir;
}

ostream& operator<< (ostream& out, const inode_state& state) {
   out << "inode_state: root = " << state.root
       << ", cwd = " << state.cwd;
   return out;
}

inode::inode(file_type f_type): inode_nr (next_inode_nr++) {
   type = f_type;
   switch (type) {
      case file_type::PLAIN_TYPE:
           contents = make_shared<plain_file>();
           name = "Plain Name";
           type = file_type::PLAIN_TYPE;
           inode_nr = next_inode_nr;
           break;
      case file_type::DIRECTORY_TYPE:
           contents = make_shared<directory>();
           name = "Directory Name";
           type = file_type::DIRECTORY_TYPE;
           inode_nr = next_inode_nr;
           break;
   }
   DEBUGF ('i', "inode " << inode_nr << ", type = " << type);
}

inode::inode(file_type f_type, string new_name): inode_nr (next_inode_nr++) {
   type = f_type;
   switch (type) {
      case file_type::PLAIN_TYPE:
           contents = make_shared<plain_file>();
           name = new_name;
           type = file_type::PLAIN_TYPE;
           inode_nr = next_inode_nr;
           break;
      case file_type::DIRECTORY_TYPE:
           contents = make_shared<directory>();
           name = new_name;
           type = file_type::DIRECTORY_TYPE;
           inode_nr = next_inode_nr;
           break;
   }
   DEBUGF ('i', "inode " << inode_nr << ", type = " << type);
}

int inode::get_inode_nr() const {
   DEBUGF ('i', "inode = " << inode_nr);
   return inode_nr;
}

file_type inode::get_type() {
   return type;
}

string inode::get_name(){
   return name;
}

void inode_state::set_root(inode_ptr new_root){
   root = new_root;
}

void inode::set_parent(inode_ptr parent){
   dynamic_pointer_cast<directory>(contents)->set_dir("..", parent);
}

int inode::size() {
   return contents->size();
}

inode_ptr inode::get_parent(){
   return dynamic_pointer_cast<directory>(contents)->get_dir("..");
}

inode_ptr inode::get_child(string c_name){
   return dynamic_pointer_cast<directory>(contents)->get_dir(c_name);
}

wordvec inode::get_names() {
   return dynamic_pointer_cast<directory>(contents)->get_content_names();
}

inode_ptr inode::make_dir(string dir_name, inode_ptr& parent){
   //Makes new inode_ptr that points to a directory with 2 base inputs
   inode_ptr new_ptr = make_shared<inode>(file_type::DIRECTORY_TYPE);
   directory_ptr new_direc = dynamic_pointer_cast<directory>(new_ptr->contents);
   new_direc->set_dir(".",new_ptr);
   new_direc->set_dir("..",parent); 
   
   directory_ptr cwd_direc = dynamic_pointer_cast<directory>(this->contents);
   cwd_direc->set_dir(dir_name, new_ptr);    
   return new_ptr;
}

inode_ptr inode::make_file(string file_name, wordvec& words){
   inode_ptr new_ptr = make_shared<inode>(file_type::PLAIN_TYPE);
   plain_file_ptr pf_ptr = dynamic_pointer_cast<plain_file>(new_ptr->contents); 
   pf_ptr->writefile(words);

   directory_ptr direc = dynamic_pointer_cast<directory>(contents);
   direc->set_dir(file_name, new_ptr);
   return new_ptr;
}

void inode::remove(string to_be_removed) {
   if(this->type == file_type::DIRECTORY_TYPE) {
      directory_ptr dir_ptr = dynamic_pointer_cast<directory>(this->contents);
      dir_ptr->remove(to_be_removed);
   }
   else {
      throw file_error ("Error cannot remove from a file");
   }
}

void inode::rremove(string to_be_removed) {
   if(this->type == file_type::DIRECTORY_TYPE) {
      directory_ptr dir_ptr = dynamic_pointer_cast<directory>(this->contents);
      wordvec all_names = dir_ptr->get_content_names();
      for(int iter = 0; iter < all_names.size(); iter++) {
         dir_ptr->remove(all_names[iter]);
      }
      
   }
   else {
      throw file_error ("Error cannot remove from a file");
   }
}

ostream& operator<< (ostream& out, const inode_ptr& node) { 
   if (node->type == file_type::DIRECTORY_TYPE){
      cout << *(dynamic_pointer_cast<directory>(node->contents));
      return out;
   } else{
      cout << *(dynamic_pointer_cast<plain_file>(node->contents));
      return out;
   }
}

ostream& operator<< (ostream& out, const base_file_ptr& node) {
   cout << "<< BASE_FILE" << endl;
   return out;
}

file_error::file_error (const string& what):
            runtime_error (what) {
}

const wordvec& base_file::readfile() const {
   throw file_error ("is a " + error_file_type());
}

void base_file::writefile (const wordvec&) {
   throw file_error ("is a " + error_file_type());
}

void base_file::remove (const string&) {
   throw file_error ("is a " + error_file_type());
}

inode_ptr base_file::mkdir (const string&) {
   throw file_error ("is a " + error_file_type());
}

inode_ptr base_file::mkfile (const string&) {
   throw file_error ("is a " + error_file_type());
}

plain_file::plain_file() {
   data.push_back("");
}

plain_file::plain_file(string str) {
   data.push_back(str);
}

size_t plain_file::size() const {
   size_t size = 0;
   int iter = 0;
   for(iter = 0; iter < static_cast<int>(data.size()); iter++) {
      size += data[iter].length();
   }
   size += data.size();
   DEBUGF ('i', "size = " << size);
   return size;
}
const wordvec& plain_file::readfile() const {
   DEBUGF ('i', data);
   return data;
}

ostream& operator<< (ostream& out, const plain_file& plain) {
   for(int iter = 0; iter < static_cast<int>(plain.data.size());iter++) {
      cout << plain.data.at(iter) << " ";
   }
   cout << endl;
   return out;
}

void plain_file::writefile (const wordvec& words) {
   //Works based on ubigint testing
   data = words; 
   DEBUGF ('i', words);
}

size_t directory::size() const {
   size_t size = dirents.size();
   DEBUGF ('i', "size = " << size);
   return size;
}

// updates or inserts directory

directory::directory() {
   dirents.insert(pair<string, inode_ptr>(".", nullptr));
   dirents.insert(pair<string, inode_ptr>("..", nullptr));
}

directory::directory (inode_ptr parent, inode_ptr root) {
   dirents.insert(pair<string, inode_ptr>(".", root));
   dirents.insert(pair<string, inode_ptr>("..", parent));
}
void directory::set_dir (string name, inode_ptr dir) {
   map<string,inode_ptr>::iterator it = dirents.find(name);
   if (it != dirents.end()){
      dirents.erase(it);
   }
   cout << "Before insert" << endl;
   //dirents.insert(it, pair<string,inode_ptr>(name,dir));
   dirents.insert(pair<string,inode_ptr>(name,dir));
   cout << "After insert" << endl;
}

map<string,inode_ptr> directory::get_dirents() {
   return dirents;
}

inode_ptr directory::get_dir (string name) {
   return dirents.at(name);
}

void directory::remove (const string& filename) {
   map<string,inode_ptr>::iterator it = dirents.find(filename);
   if (it != dirents.end()){
      inode_ptr to_be_removed = dirents.at(filename);
      if(to_be_removed->get_type() == file_type::DIRECTORY_TYPE) {
         if(to_be_removed->size() > 2) {
            throw file_error("Directory " + filename + " not empty");
         }
         set_dir(".",nullptr);
         set_dir("..",nullptr);
      }
      dirents.erase(it);
   }
   else {
      throw file_error(filename + " not found");
   }  
}

inode_ptr directory::mkdir (const string& dirname) {
   DEBUGF ('i', dirname);
   return nullptr;
}

inode_ptr directory::mkfile (const string& filename) {
   DEBUGF ('i', filename);
   return nullptr;
}

wordvec directory::get_content_names() {
   wordvec names;
   for (map<string,inode_ptr>::iterator it = dirents.begin(); it != dirents.end(); it++) {
      names.push_back(it->first);
   }
   return names;
}

ostream& operator<< (ostream& out, directory& dir) {
   cout << "In operator << for directories" << endl;
   
   for(auto iterator = dir.get_dirents().cbegin(); iterator != dir.get_dirents().cend(); ++iterator) {
      cout << (*iterator).second->get_inode_nr() << "  " << (*iterator).second->size();
      cout << "  " << (*iterator).first << endl;
   }
   
   cout << "Now leaving operator << for directors" << endl;
   return out;

}


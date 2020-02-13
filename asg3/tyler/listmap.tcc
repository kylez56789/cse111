// $Id: listmap.tcc,v 1.15 2019-10-30 12:44:53-07 - - $

#include "listmap.h"
#include "debug.h"

//
/////////////////////////////////////////////////////////////////
// Operations on listmap.
/////////////////////////////////////////////////////////////////
//

//
// listmap::~listmap()
//
template <typename key_t, typename mapped_t, class less_t>
listmap<key_t,mapped_t,less_t>::~listmap() {
   DEBUGF ('l', reinterpret_cast<const void*> (this));
	while (anchor() != anchor()->next) {
		this->erase(this->begin());
	}
}

//
// iterator listmap::insert (const value_type&)
//
template <typename key_t, typename mapped_t, class less_t>
typename listmap<key_t,mapped_t,less_t>::iterator
listmap<key_t,mapped_t,less_t>::insert (const value_type& pair) {
   DEBUGF ('l', &pair << "->" << pair);
	//if (&anchor()->next == &anchor_){
   if (this->empty()){
		node* newNode = new node(anchor(), anchor(), pair);
		anchor()->next = newNode;
		anchor()->prev = newNode;
		return newNode;
	}
	iterator it = this->find(pair.first);
	if (this->end() != it) {
		it->second = pair.second;
		return it;
	} else {
		node* Node = anchor()->next;
		while(Node != anchor() and less(Node->value.first, pair.first)) {
			Node = Node->next;
		}
		node* newNode = new node(Node, Node->prev, pair);
		Node->prev->next = newNode;
		Node->prev = newNode;
		return newNode;
	}
}

//
// listmap::find(const key_type&)
//
template <typename key_t, typename mapped_t, class less_t>
typename listmap<key_t,mapped_t,less_t>::iterator
listmap<key_t,mapped_t,less_t>::find (const key_type& that) {
	node* Node;
	for (Node = anchor()->next; Node != anchor(); Node = Node->next) {
		if (Node->value.first == that) {
			return Node;
		}
	}	
   DEBUGF ('l', that);
	return Node;
}

//
// iterator listmap::erase (iterator position)
//
template <typename key_t, typename mapped_t, class less_t>
typename listmap<key_t,mapped_t,less_t>::iterator
listmap<key_t,mapped_t,less_t>::erase (iterator position) {
   DEBUGF ('l', &*position);
	node* Node = anchor()->next;
	for (; Node != anchor() and Node->value.first != position->first; Node = Node->next){}
	if (Node != anchor()) {
		Node->prev->next = Node->next;
		Node->next->prev = Node->prev;
		Node->next = nullptr;
		Node->prev = nullptr;
		delete Node;
		position.erase();
	}
	return iterator();
}



template <typename key_t, typename mapped_t, class less_t>
void listmap<key_t,mapped_t,less_t>::iterator::erase(){
	this->where = nullptr;
}

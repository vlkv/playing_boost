#pragma once
#include <boost/container/map.hpp>
using namespace boost::container;

class TreeItem {
	int _int;
	double _square_int;

public:
	TreeItem(int i) : _int(i) {
		_square_int = i * i;
	}
	int value() const { return _int; }
	double square_value() const { return _square_int; }
};

//typedef tree_assoc_options< tree_type<avl_tree> >::type AVLTree;
//typedef boost::container::map<int, TreeItem, std::allocator<std::pair<int, TreeItem>>, AVLTree> BinTree;
typedef boost::container::map<int, TreeItem> BinTree;


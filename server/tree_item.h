#pragma once
#include <boost/container/map.hpp>
#include <boost/serialization/access.hpp>
using namespace boost::container;

class TreeItem {
	int _int;
	double _square_int;

	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & _int;
		ar & _square_int;
	}
public:
	TreeItem(int i) : _int(i) {
		_square_int = i * i;
	}
	int value() const { return _int; }
	double square_value() const { return _square_int; }
	virtual ~TreeItem() {}
};

//typedef tree_assoc_options< tree_type<avl_tree> >::type AVLTree;
//typedef boost::container::map<int, TreeItem, std::allocator<std::pair<int, TreeItem>>, AVLTree> BinTree;
typedef boost::container::map<int, TreeItem> BinTree;


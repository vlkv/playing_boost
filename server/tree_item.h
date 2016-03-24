#pragma once
#include <boost/container/map.hpp>
#include <boost/serialization/access.hpp>
using namespace boost::container;

class TreeItem {
	double _square_num; // NOTE: we keep square of value for optimization reasons
	// NOTE 2: We do not keep the num itself in TreeItem, because it's the key in the tree

	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & _square_num;
	}
public:
	TreeItem(int i) : _square_num(i * i) {}
	double square_num() const { return _square_num; }
	virtual ~TreeItem() {}
};

typedef tree_assoc_options< tree_type<avl_tree> >::type AVLTree;
typedef boost::container::map<int, TreeItem, std::less<int>, new_allocator< std::pair< const int, TreeItem> >, AVLTree> BinTree;



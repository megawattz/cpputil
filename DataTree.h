/*
Copyright 2009 by Walt Howard
$Id: DataTree.h 2428 2012-08-14 15:33:13Z whoward $
*/

#pragma once

#include <Text.h>
#include <map>
#include <vector>
#include <Enhanced.h>

template <typename DATA> class DataTree
{
public:
    struct Node
    {
	typedef std::map<Text, Node*> BRANCHES;

	DATA Data;
	BRANCHES Branches;
    };

private:
    Node Root;

    typedef std::vector<Text> ELEMENTS;

public:
    Node* Get(const char* path, Node* start = 0)
    {
	// allow starting search from any node, default to Root node
	if (start == 0)
	    start = &Root;

	// get the elements of the . delimited path example: blah.blah.blah
	ELEMENTS elements = Enhanced<std::vector<Text> >(path, ".");
	struct Node* node = start;

	for(ELEMENTS::const_iterator i(elements.begin()); i != elements.end(); ++i)
	{
	    typename Node::BRANCHES::iterator b = node->Branches.find(*i);
	    if (b == node->Branches.end())
		return 0;

	    node = b->second;
	}

	return node;
    }


    Node* Set(const char* path, const DATA& data, Node* start = 0)
    {
	// allow starting search from any node, default to Root node
	if (start == 0)
	    start = &Root;

	// get the elements of the . delimited path example: blah.blah.blah
	ELEMENTS elements = Enhanced<std::vector<Text> >(path, ".");
	struct Node* node = start;

	for(ELEMENTS::const_iterator i(elements.begin()); i != elements.end(); ++i)
	{
	    typename Node::BRANCHES::iterator b = node->Branches.find(*i);
	    if (b == node->Branches.end())
	    {
		// insert a new node
		node = node->Branches[*i] = new Node;
	    }
	    else // or use the one already there.
		node = b->second;
	}

	node->Data = data; // set the data
	return node;
    }

    template <typename EVALUATOR> void Walk(EVALUATOR& functor, const Text& name = "", int level = 0, Node* node = 0)
    {
	// allow starting search from any node, default to Root node
	if (node == 0)
	    node = &Root;

	functor(name, level, node);
    }
};

#pragma once

#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <boost/variant.hpp>
#include <boost/any.hpp>

#include <vector>
#include <string>
#include <list>
#include <map>

namespace client{
//////////////////////////////////////////////////////////////////////////
// Typedefs
//////////////////////////////////////////////////////////////////////////
//data
typedef std::vector<std::string> StringVec;
typedef std::list<std::string> StringLst;
typedef std::list<int> IntLst;
typedef std::vector<int> IntVec;
//type

//////////////////////////////////////////////////////////////////////////
// enum for node type
//////////////////////////////////////////////////////////////////////////
enum NodeType{
  UNDEF,
	SCAN,
	INDEX_SCAN,
	HASH_SCAN,
	SELECT,
	JOIN,
	UNION,
	PRODUCT,
	PROJECT
};

//////////////////////////////////////////////////////////////////////////
// Query tree node
//////////////////////////////////////////////////////////////////////////

class QueryTreeNode{
public:
	typedef boost::variant<std::string, StringVec> Attribute;
	typedef boost::shared_ptr<QueryTreeNode> QueryTreeNodePtr;
	typedef std::map<int, QueryTreeNodePtr > Children;
	typedef std::map<std::string, boost::any > ExInfo;
	typedef boost::function<bool (int id, const QueryTreeNodePtr, const QueryTreeNodePtr)> NodeCallBack;

public:
	QueryTreeNode();

	NodeType getType() const;
	void setType(NodeType val);

	Attribute getAttr() const;
	void setAttr(const Attribute& val);

	bool hasChild();
	bool hasChild(int id);

	QueryTreeNodePtr getChild(int id);
	bool setChild(int id, QueryTreeNodePtr node, bool bFailOnExist = false);

	//get the extra info
	//example: int val = any_cast<int>(getExInfo("Number"));
	boost::any getExInfo(const std::string& name);
	//set the extra info
	//example: setExInfo("Number", 102);
	void setExInfo(const std::string& name, const boost::any& val);

	std::string getAttrStr();

	QueryTreeNodePtr clone();

private:
	friend void ForEachNode_i(int id, const QueryTreeNodePtr parent, const QueryTreeNodePtr node, const NodeCallBack& cb);

public:
	friend bool SwapNode(const QueryTreeNodePtr root, const IntVec& lv1, const IntVec& lv2);
	friend void PrintTree(const QueryTreeNodePtr root, int depth);
	friend bool AppendNode(const QueryTreeNodePtr root, QueryTreeNodePtr node, const IntVec& lv1);
	friend bool RemoveNode(const QueryTreeNodePtr root, const IntVec& lv1);
	friend bool GetNodePath(const QueryTreeNodePtr root, QueryTreeNodePtr node, IntLst& lv1, int id);

	friend QueryTreeNodePtr CloneHelper(QueryTreeNodePtr node);

public:
	Children children;//map (id,ptr)

private:
	NodeType ty;//UNDEF for error
	Attribute attr;
	ExInfo exInfo;
};
//
typedef QueryTreeNode::QueryTreeNodePtr QueryTreeNodePtr;
typedef std::vector<QueryTreeNodePtr> QueryTreeNodePtrs;
//Func Obj
typedef QueryTreeNode::NodeCallBack NodeCallBack;

void ForEachNode (const QueryTreeNodePtr root, const NodeCallBack& cb);

QueryTreeNodePtrs ParseQueryTree(const std::string& text);

bool SwapNode(const QueryTreeNodePtr root, const IntVec& lv1, const IntVec& lv2);

bool SwapNodeAll(const QueryTreeNodePtr root, const IntVec& lv1, const IntVec& lv2);

bool RemoveNode(const QueryTreeNodePtr root, const IntVec& lv1);

bool InsertNode(const QueryTreeNodePtr root, QueryTreeNodePtr node, const IntVec& lv1);

bool AppendNode(const QueryTreeNodePtr root, QueryTreeNodePtr node, const IntVec& lv1);

bool GetNodePath(const QueryTreeNodePtr root, QueryTreeNodePtr node, IntLst& lv1, int id = 0);

QueryTreeNodePtr GetNearestAncestor(const QueryTreeNodePtr root, QueryTreeNodePtr node1, QueryTreeNodePtr node2);

QueryTreeNodePtrs GetNodesByType(const QueryTreeNodePtr root, NodeType type);

void PrintTree(const QueryTreeNodePtr root, int depth = 0);

};

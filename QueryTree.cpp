#include "StdAfx.h"
#include "QueryTree.h"

#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/fusion/include/at_c.hpp>
#include <boost/bind.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/foreach.hpp>

#include "DbCatalog.h"
#include "ConditionTokenizer.h"

//////////////////////////////////////////////////////////////////////////
//Global Variables
//////////////////////////////////////////////////////////////////////////
client::QueryTreeNodePtr g_currRoot;
extern DbCatalog *g_dbCata;

namespace client{
namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace fu = boost::fusion;

//////////////////////////////////////////////////////////////////////////
// Typedef for nodes
//////////////////////////////////////////////////////////////////////////

typedef boost::fusion::vector2<NodeType,StringVec> NodeType1;
typedef boost::fusion::vector2<NodeType,std::string> NodeType2;
typedef NodeType NodeType3;
typedef boost::variant <NodeType1, NodeType2, NodeType3> NodeVar;
typedef boost::fusion::vector2< IntVec, NodeVar> SubNode;


//////////////////////////////////////////////////////////////////////////
// Node variant visitor
//////////////////////////////////////////////////////////////////////////

class node_visitor : public boost::static_visitor<QueryTreeNode>
{
public:

  QueryTreeNode operator()( const NodeType1& node ) const
	{
		QueryTreeNode qtn;
		qtn.setType (fu::at_c<0>(node));
		qtn.setAttr (fu::at_c<1>(node));
		return qtn;
	}

	QueryTreeNode operator()( const NodeType2& node ) const
	{
		QueryTreeNode qtn;

		qtn.setType (fu::at_c<0>(node));
		qtn.setAttr (fu::at_c<1>(node));

		if (qtn.getType() == SELECT /*|| qtn.getType() == PROJECT*/)
		{
			std::string text = boost::get<std::string>(fu::at_c<1>(node));
			ConditionTokenizer to(text);
			qtn.setExInfo("EXPLST", to);
		}

		return qtn;
	}

	QueryTreeNode operator()( const NodeType3& node ) const
	{
		QueryTreeNode qtn;
		qtn.setType(node);
		return qtn;
	}
};

//////////////////////////////////////////////////////////////////////////
//Tree build helper
//////////////////////////////////////////////////////////////////////////
bool FixAssocTableName(int /*id*/, const QueryTreeNodePtr parent, const QueryTreeNodePtr node, const StringVec& table_can)
{
	if (node->getType() == SELECT /*|| node->getType() == PROJECT*/)
	{
		ConditionTokenizer ct = boost::any_cast<ConditionTokenizer>(node->getExInfo("EXPLST"));

		Conds conds = ct.getCons();

		BOOST_FOREACH (Condition& con, conds)
		{
			if(!con.isNeedFixed())
				continue;

			if (con.ltable_name.empty())//left
			{
				StringLst tableHasAttr = g_dbCata->GetTables(con.lfield_name);

				std::string table_name;

				BOOST_FOREACH (const std::string& val1, tableHasAttr)
					BOOST_FOREACH(const std::string& val2, table_can)
				{
					if ( val1 == val2)
					{
						table_name = val1;
						break;
					}
				}

				assert(!table_name.empty());

				ct.RemoveCon(con);

				con.ltable_name = table_name;

				ct.AppendCon(con);

				node->setExInfo("EXPLST", ct);
			}

			if (con.rtable_name.empty() && con.rtext.empty())
			{
				StringLst tableHasAttr = g_dbCata->GetTables(con.rfield_name);

				std::string table_name;

				BOOST_FOREACH (const std::string& val1, tableHasAttr)
					BOOST_FOREACH(const std::string& val2, table_can)
				{
					if ( val1 == val2)
					{
						table_name = val1;
						break;
					}
				}

				assert(!table_name.empty());

				ct.RemoveCon(con);

				con.rtable_name = table_name;

				ct.AppendCon(con);

				node->setExInfo("EXPLST", ct);
			}
		}
	}

	return true;
}
//////////////////////////////////////////////////////////////////////////

class TreeBuilder{
public:

	QueryTreeNodePtrs getTrees()
	{
		//fill in missing table names
		BOOST_FOREACH (QueryTreeNodePtr curRoot, rlz)
		{
			StringVec table_can;
			QueryTreeNodePtrs qps = GetNodesByType(curRoot, SCAN);
			BOOST_FOREACH (QueryTreeNodePtr node, qps){
				table_can.push_back(boost::get<std::string>(node->getAttr()));
			}

			NodeCallBack cb = boost::BOOST_BIND(FixAssocTableName, _1, _2, _3, boost::ref(table_can));
			ForEachNode(curRoot, cb);
		}

		//return fixed trees
		return rlz;
	}

	void onRoot( NodeVar& val )
	{
		currentRoot = QueryTreeNodePtr (
			new QueryTreeNode(boost::apply_visitor( node_visitor(), val ) ));
		g_currRoot = currentRoot;

		rlz.push_back(currentRoot);
	}

	void onNode( SubNode& val )
	{
		IntVec lvl = fu::at_c<0>(val);

		int depth = 0;

		QueryTreeNodePtr parent = currentRoot;
		while (++depth != lvl.size())
		{
			int childId = lvl.at(depth - 1);//array starts at 0
			parent = parent->getChild(childId);
		}

		int nodeId = lvl.at(lvl.size() - 1);//last

		QueryTreeNode node = boost::apply_visitor( node_visitor(), fu::at_c<1>(val) );
		parent->setChild(nodeId, QueryTreeNodePtr (new QueryTreeNode(node)));
	}

private:
	QueryTreeNodePtrs rlz;
	QueryTreeNodePtr currentRoot;
};

//////////////////////////////////////////////////////////////////////////
// Symbol table for keyword
//////////////////////////////////////////////////////////////////////////
struct relation_keywords_ : qi::symbols<char, NodeType>
{
	relation_keywords_()
	{
		add
			("SCAN", SCAN)
			("INDEX_SCAN", INDEX_SCAN)
			("HASH_SCAN", HASH_SCAN);
	}

} relation_keywords;

struct condition_keywords_ : qi::symbols<char, NodeType>
{
	condition_keywords_()
	{
		add
			("SELECT", SELECT)
			("JOIN", JOIN);
	}

} condition_keywords;

struct attr_keywords_ : qi::symbols<char, NodeType>
{
	attr_keywords_()
	{
		add
			("PROJECT"   , PROJECT);
	}

} attr_keywords;

struct null_keywords_ : qi::symbols<char, NodeType>
{
	null_keywords_()
	{
		add
			("UNION"   , UNION)
			("PRODUCT"   , PRODUCT);
	}

} null_keywords;

//////////////////////////////////////////////////////////////////////////
// Covert raw text to Tree
//////////////////////////////////////////////////////////////////////////
QueryTreeNodePtrs ParseQueryTree(const std::string& text){
	using qi::int_;
	using qi::parse;
	using qi::lit;

	using ascii::space;

	BOOST_AUTO (first, text.begin());
	BOOST_AUTO (last, text.end());

	BOOST_AUTO(levelsRule, int_ % ',' );

	qi::rule<std::string::const_iterator, std::string(), ascii::space_type>
		stringRule = +(qi::alnum | '_' | '.');

	qi::rule<std::string::const_iterator, std::string()>
		conExpRule = +(qi::alnum | '_' | '.' | '\'' | '>' | '=' | '<' | ' ');

	BOOST_AUTO(attrLstRule, lit("([") >> stringRule % ',' >> lit("])") );
	BOOST_AUTO(relationRule, lit("(") >> stringRule >> lit(")"));
	BOOST_AUTO(conditionRule, lit("(") >> conExpRule >> lit(")"));

	BOOST_AUTO(nodeRule, ( 
		(attr_keywords >> attrLstRule) 
		| (relation_keywords >> relationRule ) 
		| (condition_keywords>> conditionRule) 
		| (null_keywords) 
		) );

	BOOST_AUTO(begRule, nodeRule );
	BOOST_AUTO(expRule, levelsRule >> nodeRule );

	TreeBuilder tb;

	BOOST_AUTO (treeRule, 
		begRule[boost::bind(&TreeBuilder::onRoot, &tb, _1)] >> 
		* expRule[boost::bind(&TreeBuilder::onNode, &tb, _1)]);

	BOOST_AUTO(start, *treeRule);

	bool ret = true;
	while (first != last && ret == true)
	{
		ret = qi::phrase_parse(first, last, start, space);
	}

	return tb.getTrees();
}

//////////////////////////////////////////////////////////////////////////
// Swap node functions
//////////////////////////////////////////////////////////////////////////

struct NodeInfo {
	NodeInfo ():id (-1) {}
	bool isEmpty(){return id == -1;}
	//
	int id;//id for node
	QueryTreeNodePtr node;
	QueryTreeNodePtr parent;
};

NodeInfo getNode (const QueryTreeNodePtr root, const IntVec& levels)
{
	NodeInfo ret;
	if (!root)
		return ret;

	if (levels.size() == 0)
	{
		ret.id = 0;//0 stands for the root
		ret.node = root;
		return ret;
	}

	QueryTreeNodePtr tmpPar = root;
	for (int depth = 0; depth != levels.size() - 1; ++depth){
		if (!tmpPar->hasChild(levels.at(depth)))
			return ret;
		else
			tmpPar = tmpPar->getChild(levels.at(depth));
	}

	int id = levels[levels.size() - 1];//last

	if (tmpPar->hasChild(id)){
		ret.id = id;
		ret.parent = tmpPar;
		ret.node = tmpPar->getChild(ret.id);
	}

	return ret;
}

bool SwapNode(const QueryTreeNodePtr root, const IntVec& lv1, const IntVec& lv2)
{
	NodeInfo node1 = getNode(root, lv1);
	NodeInfo node2 = getNode(root, lv2);

	if (node1.id == -1 || node2.id == -1)
		return false;

	std::swap(node1.node->children, node2.node->children);
	node1.parent->setChild(node1.id, node2.node);
	node2.parent->setChild(node2.id, node1.node);

	return true;
}

bool SwapNodeAll( const QueryTreeNodePtr root, const IntVec& lv1, const IntVec& lv2 )
{
	NodeInfo node1 = getNode(root, lv1);
	NodeInfo node2 = getNode(root, lv2);

	if (node1.id == -1 || node2.id == -1)
		return false;

	node1.parent->setChild(node1.id, node2.node);
	node2.parent->setChild(node2.id, node1.node);

	return true;
}
//////////////////////////////////////////////////////////////////////////
// Print Tree Helper
//////////////////////////////////////////////////////////////////////////
const char* NodeType2Str(NodeType ty)
{
	switch (ty)
	{
	case UNDEF:
		return "UNDEF";

	case SCAN:
		return "SCAN";

	case  INDEX_SCAN:
		return "INDEX_SCAN";

	case HASH_SCAN:
		return "HASH_SCAN";
		
	case SELECT:
		return "SELECT";

	case JOIN:
		return "JOIN";

	case UNION:
		return "UNION";

	case PRODUCT:
		return "PRODUCT";

	case PROJECT:
		return "PROJECT";

	default :
		assert(0);
	}
}

std::string QueryTreeNode::getAttrStr()
{
	if (getType() == SELECT || getType() == JOIN)
	{
		std::string ret;
		ConditionTokenizer ct = boost::any_cast<ConditionTokenizer>(getExInfo("EXPLST"));
		ret = ct.getStr();
		return ret;
	}

	std::string* str = boost::get<std::string>( &attr );
	if (str)
		return *str;

	StringVec* strV = boost::get<StringVec>( &attr );
	if (strV)
			return boost::algorithm::join(*strV, ",");

	return std::string();
}

void PrintTree( const QueryTreeNodePtr root , int depth){//mid left->right
	using namespace std;//Cost int Algorithm std::string

	int* cost;
	std::string* algo;

	boost::any costAny = root->getExInfo("Cost");
	boost::any algoAny = root->getExInfo("Algorithm");

	cost = boost::any_cast<int>(&costAny);
	algo = boost::any_cast<std::string>(&algoAny);

	cout<< ' '<< NodeType2Str(root->getType())<< ' '<< root->getAttrStr();

	if (cost)
		cout<<" Cost: "<<*cost;

	if (algo)
		cout<<" Algo: "<<*algo;

	cout<<endl;

	if (!root->hasChild())
		return;

	BOOST_FOREACH (QueryTreeNode::Children::value_type val,
		root->children){
			int dep = depth + 1;

			while (dep-- != 0)
				cout<<' ';

			cout<<val.first;
			PrintTree(val.second, depth + 1);
	}
}

//////////////////////////////////////////////////////////////////////////
// Tree Nodes
//////////////////////////////////////////////////////////////////////////

QueryTreeNode::QueryTreeNode() :ty(UNDEF){}

NodeType QueryTreeNode::getType() const
{
	return ty;
}

void QueryTreeNode::setType( NodeType val )
{
	ty = val;
}

QueryTreeNode::Attribute QueryTreeNode::getAttr() const
{
	return attr;
}

void QueryTreeNode::setAttr( const Attribute& val )
{
	attr = val;
}

bool QueryTreeNode::hasChild( int id )
{
	return children.find(id) != children.end();
}

bool QueryTreeNode::hasChild()
{
	return children.size() != 0;
}

QueryTreeNode::QueryTreeNodePtr QueryTreeNode::getChild( int id )
{
	if (hasChild(id))
		return children[id];
	else
		return QueryTreeNodePtr();
}

bool QueryTreeNode::setChild( int id, QueryTreeNodePtr node, bool bFailOnExist)
{
	if (bFailOnExist)
		if (hasChild(id))
			return false;

	children[id] = node;
	return true;
}

boost::any QueryTreeNode::getExInfo( const std::string& name )
{
	if (exInfo.find(name) == exInfo.end())
		return boost::any();

	return exInfo[name];
}

void QueryTreeNode::setExInfo( const std::string& name, const boost::any& val )
{
	exInfo[name] = val;
}

//////////////////////////////////////////////////////////////////////////

QueryTreeNodePtr CloneHelper(QueryTreeNodePtr node)
{
	QueryTreeNodePtr newNode = QueryTreeNodePtr(new QueryTreeNode(*node));

	BOOST_FOREACH (QueryTreeNode::Children::value_type val,
		node->children){
			newNode->children[val.first] = CloneHelper(val.second);
	}

	return newNode;
}

QueryTreeNode::QueryTreeNodePtr QueryTreeNode::clone()
{
	return CloneHelper(QueryTreeNodePtr(new QueryTreeNode(*this)));
}
//////////////////////////////////////////////////////////////////////////
// Insert/Append node functions
//////////////////////////////////////////////////////////////////////////
bool InsertNode( const QueryTreeNodePtr root, QueryTreeNodePtr node, const IntVec& lv1 ){
	NodeInfo curNode = getNode(root, lv1);
	if (curNode.isEmpty())
		return false;

	assert (curNode.parent->getChild(curNode.id) == curNode.node);

	curNode.parent->setChild(curNode.id, node);
	node->setChild(1, curNode.node);//insert node at the beginning

	return true;
}

bool AppendNode(const QueryTreeNodePtr root, QueryTreeNodePtr node, const IntVec& lv1){
	NodeInfo curNode = getNode(root, lv1);
	if (curNode.isEmpty())
		return false;

	int index = curNode.node->children.size();

	assert (!curNode.node->getChild(index + 1));
	curNode.node->setChild(index + 1, node);

	return true;
}

bool RemoveNode( const QueryTreeNodePtr root, const IntVec& lv1 )
{
	//temp disabled because the index of other node may changed
	//may change map to vector?
	NodeInfo curNode = getNode(root, lv1);
	if (curNode.isEmpty())
		return false;

	assert (curNode.node -> children.size() <= 1);

	if (curNode.node -> children.size() == 1)
		curNode.parent -> children[curNode.id] = curNode.node -> children[1];

	return true;
}

bool GetNodePath( const QueryTreeNodePtr root, QueryTreeNodePtr node, IntLst& lv1, int id )
{
	if (root == node)
	{
		if (id != 0)
			lv1.push_front(id);
		return true;
	}
	BOOST_FOREACH (QueryTreeNode::Children::value_type val,
		root->children){
			bool ret = GetNodePath(val.second, node, lv1, val.first);
			if (ret == true || val.second == node)
			{
				if (id != 0)
					lv1.push_front(id);
				return true;
			}
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////

QueryTreeNodePtr GetNearestAncestor( 
	const QueryTreeNodePtr root, 
	QueryTreeNodePtr node1, 
	QueryTreeNodePtr node2 )
{
	QueryTreeNodePtr ret;

	IntLst lv1, lv2;

	bool ret1 = GetNodePath(root, node1, lv1);
	bool ret2 = GetNodePath(root, node2, lv2);

	if (ret1 == false || ret2 == false)
		return ret;

	IntVec lvl;

	BOOST_AUTO (i1, lv1.begin());
	BOOST_AUTO (i2, lv2.begin());

	for (;;)
	{
		if (i1 == lv1.end() || i2 == lv2.end())
			break;

		if (*i1 == *i2)
			lvl.push_back(*i1);
		else
			break;

		++i1;
		++i2;
	}

	NodeInfo ni = getNode(root, lvl);

	if (ni.isEmpty())
		return ret;
	else
		return ni.node;
}

void ForEachNode_i(int id, const QueryTreeNodePtr parent, const QueryTreeNodePtr node, const NodeCallBack& cb)
{
	bool ret = cb(id, parent, node);
	if (ret == false)
		return;

	BOOST_FOREACH (QueryTreeNode::Children::value_type val,
		node->children){
			ForEachNode_i(val.first, node, val.second, cb);
	}
}

void ForEachNode( const QueryTreeNodePtr root, const NodeCallBack& cb )
{
	ForEachNode_i(0, QueryTreeNodePtr(), root, cb);
}

//////////////////////////////////////////////////////////////////////////
bool NodeByType(int /*id*/, const QueryTreeNodePtr parent, const QueryTreeNodePtr node, QueryTreeNodePtrs& outVec, NodeType type)
{
	if (node->getType() == type)
		outVec.push_back(node);

	return true;
}

QueryTreeNodePtrs GetNodesByType( const QueryTreeNodePtr root, NodeType type )
{
	QueryTreeNodePtrs ret;

	NodeCallBack cb = boost::BOOST_BIND(NodeByType, _1, _2, _3, boost::ref(ret), type);
	ForEachNode(root, cb);

	return ret;
}
};

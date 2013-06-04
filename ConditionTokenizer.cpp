#include "StdAfx.h"

#include <boost/algorithm/string.hpp> 
#include <boost/foreach.hpp>
#include <boost/bind.hpp>

#include "DbCatalog.h"
#include "QueryTree.h"
#include "ConditionTokenizer.h"

using namespace client;

//////////////////////////////////////////////////////////////////////////
//Global Variables
//////////////////////////////////////////////////////////////////////////
extern DbCatalog *g_dbCata;
extern QueryTreeNodePtr g_currRoot;

//! Maintains a collection of substrings that are
//! delimited by a string of one or more characters
class Splitter {
  //! Contains the split tokens
	std::vector<std::string> _tokens;
public:
	//! Subscript type for use with operator[]
	typedef std::vector<std::string>::size_type size_type;
public:
	//! Create and initialize a new Splitter
	//!
	//! \param[in] src The string to split
	//! \param[in] delim The delimiter to split the string around
	Splitter ( const std::string& src, const std::string& delim )
	{
		reset ( src, delim );
	}

	//! Retrieve a split token at the specified index
	//!
	//! \param[in] i The index to search for a token
	//! \return The token at the specified index
	//! \throw std::out_of_range If the index is invalid
	std::string& operator[] ( size_type i )
	{
		return _tokens.at ( i );
	}

	//! Retrieve the number of split tokens
	//!
	//! \return The number of split tokens
	size_type size() const
	{
		return _tokens.size();
	}

	//! Re-initialize with a new source and delimiter
	//!
	//! \param[in] src The string to split
	//! \param[in] delim The delimiter to split the string around
	void reset ( const std::string& src, const std::string& delim )
	{
		std::vector<std::string> tokens;
		std::string::size_type start = 0;
		std::string::size_type end;

		for ( ; ; ) {
			end = src.find ( delim, start );
			tokens.push_back ( src.substr ( start, end - start ) );

			// We just copied the last token
			if ( end == std::string::npos )
				break;

			// Exclude the delimiter in the next search
			start = end + delim.size();
		}

		_tokens.swap ( tokens );
	}
};


ConditionTokenizer::ConditionTokenizer( const std::string& text )
{
	std::string::size_type end;

	end = text.find (" AND ", 0 );
	// If AND
	if ( end != std::string::npos )
	{
		Splitter Split ( text, " AND " );
		for ( Splitter::size_type i = 0; i < Split.size(); i++ )    
			conds.push_back (Condition(Split[i]) );
		ty = AND;
		return;
	}

	end = text.find (" OR ", 0 );
	// If OR
	if ( end != std::string::npos )
	{
		Splitter Split ( text, " OR " );
		for ( Splitter::size_type i = 0; i < Split.size(); i++ )    
			conds.push_back (Condition(Split[i]) );
		ty = OR;
		return;
	}

	end = text.find ("NOT ", 0 );
	// If NOT
	if ( end != std::string::npos )
	{
		Splitter Split ( text, "NOT " );
		for ( Splitter::size_type i = 0; i < Split.size(); i++ )    
			conds.push_back (Condition(Split[i]) );
		ty = NOT;
		return;
	}

	ty = ALONE;
	conds.push_back (Condition(text) );

}

ConditionTokenizer::ConditionTokenizer()
{

}
ConditionTokenizer::~ConditionTokenizer(){ }

std::string ConditionTokenizer::getStr()
{
	std::string ret;
	BOOST_FOREACH (const Condition& val, conds){
		ret += val.toString() + " ";
	}
	return ret;
}

void ConditionTokenizer::RemoveCon( Condition con )
{
	Conds::iterator iter = std::find(conds.begin(), conds.end(), con);
	if (iter == conds.end())
		return;

	conds.erase(iter);
}

void ConditionTokenizer::AppendCon( Condition con )
{
	conds.push_back(con);
}
//////////////////////////////////////////////////////////////////////////
void ConditionHelper(const std::string src, std::string& table_name, std::string& field_name, std::string& text_val){
	if (src.find_first_of('\'') != std::string::npos)//like 'Mike'
	{
		text_val.assign(src.begin() + 1, src.begin() + src.size() - 1);
	}
	else if (isdigit(src[0]) && atoi (src.c_str()) != 0)//like 10
	{
		text_val = src;
	}
	else if (src.find_first_of('.') != std::string::npos)//like A.B
	{
		StringVec names; 
		boost::split(names, src, boost::is_any_of(".")); 
		table_name = names[0];
		field_name = names[1];
	}
	else //like B //missing table name //now move to after finishing building the tree
	{
		field_name = src;
	//	StringLst tableHasAttr = g_dbCata->GetTables(field_name);
	//	if (tableHasAttr.size() == 1)
	//		table_name = *tableHasAttr.begin();
	//	else
	//	{
	//		NodeCallBack cb = boost::BOOST_BIND(GetAssocTableName, _1, _2, _3, boost::ref(table_name), boost::ref(tableHasAttr));
	//		ForEachNode(g_currRoot, cb);
	//	}
	}
}
//////////////////////////////////////////////////////////////////////////
Condition::Condition( const std::string& text ) :is_equ(false)
{
	std::vector<std::string> fields; 
	boost::split(fields, text, boost::is_any_of("<>=")); 

	is_equ = (text.find ( "=", 0 ) != std::string::npos);

	//
	ConditionHelper(fields[0], ltable_name, lfield_name, std::string());

	ConditionHelper(fields[1], rtable_name, rfield_name, rtext);

	//dbg_str = text;
}

bool Condition::isSameTable( const Condition& other ) const
{
	return ltable_name == other.ltable_name &&
		rtable_name == other.rtable_name;
}

bool Condition::operator==( const Condition& other )
{
	return ltable_name == other.ltable_name &&
		rtable_name == other.rtable_name && 
		lfield_name == other.lfield_name &&
		rfield_name == other.rfield_name &&
		rtext == other.rtext;
}

std::string Condition::toString() const
{
	return ltable_name + ' ' + lfield_name + " with " + rtable_name + ' ' + rfield_name + ' ' + rtext;
}

bool Condition::isNeedFixed() const
{
	if (ltable_name.empty())
		return true;

	if (rtable_name.empty() && rtext.empty())
		return true;

	return false;
}

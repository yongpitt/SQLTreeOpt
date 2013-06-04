#pragma once

#include <vector>
#include <string>

class Condition{
public:
  Condition(const std::string& text);

	bool isSameTable(const Condition& other) const;

	bool isNeedFixed () const;

	bool operator==(const Condition& other);

	//for dbg out
	std::string	toString() const;

	std::string ltable_name;
	std::string lfield_name;

	std::string rtable_name;
	std::string rfield_name;
	std::string rtext;
	//std::string dbg_str;
	
	bool is_equ;
};

typedef std::vector<Condition> Conds;

class ConditionTokenizer
{
public:
	enum Type{
		AND,
		OR,
		NOT,
		ALONE
	};

public:
	ConditionTokenizer();
	ConditionTokenizer(const std::string& text);
	~ConditionTokenizer();

	Type getType(){return ty;}
	void setType(Type val){ty = val;}

	Conds getCons(){return conds;}

	void RemoveCon(Condition con);
	void AppendCon(Condition con);

	//void setCons(const Conds& val);

	std::string getStr();

private:
	Type ty;
	Conds conds;
};

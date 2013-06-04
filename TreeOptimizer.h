#pragma once

#include "QueryTree.h"

namespace client{

class TreeOptimizer
{
public:
  TreeOptimizer(void);
	~TreeOptimizer(void);

	QueryTreeNodePtr optimize(QueryTreeNodePtr node);

private:
	void step1();//break up selection
	void step2();//push selection down
	void step3();
	void step4();
	void step5();
	void step6();

private:
	QueryTreeNodePtr root;
};
};

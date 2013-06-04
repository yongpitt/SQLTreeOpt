#include "StdAfx.h"
#include "QueryTree.h"
#include "Opt24.h"
#include "dbCatalog.h"
#include "ConditionTokenizer.h"
#include <boost\any.hpp>
//#include <math.h>

using namespace client;

int CostCalcTree(QueryTreeNodePtr SubTreeRoot,  DbCatalog* dbCatalog)  //should be recursive
{
  std::string Linear = "LINEAR_SEARCH";
	std::string Binary = "BINARY_SEARCH";
	std::string Btree = "BTREE_SEARCH";
	std::string Extensible = "EXTENSIBLE_HASH";
	std::string Lhash = "LINEAR_HASH";
	std::string Njoin = "NESTED_LOOP_JOIN";
	std::string Sjoin = "SINGLE_LOOP_JOIN";
	std::string Pjoin = "PARTITION_HASH_JOIN";
	std::string Msort = "MERGE_SORT";
	std::string delimiters = " ()\t\n.'=<>";

	int blocks=0;
    int costs=0;

	NodeType nt = SubTreeRoot->getType();
    
    switch(nt)
	{	 
	case	UNDEF:
		{
		//example: setExInfo("Number", 102);
		    SubTreeRoot->setExInfo("Cost", 0); 
			if (SubTreeRoot->hasChild(1))
		    {
				CostCalcTree(SubTreeRoot->getChild(1), dbCatalog);
		    } 
			if (SubTreeRoot->hasChild(2))
		    {
				CostCalcTree(SubTreeRoot->getChild(2), dbCatalog);
		    }
		    //QueryTreeNodePtr RightChild;
			break;
		}
	case	SCAN:
		{
	            //return number of blocks for JOIN
		        std::string TableName = boost::get<std::string>(SubTreeRoot->getAttr());
				
				//determine index(change the SCAN type if necessary), cost for this
				//SCAN node based on condition in the SELECT, the index availabe for 
				//this table. In addition to cost, the number of blocks also need to
				//be calculated.
				blocks = dbCatalog->GetCardi(TableName)/dbCatalog->GetBfr(TableName);
				
		
		    break;
		}
	case	INDEX_SCAN:
		{
		    break;
		}
	case	HASH_SCAN:
		{
		    break;
		}
	case	SELECT:
		{
			//SELECT node should have only one child
            assert(SubTreeRoot->hasChild(1));
			assert(!SubTreeRoot->hasChild(2));
			
			//std::string Condition = boost::get<std::string>(SubTreeRoot->getAttr());

			//get all relevant information about the select
			std::string TableInCond1, TableInCond2;
			std::string AttrInCond1, AttrInCond2;
			bool Cond1IsEq, Cond2IsEq;
			
			ConditionTokenizer CondOp = boost::any_cast<ConditionTokenizer>(SubTreeRoot->getExInfo("EXPLST"));
			ConditionTokenizer::Type CondType = CondOp.getType();

			Conds conds = CondOp.getCons();
			Cond1IsEq = conds[0].is_equ;
			TableInCond1 = conds[0].ltable_name;
			AttrInCond1 = conds[0].lfield_name;
			if(CondType != ConditionTokenizer::ALONE)
			{
			   Cond2IsEq = conds[1].is_equ;
			   TableInCond2 = conds[1].ltable_name;
			   AttrInCond2 = conds[1].lfield_name;
			}


			//int CondType = 1;  //0 equal;  1 not equal

			QueryTreeNodePtr LeftChild = SubTreeRoot->getChild(1);

			if(LeftChild->getType() == SCAN)
			{
		        std::string TableName = boost::get<std::string>(LeftChild->getAttr());
				
				//determine index(change the SCAN type if necessary), cost for this
				//SCAN node based on condition in the SELECT, the index availabe for 
				//this table. In addition to cost, the number of blocks also need to
				//be calculated.
				int test= dbCatalog->GetCardi(TableName);
				if(dbCatalog->GetCardi(TableName)/dbCatalog->GetBfr(TableName) <= 10)
				{
					if(dbCatalog->IsPk(TableName, AttrInCond1) && Cond1IsEq)
					{
					    costs =  dbCatalog->GetCardi(TableName)/(dbCatalog->GetBfr(TableName)*2);
						blocks = 1;
					}
					else
					{
					    costs =  dbCatalog->GetCardi(TableName)/dbCatalog->GetBfr(TableName);
						if(dbCatalog->GetSel(TableName,AttrInCond1)>0 && dbCatalog->GetCardi(TableName)>0 && Cond1IsEq)
						{
						   blocks = (dbCatalog->GetSel(TableName,AttrInCond1)*dbCatalog->GetCardi(TableName))/
							   dbCatalog->GetBfr(TableName);
						}
						else
						   blocks = (int) costs/2;
					}

					LeftChild->setExInfo("Cost",costs);
				    LeftChild->setExInfo("Algorithm",Linear);

				}
				else
				switch(dbCatalog->GetIdx(TableName,AttrInCond1))
				{
				case NONE_T:
					{
					if(dbCatalog->IsPk(TableName, AttrInCond1) && Cond1IsEq)
					{
					    costs =  dbCatalog->GetCardi(TableName)/(dbCatalog->GetBfr(TableName)*2);
						blocks = 1;
					}
					else
					{
						costs =  dbCatalog->GetCardi(TableName)/dbCatalog->GetBfr(TableName);
						if(dbCatalog->GetSel(TableName,AttrInCond1)>0 && dbCatalog->GetCardi(TableName)>0 && Cond1IsEq)
						{
						   blocks = (dbCatalog->GetSel(TableName,AttrInCond1)*dbCatalog->GetCardi(TableName)
							   /dbCatalog->GetBfr(TableName));
						}
						else
						   blocks = (int) costs/2;
					}

					LeftChild->setExInfo("Cost",costs);
				    LeftChild->setExInfo("Algorithm",Linear);

					break;
					}
				case BTREE_T:
					{
					int btreeBft = dbCatalog->GetIdxBfr(TableName,AttrInCond1);
					int level = (int) log((float) dbCatalog->GetCardi(TableName)/dbCatalog->GetBfr(TableName));
					
					if(dbCatalog->IsPk(TableName, AttrInCond1) && Cond1IsEq)
					{
					    costs =  level+1;
						blocks = 1;
					}
					else if(!(dbCatalog->IsPk(TableName, AttrInCond1)) && Cond1IsEq)
					{
                        if(dbCatalog->GetSel(TableName,AttrInCond1)>0 && dbCatalog->GetCardi(TableName)>0)
						{
						   blocks = (dbCatalog->GetSel(TableName,AttrInCond1)*dbCatalog->GetCardi(TableName)
							   /dbCatalog->GetBfr(TableName));
						   costs = blocks + level;
						}
						else
						{
							blocks =  dbCatalog->GetCardi(TableName)/(dbCatalog->GetBfr(TableName)*2);
							costs = blocks + level;
						}
				      
					}
					else if(!Cond1IsEq)
					{
							blocks =  dbCatalog->GetCardi(TableName)/(dbCatalog->GetBfr(TableName)*2);
						    costs = blocks + level;
				          
					}
					LeftChild->setExInfo("Cost",costs);
					LeftChild->setExInfo("Algorithm",Btree);
					break;
					}
				case EHASH_T:
					{
					
					if(Cond1IsEq)
					{
					    costs =  2;
						blocks = 1;
						LeftChild->setExInfo("Algorithm",Extensible);
					}

					else if(!Cond1IsEq)
					{
					blocks =  dbCatalog->GetCardi(TableName)/(dbCatalog->GetBfr(TableName)*2);
					costs = 2*blocks;
					LeftChild->setExInfo("Algorithm",Linear);
				          
					}
					LeftChild->setExInfo("Cost",costs);
					
					break;
					}
				case LHASH_T:
					{
					if(Cond1IsEq)
					{
					    costs =  1;
						blocks = 1;
						LeftChild->setExInfo("Algorithm",Lhash);
					}
					else
					{
					    blocks =  dbCatalog->GetCardi(TableName)/(dbCatalog->GetBfr(TableName)*2);
					    costs = 2*blocks;
					    LeftChild->setExInfo("Algorithm",Linear);      
					}
					
					LeftChild->setExInfo("Cost",costs);
					break;
					}
				}
				
				
				//CondType
			}
			else 
			{
                blocks = CostCalcTree(LeftChild,dbCatalog);
				costs = blocks;
			}
			SubTreeRoot->setExInfo("Cost",costs);
			SubTreeRoot->setExInfo("Algorithm",LeftChild->getExInfo("Algorithm"));
		    break;
		}
	case	JOIN:
		{
		    assert(SubTreeRoot->hasChild(1));
			assert(SubTreeRoot->hasChild(2));
			costs = 0;
			int BuffSizeBlock = dbCatalog->GetBuffSize()/dbCatalog->GetPageSize();
			int CostsSingle, CostsNest, CostsHash;
			int BlocksLeft, BlocksRight, BlocksResult;
			Idx_Type LeftType; //only test if left attribute has an index for single loop join
			                   //during the join order optimization, left and right child will
			                   //be switched and their costs will be compared
			int IdxLev;
			std::string alg;
			//std::string Condition = boost::get<std::string>(SubTreeRoot->getAttr());
			//std::string TableInCondL,TableInCondR;
			//std::string AttrInCondL,AttrInCondR;
			//std::string token;

			std::string TableInCond1L, TableInCond1R, TableInCond2L, TableInCond2R;
			std::string AttrInCond1L, AttrInCond1R, AttrInCond2L, AttrInCond2R;
			bool Cond1IsEq, Cond2IsEq;
			
			ConditionTokenizer CondOp = boost::any_cast<ConditionTokenizer>(SubTreeRoot->getExInfo("EXPLST"));
			ConditionTokenizer::Type CondType = CondOp.getType();

			Conds conds = CondOp.getCons();
			Cond1IsEq = conds[0].is_equ;

			TableInCond1L = conds[0].ltable_name;
			AttrInCond1L = conds[0].lfield_name;
			TableInCond1R = conds[0].rtable_name;
			AttrInCond1R = conds[0].rfield_name;

			if(CondType != ConditionTokenizer::ALONE)
			{
			   Cond2IsEq = conds[1].is_equ;
			   TableInCond2L = conds[1].ltable_name;
			   AttrInCond2L = conds[1].lfield_name;
			   TableInCond2R = conds[1].rtable_name;
			   AttrInCond2R = conds[1].rfield_name;
			}

			QueryTreeNodePtr LeftChild = SubTreeRoot->getChild(1);
			LeftType = dbCatalog->GetIdx(TableInCond1L, AttrInCond1L);
			QueryTreeNodePtr RightChild = SubTreeRoot->getChild(2);

			BlocksLeft = CostCalcTree(LeftChild, dbCatalog);
			   
			BlocksRight = CostCalcTree(RightChild, dbCatalog);
			
			if(dbCatalog->IsPk(TableInCond1L,AttrInCond1L))
			BlocksResult = BlocksRight;
			else if(dbCatalog->IsPk(TableInCond1R,AttrInCond1R))
			BlocksResult = BlocksLeft;
			else
            BlocksResult = BlocksLeft * BlocksRight/2;
			
			switch(LeftType)
			{
			case BTREE_T:
				{
				  IdxLev = log((float) dbCatalog->GetCardi(TableInCond1L)/dbCatalog->GetBfr(TableInCond1L));
				  CostsSingle = BlocksRight + BlocksRight*dbCatalog->GetBfr(TableInCond1R)*(IdxLev-1);
				  if(LeftChild->getType() == SCAN)
					  LeftChild->setExInfo("Algorithm", Btree);
				break;
				}
			case EHASH_T:
				{
				 IdxLev = 2;
				 CostsSingle = BlocksRight + BlocksRight*dbCatalog->GetBfr(TableInCond1R)*(IdxLev-1);
				 if(LeftChild->getType() == SCAN)
					  LeftChild->setExInfo("Algorithm", Extensible);
				break;
				}
			case LHASH_T:
				{
				 IdxLev = 1;
				 CostsSingle = BlocksRight + BlocksRight*dbCatalog->GetBfr(TableInCond1R)*(IdxLev-1);
				 if(LeftChild->getType() == SCAN)
					  LeftChild->setExInfo("Algorithm", Lhash);
				break;
				}
			case NONE_T:
				{
                 CostsSingle = 10000000;  //make it impossible to be used 
				break;
				}
			}
			
			if(LeftChild->getType() == SCAN)
					  LeftChild->setExInfo("Cost", BlocksLeft);
            
			if(RightChild->getType() == SCAN)
			{
					  RightChild->setExInfo("Cost", BlocksRight);
					  RightChild->setExInfo("Algorithm", Linear);
			}

			CostsNest = BlocksRight + BlocksLeft*(BlocksRight/(BuffSizeBlock-2));
			
			CostsHash = 3*(BlocksLeft + BlocksRight);

			if(CostsNest >= CostsHash)
			{
				costs = CostsHash;
				alg = "PARTITION_HASH_JOIN";
			}
			else
			{
			   costs = CostsNest;
			   alg = "NESTED_LOOP_JOIN";
			}

			if(costs >= CostsSingle)
			{
			   costs = CostsSingle;
			   alg = "SINGLE_LOOP_JOIN";
			}
            
			blocks = BlocksResult;

			costs += blocks;

			SubTreeRoot->setExInfo("Cost",costs);
			SubTreeRoot->setExInfo("Algorithm",alg);

			break;
		}
	case	UNION:
		{
		    assert(SubTreeRoot->hasChild(1));
			assert(SubTreeRoot->hasChild(2));
						
			int BlocksLeft, BlocksRight, BlocksResult;

			QueryTreeNodePtr LeftChild = SubTreeRoot->getChild(1);
			QueryTreeNodePtr RightChild = SubTreeRoot->getChild(2);

			BlocksLeft = CostCalcTree(LeftChild, dbCatalog);
			BlocksRight = CostCalcTree(RightChild, dbCatalog);
			
			BlocksResult=BlocksLeft + BlocksRight;
			
			blocks = BlocksResult;
			costs = blocks;
			SubTreeRoot->setExInfo("Cost",costs);
			SubTreeRoot->setExInfo("Algorithm",Msort);
		break;
		}
	case	PRODUCT:
		{
		    assert(SubTreeRoot->hasChild(1));
			assert(SubTreeRoot->hasChild(2));
			costs = 0;
			
			int BlocksLeft, BlocksRight, BlocksResult;

			QueryTreeNodePtr LeftChild = SubTreeRoot->getChild(1);
			QueryTreeNodePtr RightChild = SubTreeRoot->getChild(2);

			BlocksLeft = CostCalcTree(LeftChild, dbCatalog);
			BlocksRight = CostCalcTree(RightChild, dbCatalog);
			
			BlocksResult=BlocksLeft * BlocksRight;
			
			blocks = BlocksResult;
			costs = blocks;

			SubTreeRoot->setExInfo("Cost",costs);
			SubTreeRoot->setExInfo("Algorithm",Linear);
        //there should be no PRODUCT in the optimized querytree, no need to handle
		break;
		}
	case	PROJECT:
		{
		    assert(SubTreeRoot->hasChild(1));
			assert(!SubTreeRoot->hasChild(2));
						
			//std::string Condition = boost::get<std::string>(SubTreeRoot->getAttr());
			std::string TableInCond;
			std::string AttrInCond;
			std::string token;
			int CondType;  //0 equal;  1 not equal
			float ProjFrac =0.25; //The fraction of blokcs filtered by the project.

			// calculate the project fraction here:

			

			QueryTreeNodePtr LeftChild = SubTreeRoot->getChild(1);

			if(LeftChild->getType() != SCAN)
			{
			   blocks = CostCalcTree(SubTreeRoot->getChild(1), dbCatalog);

			   costs = 0;

	           blocks = blocks * ProjFrac;

			   SubTreeRoot->setExInfo("Algorithm",Msort);
			   SubTreeRoot->setExInfo("Cost",costs);
			}

			else if(LeftChild->getType() == SCAN)
			{
               int HasKey = 1;
			   std::string TableName = boost::get<std::string>(LeftChild->getAttr());

			   blocks = dbCatalog->GetCardi(TableName)/dbCatalog->GetBfr(TableName);
			   
			   
			   if(HasKey)
			   {
			      costs = blocks;
				  LeftChild->setExInfo("Algorithm",Linear);
			   }
			   else
			   {
				  costs = 4*blocks;
				  LeftChild->setExInfo("Algorithm",Msort);

			   }

			   blocks = blocks * ProjFrac;

			   LeftChild->setExInfo("Cost",costs);
			   
			   SubTreeRoot->setExInfo("Algorithm",LeftChild->getExInfo("Algorithm"));
			   SubTreeRoot->setExInfo("Cost",costs);
			}



			

		break;
		}
	}

	blocks++;

	return blocks;

}

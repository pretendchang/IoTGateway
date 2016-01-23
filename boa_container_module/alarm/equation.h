#ifndef _EQUATION_H_
#define _EQUATION_H_

#include "typedef.h"

enum ConditionValueType
{
	ConditionValueType_INT=0,
	ConditionValueType_CONDITION=1,
	ConditionValueType_FLOAT=2
};

enum ConditionRelation
{
	ConditionRelation_GREATER=0,
	ConditionRelation_SMALLER=1,
	ConditionRelation_AND=2,
	ConditionRelation_OR=3,
	ConditionRelation_GREATER_EQUAL=4,
	ConditionRelation_SMALLER_EQUAL=5,
	ConditionRelation_EQUAL=6
};

class ConditionValue
{
	public:
		ConditionValueType valuetype;
		void *value;
		U32 u32Constant;//假如condition value是常數的話，常數的值放在這邊
		float GetRealValue();
		
		
		bool operator>(ConditionValue &);
		bool operator<(ConditionValue &);
		bool operator>=(ConditionValue &);
		bool operator<=(ConditionValue &);
		bool operator==(ConditionValue &);
		bool operator&&(ConditionValue &);
		bool operator||(ConditionValue &);
};

class Condition
{
	public:
		ConditionValue lvalue;
		ConditionValue rvalue;
		ConditionRelation relation;
		int ret;//放condtion算式計算結果
		Condition(){next=0;prev=0;}
		~Condition(){}
		
		Condition *next;
		Condition *prev;
		
		int Execute();
		
};


#endif

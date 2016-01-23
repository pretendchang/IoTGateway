
#include "equation.h"
#include "util.h"
float ConditionValue::GetRealValue()
{
	switch(valuetype)
	{
		case ConditionValueType_INT:
			return (float)(*(int*)value);
			break;
			
		case ConditionValueType_CONDITION:
			return (float)(*(int*)value);
			break;
			
		case ConditionValueType_FLOAT:
			return (*(float*)value);
			break;
	}
}

bool ConditionValue::operator>(ConditionValue &v)
{
	float f1 = GetRealValue();
	float f2 = v.GetRealValue();
	util::DegPrint("%f > %f\n",f1,f2);
	return f1>f2;
}
bool ConditionValue::operator<(ConditionValue &v)
{
	float f1 = GetRealValue();
	float f2 = v.GetRealValue();
	util::DegPrint("%f < %f\n",f1,f2);
	return f1<f2;
}
bool ConditionValue::operator<=(ConditionValue &v)
{
	float f1 = GetRealValue();
	float f2 = v.GetRealValue();
	util::DegPrint("%f <= %f\n",f1,f2);
	return f1<=f2;
}
bool ConditionValue::operator>=(ConditionValue &v)
{
	float f1 = GetRealValue();
	float f2 = v.GetRealValue();
	util::DegPrint("%f >= %f\n",f1,f2);
	return f1>=f2;
}
bool ConditionValue::operator==(ConditionValue &v)
{
	float f1 = GetRealValue();
	float f2 = v.GetRealValue();
	util::DegPrint("%f == %f\n",f1,f2);
	return f1==f2;
}
bool ConditionValue::operator&&(ConditionValue &v)
{
	float f1 = GetRealValue();
	float f2 = v.GetRealValue();
	bool b1=false, b2=false;
	
	if(f1 == 1.0)
		b1=true;
		
	if(f2 == 1.0)
		b2=true;
	util::DegPrint("%d && %d\n",b1,b2);
	return b1 && b2;
}
bool ConditionValue::operator||(ConditionValue &v)
{
	float f1 = GetRealValue();
	float f2 = v.GetRealValue();
	bool b1=false, b2=false;
	
	if(f1 == 1.0)
		b1=true;
		
	if(f2 == 1.0)
		b2=true;
	util::DegPrint("%d && %d\n",b1,b2);
	return b1 || b2;
}

int Condition::Execute()
{
	switch(relation)
	{
	case ConditionRelation_GREATER:
		ret = (lvalue > rvalue);
		break;
	case ConditionRelation_SMALLER:
		ret = (lvalue < rvalue);
		break;
	case ConditionRelation_AND:
		ret = (lvalue && rvalue);
		break;
	case ConditionRelation_OR:
		ret = (lvalue || rvalue);
		break;
	case ConditionRelation_GREATER_EQUAL:
		ret = (lvalue >= rvalue);
		break;
	case ConditionRelation_SMALLER_EQUAL:
		ret = (lvalue <= rvalue);
		break;
	case ConditionRelation_EQUAL:
		ret = (lvalue == rvalue);
		break;
	}

	if(next != 0)
		return next->Execute();
	else
		return ret;
}

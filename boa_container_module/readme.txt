實作CPEModule步驟
1. 新增common.h
2. 在common.h中定義一個class公有繼承自CPEMoudle, 實作虛擬函數
Init 
ModuleThread:while loop已在外面加了，這邊只要實作每個cycle所要處理的事情 
Free
3. 定義InitModuleObject函數，初始化module實體
void InitModuleObject()
{
	printf("init here\n");
	module = new LogSystem();
}
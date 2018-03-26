#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "ID3.h"
#include <set>
#include <iterator>
#include "C4_5.h"
#include "CART.h"
using namespace std;

int USE_ID3 = 0;
int USE_C4_5 = 0;
int USE_CART = 0;
const int ATTR_NUM = 9;


typedef struct node {  //节点 
	int label = 0;
	int neg; //当前节点下正例个数
	int pos; //当前节点下负例个数 
	double CUT = 0; //是否剪枝 
	int attr;  //节点特征 
	vector <int> attr_value;//节点分支依据  
	node* children[50] = {NULL}; //有多个孩子 
}node;


set<int> *Attr_value = new set<int>[9];  //属性集合,每个属性有多个值 
node *root;	 //根节点 
vector<int> Data[1000];	//the dataset
int length; //样本个数 
vector<int> attr; //特征集合 
int current_pos = 0; //目前决策树的准确率 

void init() //initialize the root node and dataset and the attr set 
{
	/* build data set */
	fstream fin("train.csv");	 
	string line;	
	int m = 0;
	while(getline(fin,line)){
		istringstream sin(line);
		string field;
		int n = 0;
		while(getline(sin,field,',')){//Data保存数据 
			Data[m].push_back(atoi(field.c_str()));//初始化数据集 
			if(n!=ATTR_NUM) Attr_value[n].insert(atoi(field.c_str()));//初始化特征集的值 
			n++;
		}	
		m++;
	} 
	length = m;
	for(int i=0;i<ATTR_NUM;i++) attr.push_back(i);//初始化特征集 
	set<int> :: iterator it;
}

bool meet_with_bound(vector<int> index)
{
	int len = index.size();
	int pos = 0;
	int neg = 0;
	if(len==0)	return true; //当前数据集为空 
	for(int i=0;i<len;i++){
		if(Data[index[i]][ATTR_NUM]==1)  pos++;
		else neg++;
	}
	if(neg==0 || pos==0)  return true; //当前数据集标签一致 
	if(attr.size()==0) return true; //当前特征集为空 
	for(int i=0;i<attr.size();i++){
		int a = Data[index[0]][attr[i]]; //特征集取值相同 
		for(int j=1;j<len;j++)	if(Data[index[j]][attr[i]]!=a) return false;
	}
	return true;
}

int choose_attr(vector<int> index)
{
	if(USE_ID3)
	{
		ID3 id3;
		return id3.getAttr(Data,index,attr);
	}
	else if(USE_C4_5)
	{
		C4_5 c4_5;
		return c4_5.getAttr(Data,index,attr);
	}
	else if(USE_CART)
	{
		CART cart;
		return cart.getAttr(Data,index,attr);
	}
}

/* 根据属性 a 分割数据 返回的是样本的一组索引*/ 
vector<vector<int> > divide_data(int a,vector<int> index)
{
	int col = a;
	vector <vector<int>> v;
	vector <int> indexSet[50];
	vector <vector<int> > v2;
	set<int> :: iterator it;
	int m = 0;
	for(it=Attr_value[col].begin();it!=Attr_value[col].end();it++){
		for(int i=0;i<index.size();i++)	if(*it==Data[index[i]][col])  indexSet[m].push_back(index[i]);
		v.push_back(indexSet[m]);	
		indexSet[m].clear(); //释放内存 
		m++;	
	}
	return v;
}

void recursive(node *p,vector <int> index)
{	
	int attr_chosen = choose_attr(index);//选择最好的属性 
	vector<vector<int> > subsets = divide_data(attr_chosen,index);	 
	p->attr = attr_chosen;
	for(int i=0;i<attr.size();i++) //特征集删除选过的特征 
		if(attr[i]==attr_chosen) 
			 attr.erase(attr.begin()+i,attr.begin()+i+1);
	set<int> :: iterator it;
	//节点分支初始化 
	for(it=Attr_value[attr_chosen].begin();it!=Attr_value[attr_chosen].end();it++)
		p->attr_value.push_back(*it);
	it = Attr_value[attr_chosen].begin();
	//节点pos和neg的赋值
	int pos=0,neg=0;
	for(int i=0;i<index.size();i++){
		if(Data[index[i]][ATTR_NUM]==1) pos++;
		else neg++;
	}
	p->pos=pos; p->neg = neg;
	//节点分支和边界的处理 
	for(int i=0;i<subsets.size();i++,it++)
	{
		node *subnode = new node;
		/* recursion */
		p->children[*it] = subnode;
		if(meet_with_bound(subsets[i])){//可优化 如果下一个节点是递归结束，下个节点属性则在当前结点处理 
			if(subsets[i].size()==0){//下个节点数据集为空 
				if(p->pos >= p->neg) subnode->label = 1;
				else 	subnode->label =  -1;
				subnode->pos = 0;
				subnode->neg = 0;
				continue; 
			}
			else{	//满足递归结束的其他条件，在下个节点处理节点属性	
				pos=0,neg = 0;
				for(int j=0;j<subsets[i].size();j++){
					if(Data[subsets[i][j]][ATTR_NUM]==1) pos++;
					else neg++;
				}
				if(pos >= neg)	subnode->label = 1;
				else	subnode->label =  -1;
				subnode->pos = pos;
				subnode->neg = neg;
				continue;
			}
		}
		recursive(subnode,subsets[i]);		
	}
	attr.push_back(attr_chosen);
	subsets.clear();
	return;
}

bool nextLef(node* n){ //判断孩子是否都为叶子 
	for(int i=0;i<50;i++)
		if(n->children[i]!=NULL && n->children[i]->label==0)
			return false;
	return true;
} 
 


double eT(node* n){ //遍历 并标记哪个节点可以剪枝 
	if(n->label!=0){ //叶子节点 
		double et;//计算叶子节点的et 
		if(n->pos > n->neg) et = n->neg + 0.5;
		else if(n->pos < n->neg) et = n->pos + 0.5;
		else et = 0.0;
		return et;
	}
	double et;
	double ET=0.0;
	if(n->pos >= n->neg) et = n->neg + 0.5;
	else et = n->pos + 0.5;  //计算当前节点的et 
	for(int i=0;i<50;i++){
		if(n->children[i]!=NULL)
			ET+=eT(n->children[i]); //递归计算当前节点的ET 
	}
	double p_sum = n->pos + n->neg;
	double SE = ET * (p_sum-ET)/p_sum;	
	//cout<<p_sum<<" "<<ET<<endl;
	SE = sqrt(SE);
	if(et<=ET+SE) n->CUT = 1;
	return ET;
} 

void cut(node* n){ //剪枝 
	if(n->CUT==1){
		if(n->pos > n->neg) n->label = 1;
		else n->label = -1; 
	}
}

void ergo(node* n){ //遍历 
	if(n->label!=0) return;
	cut(n);
	if(n->label!=0) return;
	for(int i=0;i<50;i++)
		if(n->children[i]!=NULL)	ergo(n->children[i]);
}

void validation(vector<int> index){//评估 
	/*对生成的树评估
	  采用留一法评估
	*/ 
	int pos=0;
	fstream fout("result.txt",ios::out);
	for(int i=0;i<length;i++){
		index.erase(index.begin(),index.begin()+1);	//只有一个评估样本，其余训练 
		root = new node; //分配空间 
		recursive(root,index); //建树 
		eT(root); //遍历树，标记可以剪枝的节点
		ergo(root); //遍历树并剪枝。 
		node* n = root;
		while(n->label==0){ //验证 
			int attr_value = Data[i][n->attr];
			n = n->children[attr_value];
		} 
		fout<<n->label<<"\n";
	//	if(n->label==1)	pos++;
		index.push_back(i);
	}	
}


void predict(){
	vector<int> index;
	for(int i=0;i<length;i++) index.push_back(i);//初始化索引，代表原数据 
	root = new node; 
	recursive(root,index);
	eT(root); //遍历树，标记可以剪枝的节点
	ergo(root); //遍历树并剪枝。 
	node* n = root;
	fstream fin("test.csv");
	fstream fout("test_result.txt",ios::out);	 
	string line;	
	int m = 0;
	vector<int> Data_Test[1000];	//the test dataset
	while(getline(fin,line)){
		istringstream sin(line);
		string field;
		while(getline(sin,field,',')){//Data保存数据 
			Data_Test[m].push_back(atoi(field.c_str()));//初始化数据集 
		}		
		node* n = root;	
		int flag = 0;
		while(n->label==0){
			int attr_value = Data_Test[m][n->attr];
			if(n->children[attr_value]==NULL){
				flag = 1;
				if(n->pos >= n->neg) fout<<1<<"\n";
				else fout<<-1<<"\n";
				break;
			}
			n = n->children[attr_value];
		} 
		if(flag==0) fout<<n->label<<"\n";
		m++;
	} 
}

int main(){
	cout<<"ID3: 0"<<endl;
	cout<<"C4_5: 1"<<endl;
	cout<<"CART: 2"<<endl;
	cout<<"请输入相应的数字来选择对应的属性选择方法"<<endl; 
	int a;
	cin >> a;
	if(a==0) USE_ID3= 1;
	else if(a==1) USE_C4_5 = 1;
	else USE_CART = 1; 
	init();
	vector<int> index;
	for(int i=0;i<length;i++) index.push_back(i);//初始化索引，代表原数据 
	validation(index);
	predict();
}

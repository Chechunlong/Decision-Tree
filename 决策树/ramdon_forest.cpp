#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <time.h>
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
	int attr;  //节点特征 
	vector <int> attr_value;//节点分支依据  
	node* children[50] = {NULL}; //有多个孩子 
}node;


set<int> *Attr_value = new set<int>[9];  //属性集合,每个属性有多个值 
vector<int> Data[1000];	//the dataset
int length; //样本个数 
vector<int> attr; //特征集合 
const int sample_num =300; //每个树的样本数目 
const int attr_num = 4;  //随机选择属性个数 
const int tree_num =50; //树的个数 
node *root[tree_num]; //根节点 
int vis[787]={0};  //vis[0]属于袋内数据，用于训练 
int temp = 0;

vector<int> select_data(){  //随机选择样本   
	vector<int> index;  
	for(int i=0;i<sample_num;i++){
		int a = rand()%787;
		while(vis[a]==1) a = rand()%787;  //vis[a]=1属于袋外数据 
		index.push_back(a);
	}
	return index;
}

vector<int> select_attr(){ //随机选择属性 
	vector<int> attr;
	int attr_vis[9]={0};
	for(int i=0;i<attr_num;i++){
		int a = rand()%9;
		while(attr_vis[a]==1)	a = rand()%9;
		attr_vis[a]=1;
		attr.push_back(a);
	}
	return attr;
}

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

//测试 
void predict(){ 

	for(int i=0;i<tree_num;i++){ //建树和建森林 
		root[i] = new node; //分配空间 
		vector<int> index = select_data();
		attr.clear();
		attr = select_attr();
		recursive(root[i],index);
	}		
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
		int pos=0,neg=0;
		for(int j=0;j<tree_num;j++){
			node* n = root[j];
			while(n->label==0){
				int attr_value = Data_Test[m][n->attr];
				if(n->children[attr_value]==NULL){
					if(n->pos >= n->neg) pos++;
					else neg++;
					break;
				}
				n = n->children[attr_value];
			} 
			if(n->label==1)	pos++;
			else neg++;
		}	
		if(pos>=neg)   fout<<1<<"\n";
		else fout<<-1<<"\n";
		m++;
	} 
}

void validation(){//评估 
	/*对生成的树评估
	  采用留一法评估
	*/ 
	int pos=0;
	fstream fout("result.txt",ios::out);
	for(int i=0;i<length;i++){
		vis[i] = 1;   //第i行数据用于验证 
		for(int j=0;j<tree_num;j++){ //建树和建森林 
			root[j] = new node; //分配空间 
			vector<int> index = select_data();
			attr.clear();
			attr = select_attr();
			recursive(root[j],index);
		}	
		int pos=0,neg=0;
		for(int j=0;j<tree_num;j++) {
			node* n = root[j];
			while(n->label==0){ //验证 
				int attr_value = Data[i][n->attr];
				n = n->children[attr_value];
			} 
			if(n->label==1) pos++;
			else neg++;
		}	 
		if(pos>=neg)   fout<<1<<"\n";
		else fout<<-1<<"\n";
		vis[i]=0;
	}	
}


int main(){
	cout<<"ID3: 0"<<endl;
	cout<<"C4_5: 1"<<endl;
	cout<<"CART: 2"<<endl;
	cout<<"请输入相应的数字来选择对应的属性选择方法"<<endl; 
	int a;
	cin >> a;
	srand((unsigned)time(NULL));  
	if(a==0) USE_ID3 = 1;
	else if(a==1) USE_C4_5 = 1;
	else USE_CART = 1; 
	init();
	predict();
}

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


typedef struct node {  //�ڵ� 
	int label = 0;
	int neg; //��ǰ�ڵ�����������
	int pos; //��ǰ�ڵ��¸������� 
	double CUT = 0; //�Ƿ��֦ 
	int attr;  //�ڵ����� 
	vector <int> attr_value;//�ڵ��֧����  
	node* children[50] = {NULL}; //�ж������ 
}node;


set<int> *Attr_value = new set<int>[9];  //���Լ���,ÿ�������ж��ֵ 
node *root;	 //���ڵ� 
vector<int> Data[1000];	//the dataset
int length; //�������� 
vector<int> attr; //�������� 
int current_pos = 0; //Ŀǰ��������׼ȷ�� 

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
		while(getline(sin,field,',')){//Data�������� 
			Data[m].push_back(atoi(field.c_str()));//��ʼ�����ݼ� 
			if(n!=ATTR_NUM) Attr_value[n].insert(atoi(field.c_str()));//��ʼ����������ֵ 
			n++;
		}	
		m++;
	} 
	length = m;
	for(int i=0;i<ATTR_NUM;i++) attr.push_back(i);//��ʼ�������� 
	set<int> :: iterator it;
}

bool meet_with_bound(vector<int> index)
{
	int len = index.size();
	int pos = 0;
	int neg = 0;
	if(len==0)	return true; //��ǰ���ݼ�Ϊ�� 
	for(int i=0;i<len;i++){
		if(Data[index[i]][ATTR_NUM]==1)  pos++;
		else neg++;
	}
	if(neg==0 || pos==0)  return true; //��ǰ���ݼ���ǩһ�� 
	if(attr.size()==0) return true; //��ǰ������Ϊ�� 
	for(int i=0;i<attr.size();i++){
		int a = Data[index[0]][attr[i]]; //������ȡֵ��ͬ 
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

/* �������� a �ָ����� ���ص���������һ������*/ 
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
		indexSet[m].clear(); //�ͷ��ڴ� 
		m++;	
	}
	return v;
}

void recursive(node *p,vector <int> index)
{	
	int attr_chosen = choose_attr(index);//ѡ����õ����� 
	vector<vector<int> > subsets = divide_data(attr_chosen,index);	 
	p->attr = attr_chosen;
	for(int i=0;i<attr.size();i++) //������ɾ��ѡ�������� 
		if(attr[i]==attr_chosen) 
			 attr.erase(attr.begin()+i,attr.begin()+i+1);
	set<int> :: iterator it;
	//�ڵ��֧��ʼ�� 
	for(it=Attr_value[attr_chosen].begin();it!=Attr_value[attr_chosen].end();it++)
		p->attr_value.push_back(*it);
	it = Attr_value[attr_chosen].begin();
	//�ڵ�pos��neg�ĸ�ֵ
	int pos=0,neg=0;
	for(int i=0;i<index.size();i++){
		if(Data[index[i]][ATTR_NUM]==1) pos++;
		else neg++;
	}
	p->pos=pos; p->neg = neg;
	//�ڵ��֧�ͱ߽�Ĵ��� 
	for(int i=0;i<subsets.size();i++,it++)
	{
		node *subnode = new node;
		/* recursion */
		p->children[*it] = subnode;
		if(meet_with_bound(subsets[i])){//���Ż� �����һ���ڵ��ǵݹ�������¸��ڵ��������ڵ�ǰ��㴦�� 
			if(subsets[i].size()==0){//�¸��ڵ����ݼ�Ϊ�� 
				if(p->pos >= p->neg) subnode->label = 1;
				else 	subnode->label =  -1;
				subnode->pos = 0;
				subnode->neg = 0;
				continue; 
			}
			else{	//����ݹ�������������������¸��ڵ㴦��ڵ�����	
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

bool nextLef(node* n){ //�жϺ����Ƿ�ΪҶ�� 
	for(int i=0;i<50;i++)
		if(n->children[i]!=NULL && n->children[i]->label==0)
			return false;
	return true;
} 
 


double eT(node* n){ //���� ������ĸ��ڵ���Լ�֦ 
	if(n->label!=0){ //Ҷ�ӽڵ� 
		double et;//����Ҷ�ӽڵ��et 
		if(n->pos > n->neg) et = n->neg + 0.5;
		else if(n->pos < n->neg) et = n->pos + 0.5;
		else et = 0.0;
		return et;
	}
	double et;
	double ET=0.0;
	if(n->pos >= n->neg) et = n->neg + 0.5;
	else et = n->pos + 0.5;  //���㵱ǰ�ڵ��et 
	for(int i=0;i<50;i++){
		if(n->children[i]!=NULL)
			ET+=eT(n->children[i]); //�ݹ���㵱ǰ�ڵ��ET 
	}
	double p_sum = n->pos + n->neg;
	double SE = ET * (p_sum-ET)/p_sum;	
	//cout<<p_sum<<" "<<ET<<endl;
	SE = sqrt(SE);
	if(et<=ET+SE) n->CUT = 1;
	return ET;
} 

void cut(node* n){ //��֦ 
	if(n->CUT==1){
		if(n->pos > n->neg) n->label = 1;
		else n->label = -1; 
	}
}

void ergo(node* n){ //���� 
	if(n->label!=0) return;
	cut(n);
	if(n->label!=0) return;
	for(int i=0;i<50;i++)
		if(n->children[i]!=NULL)	ergo(n->children[i]);
}

void validation(vector<int> index){//���� 
	/*�����ɵ�������
	  ������һ������
	*/ 
	int pos=0;
	fstream fout("result.txt",ios::out);
	for(int i=0;i<length;i++){
		index.erase(index.begin(),index.begin()+1);	//ֻ��һ����������������ѵ�� 
		root = new node; //����ռ� 
		recursive(root,index); //���� 
		eT(root); //����������ǿ��Լ�֦�Ľڵ�
		ergo(root); //����������֦�� 
		node* n = root;
		while(n->label==0){ //��֤ 
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
	for(int i=0;i<length;i++) index.push_back(i);//��ʼ������������ԭ���� 
	root = new node; 
	recursive(root,index);
	eT(root); //����������ǿ��Լ�֦�Ľڵ�
	ergo(root); //����������֦�� 
	node* n = root;
	fstream fin("test.csv");
	fstream fout("test_result.txt",ios::out);	 
	string line;	
	int m = 0;
	vector<int> Data_Test[1000];	//the test dataset
	while(getline(fin,line)){
		istringstream sin(line);
		string field;
		while(getline(sin,field,',')){//Data�������� 
			Data_Test[m].push_back(atoi(field.c_str()));//��ʼ�����ݼ� 
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
	cout<<"��������Ӧ��������ѡ���Ӧ������ѡ�񷽷�"<<endl; 
	int a;
	cin >> a;
	if(a==0) USE_ID3= 1;
	else if(a==1) USE_C4_5 = 1;
	else USE_CART = 1; 
	init();
	vector<int> index;
	for(int i=0;i<length;i++) index.push_back(i);//��ʼ������������ԭ���� 
	validation(index);
	predict();
}

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>
using namespace std;

class CART{
	public:
		int getAttr(vector <int> *data,vector <int> index,vector<int> attr){
				double pos=0,neg=0;
				int len = index.size();
				for(int i=0;i<len;i++){ //计算数据集中正例与反例的个数 
					int a = index[i];
					if(data[a][data[a].size()-1]==1) pos++;
					else neg++;
				}
				/*计算各属性的基尼系数*/ 
				double *gini = new double[attr.size()];
				for(int k=0;k<attr.size();k++){
					int i = attr[k];
					double attr_pos[50]={0};
					double attr_neg[50]={0};
					for(int j=0;j<len;j++){ //计算各分支的正例与反例的个数 
						int a = index[j];
						if(data[a][data[a].size()-1]==1) attr_pos[data[a][i]]++;
						else attr_neg[data[a][i]]++;	
					}
					gini[k] = 0;  
					for(int j=0;j<50;j++) { //计算各属性的基尼系数 
						if(attr_pos[j]!=0 || attr_neg[j]!=0){
							double p_sum = attr_pos[j]+attr_neg[j];
							gini[k]=p_sum/(len*1.0)*(1-pow(attr_pos[j]/p_sum,2)-pow(attr_neg[j]/p_sum,2));	
						}		
					}
				}
				double MIN = 99999;
				int position;
				for(int i=0;i<attr.size();i++){
					if(gini[i]<MIN)	{
						MIN= gini[i]; 
						position = attr[i];
					} 
				}
				return position;
			} 
};

#ifndef DET_PRO
#define DET_PRO
#include<iostream>
#include<vector>
#include<algorithm>

using std::cout;
using std::cin;
namespace Det
{

std::vector< std::vector<long double>> a;
int rankGot=0;
const int max_rank=100;
struct Info 
{int nowrank;
std::vector<int> detail;
Info(int nowrank0):nowrank(nowrank0)
{for(int i=0;i<nowrank0;i++)
{
	detail.push_back(i);
}

}
Info(const Info & info0,int to_delete)
{
	nowrank=info0.nowrank-1;
	detail=info0.detail;
	auto it=std::find(detail.begin(),detail.end(),to_delete);
	detail.erase(detail.begin()+std::distance(detail.begin(),it));

}
};
using indic =std::array<short,4>;//0 up ,1 down ,2 left ,3 right
int compute_n_rank_(int rank,Info info0)
{
if(rank==1)
{
	return a[0][0];
}



	if(rank==2)
	{
		return -(a[rankGot-2][info0.detail.back()]*a[rankGot-1][info0.detail.front()]-a[rankGot-2][info0.detail.front()]*a[rankGot-1][info0.detail.back()]);

	}
int total=0;
for(int i=0;i<rank;i++)
{


total+= (a[rankGot-rank][info0.detail[i]])* (i%2?-1:1)*compute_n_rank_(rank-1,Info(info0,info0.detail[i]));
//相当于对行列式中第一行第i+1列做划分
//当前info0的vector中每个元素的值都要访问一次，作为上面第二个空格处缺少的参数
//计算时，本次循环的子行列式的正负来自当前对应的info0的某一个值，可以得出正负
}
return total;
}




int Getdata()
{
cin>>rankGot;
Info info(rankGot);
a.resize(rankGot);

for(int i=0;i<rankGot;i++)
{a[i].resize(rankGot);
    for(int j=0;j<rankGot;j++)
    {
cin>>a[i][j];
    }
}




return compute_n_rank_(rankGot,info);
}


}
#endif

#include "author.h"
#include "csv.h"
#include "edges.h"
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <set>
#include <map>
#include <vector>
#include <iterator>
#include <cmath>
#include <ctime>
#include <sstream>

using namespace std;

class skill
{
public:
	skill(){};
	vector<string> holders;
	void push_back(string s){holders.push_back(s);}
	void setFreq(double d){frequency = d;}
	bool isCover;// false;
	double getFreq(){return frequency;}
	string getName(){return name;}
	void setName(string n){name = n;}
	string i_prem;
	skill(string name, double freq):name(name),frequency(freq){holders.clear();isCover=false;};
private:
	string name;
	
	double frequency;
};

class author_skill : public author
{
public:
	author_skill(string name = NULL):author(name){};
	vector<skill*> skill_set;
};

class shortest
{
public:
	vector<string> road;
	double weight;
	string owner;
	shortest(double weight=100,string name=NULL):weight(weight),owner(name){};
};


void getskills(const float high,const float low ,vector<skill*>& collection);
void getIndex(map<string,vector<skill*>::size_type>& index);
void getPersonCost(map<string,double>& costMap);

template <class T> 
string ConvertToString(T value) {
  std::stringstream ss;
  ss << value;
  return ss.str();
}

int exchange(edges *a,edges *b)
{
	edges t;
	t = *a;
	*a = *b;
	*b = t;
	return 0;
}

int partition(edges*edge,int p,int r)
{
  int i = p-1,j = p;
  
  for(;j<r;j++)
  {
	  if(edge[j].getWeight() <= edge[r].getWeight())
   {
    i++;
    exchange(edge+i,edge+j);
   }
  }
  exchange(&edge[i+1],&edge[r]);
  return i+1;
}
int quick_sort(edges edge[],int p,int r)
{
 if(p < r)
 {
  int q = partition(edge,p,r);
  quick_sort(edge,p,q-1);
  quick_sort(edge,q+1,r);
 }
 return 0;
}

void MST_Kruskal(edges edge[],map<string,double>& nodeSet,set<edges>& eSet,int n);

int main()
{
	const float FREQ_THRESHOLD_HIGH = 0.05;
	const float FREQ_THRESHOLD_LOW = 0.001;
	const unsigned int NUM_OF_TASK = 100;
	//{2,4,6,8,10,12,14,16,18,20};
	const unsigned short REQUIRED_SKILL_NUM = 8;
	int num[] = {2,4,6,8,10,12,14,16,18,20};
	float lambda[] = {0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9};
	const string PATH = "E:\\ProgramData\\Java\\DBLP\\20130726\\";
	const string SUFFIX = ".csv";
	map<string,vector<skill*>::size_type> index;
	getIndex(index); //get the name-index map;
	map<string,double> costMap;
	getPersonCost(costMap);
	vector<skill*> SKILLSET;
	map<string,author_skill*> AUTHORSET;
	getskills(FREQ_THRESHOLD_HIGH,FREQ_THRESHOLD_LOW,SKILLSET);
	ofstream output; //FOR DEBUGGING
	for(int lambda_index = 0; lambda_index<9;lambda_index++)
	{
		for(int pos = 0;pos<10;pos++)
		{
			output.open(PATH+"resultApprox\\"+"result_"+ConvertToString( num[pos])+"_"+ConvertToString(lambda_index)+SUFFIX,ios::out|ios::binary|ios::trunc);
			for(unsigned int times=0;times!=NUM_OF_TASK;++times)
			{
				clock_t begin_time = clock();
				clock_t total_time = 0L;
				clock_t file_time = 0L;
				cout<<times+1<<endl;
				map<string,skill*> TASK;
				srand(time(0)+times);
				skill* rarest_skill;
				set<string> TASK_HOLDER_SET;
				while(TASK.size()!=num[pos])
				{
					int required_skill_index = rand()%SKILLSET.size();
					cout<<required_skill_index<<endl;
					SKILLSET[required_skill_index]->isCover = false;
					TASK.insert(make_pair(SKILLSET[required_skill_index]->getName(),SKILLSET[required_skill_index]));
				}
				map<string,author_skill*> AUTHORSET;
				for(map<string,skill*>::iterator it_task = TASK.begin();it_task!=TASK.end();++it_task)
				{
					for(vector<string>::iterator it_holder = it_task->second->holders.begin();it_holder!=it_task->second->holders.end();it_holder++)
					{
						if(*it_holder=="")
							continue;
						if(AUTHORSET.find(*it_holder)==AUTHORSET.end())
						{
							author_skill* au = new author_skill(*it_holder);
							AUTHORSET.insert(make_pair(*it_holder,au));
							AUTHORSET[*it_holder]->skill_set.push_back(it_task->second);
						}
						else
						{
							AUTHORSET[*it_holder]->skill_set.push_back(it_task->second);
						}
					}

				}
				cout<<"AUTHOR SIZE:"<<AUTHORSET.size()<<endl;
				map<string,map<string,double> > UNIVERSE_DIST;
				for(map<string,author_skill*>::iterator it_au_s = AUTHORSET.begin();it_au_s!=AUTHORSET.end();++it_au_s)
				{
					vector<string> row;
					ifstream holderPathFile;
					string line;
					map<string,double> shortestDist;
					string p = PATH+it_au_s->first+SUFFIX;
					clock_t read_start = clock();
					holderPathFile.open(p.c_str(),ios::in|ios::binary);
					if(holderPathFile.fail())
					{
						p = PATH + ConvertToString(index[it_au_s->first])+SUFFIX;
						holderPathFile.open(p.c_str(),ios::in|ios::binary);
					}	
					if(holderPathFile.fail())
					{
						cout<<it_au_s->first<<"OPEN FAIL"<<endl;
						return 0;
					}	
					while(getline(holderPathFile,line),holderPathFile.good())
					{
						csvRead::csvline_populate(row,line,',');
						if(row[1]=="DISCONNECTED")
						{
							UNIVERSE_DIST[it_au_s->first].insert(make_pair(row[0],999));
						}
						else
						{
							UNIVERSE_DIST[it_au_s->first].insert(make_pair(row[0],atof(row[1].c_str())));
						}
					}
					holderPathFile.close();
					read_start = clock()-read_start;
					file_time += read_start;
				}

				double leastSumDistance = 9999999;
				map<string,string> bestTeam;
				bool fail = false;
				for(map<string,skill*>::iterator it_task = TASK.begin();!fail&&it_task!=TASK.end();++it_task)
				{
					for(vector<string>::iterator candidate = it_task->second->holders.begin();candidate!=it_task->second->holders.end();++candidate)
					{
						double sumDistance = 0;
						map<string,string> team;
						team.insert(make_pair(it_task->first,*candidate));
						for(map<string,skill*>::iterator it_skill = TASK.begin();!fail&&it_skill!=TASK.end();++it_skill)
						{
							if(it_task==it_skill)
								continue;
							double distance=9999.0;
							string selectdExpert;
							for(vector<string>::iterator neighbour = it_skill->second->holders.begin();neighbour!=it_skill->second->holders.end();++neighbour)
							{
								if(UNIVERSE_DIST[*candidate][*neighbour]==999.0)
									continue;
								double tempDis = (1-lambda[lambda_index])*(costMap[*candidate]+costMap[*neighbour])+2*lambda[lambda_index]*(UNIVERSE_DIST[*candidate][*neighbour]);
								if(tempDis<distance)
								{
									distance = tempDis;
									selectdExpert = *neighbour;
								}
							}
								sumDistance+=distance;
								team.insert(make_pair(it_skill->first,selectdExpert));
						}
						if(!fail&&sumDistance<leastSumDistance)
						{
							leastSumDistance = sumDistance;
							bestTeam = map<string,string>(team.begin(),team.end());
						}
					}
				}

				if(!fail)
					total_time = clock()-begin_time-file_time;

				double diameter = 0;
				set<edges> graph;
				if(bestTeam.size()>0)
				{
					for(map<string,string>::iterator mem = bestTeam.begin();mem!=bestTeam.end();++mem)
					{
						for(map<string,string>::iterator des = bestTeam.begin();des!=bestTeam.end();++des)
						{
							if(des==mem)
								continue;
							if(UNIVERSE_DIST[mem->second][des->second]>diameter)
								diameter=UNIVERSE_DIST[mem->second][des->second];
							edges e(mem->second,des->second);
							e.setWeight(UNIVERSE_DIST[mem->second][des->second]);
							graph.insert(e);
						}
					}
				}

				double MST = 0;
				if(!fail)
				{
					edges *edge = new edges[graph.size()];
					map<string,double> MAKE_SET;
					set<edges> MST_edge; 
					if(graph.size()>=1)
					{
						int index = 0;
						for(set<edges>::iterator it = graph.begin();it!=graph.end();it++)
							edge[index++] = *it;
						index = 0;
						for(map<string,string>::iterator mem = bestTeam.begin();mem!=bestTeam.end();++mem)
						{
							MAKE_SET[mem->second]=index++;
						}
						MST_Kruskal(edge,MAKE_SET,MST_edge,graph.size());	
						for(set<edges>::iterator it = MST_edge.begin();it!=MST_edge.end();++it)
						{
							MST+= it->getWeight();
						}
					}

				}

				if(!fail)
				{
					
					int line = 0;
					string p_a_t_h="";
					string s_k_i_l_l="";
					for(map<string,string>::iterator it = bestTeam.begin();it!=bestTeam.end();++it)
					{
						cout<<"SKILL:"<<it->first<<endl;
						s_k_i_l_l+=it->first+",";
						p_a_t_h+=it->second+",";

					}
					cout<<"Computational time: "<<total_time<<endl;
					cout <<leastSumDistance<<endl;
					output<<line++<<","<<total_time<<","<<s_k_i_l_l<<","<<p_a_t_h<<","<<leastSumDistance<<","<<diameter<<","<<MST<<endl;
				}
			}
	
			output.close();
		}
	}
}


void getIndex(map<string,vector<skill*>::size_type>& index)
{
	vector<string> row;
	ifstream in("E:\\ProgramData\\Java\\DBLP\\20130725\\Test\\dblp_coauthor_l_r.csv000",ios::binary);
    if (in.fail())  { cout << "File not found" <<endl; return ; }
	unsigned int count=0;
	string line;
    while(getline(in, line)  && in.good() )
    {
        csvRead::csvline_populate(row, line, ',');

			index.insert(make_pair(row[0],count++));
    }
    in.close();
}

void getPersonCost(map<string,double>& costMap)
{
	vector<string> row;
	const double outlier_lowerBound = 150.0;
	const double upperBound = 300.0;
	const double intervalSize = 150.0-3.0;
	ifstream in("E:\\ProgramData\\Java\\DBLP\\20130725\\Test\\dblp_coauthor_l_r.csv000",ios::binary);
    if (in.fail())  { cout << "File not found" <<endl; return ; }
	unsigned int count=0;
	string line;
    while(getline(in, line)  && in.good() )
    {
        csvRead::csvline_populate(row, line, ',');
		if(atof(row[1].c_str())>=150)
			costMap.insert(make_pair(row[0],1-(upperBound-atof(row[1].c_str()))/(outlier_lowerBound/0.1)));
		else
			costMap.insert(make_pair(row[0],1-0.8*(outlier_lowerBound-atof(row[1].c_str()))/intervalSize));
    }
    in.close();
}


void getskills(const float high,const float low ,vector<skill*>& collection)
{
	ifstream readSkill ("E:\\ProgramData\\Java\\DBLP\\20130727\\Test\\skill_l.csv",ios::binary);
	vector<string> row;
	string line;
	vector<string>::size_type line_no=0;
	while(getline(readSkill,line)&&readSkill.good())
	{
		csvRead::csvline_populate(row,line,',');
		if(atof(row[1].c_str())<=high&&atof(row[1].c_str())>=low&&row[0].size()>1)
		{
			string name = row[0];
			double frequency = atof(row[1].c_str());
			skill *s = new skill(name,frequency);
			//s->index=line_no++;
			for(vector<string>::size_type i = 2;i!=row.size()&&row[i]!="";++i)
			{
				s->push_back(row[i]);
			}
			collection.push_back(s);
		}

	}
}

void MST_Kruskal(edges edge[],map<string,double>& nodeSet,set<edges>& eSet,int n)
{
	quick_sort(edge,0,n-1);
	for(int i =0;i<n;i++)
	{
		if(nodeSet[edge[i].getAuthor(false)]!=nodeSet[edge[i].getAuthor(true)])
		{
			eSet.insert(edge[i]);
			double s1 = nodeSet[edge[i].getAuthor(true)];
			double s2 = nodeSet[edge[i].getAuthor(false)];
			for(map<string,double>::iterator it = nodeSet.begin();it!=nodeSet.end();it++)
			{
				if(it->second==s1)
					it->second =s2;
			}

		}
	} 
}
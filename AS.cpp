#include <iostream>
#include <set>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <sstream>

#include <dirent.h>
#include <time.h>
#include <stdlib.h>

#include "json/json.h"


using namespace std;

struct AsConn
{
	AsConn(int as1, int asneighbor1): as(as1), asneighbor(asneighbor1) {}
	int as;
	int asneighbor;
};
struct AsIps
{
	AsIps() {}
	AsIps(int i11, int i21, int i31, int i41):
		i1(i11), i2(i21), i3(i31), i4(i41) {}
	int i1;
	int i2;
	int i3;
	int i4;
	bool operator==(const AsIps &a) const
	{
		return a.i1 == this->i1 && a.i2 == this->i2 &&
		   	a.i3 == this->i3 && a.i4 == this->i4;
	}
};
bool compareAsIps(const AsIps &a1, const AsIps &a2)
{
	if(a1.i1!=a2.i1) return a1.i1<a2.i1;
	else if(a1.i2!=a2.i2) return a1.i2 < a2.i2;
	else if(a1.i3!=a2.i3) return a1.i3 < a2.i3;
	else return a1.i4  < a2.i4;
}

struct AsInfo
{
	AsInfo(int asnom1, const vector<AsIps> &vasIps1):asnom(asnom1)
	{
		vasIps.assign(vasIps1.begin(), vasIps1.end());
	}
	int asnom;
	vector<AsIps> vasIps;
};


vector<AsConn> vasconn; //500 as connection
vector<AsInfo> vAsInfo;  //store 500 as' info(as num and ips) 
set<AsIps, decltype(compareAsIps)*> sotherips(compareAsIps);  //not used ips
set<string> susedips;  //used ips for nei
set<int> susedasnum;  //num of used as in json


int main() 
{
	//stroe as connections
	ifstream fin("./as_rel_500.txt");
	int as1, as2;
	char c1;
	string stemp;
	while(!fin.eof() && fin >> as1 >> c1 >> c1 >> c1 >> c1 >> as2)
	{
		getline(fin, stemp);
		AsConn aSConn(as1, as2);
		vasconn.push_back(aSConn);
	}
	fin.close();

	//read all files in the as directory
	struct dirent *ptr;      
    DIR *dir;  
    string PATH = "./as500";  
    dir = opendir(PATH.c_str());   
    vector<string> asfiles;  
    while((ptr=readdir(dir)) != NULL)  
    {  
   
		//ignore . and .. directory
        if(ptr->d_name[0] == '.')  
            continue;  
        asfiles.push_back(ptr->d_name);  
    }  

	srand((int)time(0));  //4th num for neighbor and update ip 
	//store as infos
	for(const auto& a: asfiles)
	{
		char ctemp;
		int ias;
		istringstream ss(a);
		ss >> ctemp >> ctemp >> ias;

		string asfilename = string("./as500/") + a;
		ifstream fin(asfilename);
		int i1, i2, i3, i4;
		vector<AsIps> vasipstemp;
		while(!fin.eof() &&
			   	fin >> i1 >> ctemp >> i2 >> ctemp >> i3 >> ctemp >> i4)
		{
			AsIps asIpsTemp(i1, i2, i3, 0);
			vasipstemp.push_back(asIpsTemp);
		}
		AsInfo asInfoTemp(ias, vasipstemp);
		vAsInfo.push_back(asInfoTemp);
		fin.close();
	}

	//handle other ips for json's networks parameter
	ifstream fin_otherips("./as00000.txt");
	int i1o, i2o, i3o, i4o;
	char cotemp;
	
	while(!fin_otherips.eof() && 
			fin_otherips >> i1o >> cotemp >> 
			i2o >> cotemp >> i3o >> cotemp >> i4o)
	{
		i4o = 0;
		AsIps asIps(i1o, i2o, i3o, i4o);
		sotherips.insert(asIps);
	}
	fin_otherips.close();


	//json
	Json::Value root;
	Json::Value server;

	int count_conn =0;
	for(const auto& a: vasconn)
	{

cout<<"used ips" <<susedips.size()<<endl;  //used ips for nei
cout<<"used as num;"<< susedasnum.size()<<endl;  //num of used as in json
		if(susedasnum.find(a.as) == susedasnum.end())
		{
			int if_realconn=0;
			Json::Value item;

			Json::Value neighbors;  //neighbors should append continusly
			Json::Value neighbor;
			vector<AsIps> vupdata_source, vneighbor_ip;
			for(const auto& aainfo: vAsInfo)
			{
				if(aainfo.asnom == a.as)
				{
					vupdata_source.assign(aainfo.vasIps.begin(),
							aainfo.vasIps.end());
					break;
				}
			}
			for(const auto& aainfo: vAsInfo)
			{
				if(aainfo.asnom == a.asneighbor)
				{
					vneighbor_ip.assign(aainfo.vasIps.begin(),
							aainfo.vasIps.end());
					break;
				}
			}
			if(vupdata_source.size() && vneighbor_ip.size())
			{
				for(const auto& au: vupdata_source)
				{
					for(const auto& an: vneighbor_ip)
					{
						if(au == an)
						{
							if_realconn = 1;
							stringstream ssu, ssn;
							string su, sn;
							int istart = rand() % 150;
							ssu << au.i1 << "." <<au.i2 <<
								"." << au.i3 << "." << istart;
							ssu >> su;
							while(susedips.find(su) != susedips.end())
							{
								istart = rand() % 150;
								ssu.str("");
								ssu.clear();
								ssu << au.i1<<"."<<au.i2<<"."<<
									au.i3<<"."<<istart;
								ssu >> su;
								cout<<"su 195:"<<su<<endl;
							}
							susedips.insert(su);

							istart = rand() % 150;
							ssn << an.i1 << "." << an.i2 << "." << an.i3 <<
								"."<<istart;
							ssn >> sn;
							while(susedips.find(sn) != susedips.end())
							{
								istart = rand() % 150;
								ssn.str("");
								ssn.clear();
								ssn << an.i1<<"."<<an.i2<<"."<<
									an.i3<<"."<<istart;
								ssn >> sn;
								cout<<"211 sn:"<<sn<<endl;
							}
							susedips.insert(sn);

							neighbor["neighbor_ip"] = sn;
							neighbor["ebgp_multihop"] = 5;
							neighbor["remote_as"] = a.asneighbor;
							neighbor["update_source"] = su;
							neighbors.append(neighbor);

							item["neighbors"] = neighbors;
						}
						if(if_realconn)break;
					}
					if(if_realconn)break;
				}
			}
			// miss conneciton, make a new one
			if(!if_realconn)
			{
				AsIps au, an, aorign;
				au = an = aorign = *(sotherips.begin());
				stringstream ssu, ssn;
				string su, sn;
				int istart = rand() % 150;
				ssu << au.i1 << "." <<au.i2 <<
					"." << au.i3 << "." << istart;
				ssu >> su;
				while(susedips.find(su) != susedips.end())
				{
					istart = rand() % 150;
					ssu.str("");
					ssu.clear();
					ssu << au.i1<<"."<<au.i2<<"."<<
						au.i3<<"."<<istart;
					ssu >> su;
					cout<<"247 su"<<su<<endl;
				}
				susedips.insert(su);

				istart = rand() % 150;
				ssn << an.i1 << "." << an.i2 << "." << an.i3 <<
					"."<<istart;
				ssn >> sn;
				while(susedips.find(sn) != susedips.end())
				{
					istart = rand() % 150;
					ssn.str("");
					ssn.clear();
					ssn << an.i1<<"."<<an.i2<<"."<<
						an.i3<<"."<<istart;
					ssn >> sn;
					cout<<"263 sn"<<sn<<endl;
				}
				susedips.insert(sn);

				neighbor["neighbor_ip"] = sn;
				neighbor["ebgp_multihop"] = 5;
				neighbor["remote_as"] = a.asneighbor;
				neighbor["update_source"] = su;
				neighbors.append(neighbor);

				item["neighbors"] = neighbors;

				//insert the ip the neighbor as
				sotherips.erase(aorign);
				for (int i =0; i< vAsInfo.size(); ++i)
				{
					if(vAsInfo[i].asnom == a.asneighbor)
					{
						AsIps asips1(an.i1, an.i2, an.i3, 0);
						vAsInfo[i].vasIps.push_back(asips1);
						break;
					}	
				}
				for (int i =0; i< vAsInfo.size(); ++i)
				{
					if(vAsInfo[i].asnom == a.as)
					{
						AsIps asips1(au.i1, au.i2, au.i3, 0);
						vAsInfo[i].vasIps.push_back(asips1);
						break;
					}	
				}
			}
				
			stringstream ss;
			string sas;
			ss << a.as;
			ss >> sas;
			item["server_name"] = string("route_") + sas;
			item["server_id"] = "";
			item["iamge"] = "0bfed3a7-a19a-4bb9-949e-d2eee0108d81";
			item["zone"] = "nova";
			item["type"] = "drt";
			item["flavorref"] = "10";
			item["as_name"] = sas;
			Json::Value timer;
			timer["holdtime"] = 30;
			timer["keeplive"] = 10;
			item["timer"] = timer;

			Json::Value networks;
			AsIps asipstemp = *(sotherips.begin());
			string ip_network;
			string cidr_network;
			stringstream ss_ip;
			stringstream ss_cidr;
			ss_ip << asipstemp.i1 <<"."<<asipstemp.i2<<"."<<asipstemp.i3
				<<"."<<asipstemp.i4;
			ss_ip >> ip_network;
			ss_cidr << asipstemp.i1 <<"."<<asipstemp.i2<<"."<<asipstemp.i3
				<<".0/24";
			ss_cidr >> cidr_network;
			networks["cidr"] = cidr_network;
			networks["ip"] = ip_network;
			item["networks"] = networks;
			sotherips.erase(asipstemp);

			server.append(item);  // added to json
			susedasnum.insert(a.as); //used as num
		}
		else
		{
			int if_realconn=0;

			int json_size = server.size();
			for(int i =0; i < json_size; i++)
			{
				cout<<"json server size: "<<json_size<<endl;
				Json::Value neighbor;
				
				stringstream ssas;
				ssas << a.as;
				string sastemp;
				ssas >> sastemp;
				if(server[i]["as_name"] == sastemp)
				{
					vector<AsIps> vupdata_source, vneighbor_ip;
					for(const auto& aainfo: vAsInfo)
					{
						if(aainfo.asnom == a.as)
						{
							vupdata_source.assign(aainfo.vasIps.begin(),
									aainfo.vasIps.end());
							break;
						}
					}
					for(const auto& aainfo: vAsInfo)
					{
						if(aainfo.asnom == a.asneighbor)
						{
							vneighbor_ip.assign(aainfo.vasIps.begin(),
									aainfo.vasIps.end());
							break;
						}
					}
					if(vupdata_source.size() && vneighbor_ip.size())
					{
						for(const auto& au:vupdata_source)
						{
							for(const auto& an:vneighbor_ip)
							{
								if(au == an)
								{
									if_realconn=1;
									stringstream ssu,ssn;
									string su, sn;
									int istart = rand() % 150;
									ssu << au.i1<<"."<<au.i2<<"."<<
										au.i3<<"."<<istart;
									ssu >> su;
									while(susedips.find(su)!=susedips.end())
									{
										istart = rand() % 150;
										ssu.str("");
										ssu.clear();
										ssu << au.i1<<"."<<au.i2<<"."<<
											au.i3<<"."<<istart;
										ssu >> su;
										cout<<"397 su:"<<su<<endl;
									}
									susedips.insert(su);

									istart = rand() % 150;
									ssn << an.i1<<"."<<an.i2<<"."<<an.i3<<
										"."<<istart;
									ssn >> sn;
									while(susedips.find(sn)!=susedips.end())
									{
										istart = rand() % 150;
										ssn.str("");
										ssn.clear();
										ssn << an.i1<<"."<<an.i2<<"."<<
											an.i3<<"."<<istart;
										ssn >> sn;
										cout<<"407 sn:"<<sn<<endl;
									}
									susedips.insert(sn);

									neighbor["neighbor_ip"] = sn;
									neighbor["ebgp_multihop"] = 5;
									neighbor["remote_as"] = a.asneighbor;
									neighbor["update_source"] = su;

									server[i]["neighbors"].append(neighbor);
								}
								if(if_realconn)break;
							}
							if(if_realconn)break;
						}
					}
					//miss as conn, make a new one
					if(!if_realconn)
					{
						AsIps au, an, aorign;
						au = an = aorign = *(sotherips.begin());
						stringstream ssu, ssn;
						string su, sn;
						int istart = rand() % 150;
						ssu << au.i1 << "." <<au.i2 <<
							"." << au.i3 << "." << istart;
						ssu >> su;
						while(susedips.find(su) != susedips.end())
						{
							istart = rand() % 150;
							ssu.str("");
							ssu.clear();
							ssu << au.i1<<"."<<au.i2<<"."<<
								au.i3<<"."<<istart;
							ssu >> su;
							cout<<"442 su:"<<su<<endl;
						}
						susedips.insert(su);

						istart = rand() % 150;
						ssn << an.i1 << "." << an.i2 << "." << an.i3 <<
							"."<<istart;
						ssn >> sn;
						while(susedips.find(sn) != susedips.end())
						{
							istart = rand() % 150;
							cout<<"istart 454: "<<istart<<endl;
							ssn.str("");
							ssn.clear();
							ssn << an.i1<<"."<<an.i2<<"."<< an.i3<<"."<<istart;
							ssn >> sn;
							cout<<"458 sn:"<<sn<<endl;
						}
						susedips.insert(sn);

						neighbor["neighbor_ip"] = sn;
						neighbor["ebgp_multihop"] = 5;
						neighbor["remote_as"] = a.asneighbor;
						neighbor["update_source"] = su;
						//neighbors.append(neighbor);

						//item["neighbors"] = neighbors;
						server[i]["neighbors"].append(neighbor);

						//insert the ip the neighbor as
						sotherips.erase(aorign);
						for (int i1 =0; i1< vAsInfo.size(); ++i1)
						{
							if(vAsInfo[i1].asnom == a.asneighbor)
							{
								AsIps asips1(an.i1, an.i2, an.i3, 0);
								vAsInfo[i1].vasIps.push_back(asips1);
								break;
							}	
						}
						for (int i2 =0; i2< vAsInfo.size(); ++i2)
						{
							if(vAsInfo[i2].asnom == a.as)
							{
								AsIps asips1(au.i1, au.i2, au.i3, 0);
								vAsInfo[i2].vasIps.push_back(asips1);
								break;
							}	
						}
					}

					//end if : ==
					break;
				}
			}
		}
	}

	root["server"] = server;
	//store json file
	string jsonout = root.toStyledString();
	ofstream fout("jsonoutput");
	fout<<jsonout;
	fout.close();

	return 0;
}

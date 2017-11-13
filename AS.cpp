#include <iostream>
#include <set>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <sstream>

#include <dirent.h>

#include"json/json.h"


using namespace std;

struct AsConn
{
	AsConn(int as1, int asneighbor1):as(as1),asneighbor(asneighbor1){}
	int as;
	int asneighbor;
};
struct AsIps
{
	AsIps(){}
	AsIps(int i11, int i21, int i31, int i41):
		i1(i11),i2(i21),i3(i31),i4(i41){}
	int i1;
	int i2;
	int i3;
	int i4;
	bool operator==(const AsIps& a)
	{
		return a.i1 == i1 && a.i2 == i2 && a.i3 == i3 && a.i4 == i4;
	}
};
bool compareAsIps(const AsIps &a1, const AsIps &a2)
{
	if(a1.i1!=a2.i1){
		return a1.i1<a2.i1;
	}
	else if(a1.i2!=a2.i2) return a1.i2 < a2.i2;
	else if(a1.i3!=a2.i3) return a1.i3 < a2.i3;
	else return a1.i4  < a2.i4;
}

struct AsInfo
{
	AsInfo(int asnom1, const vector<AsIps> &vasIps1):asnom(asnom1)
	{
		for(auto a: vasIps1)
		{
			AsIps asIpsTemp(a.i1, a.i2,a.i3,a.i4);
			vasIps.push_back(asIpsTemp);
		}
	}
	int asnom;
	vector<AsIps> vasIps;
};


vector<AsConn> vasconn; //500 as connection
vector<AsInfo> vAsInfo;  //store 500 as' info(as num and ips) 
set<AsIps, decltype(compareAsIps)*> sotherips(compareAsIps);  //not used ips
set<string> susedips;  //used ips for nei

int count_all_connections =0;
int count_real_connections =0;

int main() 
{
	//stroe as connections
	ifstream fin("./as_rel_500.txt");
	int as1, as2;
	char c1;
	string stemp;
	while(!fin.eof() && fin>>as1>>c1>>c1>>c1>>c1>>as2)
	{
		getline(fin,stemp);
		AsConn aSConn(as1, as2);
		vasconn.push_back(aSConn);
	}
	fin.close();

	//read all files in the as directory
	struct dirent *ptr;      
    DIR *dir;  
    string PATH = "./as500";  
    dir=opendir(PATH.c_str());   
    vector<string> asfiles;  
    while((ptr=readdir(dir))!=NULL)  
    {  
   
		//ignore . and .. directory
        if(ptr->d_name[0] == '.')  
            continue;  
        asfiles.push_back(ptr->d_name);  
    }  

	//store as infos
	for(auto a: asfiles)
	{
		char ctemp;
		int ias;
		istringstream ss(a);
		ss>>ctemp>>ctemp>>ias;

		string asfilename = string("./as500/") + a;
		ifstream fin(asfilename);
		int i1,i2,i3,i4;
		vector<AsIps> vasipstemp;
		while(!fin.eof() && fin>>i1>>ctemp>>i2>>ctemp>>i3>>ctemp>>i4)
		{
			AsIps asIpsTemp(i1, i2, i3, i4);
			vasipstemp.push_back(asIpsTemp);
		}
		AsInfo asInfoTemp(ias, vasipstemp);
		vAsInfo.push_back(asInfoTemp);
		fin.close();

	}

	//handle other ips for json's networks parameter
	ifstream fin_otherips("./as00000.txt");
	int i1o,i2o,i3o,i4o;
	char cotemp;
	
	while(!fin_otherips.eof() && 
			fin_otherips>>i1o>>cotemp>>i2o>>cotemp>>i3o>>cotemp>>i4o)
	{
		i4o = 1;
		AsIps asIps(i1o,i2o,i3o,i4o);
		sotherips.insert(asIps);
	}
	fin_otherips.close();


	//json
	Json::Value root;
	Json::Value server;

	set<int> susedasnum;
	for(auto a: vasconn)
	{
		if(susedasnum.find(a.as)==susedasnum.end())
		{
			int if_realconn=0;
			Json::Value item;

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

			Json::Value neighbors;  //neighbors should append continusly
			Json::Value neighbor;
			vector<AsIps> vupdata_source, vneighbor_ip;
			for(auto aainfo: vAsInfo)
			{
				if(aainfo.asnom == a.as)
				{
					vupdata_source.assign(aainfo.vasIps.begin(),
							aainfo.vasIps.end());
					break;
				}
			}
			for(auto aainfo: vAsInfo)
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
				for(auto au:vupdata_source)
				{
					for(auto an:vneighbor_ip)
					{
						if(au == an)
						{
							if_realconn=1;
							stringstream ssu,ssn;
							string su, sn;
							int istart = 2;
							ssu << au.i1<<"."<<au.i2<<"."<<au.i3<<"."<<istart;
							ssu >> su;
							while(susedips.find(su)!=susedips.end())
							{
								istart++;
								ssu.str("");
								ssu << au.i1<<"."<<au.i2<<"."<<
									au.i3<<"."<<istart;
								ssu >> su;
							}
							susedips.insert(su);

							ssn << an.i1<<"."<<an.i2<<"."<<an.i3<<
								"."<<++istart;
							ssn >> sn;
							while(susedips.find(sn)!=susedips.end())
							{
								istart++;
								ssn.str("");
								ssn << an.i1<<"."<<an.i2<<"."<<
									an.i3<<"."<<istart;
								ssn >> sn;
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


			if(if_realconn)
			{
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

				root["server"] = server;

				count_real_connections +=1;
			}
			count_all_connections +=1;

		}
		else
		{
			int if_realconn=0;

			Json::Value item = root["server"];
			int json_size = root["server"].size();
			for(int i =0;i<json_size;i++)
			{
				stringstream ssas;
				ssas << a.as;
				string sastemp;
				ssas >> sastemp;
				if(item[i]["as_name"] ==sastemp)
				{
					Json::Value neighbors = item[i]["neighbors"];
					Json::Value neighbor;
					vector<AsIps> vupdata_source, vneighbor_ip;
					for(auto aainfo: vAsInfo)
					{
						if(aainfo.asnom == a.as)
						{
							vupdata_source.assign(aainfo.vasIps.begin(),
									aainfo.vasIps.end());
							break;
						}
					}
					for(auto aainfo: vAsInfo)
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
						for(auto au:vupdata_source)
						{
							for(auto an:vneighbor_ip)
							{
								if(au == an)
								{
									if_realconn=1;
									stringstream ssu,ssn;
									string su, sn;
									int istart = 2;
									ssu << au.i1<<"."<<au.i2<<"."<<
										au.i3<<"."<<istart;
									ssu >> su;
									while(susedips.find(su)!=susedips.end())
									{
										istart++;
										ssu.str("");
										ssu << au.i1<<"."<<au.i2<<"."<<
											au.i3<<"."<<istart;
										ssu >> su;
									}
									susedips.insert(su);

									ssn << an.i1<<"."<<an.i2<<"."<<an.i3<<
										"."<<++istart;
									ssn >> sn;
									while(susedips.find(sn)!=susedips.end())
									{
										istart++;
										ssn.str("");
										ssn << an.i1<<"."<<an.i2<<"."<<
											an.i3<<"."<<istart;
										ssn >> sn;
									}
									susedips.insert(sn);

									neighbor["neighbor_ip"] = sn;
									neighbor["ebgp_multihop"] = 5;
									neighbor["remote_as"] = a.asneighbor;
									neighbor["update_source"] = su;
									neighbors.append(neighbor);

									//updated method
									root["server"][i]["neighbors"] = neighbors;
									
								}
								if(if_realconn)break;
							}
							if(if_realconn)break;
						}
					}
					if(if_realconn)
					{
						count_real_connections +=1;
					}
					count_all_connections +=1;

					break;
				}
			}
		}

	string jsonoutt = root.toStyledString();
	cout<<jsonoutt<<endl;
	}


	//store json file
	string jsonout = root.toStyledString();
	ofstream fout("jsonoutput");
	fout<<jsonout;
	fout.close();

	cout<<"all count connections:"<<count_all_connections<<endl;
	cout<<"real count connections:"<<count_real_connections<<endl;

	return 0;
}

#include <dirent.h>
#include <time.h>
#include <stdlib.h>
#include <iostream>

#include <set>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <sstream>

#include "json/json.h"

#include "AS.h"

using namespace std;

string tostring(int i)
{
	string s;
	stringstream ss;
	ss << i;
	ss >> s;
	return s;
}
string tostringip(int i1, int i2, int i3, int i4)
{
	stringstream ss;
	string s;
	ss << i1 << "." <<i2 << "." << i3 << "." << i4;
	ss >> s;
	return s;
}
string tostringip(const AsIps& a)
{
	return tostringip(a.i1, a.i2, a.i3, a.i4);
}
//stroe as connections
void store_asconn(const string& filename, 
		set<AsConn> &sasconn)
{
	ifstream fin(filename);
	int as1, as2;
	char c1;
	string stemp;
	while(!fin.eof() && fin >> as1 >> c1 >> c1 >> c1 >> c1 >> as2)
	{
		getline(fin, stemp);
		AsConn aSConn(as1, as2);
		//vasconn.push_back(aSConn);
		sasconn.insert(aSConn);
		AsConn aSConnre(as2, as1);
		sasconn.insert(aSConnre);
	}
	fin.close();
}

//read all files in the as directory
//init as's info
void init_asinfo(const string &filename,
		vector<AsInfo> &vAsInfo)
{
	struct dirent *ptr;      
    DIR *dir;  
    dir = opendir(filename.c_str());   
	std::vector<string> asfiles;  
    while((ptr=readdir(dir)) != NULL)  
    {  
   
		//ignore . and .. directory
        if(ptr->d_name[0] == '.')  
            continue;  
        asfiles.push_back(ptr->d_name);  
    }  

	//store as infos
	for(const auto& a: asfiles)
	{
		char ctemp;
		int ias;
		std::istringstream ss(a);
		ss >> ctemp >> ctemp >> ias;

		std::string asfilename = filename + a;
		std::ifstream fin(asfilename);
		int i1, i2, i3, i4;
		std::vector<AsIps> vasipstemp;
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
}

//init other as's ips
void init_other_ips(const std::string &filename,
		std::set<AsIps> &sotherips)
{
	std::ifstream fin_otherips(filename);
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
}

//get a as's ips
void get_as_ips(const int asnum, vector<AsIps> & vo, const vector<AsInfo> & vi)
{
	for(const auto& a: vi)
	{
		if(a.asnom == asnum)
		{
			vo.assign(a.vasIps.begin(),
					a.vasIps.end());
			break;
		}
	}
}

//update a as's ip (the 4th num)
void update_as_ip(const int asnum, vector<AsInfo> &a, const AsIps & ai, 
		const int v)
{
	int flag = 0;
	for(auto& aas: a)
	{
		if(aas.asnom == asnum)
		{
			for(int ii =0; ii < aas.vasIps.size(); ++ii)
			{
				if (aas.vasIps[ii] == ai)
				{
					aas.vasIps[ii].i4 = v;
					flag = 1;
					break;
				}
			}
			break;
		}
	}	
	if(!flag)
		cout << "update ip failed !!!"<<endl;
}	
//generate neighbor ip
void generate_neighbor_ip(string &s, const multiset<string> &sa,
	   const AsIps &a, int &i4)
{
	while(sa.find(s) != sa.end())
	{
		i4 = rand() % 150 + 2;
		s = tostringip(a.i1, a.i2, a.i3, i4);
	}
}

//insert a as ip when they are connected but have no matching ips
void insert_as_ip(const int asnum, vector<AsInfo> & va, 
		const AsIps & a, const int v)
{
	int flag =0;
	for (int i =0; i< va.size(); ++i)
	{
		if(va[i].asnom == asnum)
		{
			AsIps asips1(a.i1, a.i2, a.i3, v);
			va[i].vasIps.push_back(asips1);
			flag = 1;
			break;
		}	
	}
	if(!flag)
	{
		cout<<"insert new ip failed !!!"<<endl;
	}
}

//set json neighbor
void set_json_neighbor(Json::Value & neighbor, const string &ni,
		const int ebgp, const int ras, const string &us)
{
	neighbor["neighbor_ip"] = ni;
	neighbor["ebgp_multihop"] = ebgp;
	neighbor["remote_as"] = ras;
	neighbor["update_source"] = us;
}
//set basic json info
void set_basic_json(Json::Value &item, const int asnum, set<AsIps> &s)
{
	string sas = tostring(asnum);
	item["server_name"] = string("route_") + sas;
	item["server_id"] = "";
	item["image"] = "0bfed3a7-a19a-4bb9-949e-d2eee0108d81";
	item["zone"] = "nova";
	item["type"] = "drt";
	item["flavorref"] = "10";
	item["as_name"] = sas;
	Json::Value timer;
	timer["holdtime"] = 30;
	timer["keeplive"] = 10;
	item["timer"] = timer;

	Json::Value networks;
	AsIps asipstemp = *(s.begin());
	string ip_network = tostringip(asipstemp.i1, asipstemp.i2,
			asipstemp.i3, 1);
	string cidr_network = tostringip(asipstemp.i1, asipstemp.i2,
			asipstemp.i3, 0) + "/24";
	networks["cidr"] = cidr_network;
	networks["ip"] = ip_network;

	item["networks"].append(networks);

	s.erase(asipstemp);
}

//from the two vectors of ip to set json neighbor
void set_json_neighbor_from_two_as(const vector<AsIps> &vupdata_source,
	   const vector<AsIps> &vneighbor_ip, Json::Value &neighbor, 
	   const AsConn &a, int &if_realconn,
	   multiset<string> &susedips, vector<AsInfo> &vAsInfo)
{
	int flag = 0;
	for(const auto& au: vupdata_source)
	{
		for(const auto& an: vneighbor_ip)
		{
			if(au == an)
			{
				flag = 1;
				if_realconn = 1;
				if(au.i4 != 0 && an.i4 != 0)  // already exist, before inserted 
				{
					//set json neighbor
					string anip = tostringip(an);
					string auip = tostringip(au);
					set_json_neighbor(neighbor, anip, 5, a.asneighbor, auip);
				}
				else if(au.i4 == 0 && an.i4 == 0)
				{
					int istartu = rand() % 150 + 2;
					int istartn = rand() % 150 + 2;
					string su = tostringip(au.i1, au.i2, au.i3, istartu);
					//generate json neighbor ip
					generate_neighbor_ip(su, susedips, au, istartu);
					
					susedips.insert(su);

					string sn = tostringip(an.i1, an.i2, an.i3, istartn);
					
					generate_neighbor_ip(sn, susedips, an, istartn);
					susedips.insert(sn);

					set_json_neighbor(neighbor, sn, 5, a.asneighbor, su);

					//update neighbor and update ip of the two as
					update_as_ip(a.as, vAsInfo, au, istartu);
					update_as_ip(a.asneighbor, vAsInfo, an, istartn);
				}
				else
				{
					if_realconn = 0;
				}
			}
			if(flag)break;
		}
		if(flag)break;
	}
}
//add a new connection from other ips
void add_extar_conn(set<AsIps> &sotherips, multiset<string> &susedips, 
		Json::Value &neighbor, 
		vector<AsInfo> &vAsInfo, const int asnum, const int asneighbor)
{
	AsIps au, an, aorign;
	au = an = aorign = *(sotherips.begin());
	int istartu = rand() % 150 + 2;
	int istartn = rand() % 150 + 2;
	string su, sn;
	su = tostringip(au.i1, au.i2, au.i3, istartu);
	generate_neighbor_ip(su, susedips, au, istartu);
	susedips.insert(su);

	sn = tostringip(an.i1, an.i2, an.i3, istartn);
	generate_neighbor_ip(sn, susedips, an, istartn);
	susedips.insert(sn);

	set_json_neighbor(neighbor, sn, 5, asneighbor, su);
	sotherips.erase(aorign);
	//insert the ip the neighbor as
	insert_as_ip(asneighbor, vAsInfo, an, istartn);
	insert_as_ip(asnum, vAsInfo, au, istartu);
}
//output json
void output_json(const std::string &filename,
		const set<AsConn> &sasconn,
		vector<AsInfo> &vAsInfo,
		set<AsIps> &sotherips,
		multiset<string> &susedips,
		set<int> & susedasnum)
{
	int count_edge = 0;
	//json
	Json::Value root;
	Json::Value server;

	//for(const auto& a: vasconn)
	for(auto a = sasconn.cbegin(); a != sasconn.cend(); ++a)
	{
		count_edge++;
		//the as is first accured
		if(susedasnum.find(a->as) == susedasnum.end())
		{
			//weather the two as is connceted 
			int if_realconn=0;
			Json::Value item;

			Json::Value neighbors;  //neighbors should append continusly
			Json::Value neighbor;
			vector<AsIps> vupdata_source, vneighbor_ip;
			
			get_as_ips(a->as, vupdata_source, vAsInfo);
			get_as_ips(a->asneighbor, vneighbor_ip, vAsInfo);
			//the two as's ip's size is not null
			if(vupdata_source.size() && vneighbor_ip.size())
			{
				//two for, think another better way
				//from the two vectors of ip to set json neighbor
				set_json_neighbor_from_two_as(vupdata_source, vneighbor_ip,
						neighbor, *a, if_realconn, susedips, vAsInfo);
				if(if_realconn)
				{
					neighbors.append(neighbor);
					item["neighbors"] = neighbors;
				}
			}
			// miss conneciton, make a new one
			if(!if_realconn)
			{
				//add a new connection from other ips
				add_extar_conn(sotherips, susedips, neighbor,
						vAsInfo, a->as, a->asneighbor);
				neighbors.append(neighbor);
				item["neighbors"] = neighbors;
			}
				
			//set basic json info
			set_basic_json(item, a->as, sotherips);

			server.append(item);  // added to json
			susedasnum.insert(a->as); //used as num
		}
		else
		{
			int if_realconn=0;

			int json_size = server.size();
			for(int i =0; i < json_size; i++)
			{
				Json::Value neighbor;
				
				string sastemp = tostring(a->as);
				if(server[i]["as_name"] == sastemp)
				{
					vector<AsIps> vupdata_source, vneighbor_ip;
					
					get_as_ips(a->as, vupdata_source, vAsInfo);
					get_as_ips(a->asneighbor, vneighbor_ip, vAsInfo);
					if(vupdata_source.size() && vneighbor_ip.size())
					{
						//two for, think another better way
						//from the two vectors of ip to set json neighbor
						set_json_neighbor_from_two_as(vupdata_source, 
								vneighbor_ip, neighbor,
								*a, if_realconn, susedips, vAsInfo);
						if(if_realconn)
						{
							server[i]["neighbors"].append(neighbor);
						}
						
					}
					//miss as conn, make a new one
					if(!if_realconn)
					{
						//add a new connection from other ips
						add_extar_conn(sotherips, susedips, neighbor,
								vAsInfo, a->as, a->asneighbor);
						server[i]["neighbors"].append(neighbor);
					}
					//end if : ==
					break;
				}
			}
		}
	}
	cout<<"size of all edge: "<<count_edge<<endl;

	root["server"] = server;
	//store json file
	string jsonout = root.toStyledString();
	ofstream fout(filename);
	fout<<jsonout;
	fout.close();
}

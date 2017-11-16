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
//set json neighbor
void set_json_neighbor(Json::Value & neighbor, const string &ni,
		const int ebgp, const int ras, const string &us)
{
	neighbor["neighbor_ip"] = ni;
	neighbor["ebgp_multihop"] = ebgp;
	neighbor["remote_as"] = ras;
	neighbor["update_source"] = us;
}

//realize class's method
//make sure only less than 2 same ip
void AS::less_than_2_same_ip()
{
	map<string, int> mip_count;
	for(auto& v: vAsInfo)
	{
		for(auto iter = v.vasIps.begin(); iter != v.vasIps.end();)
		{
			if(mip_count[tostringip(*iter)] == 2)
			{
//				cout<< tostringip(*iter) << endl;
				iter = v.vasIps.erase(iter);
			}
			else
			{
				++(mip_count[tostringip(*iter)]);
				++iter;
			}
		}
	}
}
//test set and vector, whether have same ip
void AS::handle_repeated_ip() 
{
	set<AsIps> smulti_ip;
	for(const auto& a: vAsInfo)
	{
		for(const auto& ip:a.vasIps)
		{
			if(sotherips.find(ip) != sotherips.end())
			{
				smulti_ip.insert(ip);
				//cout<<tostringip(ip)<<endl;
			}
		}
	}
	for(const auto& s: smulti_ip)
	{
		sotherips.erase(s);
	}
}


//init as connections
void AS::init_asconn(const string &f)
{
	ifstream fin(f);
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

//read all files in the as directory, init as's info
void AS::init_asinfo(const string &f)
{
	struct dirent *ptr;      
    DIR *dir;  
    dir = opendir(f.c_str());   
	vector<string> asfiles;  
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
		istringstream ss(a);
		ss >> ctemp >> ctemp >> ias;

		string asfilename = f + a;
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
}

//init other as's ips
void AS::init_other_ips(const string &f)
{
	ifstream fin_otherips(f);
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
void AS::get_as_ips(const int asnum, vector<AsIps> &vo)
{
	for(const auto& a: vAsInfo)
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
void AS::update_as_ip(const int asnum, const AsIps & ai, const int v)
{
	int flag = 0;
	for(auto& aas: vAsInfo)
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
void AS::generate_neighbor_ip(string &s, const AsIps &a, int &i4)
{
	while(susedips.find(s) != susedips.end())
	{
		i4 = rand() % 150 + 2;
		s = tostringip(a.i1, a.i2, a.i3, i4);
	}
}

//insert a as ip when they are connected but have no matching ips
void AS::insert_as_ip(const int asnum, const AsIps & a, const int v)
{
	int flag =0;
	for (int i =0; i< vAsInfo.size(); ++i)
	{
		if(vAsInfo[i].asnom == asnum)
		{
			AsIps asips1(a.i1, a.i2, a.i3, v);
			vAsInfo[i].vasIps.push_back(asips1);
			flag = 1;
			break;
		}	
	}
	if(!flag)
	{
		cout<<"insert new ip failed !!!"<<endl;
	}
}


//set basic json info
void AS::set_basic_json(Json::Value &item, const int asnum)
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
	AsIps asipstemp = *(sotherips.begin());
	string ip_network = tostringip(asipstemp.i1, asipstemp.i2,
			asipstemp.i3, 1);
	string cidr_network = tostringip(asipstemp.i1, asipstemp.i2,
			asipstemp.i3, 0) + "/24";
	networks["cidr"] = cidr_network;
	networks["ip"] = ip_network;

	item["networks"].append(networks);

	sotherips.erase(asipstemp);
}

//from the two vectors of ip to set json neighbor
void AS::set_json_neighbor_from_two_as(const vector<AsIps> &vupdata_source,
	   const vector<AsIps> &vneighbor_ip, Json::Value &neighbor, 
	   const AsConn &a, int &if_realconn)
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
					generate_neighbor_ip(su, au, istartu);
					
					susedips.insert(su);

					string sn = tostringip(an.i1, an.i2, an.i3, istartn);
					
					generate_neighbor_ip(sn, an, istartn);
					susedips.insert(sn);

					set_json_neighbor(neighbor, sn, 5, a.asneighbor, su);

					//update neighbor and update ip of the two as
					update_as_ip(a.as, au, istartu);
					update_as_ip(a.asneighbor, an, istartn);
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
void AS::add_extra_conn(Json::Value &neighbor, const int asnum, 
		const int asneighbor)
{
	AsIps au, an, aorign;
	au = an = aorign = *(sotherips.begin());
	int istartu = rand() % 150 + 2;
	int istartn = rand() % 150 + 2;
	string su, sn;
	su = tostringip(au.i1, au.i2, au.i3, istartu);
	generate_neighbor_ip(su, au, istartu);
	susedips.insert(su);

	sn = tostringip(an.i1, an.i2, an.i3, istartn);
	generate_neighbor_ip(sn, an, istartn);
	susedips.insert(sn);

	set_json_neighbor(neighbor, sn, 5, asneighbor, su);
	sotherips.erase(aorign);
	//insert the ip the neighbor as
	insert_as_ip(asneighbor, an, istartn);
	insert_as_ip(asnum, au, istartu);
}
//set_json_neighbor_changed_ip
void AS::set_json_neighbor_ip(const AsConn &a, Json::Value &neighbor)
{
	//weather the two as is connceted 
	int if_realconn=0;
	vector<AsIps> vupdata_source, vneighbor_ip;
	
	get_as_ips(a.as, vupdata_source);
	get_as_ips(a.asneighbor, vneighbor_ip);
	//the two as's ip's size is not null
	if(vupdata_source.size() && vneighbor_ip.size())
	{
		//two for, think another better way
		//from the two vectors of ip to set json neighbor
		set_json_neighbor_from_two_as(vupdata_source, vneighbor_ip,
				neighbor, a, if_realconn);
		
	}
	// miss conneciton, make a new one
	if(!if_realconn)
	{
		//add a new connection from other ips
		add_extra_conn(neighbor, a.as, a.asneighbor);
	}
}
//output json
void AS::output_json(const string &filename)
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
			Json::Value item;
			Json::Value neighbors;  //neighbors should append continusly
			Json::Value neighbor;
			//set_json_neighbor_changed_ip
			set_json_neighbor_ip(*a, neighbor);
			
			neighbors.append(neighbor);
			item["neighbors"] = neighbors;
				
			//set basic json info
			set_basic_json(item, a->as);

			server.append(item);  // added to json
			susedasnum.insert(a->as); //used as num
		}
		else
		{
			for(int i =0; i < server.size(); i++)
			{
				if(server[i]["as_name"] == tostring(a->as))
				{
					//set_json_neighbor_changed_ip
					Json::Value neighbor;
					set_json_neighbor_ip(*a, neighbor);

					server[i]["neighbors"].append(neighbor);
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


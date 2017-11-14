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
void get_as_ips(int asnum, vector<AsIps> & vo, const vector<AsInfo> & vi)
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
void update_as_ip(int asnum, vector<AsInfo> & a, const AsIps & ai, 
		const int v)
{
	for(auto& aas: a)
	{
		if(aas.asnom == asnum)
		{
			for(int ii =0; ii < aas.vasIps.size(); ++ii)
			{
				if (aas.vasIps[ii] == ai)
				{
					aas.vasIps[ii].i4 = v;
					break;
				}
			}
			break;
		}
	}	
}	
//generate neighbor ip
void generate_neighbor_ip(string &s, const set<string> &sa,
	   const AsIps &a, int &i4)
{
	while(sa.find(s) != sa.end())
	{
		i4 = rand() % 150;
		s = tostringip(a.i1, a.i2, a.i3, i4);
	}
}

//output json
void output_json(const std::string &filename,
		const set<AsConn> &sasconn,
		vector<AsInfo> &vAsInfo,
		set<AsIps> &sotherips,
		set<string> &susedips,
		set<int> & susedasnum)
{
	//json
	Json::Value root;
	Json::Value server;

	//for(const auto& a: vasconn)
	for(auto a = sasconn.cbegin(); a != sasconn.cend(); ++a)
	{

		if(susedasnum.find(a->as) == susedasnum.end())
		{
			int if_realconn=0;
			Json::Value item;

			Json::Value neighbors;  //neighbors should append continusly
			Json::Value neighbor;
			vector<AsIps> vupdata_source, vneighbor_ip;
			
			get_as_ips(a->as, vupdata_source, vAsInfo);
			get_as_ips(a->asneighbor, vneighbor_ip, vAsInfo);
			if(vupdata_source.size() && vneighbor_ip.size())
			{
				for(const auto& au: vupdata_source)
				{
					for(const auto& an: vneighbor_ip)
					{
						if(au == an)
						{
							if_realconn = 1;
							if(au.i4 != 0)  // already exist, before inserted 
							{
								neighbor["neighbor_ip"] = tostringip(an);
								neighbor["ebgp_multihop"] = 5;
								neighbor["remote_as"] = a->asneighbor;
								neighbor["update_source"] = tostringip(au);
								neighbors.append(neighbor);

								item["neighbors"] = neighbors;
							}
							else
							{
								int istartu = rand() % 150;
								int istartn = rand() % 150;
								string su = 
									tostringip(au.i1, au.i2, au.i3, istartu);
								//generate json neighbor ip
								generate_neighbor_ip(su, susedips, au, istartu);
								
								susedips.insert(su);

								istartn = rand() % 150;
								string sn = 
									tostringip(an.i1, an.i2, an.i3, istartn);
								
								generate_neighbor_ip(sn, susedips, an, istartn);
								susedips.insert(sn);

								neighbor["neighbor_ip"] = sn;
								neighbor["ebgp_multihop"] = 5;
								neighbor["remote_as"] = a->asneighbor;
								neighbor["update_source"] = su;
								neighbors.append(neighbor);

								item["neighbors"] = neighbors;

								//update neighbor and update ip of the two as
								update_as_ip(a->as, vAsInfo, au, istartu);
								update_as_ip(a->asneighbor, 
										vAsInfo, an, istartn);
							}
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
				int istartu = rand() % 150;
				int istartn = rand() % 150;
				string su, sn;
				su = tostringip(au.i1, au.i2, au.i3, istartu);
				generate_neighbor_ip(su, susedips, au, istartu);
				susedips.insert(su);

				istartn = rand() % 150;
				sn = tostringip(an.i1, an.i2, an.i3, istartn);
				generate_neighbor_ip(sn, susedips, an, istartn);
				susedips.insert(sn);

				neighbor["neighbor_ip"] = sn;
				neighbor["ebgp_multihop"] = 5;
				neighbor["remote_as"] = a->asneighbor;
				neighbor["update_source"] = su;
				neighbors.append(neighbor);

				item["neighbors"] = neighbors;

				//insert the ip the neighbor as
				sotherips.erase(aorign);
				for (int i =0; i< vAsInfo.size(); ++i)
				{
					if(vAsInfo[i].asnom == a->asneighbor)
					{
						AsIps asips1(an.i1, an.i2, an.i3, istartn);
						vAsInfo[i].vasIps.push_back(asips1);
						break;
					}	
				}
				for (int i =0; i< vAsInfo.size(); ++i)
				{
					if(vAsInfo[i].asnom == a->as)
					{
						AsIps asips1(au.i1, au.i2, au.i3, istartu);
						vAsInfo[i].vasIps.push_back(asips1);
						break;
					}	
				}
			}
				
			stringstream ss;
			string sas;
			ss << a->as;
			ss >> sas;
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
			string ip_network;
			string cidr_network;
			stringstream ss_ip;
			stringstream ss_cidr;
			ss_ip << asipstemp.i1 <<"."<<asipstemp.i2<<"."<<asipstemp.i3
				<<".1";
			ss_ip >> ip_network;
			ss_cidr << asipstemp.i1 <<"."<<asipstemp.i2<<"."<<asipstemp.i3
				<<".0/24";
			ss_cidr >> cidr_network;
			networks["cidr"] = cidr_network;
			networks["ip"] = ip_network;

			item["networks"].append(networks);
			//item["networks"] = networks;
			sotherips.erase(asipstemp);

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
				
				stringstream ssas;
				ssas << a->as;
				string sastemp;
				ssas >> sastemp;
				if(server[i]["as_name"] == sastemp)
				{
					vector<AsIps> vupdata_source, vneighbor_ip;
					
					get_as_ips(a->as, vupdata_source, vAsInfo);
					get_as_ips(a->asneighbor, vneighbor_ip, vAsInfo);
					if(vupdata_source.size() && vneighbor_ip.size())
					{
						for(const auto& au:vupdata_source)
						{
							for(const auto& an:vneighbor_ip)
							{
								if(au == an)
								{
									if_realconn=1;
									//already exist, before inserted 
									if(au.i4 != 0) 
									{
										cout << 
											" 418 wwwwwwwwwwwwwwwwwwrong!!!"
											<<endl;
										neighbor["neighbor_ip"] = tostringip(an);
										neighbor["ebgp_multihop"] = 5;
										neighbor["remote_as"] = a->asneighbor;
										neighbor["update_source"] = 
											tostringip(au);

										server[i]["neighbors"].append(neighbor);
									}
									else
									{
										string su, sn;
										int istartu = rand() % 150;
										int istartn = rand() % 150;
										su = tostringip(au.i1, 
												au.i2, au.i3, istartu);
										generate_neighbor_ip(su, susedips, 
												au, istartu);
										susedips.insert(su);

										istartn = rand() % 150;
										sn = tostringip(an.i1, 
												an.i2, an.i3, istartn);
										generate_neighbor_ip(sn, susedips, 
												an, istartn);
										susedips.insert(sn);

										neighbor["neighbor_ip"] = sn;
										neighbor["ebgp_multihop"] = 5;
										neighbor["remote_as"] = a->asneighbor;
										neighbor["update_source"] = su;

										server[i]["neighbors"].append(neighbor);
										update_as_ip(a->as, 
												vAsInfo, au, istartu);
										update_as_ip(a->asneighbor, 
												vAsInfo, an, istartn);
									}
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
						string su, sn;
						int istartu = rand() % 150;
						int istartn = rand() % 150;
						su = tostringip(au.i1, au.i2, au.i3, istartu);
						generate_neighbor_ip(su, susedips, au, istartu);
						susedips.insert(su);

						istartn = rand() % 150;
						sn = tostringip(an.i1, an.i2, an.i3, istartn);
						generate_neighbor_ip(sn, susedips, an, istartn);
						susedips.insert(sn);

						neighbor["neighbor_ip"] = sn;
						neighbor["ebgp_multihop"] = 5;
						neighbor["remote_as"] = a->asneighbor;
						neighbor["update_source"] = su;
						//neighbors.append(neighbor);

						//item["neighbors"] = neighbors;
						server[i]["neighbors"].append(neighbor);

						//insert the ip the neighbor as
						sotherips.erase(aorign);
						for (int i1 =0; i1< vAsInfo.size(); ++i1)
						{
							if(vAsInfo[i1].asnom == a->asneighbor)
							{
								AsIps asips1(an.i1, an.i2, an.i3, istartn);
								vAsInfo[i1].vasIps.push_back(asips1);
								break;
							}	
						}
						for (int i2 =0; i2< vAsInfo.size(); ++i2)
						{
							if(vAsInfo[i2].asnom == a->as)
							{
								AsIps asips1(au.i1, au.i2, au.i3, istartu);
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
	ofstream fout(filename);
	fout<<jsonout;
	fout.close();
}

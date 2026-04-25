#include<bits/stdc++.h>
using namespace std;

struct Node{

    int id;
    string name;
    int capacity;
    double x;
    double y;
    bool ghost;
};

class SmartParking{

public:

    map<int,Node> nodes;

    map<int,vector<pair<int,int>>> road;
    map<int,vector<pair<int,int>>> transport;

    map<int,map<int,int>> slots;

    map<pair<int,int>,vector<int>> visualPaths;

    SmartParking(){

        loadNodes();

        loadGraph();

        loadTransport();

        buildVisualPaths();
    }

    void loadNodes(){

        nodes.clear();

        ifstream file("nodes_list.txt");

        int id;
        string name;
        int cap;
        double x,y;
        int ghost;

        while(
            file >>
            id >>
            name >>
            cap >>
            x >>
            y >>
            ghost
        ){

            Node n;

            n.id=id;
            n.name=name;
            n.capacity=cap;
            n.x=x;
            n.y=y;
            n.ghost=ghost;

            nodes[id]=n;

            for(int h=0;h<24;h++){

                slots[id][h]=cap;
            }
        }

        file.close();
    }

    void loadGraph(){

        road.clear();

        ifstream file("adjacency_list.txt");

        int u,v,w;

        while(file>>u>>v>>w){

            road[u].push_back({v,w});
            road[v].push_back({u,w});
        }

        file.close();
    }

    void loadTransport(){

        transport.clear();

        ifstream file("transport.txt");

        int u,v;

        while(file>>u>>v){

            transport[u].push_back({v,1});
            transport[v].push_back({u,1});
        }

        file.close();
    }

    void buildVisualPaths(){

        visualPaths[{0,3}]={1,2};
        visualPaths[{3,6}]={4,5};
        visualPaths[{6,8}]={7};

        visualPaths[{0,15}]={9,10,11,13,14};

        visualPaths[{12,24}]={22,23};

        visualPaths[{24,0}]={25,26};

        visualPaths[{8,27}]={28,29};

        visualPaths[{0,30}]={31,32};

        visualPaths[{15,33}]={34,35};

        visualPaths[{33,36}]={37,38};

        visualPaths[{36,39}]={41,40};

        visualPaths[{33,42}]={43,44};

        visualPaths[{42,48}]={49,50};

        visualPaths[{8,51}]={52,53};

        visualPaths[{6,54}]={55};

        visualPaths[{3,56}]={57,58};

        visualPaths[{8,65}]={66,67};

        visualPaths[{0,62}]={63,64};

        visualPaths[{0,68}]={69,70};



        visualPaths[{3,0}]={2,1};

        visualPaths[{6,3}]={5,4};

        visualPaths[{8,6}]={7};

        visualPaths[{15,0}]={14,13,11,10,9};

        visualPaths[{24,12}]={23,22};

        visualPaths[{0,24}]={26,25};

        visualPaths[{27,8}]={29,28};

        visualPaths[{30,0}]={32,31};

        visualPaths[{33,15}]={35,34};

        visualPaths[{36,33}]={38,37};

        visualPaths[{39,36}]={40,41};

        visualPaths[{42,33}]={44,43};

        visualPaths[{48,42}]={50,49};

        visualPaths[{51,8}]={53,52};

        visualPaths[{54,6}]={55};

        visualPaths[{56,3}]={58,57};

        visualPaths[{65,8}]={67,66};

        visualPaths[{62,0}]={64,63};

        visualPaths[{68,0}]={70,69};
    }

    int getId(string name){

        for(auto &p:nodes){

            if(p.second.name==name){

                return p.first;
            }
        }

        return -1;
    }

    vector<int> dijkstra(
        int src,
        int dest,
        map<int,vector<pair<int,int>>>& graph
    ){

        priority_queue<
            pair<int,int>,
            vector<pair<int,int>>,
            greater<pair<int,int>>
        > pq;

        map<int,int> dist;
        map<int,int> parent;

        for(auto &p:nodes){

            dist[p.first]=1e9;
        }

        dist[src]=0;

        pq.push({0,src});

        while(!pq.empty()){

            auto top=pq.top();
            pq.pop();

            int node=top.second;

            for(auto &edge:graph[node]){

                int next=edge.first;
                int wt=edge.second;

                if(
                    dist[node]+wt
                    <
                    dist[next]
                ){

                    dist[next]=dist[node]+wt;

                    parent[next]=node;

                    pq.push({
                        dist[next],
                        next
                    });
                }
            }
        }

        vector<int> path;

        if(dist[dest]==1e9){

            return path;
        }

        int cur=dest;

        while(cur!=src){

            path.push_back(cur);

            cur=parent[cur];
        }

        path.push_back(src);

        reverse(
            path.begin(),
            path.end()
        );

        return path;
    }

    vector<int> buildVisualRoute(
        vector<int>& realPath
    ){

        vector<int> finalPath;

        if(realPath.empty()){

            return finalPath;
        }

        for(
            int i=0;
            i<realPath.size()-1;
            i++
        ){

            int a=realPath[i];
            int b=realPath[i+1];

            finalPath.push_back(a);

            pair<int,int> key={a,b};

            if(visualPaths.count(key)){

                for(int g:visualPaths[key]){

                    finalPath.push_back(g);
                }
            }
        }

        finalPath.push_back(
            realPath.back()
        );

        return finalPath;
    }

    bool availableAtTime(
        int node,
        int startH,
        int endH
    ){

        for(
            int h=startH;
            h<endH;
            h++
        ){

            if(slots[node][h]<=0){

                return false;
            }
        }

        return true;
    }

    void reserveSlot(
        int node,
        int startH,
        int endH
    ){

        for(
            int h=startH;
            h<endH;
            h++
        ){

            slots[node][h]--;
        }
    }

    int fallbackParking(
        vector<int>& path,
        int startH,
        int endH
    ){

        for(int id:path){

            if(nodes[id].ghost){

                continue;
            }

            if(
                availableAtTime(
                    id,
                    startH,
                    endH
                )
            ){

                return id;
            }
        }

        return -1;
    }

    void updateCapacity(
        int id,
        int cap
    ){

        if(!nodes.count(id)){

            return;
        }

        nodes[id].capacity=cap;

        for(int h=0;h<24;h++){

            slots[id][h]=cap;
        }

        saveNodes();
    }

    void saveNodes(){

        ofstream file(
            "nodes_list.txt"
        );

        for(auto &p:nodes){

            auto n=p.second;

            file
            <<n.id<<" "
            <<n.name<<" "
            <<n.capacity<<" "
            <<n.x<<" "
            <<n.y<<" "
            <<n.ghost
            <<endl;
        }

        file.close();
    }
};

SmartParking sys;
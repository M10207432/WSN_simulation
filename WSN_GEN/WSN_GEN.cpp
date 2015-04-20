#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include <iostream>
#include<fstream>
#include <string.h>
#include <string>
using namespace std;

/*=================================
		Structure
==================================*/
struct Node{
	double coor_x,coor_y;//座標
	double distanceto_BS;//到Base station 距離
	double energy;
	double radus;
	short int hop;		//range 1~3

	struct Packet* pkt;
	struct Node* nextnd;
	struct Node* prend;
};
struct Packet{
	int id;
	int load;
	double period;
	double utilization;
	int	 pirority;

	short int hop;		//range 1~3
	int destination;	//Nest node
	double rate;

	struct Packet* nextpkt;
	struct Packet* prepkt;
};
void create();
/*=================================
		Global value
==================================*/
short int Max_X_Axis = 100;
short int Max_Y_Axis = 100;
int Level1_Nodenum = 2;
int Level2_Nodenum = 1;
int pktnum=10;
int nodenum=Level1_Nodenum+Level2_Nodenum;

const double leastperiod=100;
const double largestperiod=1000;
const double Hyperperiod=1000;
const float MIN_Uti=1.0;
const float MAX_Uti=5.0;
const short int Set=50;

string GENfile="..\\GENresult\\";//放到前一目錄下的GENresult目錄，產生txt檔
char Resultfile[]="..\\GENresult\\WSNGEN.txt";//放到前一目錄下的GENresult目錄，產生txt檔
float Total_Uti=1.0;
const double Max_pktuti=0.9;
double tmp_totaluti=0;
int Gobackcount=0;
Node* HEAD=new Node;			//總頭HEAD
Node* node=new Node;
Packet* packet=new Packet;

int main(void){
	
	srand(time(NULL));			//隨機種子
	
	for(float U=MIN_Uti;U<=MAX_Uti;U++){
		Total_Uti=U;

		//放入GEN的檔名
		string filename="U";
		filename.append(to_string(Total_Uti));
		filename.append("_Set");
		filename.append(to_string(Set));
		filename.append(".txt");
		cout<<filename<<endl;

		//放入GEN的 路徑+檔名
		string GENBuffer=GENfile;
		GENBuffer.append(filename);

		char *GENbuffer=(char*)GENBuffer.c_str();
	
		fstream fp;
		fp.open(GENbuffer, ios::out);	//開啟檔案.寫入狀態
		if(!fp){//如果開啟檔案失敗，fp為0；成功，fp為非0
			cout<<"Fail to open file: "<<GENbuffer<<endl;
		}

		/*==================================================
						GEN U 的 Set數
		==================================================*/
		for(int setcount=0;setcount<Set;setcount++){

			/*==================================================
							Create Node & Packet
			==================================================*/
			create();

			/*==================================================
							寫入資訊 TXT
			==================================================*/
			double tmpu=0;
			Packet* tmppkt=new Packet;
			Node* tmpnode=new Node;
			tmpnode=HEAD->nextnd;
			int nodeid=1;
			int packetid=1;
			fp<<nodenum<<" "<<pktnum<<" "<<Hyperperiod<<endl;
			while(tmpnode!=NULL){
				tmppkt=tmpnode->pkt;

				fp<<"Node"<<" ";
				fp<<tmpnode->coor_x<<" "<<tmpnode->coor_y<<" "<<tmpnode->radus<<endl;
					
				while(tmppkt!=NULL){
					fp<<"Pkt"<<" ";
					fp<<tmppkt->load<<" ";
					fp<<tmppkt->period<<" ";
					fp<<tmppkt->utilization<<" ";
					fp<<tmpnode->hop<<endl;//fp<<tmppkt->hop<<endl;

					cout<<tmppkt->load<<endl;
					cout<<tmppkt->period<<endl;
					tmpu=tmpu+tmppkt->utilization;

					tmppkt=tmppkt->nextpkt;
				}
				packetid=1;
				if(tmpnode!=NULL){
					tmpnode=tmpnode->nextnd;
				}
			}
			cout<<"Totaluti:"<<tmp_totaluti<<endl;
			fp<<"Totaluti:"<<tmp_totaluti<<endl;
			cout<<"=========="<<endl;
			fp<<"=========="<<endl;
		}//============================================Set end

		fp.close();
		filename.clear();
		
	}
	system("PAUSE");
	return 0;
}

void create(){
	bool Done_flag=false;
	delete HEAD;delete node;delete packet;
	HEAD=new Node();
	node=new Node();
	packet=new Packet();

	/*==================================================
					建立Link list
					Node & Packet
	==================================================*/
	/*-------------------------
		Gen node(Linklist)
	-------------------------*/
	
	HEAD->nextnd=node;
	node->prend=HEAD;
	for(int n=0;n<nodenum;n++){
		/*-------------------------
			Gen packet(Linklist)
		-------------------------*/
		packet=new Packet;
		node->pkt=packet;
		packet->prepkt=NULL;
		for (int p=0;p<pktnum;p++){

			Packet* nextpacket=new Packet;
			Packet* prepacket=packet;
			packet->nextpkt=nextpacket;
			packet=nextpacket;
			packet->prepkt=prepacket;
			
		}

		packet=packet->prepkt;
		packet->nextpkt=NULL;

		//--------------------------Packet Done

		Node* nextnode=new Node;
		Node* prenode=node;
		node->nextnd=nextnode;
		node=nextnode;
		node->prend=prenode;
	}
	node=node->prend;
	node->nextnd=NULL;
	//-----------------------------Node Done

	while(Done_flag!=true){
		/*==================================================
						分配各封包Period
						要符合Hyperperiod
		==================================================*/
		tmp_totaluti=0;
		node=HEAD->nextnd;
		double tmp_packetperiod=0;//要rand的封包period

		while(node!=NULL){
			packet=node->pkt;
			while(packet!=NULL){
			
				int periodrange=(largestperiod-leastperiod);	//要Rand的範圍
				tmp_packetperiod=rand()%periodrange+leastperiod;							//Rand出packet的period位於 largestperiod~leastperiod
				while((int(Hyperperiod)%int(tmp_packetperiod))!=0){
					tmp_packetperiod=rand()%periodrange+leastperiod;						//Rand出packet的period位於 largestperiod~leastperiod
				}
				packet->period=double(tmp_packetperiod);										//放入利用率

				packet=packet->nextpkt;
			}
			node=node->nextnd;
		}
		/*==================================================
						分配各封包利用率
		==================================================*/
		node=HEAD->nextnd;
		double avg_packetuti=Total_Uti/(nodenum*pktnum);
		double tmp_pktuti=0.0;
		double totalpacket=nodenum*pktnum;
		int rangetoavg_pktuti=nodenum*pktnum*avg_packetuti;	//Gen的範圍 0~avg_packetuti
		int rangetomax_pktuti=nodenum*pktnum*Max_pktuti;	//Gen的範圍 0~Max_pktuti
		double tmpcount=1;
		bool averageuti_flag=false;
	
		while(node!=NULL){
			packet=node->pkt;
			while(packet!=NULL){
			
				/*-------------------
					Rand Uti
				-------------------*/
				if(tmpcount*avg_packetuti < tmp_totaluti){
					if(rangetoavg_pktuti!=0)
						tmp_pktuti=((rand()%rangetoavg_pktuti)+1)/totalpacket;//0~avg_packetuti
					else
						tmp_pktuti=((rand()%1)+1)/totalpacket;
					averageuti_flag=false;
				}
				else{
					averageuti_flag=true;
					tmp_pktuti=((rand()%rangetomax_pktuti)+1)/totalpacket;//0~Max_pktuti
				}

				//開啟平均利用率計算
				if(averageuti_flag==true){
					if(((nodenum*pktnum)-tmpcount)!=0){
						avg_packetuti=(Total_Uti-tmp_totaluti)/((nodenum*pktnum)-tmpcount);
						rangetoavg_pktuti=nodenum*pktnum*avg_packetuti;
					}
				}

				//放入packet uti中
				tmpcount++;
				tmp_totaluti=tmp_totaluti+tmp_pktuti;
				packet->utilization=tmp_pktuti;
				packet=packet->nextpkt;
			
			}
			node=node->nextnd;
		}
		/*==================================================
						滿足整體總利用率
		==================================================*/
		//-------------------------------------利用率不到
		node=HEAD->nextnd;
		packet=node->pkt;
		if(tmp_totaluti<Total_Uti){
			double remainuti=Total_Uti-tmp_totaluti;
		
			//變換pkt利用率
			while(packet!=NULL){
				if(packet->utilization+remainuti>1)
					packet=packet->nextpkt;
				else{
					packet->utilization=packet->utilization+remainuti;
					tmp_totaluti=tmp_totaluti+remainuti;
					break;
				}
			}
		}
		//-------------------------------------利用率超過
		node=HEAD->nextnd;
		packet=node->pkt;
		if(tmp_totaluti>Total_Uti){
			int adjustnum=0;
			double adjuti=0;
			double remainuti=tmp_totaluti-Total_Uti;
		
			//計算需變換的pkt數量
			while(remainuti>=0){
				adjustnum++;
				remainuti=remainuti-packet->utilization;
				tmp_totaluti=tmp_totaluti-packet->utilization;
				packet->utilization=0;
				packet=packet->nextpkt;
				if(packet==NULL){
					node=node->nextnd;
					packet=node->pkt;
				}
			}
			//計算平均利用率
			adjuti=(Total_Uti-tmp_totaluti)/(adjustnum);
			node=HEAD->nextnd;
			packet=node->pkt;
			while(packet->utilization==0){
				tmp_totaluti=tmp_totaluti+adjuti;
				packet->utilization=adjuti;
				packet=packet->nextpkt;
				if(packet==NULL){
					node=node->nextnd;
					packet=node->pkt;
				}
			}
		}
		/*==================================================
						依照Packet中的
					Utilization & Period去算出load
				  (Done_flag會判定是否有load為0的情況)
		==================================================*/
		Done_flag=true;
		node=HEAD->nextnd;
		while(node!=NULL){
			packet=node->pkt;
			while(packet!=NULL){
				packet->load=packet->utilization * packet->period;
				if(packet->load==0)
					Done_flag=false;
				if(tmp_totaluti!=Total_Uti)
					Done_flag=false;
				packet=packet->nextpkt;
			}
			node=node->nextnd;
		}
	}

	/*==================================================
					分配各節點位置
	==================================================*/
	node=HEAD->nextnd;
	int tmplevel1=Level1_Nodenum;
	int tmplevel2=Level2_Nodenum;
	double ceter_x=Max_X_Axis/2;
	double ceter_y=Max_Y_Axis/2;
	double R=-1;

	while(node!=NULL){
		if(tmplevel1){
			R=-1;
			while(!(R<=Max_X_Axis/2 && R>0)){
				node->coor_x=(rand()%Max_X_Axis/2)+Max_X_Axis/4; 
				node->coor_y=(rand()%Max_Y_Axis/2)+Max_Y_Axis/4;
				
				R=sqrt(pow((ceter_x-node->coor_x),2)+pow((ceter_y-node->coor_y),2));
			}
			node->radus=R;
			node->hop=1;

			tmplevel1--;
		}else if(tmplevel2){
			R=-1;
			while(!(R>Max_X_Axis/2)){
				node->coor_x=(rand()%Max_X_Axis);
				node->coor_y=(rand()%Max_Y_Axis);

				R=sqrt(pow((ceter_x-node->coor_x),2)+pow((ceter_y-node->coor_y),2));
			}
			node->radus=R;
			node->hop=2;

			tmplevel2--;
		}

		cout<<"Node Address: "<<node->coor_x<<" "<<node->coor_y<<endl;
		node=node->nextnd;
	}
}
#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include <iostream>
#include<fstream>
#include <string.h>
#include <string>
#define transmission_time 10
#define payload 20

using namespace std;

/*=================================
		Structure
==================================*/
struct Node{
	double coor_x,coor_y;//座標
	double distanceto_BS;//到Base station 距離
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
	double time;

	short int hop;		//range 1~3

	struct Packet* set_nextpkt;
	struct Packet* nextpkt;
	struct Packet* prepkt;
};
void create();
void create_varied();

/*=================================
		Global value
==================================*/
const short int Max_X_Axis = 100;	//最大X軸範圍
const short int Max_Y_Axis = 100;	//最大Y軸範圍
const int Level1_Nodenum = 3;		//第一層Node數量<ConnNode>
const int Level2_Nodenum = 0;		//第二層Node數量<AdvNode>
const int pktnum=3;				//每個node上的封包數
const double leastperiod=80;	//period最小值
const double largestperiod=5000;//period最大值
const double Hyperperiod=10000;	
const float MIN_Uti=1.0;		//GEN 利用率的起點
const float MAX_Uti=5.0;		//GEN 利用率的終點
const float U_interval=1;		//利用率間距
const short int Set=100;			//每一利用的Set數
const bool varied_f=true;		//是否要各node的period差距較大
string GENfile="..\\GENresult\\input_varied\\";//放到前一目錄下的GENresult目錄，產生txt檔
char Resultfile[]="..\\GENresult\\WSNGEN.txt";//放到前一目錄下的GENresult目錄，產生txt檔

int nodenum=Level1_Nodenum;// without Level2_Nodenum
float Total_Uti=1.0;
const double Max_pktuti=0.9;
double tmp_totaluti=0;
int Gobackcount=0;

Node* HEAD=new Node;			//總頭HEAD
Node* node=new Node;
Packet* packet=new Packet;

int main(void){
	
	srand(time(NULL));			//隨機種子
	
	for(float U=MIN_Uti;U<=MAX_Uti;U=U+U_interval){
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
			if(varied_f){
				create_varied();
			}
			/*==================================================
							寫入資訊 TXT
			==================================================*/
			double tmpu=0;
			tmp_totaluti=0;
			Packet* tmppkt=new Packet;
			Node* tmpnode=new Node;
			tmpnode=HEAD->nextnd;

			fp<<Level1_Nodenum<<" "<<Level2_Nodenum<<" "<<pktnum<<" "<<Hyperperiod<<endl;
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
					tmp_totaluti=tmp_totaluti+(tmppkt->time/tmppkt->period);
					
					tmppkt=tmppkt->nextpkt;
				}
				if(tmpnode!=NULL){
					tmpnode=tmpnode->nextnd;
				}
			}

			cout<<"Totaluti:"<<tmp_totaluti<<endl;
			fp<<"Totaluti:"<<tmp_totaluti<<endl;
			cout<<"=========="<<setcount<<endl;
			fp<<"=========="<<endl;
			fp<<"=========="<<setcount<<endl;
		}//============================================Set end

		fp.close();
		filename.clear();
		
	}
	system("PAUSE");
	return 0;
}

void create(){
	bool Done_flag=false;					//GEN結束
	delete HEAD;delete node;delete packet;
	HEAD=new Node();
	node=new Node();
	packet=new Packet();

	/*==================================================
					建立Link list
					Node & Packet
				分配ConnNode的廣播封包
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
			packet->set_nextpkt=nextpacket;
			packet=nextpacket;
			packet->prepkt=prepacket;
			
		}

		packet=packet->prepkt;
		packet->nextpkt=NULL;
		packet->set_nextpkt=NULL;
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

	Packet* tmppkt=NULL;
	for(Node* node=HEAD->nextnd; node!=NULL; node=node->nextnd){
		for(Packet* pkt=node->pkt; pkt!=NULL; pkt=pkt->nextpkt){
			tmppkt=pkt;
		}
		if(node->nextnd!=NULL){
			tmppkt->set_nextpkt=node->nextnd->pkt;
		}
	}

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
				
				packet->time=(packet->utilization * packet->period);
				packet->time=(int(packet->time)/10)*10;
				packet->load=(packet->time/transmission_time)*payload;
				packet->utilization=packet->time/packet->period;

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
					分配AdvNode的廣播封包
	==================================================*/
	//===================================================Link list
	int countLevel2=Level2_Nodenum;
	node=HEAD->nextnd;
	while(node->nextnd!=NULL){
		node=node->nextnd;
	}
	while(countLevel2>0){
		Node *nextnode=new Node;
		Node *prenode=node;

		node->nextnd=nextnode;
		node=nextnode;
		node->prend=prenode;
		node->nextnd=NULL;

		//只能有一個封包, 只能有20byte(傳輸時間為10ms) period先設為最大
		node->pkt=new Packet;
		
		node->hop=2;
		packet=node->pkt;
		packet->period=largestperiod;
		packet->time=transmission_time;
		packet->load=(packet->time/transmission_time)*payload;
		packet->utilization=packet->time/packet->period;

		node->pkt->nextpkt=NULL;
		node->pkt->prepkt=NULL;

		//node count 減一
		countLevel2--;
	}
	/*
	//===================================================Check 是否符合利用率,不合則修改AdvNode的packet
	node=HEAD->nextnd;
	double tmp_u=0;
	while(node!=NULL){
		packet=node->pkt;
		while(packet!=NULL){
			tmp_u=tmp_u+packet->utilization;
			packet=packet->nextpkt;
		}
		node=node->nextnd;
	}

	node=HEAD->nextnd;
	if(tmp_u<Total_Uti){
		while(node!=NULL){
			//修改第二層AdvNode
			if(node->hop==2){
				tmp_u=tmp_u-node->pkt->utilization;//先減掉原本U

				double tmp_period=node->pkt->period;
				tmp_period--;
				while(int(Hyperperiod) % int(tmp_period)!=0){
					if(tmp_period>leastperiod)
						tmp_period--;
				}
				
				node->pkt->period=tmp_period;
				node->pkt->utilization=node->pkt->time/node->pkt->period;
				tmp_u=tmp_u+node->pkt->utilization;
			}
			if(tmp_u>Total_Uti)
				break;

			node=node->nextnd;
		}
	}
	*/
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

void create_varied(){

	for(Packet* pkt=HEAD->nextnd->pkt; pkt!=NULL; pkt=pkt->set_nextpkt){
		printf("Pkt%d, load=%d, period=%lf\n",pkt->id, pkt->load, pkt->period);
	}

	//selection sort
	Node* node=HEAD;

	Packet* cmppkt=node->nextnd->pkt; 
	while(cmppkt!=NULL){
		Packet *exchangepkt=NULL;
		
		for(Packet* pkt=cmppkt->set_nextpkt; pkt!=NULL; pkt=pkt->set_nextpkt){
			if(pkt->period < cmppkt->period){
				if(exchangepkt==NULL){
					exchangepkt=pkt;
				}else if(pkt->period <exchangepkt->period){
					exchangepkt=pkt;
				}
			}
		}

		if(exchangepkt!=NULL){
			exchangepkt->load=cmppkt->load^exchangepkt->load;
			cmppkt->load=cmppkt->load^exchangepkt->load;
			exchangepkt->load=cmppkt->load^exchangepkt->load;

			double tmp=cmppkt->period;
			cmppkt->period=exchangepkt->period;
			exchangepkt->period=tmp;

			tmp=cmppkt->time;
			cmppkt->time=exchangepkt->time;
			exchangepkt->time=tmp;

			tmp=cmppkt->utilization;
			cmppkt->utilization=exchangepkt->utilization;
			exchangepkt->utilization=tmp;
		}
		cmppkt=cmppkt->set_nextpkt;
	}

	cout<<"==========================="<<endl;

	//adjust period
	for(Node* n=HEAD->nextnd; n!=NULL; n=n->nextnd){
		//find small period for each node
		double smallperiod=0;
		for(Packet* pkt=n->pkt; pkt!=NULL; pkt=pkt->nextpkt){
			if(smallperiod==0 || pkt->period<smallperiod){
				smallperiod=pkt->period;
			}
		}
		//
		for(Packet* pkt=n->pkt; pkt!=NULL; pkt=pkt->nextpkt){
			pkt->load=payload;
			if(pkt->period>smallperiod && (pkt->period-smallperiod)>50){
				pkt->period=smallperiod;
				/*
				pkt->time=(pkt->utilization * pkt->period);
				pkt->time=(int(pkt->time)/10)*10;
				pkt->load=(pkt->time/transmission_time)*payload;
				*/
				pkt->load=payload;
			}
		}
	}
	for(Packet* pkt=HEAD->nextnd->pkt; pkt!=NULL; pkt=pkt->set_nextpkt){

		printf("Pkt%d, load=%d, period=%lf\n",pkt->id, pkt->load, pkt->period);
	}
	int y=0;
}
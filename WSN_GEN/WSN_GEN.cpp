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

	double period;	// At least period
	double rate;		// bytes/s

	struct Packet* pkt;
	struct Node* nextnd;
	struct Node* prend;
};
struct Packet{
	int id;

	double load;
	double period;
	double utilization;
	double time;

	short int hop;		//range 1~3

	struct Packet* set_nextpkt;
	struct Packet* nextpkt;
	struct Packet* prepkt;
};

/*=================================
		Function
==================================*/
void NodeStruct();			//建立node的節點結構與packet結構

void create();
void create_varied(double);	//Gen出Rand period (各node含有相同rate, pkt分配的rate相同)
void NodeLocation();		//分配節點位置

void CheckSetting();		//確認分配的period是否正確
/*=================================
		Gen Setting
==================================*/
double period[]={200,400,800};		
double Hyperperiod=24000;
const int Level1_Nodenum = 3;		//第一層Node數量<ConnNode>
const int Level2_Nodenum = 0;		//第二層Node數量<AdvNode>
const int pktnum=2;					//每個node上的封包數
const short int Set=100;			//每一利用的Set數
double Initrate=40;					//開始GEN的rate
double inv_r=40;					//rate差距
double Maxrate=960;					//最終 rate
string GENfile="..\\GENresult\\input_varied_node3\\";//放到前一目錄下的GENresult目錄，產生txt檔
char Resultfile[]="..\\GENresult\\WSNGEN.txt";//放到前一目錄下的GENresult目錄，產生txt檔
double periodrange=200;					//各個period上的差距 rand時的差距

const short int Max_X_Axis = 100;	//最大X軸範圍
const short int Max_Y_Axis = 100;	//最大Y軸範圍
/*=================================
		Ex setting
==================================*/
const double leastperiod=80;	//period最小值
const double largestperiod=5000;//period最大值
const float MIN_Uti=1.0;		//GEN 利用率的起點
const float MAX_Uti=5.0;		//GEN 利用率的終點
const float U_interval=1;		//利用率間距
const bool varied_f=true;		//是否要各node的period差距較大
int nodenum=Level1_Nodenum;// without Level2_Nodenum
float Total_Uti=1.0;
const double Max_pktuti=0.9;
double tmp_totaluti=0;
int Gobackcount=0;
	
/*=================================
=================================*/
Node* HEAD=new Node;			//總頭HEAD
Node* node=new Node;
Packet* packet=new Packet;

int main(void){
	
	CheckSetting();				//判斷period是否設定正確
	srand(time(NULL));			//隨機種子

	for(double rate=Initrate; rate<=Maxrate; rate+=inv_r){
		Total_Uti=rate;

		//放入GEN的檔名
		string filename="Rate";
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

			NodeStruct();			//建立node與packet 結構
			
			//create();
			create_varied(rate);	//Rand gen出varied period (含有相同data rate)

			NodeLocation();			//各個node的address

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
					tmp_totaluti=tmp_totaluti+(tmppkt->load/tmppkt->period);
					
					tmppkt=tmppkt->nextpkt;
				}
				if(tmpnode!=NULL){
					tmpnode=tmpnode->nextnd;
				}
			}

			cout<<"Rate:"<<(tmp_totaluti/nodenum)*1000<<endl;
			fp<<"Rate:"<<(tmp_totaluti/nodenum)*1000<<endl;
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
	
	
}

void create_varied(double rate){
	//==========================================================setting 
	int i=0;
	for(Node* n=HEAD->nextnd; n!=NULL; n=n->nextnd){

		n->period=ceil(period[i]);
		n->rate=rate*(period[i]/1000);	//單位為ms

		//各個node的period範圍不同，藉由i來做切換
		i++;
		if(i>=sizeof(period)/sizeof(period[0])){
			i=0;
		}
		//=========================================================splite to pkt
		for(Packet* pkt=n->pkt; pkt!=NULL; pkt=pkt->nextpkt){
			pkt->utilization=ceil(n->rate/pktnum);	//平均分配各個node中pkt的rate
			
			//開始rand 不同period給各個pkt
			double p=0;
			do{
				if(Level1_Nodenum==1){
					int range=period[sizeof(period)/sizeof(period[0])-1]-period[0]; //range為最小到最大period
					p=rand()%range+period[0];										//單一node時range為period[0]-period[MAX]
				}else{
					p=(rand()%(int)periodrange)+(n->period-periodrange/2);			//period前後periodrange一半
				}
				
			}while((int)Hyperperiod%(int)p!=0);

			//計算load與assign period (重點為period變化)
			if(p!=Hyperperiod){
				pkt->load=ceil(pkt->utilization*(p/n->period));	//計算load
				pkt->period=p;
			}else{
				pkt->period=n->period;
			}

			pkt->utilization=pkt->load/pkt->period;
		}
	}
}

/*================================
		建立Node Packet結構
================================*/
void NodeStruct(){
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

}

/*==================================================
				分配各節點位置
==================================================*/
void NodeLocation(){
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

void CheckSetting(){
	
	for(int i=0; i<sizeof(period)/sizeof(period[0]); i++){
		int min_p=period[i]-periodrange/2;
		int max_p=period[i]+periodrange/2;
		int p=period[i]-periodrange/2;

		//找範圍內的數值 p
		do{
			p++;
		}while((int)Hyperperiod%p!=0);

		//判斷是否在範圍內
		if(p>=min_p && p<=max_p){
			printf("With %lf is OK\n",period[i]);
		}else{
			printf("With %lf is Not suitable\n",period[i]);
			system("PAUSE");
		}
	}
}

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <limits>
#include <math.h>
#include <map> 
#include <memory>

using namespace std;
/*=================================
		Structure
==================================*/

struct Flow{
	struct Packet* pkt;
	struct Node* sourcenode;
	struct Node* destinationnode;
};

struct PacketBuffer{
	double pktsize;	//1~6 packets
	int load;		//0~120bytes
	Packet* pkt;
};
struct Node{
	double coor_x,coor_y;//座標
	double radius;
	double distanceto_BS;//到Base station 距離
	double energy;
	double eventinterval;
	int id;
	short int ExTimeslot;
	short int LatestTimeslot;
	short int hop;		//range 1~3

	string State;		//Wakeup、Sleep、Transmission & Idle
	Packet* pkt;
	Packet* pktQueue;
	Node* nextnd;
	Node* prend;
	Node* SendNode;//傳送節點
	Node* RecvNode;//接收節點

	PacketBuffer* NodeBuffer;
};
struct Packet{
	int id;
	int nodeid;
	int load;
	int exeload;
	double arrival;
	double period;
	double deadline;
	double utilization;
	int	 pirority;
	int	 doneflag;
	int	 readyflag;
	int	 searchdone;
	string State;		//Transmission 、 Idle

	short int hop;		//range 1~3
	short int exehop;		//range 1~3
	int destination;	//Nest node
	double rate;

	Node* node;

	Packet* nextpkt;		//Global packet link
	Packet* prepkt;			//Global packet link
	Packet* nodenextpkt;	//Local node packet link
	Packet* nodeprepkt;		//Local node packet link
	
	Packet* readynextpkt;	//Global ready packet link
	Packet* readyprepkt;	//Global ready packet link
	Packet* nodereadynextpkt;	//local node ready packet link
	Packet* nodereadyprepkt;	//local node ready packet link

	Packet* buffernextpkt;
};
struct DIFtable{
	double load;
	double length;
	double density;
	double arrival;
	double deadline;
};

void StructGEN();				
void PacketQueue();
void BufferSet();
void FlowEDF();
void AvgTransmissionRate();
void AssignRate();
void NodeEnergy();
double IntervalPower(int,int);
void NodeState();				//改變個Node狀態
void Topology();

void DIF();						//Densest Interval First比較組，preemptionenable 要enable
void TSB();						//Total Size Block
void Rate_TO_Interval(int);		//Nonpreemption 方法
/*=================================
		Global value
==================================*/
const float MIN_Uti=1.0;
const float MAX_Uti=5.0;
const short int Set=50;
string GENPath="..\\GENresult\\";
string SchedulePath="..\\WSNresult\\TSBResult_ver2\\";
string PowerPath="..\\WSNresult\\TSBResult_ver2\\";
string ResultPath="..\\WSNresult\\TSBResult_ver2\\";
short int Rateproposal=1;				//AssignRate()中的方法編號 0=>Event, 1=>TSB, 2=>DIF, 
bool preemptionenable=true;			//設定可否preemption

int Flowinterval=0;					//觸發進入flow的conneciton interval
int Pktsize;							//計算IntervalPower的pkt num
double DIFMinperiod=0;
double Meetcount=0;
double AverageE=0;
string filename;
fstream GENfile;
fstream Schdulefile;
fstream Powerfile;
fstream Resultfile;

PacketBuffer* Buffer=new PacketBuffer;
Node* Head=new Node;
Packet* Headpacket=new Packet;
Node* node=new Node;
Packet* packet=new Packet;
Packet *ReadyQ=new Packet();
Flow *Headflow=new Flow();
int ReadyQ_overflag=0;

stringstream stream;
string str_coor_x,str_coor_y,str_radius;
string strload,strperiod,strutilization,strhop;
int nodenum=0;
int pktnum=0;
long int Timeslot=0;
long int Hyperperiod=0;
double Maxrate=20;					//最高速度為20bytes/slot
double payload=20;					//payload 為 20bytes
int Maxbuffersize=6;				//Maxbuffersize 為 6個packets
double slotinterval=1;				//Time slot間距為10ms
double Connectioninterval=0;		//Conneciton inteval 只會在10ms~4000ms
double totalevent=0;				//Event數量
bool Meetflag=true;					//看是否meet deadline
/*====================
		Power function parameter
====================*/
double Vcc=3.3;			//BLE 驅動電壓
double Isleep=0.003;	//Sleep 電流 
double Ie=0.07679763;		//傳輸峰值 電流  0.002
double Te=0.0002768;		//傳輸時間1.25ms
double K=1;				//Rate power常數
double unit=0.01;		//時間單位為10ms
double TotalEnergy=0;
double parma=0.00191571992;
double parmb=24.4058498;

int main(){

	for(float U=MIN_Uti; U<=MAX_Uti; U++){
		Meetcount=0;
		AverageE=0;
		totalevent=0;

		//放入GEN的檔名
		filename="U";filename.append(to_string(U));filename.append("_Set");filename.append(to_string(Set));filename.append(".txt");

		//放入GEN的 路徑+檔名
		string GENBuffer=GENPath;
		GENBuffer.append(filename);

		//=========================開啟GENFile
		GENfile.open(GENBuffer, ios::in);	//開啟檔案.寫入狀態
		if(!GENfile){//如果開啟檔案失敗，fp為0；成功，fp為非0
			cout<<"Fail to open file: "<<GENBuffer<<endl;
			system("PAUSE");
			return 0;
		}

		//========================開啟ScheduleFile
		string ScheduleFileBuffer=SchedulePath;
		ScheduleFileBuffer.append("Schedule_");
		ScheduleFileBuffer.append(filename);

		Schdulefile.open(ScheduleFileBuffer, ios::out);	//開啟檔案.寫入狀態
		if(!Schdulefile){//如果開啟檔案失敗，fp為0；成功，fp為非0
			cout<<"Fail to open file: "<<ScheduleFileBuffer<<endl;
			system("PAUSE");
			return 0;
		}

		//========================開啟Resultfile
		string ResultBuffer=ResultPath;
		ResultBuffer.append("Result_");
		ResultBuffer.append(filename);

		Resultfile.open(ResultBuffer, ios::out);	//開啟檔案.寫入狀態
		if(!Resultfile){//如果開啟檔案失敗，fp為0；成功，fp為非0
			cout<<"Fail to open file: "<<ResultBuffer<<endl;
			system("PAUSE");
			return 0;
		}

		/*===================================================
							在同一利用率下
							跑Set數
		===================================================*/
		for(short int setnum=0;setnum<Set;setnum++){
			Meetflag=true;
			Timeslot=0;
			Hyperperiod=0;
			totalevent=0;
			/*==========================
				建立Linklist以及
				GEN的資料放進去
			==========================*/
			StructGEN();
			
			/*==========================
					Topology
			==========================*/
			Topology();

			/*==========================
				TDMA assignment
			==========================*/

			/*==========================
				計算Connection interval
			==========================*/
			switch (Rateproposal)
			{
			case 0:
				Connectioninterval=1;
				break;
			case 1:
				TSB();
				break;
			case 2:
				DIF();
				Connectioninterval=1;
				break;
			default:
				Connectioninterval=1;
				break;
			}

			Flowinterval=Connectioninterval;
			
			//放置傳輸載量
			packet=Head->nextnd->pkt;
			while(packet!=NULL){
				packet->exeload=packet->load;
				packet->exehop=packet->hop;

				packet=packet->nextpkt;
			}
			/*==========================
					EDF scheduling
			==========================*/
			Headflow->pkt=NULL;//一開始的flow中包含的封包設定為NULL
			
			while(Timeslot<Hyperperiod){
				PacketQueue();	
				BufferSet();

				Node *Flownode=Head->nextnd;
				while(Flownode!=NULL){	
					Buffer=Flownode->NodeBuffer;
					if(Timeslot % int(Flownode->eventinterval)==0)
						FlowEDF();	//主要Scheduling

					Flownode=Flownode->nextnd;
				}
				/*
				if(Timeslot%Flowinterval==0){
					

						if(Rateproposal==2){
							Rate_TO_Interval(DIFMinperiod);
							Flowinterval=Connectioninterval;	
						}
					
					Node *Flownode=Head->nextnd;
					
					Buffer=Flownode->NodeBuffer;
					FlowEDF();	//主要Scheduling
				}
				*/
				Timeslot++;
			}
			
			/*==========================
					END
			==========================*/
			double totalenergy=0;
			node=Head->nextnd;
			while(node!=NULL){
				cout<<"Node"<<node->id<<" E:"<<node->energy<<endl;
				totalenergy=totalenergy+node->energy;
				node=node->nextnd;
			}
			
			Resultfile<<"TotalEnergy:"<<totalenergy<<endl;
			cout<<"TotalEnergy:"<<totalenergy<<endl;

			node=Head->nextnd;
			while(node!=NULL){
				double AvgRate=0;
				double Pktnum=0;
				packet=node->pkt;
				while(packet!=NULL){
					AvgRate=AvgRate+packet->rate;
					Pktnum++;
					packet=packet->nodenextpkt;
				}
				AvgRate=AvgRate/Pktnum;

				Resultfile<<"Node"<<node->id<<endl;
				Resultfile<<"E:"<<node->energy<<endl;
				Resultfile<<"Connectioninterval:"<<node->eventinterval<<endl;
				Resultfile<<"Total Event:"<<totalevent<<endl;

				node=node->nextnd;
			}
			if(Meetflag==true){
				Resultfile<<"Meet Deadline:MEET"<<endl;
				cout<<"Meet Deadline:MEET"<<endl;

				AverageE=AverageE+totalenergy;
				Meetcount++;
			}
			else{
				Resultfile<<"Meet Deadline:MISS"<<endl;
				cout<<"Meet Deadline:MISS"<<endl;
			}
	
			Resultfile<<"=============================================="<<endl;
			Schdulefile<<"=============================================="<<endl;
			cout<<"=============================================="<<setnum<<endl;
		}//Set End
		cout<<"FinalResult"<<endl;
		cout<<"Meet="<<Meetcount<<endl;
		cout<<"Miss="<<Set-Meetcount<<endl;
		cout<<"MeetRatio="<<Meetcount/Set<<endl;
		cout<<"AverageEnergy="<<AverageE/Meetcount<<endl;
		cout<<"=============================================="<<endl;

		Resultfile<<"FinalResult"<<endl;
		Resultfile<<"Meet="<<Meetcount<<endl;
		Resultfile<<"Miss="<<Set-Meetcount<<endl;
		Resultfile<<"MeetRatio="<<Meetcount/Set<<endl;
		Resultfile<<"AverageEnergy="<<AverageE/Meetcount<<endl;

		GENfile.close();
		Schdulefile.close();
		Resultfile.close();
	}

	system("PAUSE");
	return 0;
}

/*==========================
	Flow EDF Scheduling
==========================*/
void FlowEDF(){
	
		/*---------------------------------------------
				判斷Flow 是否為NULL
				若有封包則進行傳輸
				(包含判斷是否結束)
		---------------------------------------------*/
		if(Buffer->pkt!=NULL){
			totalevent++;

			//cout<<"Time slot:"<<Timeslot;
			Schdulefile<<"Time slot:"<<Timeslot;
			//=============================================執行傳輸
			packet=Buffer->pkt;
			Pktsize=Buffer->pktsize;

			while(Buffer->load!=0){
								
				packet->exeload--;
				Buffer->load--;
				packet->State="Transmission";		//傳輸狀態

				NodeState();	//改變Node狀態

				if(packet->exeload==0){

					//cout<<" Packet:"<<packet->id;
					Schdulefile<<" NP:"<<packet->node->id<<","<<packet->id;
					
					//判斷是否需要hop
					packet->exehop--;
					if(packet->exehop>0)
					{
						packet->exeload=packet->load;
						Headflow->pkt=packet;
					}else{
						//判斷是否miss deadline
						if((Timeslot)>=packet->deadline){
							//cout<<"Time slot:"<<Timeslot<<"  PKT"<<packet->id<<" Miss deadline"<<endl;
							Schdulefile<<"(PKT"<<packet->id<<" Miss deadline"<<" Deadline "<<packet->deadline<<")";

							Meetflag=false;
							//system("PAUSE");
						}
						
						packet->readyflag=0;
						packet->exeload=packet->load;
						packet->arrival=packet->deadline;
						packet->deadline=packet->deadline+packet->period;
						packet->State="Idle";		//傳輸狀態
						packet->exehop=packet->hop;	
					}

					//Buffer往前移動
					packet=packet->buffernextpkt;
					Buffer->pkt=packet;	
				}
				if(Buffer->pkt==NULL)
					break;
			}
			Headflow->pkt=packet;//放置會後一個packet

			if(Headflow->pkt!=NULL){
				//cout<<" Packet:"<<packet->id;
				Schdulefile<<" NP:"<<packet->node->id<<","<<packet->id;
				packet->State="Transmission";		//傳輸狀態
			}
			
			//cout<<endl;
			Schdulefile<<endl;
			/*---------------------------
				傳輸完立即做
				狀態切換 & Energy 計算
			---------------------------*/
			NodeEnergy();	//計算個感測器Energy

		}else{
			NodeState();	//改變Node狀態
			NodeEnergy();

			//cout<<"Time slot:"<<Timeslot<<" IDLE"<<endl;
			Schdulefile<<"Time slot:"<<Timeslot<<" IDLE"<<endl;
			
		}
		//下一個時間點
		//Timeslot++;
}
/*==========================================
			Arrange Queue
{ReadyQ & WaitQ is assign by pkt->readyflag}
==========================================*/

void PacketQueue(){
	
	Packet *camparepkt;
	Packet *tmpReadyQ=new Packet;
	tmpReadyQ->readynextpkt=ReadyQ;

	packet=Head->nextnd->pkt;
	while(packet!=NULL){
		packet->readynextpkt=NULL;	//下一ready設定為NULL
		packet->readyprepkt=NULL;	//上一ready設定為NULL
		packet->searchdone=0;				//尚未找過
		packet=packet->nextpkt;
	}

	packet=Head->nextnd->pkt;
	ReadyQ_overflag=0;
	while(!ReadyQ_overflag){
	
		/*--------------------------
			找searchdone為0 的
			前兩個packet 做比較
			(packet & camparepkt)
		--------------------------*/
	
		packet=Head->nextnd->pkt;
		while(packet!=NULL){
			if(packet->searchdone==1 )
				packet=packet->nextpkt;
			else
				break;
		}

		if(packet!=NULL)
			camparepkt=packet->nextpkt;
		while(camparepkt!=NULL){
			if(camparepkt->searchdone==1 )
				camparepkt=camparepkt->nextpkt;
			else
				break;
		}
		
		/*--------------------------
			找出最小Deadline
			全部搜尋一遍
		--------------------------*/
	
		if(packet!=NULL && camparepkt!=NULL){
			while(camparepkt!=NULL){

				//找packet下一個比較的camparepkt
				if(packet!=NULL)
					camparepkt=packet->nextpkt;
				while( camparepkt!=NULL){
					if(camparepkt->searchdone==1)
						camparepkt=camparepkt->nextpkt;
					else
						break;
				}
				
				//若有packet->deadline >camparepkt->deadline ,將packet=camparepkt
				while(camparepkt!=NULL){
					if(packet->deadline >camparepkt->deadline ){
						packet=camparepkt;
					}
					if(camparepkt!=NULL)
							camparepkt=camparepkt->nextpkt;
					while(camparepkt!=NULL){
						if(camparepkt->searchdone==1 )
							camparepkt=camparepkt->nextpkt;
						else
							break;
					}
				}
			}
		}else if(camparepkt==NULL){
			ReadyQ_overflag=1;//找到剩下最後一個packet，即可終止尋找
		}
	
		/*--------------------------
		將最小Deadline放入ReadyQ當中
		並標記已尋找過(searchdone=1)
		--------------------------*/

		packet->searchdone=1;
		if(Timeslot>=packet->arrival){
			packet->readyflag=1;//封包 Arrival
		}else{
			packet->readyflag=0;//封包 尚未Arrival
		}

		ReadyQ->readynextpkt=packet;
		packet->readyprepkt=ReadyQ;
		ReadyQ=packet;
	}
	ReadyQ->readynextpkt=NULL;
	ReadyQ=tmpReadyQ->readynextpkt;

	delete camparepkt;camparepkt=NULL;
	delete tmpReadyQ;tmpReadyQ=NULL;
	//----------------------------------------Global Ready Queue done

	/*--------------------------
		建立各自node上的
		ready packet queue
	--------------------------*/
	Node *tmp_node;
	Packet *tmpreadypkt;
	Packet *tmp_nodepkt=ReadyQ->readynextpkt;
	while(tmp_nodepkt!=NULL){
		//先找所屬的 node 感測器
		tmp_node=tmp_nodepkt->node;

		//找node->pktQueue 最後一個
		tmpreadypkt=tmp_node->pktQueue;
		if(tmp_node->pktQueue==NULL){
			tmp_node->pktQueue=tmp_nodepkt;			
			tmp_node->pktQueue->nodereadynextpkt=NULL;
			tmp_node->pktQueue->nodereadyprepkt=NULL;
		}else{
			while(tmpreadypkt->nodereadynextpkt!=NULL){
				tmpreadypkt=tmpreadypkt->nodereadynextpkt;
			}
			tmpreadypkt->nodereadynextpkt=tmp_nodepkt;				
			tmpreadypkt->nodereadynextpkt->nodereadynextpkt=NULL;
			tmpreadypkt->nodereadynextpkt->nodereadyprepkt=tmp_nodepkt->nodereadyprepkt;
		}

		//換下一個Global Queue packet
		tmp_nodepkt=tmp_nodepkt->readynextpkt;
	}
}
/*=============================================	
		建立好Buffer上的packet
	pkt link list, load, packet size
=============================================*/
void BufferSet(){
	Node *Bufnode=Head->nextnd;

	while(Bufnode!=NULL){
		Bufnode->NodeBuffer->pktsize=0;//先把buffer內的封包量清空
		Bufnode->NodeBuffer->load=0;
		Bufnode->NodeBuffer->pkt=NULL;
		
		Packet *tmpbufferpkt;
		int tmpsize=Bufnode->NodeBuffer->pktsize;
		int intervalsize=0;

		packet=Bufnode->pktQueue;
		if(Bufnode->NodeBuffer->pktsize!=0){
			while(packet->readyflag!=1)			
				packet=packet->nodereadynextpkt;
			if(packet==tmpbufferpkt)
				packet=packet->nodereadynextpkt;
		}

		while(Bufnode->NodeBuffer->pktsize<Maxbuffersize && packet!=NULL){

			if(packet->readyflag!=1){			//尚未ready
				packet=packet->nodereadynextpkt;
			}else{
			
				//確認packet不存在於Buffer中
				bool existflag=false;
				Packet* existpkt=Bufnode->NodeBuffer->pkt;
				while(existpkt!=NULL){
					if(existpkt==packet)
						existflag=true;
					existpkt=existpkt->buffernextpkt;
				}

				if(existflag!=true){
					//-----------------------------------------放入Buffer link list
					if(Bufnode->NodeBuffer->pktsize == 0){
						Bufnode->NodeBuffer->pkt=packet;
						tmpbufferpkt=Bufnode->NodeBuffer->pkt;
						tmpbufferpkt->buffernextpkt=NULL;
					}else {
						tmpbufferpkt->buffernextpkt=packet;
						tmpbufferpkt=packet;
						tmpbufferpkt->buffernextpkt=NULL;
					}
					
					//-----------------------------------------設定Buffer size
					tmpsize=Bufnode->NodeBuffer->pktsize;
					if((Bufnode->NodeBuffer->pktsize+ceil(packet->exeload/payload))<=Maxbuffersize){
						Bufnode->NodeBuffer->pktsize=(Bufnode->NodeBuffer->pktsize+ceil(packet->exeload/payload));
					}else{
						Bufnode->NodeBuffer->pktsize=Maxbuffersize;
					}
					intervalsize=Bufnode->NodeBuffer->pktsize-tmpsize;//可塞的packet量

					//-----------------------------------------計算Buffer load
					if(packet->exeload>payload){
						double tmpload=packet->exeload;
						while(intervalsize!=0){
							if(tmpload>payload)
								Bufnode->NodeBuffer->load=Bufnode->NodeBuffer->load+payload;
							else
								Bufnode->NodeBuffer->load=Bufnode->NodeBuffer->load+tmpload;
							tmpload=tmpload-payload;
							intervalsize--;
						}

					}else{
						Bufnode->NodeBuffer->load=Bufnode->NodeBuffer->load+packet->exeload;
					}
				}
				packet=packet->nodereadynextpkt;
			}
		}

		Bufnode=Bufnode->nextnd;
	}

}
/*==========================================
		分別計算各個node上的Power
		每一slot上做加總
		即為E=P*t
==========================================*/
void NodeEnergy(){
	/*----------------------------
		Wakeup Sleep	  未連接情況
		Transmission Idle 連接情況
	----------------------------*/
	Node *Enode;
	Packet *Epkt;
	
	Enode=Head->nextnd;
	while(Enode!=NULL){
		Enode->LatestTimeslot=Timeslot;
		Enode=Enode->nextnd;
	}

	Enode=Head->nextnd;
	while(Enode!=NULL){
		//==========================Wakeup 狀態
		if(Enode->State=="Wakeup"){
			
		}
		//==========================Sleep 狀態
		if(Enode->State=="Sleep"){
			
		}
		//==========================Transmission 狀態
		if(Enode->State=="Transmission"){
			double P=0;

			P=IntervalPower(Pktsize,Enode->LatestTimeslot-Enode->ExTimeslot);
			Enode->energy=Enode->energy+P;
			//cout<<"Interval:"<<(Enode->LatestTimeslot-Enode->ExTimeslot)<<" E:"<<Enode->energy<<endl;
			Enode->ExTimeslot=Enode->LatestTimeslot;//紀錄此次發送時間
		}
		//==========================Idle 狀態
		if(Enode->State=="Idle"){

		}

		Enode=Enode->nextnd;
	}

}
/*==========================================
		計算在此Connection interval下的
			Power consumption
==========================================*/
double IntervalPower(int Pktnum,int interval){
	if(interval!=0){
		double Ipeak,Vpeak;
		double Tc;
		double power;

		Tc=interval*unit;
		
		//Ipeak=(Pktnum*Ie*Te+Isleep*(Tc-Te*Pktnum))/Tc;
		Ipeak=parma*Pktnum*exp(-parmb*Tc)+Isleep;
		
		Vpeak=Vcc-Ipeak;

		power=Vcc*Ipeak-(Ipeak*Ipeak)*K;
	
		//cout<<"T:"<<Timeslot<<" P:"<<power<<endl;

		return power;
	}else
		return 0;
}

/*==========================================
		計算Node State
==========================================*/

void NodeState(){
	Node   *Snode;
	Packet *Spacket;
	bool tx_flag=false;

	Snode=Head->nextnd;
	
	while(Snode!=NULL){
		Spacket=Snode->pkt;
		while(Spacket!=NULL){
			if(Spacket->State=="Transmission"){
				Snode->State="Transmission";
				tx_flag=true;
			}
			if(Spacket->State=="Idle" && tx_flag!=true){
				Snode->State="Idle";
			}

			Spacket=Spacket->nodenextpkt;
		}
		Snode=Snode->nextnd;
		tx_flag=false;
	}
	delete Snode;
	delete Spacket;
}
/*===========================
		建立linklist
		並將訊息寫入
===========================*/
void StructGEN(){
	
	delete Head;
	Head=NULL;
	delete Headpacket;delete node;delete packet;delete Headflow;
	Headpacket=NULL;node=NULL;packet=NULL;Headflow=NULL;

	Head=new Node;
	Headpacket=new Packet;
	node=new Node;
	packet=new Packet;
	ReadyQ=new Packet;
	Headflow=new Flow;
	
	/*==========================
			建立Link list
		    Node & Packet
	===========================*/
	/*-------------------------
		Gen node(Linklist)
	-------------------------*/
	string str;
	GENfile>>str;
	stream<<str; stream>>nodenum; stream.clear();
	GENfile>>str;
	stream<<str; stream>>pktnum; stream.clear();
	GENfile>>str;
	stream<<str; stream>>Hyperperiod;

	Head->nextnd=node;
	for(int n=0;n<nodenum;n++){
		/*-------------------------
			packet(Linklist)
		-------------------------*/

		node->pkt=packet;
		packet->nodeprepkt=NULL;
		for (int p=0;p<pktnum;p++){

			Packet* nextpacket=new Packet;
			Packet* prepacket=packet;

			packet->nextpkt=nextpacket;
			packet->nodenextpkt=nextpacket;
			packet->readynextpkt=packet->nextpkt;
			packet=nextpacket;
			packet->prepkt=prepacket;
			packet->nodeprepkt=prepacket;
			packet->readyprepkt=prepacket;
		}
		packet->nodeprepkt->nodenextpkt=NULL;

		//--------------------------Packet Done

		Node* nextnode=new Node;
		Node* prenode=node;
		node->nextnd=nextnode;
		node=nextnode;
		node->prend=prenode;
	}
	packet=packet->prepkt;
	packet->nextpkt=NULL;
	packet->readynextpkt=NULL;
	node=node->prend;
	node->nextnd=NULL;

	Headpacket->nextpkt=Head->nextnd->pkt;
	Head->nextnd->pkt->prepkt=Headpacket;
	Headpacket->readynextpkt=Head->nextnd->pkt;
	Head->nextnd->pkt->readyprepkt=Headpacket;
	//-----------------------------Node Done
	
	/*==========================
		寫入GEN的資訊
	==========================*/
	node=Head;
	int pktid=1;
	int ndid=1;
	while(str!="=========="){
		
		GENfile>>str;
		if(str=="Node"){
			node=node->nextnd;

			GENfile>>str_coor_x>>str_coor_y>>str_radius;
			stream.clear();	stream<<str_coor_x;stream>>node->coor_x;		//coor_x
			stream.clear();	stream<<str_coor_y;stream>>node->coor_y;		//coor_y
			stream.clear();	stream<<str_radius;stream>>node->radius;		//radius
			
			node->energy=0;
			node->pktQueue=NULL;
			node->NodeBuffer=new PacketBuffer;
			packet=node->pkt;

			node->id=ndid++;

		}
		if(str=="Pkt"){
			GENfile>>strload>>strperiod>>strutilization>>strhop;
			
			stream.clear();	stream<<strload;stream>>packet->load;					//Load
			stream.clear();	stream<<strperiod;stream>>packet->period;				//Period
			stream.clear();	stream<<strutilization;	stream>>packet->utilization;	//Utilization
			stream.clear();	stream<<strhop;	stream>>packet->hop;	//Hop
			
			node->hop=packet->hop;
			packet->nodeid=node->id;
			packet->period=packet->period;
			packet->node=node;														//所屬的感測器
			packet->exehop=packet->hop;
			packet->exeload=packet->load;
			packet->arrival=0;														//Arrival
			packet->deadline=packet->arrival+packet->period;						//Deadline
			packet->id=pktid++;														//id
			packet->readyflag=1;													//ready flag
			packet->searchdone=0;													//ready flag
			packet->rate=0;
			packet->State="Idle";
			//下一個packet
			packet=packet->nextpkt;
		}
	}
}
/*===========================
		WSN拓鋪
===========================*/
void Topology(){
	Node *TNode=Head->nextnd;
	double distance=100;

	while(TNode!=NULL){
		//--------------------------Level 1
		if(TNode->hop==1){
			TNode->SendNode=Head;
		}

		//--------------------------Level 2~
		if(TNode->hop!=1){
			Node *tmp_TNode=Head->nextnd;

			while(tmp_TNode!=NULL){
				if(tmp_TNode->hop==TNode->hop-1){
					if(distance>sqrt(pow(TNode->coor_x-tmp_TNode->coor_x,2)+pow(TNode->coor_y-tmp_TNode->coor_y,2))){
						distance=sqrt(pow(TNode->coor_x-tmp_TNode->coor_x,2)+pow(TNode->coor_y-tmp_TNode->coor_y,2));
						TNode->SendNode=tmp_TNode;
					}
				}
				tmp_TNode=tmp_TNode->nextnd;
			}
		}

		TNode=TNode->nextnd;
	}
}
/*===========================
		比較組
	找各個區間(interval)
	<arrival -> period> 
step1:找各區間 完整的packet
step2:各區間的(packet->load加總) 除以 (interval)
step3:計算各區間 rate 
step4:找出最大rate , 其在區間的packet assign 此rate
(找區間時要將有rate的區間時間拿掉)
===========================*/
void DIF(){
	PacketQueue();
	DIFMinperiod=ReadyQ->readynextpkt->period;

	Packet * DIFpacket;
	map<double,map<double,DIFtable>> Table;	//二維map 內容格式為DIFtable
	double maxarrvial,maxdeadline;			//找最大Density中 的區間
	double Maxdesity=0;						//找區間中 最大Density
	bool Doneflag=false;					//全部assign完rate
	
	while(Doneflag!=true){
		/*============================
			DIF init
			找各區間
		============================*/
		DIFpacket=Head->nextnd->pkt;
		while(DIFpacket!=NULL){
			Table[DIFpacket->arrival][DIFpacket->deadline];
		
			for (map<double,map<double,DIFtable>>::iterator a=Table.begin(); a!=Table.end(); a++){
				for(map<double,DIFtable>::iterator p=Table[a->first].begin(); p!=Table[a->first].end(); p++){
					Table[a->first][p->first];
					Table[a->first][p->first].load=0;
					Table[a->first][p->first].length=0;
				}
			}
		
			DIFpacket=DIFpacket->nextpkt;
		}
		/*============================
			各區間的(packet->load加總)
				   (區間長度)
				   (計算各區間 rate)
		============================*/
		for (map<double,map<double,DIFtable>>::iterator a=Table.begin(); a!=Table.end(); a++){
			for(map<double,DIFtable>::iterator p=Table[a->first].begin(); p!=Table[a->first].end(); p++){
				DIFpacket=Head->nextnd->pkt;

				while(DIFpacket!=NULL){
					//確定arrival 比 deadline小
					if(a->first < p->first){
						if(a->first <= DIFpacket->arrival && DIFpacket->deadline <= p->first && DIFpacket->rate==0){
							Packet* tmpDIFpacket;
							double start=a->first;
							double end=p->first;
							
							//放入區間、區間內load總值 以及 此區間Density
							Table[a->first][p->first].length=p->first - a->first;
							while(start!=end){
								
								tmpDIFpacket=Head->nextnd->pkt;
								while(tmpDIFpacket!=NULL){
									if(tmpDIFpacket->rate!=0){
										if(start>=tmpDIFpacket->arrival && (start+1)<=tmpDIFpacket->deadline){
											Table[a->first][p->first].length--;
											tmpDIFpacket=NULL;
										}
									}
									if(tmpDIFpacket!=NULL)
										tmpDIFpacket=tmpDIFpacket->nextpkt;
								}
								start++;
							}

							Table[a->first][p->first].load=Table[a->first][p->first].load+DIFpacket->load;
							Table[a->first][p->first].density=Table[a->first][p->first].load/Table[a->first][p->first].length;
							
							//找出最大rate,並紀錄區間
							if(Table[a->first][p->first].density>=Maxdesity){
								maxarrvial=a->first;
								maxdeadline=p->first;
								Maxdesity=Table[a->first][p->first].density;
							}
						}
					}

					DIFpacket=DIFpacket->nextpkt;
				}
			}	
		}
		/*============================
			在區間找出區間內的packet
		並assign rate 到 packet->rate上
		============================*/
		DIFpacket=Head->nextnd->pkt;
		while(DIFpacket!=NULL){
			if(maxarrvial <= DIFpacket->arrival && DIFpacket->deadline <= maxdeadline ){
				//尚未assign rate
				if(DIFpacket->rate==0){
					DIFpacket->rate=Table[maxarrvial][maxdeadline].density;
					
					DIFpacket->rate=(DIFpacket->rate);
				}
			}

			DIFpacket=DIFpacket->nextpkt;
		}
		Maxdesity=0;

		/*==================
			確認所有packet
			已經assign完 rate
		====================*/
		Doneflag=true;
		DIFpacket=Head->nextnd->pkt;
		while(DIFpacket!=NULL){
			if(DIFpacket->rate==0)
				Doneflag=false;
			DIFpacket=DIFpacket->nextpkt;
		}
	}
	
	delete []DIFpacket;
}
/*==============================================
(小於兩倍Minperiod的pkt size)	->Minsize
	(最大load的pkt size)			->Maxsize

	若(2*Minsize+Maxsize)大於兩倍Buffersize
		Connection interval為Minperiod/2
	否
		依照間隔buffersize做計算
==============================================*/
void TSB(){
	PacketQueue();		//先排Ready Queue
	Packet *TSBpktQ=ReadyQ;
	Packet *TSBpkt=Head->nextnd->pkt;
	double Tc=0;
	
	if(false){//(2*Minsize+Maxsize) > 2*Maxbuffersize (提供給non-preemption 用)
		//Tc=floor(Minperiod/2);
	}else{
		PacketQueue();		//先排Ready Queue

		Node *TSBnode=Head->nextnd;
		while(TSBnode!=NULL){
			Packet *TSBpkt=TSBnode->pktQueue;
			double Totalsize=0;
			double PacketSize=0;
			double Tslot=0;
			bool doneflag=false;

			//======================找Minperiod, 設定為Tc init
			Tc=TSBpkt->period;

			//======================分析每一period下, 是否能meet deadline
			Tslot=TSBpkt->period;
	
			while(doneflag!=true){
				Totalsize=0;

				//算出所需buffer量
				TSBpkt=TSBnode->pktQueue;
				while(TSBpkt->period <= Tslot){
					Totalsize=Totalsize+(ceil(TSBpkt->load/payload)*ceil(Tslot/TSBpkt->period));	
					TSBpkt=TSBpkt->nodereadynextpkt;
					if(TSBpkt==NULL)
						break;
				}

				PacketSize=floor(Tslot/Tc)*double(Maxbuffersize);
				while(Totalsize > PacketSize){
					Tc--;
					PacketSize=floor(Tslot/Tc)*double(Maxbuffersize);
				}

				//更新Time slot
				if(TSBpkt!=NULL){
					Tslot=TSBpkt->period;
				}else{
					doneflag=true;
				}
			}	

			TSBnode->eventinterval=Tc;
			TSBnode=TSBnode->nextnd;
		}
	}
	
}
void Rate_TO_Interval(int defaultMinperiod){
	//判斷是否需要改變Tc
	double exconnectioninterval=Connectioninterval;
	Connectioninterval=0;
	Packet *tmppkt=Buffer->pkt;
	double Allload=Buffer->load;
	double Maxrate=0;
	double Minperiod;

	while(tmppkt!=NULL){
		if(tmppkt->rate > Maxrate){
			if(tmppkt->exeload <= payload){
				Connectioninterval=Connectioninterval+(tmppkt->exeload/tmppkt->rate);
				Allload=Allload-tmppkt->exeload;
			}else{
				Connectioninterval=Connectioninterval+(payload/tmppkt->rate);
				Allload=Allload-payload;
			}
		}
		tmppkt=tmppkt->buffernextpkt;
	}
		
	if(Connectioninterval<1){
		Connectioninterval=exconnectioninterval;//設定為之前的interval
	}else{
		Connectioninterval=floor(Connectioninterval);
	}

	//判斷Connectioninterval是否大於Minperiod
	if(Connectioninterval>defaultMinperiod)
		Connectioninterval=defaultMinperiod;

}
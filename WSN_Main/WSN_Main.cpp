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
#include <conio.h>

#include "include/Struct/WSNFile.h"
#include "include/Struct/WSNStruct.h"
#include "include/Algorithm/ConnInterval.h"
#include "include/Schedule/FlowSchedule.h"
#include "include/Struct/WSNEnergy.h"
#include "include/Algorithm/TDMA.h"

using namespace std;

/*=================================
		Experiment Setting
==================================*/
const float inv_r=80;
const float MIN_Rate=80;
const float MAX_Rate=960;
const short int Set=100;

short int readsetting=1;					//是否要讀取本地Setting.txt
short int Rateproposal=1;				//AssignRate()中的方法編號 0=>Event, 1=>MEI, 2=>DIF, 3=>Lazy and 4=>min period<2,3屬於單一node上的調整>
short int TDMA_Rateproposal=2;			//TDMA和connection interval上的校正 0=>LDC(各除3), 1=>選最小interval除TDMA size, 2=>照lifetime ratio
short int TDMAscheduleproposal=2;		//Gateway 通知node傳輸順序 0=>做EDF排程 1=>直接照TDMA表做傳輸 2=>EIF 3=>Polling

bool sche_flag=false;					//是否要測試schedulability
short int TDMAproposal=0;				//TDMA的assign方法 0=>自己的方法(只有一個superslot), 1=>Node base方法 (會再接續加入superslot)
int EXECBclock=100;						//Lazy Timer
/*=================================
		Global value
==================================*/
bool preemptionenable=true;			//設定可否preemption
int Flowinterval=0;					//觸發進入flow的conneciton interval
int Pktsize=0;						//計算IntervalPower的pkt num
double DIFMinperiod=0;
double Meetcount=0;
double AverageE=0;
int TDMASlot=1;
int overheadcount=6;				//動態改變interval時需要等待6個interval才能變動
FrameTable* Cycle=NULL;
short int pollingcount=1;

int Callbackclock;
Edge *HeadEdge=new Edge;
Edge *MainEdge=new Edge;
Edge *ConflictEdge=new Edge;
FrameTable *FrameTbl=new FrameTable;
TDMATable *TDMA_Tbl=new TDMATable;
TDMATable *NotifyTable;
PacketBuffer* Buffer=new PacketBuffer;
Node* SetHead=new Node;
Node* Head=new Node;
Node* NotifyNode=new Node;
Packet* Headpacket=new Packet;
Node *SetNode=new Node;
Node* node=new Node;
Packet* packet=new Packet;
Packet *ReadyQ=new Packet();
Flow *Headflow=new Flow();

int ReadyQ_overflag=0;
stringstream stream;
string str_coor_x,str_coor_y,str_radius;
string strload,strperiod,strutilization,strhop;
int nodenum=0;
int nodelevel1=0;
int nodelevel2=0;
int pktnum=0;
long int Timeslot=0;
long int Hyperperiod=0;
double Maxrate=20;					//最高速度為20bytes/slot
double payload=20;					//payload 為 20bytes
int Maxbuffersize=4;				//Maxbuffersize 為 4個packets
double timeslot=10;					//單位時間為10ms

double slotinterval=10;				//Time slot間距為10ms
double Connectioninterval=0;		//Conneciton inteval 只會在10ms~4000ms
double totalevent=0;				//Event數量
bool Meetflag=true;					//看是否meet deadline
/*========================================
		Power function parameter
========================================*/
double Vcc=3.3;			//BLE 驅動電壓

double I_sleep=0.000001;	//Sleep 電流 1uA
double Time_sleep=0.01;		//Sleep 時間 10ms (uint time)
double I_notify=0.0082463;	//Notify 電流 8.2463mA
double Time_notify=0.002775;	//Notify 時間 2.775ms
double I_Tran=0.0142744;	//Transmission 電流 14.2744mA
double Time_Tran=0.00049;	//Transmission 時間 0.49ms
double BatteryCapacity=0.230; //230mAh

double Ie=0.07679763;		//傳輸峰值 電流
double Te=0.0002768;		//傳輸時間
double K=1;				//Rate power常數
double unit=0.01;		//時間單位為10ms
double TotalEnergy=0;
double parma=0.00191571992;
double parmb=24.4058498;

/*========================================
		Create Object
========================================*/
EventInterval Interval_obj;
TDMA TDMA_obj;

int main(int argc, char* argv[]){
	/*
	cout<<"Type single node interval(0->Event, 1->MEI):";
	cin>>Rateproposal;
	cout<<"Type TDMA table (0->single superslot, 1->Node base):";
	cin>>TDMAproposal;
	cout<<"Type TDMA with interval (0->EIMA, 1->Divide small interval by TDMA size):";
	cin>>TDMA_Rateproposal;
	cout<<"Type TDMA schedule (0->EDF, 1->TDMA table):";
	cin>>TDMAscheduleproposal;
	*/

	for(float U=MIN_Rate; U<=MAX_Rate; U+=inv_r){
		cout<<"Data Rate: "<<U<<endl;

		SetHead->lifetime=0;
		delete SetNode;SetNode=NULL;
		Meetcount=0;
		AverageE=0;
		totalevent=0;

		CreateFile(U,Set,argv[0]);//開啟WSNGEN 並且建立輸出檔案 (WSNFile.cpp)
		if(readsetting==1){
			ExperimentSetting(&Rateproposal, &TDMA_Rateproposal, &TDMAscheduleproposal);//做實驗設定輸出
		}
		/*===================================================
							在同一利用率下
							跑Set數
		===================================================*/
		for(short int setnum=0;setnum<Set;setnum++){
			Meetflag=true;
			Timeslot=0;
			TDMASlot=-1;
			Hyperperiod=0;
			totalevent=0;
			NotifyNode=NULL;
			Cycle=NULL;
			pollingcount=1;
			/*==========================
				建立Linklist以及
				GEN的資料放進去
				(WSNStruct.cpp)
			==========================*/
			StructGEN();		
			
			/*==========================
				計算Connection interval 
					& Adv interval
			==========================*/
			Interval_obj.Algorithm(Rateproposal);		//安排好各個node上的interval

			/*==========================
			Topology & TDMA assignment
			==========================*/
			TDMA_obj.Topology();
			TDMA_obj.NodeColoring();				
			TDMA_obj.TDMA_Assignment(TDMAproposal);		//安排好TDMA_Tbl

			/*==========================
			Interval & TDMA adjustment 
			==========================*/
			Interval_obj.Interval_TDMA_Algorithm(TDMA_Rateproposal);		//包含TDMA考量,做node上的interval修改 且含有Scan duration 計算
			Interval_obj.ConnectionPriority();								//連接順序,設定EventTime

			/*=========================
				Schedulability test
			=========================*/
			if(sche_flag){
				Schedulability();
			}
			/*==========================
				EDF scheduling
				(FlowSchedule.cpp)
				<Head, TDMA_Tbl> 
				<NotifyNode, NotifyTable>
			==========================*/
			Head->RecvNode=NULL;		//Head 接收節點要設定為NULL
			Head->FrameSize=0;
			TDMA_Tbl->currslot=true;	//一開始第一個要為true
			Callbackclock=0;

			while(Timeslot<=Hyperperiod){
				PacketQueue();
				Schedule(TDMAscheduleproposal,Rateproposal);
				
				Timeslot++;
			}
			Finalcheck();

			/*==========================
					END
			==========================*/
			SaveFile(setnum);//(WSNFile.cpp)

		}//Set End

		SaveSet(U,Set);//(WSNFile.cpp)
	}
	CloseFinal();

	system("PAUSE");
	return 0;
}

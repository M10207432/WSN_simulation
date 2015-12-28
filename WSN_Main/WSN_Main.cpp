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
const float inv_r=40;					//Input 各個rate間距
const float MIN_Rate=40;				//Input開始的rate
const float MAX_Rate=960;				//Input最終rate
const short int Set=100;				//Input set 數量

short int readsetting=1;				//是否要讀取本地Setting.txt
short int Service_interval=1;			//0=>Event, 1=>MEI, 2=>DIF, 3=>Lazy and 4=>Greedy (Min period) <單一node上對varied data調整>
short int Connection_Interval=2;		//0=>LDC (各service interval除node1level), 1=>Greedy (選 Min interval除node1level), 2=>EIMA <TDMA和connection interval上的校正>
short int WriReq_Sche=2;				//0=>NPEDF 1=>RR 2=>EIF 3=>Polling <Gateway 通知node傳輸順序>

bool sche_flag=false;					//是否要測試schedulability
int EXECBclock=100;						//Lazy Timer (ms)
bool EIMADemand_flag=false;				//判斷EIMA計算是否要用demand bound計算

/*=================================
		Global value
==================================*/
short int TDMAproposal=0;				//TDMA的assign方法 0=>自己的方法(只有一個superslot), 1=>Node base方法 (會再接續加入superslot)
int Pktsize=0;							//計算IntervalPower的pkt num
double Meetcount=0;						//在Set數量中，meet的數量
double AverageE=0;						//不管有沒miss，計算整體Set的energy

int overheadcount=6;					//動態改變interval時需要等待6個interval才能變動
FrameTable* Cycle=NULL;					//Polling schedule用到的轉換cyle
short int pollingcount=1;				//Polling schedule 計數cycle id

int Callbackclock;						//計數Callback時間
Edge *HeadEdge=new Edge;
Edge *MainEdge=new Edge;
Edge *ConflictEdge=new Edge;
FrameTable *FrameTbl=new FrameTable;
TDMATable *TDMA_Tbl=new TDMATable;
PacketBuffer* Buffer=new PacketBuffer;
Node* SetHead=new Node;
Node* Head=new Node;
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
int nodenum=0;						//所有node總數
int nodelevel1=0;					//Connection node數量		(由input取得)
int nodelevel2=0;					//Advertisement node數量		(由input取得)
int pktnum=0;						//每一node中的packet數量		(由input取得)
long int Timeslot=0;				//Schedule時 計數的time slot (由input取得)
long int Hyperperiod=0;				//Hyper-period				(由input取得)
double Maxrate=20;					//最高速度為20bytes/slot
double payload=20;					//payload 為 20bytes
int Maxbuffersize=4;				//Maxbuffersize 為 4個packets

double slotinterval=10;				//最短connection interval為10ms
double Minumum_interval=10;			//最短connection interval為10ms
double Connectioninterval=0;		//Conneciton inteval 只會在10ms~4000ms
double totalevent=0;				//Event數量
bool Meetflag=true;					//看是否meet deadline

/*========================================
		Power function parameter
========================================*/
double Vcc=3.3;					//BLE 驅動電壓
double I_sleep=0.000001;		//Sleep 電流 1uA
double Time_sleep=0.001;		//Sleep 時間 1ms (uint time)
double I_notify=0.008246;		//Notify 電流 8.246mA
double Time_notify=0.002675;	//Notify 時間 2.675ms
double I_Tran=0.014274;			//Transmission 電流 14.274mA
double Time_Tran=0.00049;		//Transmission 時間 0.49ms
double BatteryCapacity=0.230;	//230mAh (有其他篇章 是以540mAh <Energy Efficient MAC for Qos Traffic in Wireless Body Area Network)>
double unit=0.001;				//時間單位為1ms

/*========================================
		Create Object
========================================*/
EventInterval Interval_obj;
TDMA TDMA_obj;

/*========================================
			Main Function
========================================*/
int main(int argc, char* argv[]){
	/*
	cout<<"Type single node interval(0->Event, 1->MEI):";
	cin>>Service_interval;
	cout<<"Type TDMA table (0->single superslot, 1->Node base):";
	cin>>TDMAproposal;
	cout<<"Type TDMA with interval (0->EIMA, 1->Divide small interval by TDMA size):";
	cin>>Connection_Interval;
	cout<<"Type TDMA schedule (0->EDF, 1->TDMA table):";
	cin>>WriReq_Sche;
	*/	

	for(float U=MIN_Rate; U<=MAX_Rate; U+=inv_r){
		cout<<"Data Rate: "<<U<<endl;
		
		//-----------------------------Init setting for new set
		SetHead->lifetime=0;
		delete SetNode;SetNode=NULL;
		Meetcount=0;
		AverageE=0;
		totalevent=0;

		//-----------------------------Read setting file
		CreateFile(U,Set,argv[0]);//開啟WSNGEN 並且建立輸出檔案 (WSNFile.cpp)
		if(readsetting==1){
			ExperimentSetting(&Service_interval, &Connection_Interval, &WriReq_Sche);//做實驗設定輸出
		}

		/*===================================================
					在同一Data Rate下 跑Set數
		===================================================*/
		for(short int setnum=0;setnum<Set;setnum++){
			Meetflag=true;		//Reset Meet flag
			Timeslot=0;			//Reset time slot
			
			Hyperperiod=0;		
			totalevent=0;
			Cycle=NULL;
			pollingcount=1;

			/*==========================
				建立Linklist以及
				GEN的資料放進去
				(WSNStruct.cpp)
			==========================*/
			StructGEN();		
			
			/*==========================
				計算Service interval
			==========================*/
			Interval_obj.ServiceInterval_Algorithm(Service_interval);		//安排好各個node上的interval

			/*==========================
				Topology & TDMA assignment
			==========================*/
			TDMA_obj.Topology();						//建立傳輸目標node, 距離關係 & 建立conflict關係
			TDMA_obj.NodeColoring();					//安排各個node的color
			TDMA_obj.TDMA_Assignment(TDMAproposal);		//安排好TDMA_Tbl

			/*==========================
				計算Connection interval 
			==========================*/
			Interval_obj.ConnectionInterval_Algorithm(Connection_Interval);		//包含TDMA考量,做node上的interval修改 且含有Scan duration 計算

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
			==========================*/
			Head->RecvNode=NULL;		//Head 接收節點要設定為NULL
			Head->FrameSize=0;			//Head 會counting waiting time
			TDMA_Tbl->currslot=true;	//一開始第一個要為true
			Callbackclock=0;

			while(Timeslot<=Hyperperiod){
				PacketQueue();
				Schedule(WriReq_Sche,Service_interval);
				
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

	cout<<"Simulation Done :)"<<endl;
	system("PAUSE");
	return 0;
}

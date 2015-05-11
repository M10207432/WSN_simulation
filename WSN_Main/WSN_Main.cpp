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

#include "WSNFile.h"
#include "WSNStruct.h"
#include "ConnInterval.h"
#include "FlowSchedule.h"
#include "WSNEnergy.h"
#include "TDMA.h"

using namespace std;

/*=================================
		Global value
==================================*/
const float MIN_Uti=1.0;
const float MAX_Uti=5.0;
const short int Set=100;

short int Rateproposal=1;				//AssignRate()中的方法編號 0=>Event, 1=>TSB, 2=>DIF, 
bool preemptionenable=true;			//設定可否preemption

int Flowinterval=0;					//觸發進入flow的conneciton interval
int Pktsize=0;							//計算IntervalPower的pkt num
double DIFMinperiod=0;
double Meetcount=0;
double AverageE=0;
int TDMASlot=1;

Edge *HeadEdge=new Edge;
Edge *MainEdge=new Edge;
Edge *ConflictEdge=new Edge;
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
double Ie=0.07679763;		//傳輸峰值 電流
double Te=0.0002768;		//傳輸時間
double K=1;				//Rate power常數
double unit=0.01;		//時間單位為10ms
double TotalEnergy=0;
double parma=0.00191571992;
double parmb=24.4058498;

int main(){

	cout<<"Type Rateproposal(0->Event, 1->TSB):";
	cin>>Rateproposal;

	for(float U=MIN_Uti; U<=MAX_Uti; U++){
		delete SetNode;SetNode=NULL;
		Meetcount=0;
		AverageE=0;
		totalevent=0;

		CreateFile(U,Set);//開啟WSNGEN 並且建立輸出檔案 (WSNFile.cpp)
		
		/*===================================================
							在同一利用率下
							跑Set數
		===================================================*/
		for(short int setnum=0;setnum<Set;setnum++){
			Meetflag=true;
			Timeslot=0;
			TDMASlot=1;
			Hyperperiod=0;
			totalevent=0;

			/*==========================
				建立Linklist以及
				GEN的資料放進去
				(WSNStruct.cpp)
			==========================*/
			StructGEN();		
			
			/*==========================
			Topology & TDMA assignment
			(TDMASchedule.cpp)
			==========================*/
			Topology();			
			NodeColoring();		
			TDMA_Assignment();	

			/*==========================
				計算Connection interval
				(ConnInterval.cpp)
			==========================*/
			ConnAlgorithm(Rateproposal);
			

			/*=========================
				Schedulability test
			=========================*/
			Schedulability();

			/*==========================
				EDF scheduling
				(FlowSchedule.cpp)
			==========================*/			

			int FlowSlot=0;			//先由哪一Slot開始傳(TDMATable)
			bool Flow_flag=false;	//判斷有無碰撞(ConflictEdge)
			Headflow->pkt=NULL;		//一開始的flow中包含的封包設定為NULL
			
			while(Timeslot<Hyperperiod){
				PacketQueue();	
				BufferSet();
				MainSchedule(FlowSlot,Flow_flag);
				
				Timeslot++;
			}
			
			/*==========================
					END
			==========================*/
			SaveFile(setnum);//(WSNFile.cpp)

		}//Set End

		SaveSet(Set);//(WSNFile.cpp)
	}

	system("PAUSE");
	return 0;
}

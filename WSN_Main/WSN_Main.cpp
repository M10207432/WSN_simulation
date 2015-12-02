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

short int readsetting=1;					//�O�_�nŪ�����aSetting.txt
short int Rateproposal=1;				//AssignRate()������k�s�� 0=>Event, 1=>MEI, 2=>DIF, 3=>Lazy and 4=>min period<2,3�ݩ��@node�W���վ�>
short int TDMA_Rateproposal=2;			//TDMA�Mconnection interval�W���ե� 0=>LDC(�U��3), 1=>��̤pinterval��TDMA size, 2=>��lifetime ratio
short int TDMAscheduleproposal=2;		//Gateway �q��node�ǿ鶶�� 0=>��EDF�Ƶ{ 1=>������TDMA���ǿ� 2=>EIF 3=>Polling

bool sche_flag=false;					//�O�_�n����schedulability
short int TDMAproposal=0;				//TDMA��assign��k 0=>�ۤv����k(�u���@��superslot), 1=>Node base��k (�|�A����[�Jsuperslot)
int EXECBclock=100;						//Lazy Timer
/*=================================
		Global value
==================================*/
bool preemptionenable=true;			//�]�w�i�_preemption
int Flowinterval=0;					//Ĳ�o�i�Jflow��conneciton interval
int Pktsize=0;						//�p��IntervalPower��pkt num
double DIFMinperiod=0;
double Meetcount=0;
double AverageE=0;
int TDMASlot=1;
int overheadcount=6;				//�ʺA����interval�ɻݭn����6��interval�~���ܰ�
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
double Maxrate=20;					//�̰��t�׬�20bytes/slot
double payload=20;					//payload �� 20bytes
int Maxbuffersize=4;				//Maxbuffersize �� 4��packets
double timeslot=10;					//���ɶ���10ms

double slotinterval=10;				//Time slot���Z��10ms
double Connectioninterval=0;		//Conneciton inteval �u�|�b10ms~4000ms
double totalevent=0;				//Event�ƶq
bool Meetflag=true;					//�ݬO�_meet deadline
/*========================================
		Power function parameter
========================================*/
double Vcc=3.3;			//BLE �X�ʹq��

double I_sleep=0.000001;	//Sleep �q�y 1uA
double Time_sleep=0.01;		//Sleep �ɶ� 10ms (uint time)
double I_notify=0.0082463;	//Notify �q�y 8.2463mA
double Time_notify=0.002775;	//Notify �ɶ� 2.775ms
double I_Tran=0.0142744;	//Transmission �q�y 14.2744mA
double Time_Tran=0.00049;	//Transmission �ɶ� 0.49ms
double BatteryCapacity=0.230; //230mAh

double Ie=0.07679763;		//�ǿ�p�� �q�y
double Te=0.0002768;		//�ǿ�ɶ�
double K=1;				//Rate power�`��
double unit=0.01;		//�ɶ���쬰10ms
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

		CreateFile(U,Set,argv[0]);//�}��WSNGEN �åB�إ߿�X�ɮ� (WSNFile.cpp)
		if(readsetting==1){
			ExperimentSetting(&Rateproposal, &TDMA_Rateproposal, &TDMAscheduleproposal);//������]�w��X
		}
		/*===================================================
							�b�P�@�Q�βv�U
							�]Set��
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
				�إ�Linklist�H��
				GEN����Ʃ�i�h
				(WSNStruct.cpp)
			==========================*/
			StructGEN();		
			
			/*==========================
				�p��Connection interval 
					& Adv interval
			==========================*/
			Interval_obj.Algorithm(Rateproposal);		//�w�Ʀn�U��node�W��interval

			/*==========================
			Topology & TDMA assignment
			==========================*/
			TDMA_obj.Topology();
			TDMA_obj.NodeColoring();				
			TDMA_obj.TDMA_Assignment(TDMAproposal);		//�w�ƦnTDMA_Tbl

			/*==========================
			Interval & TDMA adjustment 
			==========================*/
			Interval_obj.Interval_TDMA_Algorithm(TDMA_Rateproposal);		//�]�tTDMA�Ҷq,��node�W��interval�ק� �B�t��Scan duration �p��
			Interval_obj.ConnectionPriority();								//�s������,�]�wEventTime

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
			Head->RecvNode=NULL;		//Head �����`�I�n�]�w��NULL
			Head->FrameSize=0;
			TDMA_Tbl->currslot=true;	//�@�}�l�Ĥ@�ӭn��true
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

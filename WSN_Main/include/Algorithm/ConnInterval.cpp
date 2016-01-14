#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <math.h>
#include <map>
#include <memory>

#include "../Struct/WSNFile.h"
#include "../Struct/WSNStruct.h"
#include "ConnInterval.h"
#include "../Schedule/FlowSchedule.h"
#include "../Struct/WSNEnergy.h"

#undef  _ShowLog

using namespace std;

/*==============================================
				Construct
==============================================*/
EventInterval::EventInterval(){
	printf("Connection Interval Object\n");
}

/*==============================================
	選擇需要哪一個 Service interval 計算方式
	(Single node with varied data rate)
==============================================*/
void EventInterval::ServiceInterval_Algorithm(int Rateproposal){
	switch (Rateproposal){
	case 0:
		Event();		//每一node connection interval 都為 10ms
		break;
	case 1:
		MEI(NULL);		//用Demand bound計算 service interval
		break;
	case 2:
		DIF();			//用各個pkt區間計算與load計算rate給pkt，會再轉換成service interval
		break;
	case 4:
		Greedy();		//用最短minimum period當作service interval
		break;
	case 5:
		Single_IOS(IOS_ServiceInterval);
		break;
	default:
		break;
	}

	//Reset load
	for(Packet *pkt=Head->nextnd->pkt; pkt!=NULL; pkt=pkt->nextpkt){
		pkt->exeload=pkt->load;
		pkt->exehop=pkt->hop;
	}
	
}

/*==============================================
	選擇需要哪一個 Connection interval 方法
	(Mulitple node with collision constrint)
==============================================*/
void EventInterval::ConnectionInterval_Algorithm(int proposal){

	if((nodelevel1+nodelevel2)!=1){
		switch (proposal){
		case 0:
			LDC();				//各個service interval除上node1level作為weight，分配connection interval
			break;	
		case 1:
			IntervalDivide();	//用minimum service interval除上node1level作為weight，分配connection interval
			break;
		case 2:
			EIMA();				//用avg current作為weight，分配connection interval
			break;
		case 3:
			Multiple_IOS(IOS_ConnectionInterval);
			break;
		default:
			break;
		}

		//確認每一node interval不小於10ms
		for(Node* node=Head->nextnd; node!=NULL; node=node->nextnd){
			if(node->eventinterval<Minumum_interval){
				node->eventinterval=Minumum_interval;
			}
		}
	}else{
		//-------------------------------建立FrameTable 並且assign connection interval
		double frameid=1;
		FrameTbl=new FrameTable;
		FrameTable* Ftbl=FrameTbl;
	
		for(TDMATable* tbl=TDMA_Tbl; tbl!=NULL; tbl=tbl->next_tbl){
			if(tbl->slot==frameid && tbl->n1->SendNode==Head){
				//Init setting
				Ftbl->id=frameid++;
				Ftbl->Currentflag=false;
				Ftbl->arrival=0;
				Ftbl->ConnNode= tbl->n1;					//Assign node給此Frame

				//Waiting time & period setting
				Ftbl->Period=tbl->n1->eventinterval;		//Assign Service interval as period
				Ftbl->Deadline=tbl->n1->eventinterval;		//Assign Service interval as Deadline
				Ftbl->Size=tbl->n1->eventinterval;		//Service inteval計算connection interval
				Ftbl->ConnNode->eventinterval=Ftbl->Size;	//更新node上的connection interval
				Ftbl->Utilization=Ftbl->Size/Ftbl->Period;	

				//建立下一個FrameTable
				Ftbl->next_tbl=new FrameTable;
				Ftbl->next_tbl->pre_tbl=Ftbl;
				Ftbl=Ftbl->next_tbl;
			}
		}
		Ftbl->pre_tbl->next_tbl=NULL;
	}
}

/*==============================================
		每一node connection interval 都為 10ms
==============================================*/
void EventInterval::Event(){
	for (Node* n=Head->nextnd; n!=NULL; n=n->nextnd){
		node->eventinterval=Minumum_interval;
	}
}

/*==============================================
	Multiple event Energy Efficient Interval

	>先排好PacketQueue 給pkt
	>分別計算在Tslot之前的Demand_pkt Supply_pkt
	>Update Tslot
==============================================*/
void EventInterval::MEI(Node * MEINode){

	double Tc=0;			//最後得出的service interval
	double Tslot=0;			//基準測試時間點(比較Demand_pkt v.s. Supply_pkt)			
	double Demand_pkt=0;	//在Tslot之前所需的pkt數量
	double Supply_pkt=0;	//在Tslot之前系統提供的pkt數量
	bool doneflag=false;	//判斷node的所有pkt以計算完畢

	/*------------------------------------------
		先做好所有Conn/Adv Node
		上的connection/advertisement interval
	------------------------------------------*/
	PacketQueue();		//先排Ready Queue
	for (Node* n=Head->nextnd; n!=NULL; n=n->nextnd){
		
		//========================Init
		
		Packet *pkt=n->pktQueue;	//pkt已排序好 (按照deadline)
		if(MEINode!=NULL){
			pkt=MEINode->pktQueue;	//根據某node設定
		}

		Tslot=0;
		Demand_pkt=0;
		Supply_pkt=0;
		Tc=pkt->period;		//找Minperiod, 設定為Tc init
		Tslot=pkt->period;	//找Minperiod, 設定為Tslot init
		doneflag=false;

		//========================循序增加Tslot
		while(doneflag!=true){
			Demand_pkt=0;

			//算出在Tslot之前所需的pkt數量 (Packet 數量 --> Demand_pkt)----Step1
			pkt=n->pktQueue;
			while(pkt->period <= Tslot){
				Demand_pkt=Demand_pkt+(ceil(pkt->load/payload)*ceil(Tslot/pkt->period));	
				pkt=pkt->nodereadynextpkt;
				if(pkt==NULL)
					break;
			}

			//計算在Tslot之前所提供的Service interval----------------------Step2
			Supply_pkt=floor(Tslot/Tc)*double(Maxbuffersize);
			
			//比較Demand_pkt & Supply_pkt---------------------------------Step3
			while(Demand_pkt > Supply_pkt){
				Tc--;
				Supply_pkt=floor(Tslot/Tc)*double(Maxbuffersize);
			}
			
			//更新Time slot-----------------------------------------------Step4
			if(pkt!=NULL){
				Tslot=pkt->period;
			}else{
				doneflag=true;
			}
		}	

		//已經計算完此node所有pkt，assign Service Interval to node
		n->eventinterval=Tc;

		//因是根據某一node做計算，所以只需計算一次
		if(MEINode!=NULL){
			break;			
		}
	}
}

/*===========================
		比較組
	找各個區間(interval)
	<arrival -> period> 
step1:找各區間 完整的packet
step2:各區間的(packet->load加總) 除以 (interval) <interval會算區間內最大arrival與最大deadline且要排除以assign過的packet區間>
step3:計算各區間 rate 
step4:找出最大rate , 其在區間的packet assign 此rate
(找區間時要將有rate的區間時間拿掉)

map的
分配好每一packet的rate
===========================*/
void EventInterval::DIF(){
	PacketQueue();

	Packet * DIFpacket;
	map<double,map<double,DIFtable>> Table;	//二維map 內容格式為DIFtable map[arrival][deadline]
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
						if(a->first <= DIFpacket->arrival && DIFpacket->deadline <= p->first && DIFpacket->rate==0){ //尚未assigh rate且介於區間內
							Packet* tmpDIFpacket;
							double start=a->first;
							double end=p->first;
							
							//放入區間、區間內load總值 以及 此區間Density
							Table[a->first][p->first].length=p->first - a->first;	//length <deadline-arrival>
							while(start!=end){
								
								tmpDIFpacket=Head->nextnd->pkt;
								while(tmpDIFpacket!=NULL){
									//排除在此區間內assign過rate的packet
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

							Table[a->first][p->first].load=Table[a->first][p->first].load+DIFpacket->load*((p->first-a->first)/DIFpacket->period);
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

/*============================================
				Greedy
		找出node中最小period，
		將此period 視為service interval
============================================*/
void EventInterval::Greedy(){
	for(Node *n=Head->nextnd; n!=NULL; n=n->nextnd){

		//Find the minimum period
		Packet* Minpkt=NULL;
		for(Packet* pkt=n->pkt; pkt!=NULL; pkt=pkt->nodenextpkt){
			if(Minpkt==NULL){
				Minpkt=pkt;
			}else{
				if(Minpkt->period > pkt->period){
					Minpkt=pkt;
				}
			}
		}

		//Assign Service interval
		n->eventinterval=Minpkt->period;
	}
}

/*============================================
				IOS
		於defualt setting中是以固定時間設定
============================================*/
void EventInterval::Single_IOS(double ServiceInterval){
	for(Node* n=Head->nextnd; n!=NULL; n=n->nextnd){
		n->eventinterval=ServiceInterval;
	}
}

/*==============================================
			Low Duty Cycle
		每一個connection interval為
		各自Service interval除上nodelevel1
==============================================*/
void EventInterval::LDC(){
	//-------------------------------Assign給FrameTbl,只有Connection node為nodelevel1個用
	short int frameid=1;
	FrameTbl=new FrameTable;
	FrameTable* Ftbl=FrameTbl;

	//-------------------------------建立FrameTable 並且assign connection interval
	for(TDMATable* tbl=TDMA_Tbl; tbl!=NULL; tbl=tbl->next_tbl){
		if(tbl->slot==frameid && tbl->n1->SendNode==Head){	//確認順序 & 送給Master
			//Init setting
			Ftbl->id=frameid++;
			Ftbl->Currentflag=false;
			Ftbl->arrival=0;
			Ftbl->ConnNode= tbl->n1;					//Assign node給此Frame

			//Waiting time & period setting
			Ftbl->Period=tbl->n1->eventinterval;		//Assign Service interval as period
			Ftbl->Deadline=tbl->n1->eventinterval;		//Assign Service interval as Deadline
			Ftbl->Size=floor(Ftbl->Period/nodelevel1);	//Service inteval計算connection interval
			Ftbl->ConnNode->eventinterval=Ftbl->Size;	//更新node上的connection interval
			Ftbl->Utilization=Ftbl->Size/Ftbl->Period;		

			//建立下一個FrameTable
			Ftbl->next_tbl=new FrameTable;
			Ftbl->next_tbl->pre_tbl=Ftbl;
			Ftbl=Ftbl->next_tbl;
		}
	}
	Ftbl->pre_tbl->next_tbl=NULL;

	//------------------------------------確認Frame數量不比nodelevel1多
	if(--frameid>nodelevel1){
		printf("The Frame size is larger than nodelevel1, the FrameTable is error\n");
		system("PAUSE");
	}
	
	//------------------------------------Print 出資訊
#ifdef _Showlog
	for(Node* node=Head->nextnd; node!=NULL; node=node->nextnd){
		cout<<	"Node"<<node->id<<"=> "<<
				"Interval="<<node->eventinterval<<", "<<
				"Slot="<<node->color<<", "<<
				"Scan Duaration="<<node->ScanDuration<<", "<<
				"SendNode="<<node->SendNode->id<<endl;
	}
	for(FrameTable* Ftbl=FrameTbl; Ftbl!=NULL; Ftbl=Ftbl->next_tbl){
		cout<<"Frame"<<Ftbl->id<<" =>"<<
			" size="<< Ftbl->Size<<","<<
			" period="<<Ftbl->Period<<endl;
	}
#endif
}
/*==================================
		IntervalDivide
	找最小的Service interval除上nodelevel1
	當每node的connection interval
==================================*/
void EventInterval::IntervalDivide(){

	double Mininterval=Hyperperiod;	//最小的Service interval
	short int frameid=1;
		
	//找最小的Service interval
	for(Node *node=Head->nextnd; node!=NULL; node=node->nextnd){
		if(node->SendNode==Head && node->eventinterval<Mininterval){
			Mininterval=node->eventinterval;
		}
	}

	//-------------------------------建立FrameTable 並且assign connection interval
	FrameTbl=new FrameTable;
	FrameTable* Ftbl=FrameTbl;

	for(TDMATable* tbl=TDMA_Tbl; tbl!=NULL; tbl=tbl->next_tbl){
		if(tbl->slot==frameid && tbl->n1->SendNode==Head){ //確認順序 & 送給Master
			//Init setting
			Ftbl->id=frameid++;
			Ftbl->Currentflag=false;
			Ftbl->arrival=0;
			Ftbl->ConnNode= tbl->n1;					//Assign node給此Frame

			//Waiting time & period setting
			Ftbl->Period=tbl->n1->eventinterval;		//Assign Service interval as period
			Ftbl->Deadline=tbl->n1->eventinterval;		//Assign Service interval as Deadline
			Ftbl->Size=floor(Mininterval/nodelevel1);	//Service inteval計算connection interval (用Minperiod)
			Ftbl->ConnNode->eventinterval=Ftbl->Size;	//更新node上的connection interval
			Ftbl->Utilization=Ftbl->Size/Ftbl->Period;		

			//建立下一個FrameTable
			Ftbl->next_tbl=new FrameTable;
			Ftbl->next_tbl->pre_tbl=Ftbl;
			Ftbl=Ftbl->next_tbl;
		}
	}
	Ftbl->pre_tbl->next_tbl=NULL;

	//------------------------------------確認Frame數量不比nodelevel1多
	if(--frameid>nodelevel1){
		printf("The Frame size is larger than three, the FrameTable is error\n");
		system("PAUSE");
	}
	
	//-------------------------------Scan duration 計算
	/*
	for(Node* node=Head->nextnd; node!=NULL; node=node->nextnd){
		devicenum=0;
		MaxAdvinter=0;
		if(node->SendNode==Head){
			//先找Device 數量 & 對應最大廣播間距
			Node *BelongNode=Head->nextnd;
			while(BelongNode!=NULL){
				if(node==BelongNode->SendNode){
					devicenum++;			
					if(BelongNode->eventinterval>MaxAdvinter){
						MaxAdvinter=BelongNode->eventinterval;
					}
				}
				BelongNode=BelongNode->nextnd;
			}

			//計算Scan duration
			node->ScanDuration=node->SCAN_Compute(	node->ScanWin,
													node->ScanInter,
													MaxAdvinter,
													devicenum);
		}else{
			node->ScanDuration=0;
		}
	}
	*/
	//---------------------------------Print 出資訊
	
	#ifdef _Showlog
	for(Node* node=Head->nextnd; node!=NULL; node=node->nextnd){
		cout<<	"Node"<<node->id<<"=> "<<
				"Interval="<<node->eventinterval<<", "<<
				"Slot="<<node->color<<", "<<
				"Scan Duaration="<<node->ScanDuration<<", "<<
				"SendNode="<<node->SendNode->id<<endl;
	}
	for(FrameTable* Ftbl=FrameTbl; Ftbl!=NULL; Ftbl=Ftbl->next_tbl){
		cout<<"Frame"<<Ftbl->id<<" =>"<<
			" size="<< Ftbl->Size<<","<<
			" period="<<Ftbl->Period<<endl;
	}
	#endif
}

/*==================================
	Energy Efficeincy Multilple Access
	
	weight來決定要取多少percent的
	service interval
	(weight的計算用avg current)
==================================*/
void EventInterval::EIMA(){

	short int frameid=1;
	double Sum_weight=0;

	//若有interval大於4秒 (400 unit為10ms)，要往前scaling所以unit會為1ms
	for(Node* n=Head->nextnd; n!=NULL; n=n->nextnd){
		if(n->eventinterval>400){
			unit=0.001;		
		}
	}

	//按照Node avg current做interval校正
	for(Node* n=Head->nextnd; n!=NULL; n=n->nextnd){
		
		//=======================計算Service interval的average current ----------------Weight v1
		double avgcurrent=(((I_notify*Time_notify)+(I_sleep*(n->eventinterval*unit-Time_notify)))/(n->eventinterval*unit));
		
		//=======================計算Service interval的sequence average current -------Weight v2
		double Qc=0, Qt=0;
		double totalcnt=0;
		double Seqavgcurrent=0;

		Qc=(Hyperperiod/n->eventinterval)*I_notify*Time_notify;			//單一event所造成的current consumption
		for(Packet* pkt=n->pkt; pkt!=NULL; pkt=pkt->nodenextpkt){
			double count=((Hyperperiod/pkt->period)*ceil(pkt->load/payload));
			totalcnt+=count;
		}
		Qt=(totalcnt-(Hyperperiod/n->eventinterval))*I_Tran*Time_Tran;	//連傳event所造成的current consumption
		Seqavgcurrent=((Qc+Qt)+I_sleep*(Hyperperiod*unit-(Qc/I_notify)-(Qt/I_Tran))) / (Hyperperiod*unit);

		//=======================計算total weight
		n->EIMA_avgcurrent=avgcurrent;
		Sum_weight=Sum_weight+n->EIMA_avgcurrent;
	}

	//-------------------------------建立FrameTable 並且assign connection interval
	FrameTbl=new FrameTable;
	FrameTable* Ftbl=FrameTbl;
	
	for(TDMATable* tbl=TDMA_Tbl; tbl!=NULL; tbl=tbl->next_tbl){
		if(tbl->slot==frameid && tbl->n1->SendNode==Head){
			//Init setting
			Ftbl->id=frameid++;
			Ftbl->Currentflag=false;
			Ftbl->arrival=0;
			Ftbl->ConnNode= tbl->n1;					//Assign node給此Frame

			//Waiting time & period setting
			Ftbl->Period=tbl->n1->eventinterval;		//Assign Service interval as period
			Ftbl->Deadline=tbl->n1->eventinterval;		//Assign Service interval as Deadline
			Ftbl->Size=floor(((tbl->n1->EIMA_avgcurrent)/Sum_weight)
						* tbl->n1->eventinterval);		//Service inteval計算connection interval
			Ftbl->ConnNode->eventinterval=Ftbl->Size;	//更新node上的connection interval
			Ftbl->Utilization=Ftbl->Size/Ftbl->Period;	

			//建立下一個FrameTable
			Ftbl->next_tbl=new FrameTable;
			Ftbl->next_tbl->pre_tbl=Ftbl;
			Ftbl=Ftbl->next_tbl;
		}
	}
	Ftbl->pre_tbl->next_tbl=NULL;

	/*=======================================
			加入Demand bound
		找<最小period的size> & <最大size>
		若<最小period的size> + <最大size>大於 <最小period>
		每一connection interval調整為<最小period除與2>
	=======================================*/
	if(EIMADemand_flag){
		double Minperiod_size=0;
		double Minperiod_period=-1;
		double _Maxsize=0;
		for(FrameTable* Ftbl=FrameTbl; Ftbl!=NULL; Ftbl=Ftbl->next_tbl){
			//Find size of minimum period
			if(Minperiod_period==-1){
				Minperiod_period=Ftbl->Period;
				Minperiod_size=Ftbl->Size;
			}else{
				if(Ftbl->Period<Minperiod_period){
					Minperiod_period=Ftbl->Period;
					Minperiod_size=Ftbl->Size;
				}
			}

			//Find max size
			if(Ftbl->Size>_Maxsize){
				_Maxsize=Ftbl->Size;
			}
		}

		if((_Maxsize+Minperiod_size)>Minperiod_period){
			for(FrameTable* Ftbl=FrameTbl; Ftbl!=NULL; Ftbl=Ftbl->next_tbl){
				Ftbl->Size=Minperiod_period/2;
				Ftbl->ConnNode->eventinterval=Ftbl->Size;	//更新node上的connection interval
			}	
		}
	}

	//------------------------------------確認Frame數量不比nodelevel1多
	if(--frameid>nodelevel1){
		printf("The Frame size is larger than three, the FrameTable is error\n");
		system("PAUSE");
	}

	//---------------------------------Print 出資訊
#ifdef _Showlog
	for(Node* node=Head->nextnd; node!=NULL; node=node->nextnd){
		cout<<	"Node"<<node->id<<"=> "<<
				"Interval="<<node->eventinterval<<", "<<
				"Slot="<<node->color<<", "<<
				"Scan Duaration="<<node->ScanDuration<<", "<<
				"SendNode="<<node->SendNode->id<<endl;
	}
	for(FrameTable* Ftbl=FrameTbl; Ftbl!=NULL; Ftbl=Ftbl->next_tbl){
		cout<<"Frame"<<Ftbl->id<<" =>"<<
			" size="<< Ftbl->Size<<","<<
			" period="<<Ftbl->Period<<endl;
	}
#endif
}


void EventInterval::Multiple_IOS(double ConnectionInterval){
	for(Node* n=Head->nextnd; n!=NULL; n=n->nextnd){
		n->eventinterval=ConnectionInterval;
	}

	//-------------------------------建立FrameTable 並且assign connection interval
	short int frameid=1;
	FrameTbl=new FrameTable;
	FrameTable* Ftbl=FrameTbl;
	
	for(TDMATable* tbl=TDMA_Tbl; tbl!=NULL; tbl=tbl->next_tbl){
		if(tbl->slot==frameid && tbl->n1->SendNode==Head){
			//Init setting
			Ftbl->id=frameid++;
			Ftbl->Currentflag=false;
			Ftbl->arrival=0;
			Ftbl->ConnNode= tbl->n1;					//Assign node給此Frame

			//Waiting time & period setting
			Ftbl->Period=tbl->n1->eventinterval;		//Assign Service interval as period
			Ftbl->Deadline=tbl->n1->eventinterval;		//Assign Service interval as Deadline
			Ftbl->Size=tbl->n1->eventinterval;		//Service inteval計算connection interval
			Ftbl->ConnNode->eventinterval=Ftbl->Size;	//更新node上的connection interval
			Ftbl->Utilization=Ftbl->Size/Ftbl->Period;	

			//建立下一個FrameTable
			Ftbl->next_tbl=new FrameTable;
			Ftbl->next_tbl->pre_tbl=Ftbl;
			Ftbl=Ftbl->next_tbl;
		}
	}
	Ftbl->pre_tbl->next_tbl=NULL;
}
/*=====================================
	對需要Scan 的Conn Node
	做Scan duration 計算，以及Tc重新計算
=====================================*/
void EventInterval::IntervalReassign(){
	double MaxAdvinter=0;	//最長廣播間距
	short int devicenum=0;	//Adv Device 數量

	for(Node *node=Head->nextnd; node!=NULL; node=node->nextnd){
		if(node->SendNode==Head){
			MaxAdvinter=0;
			devicenum=0;

			//先找Device 數量 & 對應最大廣播間距
			for(Node *AdvNode=Head->nextnd; AdvNode!=NULL; AdvNode=AdvNode->nextnd){
				if(node==AdvNode->SendNode){
					AdvNode->pkt->node=node;	//主要是要將所屬node轉為對應Conn Node，目的為在PacketQueue中 要做node上readynextpkt安排，且MEI重新計算
												//做完之後要設定回來 (Node上的nodenextpkt並未改變)
					devicenum++;			
					if(AdvNode->eventinterval>MaxAdvinter){
						MaxAdvinter=AdvNode->eventinterval;
					}
				}
			}

			if(devicenum>0){
				//計算Scan duration (int ScanWin, int ScanInter, int AdvInter, int device)
				node->ScanDuration=node->SCAN_Compute(	node->ScanWin,
														node->ScanInter,
														MaxAdvinter,
														devicenum);
				node->EXEScanDuration=node->ScanDuration;
				node->ScanFlag=false;

				MEI(node);

				for(Node *AdvNode=Head->nextnd; AdvNode!=NULL; AdvNode=AdvNode->nextnd){
					if(node==AdvNode->SendNode){
						AdvNode->pkt->node=AdvNode;
					}
				}
			}
		}
	}
}

void EventInterval::Rate_TO_Interval(int defaultMinperiod){
	//判斷是否需要改變Service interval
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


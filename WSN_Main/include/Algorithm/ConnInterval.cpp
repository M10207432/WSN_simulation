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

#undef  _ShowLog

using namespace std;

EventInterval::EventInterval(){
	printf("Connection Interval Object\n");
}
/*==============================================
		選擇需要哪一個 
		Connection interval 計算方式
==============================================*/
void EventInterval::Algorithm(int Rateproposal){
	switch (Rateproposal)
	{
	case 0:
		Event();
		break;
	case 1:
		MEI(NULL);
		break;
	case 2:
		DIF();
		break;
	default:
		break;
	}

	for(Packet *pkt=Head->nextnd->pkt; pkt!=NULL; pkt=pkt->nextpkt){
		pkt->exeload=pkt->load;
		pkt->exehop=pkt->hop;
	}
	
}

/*==============================================
		選擇需要哪一個 
		Connection interval 結合TDMA修正方式
==============================================*/
void EventInterval::Interval_TDMA_Algorithm(int proposal){
	switch (proposal)
	{
	case 0:
		EIMA();
		break;
	case 1:
		IntervalDivide();
		break;
	default:
		break;
	}

	//確認每一node interval不小於1
	for(Node* node=Head->nextnd; node!=NULL; node=node->nextnd){
		if(node->eventinterval<1){
			node->eventinterval=1;
		}
	}
}

/*==============================================
		每一node connection interval 都為 1
==============================================*/
void EventInterval::Event(){
	node=Head->nextnd;
	while(node!=NULL){
		node->eventinterval=1;
		node=node->nextnd;
	}
}
/*==============================================
(小於兩倍Minperiod的pkt size)	->Minsize
	(最大load的pkt size)			->Maxsize

	若(2*Minsize+Maxsize)大於兩倍Buffersize
		Connection interval為Minperiod/2
	否
		依照間隔buffersize做計算
==============================================*/
void EventInterval::MEI(Node * MEINode){
	Packet *TSBpktQ=ReadyQ;
	Packet *TSBpkt=Head->nextnd->pkt;
	double Tc=0;
	
	/*------------------------------------------
		先做好所有Conn/Adv Node
		上的connection/advertisement interval
	------------------------------------------*/
	PacketQueue();		//先排Ready Queue
	Node *TSBnode=Head->nextnd;
	while(TSBnode!=NULL){
		Packet *TSBpkt=TSBnode->pktQueue;

		if(MEINode!=NULL){
			TSBpkt=MEINode->pktQueue;
		}
		int nodehop=TSBnode->hop;
		double Totalsize=0;
		double PacketSize=0;
		double totalevent=0;
		double Tslot=0;
		bool doneflag=false;

		//======================找Minperiod, 設定為Tc init
		Tc=TSBpkt->period;

		//======================分析每一period下, 是否能meet deadline
		Tslot=TSBpkt->period;
	
		while(doneflag!=true){
			Totalsize=0;

			//算出所需buffer量 (Packet 數量 --> Totalsize)
			TSBpkt=TSBnode->pktQueue;
			while(TSBpkt->period <= Tslot){
				Totalsize=Totalsize+(ceil(TSBpkt->load/payload)*ceil(Tslot/TSBpkt->period));	
				TSBpkt=TSBpkt->nodereadynextpkt;
				if(TSBpkt==NULL)
					break;
			}

			//計算需要的event數量，反推所需buffer量 (解決Hop不連續上的問題)
			if(nodehop>1){
				totalevent=ceil(Totalsize/Maxbuffersize);
				Totalsize=(totalevent*Maxbuffersize)*nodehop;
			}

			//計算Connection interval
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

		if(MEINode!=NULL){
			break;
		}
	}
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

/*===========================
		比較組
	找各個區間(interval)
	<arrival -> period> 
step1:找各區間 完整的packet
step2:各區間的(packet->load加總) 除以 (interval) <interval會算區間內最大arrival與最大deadline且要排除以assign過的packet區間>
step3:計算各區間 rate 
step4:找出最大rate , 其在區間的packet assign 此rate
(找區間時要將有rate的區間時間拿掉)

分配好每一packet的rate
===========================*/
void EventInterval::DIF(){
	PacketQueue();

	DIFMinperiod=ReadyQ->readynextpkt->period;
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

void EventInterval::Rate_TO_Interval(int defaultMinperiod){
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

/*==============================================
		結合 Energy Efficiency Interval & TDMA
		做進一步確認可在最deadline前完成

		=>會修正各node上的interval
		=>Assign好各自SCAN duration
==============================================*/
void EventInterval::EIMA(){
	bool EIMAEDF_flag=true;	//測試EIMA EDF 上對於inteval上的調整

	double TDMASize=1;	//TDMASIZE
	double Mininterval=Hyperperiod;	//最小的interval
	double tmpinterval=Hyperperiod;
	int devicenum=0;	//device 數量
	int MaxAdvinter=0;	//對應廣播群中最大的廣播間距
	short int frameid=1;

	//-------------------------------Assign給FrameTbl,只有Connection node為3個用
	FrameTbl=new FrameTable;
	FrameTable* Ftbl=FrameTbl;

	for(TDMATable* tbl=TDMA_Tbl; tbl!=NULL; tbl=tbl->next_tbl){
		if(tbl->slot==frameid && tbl->n1->SendNode==Head){
			Ftbl->id=frameid++;
			Ftbl->Currentflag=false;
			Ftbl->arrival=0;
			Ftbl->Period=tbl->n1->eventinterval;
			Ftbl->Deadline=tbl->n1->eventinterval;
			
			//Ftbl->Size=Ftbl->Period/nodelevel1;
			Ftbl->Size=floor(Ftbl->Period/nodelevel1);

			Ftbl->Utilization=1/nodelevel1;
			
			Ftbl->ConnNode= tbl->n1;			//指向此Conn Node
			tbl->n1->eventinterval=Ftbl->Size;	//更新node上的connection interval

			Ftbl->next_tbl=new FrameTable;
			Ftbl->next_tbl->pre_tbl=Ftbl;
			Ftbl=Ftbl->next_tbl;
		}
	}
	Ftbl->pre_tbl->next_tbl=NULL;
	//-------------------------------------Assign 給 AdvNode使用
	/*
	for(FrameTable* Ftbl=FrameTbl; Ftbl!=NULL; Ftbl=Ftbl->next_tbl){
		for(Node* n=Head->nextnd; n!=NULL; n=n->nextnd)	{
			if(n->SendNode!=Head){
				
			}
		}
	}
	*/
	if(--frameid>3){
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
/*==================================
		Small Interval Divide
==================================*/
void EventInterval::IntervalDivide(){

	double TDMASize=1;	//TDMASIZE
	double Mininterval=Hyperperiod;	//最小的interval
	double tmpinterval=Hyperperiod;
	int devicenum=0;	//device 數量
	int MaxAdvinter=0;	//對應廣播群中最大的廣播間距
	short int frameid=1;
	
	//-------------------------------考量TDMA架構，做Node interval上的優化
	//找TDMA size
	for(TDMATable* tbl=TDMA_Tbl; tbl!=NULL;tbl=tbl->next_tbl){
		if(tbl->slot>TDMASize)
			TDMASize=tbl->slot;
	}
	
	//找最小的connection interval
	for(Node *node=Head->nextnd; node!=NULL; node=node->nextnd){
		if(node->SendNode==Head && node->eventinterval<Mininterval){
			Mininterval=node->eventinterval;
		}
	}

	//修正connection interval
	for(Node *node=Head->nextnd; node!=NULL; node=node->nextnd){
		if(node->SendNode==Head){
			node->eventinterval=(Mininterval/TDMASize);
			//node->eventinterval=node->eventinterval/TDMASize;
		}
	}

	//FrameTbl建立
	FrameTbl=new FrameTable;
	FrameTable* Ftbl=FrameTbl;

	for(TDMATable* tbl=TDMA_Tbl; tbl!=NULL; tbl=tbl->next_tbl){
		if(tbl->slot==frameid && tbl->n1->SendNode==Head){
			Ftbl->id=frameid++;
			Ftbl->Currentflag=false;
			Ftbl->Period=tbl->n1->eventinterval;
			Ftbl->Deadline=tbl->n1->eventinterval;
			Ftbl->Size=tbl->n1->eventinterval;
			Ftbl->Utilization=0.3;

			Ftbl->ConnNode= tbl->n1;			//指向此Conn Node
			tbl->n1->eventinterval=Ftbl->Size;	//更新node上的connection interval

			Ftbl->next_tbl=new FrameTable;
			Ftbl->next_tbl->pre_tbl=Ftbl;
			Ftbl=Ftbl->next_tbl;
		}
	}
	Ftbl->pre_tbl->next_tbl=NULL;

	if(--frameid>3){
		printf("The Frame size is larger than three, the FrameTable is error\n");
		system("PAUSE");
	}
	
	//-------------------------------Scan duration 計算
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
		連接順序
==================================*/
void EventInterval::ConnectionPriority(){
	Node* assignnode=NULL;
	double Max_eventinterval=0;
	double connorder=0;

	while(Max_eventinterval!=2000000000){
		Max_eventinterval=2000000000;

		for(Node* node=Head->nextnd; node!=NULL; node=node->nextnd){
			if(node->SendNode==Head && node->EventTime==-1){
				if(node->eventinterval<Max_eventinterval){
					assignnode=node;	
					Max_eventinterval=node->eventinterval;
				}
			}
		}

		if(assignnode!=NULL){
			assignnode->EventTime=connorder++;
		}
		assignnode=NULL;
	}
}
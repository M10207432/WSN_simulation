#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <math.h>
#include <map> 
#include <memory>
#include "WSNFile.h"
#include "WSNStruct.h"
#include "ConnInterval.h"
#include "FlowSchedule.h"

using namespace std;

ConnectionInterval::ConnectionInterval(){
	printf("Connection Interval Obj\n");
}
/*==============================================
		選擇需要哪一個 
		Connection interval 計算方式
==============================================*/
void ConnectionInterval::ConnAlgorithm(int Rateproposal){
	switch (Rateproposal)
	{
	case 0:
		Event();
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

	packet=Head->nextnd->pkt;
	while(packet!=NULL){
		packet->exeload=packet->load;
		packet->exehop=packet->hop;

		packet=packet->nextpkt;
	}
}
/*==============================================
		每一node connection interval 都為 1
==============================================*/
void ConnectionInterval::Event(){
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
void ConnectionInterval::TSB(){
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
		}
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
void ConnectionInterval::DIF(){
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

void ConnectionInterval::Rate_TO_Interval(int defaultMinperiod){
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

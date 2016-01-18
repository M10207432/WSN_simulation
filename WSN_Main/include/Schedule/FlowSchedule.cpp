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

#include "../Struct/WSNFile.h"
#include "../Struct/WSNStruct.h"
#include "../Algorithm/ConnInterval.h"
#include "FlowSchedule.h"
#include "../Struct/WSNEnergy.h"

#undef  _ShowLog

using namespace std;

/*==========================
	判斷Write-Request方法
==========================*/
void Schedule(int WRSche, int ServiceInterval){
	
	CheckPkt();	//確認有沒有miss
	/*==========================
			判斷是單一node
			或是multi node
	==========================*/
	if((nodelevel1+nodelevel2)!=1){
		switch (WRSche){
		case 0:
			NPEDF();
			break;
		case 1:
			RoundRobin();
			break;
		case 2:
			EIF();
			break;
		case 3:
			Polling();
			break;
		}

		Write_Request();
	}else{
		SingleNodeSchedule(ServiceInterval);
	}
}

/*==========================================
			Arrange Queue
{ReadyQ & WaitQ is assign by pkt->readyflag}

	先建立總total的Packet Queue
	在分配給各自的node
==========================================*/
void PacketQueue(){
	/*--------------------------------
		Init ready linklist
	--------------------------------*/
	ReadyQ->readynextpkt=NULL;		//將Global ReadyQ下一ready為NULL

	for(Node *n=Head->nextnd; n!=NULL; n=n->nextnd){
		n->pktQueue=NULL;
	}
	for(Packet* pkt=Head->nextnd->pkt; pkt!=NULL; pkt=pkt->nextpkt){
		pkt->readynextpkt=NULL;		//下一ready設定為NULL
		pkt->readyprepkt=NULL;		//上一ready設定為NULL
		pkt->nodereadynextpkt=NULL;
		pkt->nodereadyprepkt=NULL;

		pkt->searchdone=0;			//尚未找過
	}
	
	/*--------------------------------
			Setting Global Queue
	--------------------------------*/
	ReadyQ_overflag=0;
	while(!ReadyQ_overflag){
		
		/*--------------------------
		找最小deadline pkt給packet
		--------------------------*/
		packet=NULL;
		for(Packet* pkt=Head->nextnd->pkt; pkt!=NULL; pkt=pkt->nextpkt){
			//尚未search過
			if(pkt->searchdone==0){
				//assign 最小deadline pkt給packet
				if(packet==NULL){
					packet=pkt;
				}else{
					if(pkt->deadline < packet->deadline){
						packet=pkt;
					}
				}	
			}
		}
		
		/*--------------------------
		將最小Deadline放入ReadyQ當中
		並標記已尋找過(searchdone=1)
		--------------------------*/
		if(packet!=NULL){
			packet->searchdone=1;

			if(Timeslot>=packet->arrival){
				packet->readyflag=1;			//封包 Arrival
				packet->node->arrival_flag=1;	//node 含有arrival flag
			}else{
				packet->readyflag=0;//封包 尚未Arrival
			}

			//Assign 放入ReadyQ
			for(Packet* Rpkt=ReadyQ; Rpkt!=NULL; Rpkt=Rpkt->readynextpkt){
				if(Rpkt->readynextpkt==NULL){
					Rpkt->readynextpkt=packet;
					packet->readyprepkt=Rpkt;
					packet->readynextpkt=NULL;
				}
			}
			
		}else{
			ReadyQ_overflag=1;
		}

	}
	
	/*--------------------------
		建立各自node上的
		ready packet queue
	--------------------------*/
	
	for(Packet* pkt=ReadyQ->readynextpkt; pkt!=NULL; pkt=pkt->readynextpkt){ //照packet priority
		
		//pkt所屬node
		node=pkt->node;
		
		//找node->pktQueue 最後一個
		if(node->pktQueue==NULL){
			node->pktQueue=pkt;			
			pkt->nodereadynextpkt=NULL;
			pkt->nodereadyprepkt=NULL;
		}else{
			Packet *tmpreadypkt=node->pktQueue;

			while(tmpreadypkt->nodereadynextpkt!=NULL){
				tmpreadypkt=tmpreadypkt->nodereadynextpkt;
			}

			tmpreadypkt->nodereadynextpkt=pkt;				
			pkt->nodereadynextpkt=NULL;
			pkt->nodereadyprepkt=tmpreadypkt;
		}
	}
}

/*=========================
	Admission Control
	(計算原本的Tc 與 
	TDMA 因素造成的間隔)
=========================*/
void Schedulability(){
	bool reassign=true;
	double BlockingTime=0;
	double minperiod=10000000;
	double maxperiod=0;
	double U_bound=0;
	double sub=1000000000;
	FrameTable *EndFrame;

	for (FrameTable* Ftbl=FrameTbl; Ftbl!=NULL; Ftbl=Ftbl->next_tbl){
		if(Ftbl->Size>BlockingTime){
			BlockingTime=Ftbl->Size;
		}
		if(Ftbl->Period>maxperiod){
			maxperiod=Ftbl->Period;
		}
		if(minperiod>Ftbl->Period){
			minperiod=Ftbl->Period;
			EndFrame=Ftbl;
		}
	}

	while(reassign){
		reassign=false;
		while(maxperiod!=EndFrame->Period){

			//找尋要算到Frame (先找大於minperiod, 相差最小的)
			sub=100000000;
			for(FrameTable* Ftbl=FrameTbl; Ftbl!=NULL;Ftbl=Ftbl->next_tbl){
				if(Ftbl->Period>minperiod){
					if((Ftbl->Period-minperiod)<sub){
						sub=(Ftbl->Period-minperiod);
						EndFrame=Ftbl;
					}
				}
			}
			minperiod=EndFrame->Period;

			//計算U bound
			for (FrameTable* Ftbl=FrameTbl; Ftbl!=NULL; Ftbl=Ftbl->next_tbl){
				if(Ftbl->Period<=minperiod){
					if(BlockingTime!=Ftbl->Size)
						U_bound=U_bound+(Ftbl->Size/Ftbl->Period);
				}
			}
			U_bound=U_bound+BlockingTime/minperiod;
		
			//看是否可schedulable
			if(U_bound>=1){
				reassign=true;
			}
		}
		/*=================================
				重新計算Event interval
		=================================*/
		if(reassign){
			//找最大的Size做修改
			for(FrameTable* Ftbl=FrameTbl; Ftbl!=NULL; Ftbl=Ftbl->next_tbl){
				if(Ftbl->Size==BlockingTime){
					BlockingTime--;
					Ftbl->Size--;
					Ftbl->ConnNode->eventinterval--;
				}
			}
		}
	}
	#ifdef _Showlog
		cout<<"Reassign Done"<<endl;
	#endif
}

/*=======================================
			Check every pkt
			Have to Meet Deadline
=======================================*/
void CheckPkt(){
	/*
	if(Timeslot==200){
		system("PAUSE");
	}
	*/

	for(Packet *pkt=Head->nextnd->pkt; pkt!=NULL; pkt=pkt->nextpkt){
		if( pkt->CMP_D<=Timeslot && pkt->deadline<=Timeslot){
			if(pkt->CMP_D<pkt->deadline){
				pkt->CMP_D=pkt->deadline;
			}
			pkt->CMP_D=pkt->CMP_D+pkt->period;

			pkt->Miss_count++;	

			Schdulefile<<Timeslot<<" ";
			Schdulefile<<"Node"<<pkt->nodeid<<" (PKT"<<pkt->id<<" Miss deadline"<<" Deadline "<<pkt->deadline<<" CMP_D:"<<pkt->CMP_D<<")";
			Schdulefile<<endl;

			Meetflag=false;
		}
	}
}

/*============================================
		將node上的Buffer做傳輸
		針對目前的Buffer做傳輸
		進入一次只會傳輸payload大小 (20bytes)
============================================*/
void BLE_EDF(Node *node){
		CheckPkt(); //因很有可能在此進入connection event並結束造成miss算到 (Timeslot有加3)

		/*---------------------------------------------
				判斷Flow 是否為NULL
				若有封包則進行傳輸
				(包含判斷是否結束)
		---------------------------------------------*/
		PacketBuffer *Buffer=node->NodeBuffer;

		//Debug not normal situation
		if(Buffer->load<0){
			printf("Buffer->load<0\n");
			system("PAUSE");
		}

		//Transmission
		if(Buffer->load!=0){
			totalevent++;

			Node *tmpnode=Buffer->pkt->node;

			//cout<<"Time slot:"<<Timeslot<<" Node:"<<Buffer->pkt->node->id<<",";
			Schdulefile<<"Time slot:"<<Timeslot;
			
			//=============================================執行傳輸(分為小於payload與大於payload)
			packet=Buffer->pkt;
			Pktsize=Buffer->pktsize;
			if(packet->exeload<payload){
				Buffer->load=Buffer->load-packet->exeload;
				packet->exeload=0;

				packet->node->State="Transmission";
				packet->State="Transmission";		//傳輸狀態
			}else{
				Buffer->load=Buffer->load-payload;
				packet->exeload=packet->exeload-payload;

				packet->node->State="Transmission";
				packet->State="Transmission";		//傳輸狀態
			}

			Schdulefile<<" NP:"<<packet->node->id<<","<<packet->id;

			//=============================================傳完,換下一packet
			if(packet->exeload==0){

				packet->meetlatency_cnt++;
				packet->meetlatency=packet->meetlatency+(Timeslot - packet->arrival);

				//判斷是否miss deadline
				if((Timeslot)>=packet->deadline){
#ifdef _ShowLog
					cout<<"(PKT"<<packet->id<<" Miss deadline"<<" Deadline "<<packet->deadline<<")";
#endif
					Schdulefile<<"(PKT"<<packet->id<<" Miss deadline"<<" Deadline "<<packet->deadline<<")";
					

					//packet->latency=packet->latency+(Timeslot-packet->deadline);//Record latency
					packet->latency=packet->latency+(Timeslot-packet->arrival);//Record latency
					
					Meetflag=false;
					//system("PAUSE");
				}

				packet->readyflag=0;
				packet->exeload=packet->load;
				packet->arrival=packet->deadline;
				packet->deadline=packet->deadline+packet->period;
				//packet->CMP_D=packet->CMP_D+packet->period;

				packet->exehop=packet->hop;	

				//Buffer往前移動
				packet=packet->buffernextpkt;
				Buffer->pkt=packet;
			}

			//=============================================確認Buffer沒有packet,將arrival_flag設為false
			if(Buffer->load==0){
				tmpnode->arrival_flag=0;
			}
			
			Schdulefile<<endl;
		}else{
			Buffer=NULL;
		}
}

/*========================================
		對SettingNode做好NodeBuffer設定
		Refresh SettingNode->NodeBuffer
========================================*/
void NodeBufferSet(Node * SettingNode){
	
	Node *Bufnode=SettingNode;

	//-------------------------------------------------------------對此node的NodeBuffer做分配
	if(Bufnode==SettingNode){
		Bufnode->NodeBuffer->pktsize=0;	//先把buffer內的封包量清空
		Bufnode->NodeBuffer->load=0;	//load清空
		Bufnode->NodeBuffer->pkt=NULL;	//pkt清空
		
		Packet *tmpbufferpkt;
		int tmpsize=0;					//Buffer 目前pkt size
		int intervalsize=0;				//可塞的packet量

		packet=Bufnode->pktQueue;
		if(Bufnode->NodeBuffer->pktsize!=0){
			while(packet->readyflag!=1){			
				packet=packet->nodereadynextpkt;
			}		
		}

		/*-------------------------------------------
				Assign node->NodeBuffer資訊
					pktsize 最多到Maxbuffersize
					pkt->buffernextpkt
					load 最多到Maxbuffersize*payload
		-------------------------------------------*/
		while(Bufnode->NodeBuffer->pktsize<Maxbuffersize && packet!=NULL){

			if(packet->readyflag!=1){			//尚未ready，直接換下一packet
				packet=packet->nodereadynextpkt;
			}else{
			
				//確認packet不存在於Buffer中
				bool existflag=false;
				for(Packet* pkt=Bufnode->NodeBuffer->pkt; pkt!=NULL; pkt=pkt->buffernextpkt){
					if(pkt==packet){
						existflag=true;
					}
				}

				//放入Buffer
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
							if(tmpload>payload){
								Bufnode->NodeBuffer->load=Bufnode->NodeBuffer->load+payload;
							}else{
								Bufnode->NodeBuffer->load=Bufnode->NodeBuffer->load+tmpload;
							}
							tmpload=tmpload-payload;
							intervalsize--;
						}

					}else{
						Bufnode->NodeBuffer->load=Bufnode->NodeBuffer->load+packet->exeload;
					}
				}

				//換下一個packet (按照node 上 pkt priority)
				packet=packet->nodereadynextpkt;
			}
		}

	}
}

/***********************************************
	先做EIMA上的node 傳輸通知(依照EDF方式)
	對node做傳輸

	node->State為被通知的node
	node->EvtArrival為event 抵達
***********************************************/
void NPEDF(){
	
	/*----------------------------------------------
		Head對ConnNode下指令，在計算後的FrameSize內
		FrameSize若計算完畢立即換下一TDMA的frame做事
		對此frame上的node做state上的變化
	----------------------------------------------*/
	
	if(Head->FrameSize<=0){ //(Head->RecvNode目前是先擋住，但之後要對FrameSize做修改)
		
		//找最小FrameTable的deadline <Arrival>
		FrameTable* Work_tbl=NULL;
		for(FrameTable *Ftbl=FrameTbl; Ftbl!=NULL; Ftbl=Ftbl->next_tbl){
			if(Timeslot >= Ftbl->arrival){
				if(Work_tbl==NULL){
					Work_tbl=Ftbl;
				}else{
					if((Work_tbl->Deadline > Ftbl->Deadline) ){
						Work_tbl=Ftbl;
					}
				}
			}
		}
        
        if(Work_tbl!=NULL){
            Work_tbl->arrival=Work_tbl->arrival+Work_tbl->Period;
            Work_tbl->Deadline=Work_tbl->Deadline+Work_tbl->Period;
			Work_tbl->ConnNode->State="Notify";

			Head->FrameSize=Work_tbl->Size;            
            Head->FrameSize--;
        }
	}else{
		Head->FrameSize--;		
	}

	/*----------------------------------------------
				先做Scan duration
	----------------------------------------------*/
	for(Node *n=Head->nextnd; n!=NULL; n=n->nextnd){
		if(n->SendNode!=Head && n->ScanFlag && n->SendNode!=Head->RecvNode){ //傳輸Node不為Head, n現在scan 且傳輸Node現在沒傳輸
			if(n->EXEScanDuration>=0){
				n->EXEScanDuration--;
			}
		}
	}

}

/***********************************************
	先做好Head對於node的state 轉換cmd
	event arrival做NodeBufferSet，需設定當前EvtArrival來做紀錄 等到自己傳輸時間點抵達
	傳輸設定完畢後再將EvtArrival 取消
***********************************************/
void RoundRobin(){
	/*----------------------------------------------
		Head對ConnNode下指令，在計算後的FrameSize內
		FrameSize若計算完畢立即換下一TDMA的frame做事
		對此frame上的node做state上的變化
	----------------------------------------------*/
	
	if(Head->FrameSize<=0 ){ //(Head->RecvNode目前是先擋住，但之後要對FrameSize做修改)
		bool findflag=false;

		//找最小FrameTable的deadline
		FrameTable *Work_tbl=FrameTbl;
		for(FrameTable *Ftbl=FrameTbl; Ftbl!=NULL; Ftbl=Ftbl->next_tbl){
			if(Ftbl->Currentflag==true && findflag==false){
				if(Ftbl->next_tbl!=NULL){
					Work_tbl=Ftbl->next_tbl;
				}else{
					Work_tbl=FrameTbl;
				}
				findflag=true;
			}

			Ftbl->Currentflag=false;
		}

		Work_tbl->Currentflag=true;
		Work_tbl->ConnNode->State="Notify";
		Head->FrameSize=Work_tbl->Size;

		Work_tbl->Deadline=Work_tbl->Deadline+Work_tbl->Period;

		Head->FrameSize--;
	}else{
		Head->FrameSize--;
	}
	
}

/***********************************************
	先做EIMA上的node 傳輸通知(依照EDF方式)
	對node做傳輸

	node->State為被通知的node		(在切換node時 master就會要求)
	node->EvtArrival為event 抵達		(slave connection evt抵達)

	(Frame Deadline assignment is connection interval between tx event to next event.)
	<D=timeslot+Tc, at event arrival>
***********************************************/
void EIF(){
	/*----------------------------------------------
		Head對ConnNode下指令，在計算後的FrameSize內
		FrameSize若計算完畢立即換下一TDMA的frame做事
		對此frame上的node做state上的變化
	----------------------------------------------*/
	
	if(Head->FrameSize<=0){ //(Head->RecvNode目前是先擋住，但之後要對FrameSize做修改)
		
		//找Crtical Frame (Period最小) <No matter Arrival>
		FrameTable* CrticalFrame=FrameTbl;
		for(FrameTable* Ftbl=FrameTbl; Ftbl!=NULL; Ftbl=Ftbl->next_tbl){
			if(CrticalFrame->Deadline > Ftbl->Deadline){
				CrticalFrame=Ftbl;
			}
		}

		//找最小FrameTable的deadline <Arrival>
		FrameTable* Work_tbl=NULL;
		for(FrameTable *Ftbl=FrameTbl; Ftbl!=NULL; Ftbl=Ftbl->next_tbl){
			if(Timeslot >= Ftbl->arrival){
				if(Work_tbl==NULL){
					Work_tbl=Ftbl;
				}else{
					if((Work_tbl->Deadline > Ftbl->Deadline) ){
						Work_tbl=Ftbl;
					}
				}
			}
		}

		//
		if(Work_tbl!=NULL){
			
			if(CrticalFrame!=Work_tbl){
				//if(CrticalFrame->Deadline < (int(Work_tbl->Size)+int(CrticalFrame->Size)+Timeslot)){
				if(floor(CrticalFrame->Deadline/CrticalFrame->Size)*CrticalFrame->Size < (int(Work_tbl->Size)+Timeslot)){
					if(Timeslot >= CrticalFrame->arrival){
						Work_tbl=CrticalFrame;
					}else{
						Work_tbl=NULL;
					}
				}
			}
			
			if(Work_tbl!=NULL){
				Work_tbl->arrival=Work_tbl->arrival+Work_tbl->Period;
				Work_tbl->ConnNode->State="Notify";
				Head->FrameSize=Work_tbl->Size;
				Work_tbl->Deadline=Work_tbl->Deadline+Work_tbl->Period;

				Head->FrameSize--;
			}
		}
		//Work_tbl->Deadline=Timeslot+Work_tbl->Period;
	}else{
		Head->FrameSize--;		
	}
	
}

/*=========================================
			單一node上的schedule
=========================================*/
void SingleNodeSchedule(int ServiceInterval){
	Node *n=Head->nextnd;
	
	//-------------------------------------Callback Timer Trigger
	switch(ServiceInterval){
	case 2: //------------DIF
		DIFCB();
		break;
	case 3:	//------------Lazy
		LazyIntervalCB();
		break;
	}
	
	//---------------------------------------判斷connection event是否arrival
	if(Timeslot % int(n->eventinterval)==0){
		if(ServiceInterval==3){ //------------Lazy
			LazyOnWrite();		//若有OnWrite則判斷
		}

		NodeBufferSet(n);
		n->EvtArrival=true;
		
		Head->RecvNode=n;		//Head->RecvNode切換
		n->State="Transmission";
		Timeslot=Timeslot+2;

	}else{
		n->EvtArrival=false;
	}

	//---------------------------------------傳輸
	if(n->State=="Transmission"){
		BLE_EDF(n);				//對n做傳輸
	}
	Node_EnergyState(n);		//計算n的Energy

	//---------------------------------------傳輸完畢
	if(n->NodeBuffer->load==0){
		Head->RecvNode=NULL;
		n->State="Sleep";
	}
}

/*=========================================
		Lazy Decrease Alorithm
=========================================*/
void LazyOnWrite(){
	double Rate_data,Rate_BLE;
	double load=0,min_deadline=-1;
	double load_minperiod=-1,minperiod=-1;

	for(Packet *pkt=Head->nextnd->pkt; pkt!=NULL; pkt=pkt->nextpkt){
		if(pkt->readyflag){
			load=load+pkt->exeload;
			if(min_deadline==-1 || pkt->deadline<min_deadline){
				min_deadline=pkt->deadline;
			}
		}

		if(minperiod==-1 || pkt->period<minperiod){
			minperiod=pkt->period;
			load_minperiod=pkt->load;
		}
	}
	Rate_data=load/(min_deadline-Timeslot);							//計算此時的data rate
	Rate_BLE=(payload*Maxbuffersize)/(Head->nextnd->eventinterval);	//計算此時的BLE rate

	/*
	if(Rate_data>=Rate_BLE){
		Head->nextnd->eventinterval=10;
		
		Callbackclock=EXECBclock;		//Reset timer
	}
	*/
	
	if(Rate_data>=Rate_BLE){
		Head->nextnd->eventinterval=10;
		
		Callbackclock=EXECBclock;		//Reset timer
	}

}
/*=========================================
		Timer Callback
		重新assign connection interval
=========================================*/
void LazyIntervalCB(){
	if(Callbackclock==0){
		double MaxRate_data,MinRate_BLE,Rate_BLE;
		double Rate_CB,Rate_reduce;
		double load=0,min_deadline=-1,long_period=-1, min_period=-1, load_minperiod;

		for(Packet *pkt=Head->nextnd->pkt; pkt!=NULL; pkt=pkt->nextpkt){
		
			load=load+pkt->load;
			if(min_deadline==-1 || pkt->deadline<min_deadline){
					min_deadline=pkt->deadline;
			}
			if(long_period==-1 || pkt->period>long_period){
				long_period=pkt->period;
			}
			if(min_period==-1 || pkt->period<min_period){
				min_period=pkt->period;
				load_minperiod=pkt->load;
			}
		}

		//MaxRate_data=load/(min_deadline-Timeslot);						//計算此時的data rate
		MaxRate_data=(payload*Maxbuffersize)/(min_period);						//計算此時的data rate
		Rate_CB=(payload*Maxbuffersize)/EXECBclock;						//計算此時的BLE rate
		MinRate_BLE=(payload*Maxbuffersize)/(long_period);				//計算此時的BLE rate
		Rate_BLE=(payload*Maxbuffersize)/(Head->nextnd->eventinterval);	//計算此時的BLE rate

		Rate_reduce=dec_cof*(MaxRate_data-MinRate_BLE);

		if(Rate_CB<(Rate_BLE-Rate_reduce) && Rate_reduce>0){
			Head->nextnd->eventinterval=(payload*Maxbuffersize)/(Rate_BLE-Rate_reduce);
		}

		if(Timeslot==0){
			Head->nextnd->eventinterval=10;
		}else if(Head->nextnd->eventinterval<10){
			Head->nextnd->eventinterval=10;
		}

		//Reset timer
		if(EXECBclock<(Head->nextnd->eventinterval*overheadcount)){
			Callbackclock=EXECBclock;
		}else{
			Callbackclock=Head->nextnd->eventinterval*overheadcount;
		}
	}else{
		Callbackclock--;
	}
}

void DIFCB(){
	if(Callbackclock==0){
		double MaxRate=0;
		double MinPeriod=-1;
		for(Packet *pkt=Head->nextnd->pkt; pkt!=NULL; pkt=pkt->nextpkt){
			//找ready好的最大rate
			
			if(pkt->readyflag){
				if(pkt->rate>MaxRate){
					MaxRate=pkt->rate;
				}
			}

			//找Minimum period
			if(pkt->period<MinPeriod || MinPeriod==-1){
				MinPeriod=pkt->period;
			}
		}

		Head->nextnd->eventinterval=(payload*Maxbuffersize)/MaxRate;
		if(Head->nextnd->eventinterval<10){
			Head->nextnd->eventinterval=10;
		}
		
		//Reset timer
		Callbackclock=Head->nextnd->eventinterval*overheadcount;	//至少需要六次後才能做更改
	}else{
		Callbackclock--;
	}
}

void Finalcheck(){
	for(Packet* pkt=Head->nextnd->pkt; pkt!=NULL; pkt=pkt->nextpkt){
		if(pkt->deadline<=Timeslot){
			pkt->latency=pkt->latency+(Timeslot-pkt->arrival);
		}
	}
}

/*================================================
			
================================================*/
void Polling(){
	/*----------------------------------------------
	----------------------------------------------*/
	if(Head->FrameSize<=0 ){ //(Head->RecvNode目前是先擋住，但之後要對FrameSize做修改)
		if(Cycle!=NULL){
			Cycle=Cycle->polling_next;
		}
		if(Cycle==NULL){
			short int count=pollingcount;
			FrameTable *tmptbl=NULL;
			for(FrameTable* tbl=FrameTbl; tbl!=NULL; tbl=tbl->next_tbl){
				tbl->Currentflag=false;
			}

			while(count){
				//找最小period且未被assign入Cycle
				tmptbl=NULL;
				for(FrameTable* tbl=FrameTbl; tbl!=NULL; tbl=tbl->next_tbl){
					if(tbl->Currentflag==false){	//尚未assign
						if(tmptbl==NULL){
							tmptbl=tbl;
						}else if(tbl->Period < tmptbl->Period){
							tmptbl=tbl;
						}
					}
				}

				//放入cycle
				tmptbl->Currentflag=true;
				if(Cycle==NULL){
					Cycle=tmptbl;
					Cycle->polling_next=NULL;
				}else{
					FrameTable *subcycle=Cycle;
					while(subcycle->polling_next!=NULL){
						subcycle=subcycle->polling_next;
					}
					subcycle->polling_next=tmptbl;
					subcycle->polling_next->polling_next=NULL;
				}

				count--;
			}

			//下一次polling cycle
			pollingcount++;
			if(pollingcount>nodelevel1){
				pollingcount=1;
			}
		}

		FrameTable* Work_tbl=Cycle;

		Work_tbl->Currentflag=true;
		Work_tbl->ConnNode->State="Notify";

		Head->FrameSize=Work_tbl->Size;
		Work_tbl->Deadline=Work_tbl->Deadline+Work_tbl->Period;

		Head->FrameSize--;
	}else{
		Head->FrameSize--;
	}

}


void Write_Request(){
	
	bool NotifyFlag=true;		//確認傳輸只能一次(ConnSet)
	bool IncreaseSlot=false;	//確認是否需要加event時間

	/*----------------------------------------------
		ConnSet中被通知的Node做對應傳輸或SCAN
		Notify & Scan可同時運作

		node是否有event arrival		(node->EvtArrival)
		node是否有被通知				(Head->RecvNode, call BLE_EDF(node))
		node energy consumption		(node->State)
	----------------------------------------------*/
	for(Node *n=Head->nextnd; n!=NULL; n=n->nextnd){
		
		//-------------------State為Transmission, 只在event arrival 時才做Buffer規劃
		
		if(Timeslot % int(n->eventinterval)==0){
			NodeBufferSet(n);		//整理好n中的NodeBuffer
			n->EvtArrival =true;	//對此node設定EvtArrival

		}else{
			n->EvtArrival =false;
		}
		
		//-------------------進行傳輸 [有被通知(node->State=="Transmission") 且 event arrival(node->EvtArrival)]
		if(n->State!="Sleep"){
			bool eventarrival=false;
			
			if(n->EvtArrival && n->State=="Notify" && (Head->RecvNode==NULL || Head->RecvNode==n)){
				Head->RecvNode=n;			//Head->RecvNode切換
				n->State="Transmission";	//切換狀態
				
				//因為確認為n，在其傳輸時間須2.675ms，所以先加2ms (回去main會在加1ms)
				IncreaseSlot=true;
			}
			
			//-------------------進行傳輸 (Head->RecvNode要確認目前沒node或為當前node)
			if(Head->RecvNode==n && NotifyFlag){
				BLE_EDF(n);				//對n做傳輸
				/*
				if(n->NodeBuffer->load==0){
					Node_EnergyState(n);	//計算n的Energy
					Head->RecvNode=NULL;
				}
				*/
				NotifyFlag=false;
			}

			//-------------------State為Scan
			if(n->State=="Scan"){

			}
		}

		//---------------------------------------Power consumption & State切換
		Node_EnergyState(n);	//計算n的Energy
		if(n->NodeBuffer->load==0 && n->State=="Transmission" && Head->RecvNode==n){
			n->State="Sleep";
			Head->RecvNode=NULL;
		}
	}

	//
	if(IncreaseSlot){
		Timeslot=Timeslot+2;
		Head->FrameSize=Head->FrameSize-2;
	}
}

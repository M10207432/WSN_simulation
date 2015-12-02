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

void Schedule(int propose, int intervalpropose){
	
	CheckPkt();
	/*==========================
			判斷是單一node
			或是multi node
	==========================*/
	if((nodelevel1+nodelevel2)!=1){
		switch (propose){
		case 0:
			FrameEDFSchedule();
			break;
		case 1:
			TDMASchedule();
			break;
		case 2:
			EIF();
			break;
		case 3:
			Polling();
			break;
		}
	}else{
		SingleNodeSchedule(intervalpropose);
	}

	
}

/*==========================
	主體Schedule
	相同顏色同時傳輸
	與其不碰撞同時傳輸
	FlowSlot-->先由哪一Slot開始傳(TDMATable)
	Flow_flag-->判斷有無碰撞(ConflictEdge)
==========================*/
void MainSchedule(int FlowSlot,bool Flow_flag){

	//-------------------------------找出connection interval抵達的node
	Node *Flownode=Head->nextnd;
	while(Flownode!=NULL){	

		Buffer=Flownode->NodeBuffer;

		if(Timeslot % int(Flownode->eventinterval)==0 && Buffer->pkt!=NULL) //若有Recvnode，也需考量其interval
			Flownode->arrival_flag=1;
		
		/*--------------------------
			若Connection interval到
			但未有pkt可以先記錄已arrival
			set arrival_flag為10
			等待下一次有pkt情況
		--------------------------*/
		if(Flownode->arrival_flag==10 && Buffer->pkt!=NULL) //若有Recvnode，也需考量其interval
			Flownode->arrival_flag=1;
		if(Timeslot % int(Flownode->eventinterval)==0 && Buffer->pkt==NULL) //若有Recvnode，也需考量其interval
			Flownode->arrival_flag=10;

		Flownode=Flownode->nextnd;
	}

	//-------------------------------找出目前因該傳輸的TDMA slot id (FlowSlot)，
	//都會以最早的time slot為主，並未有依序情況(若要此slot傳完換下一slot為主需要加入判斷)
	//即為需加入要比 前一FlowSlot 往後Slot
	
	int Maxslot=0;	//找出TDMA最大Slot id
	TDMATable *FlowTable=TDMA_Tbl;
	while(FlowTable!=NULL){

		if(FlowTable->slot > Maxslot){
			Maxslot=FlowTable->slot;
		}

		FlowTable=FlowTable->next_tbl;
	}
	
	FlowSlot=TDMASlot++;
	if(FlowSlot>Maxslot){
		FlowSlot=1;
		TDMASlot=2;
	}
	//-------------------------------TDMA Table下找FlowSlot 的node
	FlowTable=TDMA_Tbl;
	while(FlowTable!=NULL){
					
		Flow_flag=true;
		if(FlowTable->n1->arrival_flag==1 && FlowTable->slot==FlowSlot){//找已經arrival的node 且 在此FlowSlot上
						
			//此FlowTable上的n1並未有剛剛傳輸完畢的node碰撞 (ConflictEdge->n2的arrival_flag可為1 但不可為-1)
			//(理論上來說在TDMA schedule建立Table時就有防止這一項)
			Edge *tmp_ConflictEdge=ConflictEdge;
			while(tmp_ConflictEdge!=NULL){
				if(tmp_ConflictEdge->n1==FlowTable->n1 && tmp_ConflictEdge->n2->arrival_flag==-1){ 
					Flow_flag=false;
					cout<<"At FlowSchedule the slot's node have conflict with each other"<<endl;
					printf("Node%d, Node%d Conflict\n",tmp_ConflictEdge->n1->id,tmp_ConflictEdge->n2->id);

					system("PAUSE");
				}
				tmp_ConflictEdge=tmp_ConflictEdge->next_edge;
			}

			//用Flow_flag判斷FlowTable->n1是否可傳輸
			Flownode=FlowTable->n1;
			if(Flow_flag){
				Buffer=Flownode->NodeBuffer;
							
				FlowEDF();

				Flownode->arrival_flag=-1;
			}

		}
		FlowTable=FlowTable->next_tbl;
	}

	//-----------------------------------將剛做完的flag改為傳輸完畢
	Flownode=Head->nextnd;
	while(Flownode!=NULL){
		if(Flownode->arrival_flag==-1)
			Flownode->arrival_flag=0;
		Flownode=Flownode->nextnd;
	}
}

/*==========================
	Flow EDF Scheduling
	Event Transmission
==========================*/
void FlowEDF(){
	
		/*---------------------------------------------
				判斷Flow 是否為NULL
				若有封包則進行傳輸
				(包含判斷是否結束)
		---------------------------------------------*/
		if(Buffer->pkt!=NULL){
			totalevent++;

			//cout<<"Time slot:"<<Timeslot<<" Node:"<<Buffer->pkt->node->id;
			Schdulefile<<"Time slot:"<<Timeslot;
			//=============================================執行傳輸
			packet=Buffer->pkt;
			Pktsize=Buffer->pktsize;

			while(Buffer->load!=0){
								
				packet->exeload--;
				Buffer->load--;
				packet->node->State="Transmission";
				packet->State="Transmission";		//傳輸狀態

				if(packet->exeload==0){

					//cout<<" Packet:"<<packet->id;
					Schdulefile<<" NP:"<<packet->node->id<<","<<packet->id;
					
					//判斷是否需要hop
					packet->exehop--;
					if(packet->exehop>0)
					{
						//填入SendNode 先進入的packet其priority一定較高
						
						packet->exeload=packet->load;
						Headflow->pkt=packet;
					}else if (packet->exehop==0){
						//判斷是否miss deadline
						if((Timeslot)>=packet->deadline){
							
							cout<<"(PKT"<<packet->id<<" Miss deadline"<<" Deadline "<<packet->deadline<<")";
							Schdulefile<<"(PKT"<<packet->id<<" Miss deadline"<<" Deadline "<<packet->deadline<<")";

							Meetflag=false;
							//system("PAUSE");
						}
						
						packet->readyflag=0;
						packet->exeload=packet->load;
						packet->arrival=packet->deadline;
						packet->deadline=packet->deadline+packet->period;
						packet->CMP_D=packet->CMP_D+packet->period;
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
				packet->node->State="Transmission";		//傳輸狀態
			}
			
			//cout<<endl;
			Schdulefile<<endl;
			/*---------------------------
				傳輸完立即做
				狀態切換 & Energy 計算
			---------------------------*/
			//NodeEnergy();	//計算個感測器Energy

		}else{
			//NodeEnergy();

			//cout<<"Time slot:"<<Timeslot<<" IDLE"<<endl;
			Schdulefile<<"Time slot:"<<Timeslot<<" IDLE"<<endl;
			
		}
}

/*==========================================
			Arrange Queue
{ReadyQ & WaitQ is assign by pkt->readyflag}

	先建立總total的Packet Queue
	在分配給各自的node
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
			packet->node->arrival_flag=1;
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
	Node *tmp_node=Head->nextnd;
	Packet *tmpreadypkt;
	while(tmp_node!=NULL){
		tmpreadypkt=tmp_node->pkt;
		tmp_node->pktQueue=NULL;
		while(tmpreadypkt!=NULL){
			tmpreadypkt->nodereadynextpkt=NULL;
			tmpreadypkt->nodereadyprepkt=NULL;
			tmpreadypkt=tmpreadypkt->nextpkt;
		}
		tmp_node=tmp_node->nextnd;
	}

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

			if(packet->readyflag!=1){			//尚未ready，直接換下一packet
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

	for(Packet *pkt=Head->nextnd->pkt; pkt!=NULL; pkt=pkt->nextpkt){
		if( pkt->CMP_D<=Timeslot && pkt->deadline<=Timeslot){
			pkt->CMP_D=pkt->CMP_D+pkt->period;
			pkt->Miss_count++;	

			Schdulefile<<Timeslot<<" ";
			Schdulefile<<"Node"<<pkt->nodeid<<" (PKT"<<pkt->id<<" Miss deadline"<<" Deadline "<<pkt->deadline<<")";
			Schdulefile<<endl;

			Meetflag=false;
		}
	}
}


void BLESchedule(int FlowSlot, bool Flow_flag){
	
	//-----------------------------------------------------------看NotifyNode是否為NULL,找出TDMA slot下可傳輸的node
	//Change TDMA Table
	if(NotifyTable->count<=1){
		NotifyTable=NotifyTable->next_tbl;
		if(NotifyTable==NULL)
			NotifyTable=TDMA_Tbl;
		NotifyTable->count=ceil(NotifyTable->n1->eventinterval);
	}

	//Buffer=NULL;
	for(Node* node=Head->nextnd; node!=NULL;node=node->nextnd){
		if(NotifyNode==NULL){
			if(NotifyTable->n1==node && (FlowSlot % int(node->eventinterval))==node->EventTime){
				NotifyNode=NotifyTable->n1;

				NodeBufferSet(NotifyNode);		//修改好NotifyNode中的Buffer
				Buffer=NotifyNode->NodeBuffer;
			}
		}
	}

	//-----------------------------------------------------------對NotifyNode做傳輸
	if(Buffer!=NULL){
		BLE_EDF(NotifyNode);
	}
	
	NotifyTable->count--;
}

/*============================================
		將node上的Buffer做傳輸
		針對目前的Buffer做傳輸
		進入一次只會傳輸payload大小 (20bytes)
============================================*/
void BLE_EDF(Node *node){
	
		/*---------------------------------------------
				判斷Flow 是否為NULL
				若有封包則進行傳輸
				(包含判斷是否結束)
		---------------------------------------------*/
		PacketBuffer *Buffer=node->NodeBuffer;
		if(Buffer->load<0){
			printf("Buffer->load<0\n");
			system("PAUSE");
		}
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
			//cout<<"P:"<<packet->id<<endl;

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
					
					packet->latency=packet->latency+(Timeslot-packet->deadline);//Record latency

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
				tmpnode->ContinueNotify=false;
				NotifyNode=NULL;

				int Maxslot=0;	//找出TDMA最大Slot id
				TDMATable *FlowTable=TDMA_Tbl;
				while(FlowTable!=NULL){

					if(FlowTable->slot > Maxslot){
						Maxslot=FlowTable->slot;
					}

					FlowTable=FlowTable->next_tbl;
				}
	
				TDMASlot++;
				if(TDMASlot>Maxslot){
					TDMASlot=1;
				}
			}
			
			Schdulefile<<endl;
		}else{
			//BLE_NotifyNode->arrival_flag=0;
			NotifyNode=NULL;
			Buffer=NULL;
		}
}
/*========================================
		對SettingNode做好NodeBuffer設定
		Refresh SettingNode->NodeBuffer
========================================*/
void NodeBufferSet(Node * SettingNode){
	Node *Bufnode=Head->nextnd;

	//-------------------------------------------------------------先找到特定的node
	while(Bufnode!=SettingNode)
		Bufnode=Bufnode->nextnd;

	//-------------------------------------------------------------對此node的NodeBuffer做分配
	if(Bufnode==SettingNode){
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

			if(packet->readyflag!=1){			//尚未ready，直接換下一packet
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

	}
}

/***********************************************
	先做EIMA上的node 傳輸通知(依照EDF方式)
	對node做傳輸

	node->State為被通知的node
	node->EvtArrival為event 抵達
***********************************************/
FrameTable *Work_tbl=NULL;
void FrameEDFSchedule(){
	
	TDMATable *table=TDMA_Tbl;
	Node *node=Head->nextnd;
	short int notiyfyslot=0;
	short int MaxFrameSize=0;
	bool allframeset=false;		//確認有對其中一個frame下做currslot enable
	bool NotifyFlag=true;		//確認傳輸只能一次(ConnSet)
	
	/*----------------------------------------------
		Head對ConnNode下指令，在計算後的FrameSize內
		FrameSize若計算完畢立即換下一TDMA的frame做事
		對此frame上的node做state上的變化
	----------------------------------------------*/
	
	if(Head->FrameSize<=0){ //(Head->RecvNode目前是先擋住，但之後要對FrameSize做修改)
		
		//找最小FrameTable的deadline
		FrameTable* PreFrame=Work_tbl;

		Work_tbl=FrameTbl;
		for(FrameTable *Ftbl=FrameTbl; Ftbl!=NULL; Ftbl=Ftbl->next_tbl){
			if((Work_tbl->Deadline > Ftbl->Deadline) ){
				Work_tbl=Ftbl;
			}
		}

		Work_tbl->ConnNode->State="Notify";
		Head->FrameSize=Work_tbl->Size;

		Work_tbl->Deadline=Work_tbl->Deadline+Work_tbl->Period;
		//Work_tbl->Deadline=Timeslot+Work_tbl->Period;
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
				Head->RecvNode=n;	//Head->RecvNode切換


				/******************************************
				******************************************/

				//先做Scan pkt的傳輸
				for(Node *AdvNode=Head->nextnd; AdvNode!=NULL; AdvNode=AdvNode->nextnd){
					if(AdvNode->SendNode==n && AdvNode->EXEScanDuration<0 && AdvNode->ScanFlag){
						
						n->NodeBuffer->load=n->NodeBuffer->load-payload;
						AdvNode->pkt->deadline+=AdvNode->pkt->period;

						//判斷是否miss deadline
						if((Timeslot)>=AdvNode->pkt->deadline){
#ifdef _ShowLog
								cout<<"(PKT"<<AdvNode->pkt->id<<" Miss deadline"<<" Deadline "<<AdvNode->pkt->deadline<<")";
#endif
								Schdulefile<<"(PKT"<<AdvNode->pkt->id<<" Miss deadline"<<" Deadline "<<AdvNode->pkt->deadline<<")";

								Meetflag=false;
							//system("PAUSE");
						}

						AdvNode->ScanFlag=false;
						AdvNode->EXEScanDuration=AdvNode->ScanDuration;
						AdvNode->pkt->arrival=AdvNode->pkt->deadline;
						AdvNode->pkt->deadline=AdvNode->pkt->deadline+AdvNode->pkt->period;
				
					}
				}
				/******************************************
				******************************************/

				n->State="Transmission";
			}
			
			//-------------------進行傳輸 (Head->RecvNode要確認目前沒node或為當前node)
			if(Head->RecvNode==n && NotifyFlag){
				BLE_EDF(n);				//對n做傳輸

				if(n->NodeBuffer->load==0){
					Head->RecvNode=NULL;
				}
				
				NotifyFlag=false;
			}

			//-------------------State為Scan
			if(n->State=="Scan"){
				//n->ScanDuration--;
			}
		}

		//---------------------------------------Power consumption & State切換
		Node_EnergyState(n);	//計算n的Energy
		if(n->NodeBuffer->load==0 && n->State=="Transmission"){
			n->State="Sleep";
		}
	}

}

/***********************************************
	先做好Head對於node的state 轉換cmd
	event arrival做NodeBufferSet，需設定當前EvtArrival來做紀錄 等到自己傳輸時間點抵達
	傳輸設定完畢後再將EvtArrival 取消
***********************************************/
void TDMASchedule(){
	TDMATable *table=TDMA_Tbl;
	Node *node=Head->nextnd;
	short int notiyfyslot=0;
	short int MaxFrameSize=0;
	bool allframeset=false;		//確認有對其中一個frame下做currslot enable
	bool NotifyFlag=true;		//確認傳輸只能一次(ConnSet)
	
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
				Head->RecvNode=n;	//Head->RecvNode切換
				n->State="Transmission";
			}
			
			//-------------------進行傳輸 (Head->RecvNode要確認目前沒node或為當前node)
			if(Head->RecvNode==n && NotifyFlag){
				BLE_EDF(n);				//對n做傳輸

				if(n->NodeBuffer->load==0){
					Head->RecvNode=NULL;
				}
				
				NotifyFlag=false;
			}

			//-------------------State為Scan
			if(n->State=="Scan"){
				//n->ScanDuration--;
			}
		}

		//---------------------------------------Power consumption & State切換
		Node_EnergyState(n);	//計算n的Energy
		if(n->NodeBuffer->load==0 && n->State=="Transmission"){
			n->State="Sleep";
		}
	}
}

/***********************************************
	先做EIMA上的node 傳輸通知(依照EDF方式)
	對node做傳輸

	node->State為被通知的node
	node->EvtArrival為event 抵達

	(Frame Deadline assignment is connection interval between tx event to next event.)
	<D=timeslot+Tc, at event arrival>
***********************************************/
void EIF(){
	
	TDMATable *table=TDMA_Tbl;
	Node *node=Head->nextnd;
	short int notiyfyslot=0;
	short int MaxFrameSize=0;
	bool allframeset=false;		//確認有對其中一個frame下做currslot enable
	bool NotifyFlag=true;		//確認傳輸只能一次(ConnSet)
	
	/*----------------------------------------------
		Head對ConnNode下指令，在計算後的FrameSize內
		FrameSize若計算完畢立即換下一TDMA的frame做事
		對此frame上的node做state上的變化
	----------------------------------------------*/
	
	if(Head->FrameSize<=0){ //(Head->RecvNode目前是先擋住，但之後要對FrameSize做修改)
		
		//找Crtical Frame (Period最小)
		FrameTable* CrticalFrame=FrameTbl;
		for(FrameTable* Ftbl=FrameTbl; Ftbl!=NULL; Ftbl=Ftbl->next_tbl){
			if(CrticalFrame->Deadline > Ftbl->Deadline){
				CrticalFrame=Ftbl;
			}
		}

		//找最小FrameTable的deadline
		Work_tbl=NULL;
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
				if(CrticalFrame->Deadline < (int(Work_tbl->Size)+int(CrticalFrame->Size)+Timeslot)){
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
	
	if(Timeslot==1452){
		int y=0;
	}
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
				Head->RecvNode=n;	//Head->RecvNode切換
				n->State="Transmission";
								
			}
			
			//-------------------進行傳輸 (Head->RecvNode要確認目前沒node或為當前node)
			if(Head->RecvNode==n && NotifyFlag){
				BLE_EDF(n);				//對n做傳輸

				if(n->NodeBuffer->load==0){
					Head->RecvNode=NULL;
				}
				
				NotifyFlag=false;
			}

			//-------------------State為Scan
			if(n->State=="Scan"){
				//n->ScanDuration--;
			}
		}

		//---------------------------------------Power consumption & State切換
		Node_EnergyState(n);	//計算n的Energy
		if(n->NodeBuffer->load==0 && n->State=="Transmission"){
			n->State="Sleep";
		}
	}
}

/*=========================================
			單一node上的schedule
=========================================*/
void SingleNodeSchedule(int intervalpropose){
	Node *n=Head->nextnd;
	
	//-------------------------------------Callback Timer Trigger
	switch(intervalpropose){
	case 2: //------------DIF
		DIFCB();
		break;
	case 3:	//------------Lazy
		LazyIntervalCB();
		break;
	case 4:
		SingleStatic();
		break;
	}
	

	//---------------------------------------判斷connection event是否arrival
	if(Timeslot % int(n->eventinterval)==0){
		if(intervalpropose==3){ //------------Lazy
			LazyOnWrite();		//若有OnWrite則判斷
		}

		NodeBufferSet(n);
		n->EvtArrival=true;
		
		Head->RecvNode=n;		//Head->RecvNode切換
		n->State="Transmission";
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

	for(Packet *pkt=Head->nextnd->pkt; pkt!=NULL; pkt=pkt->nextpkt){
		if(pkt->readyflag){
			load=load+pkt->exeload;
			if(min_deadline==-1 || pkt->deadline<min_deadline){
				min_deadline=pkt->deadline;
			}
		}
	}
	Rate_data=load/(min_deadline-Timeslot);							//計算此時的data rate
	Rate_BLE=(payload*Maxbuffersize)/(Head->nextnd->eventinterval);	//計算此時的BLE rate

	if(Rate_data>=Rate_BLE){
		Head->nextnd->eventinterval=1;
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
		double load=0,min_deadline=-1,long_period=-1;

		for(Packet *pkt=Head->nextnd->pkt; pkt!=NULL; pkt=pkt->nextpkt){
		
			load=load+pkt->load;
			if(min_deadline==-1 || pkt->deadline<min_deadline){
					min_deadline=pkt->deadline;
			}
			if(long_period==-1 || pkt->period>long_period){
				long_period=pkt->period;
			}
		}

		MaxRate_data=load/(min_deadline-Timeslot);							//計算此時的data rate
		Rate_CB=(payload*Maxbuffersize)/EXECBclock;					//計算此時的BLE rate
		MinRate_BLE=(payload*Maxbuffersize)/(long_period);	//計算此時的BLE rate
		Rate_BLE=(payload*Maxbuffersize)/(Head->nextnd->eventinterval);	//計算此時的BLE rate

		Rate_reduce=MaxRate_data-MinRate_BLE;

		if(Rate_CB<(Rate_BLE-Rate_reduce) && Rate_reduce>0){
			Head->nextnd->eventinterval=(payload*Maxbuffersize)/(Rate_BLE-Rate_reduce);
		}

		if(Timeslot==0){
			Head->nextnd->eventinterval=1;
		}else if(Head->nextnd->eventinterval<1){
			Head->nextnd->eventinterval=1;
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
		if(Head->nextnd->eventinterval<1){
			Head->nextnd->eventinterval=1;
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
			pkt->latency=pkt->latency+(Timeslot-pkt->deadline);
		}
	}
}

/*================================================
			
================================================*/
void Polling(){
	TDMATable *table=TDMA_Tbl;
	Node *node=Head->nextnd;
	short int notiyfyslot=0;
	short int MaxFrameSize=0;
	bool allframeset=false;		//確認有對其中一個frame下做currslot enable
	bool NotifyFlag=true;		//確認傳輸只能一次(ConnSet)

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

		Work_tbl=Cycle;
		Work_tbl->Currentflag=true;
		Work_tbl->ConnNode->State="Notify";
		Head->FrameSize=Work_tbl->Size;
		Work_tbl->Deadline=Work_tbl->Deadline+Work_tbl->Period;

		Head->FrameSize--;
	}else{
		Head->FrameSize--;
	}

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
				Head->RecvNode=n;	//Head->RecvNode切換
				n->State="Transmission";
								
			}
			
			//-------------------進行傳輸 (Head->RecvNode要確認目前沒node或為當前node)
			if(Head->RecvNode==n && NotifyFlag){
				BLE_EDF(n);				//對n做傳輸

				if(n->NodeBuffer->load==0){
					Head->RecvNode=NULL;
				}
				
				NotifyFlag=false;
			}

			//-------------------State為Scan
			if(n->State=="Scan"){
				//n->ScanDuration--;
			}
		}

		//---------------------------------------Power consumption & State切換
		Node_EnergyState(n);	//計算n的Energy
		if(n->NodeBuffer->load==0 && n->State=="Transmission"){
			n->State="Sleep";
		}
	}
}

void SingleStatic(){
	//find the minimum period
	Packet* Minpkt=NULL;
	for(Packet* pkt=Head->nextnd->pkt; pkt!=NULL; pkt=pkt->nextpkt){
		if(Minpkt==NULL){
			Minpkt=pkt;
		}else{
			if(Minpkt->period > pkt->period){
				Minpkt=pkt;
			}
		}
	}

	Head->nextnd->eventinterval=Minpkt->period;
}

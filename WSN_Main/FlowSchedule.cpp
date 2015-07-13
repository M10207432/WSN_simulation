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

using namespace std;

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
		/*
		if(FlowTable->n1->arrival_flag==1){
			FlowSlot=FlowTable->slot;
			break;
		}
		*/
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
			NodeEnergy();	//計算個感測器Energy

		}else{
			NodeEnergy();

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
	int n=0;
	int Tc=0;
	int differentvalue=0;
	int Slotid=0;
	int TDMASize=0;
	int preTXslot=0;
	int endTXslot=0;

	TDMATable *Stable=TDMA_Tbl;
	
	node=Head->nextnd;
	while(Stable!=NULL){
		if(TDMASize<Stable->slot){
			TDMASize=Stable->slot;
		}
		Stable=Stable->next_tbl;
	}
	Stable=TDMA_Tbl;


	/*---------------------------
			計算每一node
		比較其Connection interval
		與TDMA造成的延遲發送間格
	---------------------------*/
	while(node!=NULL){
		//設定原本的Connection interval
		Tc=node->eventinterval;
	
		//找出其TDMA上的 發送的Slotid
		while(Stable->n1!=node){
			Stable=Stable->next_tbl;
		}
		Slotid = Stable->slot-1;
		Stable=TDMA_Tbl;

		//
		preTXslot=Slotid;
		n=1;
		differentvalue=0;
		while(n*Tc <= Hyperperiod){
			endTXslot=Slotid+TDMASize*floor(n*Tc/TDMASize);
			while(endTXslot < n*Tc)
				endTXslot=endTXslot+TDMASize;

			differentvalue=differentvalue+(Tc-(endTXslot-preTXslot));
		
			preTXslot=endTXslot;
			n++;
		}
		/*
		if(differentvalue<0){
			printf("Node%d Predict Miss deadline\n",node->id);
		}
		*/
		node=node->nextnd;
	}

}

void BLESchedule(int FlowSlot, bool Flow_flag){

	bool Notifydone=false;
	TDMATable *FlowTable=TDMA_Tbl;
	Node *Flownode=Head->nextnd;
	Node *tmpnode=Head->nextnd;

	//------------------------------------------------------------------------------發送ACK或SCAN CMD
	Flownode=Head->nextnd;
	while(Flownode!=NULL){
		if(FlowSlot % int(Flownode->eventinterval)==0){
			if(Flownode->arrival_flag==1){
				NodeBufferSet(Flownode);		//對此node做Buffer setting
				Flownode->ContinueNotify=true;	//可傳輸

			}
		}	
		
		Flownode=Flownode->nextnd;
	}

	//--------
	Buffer=NULL;
	if(NotifyNode!=NULL){
		Buffer=NotifyNode->NodeBuffer;
	}else{
		Flownode=Head->nextnd;
		while(Flownode!=NULL){
			if(Flownode->ContinueNotify){
				NotifyNode=Flownode;	//設定為正在傳輸的node，找到就馬上進行傳輸
				Buffer=Flownode->NodeBuffer;			
				break;
			}
			Flownode=Flownode->nextnd;
		}
	}
	if(Buffer!=NULL)
		BLE_EDF(NotifyNode);
	
}

void BLE_EDF(Node *BLE_NotifyNode){
	
		/*---------------------------------------------
				判斷Flow 是否為NULL
				若有封包則進行傳輸
				(包含判斷是否結束)
		---------------------------------------------*/
		if(Buffer->pkt!=NULL){
			totalevent++;
			Node *tmpnode=Buffer->pkt->node;

			cout<<"Time slot:"<<Timeslot<<" Node:"<<Buffer->pkt->node->id<<",";
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
			NodeEnergy();	//計算個感測器Energy
			Schdulefile<<" NP:"<<packet->node->id<<","<<packet->id;
			cout<<"P:"<<packet->id<<endl;

			//=============================================傳完,換下一packet
			if(packet->exeload==0){

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
				packet->State="Idle";		//傳輸狀態
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

		}
}

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
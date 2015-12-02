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
			�P�_�O��@node
			�άOmulti node
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
	�D��Schedule
	�ۦP�C��P�ɶǿ�
	�P�䤣�I���P�ɶǿ�
	FlowSlot-->���ѭ��@Slot�}�l��(TDMATable)
	Flow_flag-->�P�_���L�I��(ConflictEdge)
==========================*/
void MainSchedule(int FlowSlot,bool Flow_flag){

	//-------------------------------��Xconnection interval��F��node
	Node *Flownode=Head->nextnd;
	while(Flownode!=NULL){	

		Buffer=Flownode->NodeBuffer;

		if(Timeslot % int(Flownode->eventinterval)==0 && Buffer->pkt!=NULL) //�Y��Recvnode�A�]�ݦҶq��interval
			Flownode->arrival_flag=1;
		
		/*--------------------------
			�YConnection interval��
			������pkt�i�H���O���warrival
			set arrival_flag��10
			���ݤU�@����pkt���p
		--------------------------*/
		if(Flownode->arrival_flag==10 && Buffer->pkt!=NULL) //�Y��Recvnode�A�]�ݦҶq��interval
			Flownode->arrival_flag=1;
		if(Timeslot % int(Flownode->eventinterval)==0 && Buffer->pkt==NULL) //�Y��Recvnode�A�]�ݦҶq��interval
			Flownode->arrival_flag=10;

		Flownode=Flownode->nextnd;
	}

	//-------------------------------��X�ثe�]�Ӷǿ骺TDMA slot id (FlowSlot)�A
	//���|�H�̦���time slot���D�A�å����̧Ǳ��p(�Y�n��slot�ǧ����U�@slot���D�ݭn�[�J�P�_)
	//�Y���ݥ[�J�n�� �e�@FlowSlot ����Slot
	
	int Maxslot=0;	//��XTDMA�̤jSlot id
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
	//-------------------------------TDMA Table�U��FlowSlot ��node
	FlowTable=TDMA_Tbl;
	while(FlowTable!=NULL){
					
		Flow_flag=true;
		if(FlowTable->n1->arrival_flag==1 && FlowTable->slot==FlowSlot){//��w�garrival��node �B �b��FlowSlot�W
						
			//��FlowTable�W��n1�å������ǿ駹����node�I�� (ConflictEdge->n2��arrival_flag�i��1 �����i��-1)
			//(�z�פW�ӻ��bTDMA schedule�إ�Table�ɴN������o�@��)
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

			//��Flow_flag�P�_FlowTable->n1�O�_�i�ǿ�
			Flownode=FlowTable->n1;
			if(Flow_flag){
				Buffer=Flownode->NodeBuffer;
							
				FlowEDF();

				Flownode->arrival_flag=-1;
			}

		}
		FlowTable=FlowTable->next_tbl;
	}

	//-----------------------------------�N�谵����flag�אּ�ǿ駹��
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
				�P�_Flow �O�_��NULL
				�Y���ʥ]�h�i��ǿ�
				(�]�t�P�_�O�_����)
		---------------------------------------------*/
		if(Buffer->pkt!=NULL){
			totalevent++;

			//cout<<"Time slot:"<<Timeslot<<" Node:"<<Buffer->pkt->node->id;
			Schdulefile<<"Time slot:"<<Timeslot;
			//=============================================����ǿ�
			packet=Buffer->pkt;
			Pktsize=Buffer->pktsize;

			while(Buffer->load!=0){
								
				packet->exeload--;
				Buffer->load--;
				packet->node->State="Transmission";
				packet->State="Transmission";		//�ǿ骬�A

				if(packet->exeload==0){

					//cout<<" Packet:"<<packet->id;
					Schdulefile<<" NP:"<<packet->node->id<<","<<packet->id;
					
					//�P�_�O�_�ݭnhop
					packet->exehop--;
					if(packet->exehop>0)
					{
						//��JSendNode ���i�J��packet��priority�@�w����
						
						packet->exeload=packet->load;
						Headflow->pkt=packet;
					}else if (packet->exehop==0){
						//�P�_�O�_miss deadline
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
						packet->State="Idle";		//�ǿ骬�A
						packet->exehop=packet->hop;	
					}

					//Buffer���e����
					packet=packet->buffernextpkt;
					Buffer->pkt=packet;	
				}
				if(Buffer->pkt==NULL)
					break;
			}
			Headflow->pkt=packet;//��m�|��@��packet

			if(Headflow->pkt!=NULL){
				//cout<<" Packet:"<<packet->id;
				Schdulefile<<" NP:"<<packet->node->id<<","<<packet->id;
				packet->State="Transmission";		//�ǿ骬�A
				packet->node->State="Transmission";		//�ǿ骬�A
			}
			
			//cout<<endl;
			Schdulefile<<endl;
			/*---------------------------
				�ǿ駹�ߧY��
				���A���� & Energy �p��
			---------------------------*/
			//NodeEnergy();	//�p��ӷP����Energy

		}else{
			//NodeEnergy();

			//cout<<"Time slot:"<<Timeslot<<" IDLE"<<endl;
			Schdulefile<<"Time slot:"<<Timeslot<<" IDLE"<<endl;
			
		}
}

/*==========================================
			Arrange Queue
{ReadyQ & WaitQ is assign by pkt->readyflag}

	���إ��`total��Packet Queue
	�b���t���U�۪�node
==========================================*/
void PacketQueue(){
	
	Packet *camparepkt;
	Packet *tmpReadyQ=new Packet;
	tmpReadyQ->readynextpkt=ReadyQ;

	packet=Head->nextnd->pkt;
	while(packet!=NULL){
		packet->readynextpkt=NULL;	//�U�@ready�]�w��NULL
		packet->readyprepkt=NULL;	//�W�@ready�]�w��NULL
		packet->searchdone=0;				//�|����L
		packet=packet->nextpkt;
	}

	packet=Head->nextnd->pkt;
	ReadyQ_overflag=0;
	while(!ReadyQ_overflag){
	
		/*--------------------------
			��searchdone��0 ��
			�e���packet �����
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
			��X�̤pDeadline
			�����j�M�@�M
		--------------------------*/
	
		if(packet!=NULL && camparepkt!=NULL){
			while(camparepkt!=NULL){

				//��packet�U�@�Ӥ����camparepkt
				if(packet!=NULL)
					camparepkt=packet->nextpkt;
				while( camparepkt!=NULL){
					if(camparepkt->searchdone==1)
						camparepkt=camparepkt->nextpkt;
					else
						break;
				}
				
				//�Y��packet->deadline >camparepkt->deadline ,�Npacket=camparepkt
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
			ReadyQ_overflag=1;//���ѤU�̫�@��packet�A�Y�i�פ�M��
		}
	
		/*--------------------------
		�N�̤pDeadline��JReadyQ��
		�üаO�w�M��L(searchdone=1)
		--------------------------*/
		packet->searchdone=1;

		if(Timeslot>=packet->arrival){
			packet->readyflag=1;//�ʥ] Arrival
			packet->node->arrival_flag=1;
		}else{
			packet->readyflag=0;//�ʥ] �|��Arrival
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
		�إߦU��node�W��
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
		//������ݪ� node �P����
		tmp_node=tmp_nodepkt->node;

		//��node->pktQueue �̫�@��
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

		//���U�@��Global Queue packet
		tmp_nodepkt=tmp_nodepkt->readynextpkt;
	}
}

/*=============================================	
		�إߦnBuffer�W��packet
	pkt link list, load, packet size
=============================================*/
void BufferSet(){
	Node *Bufnode=Head->nextnd;

	while(Bufnode!=NULL){
		Bufnode->NodeBuffer->pktsize=0;//����buffer�����ʥ]�q�M��
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

			if(packet->readyflag!=1){			//�|��ready�A�������U�@packet
				packet=packet->nodereadynextpkt;
			}else{
			
				//�T�{packet���s�b��Buffer��
				bool existflag=false;
				Packet* existpkt=Bufnode->NodeBuffer->pkt;
				while(existpkt!=NULL){
					if(existpkt==packet)
						existflag=true;
					existpkt=existpkt->buffernextpkt;
				}

				if(existflag!=true){
					//-----------------------------------------��JBuffer link list
					if(Bufnode->NodeBuffer->pktsize == 0){
						Bufnode->NodeBuffer->pkt=packet;
						tmpbufferpkt=Bufnode->NodeBuffer->pkt;
						tmpbufferpkt->buffernextpkt=NULL;
					}else {
						tmpbufferpkt->buffernextpkt=packet;
						tmpbufferpkt=packet;
						tmpbufferpkt->buffernextpkt=NULL;
					}
					
					//-----------------------------------------�]�wBuffer size
					tmpsize=Bufnode->NodeBuffer->pktsize;
					if((Bufnode->NodeBuffer->pktsize+ceil(packet->exeload/payload))<=Maxbuffersize){
						Bufnode->NodeBuffer->pktsize=(Bufnode->NodeBuffer->pktsize+ceil(packet->exeload/payload));
					}else{
						Bufnode->NodeBuffer->pktsize=Maxbuffersize;
					}
					intervalsize=Bufnode->NodeBuffer->pktsize-tmpsize;//�i�몺packet�q

					//-----------------------------------------�p��Buffer load
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
	(�p��쥻��Tc �P 
	TDMA �]���y�������j)
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

			//��M�n���Frame (����j��minperiod, �ۮt�̤p��)
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

			//�p��U bound
			for (FrameTable* Ftbl=FrameTbl; Ftbl!=NULL; Ftbl=Ftbl->next_tbl){
				if(Ftbl->Period<=minperiod){
					if(BlockingTime!=Ftbl->Size)
						U_bound=U_bound+(Ftbl->Size/Ftbl->Period);
				}
			}
			U_bound=U_bound+BlockingTime/minperiod;
		
			//�ݬO�_�ischedulable
			if(U_bound>=1){
				reassign=true;
			}
		}
		/*=================================
				���s�p��Event interval
		=================================*/
		if(reassign){
			//��̤j��Size���ק�
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
	
	//-----------------------------------------------------------��NotifyNode�O�_��NULL,��XTDMA slot�U�i�ǿ骺node
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

				NodeBufferSet(NotifyNode);		//�ק�nNotifyNode����Buffer
				Buffer=NotifyNode->NodeBuffer;
			}
		}
	}

	//-----------------------------------------------------------��NotifyNode���ǿ�
	if(Buffer!=NULL){
		BLE_EDF(NotifyNode);
	}
	
	NotifyTable->count--;
}

/*============================================
		�Nnode�W��Buffer���ǿ�
		�w��ثe��Buffer���ǿ�
		�i�J�@���u�|�ǿ�payload�j�p (20bytes)
============================================*/
void BLE_EDF(Node *node){
	
		/*---------------------------------------------
				�P�_Flow �O�_��NULL
				�Y���ʥ]�h�i��ǿ�
				(�]�t�P�_�O�_����)
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
			
			//=============================================����ǿ�(�����p��payload�P�j��payload)
			packet=Buffer->pkt;
			Pktsize=Buffer->pktsize;
			if(packet->exeload<payload){
				Buffer->load=Buffer->load-packet->exeload;
				packet->exeload=0;

				packet->node->State="Transmission";
				packet->State="Transmission";		//�ǿ骬�A
			}else{
				Buffer->load=Buffer->load-payload;
				packet->exeload=packet->exeload-payload;

				packet->node->State="Transmission";
				packet->State="Transmission";		//�ǿ骬�A
			}

			Schdulefile<<" NP:"<<packet->node->id<<","<<packet->id;
			//cout<<"P:"<<packet->id<<endl;

			//=============================================�ǧ�,���U�@packet
			if(packet->exeload==0){

				packet->meetlatency_cnt++;
				packet->meetlatency=packet->meetlatency+(Timeslot - packet->arrival);

				//�P�_�O�_miss deadline
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

				//Buffer���e����
				packet=packet->buffernextpkt;
				Buffer->pkt=packet;
			}

			//=============================================�T�{Buffer�S��packet,�Narrival_flag�]��false
			if(Buffer->load==0){
				tmpnode->arrival_flag=0;
				tmpnode->ContinueNotify=false;
				NotifyNode=NULL;

				int Maxslot=0;	//��XTDMA�̤jSlot id
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
		��SettingNode���nNodeBuffer�]�w
		Refresh SettingNode->NodeBuffer
========================================*/
void NodeBufferSet(Node * SettingNode){
	Node *Bufnode=Head->nextnd;

	//-------------------------------------------------------------�����S�w��node
	while(Bufnode!=SettingNode)
		Bufnode=Bufnode->nextnd;

	//-------------------------------------------------------------�惡node��NodeBuffer�����t
	if(Bufnode==SettingNode){
		Bufnode->NodeBuffer->pktsize=0;//����buffer�����ʥ]�q�M��
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

			if(packet->readyflag!=1){			//�|��ready�A�������U�@packet
				packet=packet->nodereadynextpkt;
			}else{
			
				//�T�{packet���s�b��Buffer��
				bool existflag=false;
				Packet* existpkt=Bufnode->NodeBuffer->pkt;
				while(existpkt!=NULL){
					if(existpkt==packet)
						existflag=true;
					existpkt=existpkt->buffernextpkt;
				}

				if(existflag!=true){
					//-----------------------------------------��JBuffer link list
					if(Bufnode->NodeBuffer->pktsize == 0){
						Bufnode->NodeBuffer->pkt=packet;
						tmpbufferpkt=Bufnode->NodeBuffer->pkt;
						tmpbufferpkt->buffernextpkt=NULL;
					}else {
						tmpbufferpkt->buffernextpkt=packet;
						tmpbufferpkt=packet;
						tmpbufferpkt->buffernextpkt=NULL;
					}
					
					//-----------------------------------------�]�wBuffer size
					tmpsize=Bufnode->NodeBuffer->pktsize;
					if((Bufnode->NodeBuffer->pktsize+ceil(packet->exeload/payload))<=Maxbuffersize){
						Bufnode->NodeBuffer->pktsize=(Bufnode->NodeBuffer->pktsize+ceil(packet->exeload/payload));
					}else{
						Bufnode->NodeBuffer->pktsize=Maxbuffersize;
					}
					intervalsize=Bufnode->NodeBuffer->pktsize-tmpsize;//�i�몺packet�q

					//-----------------------------------------�p��Buffer load
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
	����EIMA�W��node �ǿ�q��(�̷�EDF�覡)
	��node���ǿ�

	node->State���Q�q����node
	node->EvtArrival��event ��F
***********************************************/
FrameTable *Work_tbl=NULL;
void FrameEDFSchedule(){
	
	TDMATable *table=TDMA_Tbl;
	Node *node=Head->nextnd;
	short int notiyfyslot=0;
	short int MaxFrameSize=0;
	bool allframeset=false;		//�T�{����䤤�@��frame�U��currslot enable
	bool NotifyFlag=true;		//�T�{�ǿ�u��@��(ConnSet)
	
	/*----------------------------------------------
		Head��ConnNode�U���O�A�b�p��᪺FrameSize��
		FrameSize�Y�p�⧹���ߧY���U�@TDMA��frame����
		�惡frame�W��node��state�W���ܤ�
	----------------------------------------------*/
	
	if(Head->FrameSize<=0){ //(Head->RecvNode�ثe�O���צ�A������n��FrameSize���ק�)
		
		//��̤pFrameTable��deadline
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
				����Scan duration
	----------------------------------------------*/
	for(Node *n=Head->nextnd; n!=NULL; n=n->nextnd){
		if(n->SendNode!=Head && n->ScanFlag && n->SendNode!=Head->RecvNode){ //�ǿ�Node����Head, n�{�bscan �B�ǿ�Node�{�b�S�ǿ�
			if(n->EXEScanDuration>=0){
				n->EXEScanDuration--;
			}
		}
	}

	/*----------------------------------------------
		ConnSet���Q�q����Node�������ǿ��SCAN
		Notify & Scan�i�P�ɹB�@

		node�O�_��event arrival		(node->EvtArrival)
		node�O�_���Q�q��				(Head->RecvNode, call BLE_EDF(node))
		node energy consumption		(node->State)
	----------------------------------------------*/
	for(Node *n=Head->nextnd; n!=NULL; n=n->nextnd){
		
		//-------------------State��Transmission, �u�bevent arrival �ɤ~��Buffer�W��
		if(Timeslot % int(n->eventinterval)==0){
			NodeBufferSet(n);		//��z�nn����NodeBuffer
			n->EvtArrival =true;	//�惡node�]�wEvtArrival
		}else{
			n->EvtArrival =false;
		}

		//-------------------�i��ǿ� [���Q�q��(node->State=="Transmission") �B event arrival(node->EvtArrival)]
		if(n->State!="Sleep"){
			bool eventarrival=false;
			
			if(n->EvtArrival && n->State=="Notify" && (Head->RecvNode==NULL || Head->RecvNode==n)){
				Head->RecvNode=n;	//Head->RecvNode����


				/******************************************
				******************************************/

				//����Scan pkt���ǿ�
				for(Node *AdvNode=Head->nextnd; AdvNode!=NULL; AdvNode=AdvNode->nextnd){
					if(AdvNode->SendNode==n && AdvNode->EXEScanDuration<0 && AdvNode->ScanFlag){
						
						n->NodeBuffer->load=n->NodeBuffer->load-payload;
						AdvNode->pkt->deadline+=AdvNode->pkt->period;

						//�P�_�O�_miss deadline
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
			
			//-------------------�i��ǿ� (Head->RecvNode�n�T�{�ثe�Snode�ά���enode)
			if(Head->RecvNode==n && NotifyFlag){
				BLE_EDF(n);				//��n���ǿ�

				if(n->NodeBuffer->load==0){
					Head->RecvNode=NULL;
				}
				
				NotifyFlag=false;
			}

			//-------------------State��Scan
			if(n->State=="Scan"){
				//n->ScanDuration--;
			}
		}

		//---------------------------------------Power consumption & State����
		Node_EnergyState(n);	//�p��n��Energy
		if(n->NodeBuffer->load==0 && n->State=="Transmission"){
			n->State="Sleep";
		}
	}

}

/***********************************************
	�����nHead���node��state �ഫcmd
	event arrival��NodeBufferSet�A�ݳ]�w��eEvtArrival�Ӱ����� ����ۤv�ǿ�ɶ��I��F
	�ǿ�]�w������A�NEvtArrival ����
***********************************************/
void TDMASchedule(){
	TDMATable *table=TDMA_Tbl;
	Node *node=Head->nextnd;
	short int notiyfyslot=0;
	short int MaxFrameSize=0;
	bool allframeset=false;		//�T�{����䤤�@��frame�U��currslot enable
	bool NotifyFlag=true;		//�T�{�ǿ�u��@��(ConnSet)
	
	/*----------------------------------------------
		Head��ConnNode�U���O�A�b�p��᪺FrameSize��
		FrameSize�Y�p�⧹���ߧY���U�@TDMA��frame����
		�惡frame�W��node��state�W���ܤ�
	----------------------------------------------*/
	
	if(Head->FrameSize<=0 ){ //(Head->RecvNode�ثe�O���צ�A������n��FrameSize���ק�)
		bool findflag=false;

		//��̤pFrameTable��deadline
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
		ConnSet���Q�q����Node�������ǿ��SCAN
		Notify & Scan�i�P�ɹB�@

		node�O�_��event arrival		(node->EvtArrival)
		node�O�_���Q�q��				(Head->RecvNode, call BLE_EDF(node))
		node energy consumption		(node->State)
	----------------------------------------------*/
	for(Node *n=Head->nextnd; n!=NULL; n=n->nextnd){
		
		//-------------------State��Transmission, �u�bevent arrival �ɤ~��Buffer�W��
		if(Timeslot % int(n->eventinterval)==0){
			NodeBufferSet(n);		//��z�nn����NodeBuffer
			n->EvtArrival =true;	//�惡node�]�wEvtArrival
		}else{
			n->EvtArrival =false;
		}

		//-------------------�i��ǿ� [���Q�q��(node->State=="Transmission") �B event arrival(node->EvtArrival)]
		if(n->State!="Sleep"){
			bool eventarrival=false;
			
			if(n->EvtArrival && n->State=="Notify" && (Head->RecvNode==NULL || Head->RecvNode==n)){
				Head->RecvNode=n;	//Head->RecvNode����
				n->State="Transmission";
			}
			
			//-------------------�i��ǿ� (Head->RecvNode�n�T�{�ثe�Snode�ά���enode)
			if(Head->RecvNode==n && NotifyFlag){
				BLE_EDF(n);				//��n���ǿ�

				if(n->NodeBuffer->load==0){
					Head->RecvNode=NULL;
				}
				
				NotifyFlag=false;
			}

			//-------------------State��Scan
			if(n->State=="Scan"){
				//n->ScanDuration--;
			}
		}

		//---------------------------------------Power consumption & State����
		Node_EnergyState(n);	//�p��n��Energy
		if(n->NodeBuffer->load==0 && n->State=="Transmission"){
			n->State="Sleep";
		}
	}
}

/***********************************************
	����EIMA�W��node �ǿ�q��(�̷�EDF�覡)
	��node���ǿ�

	node->State���Q�q����node
	node->EvtArrival��event ��F

	(Frame Deadline assignment is connection interval between tx event to next event.)
	<D=timeslot+Tc, at event arrival>
***********************************************/
void EIF(){
	
	TDMATable *table=TDMA_Tbl;
	Node *node=Head->nextnd;
	short int notiyfyslot=0;
	short int MaxFrameSize=0;
	bool allframeset=false;		//�T�{����䤤�@��frame�U��currslot enable
	bool NotifyFlag=true;		//�T�{�ǿ�u��@��(ConnSet)
	
	/*----------------------------------------------
		Head��ConnNode�U���O�A�b�p��᪺FrameSize��
		FrameSize�Y�p�⧹���ߧY���U�@TDMA��frame����
		�惡frame�W��node��state�W���ܤ�
	----------------------------------------------*/
	
	if(Head->FrameSize<=0){ //(Head->RecvNode�ثe�O���צ�A������n��FrameSize���ק�)
		
		//��Crtical Frame (Period�̤p)
		FrameTable* CrticalFrame=FrameTbl;
		for(FrameTable* Ftbl=FrameTbl; Ftbl!=NULL; Ftbl=Ftbl->next_tbl){
			if(CrticalFrame->Deadline > Ftbl->Deadline){
				CrticalFrame=Ftbl;
			}
		}

		//��̤pFrameTable��deadline
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
		ConnSet���Q�q����Node�������ǿ��SCAN
		Notify & Scan�i�P�ɹB�@

		node�O�_��event arrival		(node->EvtArrival)
		node�O�_���Q�q��				(Head->RecvNode, call BLE_EDF(node))
		node energy consumption		(node->State)
	----------------------------------------------*/
	for(Node *n=Head->nextnd; n!=NULL; n=n->nextnd){
		
		//-------------------State��Transmission, �u�bevent arrival �ɤ~��Buffer�W��
		if(Timeslot % int(n->eventinterval)==0){
			NodeBufferSet(n);		//��z�nn����NodeBuffer
			n->EvtArrival =true;	//�惡node�]�wEvtArrival
		}else{
			n->EvtArrival =false;
		}

		//-------------------�i��ǿ� [���Q�q��(node->State=="Transmission") �B event arrival(node->EvtArrival)]
		if(n->State!="Sleep"){
			bool eventarrival=false;
			
			if(n->EvtArrival && n->State=="Notify" && (Head->RecvNode==NULL || Head->RecvNode==n)){
				Head->RecvNode=n;	//Head->RecvNode����
				n->State="Transmission";
								
			}
			
			//-------------------�i��ǿ� (Head->RecvNode�n�T�{�ثe�Snode�ά���enode)
			if(Head->RecvNode==n && NotifyFlag){
				BLE_EDF(n);				//��n���ǿ�

				if(n->NodeBuffer->load==0){
					Head->RecvNode=NULL;
				}
				
				NotifyFlag=false;
			}

			//-------------------State��Scan
			if(n->State=="Scan"){
				//n->ScanDuration--;
			}
		}

		//---------------------------------------Power consumption & State����
		Node_EnergyState(n);	//�p��n��Energy
		if(n->NodeBuffer->load==0 && n->State=="Transmission"){
			n->State="Sleep";
		}
	}
}

/*=========================================
			��@node�W��schedule
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
	

	//---------------------------------------�P�_connection event�O�_arrival
	if(Timeslot % int(n->eventinterval)==0){
		if(intervalpropose==3){ //------------Lazy
			LazyOnWrite();		//�Y��OnWrite�h�P�_
		}

		NodeBufferSet(n);
		n->EvtArrival=true;
		
		Head->RecvNode=n;		//Head->RecvNode����
		n->State="Transmission";
	}else{
		n->EvtArrival=false;
	}

	//---------------------------------------�ǿ�
	if(n->State=="Transmission"){
		BLE_EDF(n);				//��n���ǿ�
	}
	Node_EnergyState(n);		//�p��n��Energy

	//---------------------------------------�ǿ駹��
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
	Rate_data=load/(min_deadline-Timeslot);							//�p�⦹�ɪ�data rate
	Rate_BLE=(payload*Maxbuffersize)/(Head->nextnd->eventinterval);	//�p�⦹�ɪ�BLE rate

	if(Rate_data>=Rate_BLE){
		Head->nextnd->eventinterval=1;
		Callbackclock=EXECBclock;		//Reset timer
	}
}
/*=========================================
		Timer Callback
		���sassign connection interval
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

		MaxRate_data=load/(min_deadline-Timeslot);							//�p�⦹�ɪ�data rate
		Rate_CB=(payload*Maxbuffersize)/EXECBclock;					//�p�⦹�ɪ�BLE rate
		MinRate_BLE=(payload*Maxbuffersize)/(long_period);	//�p�⦹�ɪ�BLE rate
		Rate_BLE=(payload*Maxbuffersize)/(Head->nextnd->eventinterval);	//�p�⦹�ɪ�BLE rate

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
			//��ready�n���̤jrate
			
			if(pkt->readyflag){
				if(pkt->rate>MaxRate){
					MaxRate=pkt->rate;
				}
			}

			//��Minimum period
			if(pkt->period<MinPeriod || MinPeriod==-1){
				MinPeriod=pkt->period;
			}
		}

		Head->nextnd->eventinterval=(payload*Maxbuffersize)/MaxRate;
		if(Head->nextnd->eventinterval<1){
			Head->nextnd->eventinterval=1;
		}
		
		//Reset timer
		Callbackclock=Head->nextnd->eventinterval*overheadcount;	//�ܤֻݭn������~�వ���
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
	bool allframeset=false;		//�T�{����䤤�@��frame�U��currslot enable
	bool NotifyFlag=true;		//�T�{�ǿ�u��@��(ConnSet)

	/*----------------------------------------------
		
	----------------------------------------------*/
	if(Head->FrameSize<=0 ){ //(Head->RecvNode�ثe�O���צ�A������n��FrameSize���ק�)
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
				//��̤pperiod�B���Qassign�JCycle
				tmptbl=NULL;
				for(FrameTable* tbl=FrameTbl; tbl!=NULL; tbl=tbl->next_tbl){
					if(tbl->Currentflag==false){	//�|��assign
						if(tmptbl==NULL){
							tmptbl=tbl;
						}else if(tbl->Period < tmptbl->Period){
							tmptbl=tbl;
						}
					}
				}

				//��Jcycle
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

			//�U�@��polling cycle
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
		ConnSet���Q�q����Node�������ǿ��SCAN
		Notify & Scan�i�P�ɹB�@

		node�O�_��event arrival		(node->EvtArrival)
		node�O�_���Q�q��				(Head->RecvNode, call BLE_EDF(node))
		node energy consumption		(node->State)
	----------------------------------------------*/
	for(Node *n=Head->nextnd; n!=NULL; n=n->nextnd){
		
		//-------------------State��Transmission, �u�bevent arrival �ɤ~��Buffer�W��
		if(Timeslot % int(n->eventinterval)==0){
			NodeBufferSet(n);		//��z�nn����NodeBuffer
			n->EvtArrival =true;	//�惡node�]�wEvtArrival
		}else{
			n->EvtArrival =false;
		}

		//-------------------�i��ǿ� [���Q�q��(node->State=="Transmission") �B event arrival(node->EvtArrival)]
		if(n->State!="Sleep"){
			bool eventarrival=false;
			
			if(n->EvtArrival && n->State=="Notify" && (Head->RecvNode==NULL || Head->RecvNode==n)){
				Head->RecvNode=n;	//Head->RecvNode����
				n->State="Transmission";
								
			}
			
			//-------------------�i��ǿ� (Head->RecvNode�n�T�{�ثe�Snode�ά���enode)
			if(Head->RecvNode==n && NotifyFlag){
				BLE_EDF(n);				//��n���ǿ�

				if(n->NodeBuffer->load==0){
					Head->RecvNode=NULL;
				}
				
				NotifyFlag=false;
			}

			//-------------------State��Scan
			if(n->State=="Scan"){
				//n->ScanDuration--;
			}
		}

		//---------------------------------------Power consumption & State����
		Node_EnergyState(n);	//�p��n��Energy
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

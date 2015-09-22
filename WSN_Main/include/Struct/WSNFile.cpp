#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>

#include "WSNFile.h"
#include "WSNStruct.h"
#include "WSNEnergy.h"
#include "../Schedule/FlowSchedule.h"

#undef  _ShowLog

using namespace std;

/*===========================
		GEN & Output 
		File Path
===========================*/
string filename;
fstream GENfile;
fstream Schdulefile;
fstream Finalfile;
fstream Resultfile;

string GENPath="..\\GENresult\\";
string SchedulePath="..\\WSNresult\\DebugPower\\";
string FinalPath="..\\WSNresult\\DebugPower\\";
string ResultPath="..\\WSNresult\\DebugPower\\";

/*===========================
		�NGEN����ƨ��J �B
		�إ߿�X���
===========================*/
void CreateFile(float U,int Set){
	//��JGEN���ɦW
	filename="U";filename.append(to_string(U));filename.append("_Set");filename.append(to_string(Set));filename.append(".txt");

	//��JGEN�� ���|+�ɦW
	string GENBuffer=GENPath;
	GENBuffer.append(filename);

	//=========================�}��GENFile
	GENfile.open(GENBuffer, ios::in);	//�}���ɮ�.�g�J���A
	if(!GENfile){//�p�G�}���ɮץ��ѡAfp��0�F���\�Afp���D0
		cout<<"Fail to open file: "<<GENBuffer<<endl;
		
		system("PAUSE");
	}

	//========================�}��ScheduleFile
	string ScheduleFileBuffer=SchedulePath;
	ScheduleFileBuffer.append("Schedule_");
	ScheduleFileBuffer.append(filename);

	Schdulefile.open(ScheduleFileBuffer, ios::out);	//�}���ɮ�.�g�J���A
	if(!Schdulefile){//�p�G�}���ɮץ��ѡAfp��0�F���\�Afp���D0
		cout<<"Fail to open Schedule file: "<<ScheduleFileBuffer<<endl;		
		system("PAUSE");
	}

	//========================�}��Resultfile
	string ResultBuffer=ResultPath;
	ResultBuffer.append("Result_");
	ResultBuffer.append(filename);

	Resultfile.open(ResultBuffer, ios::out);	//�}���ɮ�.�g�J���A
	if(!Resultfile){//�p�G�}���ɮץ��ѡAfp��0�F���\�Afp���D0
		cout<<"Fail to open Result file: "<<ResultBuffer<<endl;
		system("PAUSE");
	}

	//========================�}��Finalfile
	string FinalBuffer=FinalPath;
	FinalBuffer.append("FinalResult.txt");
	if(Finalfile.is_open()==false){
		Finalfile.open(FinalBuffer, ios::out);	//�}���ɮ�.�g�J���A
		if(!Finalfile){//�p�G�}���ɮץ��ѡAfp��0�F���\�Afp���D0
			cout<<"Fail to open Final file: "<<FinalBuffer<<endl;
			system("PAUSE");
		}
	}
}

/*===========================
	��@taskset�x�s
===========================*/
void SaveFile(short int setnum){
	double totalenergy=0;
	node=Head->nextnd;
	while(node!=NULL){
		node->energy=node->energy*Vcc;
		node->lifetime=BatteryCapacity/((node->energy/Vcc)/(Hyperperiod*0.01));

		#ifdef _ShowLog
				cout<<"Node"<<node->id<<" E:"<<node->energy<<" Lifetime:"<<node->lifetime<<endl;
		#endif

		totalenergy=totalenergy+node->energy;
		node=node->nextnd;
	}
			
	Resultfile<<"TotalEnergy:"<<totalenergy<<endl;
	#ifdef _ShowLog
		cout<<"TotalEnergy:"<<totalenergy<<endl;
	#endif
	
	//---------------------------------------------------TDMA table
	TDMATable *FlowTable=TDMA_Tbl;
	while(FlowTable!=NULL){
		#ifdef _ShowLog
				cout<<"S"<<FlowTable->slot<<" n"<<FlowTable->n1->id<<endl;
		#endif
		Resultfile<<"S"<<FlowTable->slot<<" n"<<FlowTable->n1->id<<endl;

		FlowTable=FlowTable->next_tbl;
	}

	//---------------------------------------------------�s���U��node��T
	Resultfile<<"	Lifetime Energy Tc NotifyEvt TranEvt, Color Conflict_node, coor_x coor_y Send_node"<<endl;

	node=Head->nextnd;
	while(node!=NULL){
		double AvgRate=0;
		double Pktnum=0;
		packet=node->pkt;
		while(packet!=NULL){
			AvgRate=AvgRate+packet->rate;
			Pktnum++;
			packet=packet->nodenextpkt;
		}
		AvgRate=AvgRate/Pktnum;

		//Node id:energy connectioninterval,color conflictnode...,coor_x coor_y SendNode 
		Resultfile<<"Node"<<node->id<<":";
		Resultfile<<node->lifetime<<" "<<node->energy<<" "<<node->eventinterval<<" "<<node->Notify_evtcount<<" "<<node->Tran_evtcount<<",";
		
		Resultfile<<node->color<<" ";
		Edge *printedge=ConflictEdge;
		while(printedge!=NULL){
			if(printedge->n1->id==node->id)
				Resultfile<<printedge->n2->id<<" ";
			printedge=printedge->next_edge;
		}
		Resultfile<<",";

		Resultfile<<node->coor_x<<" "<<node->coor_y<<" ";
		Resultfile<<node->SendNode->id<<endl;

		//Resultfile<<"Total Event:"<<totalevent<<endl;

		node=node->nextnd;
	}
	#ifdef _ShowLog
	cout<<"	FrameSize, FramePeriod"<<endl;
	#endif
	for(FrameTable* Ftbl=FrameTbl; Ftbl!=NULL; Ftbl=Ftbl->next_tbl){
	#ifdef _ShowLog
			cout<<"Frame"	<<Ftbl->id<<":"
							<<Ftbl->Size<<", "
							<<Ftbl->Period<<endl;
	#endif
		Resultfile<<"Frame"	<<Ftbl->id<<":"
							<<Ftbl->Size<<", "
							<<Ftbl->Period<<endl;
	}

	if(Meetflag==true){
		Resultfile<<"Meet Deadline:MEET"<<endl;
		#ifdef _ShowLog
				cout<<"Meet Deadsline:MEET"<<endl;
		#endif
				
		node=Head->nextnd;
		SetNode=SetHead->nextnd;
		while(node!=NULL){
			SetNode->avgenergy=SetNode->avgenergy+node->energy;

			SetNode=SetNode->nextnd;
			node=node->nextnd;
		}
		SetNode=SetHead->nextnd;

		AverageE=AverageE+totalenergy;
		Meetcount++;
	}
	else{
		Resultfile<<"Meet Deadline:MISS"<<endl;
		#ifdef _ShowLog
		cout<<"Meet Deadline:MISS"<<endl;
		#endif
	}
	
	Resultfile<<"==============================================="<<setnum<<endl;
	Schdulefile<<"=============================================="<<setnum<<endl;
	#ifdef _ShowLog
	cout<<"=============================================="<<setnum<<endl;
	#endif
}

/*===========================
	���Q�βvSet�x�s
===========================*/
void SaveSet(int Set){
	cout<<"FinalResult"<<endl;
	cout<<"Meet="<<Meetcount<<endl;
	cout<<"Miss="<<Set-Meetcount<<endl;
	cout<<"MeetRatio="<<Meetcount/Set<<endl;
	SetNode=SetHead->nextnd;
	while(SetNode!=NULL){
		cout<<"Node"<<SetNode->id<<"="<<SetNode->avgenergy/Meetcount<<endl;
		SetNode=SetNode->nextnd;
	}
	cout<<"AverageEnergy="<<AverageE/Meetcount<<endl;
	cout<<"=============================================="<<endl;

	Finalfile<<"FinalResult"<<endl;
	Finalfile<<"Meet="<<Meetcount<<endl;
	Finalfile<<"Miss="<<Set-Meetcount<<endl;
	Finalfile<<"MeetRatio="<<Meetcount/Set<<endl;
	SetNode=SetHead->nextnd;
	while(SetNode!=NULL){
		Finalfile<<"Node"<<SetNode->id<<"="<<SetNode->avgenergy/Meetcount<<endl;
		SetNode=SetNode->nextnd;
	}
	Finalfile<<"AverageEnergy="<<AverageE/Meetcount<<endl;
	Finalfile<<"=============================================="<<endl;

	GENfile.close();
	Schdulefile.close();
	Resultfile.close();
}

void CloseFinal(){
	Finalfile.close();
}
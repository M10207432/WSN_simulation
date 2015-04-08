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

using namespace std;
/*=================================
		Structure
==================================*/
struct Flow{
	struct Packet* pkt;
	struct Node* sourcenode;
	struct Node* destinationnode;
};
struct Node{
	double coor_x,coor_y;//�y��
	double distanceto_BS;//��Base station �Z��
	double energy;
	int id;
	short int ExTimeslot;
	short int LatestTimeslot;

	string State;		//Wakeup�BSleep�BTransmission & Idle
	Packet* pkt;
	Node* nextnd;
	Node* prend;
};
struct Packet{
	int id;
	int load;
	int exeload;
	double arrival;
	double period;
	double deadline;
	double utilization;
	int	 pirority;
	int	 doneflag;
	int	 readyflag;
	int	 searchdone;
	string State;		//Transmission �B Idle

	short int hop;		//range 1~3
	short int exehop;		//range 1~3
	int destination;	//Nest node
	double rate;

	Node* node;
	Packet* nextpkt;
	Packet* prepkt;
	Packet* nodenextpkt;
	Packet* nodeprepkt;
	Packet* readynextpkt;
	Packet* readyprepkt;

	Packet* buffernextpkt;
};
struct PacketBuffer{
	double pktsize;	//1~6 packets
	int load;		//0~120bytes
	Packet* pkt;
};

struct DIFtable{
	double load;
	double length;
	double density;
	double arrival;
	double deadline;
};

void StructGEN();				
void PacketQueue();
void BufferSet();
void FlowEDF();
void AvgTransmissionRate();
void AssignRate();
void NodeEnergy();
double IntervalPower(int,int);
void NodeState();				//���ܭ�Node���A

void DIF();						//Densest Interval First����աApreemptionenable �nenable
void TSB();						//Total Size Block
void Rate_TO_Interval(int);		//Nonpreemption ��k
/*=================================
		Global value
==================================*/
const float MIN_Uti=1.0;
const float MAX_Uti=2.0;
const short int Set=50;
string GENPath="..\\GENresult\\";
string SchedulePath="..\\WSNresult\\TSBResult_ver2\\";
string PowerPath="..\\WSNresult\\TSBResult_ver2\\";
string ResultPath="..\\WSNresult\\TSBResult_ver2\\";
short int Rateproposal=2;				//AssignRate()������k�s�� 0=>Event, 1=>TSB, 2=>DIF, 
bool preemptionenable=true;			//�]�w�i�_preemption

int Flowinterval=0;					//Ĳ�o�i�Jflow��conneciton interval
int Pktsize;							//�p��IntervalPower��pkt num
double DIFMinperiod=0;
double Meetcount=0;
double AverageE=0;
string filename;
fstream GENfile;
fstream Schdulefile;
fstream Powerfile;
fstream Resultfile;

PacketBuffer* Buffer=new PacketBuffer;
Node* Head=new Node;
Packet* Headpacket=new Packet;
Node* node=new Node;
Packet* packet=new Packet;
Packet *ReadyQ=new Packet();
Flow *Headflow=new Flow();
int ReadyQ_overflag=0;

stringstream stream;
string strload,strperiod,strutilization,strhop;
int nodenum=0;
int pktnum=0;
long int Timeslot=0;
long int Hyperperiod=0;
double Maxrate=20;					//�̰��t�׬�20bytes/slot
double payload=20;					//payload �� 20bytes
int Maxbuffersize=6;				//Maxbuffersize �� 6��packets
double slotinterval=1;				//Time slot���Z��10ms
double Connectioninterval=0;		//Conneciton inteval �u�|�b10ms~4000ms
double totalevent=0;				//Event�ƶq
bool Meetflag=true;					//�ݬO�_meet deadline
/*====================
		Power function parameter
====================*/
double Vcc=3.3;			//BLE �X�ʹq��
double Isleep=0.003;	//Sleep �q�y 
double Ie=0.07679763;		//�ǿ�p�� �q�y  0.002
double Te=0.0002768;		//�ǿ�ɶ�1.25ms
double K=1;				//Rate power�`��
double unit=0.01;		//�ɶ���쬰10ms
double TotalEnergy=0;
double parma=0.00191571992;
double parmb=24.4058498;

int main(){

	for(float U=MIN_Uti; U<=MAX_Uti; U++){
		Meetcount=0;
		AverageE=0;
		totalevent=0;

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
			return 0;
		}

		//========================�}��ScheduleFile
		string ScheduleFileBuffer=SchedulePath;
		ScheduleFileBuffer.append("Schedule_");
		ScheduleFileBuffer.append(filename);

		Schdulefile.open(ScheduleFileBuffer, ios::out);	//�}���ɮ�.�g�J���A
		if(!Schdulefile){//�p�G�}���ɮץ��ѡAfp��0�F���\�Afp���D0
			cout<<"Fail to open file: "<<ScheduleFileBuffer<<endl;
			system("PAUSE");
			return 0;
		}
		//========================�}��PowerPath
		string PowerFileBuffer=PowerPath;
		PowerFileBuffer.append("Power_");
		PowerFileBuffer.append(filename);

		Powerfile.open(PowerFileBuffer, ios::out);	//�}���ɮ�.�g�J���A
		if(!Powerfile){//�p�G�}���ɮץ��ѡAfp��0�F���\�Afp���D0
			cout<<"Fail to open file: "<<PowerFileBuffer<<endl;
			system("PAUSE");
			return 0;
		}

		//========================�}��Resultfile
		string ResultBuffer=ResultPath;
		ResultBuffer.append("Result_");
		ResultBuffer.append(filename);

		Resultfile.open(ResultBuffer, ios::out);	//�}���ɮ�.�g�J���A
		if(!Resultfile){//�p�G�}���ɮץ��ѡAfp��0�F���\�Afp���D0
			cout<<"Fail to open file: "<<ResultBuffer<<endl;
			system("PAUSE");
			return 0;
		}

		/*===================================================
							�b�P�@�Q�βv�U
							�]Set��
		===================================================*/
		for(short int setnum=0;setnum<Set;setnum++){
			Meetflag=true;
			Timeslot=0;
			Hyperperiod=0;
			totalevent=0;
			/*==========================
				�إ�Linklist�H��
				GEN����Ʃ�i�h
			==========================*/
			StructGEN();

			/*==========================
				�p��Connection interval
			==========================*/
			switch (Rateproposal)
			{
			case 0:
				Connectioninterval=1;
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

			Flowinterval=Connectioninterval;
			
			//��m�ǿ���q
			packet=Head->nextnd->pkt;
			while(packet!=NULL){
				packet->exeload=packet->load;
				packet->exehop=packet->hop;

				packet=packet->nextpkt;
			}
			/*==========================
					EDF scheduling
			==========================*/
			Headflow->pkt=NULL;//�@�}�l��flow���]�t���ʥ]�]�w��NULL
			
			while(Timeslot<Hyperperiod){
				Powerfile<<Timeslot;

				if(Timeslot%Flowinterval==0){
					PacketQueue();	
					BufferSet();

					if(Rateproposal==2){
						Rate_TO_Interval(DIFMinperiod);
						Flowinterval=Connectioninterval;	
					}

					FlowEDF();	//�D�nScheduling
				}

				Powerfile<<endl;
				Timeslot++;
			}
			
			/*==========================
					END
			==========================*/
			double totalenergy=0;
			node=Head->nextnd;
			while(node!=NULL){
				cout<<"Node"<<node->id<<" E:"<<node->energy<<endl;
				totalenergy=totalenergy+node->energy;
				node=node->nextnd;
			}
			
			Resultfile<<"TotalEnergy:"<<totalenergy<<endl;
			cout<<"TotalEnergy:"<<totalenergy<<endl;

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

				Resultfile<<"Node"<<node->id<<endl;
				Resultfile<<"E:"<<node->energy<<endl;
				Resultfile<<"Connectioninterval:"<<Connectioninterval<<endl;
				Resultfile<<"Total Event:"<<totalevent<<endl;

				node=node->nextnd;
			}
			if(Meetflag==true){
				Resultfile<<"Meet Deadline:MEET"<<endl;
				cout<<"Meet Deadline:MEET"<<endl;

				AverageE=AverageE+totalenergy;
				Meetcount++;
			}
			else{
				Resultfile<<"Meet Deadline:MISS"<<endl;
				cout<<"Meet Deadline:MISS"<<endl;
			}
	
			Resultfile<<"=============================================="<<endl;
			Schdulefile<<"=============================================="<<endl;
			cout<<"=============================================="<<setnum<<endl;
		}//Set End
		cout<<"FinalResult"<<endl;
		cout<<"Meet="<<Meetcount<<endl;
		cout<<"Miss="<<Set-Meetcount<<endl;
		cout<<"MeetRatio="<<Meetcount/Set<<endl;
		cout<<"AverageEnergy="<<AverageE/Meetcount<<endl;
		cout<<"=============================================="<<endl;

		Resultfile<<"FinalResult"<<endl;
		Resultfile<<"Meet="<<Meetcount<<endl;
		Resultfile<<"Miss="<<Set-Meetcount<<endl;
		Resultfile<<"MeetRatio="<<Meetcount/Set<<endl;
		Resultfile<<"AverageEnergy="<<AverageE/Meetcount<<endl;

		GENfile.close();
		Schdulefile.close();
		Powerfile.close();
		Resultfile.close();
	}

	system("PAUSE");
	return 0;
}

/*==========================
	Flow EDF Scheduling
==========================*/
void FlowEDF(){
		
		/*---------------------------------------------
				�P�_Flow �O�_��NULL
				�Y���ʥ]�h�i��ǿ�
				(�]�t�P�_�O�_����)
		---------------------------------------------*/
		if(Buffer->pkt!=NULL){
			totalevent++;

			//cout<<"Time slot:"<<Timeslot;
			Schdulefile<<"Time slot:"<<Timeslot;
			//=============================================����ǿ�
			packet=Buffer->pkt;
			Pktsize=Buffer->pktsize;

			while(Buffer->load!=0){
								
				packet->exeload--;
				Buffer->load--;
				packet->State="Transmission";		//�ǿ骬�A

				NodeState();	//����Node���A

				if(packet->exeload==0){

					//cout<<" Packet:"<<packet->id;
					Schdulefile<<" Packet:"<<packet->id;
					
					//�P�_�O�_�ݭnhop
					packet->exehop--;
					if(packet->exehop>0)
					{
						packet->exeload=packet->load;
						Headflow->pkt=packet;
					}else{
						//�P�_�O�_miss deadline
						if((Timeslot)>packet->deadline){
							//cout<<"Time slot:"<<Timeslot<<"  PKT"<<packet->id<<" Miss deadline"<<endl;
							Schdulefile<<"(PKT"<<packet->id<<" Miss deadline"<<" Deadline "<<packet->deadline<<")";

							Meetflag=false;
							//system("PAUSE");
						}
						
						packet->readyflag=0;
						packet->exeload=packet->load;
						packet->arrival=packet->deadline;
						packet->deadline=packet->deadline+packet->period;
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
				Schdulefile<<" Packet:"<<packet->id;
				packet->State="Transmission";		//�ǿ骬�A
			}
			
			//cout<<endl;
			Schdulefile<<endl;
			/*---------------------------
				�ǿ駹�ߧY��
				���A���� & Energy �p��
			---------------------------*/
			NodeEnergy();	//�p��ӷP����Energy

		}else{
			NodeState();	//����Node���A
			NodeEnergy();

			//cout<<"Time slot:"<<Timeslot<<" IDLE"<<endl;
			Schdulefile<<"Time slot:"<<Timeslot<<" IDLE"<<endl;
			
		}
		//�U�@�Ӯɶ��I
		//Timeslot++;
}
/*==========================================
			Arrange Queue
{ReadyQ & WaitQ is assign by pkt->readyflag}
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
	//----------------------------------------Ready Queue done
}
/*=============================================	
		�إߦnBuffer�W��packet
	pkt link list, load, packet size
=============================================*/
void BufferSet(){
	
	Buffer->pktsize=0;//����buffer�����ʥ]�q�M��
	Buffer->load=0;
	Buffer->pkt=NULL;
		
	Packet *tmpbufferpkt;

	if(preemptionenable!=true){
		//���bBuffer�W���٥�������packet�A����Buffer�̫�@��packet
		if(Headflow->pkt!=NULL){
			//��JBuffer link list
			Buffer->pkt=Headflow->pkt;
			tmpbufferpkt=Buffer->pkt;
				
			//�]�wBuffer size
			if((Buffer->pktsize+ceil(tmpbufferpkt->exeload/payload))<=Maxbuffersize){
				Buffer->pktsize=Buffer->pktsize+ceil(tmpbufferpkt->exeload/payload);//�٦�packet
			}else{
				Buffer->pktsize=Maxbuffersize;
			}
				
			//�p��Buffer load
			if(tmpbufferpkt->exeload<(Maxbuffersize*payload))
				Buffer->load=tmpbufferpkt->exeload;
			else
				Buffer->load=Maxbuffersize*payload;

			tmpbufferpkt->buffernextpkt=NULL;
		}	
	}

	int tmpsize=Buffer->pktsize;
	int intervalsize=0;

	packet=ReadyQ->readynextpkt;
	if(Buffer->pktsize!=0){
		while(packet->readyflag!=1)			
			packet=packet->readynextpkt;
		if(packet==tmpbufferpkt)
			packet=packet->readynextpkt;
	}

	while(Buffer->pktsize<Maxbuffersize && packet!=NULL){

		if(packet->readyflag!=1){			//�|��ready
			packet=packet->readynextpkt;
		}else{
			
			//�T�{packet���s�b��Buffer��
			bool existflag=false;
			Packet* existpkt=Buffer->pkt;
			while(existpkt!=NULL){
				if(existpkt==packet)
					existflag=true;
				existpkt=existpkt->buffernextpkt;
			}


			if(existflag!=true){
				//-----------------------------------------��JBuffer link list
				if(Buffer->pktsize == 0){
					Buffer->pkt=packet;
					tmpbufferpkt=Buffer->pkt;
					tmpbufferpkt->buffernextpkt=NULL;
				}else {
					tmpbufferpkt->buffernextpkt=packet;
					tmpbufferpkt=packet;
					tmpbufferpkt->buffernextpkt=NULL;
				}
					
				//-----------------------------------------�]�wBuffer size
				tmpsize=Buffer->pktsize;
				if((Buffer->pktsize+ceil(packet->load/payload))<=Maxbuffersize){
					Buffer->pktsize=(Buffer->pktsize+ceil(packet->load/payload));
				}else{
					Buffer->pktsize=Maxbuffersize;
				}
				intervalsize=Buffer->pktsize-tmpsize;//�i�몺packet�q

				//-----------------------------------------�p��Buffer load
				if(packet->load>payload){
					double tmpload=packet->load;
					while(intervalsize!=0){
						if(tmpload>payload)
							Buffer->load=Buffer->load+payload;
						else
							Buffer->load=Buffer->load+tmpload;
						tmpload=tmpload-payload;
						intervalsize--;
					}

				}else{
					Buffer->load=Buffer->load+packet->load;
				}
			}
			packet=packet->readynextpkt;
		}
	}

}
/*==========================================
		���O�p��U��node�W��Power
		�C�@slot�W���[�`
		�Y��E=P*t
==========================================*/
void NodeEnergy(){
	/*----------------------------
		Wakeup Sleep	  ���s�����p
		Transmission Idle �s�����p
	----------------------------*/
	Node *Enode;
	Packet *Epkt;
	
	Enode=Head->nextnd;
	while(Enode!=NULL){
		Enode->LatestTimeslot=Timeslot;
		Enode=Enode->nextnd;
	}

	Enode=Head->nextnd;
	while(Enode!=NULL){
		//==========================Wakeup ���A
		if(Enode->State=="Wakeup"){
			
		}
		//==========================Sleep ���A
		if(Enode->State=="Sleep"){
			
		}
		//==========================Transmission ���A
		if(Enode->State=="Transmission"){
			double P=0;

			P=IntervalPower(Pktsize,Enode->LatestTimeslot-Enode->ExTimeslot);
			Enode->energy=Enode->energy+P;
			//cout<<"Interval:"<<(Enode->LatestTimeslot-Enode->ExTimeslot)<<" E:"<<Enode->energy<<endl;
			Powerfile<<","<<P;
			Enode->ExTimeslot=Enode->LatestTimeslot;//���������o�e�ɶ�
		}
		//==========================Idle ���A
		if(Enode->State=="Idle"){
			Powerfile<<","<<0;
		}

		Enode=Enode->nextnd;
	}

}
/*==========================================
		�p��b��Connection interval�U��
			Power consumption
==========================================*/
double IntervalPower(int Pktnum,int interval){
	if(interval!=0){
		double Ipeak,Vpeak;
		double Tc;
		double power;

		Tc=interval*unit;
		
		//Ipeak=(Pktnum*Ie*Te+Isleep*(Tc-Te*Pktnum))/Tc;
		Ipeak=parma*Pktnum*exp(-parmb*Tc)+Isleep;
		
		Vpeak=Vcc-Ipeak;

		power=Vcc*Ipeak-(Ipeak*Ipeak)*K;
	
		//cout<<"T:"<<Timeslot<<" P:"<<power<<endl;

		return power;
	}else
		return 0;
}

/*==========================================
		�p��Node State
==========================================*/

void NodeState(){
	Node   *Snode;
	Packet *Spacket;
	bool tx_flag=false;

	Snode=Head->nextnd;
	
	while(Snode!=NULL){
		Spacket=Snode->pkt;
		while(Spacket!=NULL){
			if(Spacket->State=="Transmission"){
				Snode->State="Transmission";
				tx_flag=true;
			}
			if(Spacket->State=="Idle" && tx_flag!=true){
				Snode->State="Idle";
			}

			Spacket=Spacket->nodenextpkt;
		}
		Snode=Snode->nextnd;
		tx_flag=false;
	}
	delete Snode;
	delete Spacket;
}
/*===========================
		�إ�linklist
		�ñN�T���g�J
===========================*/
void StructGEN(){
	
	delete Head;
	Head=NULL;
	delete Headpacket;delete node;delete packet;delete Headflow;
	Headpacket=NULL;node=NULL;packet=NULL;Headflow=NULL;

	Head=new Node;
	Headpacket=new Packet;
	node=new Node;
	packet=new Packet;
	ReadyQ=new Packet;
	Headflow=new Flow;
	
	/*==========================
			�إ�Link list
		    Node & Packet
	===========================*/
	/*-------------------------
		Gen node(Linklist)
	-------------------------*/
	string str;
	GENfile>>str;
	stream<<str; stream>>nodenum; stream.clear();
	GENfile>>str;
	stream<<str; stream>>pktnum; stream.clear();
	GENfile>>str;
	stream<<str; stream>>Hyperperiod;

	Head->nextnd=node;
	for(int n=0;n<nodenum;n++){
		/*-------------------------
			packet(Linklist)
		-------------------------*/

		node->pkt=packet;
		packet->nodeprepkt=NULL;
		for (int p=0;p<pktnum;p++){

			Packet* nextpacket=new Packet;
			Packet* prepacket=packet;

			packet->nextpkt=nextpacket;
			packet->nodenextpkt=nextpacket;
			packet->readynextpkt=packet->nextpkt;
			packet=nextpacket;
			packet->prepkt=prepacket;
			packet->nodeprepkt=prepacket;
			packet->readyprepkt=prepacket;
		}
		packet->nodeprepkt->nodenextpkt=NULL;

		//--------------------------Packet Done

		Node* nextnode=new Node;
		Node* prenode=node;
		node->nextnd=nextnode;
		node=nextnode;
		node->prend=prenode;
	}
	packet=packet->prepkt;
	packet->nextpkt=NULL;
	packet->readynextpkt=NULL;
	node=node->prend;
	node->nextnd=NULL;

	Headpacket->nextpkt=Head->nextnd->pkt;
	Head->nextnd->pkt->prepkt=Headpacket;
	Headpacket->readynextpkt=Head->nextnd->pkt;
	Head->nextnd->pkt->readyprepkt=Headpacket;
	//-----------------------------Node Done
	
	/*==========================
		�g�JGEN����T
	==========================*/
	node=Head;
	int pktid=1;
	int ndid=1;
	while(str!="=========="){
		
		GENfile>>str;
		if(str=="Node"){
			node=node->nextnd;
			node->energy=0;
			packet=node->pkt;

			node->id=ndid++;
		}
		if(str=="Pkt"){
			GENfile>>strload>>strperiod>>strutilization>>strhop;
			
			stream.clear();	stream<<strload;stream>>packet->load;					//Load
			stream.clear();	stream<<strperiod;stream>>packet->period;				//Period
			stream.clear();	stream<<strutilization;	stream>>packet->utilization;	//Utilization
			stream.clear();	stream<<strhop;	stream>>packet->hop;	//Hop

			packet->load=(packet->load);

			packet->period=packet->period;
			packet->node=node;														//���ݪ��P����
			packet->exehop=packet->hop;
			packet->exeload=packet->load;
			packet->arrival=0;														//Arrival
			packet->deadline=packet->arrival+packet->period;						//Deadline
			packet->id=pktid++;														//id
			packet->readyflag=1;													//ready flag
			packet->searchdone=0;													//ready flag
			packet->rate=0;
			packet->State="Idle";
			//�U�@��packet
			packet=packet->nextpkt;
		}
	}
}

/*===========================
		�����
	��U�Ӱ϶�(interval)
	<arrival -> period> 
step1:��U�϶� ���㪺packet
step2:�U�϶���(packet->load�[�`) ���H (interval)
step3:�p��U�϶� rate 
step4:��X�̤jrate , ��b�϶���packet assign ��rate
(��϶��ɭn�N��rate���϶��ɶ�����)
===========================*/
void DIF(){
	PacketQueue();
	DIFMinperiod=ReadyQ->readynextpkt->period;

	Packet * DIFpacket;
	map<double,map<double,DIFtable>> Table;	//�G��map ���e�榡��DIFtable
	double maxarrvial,maxdeadline;			//��̤jDensity�� ���϶�
	double Maxdesity=0;						//��϶��� �̤jDensity
	bool Doneflag=false;					//����assign��rate
	
	while(Doneflag!=true){
		/*============================
			DIF init
			��U�϶�
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
			�U�϶���(packet->load�[�`)
				   (�϶�����)
				   (�p��U�϶� rate)
		============================*/
		for (map<double,map<double,DIFtable>>::iterator a=Table.begin(); a!=Table.end(); a++){
			for(map<double,DIFtable>::iterator p=Table[a->first].begin(); p!=Table[a->first].end(); p++){
				DIFpacket=Head->nextnd->pkt;

				while(DIFpacket!=NULL){
					//�T�warrival �� deadline�p
					if(a->first < p->first){
						if(a->first <= DIFpacket->arrival && DIFpacket->deadline <= p->first && DIFpacket->rate==0){
							Packet* tmpDIFpacket;
							double start=a->first;
							double end=p->first;
							
							//��J�϶��B�϶���load�`�� �H�� ���϶�Density
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
							
							//��X�̤jrate,�ì����϶�
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
			�b�϶���X�϶�����packet
		��assign rate �� packet->rate�W
		============================*/
		DIFpacket=Head->nextnd->pkt;
		while(DIFpacket!=NULL){
			if(maxarrvial <= DIFpacket->arrival && DIFpacket->deadline <= maxdeadline ){
				//�|��assign rate
				if(DIFpacket->rate==0){
					DIFpacket->rate=Table[maxarrvial][maxdeadline].density;
					
					DIFpacket->rate=(DIFpacket->rate);
				}
			}

			DIFpacket=DIFpacket->nextpkt;
		}
		Maxdesity=0;

		/*==================
			�T�{�Ҧ�packet
			�w�gassign�� rate
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
/*==============================================
(�p��⭿Minperiod��pkt size)	->Minsize
	(�̤jload��pkt size)			->Maxsize

	�Y(2*Minsize+Maxsize)�j��⭿Buffersize
		Connection interval��Minperiod/2
	�_
		�̷Ӷ��jbuffersize���p��
==============================================*/
void TSB(){
	PacketQueue();		//����Ready Queue
	Packet *TSBpktQ=ReadyQ;
	Packet *TSBpkt=Head->nextnd->pkt;
	double Tc=0;
	
	//===========================================��Maxload
	double Minperiod=TSBpktQ->readynextpkt->period;//��Minperiod
	double Maxload=0;
	double Maxsize=0;

	while(TSBpkt!=NULL){
		if(TSBpkt->load > Maxload)
			Maxload=TSBpkt->load;
		TSBpkt=TSBpkt->nextpkt;
	}
	TSBpkt=Head->nextnd->pkt;
	while(TSBpkt!=NULL){
		if(TSBpkt->load == Maxload)
			Maxsize++;
		TSBpkt=TSBpkt->nextpkt;
	}

	//===========================================��X2��Minperiod�����̤jblock���q
	Maxsize=Maxsize*ceil(Maxload/payload);


	/*
	double Minsize=0;
	
	while(TSBpktQ->readynextpkt->period < 2*Minperiod){
		Minsize++;
		TSBpktQ=TSBpktQ->readynextpkt;
		if(TSBpktQ==NULL)
			break;
	}
	*/
	/*---------------------------------
	(2*Minsize+Maxsize)�j��⭿Buffersize
	---------------------------------*/
	
	if(false){//(2*Minsize+Maxsize) > 2*Maxbuffersize (���ѵ�non-preemption ��)
		Tc=floor(Minperiod/2);
	}else{
		PacketQueue();		//����Ready Queue
		Packet *TSBpkt=ReadyQ;
		double Totalsize=0;
		double PacketSize=0;
		double Tslot=0;
		bool doneflag=false;

		//======================��Minperiod, �]�w��Tc init
		Tc=TSBpkt->readynextpkt->period;

		//======================���R�C�@period�U, �O�_��meet deadline
		Tslot=TSBpkt->readynextpkt->period;
	
		while(doneflag!=true){
			Totalsize=0;

			//��X�һ�buffer�q
			TSBpkt=ReadyQ->readynextpkt;
			while(TSBpkt->period <= Tslot){
				Totalsize=Totalsize+(ceil(TSBpkt->load/payload)*ceil(Tslot/TSBpkt->period));	
				TSBpkt=TSBpkt->readynextpkt;
				if(TSBpkt==NULL)
					break;
			}

			PacketSize=floor(Tslot/Tc)*double(Maxbuffersize);
			while(Totalsize > PacketSize){
				Tc--;
				PacketSize=floor(Tslot/Tc)*double(Maxbuffersize);
			}

			//��sTime slot
			if(TSBpkt!=NULL){
				Tslot=TSBpkt->period;
			}else{
				doneflag=true;
			}
		}
	}

	Connectioninterval=Tc;

}
void Rate_TO_Interval(int defaultMinperiod){
	//�P�_�O�_�ݭn����Tc
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
		Connectioninterval=exconnectioninterval;//�]�w�����e��interval
	}else{
		Connectioninterval=floor(Connectioninterval);
	}

	//�P�_Connectioninterval�O�_�j��Minperiod
	if(Connectioninterval>defaultMinperiod)
		Connectioninterval=defaultMinperiod;

}
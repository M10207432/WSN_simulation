#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "WSNFile.h"
#include "WSNStruct.h"
#include "../Schedule/FlowSchedule.h"
#include "WSNEnergy.h"

using namespace std;

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
		//==========================Sleep ���A
		if(Enode->State=="Sleep"){
			double P=0;
			P=Vcc*I_sleep;
			Enode->energy=Enode->energy+P;
			Enode->State="Sleep";
		}
		//==========================Transmission ���A
		if(Enode->State=="Transmission"){
			double P=0;

			//P=IntervalPower(Pktsize,Enode->LatestTimeslot-Enode->ExTimeslot);
			
			//cout<<"Interval:"<<(Enode->LatestTimeslot-Enode->ExTimeslot)<<" E:"<<Enode->energy<<endl;
			//Enode->ExTimeslot=Enode->LatestTimeslot;//���������o�e�ɶ�
			P=Vcc*I_notify;
			Enode->energy=Enode->energy+P;
			Enode->State="Sleep";
			
		}
		//==========================Scan ���A
		if(Enode->State=="Scan"){
			
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
		Ipeak=parma*Pktnum*exp(-parmb*Tc)+I_sleep;
		
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

/*========================================
		����e��Node��Energy�W���p��
		�B��buffer�b�šA�ߧY����State
========================================*/

void Node_EnergyState(Node *node){

	if(node!=NULL){
		//==========================Sleep ���A
		if(node->State=="Sleep"){
			double P=0;
			P=Vcc*I_sleep;
			node->energy=node->energy+P;
			node->State="Sleep";
		}

		//==========================Transmission ���A
		if(node->State=="Transmission"){
			double P=0;

			//P=IntervalPower(Pktsize,Enode->LatestTimeslot-Enode->ExTimeslot);
			
			//cout<<"Interval:"<<(Enode->LatestTimeslot-Enode->ExTimeslot)<<" E:"<<Enode->energy<<endl;
			//Enode->ExTimeslot=Enode->LatestTimeslot;//���������o�e�ɶ�
			P=Vcc*I_notify;
			node->energy=node->energy+P;

			if(node->NodeBuffer->load==0)
				node->State="Sleep";
		}

		//==========================Scan ���A
		if(node->State=="Scan"){
			
		}

	}
}
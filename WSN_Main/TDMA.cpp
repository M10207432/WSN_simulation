#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "WSNFile.h"
#include "WSNStruct.h"
#include "ConnInterval.h"
#include "TDMA.h"

using namespace std;

/*===========================
		WSN�ݾQ
===========================*/
void Topology(){
	Node *TNode=Head->nextnd;
	double distance=100;

	/*---------------------
		��̵u�Z���W��
		�۳s��
	---------------------*/
	while(TNode!=NULL){
		//--------------------------Level 1
		if(TNode->hop==1){
			TNode->SendNode=Head;
		}

		//--------------------------Level 2~
		if(TNode->hop!=1){
			Node *tmp_TNode=Head->nextnd;
			distance=100;
			while(tmp_TNode!=NULL){
				if(tmp_TNode->hop==TNode->hop-1){
					if(distance>sqrt(pow(TNode->coor_x-tmp_TNode->coor_x,2)+pow(TNode->coor_y-tmp_TNode->coor_y,2))){
						distance=sqrt(pow(TNode->coor_x-tmp_TNode->coor_x,2)+pow(TNode->coor_y-tmp_TNode->coor_y,2));
						TNode->SendNode=tmp_TNode;
					}
				}
				tmp_TNode=tmp_TNode->nextnd;
			}
		}

		TNode=TNode->nextnd;
	}

	/*---------------------
		Graph Conflict
	---------------------*/
	//=====================�إ�Edge
	Edge *tmpedge;
	HeadEdge=MainEdge;
	node=Head->nextnd;
	while(node!=NULL){
		tmpedge=new Edge;
		MainEdge->n1=node;
		MainEdge->n2=node->SendNode;
		
		//�إ�Child node
		if(node->SendNode->ChildNode==NULL){
			node->SendNode->ChildNode=node;
			node->SendNode->ChildNode->next_child=NULL;
		}else{
			Node* tmp_child=node->SendNode->ChildNode;
			while(tmp_child->next_child!=NULL){
				tmp_child=tmp_child->next_child;
			}
			tmp_child->next_child=node;
			tmp_child=tmp_child->next_child;
			tmp_child->next_child=NULL;
		}

		MainEdge->next_edge=tmpedge;
		tmpedge->pre_edge=MainEdge;
		MainEdge=tmpedge;

		node=node->nextnd;
	}
	MainEdge->next_edge=NULL;

	//=====================�إ�Conflict Edge
	//ConflictEdge
	Edge *MainConflictEdge=ConflictEdge;
	Node *Childchild_node;
	node=Head->nextnd;
	while(node!=NULL){

		//---------------------------------------------------SendNode
		ConflictEdge->n1=node;
		ConflictEdge->n2=node->SendNode;
		tmpedge=new Edge;
		ConflictEdge->next_edge=tmpedge;
		tmpedge->pre_edge=ConflictEdge;
		ConflictEdge=tmpedge;
		//------SendNode's edge1(Send)
		if(node->SendNode!=Head){
			ConflictEdge->n1=node;
			ConflictEdge->n2=node->SendNode->SendNode;
			tmpedge=new Edge;
			ConflictEdge->next_edge=tmpedge;
			tmpedge->pre_edge=ConflictEdge;
			ConflictEdge=tmpedge;
		}
		//------SendNode's edge2~(Child)
		Childchild_node=node->SendNode->ChildNode;
		if(Childchild_node==node)
				Childchild_node=Childchild_node->next_child;
		while(Childchild_node!=NULL){
			ConflictEdge->n1=node;
			ConflictEdge->n2=Childchild_node;
			tmpedge=new Edge;
			ConflictEdge->next_edge=tmpedge;
			tmpedge->pre_edge=ConflictEdge;
			ConflictEdge=tmpedge;

			Childchild_node=Childchild_node->next_child;
			if(Childchild_node==node)
				Childchild_node=Childchild_node->next_child;
		}

		//---------------------------------------------------ChildNode
		Node* Children=node->ChildNode;
		while(Children!=NULL){
			ConflictEdge->n1=node;
			ConflictEdge->n2=Children;
			tmpedge=new Edge;
			ConflictEdge->next_edge=tmpedge;
			tmpedge->pre_edge=ConflictEdge;
			ConflictEdge=tmpedge;
			//------ChildNode's edge2(Child)
			if(Children!=NULL){
				Childchild_node=Children->ChildNode;
				while(Childchild_node!=NULL){
					ConflictEdge->n1=node;
					ConflictEdge->n2=Childchild_node;
					tmpedge=new Edge;
					ConflictEdge->next_edge=tmpedge;
					tmpedge->pre_edge=ConflictEdge;
					ConflictEdge=tmpedge;

					Childchild_node=Childchild_node->next_child;
				}
			}

			Children=Children->next_child;
		}

		node=node->nextnd;
	}
	ConflictEdge->pre_edge->next_edge=NULL;
	ConflictEdge=MainConflictEdge;
}

/*========================
	�U�۸`�I���C��
========================*/
void NodeColoring(){
	Edge *N_CEdge=ConflictEdge;
	Edge *AssignEdge;
	Node *AssignNode;
	bool Assign_flag=false;
	int colorid=1;

	/*--------------------
		edge�p�� & Init
	--------------------*/
	node=N_CEdge->n1;
	while(N_CEdge!=NULL){
		
		if(node==N_CEdge->n1){
			node->edge=node->edge+1;
		}else{
			node=N_CEdge->n1;
			node->edge=node->edge+1;
		}
			
		N_CEdge=N_CEdge->next_edge;
	}
	
	node=Head->nextnd;
	while(node!=NULL){
		node->order_flag=false;
		node->color=0;
		node=node->nextnd;
	}

	/*--------------------
		���Ncolor assign
		���̤j��edge
	--------------------*/
	while(!Assign_flag){
		//--------------------------------��X�̤jedge��node�A��AssignNode 
		AssignNode=Head->nextnd;
		node=Head->nextnd;
		//����X�٥�assign��node
		while(node!=NULL){
			if(node->order_flag==false){
				AssignNode=node;
				break;
			}
			node=node->nextnd;
		}

		node=Head->nextnd;
		while(node!=NULL){
			if(node->edge > AssignNode->edge && node->order_flag==false){
				AssignNode=node;
			}
			node=node->nextnd;
		}

		//--------------------------------��AssignNode color
		N_CEdge=ConflictEdge;
		while(N_CEdge->n1 !=AssignNode)
			N_CEdge=N_CEdge->next_edge;
		AssignEdge=N_CEdge;				//AssignNode�bConflictEdge�����Ĥ@��node
	
		colorid=1;
		AssignNode->color=colorid;
		node=Head->nextnd;
		bool research_flag=false;
		while(node!=NULL){

			if(node->color == colorid){	//�ۦP�C�⪺
				N_CEdge=AssignEdge;				
				while(N_CEdge!=NULL){				
					if(N_CEdge->n1==AssignNode && N_CEdge->n2==node ){//�I����
						colorid++;
						research_flag=true;	//�Y���ۦP�C��B�I����node�A�h�ݭn���s����node����@��
					}
					N_CEdge=N_CEdge->next_edge;
				}
			}
			if(research_flag){
				node=Head;
				research_flag=false;
			}
			node=node->nextnd;
		}
		AssignNode->order_flag=true;
		AssignNode->color=colorid;

		//--------------------------------�P�_�O�_�����H�j�M����
		Assign_flag=true;
		node=Head->nextnd;
		while(node!=NULL){
			if(node->color==0)
				Assign_flag=false;
			node=node->nextnd;
		}
	}
}

/*========================
	TDMA Schedule
========================*/
void TDMA_Assignment(){
	delete TDMA_Tbl;TDMA_Tbl=NULL;
	TDMA_Tbl=new TDMATable;
	node=Head->nextnd;
	int colorid=1;		//���P��slot time
	TDMATable *tmp_tbl;
	TDMATable *MainTable=TDMA_Tbl;
	Edge *N_CEdge=ConflictEdge;
	int Maxcolor=0;

	//----------------------------------------����X�̤jcolor id
	node=Head->nextnd;
	while(node!=NULL){
		if(node->color > Maxcolor)
			Maxcolor=node->color;
		node=node->nextnd;
	}

	while(Maxcolor>=colorid){

		node=Head->nextnd;
		//-----------------------------------�����t�ۦP�C�⪺node �b�P�@slot�W
		while(node!=NULL){
			if(node->color == colorid){
				//assign pair{slot,node}
				TDMA_Tbl->slot=colorid;	
				TDMA_Tbl->n1=node;		

				tmp_tbl=new TDMATable;
				TDMA_Tbl->next_tbl=tmp_tbl;
				tmp_tbl->pre_tbl=TDMA_Tbl;
				TDMA_Tbl=tmp_tbl;
			}

			node=node->nextnd;
		}
	
		//-----------------------------------�b��slot�W��node�A�P�_�P�䤣�I����node����
		//��������Node-Based �� Level-Based ������
		N_CEdge=ConflictEdge;
		node=Head->nextnd;
		while(node!=NULL){
			if(node->color==colorid){ //�ۦP�C�ⱡ�p�U
				Node *tmpnode=Head->nextnd;
				while(tmpnode!=NULL){ 

					if(node->color != tmpnode->color){ //��colorid��node�P��L���P�C��node
					
						bool conflict_flag=false;
						N_CEdge=ConflictEdge;
						while(N_CEdge!=NULL){
							if(N_CEdge->n1==node && N_CEdge->n2==tmpnode){
								conflict_flag=true;	
							}
							N_CEdge=N_CEdge->next_edge;
						}
						/*
						//�P�_node �P tmpnode�O�_���I��
						if(!conflict_flag){
							//assign pair{slot,node}
							TDMA_Tbl->slot=colorid;	
							TDMA_Tbl->n1=node;	//tmpnode	
							
							tmp_tbl=new TDMATable;
							TDMA_Tbl->next_tbl=tmp_tbl;
							tmp_tbl->pre_tbl=TDMA_Tbl;
							TDMA_Tbl=tmp_tbl;
						}
						*/
					}

					tmpnode=tmpnode->nextnd;
				}
			}

			node=node->nextnd;
		}

		colorid++;
		node=Head->nextnd;
	}

	TDMA_Tbl=TDMA_Tbl->pre_tbl;
	TDMA_Tbl->next_tbl=NULL;

	TDMA_Tbl=MainTable;

}
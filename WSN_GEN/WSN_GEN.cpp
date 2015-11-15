#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include <iostream>
#include<fstream>
#include <string.h>
#include <string>
#define transmission_time 10
#define payload 20

using namespace std;

/*=================================
		Structure
==================================*/
struct Node{
	double coor_x,coor_y;//�y��
	double distanceto_BS;//��Base station �Z��
	double radus;
	short int hop;		//range 1~3

	double period;	// At least period
	double rate;		// bytes/s

	struct Packet* pkt;
	struct Node* nextnd;
	struct Node* prend;
};
struct Packet{
	int id;

	double load;
	double period;
	double utilization;
	double time;

	short int hop;		//range 1~3

	struct Packet* set_nextpkt;
	struct Packet* nextpkt;
	struct Packet* prepkt;
};

/*=================================
		Function
==================================*/
void NodeStruct();	//�إ�node���`�I���c�Ppacket���c

void create();
void create_varied(double);

void NodeLocation();	//���t�`�I��m

/*=================================
		Global value
==================================*/
double period[]={200,500,1000}; //�C�@node��period (ms) (���i�p��200ms)
double periodrange=150;			//period  rand�ɪ��t�Z
const int Level1_Nodenum = 3;		//�Ĥ@�hNode�ƶq<ConnNode>
const int Level2_Nodenum = 0;		//�ĤG�hNode�ƶq<AdvNode>
const int pktnum=4;				//�C��node�W���ʥ]��
const short int Set=100;			//�C�@�Q�Ϊ�Set��
double Initrate=80;					//�}�lGEN��rate
double inv_r=80;							//rate�t�Z
double Maxrate=1000;				//�̲� rate
string GENfile="..\\GENresult\\input_varied\\";//���e�@�ؿ��U��GENresult�ؿ��A����txt��
char Resultfile[]="..\\GENresult\\WSNGEN.txt";//���e�@�ؿ��U��GENresult�ؿ��A����txt��

const short int Max_X_Axis = 100;	//�̤jX�b�d��
const short int Max_Y_Axis = 100;	//�̤jY�b�d��
/*=================================
					Ex setting
==================================*/
const double leastperiod=80;	//period�̤p��
const double largestperiod=5000;//period�̤j��
const double Hyperperiod=10000;	
const float MIN_Uti=1.0;		//GEN �Q�βv���_�I
const float MAX_Uti=5.0;		//GEN �Q�βv�����I
const float U_interval=1;		//�Q�βv���Z
int nodenum=Level1_Nodenum;// without Level2_Nodenum
float Total_Uti=1.0;
const double Max_pktuti=0.9;
double tmp_totaluti=0;
int Gobackcount=0;

Node* HEAD=new Node;			//�`�YHEAD
Node* node=new Node;
Packet* packet=new Packet;

int main(void){
	
	srand(time(NULL));			//�H���ؤl

	for(double rate=Initrate; rate<=Maxrate; rate+=inv_r){
		Total_Uti=rate;

		//��JGEN���ɦW
		string filename="Rate";
		filename.append(to_string(Total_Uti));
		filename.append("_Set");
		filename.append(to_string(Set));
		filename.append(".txt");
		cout<<filename<<endl;

		//��JGEN�� ���|+�ɦW
		string GENBuffer=GENfile;
		GENBuffer.append(filename);

		char *GENbuffer=(char*)GENBuffer.c_str();
	
		fstream fp;
		fp.open(GENbuffer, ios::out);	//�}���ɮ�.�g�J���A
		if(!fp){//�p�G�}���ɮץ��ѡAfp��0�F���\�Afp���D0
			cout<<"Fail to open file: "<<GENbuffer<<endl;
		}

		/*==================================================
						GEN U �� Set��
		==================================================*/
		
		for(int setcount=0;setcount<Set;setcount++){

			/*==================================================
							Create Node & Packet
			==================================================*/
			NodeStruct();
			
			//create();
			create_varied(rate);

			NodeLocation();
			/*==================================================
							�g�J��T TXT
			==================================================*/
			double tmpu=0;
			tmp_totaluti=0;
			Packet* tmppkt=new Packet;
			Node* tmpnode=new Node;
			tmpnode=HEAD->nextnd;

			fp<<Level1_Nodenum<<" "<<Level2_Nodenum<<" "<<pktnum<<" "<<Hyperperiod<<endl;
			while(tmpnode!=NULL){
				tmppkt=tmpnode->pkt;

				fp<<"Node"<<" ";
				fp<<tmpnode->coor_x<<" "<<tmpnode->coor_y<<" "<<tmpnode->radus<<endl;
					
				while(tmppkt!=NULL){
					fp<<"Pkt"<<" ";
					fp<<tmppkt->load<<" ";
					fp<<tmppkt->period<<" ";
					fp<<tmppkt->utilization<<" ";
					fp<<tmpnode->hop<<endl;//fp<<tmppkt->hop<<endl;

					cout<<tmppkt->load<<endl;
					cout<<tmppkt->period<<endl;
					tmpu=tmpu+tmppkt->utilization;
					tmp_totaluti=tmp_totaluti+(tmppkt->load/tmppkt->period);
					
					tmppkt=tmppkt->nextpkt;
				}
				if(tmpnode!=NULL){
					tmpnode=tmpnode->nextnd;
				}
			}

			cout<<"Rate:"<<(tmp_totaluti/nodenum)*1000<<endl;
			fp<<"Rate:"<<(tmp_totaluti/nodenum)*1000<<endl;
			cout<<"=========="<<setcount<<endl;
			fp<<"=========="<<endl;
			fp<<"=========="<<setcount<<endl;
		}//============================================Set end

		fp.close();
		filename.clear();
		
	}
	system("PAUSE");
	return 0;
}

void create(){
	bool Done_flag=false;					//GEN����

	while(Done_flag!=true){
		/*==================================================
						���t�U�ʥ]Period
						�n�ŦXHyperperiod
		==================================================*/
		tmp_totaluti=0;
		node=HEAD->nextnd;
		double tmp_packetperiod=0;//�nrand���ʥ]period

		while(node!=NULL){
			packet=node->pkt;
			while(packet!=NULL){
			
				int periodrange=(largestperiod-leastperiod);	//�nRand���d��
				tmp_packetperiod=rand()%periodrange+leastperiod;							//Rand�Xpacket��period��� largestperiod~leastperiod
				while((int(Hyperperiod)%int(tmp_packetperiod))!=0){
					tmp_packetperiod=rand()%periodrange+leastperiod;						//Rand�Xpacket��period��� largestperiod~leastperiod
				}
				packet->period=double(tmp_packetperiod);										//��J�Q�βv

				packet=packet->nextpkt;
			}
			node=node->nextnd;
		}
		/*==================================================
						���t�U�ʥ]�Q�βv
		==================================================*/
		node=HEAD->nextnd;
		double avg_packetuti=Total_Uti/(nodenum*pktnum);
		double tmp_pktuti=0.0;
		double totalpacket=nodenum*pktnum;
		int rangetoavg_pktuti=nodenum*pktnum*avg_packetuti;	//Gen���d�� 0~avg_packetuti
		int rangetomax_pktuti=nodenum*pktnum*Max_pktuti;	//Gen���d�� 0~Max_pktuti
		double tmpcount=1;
		bool averageuti_flag=false;
	
		while(node!=NULL){
			packet=node->pkt;
			while(packet!=NULL){
			
				/*-------------------
					Rand Uti
				-------------------*/
				if(tmpcount*avg_packetuti < tmp_totaluti){
					if(rangetoavg_pktuti!=0)
						tmp_pktuti=((rand()%rangetoavg_pktuti)+1)/totalpacket;//0~avg_packetuti
					else
						tmp_pktuti=((rand()%1)+1)/totalpacket;
					averageuti_flag=false;
				}
				else{
					averageuti_flag=true;
					tmp_pktuti=((rand()%rangetomax_pktuti)+1)/totalpacket;//0~Max_pktuti
				}

				//�}�ҥ����Q�βv�p��
				if(averageuti_flag==true){
					if(((nodenum*pktnum)-tmpcount)!=0){
						avg_packetuti=(Total_Uti-tmp_totaluti)/((nodenum*pktnum)-tmpcount);
						rangetoavg_pktuti=nodenum*pktnum*avg_packetuti;
					}
				}

				//��Jpacket uti��
				tmpcount++;
				tmp_totaluti=tmp_totaluti+tmp_pktuti;
				packet->utilization=tmp_pktuti;
				packet=packet->nextpkt;
			
			}
			node=node->nextnd;
		}
		/*==================================================
						���������`�Q�βv
		==================================================*/
		//-------------------------------------�Q�βv����
		node=HEAD->nextnd;
		packet=node->pkt;
		if(tmp_totaluti<Total_Uti){
			double remainuti=Total_Uti-tmp_totaluti;
		
			//�ܴ�pkt�Q�βv
			while(packet!=NULL){
				if(packet->utilization+remainuti>1)
					packet=packet->nextpkt;
				else{
					packet->utilization=packet->utilization+remainuti;
					tmp_totaluti=tmp_totaluti+remainuti;
					break;
				}
			}
		}
		//-------------------------------------�Q�βv�W�L
		node=HEAD->nextnd;
		packet=node->pkt;
		if(tmp_totaluti>Total_Uti){
			int adjustnum=0;
			double adjuti=0;
			double remainuti=tmp_totaluti-Total_Uti;
		
			//�p����ܴ���pkt�ƶq
			while(remainuti>=0){
				adjustnum++;
				remainuti=remainuti-packet->utilization;
				tmp_totaluti=tmp_totaluti-packet->utilization;
				packet->utilization=0;
				packet=packet->nextpkt;
				if(packet==NULL){
					node=node->nextnd;
					packet=node->pkt;
				}
			}
			//�p�⥭���Q�βv
			adjuti=(Total_Uti-tmp_totaluti)/(adjustnum);
			node=HEAD->nextnd;
			packet=node->pkt;
			while(packet->utilization==0){
				tmp_totaluti=tmp_totaluti+adjuti;
				packet->utilization=adjuti;
				packet=packet->nextpkt;
				if(packet==NULL){
					node=node->nextnd;
					packet=node->pkt;
				}
			}
		}
		/*==================================================
						�̷�Packet����
					Utilization & Period�h��Xload
				  (Done_flag�|�P�w�O�_��load��0�����p)
		==================================================*/
		Done_flag=true;
		node=HEAD->nextnd;
		while(node!=NULL){
			packet=node->pkt;
			while(packet!=NULL){
				
				packet->time=(packet->utilization * packet->period);
				packet->time=(int(packet->time)/10)*10;
				packet->load=(packet->time/transmission_time)*payload;
				packet->utilization=packet->time/packet->period;

				if(packet->load==0)   
					Done_flag=false;
				if(tmp_totaluti!=Total_Uti)
					Done_flag=false;
				packet=packet->nextpkt;
			}
			node=node->nextnd;
		}
	}

	/*==================================================
					���tAdvNode���s���ʥ]
	==================================================*/
	//===================================================Link list
	int countLevel2=Level2_Nodenum;
	node=HEAD->nextnd;
	while(node->nextnd!=NULL){
		node=node->nextnd;
	}
	while(countLevel2>0){
		Node *nextnode=new Node;
		Node *prenode=node;

		node->nextnd=nextnode;
		node=nextnode;
		node->prend=prenode;
		node->nextnd=NULL;

		//�u�঳�@�ӫʥ], �u�঳20byte(�ǿ�ɶ���10ms) period���]���̤j
		node->pkt=new Packet;
		
		node->hop=2;
		packet=node->pkt;
		packet->period=largestperiod;
		packet->time=transmission_time;
		packet->load=(packet->time/transmission_time)*payload;
		packet->utilization=packet->time/packet->period;

		node->pkt->nextpkt=NULL;
		node->pkt->prepkt=NULL;

		//node count ��@
		countLevel2--;
	}
	
	
}

void create_varied(double rate){

	//==========================================================setting 
	int i=0;
	for(Node* n=HEAD->nextnd; n!=NULL; n=n->nextnd){
		n->period=period[i];
		n->rate=rate/(1000/period[i]);
		i++;
		//=========================================================splite to pkt
		for(Packet* pkt=n->pkt; pkt!=NULL; pkt=pkt->nextpkt){
			pkt->load=ceil(n->rate/pktnum);
				
			double p=0;
			do{
				p=((double(rand()%int(pkt->load))+1)/pkt->load)*n->period;//0~avg pkt->load ===========rand
			}while((n->period-periodrange)>=p);

			//save data
			pkt->load=ceil(pkt->load*(p/n->period));
			pkt->period=p;
			pkt->utilization=pkt->load/pkt->period;
		}
	}
}

void NodeStruct(){
	delete HEAD;delete node;delete packet;
	HEAD=new Node();
	node=new Node();
	packet=new Packet();

	/*==================================================
					�إ�Link list
					Node & Packet
				���tConnNode���s���ʥ]
	==================================================*/
	/*-------------------------
		Gen node(Linklist)
	-------------------------*/
	
	HEAD->nextnd=node;
	node->prend=HEAD;
	for(int n=0;n<nodenum;n++){
		/*-------------------------
			Gen packet(Linklist)
		-------------------------*/
		packet=new Packet;
		node->pkt=packet;
		packet->prepkt=NULL;
		for (int p=0;p<pktnum;p++){
			
			Packet* nextpacket=new Packet;
			Packet* prepacket=packet;
			packet->nextpkt=nextpacket;
			packet->set_nextpkt=nextpacket;
			packet=nextpacket;
			packet->prepkt=prepacket;
			
		}

		packet=packet->prepkt;
		packet->nextpkt=NULL;
		packet->set_nextpkt=NULL;
		//--------------------------Packet Done

		Node* nextnode=new Node;
		Node* prenode=node;
		node->nextnd=nextnode;
		node=nextnode;
		node->prend=prenode;
	}
	node=node->prend;
	node->nextnd=NULL;
	//-----------------------------Node Done

	Packet* tmppkt=NULL;
	for(Node* node=HEAD->nextnd; node!=NULL; node=node->nextnd){
		for(Packet* pkt=node->pkt; pkt!=NULL; pkt=pkt->nextpkt){
			tmppkt=pkt;
		}
		if(node->nextnd!=NULL){
			tmppkt->set_nextpkt=node->nextnd->pkt;
		}
	}

}

void NodeLocation(){
	/*==================================================
					���t�U�`�I��m
	==================================================*/
	node=HEAD->nextnd;
	int tmplevel1=Level1_Nodenum;
	int tmplevel2=Level2_Nodenum;
	double ceter_x=Max_X_Axis/2;
	double ceter_y=Max_Y_Axis/2;
	double R=-1;

	while(node!=NULL){
		if(tmplevel1){
			R=-1;
			while(!(R<=Max_X_Axis/2 && R>0)){
				node->coor_x=(rand()%Max_X_Axis/2)+Max_X_Axis/4; 
				node->coor_y=(rand()%Max_Y_Axis/2)+Max_Y_Axis/4;
				
				R=sqrt(pow((ceter_x-node->coor_x),2)+pow((ceter_y-node->coor_y),2));
			}
			node->radus=R;
			node->hop=1;

			tmplevel1--;
		}else if(tmplevel2){
			R=-1;
			while(!(R>Max_X_Axis/2)){
				node->coor_x=(rand()%Max_X_Axis);
				node->coor_y=(rand()%Max_Y_Axis);

				R=sqrt(pow((ceter_x-node->coor_x),2)+pow((ceter_y-node->coor_y),2));
			}
			node->radus=R;
			node->hop=2;

			tmplevel2--;
		}

		cout<<"Node Address: "<<node->coor_x<<" "<<node->coor_y<<endl;
		node=node->nextnd;
	}
}
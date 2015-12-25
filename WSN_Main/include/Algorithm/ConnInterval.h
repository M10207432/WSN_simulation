#ifndef CONNINTERVAL_H
#define CONNINTERVAL_H

extern double Connectioninterval;
extern double DIFMinperiod;
extern short int Rateproposal;
extern double timeslot;
extern double Minumum_interval;

class EventInterval{
	public:
		EventInterval();
		
		void ServiceInterval_Algorithm(int);		//��Single node��k
		void ConnectionInterval_Algorithm(int);		//��Muliple node��k

		/*------------------------------
			Service interval�p��
			(Single node)
		------------------------------*/
		void Event();						//�C�@node connection interval ���� 10ms
		void MEI(Node *);					//��Demand bound�p�� service interval
		void DIF();							//�ΦU��pkt�϶��p��Pload�p��rate��pkt�A�|�A�ഫ��service interval
		void Greedy();						//�γ̵uminimum period��@service interval

		/*------------------------------
			Connection interval�p��
			(Muliple nodes network)
		------------------------------*/
		void EIMA();						//��avg current�@��weight�A���tconnection interval
		void LDC();							//�U��service interval���Wnode1level�@��weight�A���tconnection interval
		void IntervalDivide();				//��minimum service interval���Wnode1level�@��weight�A���tconnection interval

		/*------------------------------
				Others
		------------------------------*/
		void Rate_TO_Interval(int );
		void ConnectionPriority();
		void IntervalReassign();
};

#endif
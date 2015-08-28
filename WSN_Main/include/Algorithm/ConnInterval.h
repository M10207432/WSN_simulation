#ifndef CONNINTERVAL_H
#define CONNINTERVAL_H

extern double Connectioninterval;
extern double DIFMinperiod;
extern short int Rateproposal;

class EventInterval{
	public:
		EventInterval();
		void Event();
		void TSB();
		void DIF();
		void Rate_TO_Interval(int );
		void Algorithm(int);

		void EIMA(Node*, TDMATable*);
		void ConnectionPriority();
};

#endif
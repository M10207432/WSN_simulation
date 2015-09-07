#ifndef CONNINTERVAL_H
#define CONNINTERVAL_H

extern double Connectioninterval;
extern double DIFMinperiod;
extern short int Rateproposal;
extern double timeslot;

class EventInterval{
	public:
		EventInterval();
		void Event();
		void TSB();
		void DIF();
		void Rate_TO_Interval(int );
		void Algorithm(int);
		void Interval_TDMA_Algorithm(int);

		void EIMA();
		void IntervalDivide();

		void ConnectionPriority();
};

#endif
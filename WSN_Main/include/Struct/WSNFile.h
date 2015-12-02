using namespace std;

extern fstream GENfile;
extern fstream Schdulefile;
extern fstream Powerfile;
extern fstream Resultfile;
extern string filename;
extern double Meetcount;
extern double AverageE;
extern short int readsetting;

void CreateFile(float ,int,char *);
void SaveFile(short int);
void SaveSet(float, int);
void CloseFinal();

void PathSetting(string);
void ExperimentSetting(short int*, short int*, short int*);
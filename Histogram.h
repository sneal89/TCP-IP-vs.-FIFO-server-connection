#ifndef Histogram_h
#define Histogram_h

#include <queue>
#include <string>
#include <vector>
#include <mutex>
using namespace std;

class Histogram {
private:
	vector<int> hist;
	int nbins;
	double start, end;
	mutex mute;
public:
    Histogram(int, double, double); //max and min for start and end
	~Histogram();
	void update (double ); 		// updates the histogram
    vector<int> get_hist();		// returns the histogram
    int size ();
	vector<double> get_range ();
};

#endif 
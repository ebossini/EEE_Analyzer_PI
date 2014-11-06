#ifndef BOSS__E3EVENT__LIBGUARD
#define BOSS__E3EVENT__LIBGUARD



class E3Event
{

private:

	std::vector<std::vector<UInt_32b> > _rawData;

	//basic event info
	UInt_32b	_orbit;
	UInt_32b	_sec;
	UInt_32b	_bunch;
	UInt_32b	_evtNum;
	UInt_32b	_trgNum;
	UInt_32b	_calib;

	UInt_32b	_ninoMap;

	//junk code start
	//junk code end

public:

	//junk code start	
	
	float xt,yt,zt,r,rho,th,ph,xr,yr,cosx,cosy,cosz;
	int trkNum;
	long mean_time[24][8];
	int num_hit[24][8];
	float time_diff[24][3];
	int coinc[24][3];
	int multiplicity[3];
	float Xcoord[100];
	float Ycoord[100];
	float Zcoord[100];   
	int g_multiplicity;
	void compute_time(int plus, int minus, int plane);
	//junk code end

	E3Event(void);
	~E3Event(void);

	inline void setRawData(std::vector<std::vector<UInt_32b> > rawDataVector) {_rawData=rawDataVector;};
	
	// retrive general event infos
	inline UInt_32b getEvtNum()		{return _evtNum;}
	inline UInt_32b getEvtSec()		{return _sec;}
	inline UInt_32b getEvtOrbit()	{return _orbit;}
	inline UInt_32b getEvtBunch()	{return _bunch;}
	inline UInt_32b getEvtCal()		{return _calib;}
	inline UInt_32b getTrgNum()		{return _trgNum;}


	inline void		setNinoMap(UInt_32b NinoMap)		{_ninoMap=NinoMap;}

	UInt_16b analyzeEvent();



};

#endif
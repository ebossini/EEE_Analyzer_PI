#include "stdafx.h"
#include "E3Run.h"


E3Run::E3Run(void):_gps(),_gHitMult(3,std::vector< UInt_32b>(1000,0)),_gSumHitMult(3000,0),
		_gClusterMult(3,std::vector< UInt_32b>(1000,0)),_gSumClusterMult(3000,0)
{
	
	_gLowHitMult=0;
	_gMediumHitMult=0;
	_gHighHitMult=0;
	_gLowClusterMult=0;
	_gMediumClusterMult=0;
	_gHighClusterMult=0;
}


E3Run::~E3Run(void)
{
}

void E3Run::analyzeRun(std::string Source,std::string OutDir)
{
	_sourceStream.open(Source.c_str(),std::ios::binary);

	//get file size
    _sourceStream.seekg (0, _sourceStream.end);
	UInt_32b FileLength = _sourceStream.tellg();
    _sourceStream.seekg (0, _sourceStream.beg);
	


	// header parsing (contains gps inforamtion)

	t_gps EEE_gps;
	E3Track bestTrack;

	do
	{
		_sourceStream.read((char*)&_hH,			sizeof(_hH)				);  //fix for standard EEE header
		//std::cout<< "Found "<<_hH<<std::endl;
	} while (_hH!=0xfbfbfbfb && (_sourceStream.tellg()<FileLength));

	_sourceStream.read((char*)&_hVersion,	sizeof(_hVersion)		);
	_sourceStream.read((char*)&_hMachineID,	sizeof(_hMachineID)		);
	_sourceStream.read((char*)&_hRunNumber,	sizeof(_hRunNumber)		);
	_sourceStream.read((char*)&_hRunNameL,	sizeof(_hRunNameL)		);
	_sourceStream.read((char*)&_hRunName,	_hRunNameL				);
	_sourceStream.read((char*)&_hTrgMask,	sizeof(_hTrgMask)		);
	_sourceStream.read((char*)&_hNinoMap,	sizeof(_hNinoMap)		);
	_sourceStream.read((char*)&_hStartTime,	sizeof(_hStartTime)		);
	_sourceStream.read((char*)&EEE_gps,		sizeof(t_gps)			);
	_sourceStream.read((char*)&_hT,			sizeof(_hT)				);

	//set event gps timestamp
	_event.setGpsTimestamp(getGpsE3Timestamp());

	//print Header Info

	writeHeaderInfo(std::cout);
	
	//set and print Gps info
	setGpsStruct(EEE_gps);
	writeGpsInfo(std::cout);

	//generate output files
	if(!createOutFile(OutDir)) return;

	//write file header
	_timFile << "  run_number,event_number,event_timesecs since 1.1.2007,nanosec"<<std::endl;
	_outFile << "  run_number,event_number,secs_since_1.1.2007,nanosec,microseconds_since_start_of_runtrack:"
		<<" unit_vector_x,unit_vector_y,unit_vector_z,chi_squared,time of flight[ns],track length[cm]"<<std::endl;
	_2ttFile << "  run_number,event_number,secs_since_1.1.2007,nanosecs,microseconds_since_start_of_runtrack1:"
		<<" unit_vector_x,unit_vector_y,unit_vector_z,chi_squared,time of flight[ns],track length[cm],track2:"
		<<" unit_vector_x,unit_vector_y,unit_vector_z,chi_squared,time of flight[ns],track length[cm]"<<std::endl;

	//start event loop

	_event.setNinoMap(_hNinoMap);
	_analyzed=0;
	int GoodEvent=0;
	int trackfound=0;
	while (_sourceStream.tellg()<FileLength)
	{
		_event.clear();
		if(getEvent()==1)
		{
			std::cout<<"Event stream: unexpected fail to read file"<<std::endl;
			break;
		}
		
		if(_event.unpack()==0)
		{
			if(_event.reconstruct()==0)
			{
				_analyzed++;
				bestTrack=_event.bestTrack();
				getMultiplicity();
				if(_event.numTracks())
				{
					trackfound++;
					if(bestTrack.chisquare()<10)	GoodEvent++;

				}
				
				//write event
				writeEventTim(_timFile);
				writeEventOut(_outFile);
			}
		}
	}
		_sourceStream.close();
		/*std::cout<<GoodEvent<<std::endl;
		std::cout<<trackfound<<std::endl;
		std::cout<<_analyzed<<std::endl;*/
		writeRunSum(_sumFile);

		_timFile.close();
		_sumFile.close();
		_outFile.close();
		_2ttFile.close();

}
void E3Run::getMultiplicity()
{
	for (UInt_16b ChamberIdx=0;ChamberIdx<3;ChamberIdx++)
	{
		_gHitMult.at(ChamberIdx).at(_event.numHits(ChamberIdx))++;          //chamber hit multiplicity
		_gClusterMult.at(ChamberIdx).at(_event.numClusters(ChamberIdx))++;	//chamber cluster multiplicity
	}
	if (_event.numHits(0)==0 || _event.numHits(1)==0 || _event.numHits(2)==0) _gLowHitMult++; //at least 1 chamber with no hits
	if (_event.numClusters(0)==1 && _event.numClusters(1)==1 && _event.numClusters(2)==1) _gLowClusterMult++;  //1 cluster in each chamber
	
	if ((_event.numHits(0)==1 || _event.numHits(0)==2)  &&  
		(_event.numHits(1)==1 || _event.numHits(1)==2)  &&
		(_event.numHits(2)==1 || _event.numHits(2)==2)) _gMediumHitMult++;   //events with 1/2 hits in each chamber
	if ((_event.numClusters(0)==2)  &&  
		(_event.numClusters(1)==2)  &&
		(_event.numClusters(2)==2)) _gMediumClusterMult++; //events with 2 clusters in each chamber
	
	if (_event.numHits(0)>2 || _event.numHits(1)>2 || _event.numHits(2)>2) _gHighHitMult++;			//at least one chambers with more than 2 hits
	if (_event.numClusters(0)>2 || _event.numClusters(1)>2 || _event.numClusters(2)>2) _gHighClusterMult++;//at least one chambers with more than 2 cluster

	_gSumHitMult.at(_event.numHits())++;
	_gSumClusterMult.at(_event.numClusters())++;
}

UInt_16b E3Run::getEvent()
{

	UInt_16b nword[2];
	UInt_64b time;
	UInt_32b word;
	
	_sourceStream.read((char*)&time,sizeof(time));
	_sourceStream.read((char*)nword,sizeof(nword[0])*2);
		
	for (UInt_16b i = 0; i < nword[0] + nword[1]; i++) 
	{
		_sourceStream.read((char*)&word, sizeof(word));
		_event.addRawData(word);
	}

	if (_sourceStream.fail()) return 1;

	return 0;
}




std::ostream& E3Run::writeHeaderInfo(std::ostream& os)
{
	os<<"****************** HEADER INFO *****************"<<std::endl<<std::endl;

	os 	<< "Run Name	= " <<	_hRunName<<std::endl
		<< "Run Number	= " <<	_hRunNumber<<std::endl
		<< "Machine ID	= " <<	(UInt_16b)_hMachineID <<std::endl
		<< "DAQ Version	= " <<	_hVersion<<std::endl
		<< "DAQ Start	= " <<	_hStartTime <<std::endl
		<< "Trg Mask	= " <<std::hex<<	(UInt_16b)_hTrgMask<<std::endl
		<< "Nino Map	= " <<std::hex<<	_hNinoMap<<std::endl<<std::endl;
	return os;
}

bool E3Run::createOutFile(std::string OutDir)
{
	
	std::string fileName;

	fileName=OutDir;
	fileName.append(_hRunName);
	fileName.append(".tim");	
	_timFile.open(fileName.c_str());

	fileName.clear();
	fileName=OutDir;
	fileName.append(_hRunName);
	fileName.append(".out");	
	_outFile.open(fileName.c_str());

	fileName.clear();
	fileName=OutDir;
	fileName.append(_hRunName);
	fileName.append(".2tt");	
	_2ttFile.open(fileName.c_str());

	fileName.clear();
	fileName=OutDir;
	fileName.append(_hRunName);
	fileName.append(".sum");
	_sumFile.open(fileName.c_str());

	if (_timFile.good() && _2ttFile.good() && _sumFile.good() && _outFile.good()) return true;
	else
	{
		std::cout<<"Unable to create output file"<<std::endl;
		return false;
	}
}

std::ostream& E3Run::writeEventTim(std::ostream& os)
{
	//run number from header
	os.width(6);
	os << std::dec<<std::right<<_hRunNumber;

	//event number(corrected for 1 event offset)
	os.width(11);		
	os << _event.getEvtNum()-1;

	//seconds from 1.1.2007
	os.width(11);
	os <<_event.getEvtSec()+getGpsE3Timestamp();
	os.width(11);

	//nanosecond from last PPS
	long clkNum=_event.getEvtBunch()+_event.getEvtOrbit()*4000;
	double nsec=1.0/(double)(_event.getEvtCal())*(double)clkNum/1e-9;
	os <<std::right<<(long)nsec<<std::endl;
		
	return os;
}

std::ostream& E3Run::writeEventOut(std::ostream& os)
{


	//run number from header
	os.width(6);
	os << std::fixed<<std::dec<<std::right<<_hRunNumber;

	//event number(corrected for 1 event offset)
	os.width(11);		
	os << _event.getEvtNum()-1;


	E3Track bestTrack;
	if (!_event.numTracks())  // if no tracks are reconstructed,write it and return
	{
		os.width(11);
		os <<"no hits"<<std::endl;
		return os;
	}
	else bestTrack=_event.bestTrack();


	//seconds from 1.1.2007
	os.width(11);
	os <<_event.getEvtSec()+getGpsE3Timestamp();

	//nanosecond from last PPS
	os.width(11);
	long clkNum=_event.getEvtBunch()+_event.getEvtOrbit()*4000;
	double nsec=1.0/(double)(_event.getEvtCal())*(double)clkNum/1e-9;
	os <<std::right<<(long)nsec;

	//micro from run start
	os.width(13);
	long micro=nsec/1000+_event.getEvtSec()*1000000;
	os <<std::right<<(long)micro;

	//cosine direction
	os.width(10);	
	os << std::setprecision(5)<<bestTrack.xdir();
	os.width(10);		
	os << bestTrack.ydir();
	os.width(10);		
	os << bestTrack.zdir();

	//chi2
	os.width(9);	
	int aux=0;
	os << std::setprecision(3)<<bestTrack.chisquare();

	//tof
	os.width(11);	
	os <<std::setprecision(3)<< bestTrack.timeOfFlight();

	//track L
	os.width(11);	
	os <<std::setprecision(2)<< (double)bestTrack.length()<<std::endl;
		
	return os;
}

std::ostream& E3Run::writeRunSum(std::ostream& os)
{
	os<<" Analyzed events =";
	os.width(13);
	os << std::dec<<std::right<<_analyzed<<std::endl;
	
	os<<"  GPS events";
	os.width(12);
	int aux=0;
	os << std::dec<<std::right<<aux<<std::endl;
	
	//multi hit section
	
	os<<" ****** Hit analysis ***************"<<std::endl;
	 os<<"Events with no hits in a chamber =";
	os.width(13);
	os << std::dec<<std::right<<_gLowHitMult<<std::endl;
	
	 os<<" Events with 1 or 2 hits/chamber =";
	os.width(13);
	os << std::dec<<std::right<<_gMediumHitMult<<std::endl;
	
	 os<<" Event with more than 2 hits in a chamber =";
	os.width(13);
	os << std::dec<<std::right<<_gHighHitMult<<std::endl;
	
	os<<" Hits multiplicity chamber BOTTOM"<<std::endl;
	
	for (int muxIdx = 0; muxIdx < _gHitMult.at(2).size(); muxIdx++)
		if (_gHitMult.at(2).at(muxIdx) > 0)
		{
			os.width(12);
			os <<std::right<<muxIdx;
			os.width(12);
			os <<std::right<<_gHitMult.at(2).at(muxIdx)<<std::endl;
		}
	
	os<<" Hits multiplicity chamber MIDDLE"<<std::endl;
	
	for (int muxIdx = 0; muxIdx < _gHitMult.at(1).size(); muxIdx++)
		if (_gHitMult.at(1).at(muxIdx) > 0)
		{
			os.width(12);
			os <<std::right<<muxIdx;
			os.width(12);
			os <<std::right<<_gHitMult.at(1).at(muxIdx)<<std::endl;
		}
	
	os<<" Hits multiplicity chamber UP"<<std::endl;
	
	for (int muxIdx = 0; muxIdx < _gHitMult.at(0).size(); muxIdx++)
		if (_gHitMult.at(0).at(muxIdx) > 0)
		{
			os.width(12);
			os <<std::right<<muxIdx;
			os.width(12);
			os <<std::right<<_gHitMult.at(0).at(muxIdx)<<std::endl;
		}
	
	os<<" Hits total multiplicity"<<std::endl;
	
	for (int muxIdx = 0; muxIdx < _gSumHitMult.size(); muxIdx++)
		if (_gSumHitMult.at(muxIdx) > 0)
		{
			os.width(12);
			os <<std::right<<muxIdx;
			os.width(12);
			os <<std::right<<_gSumHitMult.at(muxIdx)<<std::endl;
		}
//cluster info (to be adjusted)
		
	os<<" ******** Cluster analysis ************"<<std::endl;
	os<<" Events with 1 cluster in each chamber =";
	os.width(13);
	os << std::dec<<std::right<<_gLowClusterMult<<std::endl;
	
	 os<<" Events with >=2 clusters in a chamber =";
	os.width(13);
	os << std::dec<<std::right<<_gHighClusterMult<<std::endl;
	
	 os<<" Events with 2 clusters  in each chamber =";
	os.width(13);
	os << std::dec<<std::right<<_gMediumClusterMult<<std::endl;

	os<<" Cluster multiplicity chamber BOTTOM"<<std::endl;

	for (int muxIdx = 0; muxIdx < _gClusterMult.at(2).size(); muxIdx++)
		if (_gClusterMult.at(2).at(muxIdx) > 0)
		{
			os.width(12);
			os <<std::right<<muxIdx;
			os.width(12);
			os <<std::right<<_gClusterMult.at(2).at(muxIdx)<<std::endl;
		}

	os<<" Cluster multiplicity chamber MIDDLE"<<std::endl;

	for (int muxIdx = 0; muxIdx < _gClusterMult.at(1).size(); muxIdx++)
		if (_gClusterMult.at(1).at(muxIdx) > 0)
		{
			os.width(12);
			os <<std::right<<muxIdx;
			os.width(12);
			os <<std::right<<_gClusterMult.at(1).at(muxIdx)<<std::endl;
		}

	os<<" Cluster multiplicity chamber UP"<<std::endl;

	for (int muxIdx = 0; muxIdx < _gClusterMult.at(0).size(); muxIdx++)
		if (_gClusterMult.at(0).at(muxIdx) > 0)
		{
			os.width(12);
			os <<std::right<<muxIdx;
			os.width(12);
			os <<std::right<<_gClusterMult.at(0).at(muxIdx)<<std::endl;
		}

	os<<" Cluster total multiplicity"<<std::endl;

	for (int muxIdx = 0; muxIdx < _gSumClusterMult.size(); muxIdx++)
		if (_gSumClusterMult.at(muxIdx) > 0)
		{
			os.width(12);
			os <<std::right<<muxIdx;
			os.width(12);
			os <<std::right<<_gSumClusterMult.at(muxIdx)<<std::endl;
		}

		//other
		 os<< "time cuts"<<std::endl;
			 
	return os;		
}
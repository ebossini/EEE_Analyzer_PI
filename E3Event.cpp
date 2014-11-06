#include "stdafx.h"
#include "E3Event.h"




E3Event::E3Event(void):_rawData(0,std::vector<UInt_32b>(0,0))
{
}


E3Event::~E3Event(void)
{
}

UInt_16b E3Event::analyzeEvent()
{
	//retrive basic info (Assume board0-board1 data consistency within daq limits.DAQ took care to check it!!)

	_evtNum	=	_rawData[0][1];
	_trgNum	=	_rawData[0][2];
	_sec	=	_rawData[0][3];
	_orbit	=	(_rawData[0][4] >> 16) & 0xffff;
	_bunch	=	_rawData[0][4] & 0xffff;
	_calib	=	_rawData[0][5];


	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!begin DAQ code!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	trkNum=0;

		for (int j=0;j<8;j++) for (int i=0;i<=24;i++) 
	{
		mean_time[i][j]=0;	
		num_hit[i][j]=0;
	}
	for (int j=0;j<3;j++) 	multiplicity[j]=0;
	g_multiplicity=0;

	for (UInt_16b boardIdx=0; boardIdx<=1; boardIdx++)  //loop sulle board  //was i
	{
		std::vector<UInt_32b>::iterator BoardIt = _rawData.at(boardIdx).begin();
		if(*BoardIt!=0xbbbbbbbb)
		{
			std::cout<<"Warnig: No header found in event "<< _evtNum <<" board "<<boardIdx<<". Event analysis skipped" << std::endl;
			return 1;
		}
		
		UInt_32b BeginEvent=0;
		UInt_32b NINO_id=0xf;
		UInt_32b EndEvent=0;
		UInt_32b compare=0;
		
		BoardIt+=6;      //iteretor set to first tdc header

		for (UInt_16b TDCIdx=0;TDCIdx<=3;TDCIdx++)  //loop sui 4 tdc //was j
		{
			NINO_id=(_ninoMap>>(16*boardIdx+4*TDCIdx))&0xf;
			if (*BoardIt != (0x00ebebeb + (0x01000000*TDCIdx))) //cerco l'inizio del TDC
			{
				std::cout<<"Warnig: No TDC "<<TDCIdx<<" header found in event "<< _evtNum <<" board "<<boardIdx<<". Event analysis skipped" << std::endl;

				return 2;
			}
			BoardIt++;

			if (((*BoardIt)  & 0xf0000000)!=0x20000000)  //detection of TDC header
			{
				std::cout<<"Warnig: No TDC "<<TDCIdx<<"internal header found in event "<< _evtNum <<" board "<<boardIdx<<". Event analysis skipped" << std::endl;
				return 3;
			}
			BoardIt++;
			//printf("DEBUG:	found Begin event in position for tdc %d board %d\n",index/2,j,i);
				
			while (((*BoardIt) & 0xf0000000)!=0x30000000) //loop fino alla fine dell'evento del TDC
			{
				if (BoardIt == (_rawData.at(boardIdx).end()-1)) //cerco l'inizio del TDC
				{
					std::cout<<"Warnig: No TDC "<<TDCIdx<<" internal trailer found in event "<< _evtNum <<" board "<<boardIdx<<". Event analysis skipped" << std::endl;
					return 4;
				}
				//if ((data_rebuild[i][index]  & 0xf0000000)==0x30000000)  //detection of TDC trailer
					//TDC_Trail_found=1;
				if (((*BoardIt)  & 0xf0000000)==0x40000000)  //detection of leading word
				{
					//determinazione del canale
					UInt_32b channel = ((*BoardIt)>>19) & 0x1f;
					//SWAP dei canali a gruppi di 3 //per la board centrale!!!!! no per tutte
					if (channel<8) channel=7-channel;
					else if (channel<16) channel=23-channel;
					else if (channel<24) channel=39-channel;
					//estrazione del tempo
					UInt_32b rec_time = (*BoardIt) & 0x7ffff;		
					//std::cout<<channel<<"    "<<rec_time<<std::endl;
					num_hit[channel][NINO_id]++;
					mean_time[channel][NINO_id]=(long)((float)((mean_time[channel][NINO_id]*(num_hit[channel][NINO_id]-1))+(rec_time))/(float)(num_hit[channel][NINO_id]));	//media sui tempi di rilevazione			
				}
				BoardIt++;
			}
			BoardIt++;
			if(*BoardIt != (0x00eeee00 + (0x01000000*TDCIdx)))
			{
				std::cout<<"Warnig: No TDC "<<TDCIdx<<" trailer found in event "<< _evtNum <<" board "<<boardIdx<<". Event analysis skipped" << std::endl;
				return 3;
			}
		BoardIt++;
			
		}
	}

			
// ricerca delle coincidenze e calcolo della differenza dei tempi
	compute_time(0,1,0); // top chamber
	compute_time(2,3,1); // middle chamber
	compute_time(4,5,2); // bottom chamber
	//for (i=0;i<3;i++) hfill(103+i,(float)the_event.multiplicity[i],0.,1.);
	Zcoord[g_multiplicity]=-1.;
	Ycoord[g_multiplicity]=-1.;																		
	Xcoord[g_multiplicity]=-1.;

//printf("%f %f %f\n",multiplicity[0],multiplicity[1],multiplicity[2]);		
	// calcolo angolo Theta
	if(multiplicity[0]==1 && multiplicity[1]==1 && multiplicity[2]==1)
	{
		xt=(Xcoord[2]-Xcoord[0])*3.875; // 93cm / 24 strips
		yt=(Ycoord[2]-Ycoord[0])*9.5; // 30/sqrt(4) cm/ns (factor 2 for dual traveling)
		zt= Zcoord[0]-Zcoord[2];
		rho=sqrt(xt*xt+yt*yt);
		r=sqrt(rho*rho+zt*zt);
		ph=atan2(yt,xt)*57.3;
		if(ph<0)ph+=360.;
		//hfill(201,ph,0.,1.);
		th=atan(rho/sqrt(zt*zt))*57.3;

		
		cosx=xt/r;
		cosy=yt/r;
		cosz=zt/r;
		//hfill(200,th,0.,1.);
//printf("xt,yt,zt,rho,r,th,ph,cosx,cosy,cosz=%f %f %f %f %f %f %f %f %f %f\n",xt,yt,zt,rho,r,th,ph,cosx,cosy,cosz);
		trkNum=1;
		xr= Xcoord[2]*5. +  (Xcoord[0]*5.- Xcoord[2]*5.)/ (Zcoord[0]-Zcoord[2]) *(Zcoord[1]-Zcoord[2])-Xcoord[1]*5.;
		yr= Ycoord[2]*15. + (Ycoord[0]*15.-Ycoord[2]*15.)/(Zcoord[0]-Zcoord[2]) *(Zcoord[1]-Zcoord[2])-Ycoord[1]*15.;
		//hfill(202,xr,0.,1.);
		//hfill(203,yr,0.,1.);
	}
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!end DAQ code!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


	return 0;
}

void E3Event::compute_time(int plus, int minus, int plane)
{
	int i;	
	for (i=0;i<24;i++)
	{
		if (num_hit[i][plus]>0 && num_hit[i][minus]>0)  
		{
			time_diff[i][plane]= (mean_time[i][plus] - mean_time[i][minus])*96.;
			if (plane==1) time_diff[i][plane]+= 4250;																		
			 //printf("plane %d channel %d time %f ps\n",plane,i,time_diff[i][plane]);
			coinc[i][plane]=3;
			double planeDist[3]={120,80,40};
			Zcoord[g_multiplicity]=planeDist[plane];
			Ycoord[g_multiplicity]=time_diff[i][plane]/1000;																		
			Xcoord[g_multiplicity]=(float)i;																		

			multiplicity[plane]++;
			if (g_multiplicity<99) g_multiplicity++;
			//hfill(100+plane,(float)i,0.,1.);
		}
		else if (num_hit[i][plus]==0 && num_hit[i][minus]>0) 
		{
			time_diff[i][plane] = 0;
			coinc[i][plane]=2;
		}
		else if (num_hit[i][plus]>0 && num_hit[i][minus]==0) 
		{
			time_diff[i][plane] = 0;
			coinc[i][plane]=1;
		}
		else if (num_hit[i][plus]==0 &&num_hit[i][minus]==0) 
		{
			time_diff[i][plane] = 0;
			coinc[i][plane]=0;
		}
	}
}
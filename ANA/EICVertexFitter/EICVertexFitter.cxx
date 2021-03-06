#include <limits>
#include <cmath>
#include <algorithm>
#include <fstream>
#include <string>
#include <iostream>
#include "EICVertexFitter.h"

ClassImp(EICVertexFitter)

//=========================================================================================  
EICVertexFitter::EICVertexFitter() : TNamed("EICVertexFitter", "EICVertexFitter")
{
  // -- default constructor
    mntracks = 0; 
    mseedx   = 0;
    mseedy   = 0;
    mseedz   = 0;
    mtolx    = 0.1;
    mtoly    = 0.1;
    mtolz    = 0.1;
    mflag    = 0;
}
//=========================================================================================  
EICVertexFitter::EICVertexFitter(const Char_t *name) : TNamed(name, name)
{
  // -- constructor
    mntracks = 0;
    mseedx   = 0;
    mseedy   = 0;
    mseedz   = 0;
    mtolx    = 0.1;
    mtoly    = 0.1;
    mtolz    = 0.1;
    mflag    = 0;
}
//=========================================================================================  
EICVertexFitter::~EICVertexFitter() 
{ 
  // destructor
    mpx.clear();mpy.clear();mpz.clear();
    mvx.clear();mvy.clear();mvz.clear();
    mnerrvx.clear();mnerrvy.clear();mnerrvz.clear();
}
void EICVertexFitter::init(int i){
    mfit = i;
    mStartZ=0;
    mpx.clear();mpy.clear();mpz.clear();
    mvx.clear();mvy.clear();mvz.clear();
    mnerrvx.clear();mnerrvy.clear();mnerrvz.clear();
}
//=========================================================================================
void EICVertexFitter::addParticle(TVector3 mom, TVector3 pos)
{
    mpx.push_back(mom.X());
    mpy.push_back(mom.Y());
    mpz.push_back(mom.Z());
    mvx.push_back(pos.X());
    mvy.push_back(pos.Y());
    mvz.push_back(pos.Z());
    mnvx.push_back(pos.X());
    mnvy.push_back(pos.Y());
    mnvz.push_back(pos.Z());
    mnerrvx.push_back(1.);
    mnerrvy.push_back(1.);
    mnerrvz.push_back(1.);
    mntracks++;
}
//=========================================================================================  
void EICVertexFitter::addParticle(TVector3 mom, TVector3 pos, double ER1, double ER2)
{
    mpx.push_back(mom.X());
    mpy.push_back(mom.Y());
    mpz.push_back(mom.Z());
    mvx.push_back(pos.X());
    mvy.push_back(pos.Y());
    mvz.push_back(pos.Z());
    mnvx.push_back(pos.X());
    mnvy.push_back(pos.Y());
    mnvz.push_back(pos.Z());
    mnerrvx.push_back(sqrt(ER1));
    mnerrvy.push_back(sqrt(ER1));
    mnerrvz.push_back(ER2);
    mntracks++;
}
//=========================================================================================  
void EICVertexFitter::doFit(int ifit)
{
    //i==0 3D fit
    //i==1 2D XY fit
    //i==2 1D Z fit
    if(mntracks>1){ //Can't fit with less than 2 tracks
	if(DEBUG){
	    cout <<"--->Now in vertex fitter<---" << endl;
	    cout <<" Fitting with " << mntracks <<" particles " << endl;
	}
	init2Seed(ifit);
	TMinuit *gMinuit = new TMinuit(3);  //initialize TMinuit with a maximum of 3 params
	if(!VERBOSE)gMinuit->SetPrintLevel(-1);
	if(ifit==0)gMinuit->SetFCN(fcn);    
	else if(ifit==1)gMinuit->SetFCN(fcnxy);
	else if(ifit==2)gMinuit->SetFCN(fcnz);
	Double_t arglist[10];
	Int_t ierflg = 0; 
	arglist[0] = 1;
	gMinuit->mnexcm("SET ERR", arglist ,1,ierflg);
	// ready to do fit
	arglist[0] = 500;
	arglist[1] = 1.;
	double xx=100;double yy=100;double zz=100;
	int cnt=0;
	int fin=0;

	static Double_t vstart[3] = {0,0,0};
	static Double_t step[3] = {0.01,0.01,0.01};    
	if(ifit==2)vstart[0]=mStartZ;
	do{
	    // See if current fit is good; fin is a catch the can be set to 1 to just break loop
	    if(isGoodFit(xx,yy,zz,ifit) || fin==1){
		mflag=1;break;//Break itterative fit if within tolerance
	    }
	    // See if at max iterations; re-set to vertex seed and do one more
	    if(cnt == _MAX_){
		init2Seed(ifit);fin = 1;
	    }
	    gMinuit->mnparm(0, "a1", vstart[0], step[0], 0,0,ierflg);
	    if(ifit==0||ifit==1)gMinuit->mnparm(1, "a2", vstart[1], step[1], 0,0,ierflg);
	    if(ifit==0)         gMinuit->mnparm(2, "a3", vstart[2], step[2], 0,0,ierflg);
	    gMinuit->mnexcm("MIGRAD",arglist,1,ierflg);
	    if(DEBUG){
		Double_t amin,edm,errdef;Int_t nvpar,nparx,icstat;
		gMinuit->mnstat(amin,edm,errdef,nvpar,nparx,icstat);
	    }
	    double vals[3];double errs[3];
	    for(int i = 0; i<3; i++)gMinuit->GetParameter(i,vals[i],errs[i]);
	    // ==== Set pVtx to fit results
	    if(ifit==0)mPrimVtx.SetXYZ(vals[0],vals[1],vals[2]);
	    else if(ifit==1){
		mPrimVtx.SetX(vals[0]);
		mPrimVtx.SetY(vals[1]);
	    }
	    else if(ifit==2)mPrimVtx.SetZ(vals[0]);
	    // If iterating, initiate to fit vertex for next round
	    init2Vtx(ifit);
	    if(mfit==1)fin=1;//Do only one iteration
	    cnt++;
	}while(true);
	if(VERBOSE || DEBUG)cout <<" Finished vertex fit with " << cnt << " iterations "<<endl;
    }
    delete gMinuit;
}
//=========================================================================================   
bool EICVertexFitter::isGoodFit(double xx,double yy,double zz, int i){
    if(i==0){
	if(fabs(mPrimVtx.X()-xx)/mPrimVtx.X() < mtolx && 
	   fabs(mPrimVtx.Y()-yy)/mPrimVtx.Y() < mtoly &&
	   fabs(mPrimVtx.Z()-zz)/mPrimVtx.Z() < mtolz)
	    return true;
    }else if(i==1){
	if(fabs(mPrimVtx.X()-xx)/mPrimVtx.X() < mtolx &&
           fabs(mPrimVtx.Y()-yy)/mPrimVtx.Y() < mtoly)
            return true;
    }
    else if(i==2){
	if(fabs(mPrimVtx.Z()-zz)/mPrimVtx.Z() < mtolz)
	   return true;
    }
    return false;
}
//=========================================================================================  
void EICVertexFitter::init2Seed(int ifit)
{
    // Function for walking particle vectors to point of closest approach to seed vertex from                                                                                                           // an arbitrary starting position   
    
    if(DEBUG){
	cout <<"--->Init2Seed<---" << endl;
	cout << " PVTX " << mPrimVtx.X() <<" " << mPrimVtx.Y() << " " << mPrimVtx.Z() << endl;
    }
    double ave_z=0;
    for(int i = 0; i < mntracks; i++){
	TVector3 track_mom(mpx[i],mpy[i],mpz[i]);
	TVector3 track_pos(mvx[i],mvy[i],mvz[i]);
        TVector3 PrimVtx(0,0,0);
	if(ifit==0)PrimVtx.SetXYZ(mseedx,mseedy,mseedz);
	if(ifit==1){
	    PrimVtx.SetXYZ(mseedx,mseedy,0);
            track_mom.SetXYZ(mpx[i],mpy[i],0);
            track_pos.SetXYZ(mvx[i],mvy[i],0);
	}
        if(ifit==2){
            PrimVtx.SetZ(mseedz);
            track_mom.SetXYZ(0,0,mpz[i]);
            track_pos.SetXYZ(0,0,mvz[i]);
	}
	TVector3 diff = track_pos-PrimVtx;//Vector from seed vertex to known xyz point                                                                                                    
	double proj_xyz = track_mom.Dot(diff); // scalar product of track on vecotr between known point to vtx                                                                             
	TVector3 newXYZ = track_pos + proj_xyz*track_mom.Unit(); // New xyz point                                                                                                          
	mnvx[i] = newXYZ.X();
	mnvy[i] = newXYZ.Y();
	mnvz[i] = newXYZ.Z();
        if(ifit==2)ave_z+=mnvz[i];
	if(DEBUG)cout << " Track DCAs " << mnvx[i] << " " << mnvy[i] << " " << mnvz[i] << endl;		
    }
    if(ifit==2) mStartZ=ave_z/mntracks;
}
//=========================================================================================  
void EICVertexFitter::init2Vtx(int ifit)
{
// Function for walking particle vectors to point of closest approach to vertex  
    if(DEBUG){
	cout <<"--->Init2Vtx<---" << endl;
	cout << " PVTX " << mPrimVtx.X() <<" " << mPrimVtx.Y() << " " << mPrimVtx.Z() << endl;
    }
    for(int i = 0; i < mntracks; i++){
	 TVector3 track_mom(mpx[i],mpy[i],mpz[i]);
	 TVector3 track_pos(mvx[i],mvy[i],mvz[i]);
	 if(ifit==1){
	     track_mom.SetXYZ(mpx[i],mpy[i],0);
	     track_pos.SetXYZ(mvx[i],mvy[i],0);
	 }
	 if(ifit==2){
	     track_mom.SetXYZ(0,0,mpz[i]);
	     track_pos.SetXYZ(0,0,mvz[i]);
	 }
        TVector3 diff = track_pos-mPrimVtx;//Vector from seed vertex to known xyz point                                                                                                       
        double proj_xyz = track_mom.Dot(diff); // scalar product of track on vecotr between known point to vtx                                                        
        TVector3 newXYZ = track_pos + proj_xyz*track_mom.Unit(); // New xyz point                                                                                                          
        mnvx[i] = newXYZ.X();
        mnvy[i] = newXYZ.Y();
        mnvz[i] = newXYZ.Z();
	if(DEBUG)cout << " Track DCAs " << mnvx[i] <<" " << mnvy[i] << " " <<mnvz[i] << endl;   
    }
}
//=========================================================================================  
Double_t EICVertexFitter::leastSquares(float x, float y, float z, float ex, float ey, float ez, Double_t *par)
{
    return sqrt( (x-par[0])*(x-par[0])/ex/ex + (y-par[1])*(y-par[1])/ey/ey + (z-par[2])*(z-par[2])/ez/ez );
}
//=========================================================================================                                                                                                     
Double_t EICVertexFitter::leastSquares(float x, float y, float ex, float ey, Double_t *par)
{
    return sqrt( (x-par[0])*(x-par[0])/ex/ex + (y-par[1])*(y-par[1])/ey/ey );
}
//=========================================================================================                                                                                                     
Double_t EICVertexFitter::leastSquares(float x, float ex, Double_t *par)
{
    return sqrt( (x-par[0])*(x-par[0])/ex/ex );
}
//=========================================================================================  
void EICVertexFitter::fcn(Int_t &npar, Double_t *gin, Double_t &f, Double_t *par, Int_t iflag)
{
    //calculate chisquare
    Double_t chisq = 0;
    for (int i = 0; i < mntracks; i++)chisq += leastSquares(mnvx[i],mnvy[i],mnvz[i],mnerrvx[i],mnerrvy[i],mnerrvz[i],par);
    f = chisq;
}
//========================================================================================= 
void EICVertexFitter::fcnxy(Int_t &npar, Double_t *gin, Double_t &f, Double_t *par, Int_t iflag)
{
    //calculate chisquare                                                                                                                                                                     
    Double_t chisq = 0;
    for (int i = 0; i < mntracks; i++)chisq += leastSquares(mnvx[i],mnvy[i],mnerrvx[i],mnerrvy[i],par);
    f = chisq;
}
//========================================================================================= 
void EICVertexFitter::fcnz(Int_t &npar, Double_t *gin, Double_t &f, Double_t *par, Int_t iflag)
{
    //calculate chisquare                                                                                                                                                                 
    Double_t chisq = 0;
    for (int i = 0; i < mntracks; i++)chisq += leastSquares(mnvz[i],mnerrvz[i],par);
    f = chisq;
}

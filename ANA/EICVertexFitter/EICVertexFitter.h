#ifndef EICVertexFitter_H
#define EICVertexFitter_H

/* **************************************************
 *  Class for simple EIC vertex fitter
 *
 *  Authors:  
 *            Matthew Kelsey        (mkelsey@wayne.gov)
 * **************************************************
 */

#include <vector>
#include "TNamed.h"
#include "TString.h"
#include "TVector3.h"
#include <iostream>
#include "TMinuit.h"

using namespace std;
//Need below variables global to avoid problems with TMinuit 
vector <float> mnvx;  // origin proj x                                                                                                                                                             
vector <float> mnvy;  // origin proj y                                                                                                                                                              
vector <float> mnvz;  // origin proj z    
vector <float> mvx;  // origin x                                                                                                                                                            
vector <float> mvy;  // origin y                                                                                                                                                            
vector <float> mvz;  // origin z                                                                                                                                                              
vector <float> mnerrvx;  // error x
vector <float> mnerrvy;  // error y
vector <float> mnerrvz;  // error z   
int mntracks;        // number of tracks     
int _MAX_ = 100; // maximum number of fit iterations 

class EICVertexFitter : public TNamed
{
public:
    
    EICVertexFitter();
    EICVertexFitter(const Char_t *name);
    ~EICVertexFitter();
    void init(int);
    void addParticle(TVector3,TVector3);
    void addParticle(TVector3,TVector3,double,double);
    void doFit(int);
    bool isGoodFit(double,double,double,int);
    void init2Seed(int);
    void init2Vtx(int);
    void setSeedX(float i);
    void setSeedY(float i);
    void setSeedZ(float i);
    void setVerbose(bool i);
    void setDebug(bool i);
    TVector3 getVtx();
    int getFitStatus();
    float mseedx;      // seed for x-dim
    float mseedy;      // seed for y-dim
    float mseedz;      // seed for z-dim                                                                                                                                                       
    float mtolx;       // tolerance for x-dim                                                                                                                                                  
    float mtoly;       // tolerance for y-dim                                                                                                                                                  
    float mtolz;       // tolerance for z-dim  
private:
    
    EICVertexFitter(EICVertexFitter const &);       
    EICVertexFitter& operator=(EICVertexFitter const &); 
    bool VERBOSE;
    bool DEBUG;
    int mflag;
    int mfit;
    float mStartZ;
    vector <float> mpx;  // Px  
    vector <float> mpy;  // Py   
    vector <float> mpz;  // Pz  
    TVector3 mPrimVtx;   // primary vertex of current event
    static Double_t leastSquares(float,float,float,float,float,float,Double_t *par);
    static Double_t leastSquares(float,float,float,float,Double_t *par);
    static Double_t leastSquares(float,float,Double_t *par);
    static void fcn(Int_t &npar, Double_t *gin, Double_t &f, Double_t *par, Int_t iflag);
    static void fcnxy(Int_t &npar, Double_t *gin, Double_t &f, Double_t *par, Int_t iflag);
    static void fcnz(Int_t &npar, Double_t *gin, Double_t &f, Double_t *par, Int_t iflag);
    ClassDef(EICVertexFitter,1)
};
inline void EICVertexFitter::setSeedX(float i) {mseedx = i; }
inline void EICVertexFitter::setSeedY(float i) {mseedy = i; }
inline void EICVertexFitter::setSeedZ(float i) {mseedz = i; }
inline TVector3 EICVertexFitter::getVtx(){return mPrimVtx; }
inline void EICVertexFitter::setVerbose(bool i){VERBOSE = i;}
inline void EICVertexFitter::setDebug(bool i){DEBUG = i;}
inline int EICVertexFitter::getFitStatus(){return mflag;}
#endif

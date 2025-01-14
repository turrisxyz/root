/*****************************************************************************
 * Project: RooFit                                                           *
 * Package: RooFitBabar                                                      *
 * @(#)root/roofit:$Id$
 * Authors:                                                                  *
 *    Aaron Roodman, Stanford Linear Accelerator Center, Stanford University *
 *                                                                           *
 * Copyright (c) 2004, Stanford University. All rights reserved.        *
 *
 * Redistribution and use in source and binary forms,                        *
 * with or without modification, are permitted according to the terms        *
 * listed in LICENSE (http://roofit.sourceforge.net/license.txt)             *
 *****************************************************************************/

/** \class RooParametricStepFunction
    \ingroup Roofit

The Parametric Step Function PDF is a binned distribution whose parameters
are the heights of each bin.  This PDF was first used in BaBar's B0->pi0pi0
paper BABAR Collaboration (B. Aubert et al.) Phys.Rev.Lett.91:241801,2003.

This PDF may be used to describe oddly shaped distributions.  It differs
from a RooKeysPdf or a RooHistPdf in that a RooParametricStepFunction
has free parameters.  In particular, any statistical uncertainty in
sample used to model this PDF may be understood with these free parameters;
this is not possible with non-parametric PDFs.

The RooParametricStepFunction has Nbins-1 free parameters. Note that
the limits of the dependent variable must match the low and hi bin limits.

An example of usage is:

~~~ {.cpp}
Int_t nbins(10);
TArrayD limits(nbins+1);
limits[0] = 0.0; //etc...
RooArgList* list = new RooArgList("list");
RooRealVar* binHeight0 = new RooRealVar("binHeight0","bin 0 Value",0.1,0.0,1.0);
list->add(binHeight0); // up to binHeight8, ie. 9 parameters

RooParametricStepFunction  aPdf = ("aPdf","PSF",*x,
                                   *list,limits,nbins);
~~~
*/

#include "Riostream.h"
#include "TArrayD.h"
#include <math.h>

#include "RooParametricStepFunction.h"
#include "RooAbsReal.h"
#include "RooRealVar.h"
#include "RooArgList.h"

#include "TError.h"

using namespace std;

ClassImp(RooParametricStepFunction);

////////////////////////////////////////////////////////////////////////////////
/// Constructor

RooParametricStepFunction::RooParametricStepFunction(const char* name, const char* title,
              RooAbsReal& x, const RooArgList& coefList, TArrayD& limits, Int_t nBins) :
  RooAbsPdf(name, title),
  _x("x", "Dependent", this, x),
  _coefList("coefList","List of coefficients",this),
  _nBins(nBins)
{

  // Check lowest order
  if (_nBins<0) {
    cout << "RooParametricStepFunction::ctor(" << GetName()
    << ") WARNING: nBins must be >=0, setting value to 0" << endl ;
    _nBins=0 ;
  }

  for (auto *coef : coefList) {
    if (!dynamic_cast<RooAbsReal*>(coef)) {
      cout << "RooParametricStepFunction::ctor(" << GetName() << ") ERROR: coefficient " << coef->GetName()
      << " is not of type RooAbsReal" << endl ;
      R__ASSERT(0) ;
    }
    _coefList.add(*coef) ;
  }

  // Bin limits
  limits.Copy(_limits);

}

////////////////////////////////////////////////////////////////////////////////
/// Copy constructor

RooParametricStepFunction::RooParametricStepFunction(const RooParametricStepFunction& other, const char* name) :
  RooAbsPdf(other, name),
  _x("x", this, other._x),
  _coefList("coefList",this,other._coefList),
  _nBins(other._nBins)
{
  (other._limits).Copy(_limits);
}

////////////////////////////////////////////////////////////////////////////////

Int_t RooParametricStepFunction::getAnalyticalIntegral(RooArgSet& allVars, RooArgSet& analVars, const char* /*rangeName*/) const
{
  if (matchArgs(allVars, analVars, _x)) return 1;
  return 0;
}

////////////////////////////////////////////////////////////////////////////////

double RooParametricStepFunction::analyticalIntegral(Int_t code, const char* rangeName) const
{
  R__ASSERT(code==1) ;

  // Case without range is trivial: p.d.f is by construction normalized
  if (!rangeName) {
    return 1.0 ;
  }

  // Case with ranges, calculate integral explicitly
  double xmin = _x.min(rangeName) ;
  double xmax = _x.max(rangeName) ;

  double sum=0 ;
  Int_t i ;
  for (i=1 ; i<=_nBins ; i++) {
    double binVal = (i<_nBins) ? (static_cast<RooRealVar*>(_coefList.at(i-1))->getVal()) : lastBinValue() ;
    if (_limits[i-1]>=xmin && _limits[i]<=xmax) {
      // Bin fully in the integration domain
      sum += (_limits[i]-_limits[i-1])*binVal ;
    } else if (_limits[i-1]<xmin && _limits[i]>xmax) {
      // Domain is fully contained in this bin
      sum += (xmax-xmin)*binVal ;
      // Exit here, this is the last bin to be processed by construction
      return sum ;
    } else if (_limits[i-1]<xmin && _limits[i]<=xmax && _limits[i]>xmin) {
      // Lower domain boundary is in bin
      sum +=  (_limits[i]-xmin)*binVal ;
    } else if (_limits[i-1]>=xmin && _limits[i]>xmax && _limits[i-1]<xmax) {
      sum +=  (xmax-_limits[i-1])*binVal ;
      // Upper domain boundary is in bin
      // Exit here, this is the last bin to be processed by construction
      return sum ;
    }
  }

  return sum;
}

////////////////////////////////////////////////////////////////////////////////

double RooParametricStepFunction::lastBinValue() const
{
  double sum(0.);
  double binSize(0.);
  for (Int_t j=1;j<_nBins;j++){
    RooRealVar* tmp = (RooRealVar*) _coefList.at(j-1);
    binSize = _limits[j] - _limits[j-1];
    sum = sum + tmp->getVal()*binSize;
  }
  binSize = _limits[_nBins] - _limits[_nBins-1];
  return (1.0 - sum)/binSize;
}

////////////////////////////////////////////////////////////////////////////////

double RooParametricStepFunction::evaluate() const
{
  double value(0.);
  if (_x >= _limits[0] && _x < _limits[_nBins]){

    for (Int_t i=1;i<=_nBins;i++){
      if (_x < _limits[i]){
   // in Bin i-1 (starting with Bin 0)
   if (i<_nBins) {
     // not in last Bin
     RooRealVar* tmp = (RooRealVar*) _coefList.at(i-1);
     value =  tmp->getVal();
     break;
   } else {
     // in last Bin
     double sum(0.);
     double binSize(0.);
     for (Int_t j=1;j<_nBins;j++){
       RooRealVar* tmp = (RooRealVar*) _coefList.at(j-1);
       binSize = _limits[j] - _limits[j-1];
       sum = sum + tmp->getVal()*binSize;
     }
     binSize = _limits[_nBins] - _limits[_nBins-1];
     value = (1.0 - sum)/binSize;
     if (value<=0.0){
       value = 0.000001;
       //       cout << "RooParametricStepFunction: sum of values gt 1.0 -- beware!!" <<endl;
     }
     break;
   }
      }
    }

  }
  return value;

}

////////////////////////////////////////////////////////////////////////////////

Int_t RooParametricStepFunction::getnBins(){
  return _nBins;
}

////////////////////////////////////////////////////////////////////////////////

double* RooParametricStepFunction::getLimits(){
  double* limoutput = _limits.GetArray();
  return limoutput;
}

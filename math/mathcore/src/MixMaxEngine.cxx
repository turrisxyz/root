// @(#)root/mathcore:$Id$
// Authors: L. Moneta    8/2015

/**********************************************************************
 *                                                                    *
 * Copyright (c) 2015 , ROOT MathLib Team                             *
 *                                                                    *
 *                                                                    *
 **********************************************************************/

// implementation file of MixMax engine
//
//
// Created by: Lorenzo Moneta  : Tue 4 Aug 2015
//
//
#include "Math/MixMaxEngine.h"

#include "mixmax.h"


namespace ROOT {
namespace Math {
   
   MixMaxEngine::MixMaxEngine(uint64_t seed) { 
      fRngState = rng_alloc();
      SetSeed64(seed); 
   }

   MixMaxEngine::~MixMaxEngine() { 
      rng_free(fRngState);
   }


   // void MixMaxEngine::SeedUniqueStream(unsigned int clusterID, unsigned int machineID, unsigned int runID, unsigned int  streamID) { 
   //    seed_uniquestream(fRngState, clusterID,  machineID,  runID,   streamID);
   // }

   /// set decimation number
   void MixMaxEngine::SetSkipNumber(int nskip) {
      set_skip_number(nskip);
   }
   
   /// set initial number to be used in the vector (the previous elements are skipped and not returned)
   void MixMaxEngine::SetFirstReturnElement(int index)  {
      set_first_return_element(index); 
   }

   
   void MixMaxEngine::SetSeed(unsigned int seed) { 
      seed_spbox(fRngState, seed);
      // no need to call iterate. The iteration will happen anyway
   }

   void MixMaxEngine::SetSeed64(uint64_t seed) { 
      seed_spbox(fRngState, seed);
   }

   // unsigned int MixMaxEngine::GetSeed() const { 
   //    return get_next(fRngState);
   // }
         

   // generate one random number in interval ]0,1]
   double MixMaxEngine::Rndm_impl()  { 
      return get_next_float(fRngState);
   }

   // generate one integer number 
   uint64_t MixMaxEngine::IntRndm() { 
      return get_next(fRngState);
   }

                  
   void  MixMaxEngine::RndmArray(int n, double *array){
      // Return an array of n random numbers uniformly distributed in ]0,1]
      fill_array(fRngState, n,  array);
   }



   
  
   } // namespace Math
} // namespace ROOT

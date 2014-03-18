/*
   CheMPS2: a spin-adapted implementation of DMRG for ab initio quantum chemistry
   Copyright (C) 2013, 2014 Sebastian Wouters

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef CASSCF_H
#define CASSCF_H

#include <string>

#include "Hamiltonian.h"
#include "Irreps.h"
#include "TwoDM.h"
#include "DMRG.h"
#include "Problem.h"
#include "Options.h"
#include "ConvergenceScheme.h"
#include "DMRGSCFindices.h"
#include "DMRGSCFunitary.h"

namespace CheMPS2{
/** CASSCF class.
    \author Sebastian Wouters <sebastianwouters@gmail.com>
    \date June 18, 2013
    
    The CASSCF class provides an implementation of DMRGSCF [Zgid et al., J. Chem. Phys. 128, 144116 (2008)] with as DMRG solver the spin-adapted DMRG code CheMPS2. For the CASSCF update, the following implementations are included:\n
    - the quadratically convergent Newton-Raphson approach [Siegbahn et al., J. Chem. Phys. 74, 2384 (1981)]
    - the augmented Hessian Newton-Raphson approach [Lengsfield, J. Chem. Phys. 73, 382 (1980)]; which is ORCA's prefered choice
*/
   class CASSCF{

      public:
      
         //! Constructor
         /** \param filename The file containing the Psi output to start the DMRGSCF calculations. */
         CASSCF(const string filename);
         
         //! Constructor
         /** \param HamIn Hamiltonian containing the matrix elements of the Hamiltonian for which a CASSCF calculation is desired
             \param DOCCin Array containing the number of doubly occupied HF orbitals per irrep
             \param SOCCin Array containing the number of singly occupied HF orbitals per irrep */
         CASSCF(Hamiltonian * HamIn, int * DOCCin, int * SOCCin);
         
         //! Destructor
         ~CASSCF();
         
         //! Get the number of irreps
         /** \return The number of irreps */
         int getNumberOfIrreps();
         
         //! Set the start of the CASSCF calculation
         /** \param NoccIn Array of length numberOfIrreps containing the number of double occupied HF orbitals per irrep for the CASSCF loop.
             \param NDMRGIn Array of length numberOfIrreps containing the number of active orbitals per irrep for the CASSCF loop.
             \param NvirtIn Array of length numberOfIrreps containing the number of empty orbitals per irrep for the CASSCF loop. */
         void setupStart(int * NoccIn, int * NDMRGIn, int * NvirtIn);
         
         //! Does the state-specific CASSCF cycle with the (augmented hessian) newton raphson method
         /** \param Nelectrons Total number of electrons in the system: occupied HF orbitals + active space
             \param TwoS Twice the targeted spin
             \param Irrep Desired wave-function irrep
             \param OptScheme The optimization scheme to run the inner DMRG loop
             \param rootNum Denotes the targeted state in state-specific CASSCF; 1 means ground state, 2 first excited state etc. */
         double doCASSCFnewtonraphson(const int Nelectrons, const int TwoS, const int Irrep, ConvergenceScheme * OptScheme, const int rootNum);
         
         //! CASSCF unitary rotation remove call
         void deleteStoredUnitary(){ unitary->deleteStoredUnitary(); }
         
      private:
      
         //Index convention handler
         DMRGSCFindices * iHandler;
         
         //Unitary matrix storage and manipulator
         DMRGSCFunitary * unitary;
      
         //The original Hamiltonian
         Hamiltonian * HamOrig;
         
         //The rotated Hamiltonian
         Hamiltonian * HamRotated;
         
         //Irreps controller
         Irreps SymmInfo;
         
         //The number of orbitals
         int L;
         
         //The numberOfIrreps;
         int numberOfIrreps;
         
         //Double occupations
         int * DOCC;
         
         //Single occupations
         int * SOCC;
         
         //Boolean whether or not setupStart has been called
         bool setupStartCalled;
         
         //Number of DMRG orbitals
         int nOrbDMRG;

         //Copy theDMRG2DM over to CASSCF::DMRG2DM
         void copy2DMover(TwoDM * theDMRG2DM);

         //Update the unitary, 2DM and 1DM with the given NO basis
         void rotate2DMand1DM(const int N, double * eigenvecs, double * work); 
         
         //Space for the DMRG 1DM
         double * DMRG1DM;

         //Space for the DMRG 2DM
         double * DMRG2DM;
         
         //Set the DMRG 1DM
         void setDMRG1DM(const int N);

         //The NO in terms of the active space orbitals are stored in the nOrbDMRG*nOrbDMRG array eigenvecs
         void calcNOON(double * eigenvecs);
         
         //Helper function to fetch DOCC and SOCC from filename
         void allocateAndFillOCC(const string filename);
         
         //Helper function to copy the DOCC and SOCC arrays
         void allocateAndFillOCC(int * DOCCin, int * SOCCin);
         
         //Helper function to check HF
         void checkHF();
         
         //Helper function to fill the Hamiltonian for the DMRG part
         void fillHamDMRG(Hamiltonian * HamDMRG);
         
         //With the updated unitary, the rotated matrix elements can be determined --> do everything in memory
         void fillRotatedHamAllInMemory(double * workmem1, double * workmem2);
         
         //With the updated unitary, the rotated matrix elements can be determined --> do everything in memory, but blockwise
         void fillRotatedHamInMemoryBlockWise(double * mem1, double * mem2, double * mem3, const int maxBlockSize);
         
         //Calculate the gradient, return function is the gradient 2-norm
         double calcGradient(double * gradient);
         
         //Calculate the hessian
         void calcHessian(double * hessian, const int rowjump);
         
         //Based on the new 2DM and 1DM from the DMRG calculation, calculate the gradient, Hessian, and new x
         double updateXmatrixNewtonRaphson();
         
         //Do Augmented Hessian form of NR
         double updateXmatrixAugmentedHessianNR();
         
         //Fmat function as defined by Eq. (11) in the Siegbahn paper.
         double FmatHelper(const int index1, const int index2) const;
         double Fmat(const int index1, const int index2) const;
         double ** Fmatrix;
         void buildFmat();
         
         //Wmat function as defined by Eq.(21b) in the Siegbahn paper.
         double Wmat(const int index1, const int index2, const int index3, const int index4) const;
         
         //Function to get the occupancies to obtain coefficients of certain Slater determinants for neutral C2. Important to figure out diatomic D(inf)h symmetries when calculating them in D2h symmetry. The function is not basis set and active space dependent (at least if no B2g, B3g, B2u and B3u orbitals are condensed).
         void PrintCoeff_C2(DMRG * theDMRG);
         
   };
}

#endif
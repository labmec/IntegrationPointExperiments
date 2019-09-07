//
//  TPBrConstitutiveLawProcessor.hpp
//  IntPointsFEM
//
//  Created by Omar Durán on 9/5/19.
//
#include "TPBrConstitutiveLawProcessor.h"

template<class T, class MEM>
TPBrConstitutiveLawProcessor<T, MEM>::TPBrConstitutiveLawProcessor() : fPlasticModel() {
    fNpts = -1;
    fWeight.Resize(0);
    fStateVec.resize(0);
}

template<class T, class MEM>
TPBrConstitutiveLawProcessor<T, MEM>::TPBrConstitutiveLawProcessor(int npts, TPZVec<REAL> &weight) {
    SetUpDataByIntPoints(npts);
    SetWeightVector(weight);
}

template<class T, class MEM>
TPBrConstitutiveLawProcessor<T, MEM>::~TPBrConstitutiveLawProcessor() {

}

template<class T, class MEM>
TPBrConstitutiveLawProcessor<T, MEM>::TPBrConstitutiveLawProcessor(const TPBrConstitutiveLawProcessor &copy) {
    fNpts = copy.fNpts;
    fWeight = copy.fWeight;
    fPlasticModel = copy.fPlasticModel;
    fStateVec = copy.fStateVec;
}

template<class T, class MEM>
TPBrConstitutiveLawProcessor<T, MEM> &
TPBrConstitutiveLawProcessor<T, MEM>::operator=(const TPBrConstitutiveLawProcessor &copy) {
    if (&copy == this) {
        return *this;
    }

    fNpts = copy.fNpts;
    fWeight = copy.fWeight;
    fPlasticModel = copy.fPlasticModel;
    fStateVec = copy.fStateVec;

    return *this;
}

template<class T, class MEM>
void TPBrConstitutiveLawProcessor<T, MEM>::SetWeightVector(TPZVec<REAL> &weight) {
    fWeight = weight;
}

template<class T, class MEM>
void TPBrConstitutiveLawProcessor<T, MEM>::SetUpDataByIntPoints(int npts) {
    fNpts = npts;
    fStateVec.resize(fNpts);
}

template<class T, class MEM>
TPZVec<REAL> &TPBrConstitutiveLawProcessor<T, MEM>::WeightVector() {
    return fWeight;
}

template<class T, class MEM>
void
TPBrConstitutiveLawProcessor<T, MEM>::ComputeSigma(TPZFMatrix<REAL> &glob_delta_strain, TPZFMatrix<REAL> &glob_sigma) {

    glob_sigma.Resize(3 * fNpts, 1);

    for (int ipts = 0; ipts < fNpts; ipts++) {
        TPZTensor<REAL> epsTotal;
        TPZTensor<REAL> sigma;

        epsTotal[_XX_] = glob_delta_strain(3 * ipts + 0, 0);
        epsTotal[_XY_] = glob_delta_strain(3 * ipts + 1, 0);
        epsTotal[_XZ_] = 0;
        epsTotal[_YY_] = glob_delta_strain(3 * ipts + 2, 0);
        epsTotal[_YZ_] = 0;
        epsTotal[_ZZ_] = 0;

        fPlasticModel.SetState(fStateVec[ipts]);
        fPlasticModel.ApplyStrainComputeSigma(epsTotal, sigma);
        sigma.operator*=(fWeight[ipts]);

        glob_sigma(3 * ipts + 0, 0) = sigma[_XX_];
        glob_sigma(3 * ipts + 1, 0) = sigma[_XY_] / 2;
        glob_sigma(3 * ipts + 2, 0) = sigma[_YY_];
    }
}
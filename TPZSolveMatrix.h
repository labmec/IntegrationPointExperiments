/**
 * @file
 * @brief Contains the TPZSolveMatrix class which implements a solution based on a matrix procedure.
 */

#ifndef TPZSolveMatrix_h
#define TPZSolveMatrix_h

#include "pzmatrix.h"
#include "pzfmatrix.h"
#include <mkl.h>
#include "pzinterpolationspace.h"
#include "pzcmesh.h"
#ifdef USING_MKL
#include "mkl.h"
#endif
#ifdef __CUDACC__
#include <cuda.h>
#include <cublas_v2.h>
#include <cusparse.h>
#include "mkl.h"
#endif

class TPZSolveMatrix : public TPZMatrix<STATE> {

public:

    TPZSolveMatrix() : TPZMatrix<STATE>(), fStorage(), fIndexes(), fColSizes(), fRowSizes(), fMatrixPosition() {

    }

    TPZSolveMatrix(int64_t rows, int64_t cols, TPZVec<MKL_INT> rowsizes, TPZVec<MKL_INT> colsizes) : TPZMatrix(rows,
                                                                                                               cols) {
        SetParameters(rowsizes, colsizes);
        cuSparseHandle();
        cuBlasHandle();
    }


    ~TPZSolveMatrix() {

    }

    TPZSolveMatrix(const TPZSolveMatrix &copy) : TPZMatrix<STATE>(copy),
                                                 fStorage(copy.fStorage), fIndexes(copy.fIndexes),
                                                 fColSizes(copy.fColSizes), fRowSizes(copy.fRowSizes),
                                                 fMatrixPosition(copy.fMatrixPosition),
                                                 fRowFirstIndex(copy.fRowFirstIndex),
                                                 fElemColor(copy.fElemColor), fIndexesColor(copy.fIndexesColor),
                                                 fColFirstIndex(copy.fColFirstIndex), dglobal_solution(copy.dglobal_solution),
                                                 dindexes(copy.dindexes), dstorage(copy.dstorage), dstoragevec(copy.dstoragevec), dexpandsolution(copy.dexpandsolution),
                                                 dresult(copy.dresult), dweight(copy.dweight), dsigma(copy.dsigma), dnodal_forces_vec(copy.dnodal_forces_vec),
                                                 dindexescolor(copy.dindexescolor), dnodal_forces_global(copy.dnodal_forces_global) {

    }

    TPZSolveMatrix &operator=(const TPZSolveMatrix &copy) {
        TPZMatrix::operator=(copy);
        fStorage = copy.fStorage;
        fIndexes = copy.fIndexes;
        fColSizes = copy.fColSizes;
        fRowSizes = copy.fRowSizes;
        fElemColor = copy.fElemColor;
        fIndexesColor = copy.fIndexesColor;
        fMatrixPosition = copy.fMatrixPosition;
        fRowFirstIndex = copy.fRowFirstIndex;
        fColFirstIndex = copy.fColFirstIndex;
        dglobal_solution = copy.dglobal_solution;
        dindexes = copy.dindexes;
        dstorage = copy.dstorage;
        dstoragevec = copy.dstoragevec;
        dexpandsolution = copy.dexpandsolution;
        dresult = copy.dresult;
        dweight = copy.dweight;
        dsigma = copy.dsigma;
        dnodal_forces_vec = copy.dnodal_forces_vec;
        dindexescolor = copy.dindexescolor;
        dnodal_forces_global = copy.dnodal_forces_global;

        return *this;
    }

    TPZMatrix<STATE> *Clone() const {
        return new TPZSolveMatrix(*this);
    }

    void SetParameters(TPZVec<MKL_INT> rowsize, TPZVec<MKL_INT> colsize) {
        int64_t nelem = rowsize.size();

        fRowSizes.resize(nelem);
        fColSizes.resize(nelem);
        fMatrixPosition.resize(nelem + 1);
        fRowFirstIndex.resize(nelem + 1);
        fColFirstIndex.resize(nelem + 1);

        fRowSizes = rowsize;
        fColSizes = colsize;

        fMatrixPosition[0] = 0;
        fRowFirstIndex[0] = 0;
        fColFirstIndex[0] = 0;

        for (int64_t iel = 0; iel < nelem; ++iel) {
            fMatrixPosition[iel + 1] = fMatrixPosition[iel] + fRowSizes[iel] * fColSizes[iel];
            fRowFirstIndex[iel + 1] = fRowFirstIndex[iel] + fRowSizes[iel];
            fColFirstIndex[iel + 1] = fColFirstIndex[iel] + fColSizes[iel];
        }
        fStorage.resize(fMatrixPosition[nelem]);
        fElemColor.resize(nelem);
        fElemColor.Fill(-1);
    }

    void SetElementMatrix(int iel, TPZFMatrix<REAL> &elmat) {
        int64_t rows = fRowSizes[iel];
        int64_t cols = fColSizes[iel];
        int64_t pos = fMatrixPosition[iel];
        int64_t nelem = fRowSizes.size();

        TPZFMatrix<REAL> elmatloc(rows, cols, &fStorage[pos], rows*cols);
        elmatloc = elmat;
    }

    void SetIndexes(TPZVec<MKL_INT> indexes) {
        int64_t indsize = indexes.size();
        fIndexes.resize(indsize);
        fIndexes = indexes;
        fIndexesColor.resize(indsize);
    }

    /** @brief Solve procedure */

    //USING CUDA
    void AllocateMemory(TPZCompMesh *cmesh);
 
    void FreeMemory();

    void cuSparseHandle();

    void cuBlasHandle();


    void MultiplyCUDA(const TPZFMatrix<STATE> &global_solution, TPZFMatrix<STATE> &result) const;

    void ComputeSigmaCUDA(TPZStack<REAL> &weight, TPZFMatrix<REAL> &result, TPZFMatrix<REAL> &sigma);

    void MultiplyTransposeCUDA(TPZFMatrix<STATE> &intpoint_solution, TPZFMatrix<STATE> &nodal_forces_vec);

    void ColoredAssembleCUDA(TPZFMatrix<STATE> &nodal_forces_vec, TPZFMatrix<STATE> &nodal_forces_global);



    void Multiply(const TPZFMatrix<STATE> &global_solution, TPZFMatrix<STATE> &result) const;

    void ComputeSigma(TPZStack<REAL> &weight, TPZFMatrix<REAL> &result, TPZFMatrix<REAL> &sigma);

    void MultiplyTranspose(TPZFMatrix<STATE> &intpoint_solution, TPZFMatrix<STATE> &nodal_forces_vec);

    void ColoredAssemble(TPZFMatrix<STATE> &nodal_forces_vec, TPZFMatrix<STATE> &nodal_forces_global);


    void TraditionalAssemble(TPZFMatrix<STATE> &nodal_forces_vec, TPZFMatrix<STATE> &nodal_forces_global) const;

    void ColoringElements(TPZCompMesh *cmesh) const;

protected:

/// vector containing the matrix coefficients
    TPZVec<REAL> fStorage;

/// number of rows of each block matrix
    TPZVec<MKL_INT> fRowSizes;

/// number of columns of each block matrix
    TPZVec<MKL_INT> fColSizes;

/// indexes vector in x and y direction
    TPZVec<MKL_INT> fIndexes;

/// indexes vector in x and y direction by color
    TPZVec<MKL_INT> fIndexesColor;

/// color indexes of each element
    TPZVec<int64_t> fElemColor;

/// position of the matrix of the elements
    TPZVec<int64_t> fMatrixPosition;

/// position of the result vector
    TPZVec<int64_t> fRowFirstIndex;

/// position in the fIndex vector of each element
    TPZVec<int64_t> fColFirstIndex;

/// Parameters stored on device
    double *dglobal_solution;
    int *dindexes;
    double *dstorage;
    double *dstoragevec;
    double *dexpandsolution;
    double *dresult;
    double *dweight;
    double *dsigma;
    double *dnodal_forces_vec;
    int *dindexescolor;
    double *dnodal_forces_global;

//Libraries handles
#ifdef __CUDACC__
    cusparseHandle_t handle_cusparse;
    cublasHandle_t handle_cublas;
#endif

};

#endif /* TPZSolveMatrix_h */

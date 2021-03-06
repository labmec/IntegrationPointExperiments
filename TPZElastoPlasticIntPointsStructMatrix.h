/**
 * @file
 * @brief Contains the TPZSolveMatrix class which implements a solution based on a matrix procedure.
 */

#ifndef TPZIntPointsFEM_h
#define TPZIntPointsFEM_h

//#include <StrMatrix/TPZSSpStructMatrix.h>
#include <TPZSSpStructMatrix.h>
#include "pzsysmp.h"
#include "tpzverysparsematrix.h"
#include "TPZIrregularBlocksMatrix.h"
#include "TPZNumericalIntegrator.h"
#include <map>


class TPZElastoPlasticIntPointsStructMatrix : public TPZSymetricSpStructMatrix {
    
public:
    
    /** @brief Default constructor */
    TPZElastoPlasticIntPointsStructMatrix();

    /** @brief Creates the object based on a Compmesh
     * @param Compmesh : Computational mesh */
    TPZElastoPlasticIntPointsStructMatrix(TPZCompMesh *cmesh);

    /** @brief Default destructor */
    ~TPZElastoPlasticIntPointsStructMatrix();

    /** @brief Clone */
    TPZStructMatrix *Clone();
    
    TPZMatrix<STATE> * Create();

    TPZMatrix<STATE> *CreateAssemble(TPZFMatrix<STATE> &rhs, TPZAutoPointer<TPZGuiInterface> guiInterface);


    void Assemble(TPZMatrix<STATE> & mat, TPZFMatrix<STATE> & rhs, TPZAutoPointer<TPZGuiInterface> guiInterface);
    
    void Assemble(TPZFMatrix<STATE> & rhs, TPZAutoPointer<TPZGuiInterface> guiInterface);

private:
    
#ifdef USING_CUDA
    void TransferDataToGPU();
#endif

    void SetUpDataStructure();

    void ComputeDomainElementIndexes(TPZVec<int> &element_indexes);
    
    void ClassifyMaterialsByDimension();
    
    void AssembleBoundaryData();

    int fDimension;

    TPZNumericalIntegrator fIntegrator;

    TPZVerySparseMatrix<STATE> fSparseMatrixLinear; //-> BC data

    TPZFMatrix<STATE> fRhsLinear; //-> BC data
    
    std::set<int> fBCMaterialIds;

    #ifdef USING_CUDA
    TPZCudaCalls fCudaCalls;

    TPZVecGPU<REAL> dRhsLinear;
    #endif
    
};

#endif /* TPZIntPointsFEM_h */

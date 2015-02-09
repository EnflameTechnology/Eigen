// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 20015 Gael Guennebaud <gael.guennebaud@inria.fr>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

// This unit test cannot be easily written to work with EIGEN_DEFAULT_TO_ROW_MAJOR
#ifdef EIGEN_DEFAULT_TO_ROW_MAJOR
#undef EIGEN_DEFAULT_TO_ROW_MAJOR
#endif

static long int nb_temporaries;

inline void on_temporary_creation() {
  // here's a great place to set a breakpoint when debugging failures in this test!
  nb_temporaries++;
}

#define EIGEN_SPARSE_CREATE_TEMPORARY_PLUGIN { on_temporary_creation(); }

#include "main.h"
#include <Eigen/SparseCore>

#define VERIFY_EVALUATION_COUNT(XPR,N) {\
    nb_temporaries = 0; \
    XPR; \
    if(nb_temporaries!=N) std::cerr << "nb_temporaries == " << nb_temporaries << "\n"; \
    VERIFY( (#XPR) && nb_temporaries==N ); \
  }

template<typename PlainObjectType> void check_const_correctness(const PlainObjectType&)
{
  // verify that ref-to-const don't have LvalueBit
  typedef typename internal::add_const<PlainObjectType>::type ConstPlainObjectType;
  VERIFY( !(internal::traits<Ref<ConstPlainObjectType> >::Flags & LvalueBit) );
  VERIFY( !(internal::traits<Ref<ConstPlainObjectType, Aligned> >::Flags & LvalueBit) );
  VERIFY( !(Ref<ConstPlainObjectType>::Flags & LvalueBit) );
  VERIFY( !(Ref<ConstPlainObjectType, Aligned>::Flags & LvalueBit) );
}

template<typename B>
EIGEN_DONT_INLINE void call_ref_1(Ref<SparseMatrix<float> > a, const B &b) { VERIFY_IS_EQUAL(a.toDense(),b.toDense()); }

template<typename B>
EIGEN_DONT_INLINE void call_ref_2(const Ref<const SparseMatrix<float> >& a, const B &b) { VERIFY_IS_EQUAL(a.toDense(),b.toDense()); }

void call_ref()
{
//   SparseVector<std::complex<float> > ca = VectorXcf::Random(10).sparseView();
//   SparseVector<float>                a  = VectorXf::Random(10).sparseView();
  SparseMatrix<float>               A = MatrixXf::Random(10,10).sparseView();
  SparseMatrix<float,RowMajor>      B = MatrixXf::Random(10,10).sparseView();
  const SparseMatrix<float>&        Ac(A);
  Block<SparseMatrix<float> >       Ab(A,0,1, 3,3);
  const Block<SparseMatrix<float> > Abc(A,0,1,3,3);
  

  VERIFY_EVALUATION_COUNT( call_ref_1(A, A),  0);
//   VERIFY_EVALUATION_COUNT( call_ref_1(Ac, Ac),  0); // does not compile on purpose
  VERIFY_EVALUATION_COUNT( call_ref_2(A, A),  0);
  VERIFY_EVALUATION_COUNT( call_ref_2(A.transpose(), A.transpose()),  1);
  VERIFY_EVALUATION_COUNT( call_ref_2(Ac,Ac), 0);
  VERIFY_EVALUATION_COUNT( call_ref_2(A+A,2*Ac), 1);
  VERIFY_EVALUATION_COUNT( call_ref_2(B, B),  1);
  VERIFY_EVALUATION_COUNT( call_ref_2(B.transpose(), B.transpose()),  0);
  VERIFY_EVALUATION_COUNT( call_ref_2(A*A, A*A),  1);
  
  Ref<SparseMatrix<float> > Ar(A);
  VERIFY_IS_APPROX(Ar+Ar, A+A);
  VERIFY_EVALUATION_COUNT( call_ref_1(Ar, A),  0);
  VERIFY_EVALUATION_COUNT( call_ref_2(Ar, A),  0);
  
  Ref<SparseMatrix<float,RowMajor> > Br(B);
  VERIFY_EVALUATION_COUNT( call_ref_1(Br.transpose(), Br.transpose()),  0);
  VERIFY_EVALUATION_COUNT( call_ref_2(Br, Br),  1);
  VERIFY_EVALUATION_COUNT( call_ref_2(Br.transpose(), Br.transpose()),  0);
  
  Ref<const SparseMatrix<float> > Arc(A);
//   VERIFY_EVALUATION_COUNT( call_ref_1(Arc, Arc),  0); // does not compile on purpose
  VERIFY_EVALUATION_COUNT( call_ref_2(Arc, Arc),  0);
  
  VERIFY_EVALUATION_COUNT( call_ref_2(A.middleCols(1,3), A.middleCols(1,3)),  1); // should be 0
  
  VERIFY_EVALUATION_COUNT( call_ref_2(A.block(1,1,3,3), A.block(1,1,3,3)),  1); // should be 0 (allocate starts/nnz only)
}

void test_sparse_ref()
{
  for(int i = 0; i < g_repeat; i++) {
    CALL_SUBTEST_1( check_const_correctness(SparseMatrix<float>()) );
    CALL_SUBTEST_1( check_const_correctness(SparseMatrix<double,RowMajor>()) );
    CALL_SUBTEST_2( call_ref() );
  }
}

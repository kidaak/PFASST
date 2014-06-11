/*
 * Host based encapsulated base sweeper.
 */

#ifndef _PFASST_VECTOR_HPP_
#define _PFASST_VECTOR_HPP_

#include <algorithm>
#include <vector>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

#define GPCHKERR(err, msg) if ((err) == -1) { perror(msg); return; }

#include "encapsulation.hpp"

using namespace std;

namespace pfasst {
  namespace encap {

    template<typename scalar, typename time>
    class VectorEncapsulation : public vector<scalar>, public Encapsulation<scalar,time> {

    public:
      VectorEncapsulation(int size) : vector<scalar>(size) { }

      void setval(scalar v) {
	std::fill(this->begin(), this->end(), v);
      }

      void copy(const Encapsulation<scalar,time>* X) {
	const auto* x = dynamic_cast<const VectorEncapsulation*>(X);
	std::copy(x->begin(), x->end(), this->begin());
      }

      void saxpy(time a, const Encapsulation<scalar,time> *X) {
	const auto& x = *dynamic_cast<const VectorEncapsulation*>(X);
	auto&       y = *this;

	for (int i=0; i<y.size(); i++)
	  y[i] += a * x[i];
      }

      void mat_apply(vector<Encapsulation<scalar,time>*> DST, time a, matrix<time> mat,
		     vector<Encapsulation<scalar,time>*> SRC, bool zero=true) {

	int ndst = DST.size();
	int nsrc = SRC.size();

	vector<VectorEncapsulation<scalar,time>*> dst(ndst), src(nsrc);
	for (int n=0; n<ndst; n++)
	  dst[n] = dynamic_cast<VectorEncapsulation*>(DST[n]);
	for (int m=0; m<nsrc; m++)
	  src[m] = dynamic_cast<VectorEncapsulation*>(SRC[m]);

	if (zero)
	  for (int n=0; n<ndst; n++)
	    dst[n]->setval(0.0);

	int ndofs = (*dst[0]).size();
	for (int i=0; i<ndofs; i++)
	  for (int n=0; n<ndst; n++)
	    for (int m=0; m<nsrc; m++)
	      (*dst[n])[i] += a * mat(n,m) * (*src[m])[i];
      }

      scalar norm0() const
      {
	scalar max = 0.0;
	for (int i=0; i<this->size(); i++) {
	  scalar v = abs((*this)[i]);
	  if (v > max)
	    max = v;
	}
	return max;
      }

    };

    template<typename scalar, typename time>
    class VectorFactory : public EncapsulationFactory<scalar,time> {
      int size;
    public:
      int dofs() { return size; }
      VectorFactory(const int size) : size(size) { }
      Encapsulation<scalar,time>* create(const EncapType) {
	return new VectorEncapsulation<scalar,time>(size);
      }
    };

  }
}

#endif

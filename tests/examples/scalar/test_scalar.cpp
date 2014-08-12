/*
 * Tests for the scalar example solving the test equation
 */
#include <cmath>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace ::testing;

#include <pfasst/quadrature.hpp>

#define PFASST_UNIT_TESTING
#include "../examples/scalar/scalar_sdc.cpp"
#undef PFASST_UNIT_TESTING

/*
 * parameterized test fixture with number of nodes as parameter
 */
class ConvergenceTest
  : public TestWithParam<tuple<size_t, pfasst::QuadratureType>>
{
  protected:
    size_t nnodes; // parameter

    complex<double> lambda;
    double Tend;
    vector<size_t> nsteps;
    size_t nsteps_l;
    vector<double> err;
    vector<double> convrate;
    double dt;
    size_t niters;
    pfasst::QuadratureType nodetype;
    size_t nnodes_in_call;

    // set parameters base on node type
    void set_parameters()
    {
      switch (this->nodetype)
      {
        case pfasst::QuadratureType::GaussLobatto:
          this->niters = 2 * this->nnodes - 2;
          this->Tend = 4.0;
          this->lambda = complex<double>(-1.0, 1.0);
          this->nsteps = { 2, 5, 10, 15, 20 };
          this->nnodes_in_call = this->nnodes;
          break;

        case pfasst::QuadratureType::GaussLegendre:
          this->niters = 2 * this->nnodes;
          this->Tend = 6.0;
          this->lambda = complex<double>(-1.0, 2.0);
          this->nsteps = { 2, 4, 6, 8, 10 };
          this->nnodes_in_call = this->nnodes + 2;
          break;

        /*
        case pfasst::QuadratureType::GaussRadau:
          break;

        case pfasst::QuadratureType::ClenshawCurtis:
          break;

        case pfasst::QuadratureType::Uniform:
          break;
         */

        default:
          break;
      }
    }

  public:
    virtual void SetUp()
    {
      this->nnodes = get<0>(GetParam());
      this->nodetype = get<1>(GetParam());
      this->set_parameters();
      this->nsteps_l = this->nsteps.size();
      this->err.resize(this->nsteps.size());
      this->convrate.resize(this->nsteps.size());

      // run to compute errors
      for (size_t i = 0; i <= this->nsteps_l - 1; ++i) {
        this->dt = this->Tend / double(this->nsteps[i]);
        this->err[i] = run_scalar_sdc(this->nsteps[i], this->dt, this->nnodes_in_call, 
                                      this->niters, this->lambda, this->nodetype);
      }

      // compute convergence rates
      for (size_t i = 0; i <= this->nsteps_l - 2; ++i) {
        this->convrate[i] = log10(this->err[i+1] / this->err[i]) / 
                              log10(double(this->nsteps[i]) / double(this->nsteps[i + 1]));
      }
    }

    virtual void TearDown()
    {}
};

/*
 * The test below verifies that the code approximately (up to a safety factor) reproduces
 * the theoretically expected rate of convergence
 */
TEST_P(ConvergenceTest, GaussNodes)
{
  for (size_t i = 0; i <= nsteps_l - 2; ++i) {
    switch (this->nodetype)
    {
      case pfasst::QuadratureType::GaussLobatto:
        // Expect convergence rate of 2*nodes-2 from collocation formula, doing an identical number
        // of iteration should suffice to reach this as each iteration should increase order by one
        EXPECT_THAT(convrate[i],
                    DoubleNear(double(2 * nnodes - 2), 0.99)) << "Convergence rate for " 
                                                              << this->nnodes
                                                              << " Gauss-Lobatto nodes "
                                                              << " at node " << i 
                                                              << " not within expected range.";
        break;

      case pfasst::QuadratureType::GaussLegendre:
        // convergence rates for Legendre nodes should be 2*nodes but are actually better, so
        // use Ge here
        EXPECT_THAT(convrate[i], Ge<double>(2 * this->nnodes)) << "Convergence rate for " 
                                                               << this->nnodes
                                                               << " Gauss-Legendre nodes "
                                                               << " at node " << i 
                                                               << " not within expected range.";
        break;

      /*
      case pfasst::QuadratureType::GaussRadau:
        break;

      case pfasst::QuadratureType::ClenshawCurtis:
        break;

      case pfasst::QuadratureType::Uniform:
        break;
       */

      default:
        EXPECT_TRUE(false);
        break;
    }
  }
}


INSTANTIATE_TEST_CASE_P(ScalarSDC, ConvergenceTest,
                        Combine(Range<size_t>(2, 7),
                                Values(pfasst::QuadratureType::GaussLobatto,
                                       pfasst::QuadratureType::GaussLegendre/*,
                                       pfasst::QuadratureType::GaussRadau,
                                       pfasst::QuadratureType::ClenshawCurtis,
                                       pfasst::QuadratureType::Uniform*/))
);


int main(int argc, char** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

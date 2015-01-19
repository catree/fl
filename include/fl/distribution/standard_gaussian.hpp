/**
 * \file standard_gaussian.hpp
 * \date May 2014
 * @author Manuel Wuthrich (manuel.wuthrich@gmail.com)
 * @author Jan Issac (jan.issac@gmail.com)
 */

#ifndef FL__DISTRIBUTION__STANDARD_GAUSSIAN_HPP
#define FL__DISTRIBUTION__STANDARD_GAUSSIAN_HPP

#include <Eigen/Dense>

#include <random>

#include <fl/util/random.hpp>
#include <fl/util/traits.hpp>
#include <fl/util/math.hpp>
#include <fl/distribution/interface/sampling.hpp>
#include <fl/exception/exception.hpp>

namespace fl
{

/// \todo MISSING DOC. MISSING UTESTS

/**
 * \ingroup distributions
 */
template <typename NormalVariate>
class StandardGaussian:
        public Sampling<NormalVariate>
{
public:
    explicit StandardGaussian(size_t dim = DimensionOf<NormalVariate>())
        : dimension_ (dim),
          generator_(RANDOM_SEED),
          gaussian_distribution_(0.0, 1.0)
    {
    }

    virtual ~StandardGaussian() { }

    virtual NormalVariate sample()
    {
        NormalVariate gaussian_sample(dimension());
        for (int i = 0; i < dimension(); i++)
        {
            gaussian_sample(i) = gaussian_distribution_(generator_);
        }

        return gaussian_sample;
    }

    virtual int dimension() const
    {
        return dimension_;
    }

    virtual void dimension(size_t new_dimension)
    {
        if (dimension_ == new_dimension) return;

        if (fl::IsFixed<NormalVariate::SizeAtCompileTime>())
        {
            fl_throw(
                fl::ResizingFixedSizeEntityException(dimension_,
                                                     new_dimension,
                                                     "Gaussian"));
        }

        dimension_ = new_dimension;
    }

private:
    int dimension_;
    fl::mt11213b generator_;
    std::normal_distribution<> gaussian_distribution_;
};

// specialization for scalar
template<>
class StandardGaussian<double>: public Sampling<double>
{
public:
    StandardGaussian()
        : generator_(RANDOM_SEED),
          gaussian_distribution_(0.0, 1.0)
    { }

    virtual ~StandardGaussian() { }

    virtual double sample()
    {
        return gaussian_distribution_(generator_);
    }

    virtual int dimension() const
    {
        return 1;
    }

private:
    fl::mt11213b generator_;
    std::normal_distribution<> gaussian_distribution_;
};

}

#endif

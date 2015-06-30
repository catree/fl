/*
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2014 Max-Planck-Institute for Intelligent Systems,
 *                     University of Southern California
 *    Jan Issac (jan.issac@gmail.com)
 *    Manuel Wuthrich (manuel.wuthrich@gmail.com)
 *
 *
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of Willow Garage, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 */

/**
 * @date 2015
 * @author Manuel Wuthrich (manuel.wuthrich@gmail.com)
 * Max-Planck-Institute for Intelligent Systems
 */


#ifndef FL__FILTER__PARTICLE__PARTICLE_FILTER_HPP
#define FL__FILTER__PARTICLE__PARTICLE_FILTER_HPP


#include <fl/distribution/discrete_distribution.hpp>
#include <fl/distribution/standard_gaussian.hpp>


/// TODO: REMOVE UNNECESSARY INCLUDES

#include <map>
#include <tuple>
#include <memory>

#include <fl/util/meta.hpp>
#include <fl/util/traits.hpp>
#include <fl/util/profiling.hpp>

#include <fl/exception/exception.hpp>
#include <fl/filter/filter_interface.hpp>
#include <fl/filter/gaussian/point_set.hpp>
#include <fl/filter/gaussian/feature_policy.hpp>

#include <fl/model/observation/joint_observation_model.hpp>

namespace fl
{

template <typename...> class ParticleFilter;

/**
 * ParticleFilter Traits
 */
template <typename ProcessModel, typename ObservationModel>
struct Traits<ParticleFilter<ProcessModel, ObservationModel>>
{
    typedef typename ProcessModel::State        State;
    typedef typename ProcessModel::Input        Input;
    typedef typename ObservationModel::Obsrv    Obsrv;
    typedef DiscreteDistribution<State>         Belief;
};


template<typename ProcessModel, typename ObservationModel>
class ParticleFilter<ProcessModel, ObservationModel>
    : public FilterInterface<ParticleFilter<ProcessModel, ObservationModel>>
{
private:
    /** \cond INTERNAL */
    typedef typename ProcessModel::Noise      ProcessNoise;
    typedef typename ObservationModel::Noise  ObsrvNoise;
    /** \endcond */

public:
    typedef typename ProcessModel::State        State;
    typedef typename ProcessModel::Input        Input;
    typedef typename ObservationModel::Obsrv    Obsrv;
    typedef DiscreteDistribution<State>         Belief;

public:
    ParticleFilter(const ProcessModel& process_model,
                   const ObservationModel& obsrv_model,
                   const Real& max_kl_divergence = 1.0)
        : process_model_(process_model),
          obsrv_model_(obsrv_model),
          process_noise_(process_model.noise_dimension()),
          obsrv_noise_(obsrv_model.noise_dimension()),
          max_kl_divergence_(max_kl_divergence)
    { }


    /// predict ****************************************************************
    virtual void predict(const Belief& prior_belief,
                         const Input& input,
                         Belief& predicted_belief)
    {
        predicted_belief = prior_belief;
        for(int i = 0; i < predicted_belief.size(); i++)
        {
            predicted_belief.location(i) =
                    process_model_.state(prior_belief.location(i),
                                         process_noise_.sample(),
                                         input);
        }
    }

    /// update *****************************************************************
    virtual void update(const Belief& predicted_belief,
                        const Obsrv& obsrv,
                        Belief& posterior_belief)
    {
        // if the samples are too concentrated then resample
        if(predicted_belief.kl_given_uniform() > max_kl_divergence_)
        {
            posterior_belief.from_distribution(predicted_belief,
                                               predicted_belief.size());
        }
        else
        {
            posterior_belief = predicted_belief;
        }

        // update the weights of the particles with the likelihoods
        posterior_belief.delta_log_prob_mass(
             obsrv_model_.log_probabilities(obsrv, predicted_belief.locations()));
    }

    /// predict and update *****************************************************
    virtual void predict_and_update(const Belief& prior_belief,
                                    const Input& input,
                                    const Obsrv& observation,
                                    Belief& posterior_belief)
    {
        predict(prior_belief, input, posterior_belief);
        update(posterior_belief, observation, posterior_belief);
    }


    /// set and get ************************************************************
    ProcessModel& process_model()
    {
        return process_model_;
    }
    ObservationModel& obsrv_model()
    {
        return obsrv_model_;
    }

    const ProcessModel& process_model() const
    {
        return process_model_;
    }

    const ObservationModel& obsrv_model() const
    {
        return obsrv_model_;
    }

    virtual Belief create_belief() const
    {
        auto belief = Belief(process_model().state_dimension());
        return belief;
    }

protected:
    ProcessModel process_model_;
    ObservationModel obsrv_model_;

    StandardGaussian<ProcessNoise> process_noise_;
    StandardGaussian<ObsrvNoise> obsrv_noise_;

    // when the KL divergence KL(p||u), where p is the particle distribution
    // and u is the uniform distribution, exceeds max_kl_divergence_, then there
    // is a resampling step. can be understood as -log(f) where f is the
    // fraction of nonzero particles.
    fl::Real max_kl_divergence_;
};

}


#endif

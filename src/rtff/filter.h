#ifndef RTFF_FILTER_H_
#define RTFF_FILTER_H_

#include <functional>

#include "rtff/abstract_filter.h"

namespace rtff {

/**
 * @brief Simple frequential filter that applies the execute function on each
 * frame
 */
class Filter : public AbstractFilter {
 public:
  Filter();
  virtual ~Filter();

  /**
   * @brief the function to be executed on each time frequency block
   * @see rtff::AbstractFilter::ProcessTransformedBlock for more.
   */
  std::function<void(Block<std::complex<float>>*)> execute;

 protected:
  void ProcessTransformedBlock(Block<std::complex<float>>*) override;
};

}  // namespace rtff

#endif  // RTFF_FILTER_H_

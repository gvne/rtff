#include "rtff/fft/fftw/fftw_fft.h"

#include <iostream>
#include <vector>
#include "fftw3.h"

#include <Eigen/Core>

namespace rtff {

class FFTWFft::Impl {
 public:
  Impl() : real_to_complex_(nullptr), complex_to_real_(nullptr) {}
  ~Impl() { Cleanup(); }

  void Init(uint32_t nfft) {
    real_data_.resize(nfft);
    complex_data_.resize(nfft / 2 + 1);

    // Initialize the fftw_plan
    auto complex_data_ptr = complex_data_.data();
    auto fftw_complex_data_ptr =
        reinterpret_cast<fftwf_complex*>(complex_data_ptr);
    auto real_data_ptr = real_data_.data();
    auto fftw_flags = FFTW_ESTIMATE;

#ifdef RTFF_FFTW_USE_WISDOM
    fftw_flags = FFTW_EXHAUSTIVE;
    // make the wisdom filename
    std::string wisdom_filename("rtff_" + std::to_string(nfft) + ".fftw");
    auto export_wisdom = true;

    // if the wisdom file exists
    FILE* wisdom = fopen(wisdom_filename.c_str(), "r");
    if (wisdom) {
      export_wisdom = false;
      fftw_flags = FFTW_WISDOM_ONLY | FFTW_EXHAUSTIVE;
      fftwf_import_wisdom_from_file(wisdom);
      fclose(wisdom);
    }
#endif  // RTFF_FFTW_USE_WISDOM

    // create the plan
    real_to_complex_ = fftwf_plan_dft_r2c_1d(
        nfft, real_data_ptr, fftw_complex_data_ptr, fftw_flags);
    complex_to_real_ = fftwf_plan_dft_c2r_1d(nfft, fftw_complex_data_ptr,
                                             real_data_ptr, fftw_flags);

#ifdef RTFF_FFTW_USE_WISDOM
    // export the wisdom if it didn't exist
    if (export_wisdom) {
      fftwf_export_wisdom_to_filename(wisdom_filename.c_str());
    }
#endif  // RTFF_FFTW_USE_WISDOM
  }

  void Forward(const float* in, std::complex<float>* out) {
    auto complex_data_ptr = complex_data_.data();
    auto real_data_ptr = real_data_.data();

    std::copy(in, in + get_nfft(), real_data_ptr);
    fftwf_execute(real_to_complex_);
    std::copy(complex_data_ptr, complex_data_ptr + get_nfft() / 2 + 1, out);
  }

  void Backward(const std::complex<float>* in, float* out) {
    auto complex_data_ptr = complex_data_.data();
    auto real_data_ptr = real_data_.data();

    std::copy(in, in + get_nfft() / 2 + 1, complex_data_ptr);
    // TODO also rebuild the complex conjugate in the second part
    fftwf_execute(complex_to_real_);
    std::copy(real_data_ptr, real_data_ptr + get_nfft(), out);

    // we need to devide the output by nfft
    Eigen::Map<Eigen::MatrixXf> out_matrix(out, get_nfft(), 1);
    out_matrix /= get_nfft();
  }

 private:
  uint32_t get_nfft() const { return real_data_.size(); }

  void Cleanup() {
    if (real_to_complex_) {
      fftwf_destroy_plan(real_to_complex_);
    }
    if (complex_to_real_) {
      fftwf_destroy_plan(complex_to_real_);
    }
  }

  std::vector<float> real_data_;
  std::vector<std::complex<float>> complex_data_;
  fftwf_plan real_to_complex_;
  fftwf_plan complex_to_real_;
};

FFTWFft::FFTWFft() : impl_(std::make_shared<FFTWFft::Impl>()) {}

void FFTWFft::Init(uint32_t nfft, std::error_code& err) { impl_->Init(nfft); }

void FFTWFft::Forward(const float* in, std::complex<float>* out) {
  impl_->Forward(in, out);
}

void FFTWFft::Backward(const std::complex<float>* in, float* out) {
  impl_->Backward(in, out);
}

}  // namespace rtff

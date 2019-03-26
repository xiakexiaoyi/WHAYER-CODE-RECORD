#include <Eigen/Eigen/Dense>
using namespace Eigen;

extern "C"
void convmul(const float* col_buffer, const float* weights, float* output, int M, int N, int K)
{
	Eigen::MatrixXf eA;
	Eigen::MatrixXf eB;

	Eigen::Map<Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> > mappedA ((float*)weights, M, K);
	Eigen::Map<Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> > mappedB ((float*)col_buffer, K, N);
	Eigen::Map<Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> > mappedC (output, M, N);

	eA = mappedA;
	eB = mappedB;

	mappedC = eA * eB;
}

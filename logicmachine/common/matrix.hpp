#ifndef COMMON_MATRIX_HPP
#define COMMON_MATRIX_HPP

#include <vector>
#include <stdexcept>
#include <cassert>


template <typename T>
class Matrix {

public:
	using value_type = T;
	using self_type = Matrix<T>;

private:
	size_t m_num_rows;
	size_t m_num_columns;
	std::vector<value_type> m_data;


	template <typename F>
	self_type apply_unary(F&& f) const {
		const size_t nm = m_data.size();
		self_type x(m_num_rows, m_num_columns);
		for(size_t i = 0; i < nm; ++i){ x.m_data[i] = f(m_data[i]); }
		return x;
	}

	template <typename F>
	self_type apply_binary(const self_type& x, F&& f) const {
		assert(m_num_rows == x.rows());
		assert(m_num_columns == x.cols());
		const size_t nm = m_data.size();
		self_type y(m_num_rows, m_num_columns);
		for(size_t i = 0; i < nm; ++i){
			y.m_data[i] = f(m_data[i], x.m_data[i]);
		}
		return y;
	}


public:
	Matrix()
		: m_num_rows(0)
		, m_num_columns(0)
		, m_data()
	{ }

	Matrix(size_t n, size_t m, const value_type& x = value_type())
		: m_num_rows(n)
		, m_num_columns(m)
		, m_data(n * m, x)
	{ }

	Matrix(size_t n, size_t m, const std::initializer_list<value_type>& init)
		: m_num_rows(n)
		, m_num_columns(m)
		, m_data(init)
	{
		assert(n * m == init.size());
	}


	size_t rows() const {
		return m_num_rows;
	}

	size_t cols() const {
		return m_num_columns;
	}


	value_type operator()(size_t i, size_t j) const {
		return m_data[i * m_num_columns + j];
	}

	value_type& operator()(size_t i, size_t j) noexcept {
		return m_data[i * m_num_columns + j];
	}


	bool operator==(const self_type& x) const {
		if(rows() != x.rows()){ return false; }
		if(cols() != x.cols()){ return false; }
		for(size_t i = 0; i < rows(); ++i){
			for(size_t j = 0; j < cols(); ++j){
				if((*this)(i, j) != x(i, j)){ return false; }
			}
		}
		return true;
	}

	bool operator!=(const self_type& x) const {
		return !(*this == x);
	}


	self_type operator+() const {
		return *this;
	}

	self_type operator-() const {
		return apply_unary(std::negate<value_type>());
	}


	self_type operator+(const self_type& x) const {
		return apply_binary(x, std::plus<value_type>());
	}

	self_type operator-(const self_type& x) const {
		return apply_binary(x, std::minus<value_type>());
	}

	Matrix<T> operator*(const Matrix<T>& x) const {
		assert(cols() == x.rows());
		const size_t N = this->rows(), M = this->cols(), K = x.cols();
		Matrix y(N, K);
		for(size_t i = 0; i < N; ++i){
			for(size_t j = 0; j < M; ++j){
				for(size_t k = 0; k < K; ++k){
					y(i, k) += (*this)(i, j) * x(j, k);
				}
			}
		}
		return y;
	}

	self_type operator*(const value_type& x) const {
		return apply_unary([x](const value_type& y){ return y * x; });
	}

	self_type operator/(const value_type& x) const {
		return apply_unary([x](const value_type& y){ return y / x; });
	}


	self_type& operator+=(const self_type& x){
		return (*this = *this + x);
	}

	self_type& operator-=(const self_type& x){
		return (*this = *this - x);
	}

	self_type& operator*=(const self_type& x){
		return (*this = *this * x);
	}

	self_type& operator*=(const value_type& x){
		return (*this = *this * x);
	}


	self_type operator~() const {
		return apply_unary([](const value_type& x){ return ~x; });
	}


	self_type operator&(const self_type& x) const {
		return apply_binary(x, std::bit_and<value_type>());
	}

	self_type operator|(const self_type& x) const {
		return apply_binary(x, std::bit_or<value_type>());
	}

	self_type operator^(const self_type& x) const {
		return apply_binary(x, std::bit_xor<value_type>());
	}


	self_type& operator&=(const self_type& x){
		return (*this = (*this & x));
	}

	self_type& operator|=(const self_type& x){
		return (*this = (*this | x));
	}

	self_type& operator^=(const self_type& x){
		return (*this = (*this ^ x));
	}

};


template <typename T>
Matrix<T> operator*(const T& x, const Matrix<T>& y){
	Matrix<T> z(y.rows(), y.cols());
	for(size_t i = 0; i < y.rows(); ++i){
		for(size_t j = 0; j < y.cols(); ++j){ z(i, j) = x * y(i, j); }
	}
	return z;
}

#endif

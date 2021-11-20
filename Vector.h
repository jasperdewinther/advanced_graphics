#pragma once
#include <utility>
#include <string>

class Vector3
{
public:
	std::array<float, 3> data = {};
	Vector3(const float elem1, const float elem2, const float elem3);
	Vector3 operator*(const Vector3& rhs) const;
	Vector3 operator/(const Vector3& rhs) const;
	Vector3 operator-(const Vector3& rhs) const;
	Vector3 operator+(const Vector3& rhs) const;
	float dot(const Vector3& rhs) const;
	Vector3 cross(const Vector3& rhs) const;
	bool operator==(const Vector3& rhs) const;
	bool operator!=(const Vector3& rhs) const;
	float& operator[](int rhs);
	void normalize();
	std::string to_string() const;
};

Vector3 operator*(const float lhs, const Vector3& rhs);
Vector3 operator/(const float lhs, const Vector3& rhs);
Vector3 operator-(const float lhs, const Vector3& rhs);
Vector3 operator+(const float lhs, const Vector3& rhs);

Vector3 operator*(const Vector3& lhs, const float rhs);
Vector3 operator/(const Vector3& lhs, const float rhs);
Vector3 operator-(const Vector3& lhs, const float rhs);
Vector3 operator+(const Vector3& lhs, const float rhs);
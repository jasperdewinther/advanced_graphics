#include "precomp.h"
#include "Vector.h"



Vector3::Vector3(float elem1, float elem2, float elem3){
	data[0] = elem1;
	data[1] = elem2;
	data[2] = elem3;
}

bool Vector3::operator==(const Vector3& rhs) const
{
	return (data[0] == rhs.data[0] && data[1] == rhs.data[1] && data[2] == rhs.data[2]);
}

bool Vector3::operator!=(const Vector3& rhs) const
{
	return !(*this == rhs);
}

Vector3 Vector3::operator*(const Vector3& rhs) const
{
	return Vector3(data[0] * rhs.data[0], data[1] * rhs.data[1], data[2] * rhs.data[2]);
}

Vector3 Vector3::operator/(const Vector3& rhs) const
{
	return Vector3(data[0] / rhs.data[0], data[1] / rhs.data[1], data[2] / rhs.data[2]);
}

Vector3 Vector3::operator-(const Vector3& rhs) const
{
	return Vector3(data[0] - rhs.data[0], data[1] - rhs.data[1], data[2] - rhs.data[2]);
}

Vector3 Vector3::operator+(const Vector3& rhs) const
{
	return Vector3(data[0] + rhs.data[0], data[1] + rhs.data[1], data[2] + rhs.data[2]);
}

float Vector3::dot(const Vector3& rhs) const
{
	return data[0] * rhs.data[0] + data[1] * rhs.data[1] + data[2] * rhs.data[2];
}

Vector3 Vector3::cross(const Vector3& rhs) const
{
	return Vector3(data[1]*rhs.data[2] - data[2]*rhs.data[1], data[2] * rhs.data[0] - data[0] * rhs.data[2], data[0] * rhs.data[1] - data[1] * rhs.data[0]);
}

void Vector3::normalize()
{
	float length = sqrt(this->dot(*this));
	data[0] /= length;
	data[1] /= length;
	data[2] /= length;
}

float& Vector3::operator[](int rhs)
{
	return data[rhs];
}

std::string Vector3::to_string() const {
	return "[" + std::to_string(data[0]) + ", " + std::to_string(data[1]) + ", " + std::to_string(data[2]) + "]";
}

Vector3 operator*(const Vector3& lhs, const float rhs) {
	return Vector3(lhs.data[0] * rhs, lhs.data[1] * rhs, lhs.data[2] * rhs);
}

Vector3 operator/(const Vector3& lhs, const float rhs) {
	return Vector3(lhs.data[0] / rhs, lhs.data[1] / rhs, lhs.data[2] / rhs);
}

Vector3 operator-(const Vector3& lhs, const float rhs) {
	return Vector3(lhs.data[0] - rhs, lhs.data[1] - rhs, lhs.data[2] - rhs);
}

Vector3 operator+(const Vector3& lhs, const float rhs) {
	return Vector3(lhs.data[0] + rhs, lhs.data[1] + rhs, lhs.data[2] + rhs);
}

Vector3 operator*(const float lhs, const Vector3& rhs) {
	return Vector3(lhs * rhs.data[0], lhs * rhs.data[1], lhs * rhs.data[2]);
}

Vector3 operator/(const float lhs, const Vector3& rhs) {
	return Vector3(lhs / rhs.data[0], lhs / rhs.data[1], lhs / rhs.data[2]);
}

Vector3 operator-(const float lhs, const Vector3& rhs) {
	return Vector3(lhs - rhs.data[0], lhs - rhs.data[1], lhs - rhs.data[2]);
}

Vector3 operator+(const float lhs, const Vector3& rhs) {
	return Vector3(lhs + rhs.data[0], lhs + rhs.data[1], lhs + rhs.data[2]);
}
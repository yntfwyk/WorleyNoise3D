#pragma once

#include <cmath>
#include <vector>
#include <random>
#include <limits>

namespace worley
{
	namespace details
	{
		template<typename T>
		struct Vector3
		{
			T x;
			T y;
			T z;
		};

		using Vector3f = Vector3<float>;
		using Vector3i = Vector3<int>;

		[[nodiscard]] float distance(Vector3f v1, Vector3f v2)
		{
			const float dx = v2.x - v1.x;
			const float dy = v2.y - v1.y;
			const float dz = v2.z - v1.z;
			return std::sqrtf(dx * dx + dy * dy + dz * dz);
		}

		[[nodiscard]] Vector3f toVector3f(Vector3i v)
		{
			return { static_cast<float>(v.x), static_cast<float>(v.y), static_cast<float>(v.z) };
		}

		[[nodiscard]] std::vector<Vector3i> generateFeaturePoints(std::size_t size, std::size_t gridSize)
		{
			std::random_device rd;
			std::mt19937 engine(rd());

			const int sizeOfEachCell = static_cast<int>(size / gridSize);
			std::uniform_int_distribution dist(0, sizeOfEachCell - 1);

			std::vector<Vector3i> featurePoints;
			featurePoints.reserve(size * size * size);

			for (std::size_t i = 0; i < gridSize; i++)
			{
				for (std::size_t j = 0; j < gridSize; j++)
				{
					for (std::size_t k = 0; k < gridSize; k++)
					{
						featurePoints.emplace_back(static_cast<int>(i * sizeOfEachCell + dist(engine)),
							static_cast<int>(j * sizeOfEachCell + dist(engine)),
							static_cast<int>(k * sizeOfEachCell + dist(engine)));
					}
				}
			}

			return featurePoints;
		}

		[[nodiscard]] float mapValue(float value, float oldMin, float oldMax, float newMin, float newMax)
		{
			return newMin + (value - oldMin) * (newMax - newMin) / (oldMax - oldMin);
		}
	}

	/// @brief Generates inverted worley noise, points closer to feature points are lighter and the points
	/// farther to the feature points are darker. Each grid cell has exactly one feature point randomly placed
	/// local to that grid cell.
	/// @param size Number of pixels in each dimension
	/// @param gridSize Number of cells in each dimension
	/// @return An floating point array of noise values
	[[nodiscard]] std::vector<float> worleyNoise3D(std::size_t size, std::size_t gridSize)
	{
		auto toLinearIndex = [size](std::size_t w, std::size_t h, std::size_t d)
			{
				return w + h * size + d * size * size;
			};

		std::vector<float> resultNoise(size * size * size);
		const float sizeOfEachCell = size / gridSize;
		const float maxCellDistance = std::sqrtf(sizeOfEachCell * sizeOfEachCell * 3.f);
		auto featurePoints = details::generateFeaturePoints(size, gridSize);

		for (std::size_t x = 0; x < size; x++)
		{
			for (std::size_t y = 0; y < size; y++)
			{
				for (std::size_t z = 0; z < size; z++)
				{
					float min = std::numeric_limits<float>::max();
					for (auto point : featurePoints)
					{
						const float d = distance(details::toVector3f(point), details::Vector3f(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)));

						if (d < min)
							min = d;
					}
					resultNoise[toLinearIndex(x, y, z)] = 1.f - details::mapValue(min, 0.f, maxCellDistance, 0.f, 1.f);
				}
			}
		}

		return resultNoise;
	}
}

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace VoxReader
{
	using uint8 = std::uint8_t;
	using uint32 = std::uint32_t;
	using sint32 = std::int32_t;
	using usize = std::size_t;

	union Matrix
	{
		float data[16]{};
		float cells[4][4];
	};

	struct Vector
	{
		float x{ 0.0f };
		float y{ 0.0f };
		float z{ 0.0f };
	};

	class Transform
	{
	public:
		Transform(const Vector& position, uint8 rotation);

		[[nodiscard]] Vector& GetPosition() { return reinterpret_cast<Vector&>(matrix.cells[3][0]); }
		[[nodiscard]] const Vector& GetPosition() const { return reinterpret_cast<const Vector&>(matrix.cells[3][0]); }
		void MakeRelativeTo(const Matrix& parent_matrix);

		std::string name;
		Matrix matrix;
		bool hidden{ false };

		Vector local_position;
	};

	struct Model
	{
		struct Size
		{
			uint32 x;
			uint32 y;
			uint32 z;
		} size;

		std::vector<uint8> voxel_data;
	};

	struct Instance
	{
		Instance(const uint32 transform_index, const uint32 model_index) : transform_index{ transform_index }, model_index{ model_index } {}
		uint32 transform_index;
		uint32 model_index;
	};

	struct Group
	{
		Group(const uint32 transform_index, std::vector<uint32>&& child_transform_indices) : transform_index{ transform_index }, child_transform_indices{ std::move(child_transform_indices) } {}
		uint32 transform_index;
		std::vector<uint32> child_transform_indices;
	};

	struct Material
	{
		enum Type : uint8
		{
			DIFFUSE,
			METAL,
			EMIT,
			GLASS,
			BLEND,
			CLOUD,
		} type{ DIFFUSE };

		enum MediaType : uint8
		{
			ABSORB,
			SCATTER,
			EMISSIVE,
			SUBSURFACE_SCATTERING
		} media_type{ ABSORB };

		// Range [0.0 ~ 100.0].
		float roughness{ 10.0f };
		// Range [1.0 ~ 3.0].
		float ior{ 1.3f };
		// Range [1.0 ~ 2.0].
		float specular{ 1.0f };
		// Range [0.0 ~ 100.0].
		float emission{ 0.0f };
		// Range [0 ~ 4].
		uint8 power{ 0 };
		// Range [0.0 ~ 100.0] (was called "glow" before version 200).
		float ldr{ 0.0f };
		// Range [0.0 ~ 100.0].
		float metallic{ 0.0f };
		// Range [0.0 ~ 100.0] (was called "glass" before version 200).
		float transparency{ 0.0f };
		// Range [0.0 ~ 100.0] (was called "attenuate" before version 200).
		float density{ 50.0f };
		// Range [-0.9 ~ 0.9].
		float phase{ 0.0f };
	};

	// 1 byte per component RGBA color struct.
	struct Color
	{
		uint8 r{ 0 };
		uint8 g{ 0 };
		uint8 b{ 0 };
		uint8 a{ 0 };
	};

	class Scene
	{
	public:
		Scene() = default;
		Scene(const void* data, usize data_size);

		// Converts a palette color (uint32) into its rgba components (1 byte per component).
		[[nodiscard]] Color PaletteToColor(const usize i) const
		{
			const uint32 color = palette[i];
			return Color
			{
				static_cast<uint8>(color & 0xFF),
				static_cast<uint8>((color >> 8) & 0xFF),
				static_cast<uint8>((color >> 16) & 0xFF),
				static_cast<uint8>((color >> 24) & 0xFF),
			};
		}

		std::vector<Transform> transforms;
		std::vector<Model> models;

		std::vector<Instance> instances;
		std::vector<Group> groups;

		// Colors with format RGBA (index 0 means the voxel is empty).
		uint32 palette[256]{};
		// Material palette.
		Material materials[256]{};

	private:
		uint32 ParseSceneGraph(const void*& data, const uint32 parent_transform_index = UINT32_MAX);
	};


}

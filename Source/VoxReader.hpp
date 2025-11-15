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
		float data[16];
		float cells[4][4]
		{
			{ 1.0f, 0.0f, 0.0f, 0.0f },
			{ 0.0f, 1.0f, 0.0f, 0.0f },
			{ 0.0f, 0.0f, 1.0f, 0.0f },
			{ 0.0f, 0.0f, 0.0f, 1.0f }
		};
	};

	struct Vector
	{
		float x{ 0.0f };
		float y{ 0.0f };
		float z{ 0.0f };
	};

	struct Quaternion
	{
		float x{ 0.0f };
		float y{ 0.0f };
		float z{ 0.0f };
		float w{ 1.0f }; 
	};

	struct ReaderSettings
	{
		enum CoordSystem : sint32
		{
			LH = -1,
			RH = 1,

			Y_UP = 0,
			Z_UP = 1
		};

		// Set the coordinate system to transform the transforms and voxel data to, this will automatically flip the voxel data and the transform data.
		void SetCoordinateSystem(CoordSystem handedness = RH, CoordSystem up_axis = Z_UP);

		// Custom voxel scale for adjusting the positions.
		Vector voxel_scale{ 1.0f, 1.0f, 1.0f };
		// Calculate the local quaternion rotation from the rotation matrix (can be expensive and unnecessary).
		bool calculate_local_rotation{ true };
		// Add half voxel offsets to instance transforms, this fixes alignment issues with odd numbered voxel object scales.
		bool add_voxel_offsets{ true };
		// Avoid instance transforms with negative scale by creating an inverted duplicate of the voxel model it uses.
		bool avoid_negative_scale{ true };

		// Internal use for converting coordinate systems. Use ReadSettings::SetCoordinateSystem() to generate them.
		Matrix coord_system_matrix{};
		Matrix inverse_coord_system_matrix{};
		bool flipped_handedness{ false };
		bool flipped_up_axis{ false };
	};

	class Transform
	{
	public:
		Transform(const Vector& position, uint8 rotation, const ReaderSettings& reader_settings);

		[[nodiscard]] Vector& GetPosition() { return reinterpret_cast<Vector&>(matrix.cells[3][0]); }
		[[nodiscard]] const Vector& GetPosition() const { return reinterpret_cast<const Vector&>(matrix.cells[3][0]); }

		std::string name;
		Matrix matrix{};
		bool hidden{ false };

		Vector local_position{};
		Quaternion local_rotation{};
	};

	struct Model
	{
		struct Size
		{
			uint32 x;
			uint32 y;
			uint32 z;
		};

		Model() = default;
		Model(const Size& size, std::vector<uint8>&& voxel_data) : size{size}, voxel_data{std::move(voxel_data)} {}

		Size size;
		std::vector<uint8> voxel_data;
	};

	struct Instance
	{
		Instance() = default;
		Instance(const uint32 transform_index, const uint32 model_index) : transform_index{ transform_index }, model_index{ model_index } {}

		uint32 transform_index;
		uint32 model_index;
	};

	struct Group
	{
		Group() = default;
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
		Scene(const void* data, usize data_size, const ReaderSettings& reader_settings = {});

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
		uint32 ParseSceneGraph(const void*& data, const ReaderSettings& reader_settings, uint32 parent_transform_index = UINT32_MAX);
	};
}

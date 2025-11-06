/*
* Info used to parse the .vox file format:
* - https://github.com/ephtracy/voxel-model/blob/master/MagicaVoxel-file-format-vox.txt
* - https://github.com/ephtracy/voxel-model/blob/master/MagicaVoxel-file-format-vox-extension.txt
*/

#include "VoxReader.hpp"

#include <map>
#include <array>
#include <cassert>
#include <charconv>
#include <string_view>

namespace VoxReader
{
	namespace
	{
		constexpr uint32 default_palette[256]
		{
			0x00000000, 0xFFFFFFFF, 0xFFCCFFFF, 0xFF99FFFF, 0xFF66FFFF, 0xFF33FFFF, 0xFF00FFFF, 0xFFFFCCFF,
			0xFFCCCCFF, 0xFF99CCFF, 0xFF66CCFF, 0xFF33CCFF, 0xFF00CCFF, 0xFFFF99FF, 0xFFCC99FF, 0xFF9999FF,
			0xFF6699FF, 0xFF3399FF, 0xFF0099FF, 0xFFFF66FF, 0xFFCC66FF, 0xFF9966FF, 0xFF6666FF, 0xFF3366FF,
			0xFF0066FF, 0xFFFF33FF, 0xFFCC33FF, 0xFF9933FF, 0xFF6633FF, 0xFF3333FF, 0xFF0033FF, 0xFFFF00FF,
			0xFFCC00FF, 0xFF9900FF, 0xFF6600FF, 0xFF3300FF, 0xFF0000FF, 0xFFFFFFCC, 0xFFCCFFCC, 0xFF99FFCC,
			0xFF66FFCC, 0xFF33FFCC, 0xFF00FFCC, 0xFFFFCCCC, 0xFFCCCCCC, 0xFF99CCCC, 0xFF66CCCC, 0xFF33CCCC,
			0xFF00CCCC, 0xFFFF99CC, 0xFFCC99CC, 0xFF9999CC, 0xFF6699CC, 0xFF3399CC, 0xFF0099CC, 0xFFFF66CC,
			0xFFCC66CC, 0xFF9966CC, 0xFF6666CC, 0xFF3366CC, 0xFF0066CC, 0xFFFF33CC, 0xFFCC33CC, 0xFF9933CC,
			0xFF6633CC, 0xFF3333CC, 0xFF0033CC, 0xFFFF00CC, 0xFFCC00CC, 0xFF9900CC, 0xFF6600CC, 0xFF3300CC,
			0xFF0000CC, 0xFFFFFF99, 0xFFCCFF99, 0xFF99FF99, 0xFF66FF99, 0xFF33FF99, 0xFF00FF99, 0xFFFFCC99,
			0xFFCCCC99, 0xFF99CC99, 0xFF66CC99, 0xFF33CC99, 0xFF00CC99, 0xFFFF9999, 0xFFCC9999, 0xFF999999,
			0xFF669999, 0xFF339999, 0xFF009999, 0xFFFF6699, 0xFFCC6699, 0xFF996699, 0xFF666699, 0xFF336699,
			0xFF006699, 0xFFFF3399, 0xFFCC3399, 0xFF993399, 0xFF663399, 0xFF333399, 0xFF003399, 0xFFFF0099,
			0xFFCC0099, 0xFF990099, 0xFF660099, 0xFF330099, 0xFF000099, 0xFFFFFF66, 0xFFCCFF66, 0xFF99FF66,
			0xFF66FF66, 0xFF33FF66, 0xFF00FF66, 0xFFFFCC66, 0xFFCCCC66, 0xFF99CC66, 0xFF66CC66, 0xFF33CC66,
			0xFF00CC66, 0xFFFF9966, 0xFFCC9966, 0xFF999966, 0xFF669966, 0xFF339966, 0xFF009966, 0xFFFF6666,
			0xFFCC6666, 0xFF996666, 0xFF666666, 0xFF336666, 0xFF006666, 0xFFFF3366, 0xFFCC3366, 0xFF993366,
			0xFF663366, 0xFF333366, 0xFF003366, 0xFFFF0066, 0xFFCC0066, 0xFF990066, 0xFF660066, 0xFF330066,
			0xFF000066, 0xFFFFFF33, 0xFFCCFF33, 0xFF99FF33, 0xFF66FF33, 0xFF33FF33, 0xFF00FF33, 0xFFFFCC33,
			0xFFCCCC33, 0xFF99CC33, 0xFF66CC33, 0xFF33CC33, 0xFF00CC33, 0xFFFF9933, 0xFFCC9933, 0xFF999933,
			0xFF669933, 0xFF339933, 0xFF009933, 0xFFFF6633, 0xFFCC6633, 0xFF996633, 0xFF666633, 0xFF336633,
			0xFF006633, 0xFFFF3333, 0xFFCC3333, 0xFF993333, 0xFF663333, 0xFF333333, 0xFF003333, 0xFFFF0033,
			0xFFCC0033, 0xFF990033, 0xFF660033, 0xFF330033, 0xFF000033, 0xFFFFFF00, 0xFFCCFF00, 0xFF99FF00,
			0xFF66FF00, 0xFF33FF00, 0xFF00FF00, 0xFFFFCC00, 0xFFCCCC00, 0xFF99CC00, 0xFF66CC00, 0xFF33CC00,
			0xFF00CC00, 0xFFFF9900, 0xFFCC9900, 0xFF999900, 0xFF669900, 0xFF339900, 0xFF009900, 0xFFFF6600,
			0xFFCC6600, 0xFF996600, 0xFF666600, 0xFF336600, 0xFF006600, 0xFFFF3300, 0xFFCC3300, 0xFF993300,
			0xFF663300, 0xFF333300, 0xFF003300, 0xFFFF0000, 0xFFCC0000, 0xFF990000, 0xFF660000, 0xFF330000,
			0xFF0000EE, 0xFF0000DD, 0xFF0000BB, 0xFF0000AA, 0xFF000088, 0xFF000077, 0xFF000055, 0xFF000044,
			0xFF000022, 0xFF000011, 0xFF00EE00, 0xFF00DD00, 0xFF00BB00, 0xFF00AA00, 0xFF008800, 0xFF007700,
			0xFF005500, 0xFF004400, 0xFF002200, 0xFF001100, 0xFFEE0000, 0xFFDD0000, 0xFFBB0000, 0xFFAA0000,
			0xFF880000, 0xFF770000, 0xFF550000, 0xFF440000, 0xFF220000, 0xFF110000, 0xFFEEEEEE, 0xFFDDDDDD,
			0xFFBBBBBB, 0xFFAAAAAA, 0xFF888888, 0xFF777777, 0xFF555555, 0xFF444444, 0xFF222222, 0xFF111111
		};

		const std::map<std::string_view, Material::Type> type_mapping
		{
			{"_diffuse", Material::DIFFUSE},
			{"_metal", Material::METAL},
			{"_emit", Material::EMIT},
			{"_glass", Material::GLASS},
			{"_blend", Material::BLEND},
			{"_cloud", Material::CLOUD}
		};

		const std::map<std::string_view, Material::MediaType> media_type_mapping
		{
			{"_absorb", Material::ABSORB},
			{"_scatter", Material::SCATTER},
			{"_emit", Material::EMISSIVE},
			{"_sss", Material::SUBSURFACE_SCATTERING},
		};

		struct VoxHeader
		{
			char id[4]{ "" };
			uint32 : 32; // Version number of the file, either 150 or 200, the library is set up to handle both versions simultaneously so no need to check the version.
		};

		struct ChunkHeader
		{
			char id[4]{};
			uint32 content_size{ 0 }; // Number of chunk content bytes.
			uint32 : 32; // Number of children chunk bytes, not used but padded in struct.
		};

		template <typename Type>
		class ArrayView
		{
		public:
			operator std::vector<Type>() const { return { data, data + size }; }

			const Type& operator[](const usize i) const { return data[i]; }

			[[nodiscard]] const Type* begin() const { return data; }
			[[nodiscard]] const Type* end() const { return data + size; }

			const Type* data;
			uint32 size;
		};

		void SkipData(const void*& pointer, const usize byte_count)
		{
			pointer = static_cast<const uint8*>(pointer) + byte_count;
		}

		template <typename Type>
		const Type& ReadData(const void*& pointer)
		{
			const Type& data = *static_cast<const Type*>(pointer);
			SkipData(pointer, sizeof(Type));

			return data;
		}

		template <typename Type>
		ArrayView<Type> ReadArray(const void*& pointer)
		{
			const uint32 view_size = ReadData<uint32>(pointer);
			const auto* view = static_cast<const Type*>(pointer);

			pointer = view + view_size;
			return ArrayView<Type>{ view, view_size };
		}

		std::string_view ReadString(const void*& pointer)
		{
			const uint32 string_size = ReadData<uint32>(pointer);
			const auto* string = static_cast<const char*>(pointer);

			pointer = string + string_size;
			return { string, string_size };
		}

		using StringMap = std::map<std::string_view, std::string_view>;

		StringMap ReadDict(const void*& pointer)
		{
			const uint32 map_size = ReadData<uint32>(pointer);

			StringMap map;
			for (usize i = 0; i < map_size; i++)
			{
				const std::string_view key = ReadString(pointer);
				const std::string_view value = ReadString(pointer);
				map[key] = value;
			}

			return map;
		}

		template <typename Type>
		Type StringViewToData(const std::string_view& string_view)
		{
			Type value;
			std::from_chars(string_view.data(), string_view.data() + string_view.size(), value);
			return value;
		}

		// Slightly cleaner interface for the std::map::find function.
		const std::string_view* MapFind(const StringMap& map, const std::string_view& key)
		{
			const auto& iterator = map.find(key);
			if (iterator == map.end()) return nullptr;

			return &iterator->second;
		}

		// Parse a std::string_view containing 3 values separated by spaces (needed for the nTRN chunk's frame attribute translation).
		std::array<std::string_view, 3> ParseViewVector(const std::string_view& vector_view)
		{
			const usize second_start = vector_view.find(' ') + 1;
			const usize third_start = vector_view.rfind(' ') + 1;

			const std::string_view first{ vector_view.data(), second_start - 1 };
			const std::string_view second{ vector_view.data() + second_start, third_start - second_start - 1 };
			const std::string_view third{ vector_view.data() + third_start, vector_view.size() - third_start };

			return { first, second, third };
		}

	}

	Transform::Transform(const Vector& position, const uint8 rotation)
	{
		matrix.cells[3][0] = position.x;
		matrix.cells[3][1] = position.y;
		matrix.cells[3][2] = position.z;
		matrix.cells[3][3] = 1.0f;

		if (rotation != 0)
		{
			const uint32 index_x = rotation & 0b11;
			matrix.cells[index_x][0] = rotation & (1 << 4) ? -1.0f : 1.0f;

			const uint32 index_y = (rotation >> 2) & 0b11;
			matrix.cells[index_y][1] = rotation & (1 << 5) ? -1.0f : 1.0f;

			const uint32 index_z = 3 - (index_x + index_y);
			matrix.cells[index_z][2] = rotation & (1 << 6) ? -1.0f : 1.0f;
		}
		else
		{
			matrix.cells[0][0] = 1.0f;
			matrix.cells[1][1] = 1.0f;
			matrix.cells[2][2] = 1.0f;
		}

		local_position = position;
	}

	void Transform::MakeRelativeTo(const Matrix& parent_matrix)
	{
		const Matrix old_matrix = matrix;
		for (usize row = 0; row < 4; row++)
		{
			for (usize column = 0; column < 4; column++)
			{
				matrix.cells[row][column] = 0;

				matrix.cells[row][column] += old_matrix.cells[row][0] * parent_matrix.cells[0][column];
				matrix.cells[row][column] += old_matrix.cells[row][1] * parent_matrix.cells[1][column];
				matrix.cells[row][column] += old_matrix.cells[row][2] * parent_matrix.cells[2][column];
				matrix.cells[row][column] += old_matrix.cells[row][3] * parent_matrix.cells[3][column];
			}
		}
	}

	Scene::Scene(const void* data, const usize data_size)
	{
		const void* const data_end = static_cast<const uint8*>(data) + data_size;

		const VoxHeader& file_header = ReadData<VoxHeader>(data); // Skip the voxel 
		assert(std::string_view(file_header.id, 4) == "VOX " && "Voxel file is invalid, header not valid!"); // Check that the file is valid using the header id.

		bool has_palette = false;

		SkipData(data, sizeof(ChunkHeader)); // Skip the root chunk (only has a header).
		while (data < data_end)
		{
			// While we aren't at the end of the file, we try to process a new chunk.
			const ChunkHeader& next_chunk = ReadData<ChunkHeader>(data);

			std::string_view chunk_id{ next_chunk.id, 4 };
			if (chunk_id == "SIZE")
			{
				Model& model = models.emplace_back();

				model.size = ReadData<Model::Size>(data);

				const uint32 voxel_count = model.size.x * model.size.y * model.size.z;
				model.voxel_data.resize(voxel_count, 0);

				SkipData(data, sizeof(ChunkHeader)); // Skip the header for the XYZI chunk since it's guaranteed to be after the SIZE chunk.

				const ArrayView<uint32> packed_voxel_data = ReadArray<uint32>(data);
				for (const uint32 voxel : packed_voxel_data)
				{
					const uint8 x = voxel & 0xFF;
					const uint8 y = (voxel >> 8) & 0xFF;
					const uint8 z = (voxel >> 16) & 0xFF;
					const uint32 index = x + (y * model.size.x) + (z * model.size.x * model.size.y);

					model.voxel_data[index] = voxel >> 24;
				}
			}
			else if (chunk_id == "RGBA")
			{
				// Read the 255 colors from the palette and copy them to the range [1 ~ 255] in the scene's palette (palette index 0 is skipped since it represents the absence of a voxel).
				std::memcpy(&palette[1], data, 255 * sizeof(uint32));
				SkipData(data, next_chunk.content_size);

				has_palette = true;
			}
			else if (chunk_id == "nTRN")
			{
				// We hierarchically parse the nTRN chunks, so we can assume that this case will only be triggered once.

				// First nTRN chunk is the root transform, which we can skip processing.
				SkipData(data, next_chunk.content_size);

				// After the root nTRN chunk we get the root nGRP chunk, if it has no children there are no more transforms in the file.
				SkipData(data, sizeof(ChunkHeader)); // Skip over the header, we know that it's a nGRP chunk.

				SkipData(data, sizeof(uint32)); // Skip over the node id.
				ReadDict(data); // Ignore the node attributes.

				// Get the root children, and for each child parse its children and so on.
				const ArrayView<uint32> root_children = ReadArray<uint32>(data);
				for (uint32 i = 0; i < root_children.size; i++)
				{
					SkipData(data, sizeof(ChunkHeader)); // Skip the child nTRN node's header.
					ParseSceneGraph(data);
				}

			}
			else if (chunk_id == "MATL")
			{
				const uint32 material_id = ReadData<uint32>(data);
				const StringMap material_properties = ReadDict(data);

				Material& material = materials[material_id];

				const std::string_view* material_type = MapFind(material_properties, "_type");
				if (material_type != nullptr)
				{
					material.type = type_mapping.at(*material_type);

					const std::string_view* media_type = MapFind(material_properties, "_media_type");
					if (media_type != nullptr) material.media_type = media_type_mapping.at(*media_type);

					const std::string_view* roughness = MapFind(material_properties, "_rough");
					if (roughness != nullptr) material.roughness = StringViewToData<float>(*roughness) * 100.0f; // Range is incorrect [0.0 ~ 1.0], multiply by 100 to compensate.

					// _ir seams to be the new name for ior since version 200.
					const std::string_view* ior = MapFind(material_properties, "_ri");
					if (ior != nullptr)
					{
						material.ior = StringViewToData<float>(*ior);
					}
					else
					{
						// Support the old name of ior as well.
						ior = MapFind(material_properties, "_ior");
						if (ior != nullptr) material.ior = StringViewToData<float>(*ior) + 1.0f; // Range is incorrect [0.0 ~ 2.0], add 1 to compensate.
					}

					// _sp is the new name for _spec since version 200.
					const std::string_view* specular = MapFind(material_properties, "_sp");
					if (specular != nullptr)
					{
						material.specular = StringViewToData<float>(*specular);
					}
					else
					{
						// Support the old name of ior as well.
						specular = MapFind(material_properties, "_spec");
						if (specular != nullptr) material.specular = StringViewToData<float>(*specular) + 1.0f; // Range is incorrect [0.0 ~ 1.0], add 1 to compensate.
					}

					// _emit was _weight before version 200 (just like _trans).
					const std::string_view* emission = MapFind(material_properties, "_emit");
					if (emission != nullptr)
					{
						material.emission = StringViewToData<float>(*emission) * 100.0f; // Range is incorrect [0.0 ~ 2.0], add 1 to compensate.
					}
					else
					{
						// Support the old name of emission as well.
						emission = MapFind(material_properties, "_weight");
						if (specular != nullptr) material.emission = StringViewToData<float>(*emission) * 100.0f; // Range is incorrect [0.0 ~ 1.0], multiply by 100 to compensate.
					}

					const std::string_view* power = MapFind(material_properties, "_flux");
					if (power != nullptr) material.power = StringViewToData<uint8>(*power);

					// _ldr was _glow before version 200.
					const std::string_view* ldr = MapFind(material_properties, "_ldr");
					if (ldr != nullptr)
					{
						material.ldr = StringViewToData<float>(*ldr) * 100.0f; // Range is incorrect [0.0 ~ 1.0], multiply by 100 to compensate.
					}
					else
					{
						ldr = MapFind(material_properties, "_glow");
						if (ldr != nullptr) material.ldr = StringViewToData<float>(*ldr) * 100.0f; // Range is incorrect [0.0 ~ 1.0], multiply by 100 to compensate.
					}

					const std::string_view* metallic = MapFind(material_properties, "_metal");
					if (metallic != nullptr) material.metallic = StringViewToData<float>(*metallic) * 100.0f; // Range is incorrect [0.0 ~ 1.0], multiply by 100 to compensate.

					// _alpha and _trans seam to be the same value always? We'll ignore _alpha since I'm not sure how to use it. 
					// _trans was _weight before version 200 (just like _emit).
					const std::string_view* transparency = MapFind(material_properties, "_trans");
					if (transparency != nullptr)
					{
						material.transparency = StringViewToData<float>(*transparency) * 100.0f; // Range is incorrect [0.0 ~ 1.0], multiply by 100 to compensate.
					}
					else
					{
						transparency = MapFind(material_properties, "_weight");
						if (transparency != nullptr) material.transparency = StringViewToData<float>(*transparency) * 100.0f; // Range is incorrect [0.0 ~ 1.0], multiply by 100 to compensate.
					}

					// _d was _att before version 200.
					const std::string_view* density = MapFind(material_properties, "_d");
					if (density != nullptr)
					{
						material.density = StringViewToData<float>(*density) * 1000.0f; // Range is incorrect [0.0 ~ 0.1]????, multiply by 1000 to compensate.
					}
					else
					{
						density = MapFind(material_properties, "_att");
						if (density != nullptr) material.density = StringViewToData<float>(*density) * 100.0f; // Range is incorrect [0.0 ~ 1.0], multiply by 100 to compensate.
					}

					const std::string_view* phase = MapFind(material_properties, "_g");
					if (phase != nullptr) material.phase = StringViewToData<float>(*phase);
				}
			}
			else // Skip unimplemented chunks.
			{
				// Unimplemented: IMAP, rCAM, rOBJ, NOTE, LAYR, MATT (deprecated, should be supported for compatibility), PACK.
				SkipData(data, next_chunk.content_size);
			}
		}

		if (!has_palette)
		{
			// If no palette was included in the file, copy the default palette.
			std::memcpy(palette, default_palette, sizeof(default_palette));
		}
	}

	uint32 Scene::ParseSceneGraph(const void*& data, const uint32 parent_transform_index)
	{
		SkipData(data, sizeof(uint32)); // Skip transform id.
		const StringMap node_attributes = ReadDict(data); // Transform node's attributes (name, hidden).

		ReadData<uint32>(data); // Skip child node id.
		assert(ReadData<sint32>(data) == -1 && "Invalid voxel file, reserved id in transform isn't -1!"); // Read reserved id.

		SkipData(data, sizeof(uint32)); // Skip layer ID.
		const uint32 frame_count = ReadData<uint32>(data);
		assert(frame_count != 0 && "Invalid voxel file, voxel instance frame count is 0!"); // Double check frame count for validity, more than 1 frame isn't supported yet.

		StringMap transform_attributes;
		for (usize i = 0; i < frame_count; i++)
		{
			const StringMap frame_attributes = ReadDict(data);

			if (i != 0) continue; // Skip processing all but the first frame.

			transform_attributes = frame_attributes;
		}

		Vector position{};
		const std::string_view* translation = MapFind(transform_attributes, "_t");
		if (translation != nullptr)
		{
			// Split the string into the xyz value strings.
			const std::array<std::string_view, 3> translations = ParseViewVector(*translation);

			// Technically the strings values are in sint32 format, but we can directly interpret them as float values as well.
			position.x = StringViewToData<float>(translations.at(0));
			position.y = StringViewToData<float>(translations.at(1));
			position.z = StringViewToData<float>(translations.at(2));
		}

		uint8 rotation = 0;
		const std::string_view* rotation_view = MapFind(transform_attributes, "_r");
		if (rotation_view != nullptr)
		{
			rotation = StringViewToData<uint8>(*rotation_view);
		}

		const uint32 transform_index = static_cast<uint32>(transforms.size());
		Transform& transform = transforms.emplace_back(position, rotation);
		if (parent_transform_index != UINT32_MAX)
		{
			transform.MakeRelativeTo(transforms[parent_transform_index].matrix);
		}

		const std::string_view* name = MapFind(node_attributes, "_name");
		if (name != nullptr) transform.name = *name;

		const std::string_view* hidden = MapFind(node_attributes, "_hidden");
		if (hidden != nullptr) transform.hidden = (StringViewToData<uint8>(*hidden) != 0);

		// Get the next chunk, guaranteed to be either nGRP or nSHP.
		const ChunkHeader& next_node_chunk = ReadData<ChunkHeader>(data);
		std::string_view next_chunk_id{ next_node_chunk.id, 4 };
		if (next_chunk_id == "nGRP")
		{
			SkipData(data, sizeof(uint32)); // Skip group node id.
			ReadDict(data); // Group node attributes, we can ignore these.

			const usize group_index = groups.size();
			const Group& group = groups.emplace_back(transform_index, ReadArray<uint32>(data));

			const usize children_count = group.child_transform_indices.size();
			for (usize i = 0; i < children_count; i++)
			{
				SkipData(data, sizeof(ChunkHeader)); // Skip the next nTRN chunk header, since we know it'll be next.
				groups[group_index].child_transform_indices[i] = ParseSceneGraph(data, transform_index);
			}
		}
		else
		{
			SkipData(data, sizeof(uint32)); // Skip shape node id.
			ReadDict(data); // Shape node attributes, we can ignore these.

			const uint32 model_count = ReadData<uint32>(data);
			assert(model_count != 0 && "Invalid voxel file, voxel model count is 0!");

			uint32 instance_model_index = 0;
			for (usize i = 0; i < model_count; i++)
			{
				const uint32 model_index = ReadData<uint32>(data);

				if (i != 0) continue; // Skip processing all but the first model.

				instance_model_index = model_index;

				ReadDict(data); // Model attributes, we can ignore this until implementing animation.
			}

			instances.emplace_back(transform_index, instance_model_index);
		}

		return transform_index;
	}
}

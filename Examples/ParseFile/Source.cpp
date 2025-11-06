#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>

#include "VoxReader.hpp"

namespace
{
	std::string MakeColorString(const std::uint32_t color)
	{
		return "\x1b[48;2;" + std::to_string(color & 0xFF) + ';' + std::to_string((color >> 8) & 0xFF) + ';' + std::to_string((color >> 16) & 0xFF) + "m  ";
	}
}

int main(int, char*)
{
	std::filesystem::path path;
	do
	{
		std::cout << ".vox file path: ";
		std::cin >> path;

	} while (path.empty() || !std::filesystem::exists(path));

	std::ifstream file{ path, std::ios::binary };
	if (!file.is_open()) return 1;

	const std::size_t file_size = std::filesystem::file_size(path);
	std::vector<char> file_buffer(file_size);
	file.read(file_buffer.data(), static_cast<std::streamsize>(file_size));
	file.close();

	VoxReader::Scene voxel_scene{ file_buffer.data(), file_buffer.size() };

	for (const VoxReader::Model& model : voxel_scene.models)
	{
		std::cout << "Model:" << '\n';
		std::cout << "    Size: " << model.size.x << ", " << model.size.y << ", " << model.size.z << '\n';
		std::cout << "    Voxel data size: " << model.voxel_data.size() << '\n';
		std::cout << '\n';
	}

	std::cout << '\n';

	for (const VoxReader::Transform& transform : voxel_scene.transforms)
	{
		std::cout << "Transform:" << '\n';
		std::cout << "    Name: " << transform.name << '\n';
		std::cout << "    Hidden: " << transform.hidden << '\n';
		const VoxReader::Vector& position = transform.GetPosition();
		std::cout << "    World position: " << position.x << ", " << position.y << ", " << position.z << '\n';
		std::cout << "    Local position: " << transform.local_position.x << ", " << transform.local_position.y << ", " << transform.local_position.z << '\n';
		std::cout << '\n';
	}

	std::cout << '\n';

	for (const VoxReader::Instance& instance : voxel_scene.instances)
	{
		std::cout << "Instance:" << '\n';
		std::cout << "    Name: " << voxel_scene.transforms[instance.transform_index].name << '\n';
		std::cout << "    Model index: " << instance.model_index << '\n';
		std::cout << '\n';
	}

	std::cout << '\n';

	for (const VoxReader::Group& group : voxel_scene.groups)
	{
		std::cout << "Group:" << '\n';
		std::cout << "    Name: " << voxel_scene.transforms[group.transform_index].name << '\n';
		std::cout << "    Children: " << '\n';
		for (const std::uint32_t child_transform_index : group.child_transform_indices)
		{
			std::cout << "        Child transform: " << child_transform_index;

			const std::string& child_name = voxel_scene.transforms[child_transform_index].name;
			if (!child_name.empty())
			{
				std::cout << " (" << voxel_scene.transforms[child_transform_index].name << ')';
			}
			std::cout << '\n';
		}
		std::cout << '\n';
	}

	std::cout << '\n';

	std::size_t diffuse_material_count = 0;
	for (std::size_t i = 0; i < 256; i++)
	{
		const VoxReader::Material& material = voxel_scene.materials[i];

		// Skip diffuse materials, no material property info in there.
		if (material.type == VoxReader::Material::DIFFUSE) 
		{
			diffuse_material_count++;
			continue;
		}

		std::cout << "Material:" << '\n';
		std::cout << "    Material index: " << i << '\n';

		std::string_view material_type_string;
		switch (material.type)
		{
		case VoxReader::Material::DIFFUSE:
			break;

		case VoxReader::Material::METAL:
			material_type_string = "Metal";
			break;

		case VoxReader::Material::EMIT:
			material_type_string = "Emissive";
			break;

		case VoxReader::Material::GLASS:
			material_type_string = "Glass";
			break;

		case VoxReader::Material::BLEND:
			material_type_string = "Blend";
			break;

		case VoxReader::Material::CLOUD:
			material_type_string = "Cloud";
			break;
		}
		std::cout << "    Type: ";
		std::cout.write(material_type_string.data(), static_cast<std::streamsize>(material_type_string.size()));
		std::cout << '\n';

		std::cout << "    Roughness: " << material.roughness << '\n';
		std::cout << "    Index or refraction: " << material.ior << '\n';
		std::cout << "    Specular: " << material.specular << '\n';
		std::cout << "    Emission: " << material.emission << '\n';
		std::cout << "    Power: " << material.power << '\n';
		std::cout << "    Ldr: " << material.ldr << '\n';
		std::cout << "    Metallic: " << material.metallic << '\n';
		std::cout << "    Transparency: " << material.transparency << '\n';
		std::cout << "    Density: " << material.density << '\n';
		std::cout << "    Phase: " << material.phase << '\n';

		std::cout << '\n';
	}
	std::cout << "+ diffuse material count: " << diffuse_material_count << '\n';

	std::cout << '\n';

	std::cout << "Palette:" << '\n';
	for (std::size_t y = 31; y < 32; y--)
	{
		for (std::size_t x = 0; x < 8; x++)
		{
			std::size_t index = x + y * 8 + 1;
			std::cout << MakeColorString(voxel_scene.palette[index]);
		}
		std::cout << "\x1b[0m" << '\n';
	}

	return 0;
}
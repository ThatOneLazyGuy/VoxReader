# VoxReader

A MagicaVoxel .vox loader written in C++.


# Details

A voxel loader for MagicaVoxel's .vox file format written in c++.

Currently supported data:
- Models.
- Instances.
- Groups.
- Color palette.
- Materials.

Not yet supported:
- Animations.
- Camera info.
- Old MATT material format.
- Render objects.

When creating a scene and reading a voxel file, a ReaderSettings instance can be given in order to process the parsed data in some ways:
- **voxel_scale:** The scale that the voxels should be at, this is used to scale the transform positions to match with your program's scale.
- **calculate_local_rotation:** When set, calculates the local rotation quaternion from the transform's matrix, otherwise the local_rotation parameter of the transform will be a unit quaternion.
- **add_voxel_offsets:** When set, adds half a voxel_scale of spacing to transforms of instances, this corrects for incorrect spacing caused by odd-numbered voxel model scales.
- **avoid_negative_scale:** When set, duplicates the voxel models for instances that have transforms with a negative scale and flips the order of voxels instead of making the transform's scale negative.
- **SetCoordinateSystem():** This function is used to set the rest of the internally used member variables, and when set to any other values than right-handed z-up (MagicaVoxel's coordinate system) will automatically transform all instance and group transforms to the new coordinate system and will also correctly adjust the voxel model data to the new coordinate system.

# Usage

Simply construct a `VoxReader::Scene` to parse the .vox file's data.
```cpp
const std::filesystem::path file_path{"path/to/vox_file.vox"};

// Read the file's binary data into a buffer.
const std::size_t file_size = std::filesystem::file_size(file_path);
std::ifstream file{ file_path, std::ios::binary };

std::vector<char> file_buffer(file_size);
file.read(file_buffer.data(), file_size);

// Pass the buffer to the constructor of the VoxReader::Scene to parse the file.
VoxReader::Scene voxel_scene{ file_buffer.data(), file_buffer.size() };

// File buffer can be freed after constructing the VoxReader::Scene.
file_buffer.clear();
```

And example parser project is provided, it parses the file and prints out all the parsed data.

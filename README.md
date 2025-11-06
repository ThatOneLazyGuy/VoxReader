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

Support is planned for flags to format the data in a format more suited for you purpose.


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

# CNA glTF View

Desktop C++ application for viewing glTF 2.0 files (`.gltf` and `.glb`).

The project uses the sibling [CNA](../cna) checkout. At runtime, the viewer
converts a model with CNA's `cna_tool_gltf_to_cnj` utility, loads the native
CNJ result through `ContentManager`, and renders it with CNA's 3D API.

The application implementation is developed on the `develop` branch.

## License

The source code in this repository is available under the [MIT License](LICENSE).
CNA is a separate dependency and remains licensed under the Microsoft Public
License (Ms-PL).

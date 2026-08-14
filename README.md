# CNA glTF Viewer

`cna-gltf-viewer` is a desktop C++ application for viewing glTF 2.0 assets
(`.gltf` and `.glb`). It uses the sibling [CNA](../cna) checkout for both the
conversion and rendering paths:

1. CNA's `cna_tool_gltf_to_cnj` converts the input file to CNA's native CNJ
   model format.
2. CNA's `ContentManager` loads the generated CNJ models and their sidecars.
3. The application renders every generated model with an interactive orbit
   camera.

The current viewer intentionally targets the converted model's bind pose.
Animation playback and a scene inspector are planned follow-up work.

## Requirements

- CMake 3.21 or newer
- A C++23 compiler
- A sibling checkout of [CNA](https://github.com/openeggbert/cna) at `../cna`
- A desktop OpenGL environment for the default `OPENGLES3` renderer

## Build

```bash
cmake -S . -B build -G Ninja -DCNA_GRAPHICS_RENDERER=OPENGLES3
cmake --build build --target cna_gltf_viewer --parallel 3
```

`CNA_ROOT_DIR` may be used when CNA is not checked out next to this repository:

```bash
cmake -S . -B build -G Ninja -DCNA_ROOT_DIR=/path/to/cna
```

The build also produces CNA's `cna_tool_gltf_to_cnj` converter. Its absolute
build-time path is embedded in the viewer, so the viewer always invokes the
converter from the same build tree.

## Run

```bash
./build/cna_gltf_viewer path/to/model.glb
```

Optional arguments:

```text
cna-gltf-viewer <model.gltf|model.glb> [--scale <positive-number>] [--output <empty-directory>]
```

`--scale` is passed to CNA's converter and is useful for assets authored in
centimetres (`--scale 0.01`). Without `--output`, generated CNJ files are
written to a new directory below the system temporary directory; the path is
printed at startup. An explicit output directory must be empty, preventing the
viewer from overwriting unrelated converted content.

Controls:

- Left mouse drag: orbit
- Mouse wheel: zoom
- `R`: reset the camera
- `Esc`: exit

## Tests

```bash
ctest --test-dir build --output-on-failure
```

The test suite validates the command-line help path and converts a minimal
glTF triangle through CNA's real converter, checking that a Model CNJ file is
created.

## License

The source code in this repository is licensed under the [MIT License](LICENSE).
CNA is a separate dependency and remains licensed under its own Microsoft
Public License (Ms-PL).

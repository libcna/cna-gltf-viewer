# CNA glTF Viewer

`cna-gltf-viewer` is a desktop C++ application for viewing glTF 2.0 assets
(`.gltf` and `.glb`) through CNA. Its default path converts the source with
`cna_tool_gltf_to_cnj` and loads the resulting CNJ model; `--direct` exercises
CNA's runtime glTF loader without intermediate files.

The viewer applies the rendering policy carried by CNA's imported model:
single- versus double-sided materials, mirrored placements, alpha blending and
per-texture sampler state. A light-less scene receives CNA's default lighting
(except authored unlit materials), skin palettes are applied in the bind pose,
and a selected skeletal or rigid-node animation is looped. CNA's structured
import diagnostics remain visible on screen and are printed in full to stdout.

## Requirements

- CMake 3.21 or newer
- A C++23 compiler
- A sibling checkout of [CNA](https://github.com/openeggbert/cna) at `../cna`
- A desktop OpenGL environment for the default `OPENGLES3` renderer

## Build

```bash
cmake -S . -B build -G Ninja \
  -DCNA_ROOT_DIR=/path/to/cna \
  -DCNA_GRAPHICS_RENDERER=OPENGLES3
cmake --build build --target cna_gltf_viewer --parallel 3
```

`CNA_ROOT_DIR` may be omitted when the configured default points at the desired
CNA checkout.

The build also produces CNA's `cna_tool_gltf_to_cnj` converter. Its absolute
build-time path is embedded in the viewer, so the viewer always invokes the
converter from the same build tree.

## Run

```bash
./build/cna_gltf_viewer path/to/model.glb
```

Optional arguments:

```text
cna-gltf-viewer <model.gltf|model.glb> [options]

--direct            Load glTF directly, without generated CNJ files
--scale <number>    Offline conversion unit scale (default: 1)
--output <dir>      Keep offline CNJ output in an empty directory
--dump-oracle <dir> Write deterministic L2-L5 import evidence to an empty directory
--clip <name>       Select and loop an imported animation clip
--animation-time <seconds>
                     Freeze the selected clip at a reproducible looping time
--camera <name|#n>  Use an imported camera explicitly (requires --direct)
--no-cull           Disable face culling for debugging
--capture <file>    Save the first rendered frame as PNG and exit
--reference-capture Capture a clean 512x512 frame for renderer comparison
```

`--scale` is passed to CNA's converter and is useful for assets authored in
centimetres (`--scale 0.01`). It cannot be combined with `--direct`, whose
runtime path preserves glTF's metre units. Without `--output`, generated CNJ
files are written below the system temporary directory and automatically
removed on exit. An explicit output directory must be empty and is preserved.

`--dump-oracle` invokes the converter embedded in the same viewer build and
writes `oracle.json`: decoded L2 accessors, L3 semantic primitives, independent
expected/CNA L4 world geometry, and exact hexadecimal L5 vertex/index bytes.
It works with both direct and offline rendering, refuses a non-empty directory,
and must use a directory distinct from `--output`; the evidence is always
preserved for diffing against the corpus manifest and golden buffers.

`--animation-time` requires `--clip` and freezes both skeletal and rigid-node
playback at the requested looping time. This makes an animated capture
repeatable across independent processes instead of sampling whichever elapsed
time happens to precede the first rendered frame.

`--capture` is intended for deterministic smoke and visual-comparison runs. It
captures the 800×480 back buffer after the model and diagnostics overlay have
been drawn, then exits. The normal interactive mode continues until `Esc` or
the window close action.

`--reference-capture` requires `--capture` and switches only that capture to a
512×512 back buffer with a transparent clear colour and no diagnostics overlay.
This matches CNA's pinned Khronos-reference protocol and leaves lighting,
materials, culling, camera framing and the model draw path unchanged.
For the default orbit camera it also prints one `CNA_REFERENCE_CAMERA=` JSON
line with the exact target, radius, distance and clipping planes consumed by
the frame, so an independent renderer can reproduce the presentation rig.

The viewer's bounds-framed orbit camera is always the default, even when the
asset contains cameras. `--camera MainCam` (or `--camera '#0'` for an unnamed
or duplicate camera) is the explicit opt-in. It requires `--direct`, uses the
camera node's live bone transform so animation is visible, and rebuilds an
unauthored perspective aspect ratio from the actual viewport.

The 2026-08-14 pinned-reference run passed all 12 selected CNA fixtures. It
also found an EasyGL unlit-origin NaN regression before passing after the CNA
shader fix; the reproducible metric report is committed in CNA as
`docs/gltf-reference-comparison.json`. This subset does not replace the final
14-row viewer retake, whose ≥50 MB fetch-on-demand case remains outstanding.

Controls:

- Left mouse drag: orbit
- Mouse wheel: zoom
- `R`: reset the camera
- `Esc`: exit

## Tests

```bash
ctest --test-dir build --output-on-failure
```

The registered test suite validates the command-line help path and converts a
minimal glTF triangle through CNA's real converter. Release retakes additionally
run the executable under a real renderer, capture representative direct and
offline frames, and compare those images.

## License

The source code in this repository is licensed under the [MIT License](LICENSE).
CNA is a separate dependency and remains licensed under its own Microsoft
Public License (Ms-PL).

# Implementation Plan

## Goal

Provide a small, maintainable C++ desktop viewer for glTF 2.0 files that
exercises both CNA import paths and renders the result without asset-specific
compensations.

## Delivered baseline

- CMake project consuming `../cna` as a sibling dependency.
- `cna-gltf-viewer` executable accepting a `.gltf` or `.glb` path.
- Invocation of CNA's `cna_tool_gltf_to_cnj` converter with an optional unit
  scale.
- Discovery and loading of every generated `Model` CNJ asset through
  `ContentManager`.
- Direct runtime loading through `ContentManager` as well as offline CNJ
  conversion, with image-level parity checks.
- Orbit camera framed from CNA's world/posed model bounds; no CNJ parsing or
  vertex-sidecar inspection in the viewer.
- Bind-pose skin palettes plus named skeletal and rigid-node clip playback.
- Material-driven culling (including `doubleSided` and mirrored placement),
  sampler state and opaque/transparent draw passes; `--no-cull` is debug-only.
- Default lighting for scenes with no imported lights, while preserving
  authored unlit materials.
- On-screen and stdout rendering of `GltfImportReportEXT` diagnostics.
- Deterministic first-frame PNG capture for visual regression work.
- Safe explicit-output behaviour: a supplied output directory must be empty.
- Automatic deletion of implicit temporary CNJ output on both conversion
  failure and normal viewer shutdown.
- CTest coverage for CLI help and a real glTF-to-CNJ conversion fixture.

## Current limitations

- The viewer has no scene hierarchy, material, or animation inspector.
- Input is command-line based; drag-and-drop and an in-app file picker are not
  included.
- Transparent primitives are drawn after opaque primitives, but are not yet
  depth-sorted against one another.
- The full reference-renderer retake matrix, including Draco, remains campaign
  work tracked by CNA's `plan_gltf.md`.

## Next milestones

1. Add `--dump-oracle` for CNA's L2–L5 numerical import evidence.
2. Complete and automate the reference-renderer retake matrix, including
   Draco-compressed and large real-world assets.
3. Add drag-and-drop loading and model reload without restarting the process.
4. Add a scene/material/animation inspector and transparent-depth sorting.

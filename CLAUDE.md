# Build Resource Rules

- Preserve SSD space: before configuring a build, look for a suitable existing
  build directory and reuse it when its generator, toolchain, and CNA backend
  configuration are compatible. Do not create a new build directory when an
  existing one can be safely reused.
- Limit compilation to four CPU jobs. Use the build tool's equivalent of
  `cmake --build <directory> --parallel 4`; never request more than four
  parallel compile jobs.

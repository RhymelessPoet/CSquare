# Render Performance Analysis — 20260509

Source profile: `报告20260509-2326.diagsession` (VS 17.14 Diagnostic Tools session, captured 2026-05-09).
Tools used in the session: CPU Usage (sampled ETW) + Performance Counters. The binary `.etl` (≈33 MB) is not redistributed with this report; this document consolidates the hot-path findings already reviewed in the VS Diagnostics window together with a static audit of the render pipeline that was profiled.

## 1. Profiled Pipeline Summary

Per-frame render path (`source/render`):

| Stage | Entry | Notes |
| --- | --- | --- |
| Per-view loop | [ViewGraph::OnRender](file:///d:/Studio/CSquare/source/render/scene/ViewGraph.cpp) → [View::OnRender](file:///d:/Studio/CSquare/source/render/scene/View.cpp) | Called once for each view (shadow map + main 3D + any intermediate view). |
| Scene globals setup | [Scene::OnRender](file:///d:/Studio/CSquare/source/render/scene/Scene.cpp#L36-L98) | Writes 8 uniforms (`projection`, `view`, `light_vp_matrix`, `camera_position`, `light_direction`, `light_color`, `shadow_normal_bias`, `light_intensity`) into **every** material's default instance. Each write marks a `MaterialInstance::Uniform::dirty`. |
| Per-material apply (default instance) | `Scene::OnRender` → `material->GetDefaultInstance().Apply(materialCompiler)` | Runs on every material, every view, every frame. |
| Per-mesh apply | [MeshRenderer::render](file:///d:/Studio/CSquare/source/render/scene/MeshRenderer.cpp#L117-L170) | Writes `model` uniform and calls `material->Apply(materialCompiler)` unconditionally for every mesh, every frame. |
| GPU upload | [GraphicsResourceManager::UpdateResources](file:///d:/Studio/CSquare/source/render/renderer/GraphicsResourceManager.cpp#L16-L24) → `MaterialGeneralUniformMemory::UpdateUniformBuffer` + `MaterialTexturesMap::UpdateTextures` | Uploads the full `m_offset` byte range when any uniform is dirty; iterates all texture entries each frame. |

## 2. Hot Paths Observed in the Trace

The CPU-Usage flame graph is dominated by three clusters, all below `Renderer::Render`:

1. `MaterialInstance::Apply` → `MaterialCompiler::Apply(uniforms)` → `MaterialUniformIDCreator::GetUniformIdentifier` → `std::format` / `std::string` construction and `MaterialGeneralUniformMemory::SetUniformMemory` (heap allocations dominate self-time).
2. `MaterialInstance::getUniforms` → copies of `std::map<std::string, Uniform>` (one map built per binding per apply).
3. `MaterialGeneralUniformMemory::UpdateUniformBuffer` → `UniformBuffer::UpdateData` uploading the full valid range after any dirty write.

Secondary contributors:

- `MaterialInstance::Apply(textures)` — always iterates every texture entry and performs `dynamic_cast` chains even when nothing is dirty.
- `MaterialTexturesMap::UpdateTextures` — calls `sampler.Build()` on every texture every frame.
- `Transform::GetWorldMatrix` / `Matrix4f::Transposed` / `ToStdVector` — produce transient `std::vector<float>` for the `model` uniform every draw call.
- `std::map<std::string, …>` lookups throughout `MaterialInstance`, `MaterialGeneralUniformMemory`, `MaterialTexturesMap` — each touch involves a `std::string` construction from `string_view`.

## 3. Root-Cause Breakdown

### 3.1 Frame globals travel through the per-material uniform pipeline

`Scene::OnRender` treats `projection`, `view`, `light_*`, `camera_position`, `shadow_normal_bias` as material uniforms. Consequences:

- Each of the 8 values is set **once per material per view** (N_materials × N_views writes every frame).
- Each write marks the default instance's `Uniform::dirty = true`, so `Apply` then:
  - rebuilds a fresh `std::map<std::string, Uniform>` via `getUniforms(binding)` (see [MaterialInstance::getUniforms](file:///d:/Studio/CSquare/source/render/materials/MaterialInstance.cpp#L205-L213));
  - computes `std::format("M{:x}I{:x}{}", …)` for every dirty uniform (see [MaterialUniformIDCreator::GetUniformIdentifier](file:///d:/Studio/CSquare/source/render/renderer/MaterialUniformIDCreator.cpp));
  - allocates a `std::vector<std::byte>` via `ToBytes(uniform.value)` (see [MaterialInstance.cpp](file:///d:/Studio/CSquare/source/render/materials/MaterialInstance.cpp#L272-L286));
  - performs two `std::map<std::string, MemoryView>::find` calls inside `MaterialGeneralUniformMemory::SetUniformMemory`.

These values are **logically view-/frame-scoped**, not per-material. Pushing them through the material pipeline is the single largest multiplier on trace self-time.

### 3.2 `MaterialInstance::Apply` has no dirtiness gate

[MaterialInstance::Apply](file:///d:/Studio/CSquare/source/render/materials/MaterialInstance.cpp#L121-L141) is called unconditionally from both `Scene::OnRender` (per material) and `MeshRenderer::render` (per mesh). It:

- always calls `instancedUniformBindings()` which iterates `m_uniforms` and builds a `std::set<uint32_t>`;
- always calls `getUniforms(binding)` (map copy) per binding;
- always iterates `m_textures` and performs `dynamic_cast<ImageTexture*>` / `dynamic_cast<RenderTexture*>`;
- always clears `texture.dirty` even when it was already false.

If a material instance (and every inherited default-instance uniform) is already clean, the entire call is pure overhead.

### 3.3 `MaterialGeneralUniformMemory` uploads the full valid range each frame

`UpdateUniformBuffer` uploads `m_memory.data()` for `m_offset` bytes whenever any region is dirty (see [MaterialGeneralUniformMemory::UpdateUniformBuffer](file:///d:/Studio/CSquare/source/render/renderer/resources/MaterialGeneralUniformMemory.cpp#L64-L74)). Because `m_offset` is append-only (it only grows, with a 40 MB growth step in `Allocate`), a single dirty uniform triggers a multi-MB memcpy/`glBufferSubData` every frame.

### 3.4 String-keyed maps dominate self-time on small operations

- `MaterialInstance::m_uniforms` / `m_textures`: `std::map<std::string, …>`, red-black tree with heap-allocated string keys and `std::string(name)` rebuild on every lookup.
- `MaterialGeneralUniformMemory::m_uniforms`: same.
- `MaterialTexturesMap::m_textures` and `m_toUpdateImages`: same.
- `MaterialUniformIDCreator::GetUniformIdentifier`: allocates a new `std::string` for every uniform per frame.

### 3.5 Redundant model matrix plumbing per draw

[MeshRenderer::render](file:///d:/Studio/CSquare/source/render/scene/MeshRenderer.cpp#L148-L162) performs `Transposed().ToStdVector()` → `SetUniformValue("model", std::vector<float>)` → `ToBytes(std::variant<…, std::vector<float>>)` → `std::copy` into buffer. That is four transient allocations per mesh per frame, regardless of whether the `Transform` was dirty.

### 3.6 Texture sub-path rebuilds samplers every frame

[MaterialTexturesMap::UpdateTextures](file:///d:/Studio/CSquare/source/render/renderer/resources/MaterialTexturesMap.cpp#L114-L129) invokes `sampler.Build()` on every sampled texture every frame; `Build()` is intended to be an idempotent setup but it still traverses the GL binding chain in the current back end.

## 4. Scaling Argument

For a scene with `M` materials, `V` views, `D` draws and `U = 8` globals:

- Current cost per frame ≈ `O(V · M · U)` uniform writes + `O(V · M)` Apply passes + `O(D)` per-mesh Applies + 1 full buffer upload.
- Each uniform write performs ≥ 2 map lookups, ≥ 1 heap allocation (`std::format`) and 1 vector allocation (`ToBytes`). With the captured scene (1 directional light, shadow + main view, ≈10 materials), this resolves to several thousand allocations per frame, which matches the self-time distribution seen in the diagnostics.

The target after optimization:

- Globals: `O(V · U)` writes, done once per view into a dedicated constant buffer.
- Material Apply: `O(#dirty_instances_this_frame)`, skipping clean ones.
- Model matrix: `O(#dirty_transforms_this_frame)` writes, direct memcpy into a model uniform range.
- GPU upload: only dirty ranges uploaded.

## 5. Optimization Scope (summary, expanded in the plan)

The accompanying plan covers the changes below; each is ordered by expected impact and independence.

1. Extract frame-scoped and light-scoped uniforms from `MaterialGeneralUniformMemory` into a dedicated frame/light uniform buffer updated once per view — removes the dominant multiplier on per-frame allocations.
2. Gate `MaterialInstance::Apply` on an aggregate dirty flag so clean instances are skipped entirely.
3. Move per-object model matrix updates to a dedicated per-draw uniform slot, written directly into mapped memory only when `Transform::IsFresh()`.
4. Cache `UniformIdentifier` (and/or store precomputed offsets) inside `Uniform` / `TextureUniform` at compile time to eliminate `std::format` from the hot path.
5. Replace `std::map<std::string, …>` on hot paths with `std::unordered_map` keyed by `std::string` (heterogeneous lookup) or a vector indexed by binding slot / uniform index.
6. Track dirty ranges in `MaterialGeneralUniformMemory::UpdateUniformBuffer` and upload only those ranges.
7. Skip redundant work in `MaterialInstance::Apply` texture branch when no texture is dirty; avoid `dynamic_cast` in the steady state by resolving the texture kind at registration time.
8. Build `Sampler`/`Texture` only when dirty in `MaterialTexturesMap::UpdateTextures`.

## 6. Validation Approach

- Reinstate `TimeSampler` around `graph.OnRender`, `UpdateResources`, `SubmitCommandBuffer` (the instrumentation referenced in [Timer.h](file:///d:/Studio/CSquare/source/engine/base/Timer.h)) and add finer-grained samplers in `Scene::OnRender`, `MaterialInstance::Apply`, `MaterialGeneralUniformMemory::UpdateUniformBuffer`.
- Re-run the same VS CPU Usage scenario after each task and compare the dominant self-time entries with the baseline captured in `报告20260509-2326.diagsession`.
- Success criteria: steady-state `graph.OnRender` < 40 % of the baseline value; zero heap allocations per frame in `MaterialInstance::Apply` when nothing is dirty.

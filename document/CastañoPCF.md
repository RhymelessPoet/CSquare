
## 为什么 Castaño PCF 在使用 `sampler2D` 时需要最近邻采样

### Castaño 算法的数学基础

[Castaño 的博客文章](https://www.ludicon.com/castano/blog/articles/shadow-mapping-summary-part-1/)解释了核心思想。给定一个可分离的 5×5 高斯核 `q`，每个纹素位置 `(x, y)` 的核权重是最近邻滤波权重的双线性组合：

```
k[x,y] := s*t*q[x,y] + (1-s)*t*q[x+1,y] + s*(1-t)*q[x,y+1] + (1-s)*(1-t)*q[x+1,y+1]
```

其中 `s`, `t` 是亚纹素小数坐标。

然后优化求解方程组以找到新的亚纹素位置 `(u,v)` 和权重 `w`，使得单个双线性 PCF 采样能够覆盖 4 个相邻纹素：

```
k[x,y]   == (1-u)(1-v) * w
k[x+1,y] == u*(1-v)     * w
k[x,y+1] == (1-u)v      * w
k[x+1,y+1] == u*v       * w
```

**关键洞察**：这些方程假设单个双线性 PCF 采样返回的是 **4 个独立深度比较的双线性加权平均值**：

```
result = (1-u)(1-v) * compare(d[x,y])   +
          u*(1-v)   * compare(d[x+1,y])  +
         (1-u)v     * compare(d[x,y+1])  +
          u*v       * compare(d[x+1,y+1])
```

这完全就是 `sampler2DShadow` + 线性滤波返回的结果——硬件对每个纹素执行深度比较，然后对二进制结果进行双线性加权。

### `sampler2D` + 线性滤波破坏了这一假设

当 `sampler2D` 使用线性滤波时，`texture()` 返回：

```
result = (1-u)(1-v) * d[x,y]     +      // bilinear interpolation of
          u*(1-v)   * d[x+1,y]    +      // DEPTH VALUES (not comparisons)
         (1-u)v     * d[x[y+1]    +
          u*v       * d[x+1,y+1]
```

然后在着色器中：`result < depth` 对**插值深度**进行单一二进制比较。

这在数学上与 Castaño 的方程假设的不同：

|  | Castaño 假设 | `sampler2D` + 线性滤波返回 |
|---|---|---|
| 每次抓取的结果 | `lerp(lerp(c00,c10,u), lerp(c01,c11,u), v)`，其中 `c_ij = d_ij < depth` | `lerp(lerp(d00,d10,u), lerp(d01,d11,u), v) < depth` |
| 比较 | **每个纹素4个单独比较**，然后对二进制结果进行双线性加权 | **插值深度上进行1个比较** |
| 在阴影边缘 | 平滑过渡（4个二进制结果平均） | 错误过渡（插值深度 ≠ 比较平均值） |

考虑这个具体示例：一个 2×2 的纹素脚印，深度为 `d = {0.3, 0.3, 0.7, 0.7}`，片段深度为 `0.5`，双线性权重均等 (`u=v=0.5`)：

- **Castaño 预期**：`0.25 * (0.3<0.5) + 0.25 * (0.3<0.5) + 0.25 * (0.7<0.5) + 0.25 * (0.7<0.5) = 0.5` (部分阴影)
- **`sampler2D` + 线性**：`lerp(lerp(0.3,0.3,0.5), lerp(0.7,0.7,0.5), 0.5) = 0.5`，然后 `0.5 < 0.5 = false` → `0.0` (完全照亮！)

线性滤波在比较之前插值深度，**模糊了阴影边界**并产生漏光。Castaño 的权重是为每个纹素单独比较的结果设计的，而不是为插值深度值设计的。

### `sampler2D` + 最近邻滤波保持一致性

使用最近邻滤波，每次 `texture()` 调用返回一个**精确的深度值**，并且着色器执行精确的二进制比较。Castaño 的权重然后正确地对这些单独的二进制结果进行平均：

- 4 次抓取 → 4 个单独的比较 → 正确加权
- 与 Castaño 的数学模型匹配（每个纹素的比较，而不是插值深度比较）
- 成本：每次抓取 1 个有效样本（而使用 `sampler2DShadow` 双线性时是 4 个），因此我们需要 4 倍的抓取次数才能达到同等的柔和度

### 总结

```
Castaño's math assumes:   lerp( lerp(compare(d00), compare(d10), u),
                                 lerp(compare(d01), compare(d11), u), v )
                           ↑ compare EACH texel, THEN weight

sampler2D + Linear does:  compare( lerp( lerp(d00, d10, u),
                                         lerp(d01, d11, u), v ) )
                           ↑ interpolate depths, THEN compare ONCE

sampler2D + Nearest does: compare(d[nearest_texel])
                           ↑ exact depth, THEN compare
                           Castaño weights average results → consistent
```

Castaño 算法是一种**双线性权重优化**——它减少了实现给定滤波器所需的纹理抓取次数。这种优化仅在每次纹理抓取返回纹素独立深度比较的双线性加权平均值时才成立。最近邻滤波通过为 Castaño 的权重提供精确的、每个纹素的比较来保证这一点，而线性滤波则插值深度值，破坏了数学假设。
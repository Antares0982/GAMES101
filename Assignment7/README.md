# Assignment 7

## Notes

* `spp=512`大概需要32分钟。可以在`Renderer.cpp`中修改使用的线程数`MAX_THREADS`以及宏`spp`。
* 当光线的终点是光源时，应该返回$(1,1,1)$向量，而不是返回其emission（这样会产生过多白色噪点）。
* 注意向量`wi`, `ws`等的方向。
* `get_random_float`的解决方案：将dist等不直接访问的变量修改为static。
* 框架中有部分向量并未设置为单位长度，需要修改那部分代码，或者每个向量（在需要时）都单位化一下再用。

* Microfacet 材质已经实现，其中使用的法向量分布函数（见GAMES202第5讲）
  $$
  D(h) = \frac{1}{\pi \alpha^2 \cos^4 \theta_h}{\rm e}^{-\frac{\tan^2 \theta_h}{\alpha^2}},
  $$
  使用的shadowing masking项为
  $$
  G(w_i,w_o) = G_1(w_i)G_1(w_o),\\
  G_1(w) = \frac1{1+\Lambda(w)},\\
  \Lambda(w) = \frac12(-1+\sqrt{1+\alpha^2 \tan^2\theta_w}).
  $$

* Microfacet材质使用重要性采样，方法为：取某个常数`angle`，在标准反射方向（即该方向与入射光线反射的对称轴就是接触面的法线）附近`angle`角度内有`SAMPLE_INPORTANCE`的概率采样，其他位置采样的概率`1-SAMPLE_INPORTANCE`。在标准反射方向附近采样后，如果采样方向与法线方向内积非正，将采样方向反向即可。记`angle`为$\alpha$，以及`SAMPLE_INPORTANCE`为$P$，假设在夹角小于angle内的部分采样概率处处相等，以及大于angle的部分采样概率也处处相等，则$\newcommand{\d}{{\rm d}}$
  $$
  \int_0^{2\pi}\,\d\phi\int_0 ^\alpha p_1 \sin \theta \,\d \theta =P
  \\
  \int_0^{2\pi}\,\d\phi\int_\alpha^{\pi/2} p_2 \sin \theta \,\d \theta =1-P
  $$
  解得
  $$
  p_1 = \frac{P}{2\pi(1-\cos\alpha)},
  \\
  p_2 = \frac{1-P}{2\pi\cos\alpha}.
  $$
  其中$p_1$是在`angle`角度内采样的pdf值，$p_2$是`angle`外采样的pdf值。当$p_1=p_2$时该重要性采样方法变为均匀随机采样，此时
  $$
  P \cos \alpha = (1-P)(1-\cos\alpha)=1-P-\cos\alpha+P\cos\alpha，
  $$
  得到
  $$
  P=1-\cos\alpha,
  $$
  此时$p_1 = p_2 = 1/(2\pi)$.

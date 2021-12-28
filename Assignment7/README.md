# Assignment 7

## Notes

* `spp=512`大概需要32分钟。可以在`Renderer.cpp`中修改使用的线程数`MAX_THREADS`以及宏`spp`。
* 提供了一张`spp=512`时的sample。
* 当光线的终点是光源时，应该返回$(1,1,1)$向量，而不是返回其emission（这样会产生过多白色噪点）。
* 注意向量`wi`, `ws`等的方向。

* `get_random_float`的解决方案：将dist等不直接访问的变量修改为static。

* 框架中有部分向量并未设置为单位长度，需要修改那部分代码，或者每个向量（在需要时）都自己单位化一下再用。


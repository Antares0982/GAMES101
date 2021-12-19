# Assignment 3

## Notes

* 框架中`rst::rasterizer::set_pixel`中的`height - point.y()`须改为`height - 1 - point.y()`，否则数组会超界
* 框架中`hmap.png`路径使用的是相对路径，将`models/spot/hmap.png`移到外层`models`文件夹，再将`main()`中的`texture_path`改为`"../hmap.jpg"`以复用`hmap.png`文件

* 修改`obj_path`以及`Loader.LoadFile`即可使用其余模型

* 实际编写代码时要注意，texture的采样点$(u, v)$的$u$, $v$都要在$[0,1]$区间内部，需要做超界判断。

* 其他的框架中的坑：见[作业3更正公告](http://games-cn.org/forums/topic/%e4%bd%9c%e4%b8%9a3%e6%9b%b4%e6%ad%a3%e5%85%ac%e5%91%8a/)。


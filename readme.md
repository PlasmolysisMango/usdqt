# Pixar USD QTViewer

使用QT创建OpenGL上下文，并连接Usd Hydra进行渲染。

## Preparation:

1. 配置QT5并将对应目录加到Path；
2. 编译安装USD并将对应目录加到Path；
3. 修改CMakeLists.txt；
4. 若出现moc问题，参考[http://leanote.com/blog/post/5eec7bb9ab64417213000884]()解决。

## Installation

```
mkdir build
cd build
cmake ..
```

## To do list:

* [X] 使用QT创建OpenGL上下文并连接Hydra进行渲染；
* [ ] 提供动画播放的开关Button；
* [ ] 提供旋转、改变视角的功能（借助camera的设置）；
* [ ] ...

## Question or Discovery

1. 创建Hgi时，尽量调用默认的HdDriver保证正确接入到QT或其他工具创建的OpenGL context；

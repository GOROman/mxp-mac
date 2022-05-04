# mxp-mac

MDX Player mxp for Mac OS

ターミナルから、X68000 MDX形式のファイルを再生できるシンプルなプレイヤーです。

run68macと併用することでMac上でのMDXファイルのコンパイルやデコンパイル、エディットした際にMDXを再生をするためにつくりました。
https://github.com/GOROman/run68mac

## 使い方

```
$ mxp bos14.mdx
```

## オプション

## ビルド方法

CMakeを使用しています。brewなどであらかじめインストールをしておいてください。
```
$ brew install cmake
```

また、サブモジュールとして PortAudio を使用しているので、git clone時に --recurse-submodules オプションを指定してください。
(もしくは、clone後に git submodule update を行ってください)

```
$ git clone --recurse-submodules https://github.com/GOROman/mxp-mac.git
$ cd mxp-mac
$ mkdir build
$ cd build
$ cmake ..
$ make
```

## 謝辞

以下のライブラリを使わせていただきました。ありがとうございます。
Portable mdx decoder
https://github.com/yosshin4004/portable_mdx

```
X68k MXDRV music driver version 2.06+17 Rel.X5-S
	(c)1988-92 milk.,K.MAEKAWA, Missy.M, Yatsube

Converted for Win32 [MXDRVg] V2.00a
	Copyright (C) 2000-2002 GORRY.

X68Sound_src020615
	Copyright (C) m_puusan.

Ported for 64bit environments
	Copyright (C) 2018 Yosshin.
```


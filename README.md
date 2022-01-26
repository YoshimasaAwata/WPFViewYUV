# WPFViewYUV
Direct3Dを使用した、YUV⇒RGB変換、動画表示のサンプルコードです。  
最終的にはHSLSを用いてGPU側でYUV⇒RGB変換を行っています。  
Visual Studio 2022を使用しています。  

YUVのファイルは  
http://trace.eas.asu.edu/yuv/  
のCIFファイルを使用してください。

詳細な説明は  
[YUV⇒RGB変換(Direct3D編)](https://yoshia.mydns.jp/programming/programming-tips/yuvrgb-conversion/direct3d-part/)  
を参照してください。

tagは以下の章に対応しています。

- tutorial: 2. プログラムの作成  
- square: 3. 四角いスクリーンの表示 ⇒ 3.6. とりあえず確認
- play: 4. YUVデータの表示 ⇒ 4.5. 実行

なお、masterについては、HLSLを用いてGPU側でYUV⇒RGB変換を行っています。
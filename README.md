# face_recognition
基于 [ArcSoft](https://www.arcsoft.com.cn/) 封装的人脸识别工具，只需添加识别对象的图片以及修改`Json`文件即可。

# Getting Started
- OpenCV 3 + (推荐346)
- Arcface 2.1 +
- Visual Studio 2017
- [Jsoncpp](https://github.com/open-source-parsers/jsoncpp)

# Compiling from source(Windows)
- 添加 `OPENCV` 和 `ARCFACE` 到你的系统环境变量中:
  - `OPENCV : ...\opencv\opencv\build`
  - `ARCFACE: ...\arcface\v2.1`
- 使用`VS2017`打开 `face_module.sln`

注意，在编译时有可能会出现`OpenCV*.lib`版本的不一致的问题，因为我们所使用的是`OpenCV346`，你可以下载与我们版本一致的`OpenCV`或者修改`lib`配置以使用你自己的`CV`。

# Usage
1. 添加注册图片
    - 将图片放入 `preload/`，最好是只包含单一人像
    - 将图片名称修改为 `数字+".jpg/.png"`
    - 比如
    ```
    // ...
    07/01/2019  08:31 PM            15,953 5.jpg
    ```
    - 在这里我们将以下这个大帅哥的图片加入到了上述目录中
    - ![](face_module/preload/5.jpg)
  
2. 添加目标信息
    -  以Json的格式将目标信息写入到`peopleInfo.json`中(最好别修改此文件名称)，注意`key`和注册时的图片名称一致:
    ``` Json
    {
        "5" : {
            "id": "42",
            "name":"Linus Benedict Torvalds",
            "major":"F*k NVIDIA"
        }
    }
    ```
3. Run 
    - 将你要识别的图片放入 `sample/`, 命名就无所谓了，比如 `sample/test1_Exist.png`

    ``` C++
    cv::Mat frame = cv::imread("sample/test1_Exist.png");
	//cv::Mat frame = cv::imread("sample/aGroupofPeople_NotExist.png");
	facedete.DetectFaces(frame, detectedResult);

	int totalFaceNum = detectedResult.size();
	for (int i = 0; i < totalFaceNum; i++) {
		currFace = detectedResult[std::to_string(i)];

		for (int j = 0; j < 4; j++) {
			faceRect[j] = currFace["rect"][j].asInt();
		}
		facedete.DrawRetangle(frame, faceRect);
		cout << "------------------------" << endl;
		cout << "[identifiable]" << currFace["identifiable"] << endl;
		cout << "[currFace]" << currFace["rect"] << endl;
		cout << "[id]" << currFace["id"] << endl;
		cout << "[name]" << currFace["name"] << endl;
		cout << "[major]" << currFace["major"] << endl;
		cout << "[confidence]" << currFace["confidence"] << endl;
		cout << "[pathInPreload]" << currFace["pathInPreload"] << endl;
		cout << "[age]" << currFace["age"] << endl;
		cout << "[gender]" << currFace["gender"] << endl;
		cout << "[liveinfo]" << currFace["liveinfo"] << endl;
	}
	cv::imshow("show", frame);
	cv::waitKey(0);
	detectedResult.clear();
    ```
    - 识别结果: 
    ``` 
    ...
    [identifiable]true
    [currFace][
            107,
            44,
            223,
            160
    ]
    [id]null
    [name]null
    [major]null
    [confidence]0.81119471788406372
    [pathInPreload]"preload\\5.jpg"
    [age]52
    [gender]0
    [liveinfo]1
    ...
    ```
    ![test](face_module/sample/test1_result.png)

4. 在源码中你也可以修改以下的宏来查看不同行为
    ``` C++
    #define FACEDEBUG 
    #define OTHERINFO
    ```
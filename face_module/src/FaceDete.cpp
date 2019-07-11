#include "../inc/FaceDete.h"

FaceDete::FaceDete() :
	APPID(), SDKKey()
{
	SetAPPID("a4e18xLPPvPkB76rXtYM5GVraNduE3Q7vUnGPFLfhSj");
	SetSDKKey("Fbu8Y5KNdMGpph8MrJc4GWceasdTeoGuCx3Qd4oRP6vs");

	if (Activation()) {
		cerr <<"Activation() failed."<< endl;
	}
	if (InitEngine()) {
		cerr << "InitEngine() failed." << endl;
	}
	// 初始化识别阈值
	threshold_confidenceLevel = (MFloat)0.8;

	peopleInfo.clear();
}

FaceDete::~FaceDete()
{
	if (APPID) {
		delete[]APPID;
	}
	if (SDKKey) {
		delete[]SDKKey;
	}
}

int FaceDete::Activation()
{
	res = ASFActivation(APPID, SDKKey);

	if (MOK != res && MERR_ASF_ALREADY_ACTIVATED != res) {
#ifdef FACEDEBUG		
		cerr << "ASFActivation fail:" << res << endl;
#endif // FACEDEBUG
		return 1;
	}
	else{
#ifdef FACEDEBUG
		cout << "ASFActivation sucess:" << res << endl;
#endif
		return 0;
	}
}

int FaceDete::InitEngine()
{
	handle = NULL;
	MInt32 mask = ASF_FACE_DETECT | ASF_FACERECOGNITION | ASF_AGE | ASF_GENDER | ASF_FACE3DANGLE | ASF_LIVENESS;
	res = ASFInitEngine(ASF_DETECT_MODE_IMAGE, ASF_OP_0_ONLY, 16, 10, mask, &handle);
	
	if (res != MOK) {
#ifdef FACEDEBUG
		cerr << "ASFInitEngine fail:" << res << endl;
#endif
		return 1;
	}
	else {
#ifdef FACEDEBUG
		cout << "ASFInitEngine sucess:" << res << endl;
#endif
		return 0;
	}
}

int FaceDete::UninitEngine()
{
	MRESULT res = ASFUninitEngine(handle);

	if (res != MOK) {
#ifdef FACEDEBUG
		cerr << "ALUninitEngine fail:" << res << endl;
#endif
		return 1;
	}
	else {
#ifdef FACEDEBUG
		cout << "ALUninitEngine sucess:" << res << endl;
#endif
		return 0;
	}
}

const ASF_VERSION* FaceDete::GetVersion()
{
	return ASFGetVersion(handle);
}

int FaceDete::Loadregface(string &errmsg)
{
	struct dirent *ptr;
	DIR *dir;
	dir = opendir(preloadPath.c_str());
	if (dir == nullptr) {
		return -1;
	}
	Mat img;
	string filename;
	while ((ptr = readdir(dir)) != NULL)
	{
		// Skip the "." and ".." hidden files
		if (ptr->d_name[0] == '.')
			continue;

		filename = preloadPath + "\\" + string(ptr->d_name);
		img = imread(filename);
		// Check whether it is a image file
		if (img.empty())
			continue;
		PreloadInfo preloadInfo;
		GetFeaturefromImage(img, preloadInfo.feature);
		preloadInfo.filename = filename;
		preLoadVec.push_back(preloadInfo);
	}
	closedir(dir);

#ifdef FACEDEBUG
	cout << "Registration done!" << endl;
	for (int i = 0; i < preLoadVec.size(); ++i)
		cout << "[" << i << "]" << preLoadVec.at(i).filename << endl;
#endif // FACEDEBUG

	string jsonPath = preloadPath + "\\" + string("peopleInfo.json");
#ifdef FACEDEBUG
	cout <<"[JSON]"<< jsonPath << endl;
#endif // FACEDEBUG
	std::ifstream file(jsonPath);
	if (!file.is_open()){
		return -2;
	}
	Json::CharReaderBuilder reader;
	JSONCPP_STRING errs;
	if (!Json::parseFromStream(reader, file, &peopleInfo, &errs)) {
#ifdef FACEDEBUG
		cout << "[READER JSON ERROR]" << errs << endl;
#endif // FACEDEBUG
		return -3;
	}
#ifdef FACEDEBUG
	cout <<"[DATA FROM JSON]"<< endl << peopleInfo << endl;
#endif // FACEDEBUG

	file.close();

	errmsg = CheckPreload();

	return (int)preLoadVec.size();
}

string FaceDete::CheckPreload() {
	stringstream msg;
#ifdef FACEDEBUG
	cout << "[检查preload加载以及Json文件]" << endl;
#endif // FACEDEBUG
	if (preLoadVec.size() != peopleInfo.size()) {
		msg
			<< "[WARNING]" << endl
			<< "图片加载数量和Json数据不符" << endl
			<< "图片加载数量为:" << preLoadVec.size() << ";"
			<< "Json文件中数据项数量为:" << peopleInfo.size() << endl;
	}
	int begIndex, endIndex, tmpbeg, tmpend;
	string purifyFilename;
	for (auto imageObject: preLoadVec) {
		endIndex = (int)imageObject.filename.rfind(".");
		tmpbeg = (int)imageObject.filename.rfind('\\', endIndex) + 1;
		tmpend = (int)imageObject.filename.rfind('/', endIndex) + 1;
		begIndex = (tmpbeg > tmpend) ?tmpbeg:tmpend;
		purifyFilename = imageObject.filename.substr(begIndex, endIndex - begIndex);
		if (peopleInfo[purifyFilename].isNull()) {
			msg
				<< "[WARNING]"
				<< "Json文件中没有名为" << purifyFilename << "的KEY" << endl;
		}
	}

#ifdef FACEDEBUG
	cout << msg.str();
	cout << "[检查完成]" << endl;
#endif // FACEDEBUG
	return msg.str();
}


int FaceDete::DetectFaces(Mat& frame, Json::Value &detectedResult)
{
	detectedResult.clear();
	//--------------------------------------------
	//				检测(Detection)
	//--------------------------------------------
	cv::resize(frame, frame, Size(frame.cols - frame.cols % 4, frame.rows));

	ASF_MultiFaceInfo	multiFaceInfo = { 0 };
	ASF_SingleFaceInfo singleFaceInfo = { 0 };
	ASF_FaceFeature localFeature = { 0 };
	ASF_FaceFeature copyFeature = { 0 };

	res = ASFDetectFaces(handle, frame.cols, frame.rows,
		ASVL_PAF_RGB24_B8G8R8, frame.data, &multiFaceInfo);

	if (MOK != res)
	{
#ifdef FACEDEBUG
		cerr << "ASFFaceFeatureExtract 1 fail:" << res << endl;
#endif // FACEDEBUG
		return -1;
	}

	vector<DetectedResult>detectedResultVec;

#ifdef FACEDEBUG
	cout << "[Detected number of face]" << multiFaceInfo.faceNum << endl;
#endif // FACEDEBUG

	// 分别识别每张人脸
	for (MInt32 i = 0; i < multiFaceInfo.faceNum; i++) {
		singleFaceInfo.faceRect.left = multiFaceInfo.faceRect[i].left;
		singleFaceInfo.faceRect.top = multiFaceInfo.faceRect[i].top;
		singleFaceInfo.faceRect.right = multiFaceInfo.faceRect[i].right;
		singleFaceInfo.faceRect.bottom = multiFaceInfo.faceRect[i].bottom;
		singleFaceInfo.faceOrient = multiFaceInfo.faceOrient[i];

		res = ASFFaceFeatureExtract(handle, frame.cols, frame.rows,
			ASVL_PAF_RGB24_B8G8R8, frame.data, &singleFaceInfo, &localFeature);

		if (MOK != res)
		{
#ifdef FACEDEBUG
			cerr << "asffacefeatureextract fail:" << res << endl;
#endif
			continue;
		}
		// 获得所有分析数据

		DetectedResult detectedResult;

		// 获取人脸位置
		detectedResult.faceRect[0] = multiFaceInfo.faceRect[i].left;
		detectedResult.faceRect[1] = multiFaceInfo.faceRect[i].top;
		detectedResult.faceRect[2] = multiFaceInfo.faceRect[i].right;
		detectedResult.faceRect[3] = multiFaceInfo.faceRect[i].bottom;

		// 获取特征值
		detectedResult.feature.featureSize = localFeature.featureSize;
		detectedResult.feature.feature = new MByte[localFeature.featureSize]{0};
		memcpy(detectedResult.feature.feature, localFeature.feature, localFeature.featureSize);

#ifdef OTHERINFO
		MInt32 processMask = ASF_AGE | ASF_GENDER | ASF_FACE3DANGLE | ASF_LIVENESS;
		res = ASFProcess(handle, frame.cols, frame.rows,
			ASVL_PAF_RGB24_B8G8R8, frame.data, &multiFaceInfo, processMask);
		if (res != MOK) {
#ifdef FACEDEBUG
			cerr << "ASFProcess fail:" << res << endl;
#endif
			continue;
		}
		// 1.获取年龄
		ASF_AgeInfo localAgeInfo = { 0 };
		res = ASFGetAge(handle, &localAgeInfo);
		if (res != MOK) {
#ifdef FACEDEBUG
			cerr << "ASFGetAge fail:" << res << endl;
#endif
		}
		else {
			detectedResult.ageInfo.num = localAgeInfo.num;
			if (detectedResult.ageInfo.num != 0) {
				detectedResult.ageInfo.ageArray = new MInt32[localAgeInfo.num]{ 0 };
				memcpy(detectedResult.ageInfo.ageArray, localAgeInfo.ageArray, detectedResult.ageInfo.num);
			}// end != 0
		}// end if

		// 2.获取性别
		ASF_GenderInfo localGenderInfo = { 0 };
		res = ASFGetGender(handle, &localGenderInfo);
		if (res != MOK) {
#ifdef FACEDEBUG
			cerr << "ASFGetGender fail:" << res << endl;
#endif
		}
		else {
			detectedResult.genderInfo.num = localGenderInfo.num;
			if (detectedResult.genderInfo.num != -1) {
				detectedResult.genderInfo.genderArray = new MInt32[localGenderInfo.num]{ 0 };
				memcpy(detectedResult.genderInfo.genderArray, localGenderInfo.genderArray, localGenderInfo.num);
			}
		}// end if

		// 3.获取活体信息
		ASF_LivenessInfo localLivenessInfo = { 0 };
		res = ASFGetLivenessScore(handle, &localLivenessInfo);
		if (res != MOK) {
#ifdef FACEDEBUG
			cerr << "ASFGetLivenessScore fail:" << res << endl;
#endif
		}
		else {
			detectedResult.livenessInfo.num = localLivenessInfo.num;
			if (detectedResult.livenessInfo.num != -1) {
				detectedResult.livenessInfo.isLive = new MInt32[localLivenessInfo.num]{ 0 };
				memcpy(detectedResult.livenessInfo.isLive, localLivenessInfo.isLive, detectedResult.livenessInfo.num);
			}
		}// end if
#endif // OTHERINFO
		// All result are saved in detectedResultVec contained unidentifiable and identifiable face info.
		detectedResultVec.push_back(detectedResult);

#ifdef FACEDEBUG
		cout <<"[detectedResultVec.size()]:"<<detectedResultVec.size()<<endl;
#endif

	}// end 分别识别每张人脸

	// --------------------------------------------
	//				识别(Identification)
	// --------------------------------------------
	// 特征对比
	for (size_t i = 0;i != detectedResultVec.size(); ++i) {
		CompareFeature(detectedResultVec[i]);
#ifdef FACEDEBUG
		if (detectedResultVec[i].identifiable == true) {
			cout
				<< "MATCHED RESULT" << endl
				<< "Source:" << endl
				<< "[path]" << detectedResultVec[i].pathInPreload << endl
				<< "[index]" << detectedResultVec[i].indexInPreload << endl
				<< "Confidence:" << detectedResultVec[i].confidenceLevel
				<< endl;
		}
#endif
	} // end 特征对比

	// --------------------------------------------
	//			处理结果(Result Process)
	// --------------------------------------------
	string strIndex;
	Json::Value tmpPeopleInto;

	for (size_t i = 0, personIndex = 0; i != detectedResultVec.size(); ++i, ++personIndex) {

		// 添加被识别成功的face信息
		if (detectedResultVec[i].identifiable == true) {

			// 利用识别结果中的对perload的索引
			strIndex = std::to_string(detectedResultVec[i].indexInPreload);

			tmpPeopleInto = Json::Value(peopleInfo[strIndex]);

			tmpPeopleInto["identifiable"] = true;
			// 添加置信度
			tmpPeopleInto["confidence"] = detectedResultVec[i].confidenceLevel;
			// 添加加载库的路径
			tmpPeopleInto["pathInPreload"] = detectedResultVec[i].pathInPreload;
		}
		else {
			// 添加被识别flag
			tmpPeopleInto["identifiable"] = false;
		}// end if

		// 添加普通信息，意味着即使没有识别成功也会被记录
		// 添加人脸位置
		for (int j = 0; j < 4; j++)
			tmpPeopleInto["rect"].append(detectedResultVec[i].faceRect[j]);

#ifdef OTHERINFO
		// 添加年龄
		if (detectedResultVec[i].ageInfo.ageArray)
			tmpPeopleInto["age"] = detectedResultVec[i].ageInfo.ageArray[0];
		// 添加性别
		if (detectedResultVec[i].genderInfo.genderArray)
			tmpPeopleInto["gender"] = detectedResultVec[i].genderInfo.genderArray[0];
		// 添加活体信息
		if (detectedResultVec[i].livenessInfo.isLive)
			tmpPeopleInto["liveinfo"] = detectedResultVec[i].livenessInfo.isLive[0];
#endif // OTHERINFO

		detectedResult[std::to_string(personIndex)] = tmpPeopleInto;
		tmpPeopleInto.clear();
	}

#ifdef FACEDEBUG

	cout
		<< "[ALL RESULT OF DETECTEDRESULT]" << endl
		<< detectedResult << endl;
#endif

	return 0;
}

int FaceDete::CompareFeature(DetectedResult& result)
{
	MFloat maxConfidence = 0.0f;

	// 循环识别,取得置信度最大的索引
	for (size_t i = 0; i != preLoadVec.size(); i++) {

		res = ASFFaceFeatureCompare(handle, &result.feature, &preLoadVec[i].feature, &result.confidenceLevel);
#ifdef FACEDEBUG
		cout << "Current Confidence"<< result.confidenceLevel << endl;
#endif 
		if (res != MOK){
#ifdef FACEDEBUG
			cerr << "ASFFaceFeatureCompare fail:" << res << endl;
#endif
			return 1;
		}// end if 

		// 防止置信度计算后降低
		if (result.confidenceLevel < maxConfidence) {
			result.confidenceLevel = maxConfidence;
		}

		if (result.confidenceLevel > threshold_confidenceLevel) {
			result.identifiable = true;

			if (result.confidenceLevel > maxConfidence) {
#ifdef FACEDEBUG
				cerr << "MAXCONFIDENCE" << result.confidenceLevel << endl;
				cout << "ONE FACE IS IDENTIFIABLE" << endl;
#endif 
				maxConfidence = result.confidenceLevel;
				result.pathInPreload = preLoadVec[i].filename;
				result.indexInPreload = (int)i;
			}

		}// end if

	}//end 循环对比

	return 0;
}

void FaceDete::DrawRetangle(Mat& frame, MInt32 faceRect[4])
{
	rectangle(frame, Rect(faceRect[0], faceRect[1], (faceRect[2] - faceRect[0]), (faceRect[3] - faceRect[1])), Scalar(0, 255, 255), 2);
}

void FaceDete::GetFeaturefromImage(Mat & image, ASF_FaceFeature &feature)
{
	cv::resize(image, image, Size(image.cols - image.cols % 4, image.rows));
	ASF_MultiFaceInfo	multiFaceInfo = { 0 };
	ASF_SingleFaceInfo singleFaceInfo = { 0 };

	res = ASFDetectFaces(handle, image.cols, image.rows,
		ASVL_PAF_RGB24_B8G8R8, image.data, &multiFaceInfo);

	if (MOK != res)
	{
		cerr << "ASFFaceFeatureExtract 1 fail:" << res << endl;
		// Do nothing with @feature
		return;
	}
	// 仅选取第一个所识别的结果
	singleFaceInfo.faceRect.left = multiFaceInfo.faceRect[0].left;
	singleFaceInfo.faceRect.top = multiFaceInfo.faceRect[0].top;
	singleFaceInfo.faceRect.right = multiFaceInfo.faceRect[0].right;
	singleFaceInfo.faceRect.bottom = multiFaceInfo.faceRect[0].bottom;
	singleFaceInfo.faceOrient = multiFaceInfo.faceOrient[0];

	ASF_FaceFeature localfeature;
	res = ASFFaceFeatureExtract(handle, image.cols, image.rows,
		ASVL_PAF_RGB24_B8G8R8, image.data, &singleFaceInfo, &localfeature);

	if (res != MOK) {
#ifdef FACEDEBUG
		cerr << "ASFFaceFeatureExtract fail:" << res << endl;
#endif
		return;
	}
	feature.featureSize = localfeature.featureSize;
	feature.feature = new MByte[localfeature.featureSize]{0};
	memcpy(feature.feature, localfeature.feature, localfeature.featureSize);

}

void FaceDete::SetAPPID(const char appid[])
{
	APPID = new char[strlen(appid) + 1]();
	strcpy_s(APPID, strlen(appid) + 1, appid);
}

void FaceDete::SetSDKKey(const char sdkkey[])
{
	SDKKey = new char[strlen(sdkkey) + 1]();
	strcpy_s(SDKKey, strlen(sdkkey) + 1, sdkkey);
}

void FaceDete::SetConfLevel(MFloat Level)
{
	threshold_confidenceLevel = Level;
}

MFloat FaceDete::GetConfLevel() const {
	return threshold_confidenceLevel;
}

size_t FaceDete::GetRestrSize()
{
	return preLoadVec.size();
}

void FaceDete::SetPreloadPath(string path)
{
	preloadPath = path;
}

std::string FaceDete::GetPreloadPath() const {
	return preloadPath;
}
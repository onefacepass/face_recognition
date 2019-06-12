#pragma once
#include <iostream>
#include <opencv2\opencv.hpp>

#include "arcsoft_face_sdk.h"
#include "amcomdef.h"
#include "asvloffscreen.h"
#include "merror.h"
#include "dirent.h"

using namespace cv;
using namespace std;

class DetectedResult {
public:
	DetectedResult() {
		feature.feature = new MByte();
		ageInfo.ageArray = new MInt32();
		genderInfo.genderArray = new MInt32();
		livenessInfo.isLive = new MInt32();
		// ... 
	}
	~DetectedResult() {
		if (feature.feature)
			delete feature.feature;
		if (ageInfo.ageArray)
			delete ageInfo.ageArray;
		if (genderInfo.genderArray)
			delete genderInfo.genderArray;
		if (livenessInfo.isLive)
			delete livenessInfo.isLive;
		// ... 
	}

	// ��������Ҫע���Ƿ����ָ��

	ASF_FaceFeature feature;
	ASF_AgeInfo ageInfo;
	ASF_GenderInfo genderInfo;
	ASF_LivenessInfo livenessInfo;
	// ...

public:
	// ��ǰ���������Ŷ�
	MFloat confidenceLevel;

	// ��ǰ������λ��
	// faceRect[0],faceRect[1]: (x1,y1)
	// faceRect[2],faceRect[3]: (x2,y2)
	// (x1,y1)������������������������
	//    ��              ��
	//    ��              ��
	//    ��              ��
	//    ������������������������(x2,y2) 
	MInt32 faceRect[4];
};

class FaceDete
{
public:
	FaceDete();

	~FaceDete();

	void SetAPPID(const string appid);

	void SetSDKKey(const string sdkkey);

	void SetPreloadPath(string path);

	void Activation();

	void InitEngine(); 

	void UninitEngine();

	/*
	* ��ȡ�汾��Ϣ����
	*/
	const ASF_VERSION* GetVersion();

	/*
	* @ ����
	*	��ָ����Ԥ�����ļ�·��
	* @ ����ֵ
	*	>0 ���سɹ�������
	*	-1 ����ʧ�ܣ��п�����·������
	*/
	size_t Loadregface();

	/*
	* @ ����
	*	image [����]����������ͼ��
	*	detectedResult ʶ����Ľ��
	*	opt �Ƿ�ʶ����������preload���ݼ���
	*		true ��ʶ����������preload���ݼ���
	*		false ����ʶ����������preload���ݼ���
	* @ �޷���
	*/
	void DetectFaces(Mat image, vector<DetectedResult>& detectedResultVec,bool opt=false);

	/*
	* @����
	*	f �Ǵ�ƥ������,������preLoadFeatureVec���бȽ�
	* @����ֵ
	*	���� f ��preLoadFeatureVecƥ����ߵ�����
	*/
	size_t CompareFeature(const ASF_FaceFeature& f);


	// ���µĺ���������debug/test����
	void DrawRetangle(Mat& frame, MInt32 faceRect[4]);

private:
	MRESULT res;
	MHandle handle;
	string preloadPath;
	vector <ASF_FaceFeature> preLoadFeatureVec;

	char * APPID;
	char * SDKKey;
};


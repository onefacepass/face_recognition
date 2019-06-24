#pragma once
#include "stdafx.h"

class FaceDete
{
public:
	FaceDete();

	~FaceDete();

	void SetAPPID(const char appid[]);

	void SetSDKKey(const char sdkkey[]);

	void SetPreloadPath(string path);

	void SetConfLevel(MFloat Level);

	MFloat GetConfLevel();

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
	int Loadregface();

	/*
	* @ ����
	*	image [����]����������ͼ��
	*	detectedResult ʶ����Ľ��
	*	opt �Ƿ�ʶ����������preload���ݼ���
	*		true ��ʶ����������preload���ݼ���
	*		false ����ʶ����������preload���ݼ���
	* @ �޷���
	*/
	int DetectFaces(Mat &image, Json::Value &detectedResult);

	/*
	* @����
	*	result ��ͨ��DetectFaces�����ʶ����
	* @����ֵ
	*	����preLoadFeatureVecƥ��������ֵ������
	*   -1 ��ƥ��ģ����ǿ������������
	*		1) ȷʵû��ƥ���
	*		2) ƥ����С����ֵ
	*	>=0 ƥ��ɹ������� 
	*		
	*/
	int CompareFeature(DetectedResult& result);

	// ���µĺ���������debug/test����
	void DrawRetangle(Mat& frame, MInt32 faceRect[4]);

private:

	void GetFeaturefromImage(Mat & image, ASF_FaceFeature &feature);


private:
	MRESULT res;
	MHandle handle;
	string preloadPath;
	MFloat threshold_confidenceLevel;
	/*
	*	struct PreloadInfo {
	*		eature feature;
	*  		std::string filename;
	*	};
	*/
	vector <PreloadInfo> preLoadVec;

	Json::Value stuTable;

	char * APPID;
	char * SDKKey;
};


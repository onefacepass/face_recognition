#include "inc/FaceDete.h"

//#define USINGLIVEVIDEO
#define USINGIMAGE

int main() {
	FaceDete facedete;

	//facedete.GetVersion();
	facedete.SetPreloadPath("preload");
	facedete.SetConfLevel((MFloat)0.8);

	if (facedete.Loadregface() == 0)
		return 1;

	Json::Value detectedResult;
	MInt32 faceRect[4] = { 0 };
	Json::Value currFace;

#ifdef USINGLIVEVIDEO
	cv::VideoCapture cap(0);
	if (!cap.isOpened())
		return 1;

	cv::Mat frame;
	while (cap.isOpened())
	{
		cap >> frame;

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

		imshow("show", frame);
		if (waitKey(33) == 27) break;

		detectedResult.clear();
	}

#endif // USINGLIVEVIDEO

#ifdef USINGIMAGE
	//cv::Mat frame = cv::imread("sample/test1_Exist.png");
	cv::Mat frame = cv::imread("sample/aGroupofPeople_NotExist.png");
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

#endif // USINGIMAGE

	facedete.UninitEngine();
	return 0;
}
#include <cmath>
#include <vector>
#include "ftp.cpp"
#include <fstream>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <pthread.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <opencv2/ml/ml.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/dnn/dnn.hpp>
#include <opencv2/core/types.hpp>
using namespace cv::ml;
using namespace std;


bool enable_ftp = true;

void face(const cv::Ptr<SVM>& svm, cv::dnn::Net& embedder);
void extract(cv::dnn::Net& embedder);
void train(const cv::Ptr<SVM>& svm);
void recognize(const cv::Ptr<SVM>& svm, cv::dnn::Net& embedder);

void idcard();
void ocr_load(vector<cv::Mat>& refs);
void ocr(vector<cv::Mat>& refs);
// void pull_ocr(int a, int b, vector<cv::Mat>& refs);
void *pull_ocr(void* args);

void ftp_pull(const string& remote, const string& local);
void ftp_push(const string& local, const string& remote);


vector<int> known_names = vector<int>();							// Names of people
vector<cv::Mat> known_embeddings = vector<cv::Mat>();			// Collections of image features from dnn net

cv::Mat trainingDataMat;
cv::Mat labels;

int num_of_people = 5;
string photolist[20] = { 
	"1_a.jpg", "1_b.jpg", "1_c.jpg", "1_d.jpg",
	"2_a.jpg", "2_b.jpg", "2_c.jpg", "2_d.jpg",
	"3_a.jpg", "3_b.jpg", "3_c.jpg", "3_d.jpg",
	"4_a.jpg", "4_b.jpg", "4_c.jpg", "4_d.jpg",
	"5_a.jpg", "5_b.jpg", "5_c.jpg", "5_d.jpg"
};

vector<cv::Mat> refs = vector<cv::Mat>();

pthread_t tid[4];

string ftpip = "10.30.11.68";
string userpass = "ugv:ugv";
string user = "ugv";
string pass = "ugv";


int main(){
	ocr_load(refs);

	cout << "[INFO] loading face recognizer..." << endl;
	cv::dnn::Net embedder = cv::dnn::readNet("epoch40.onnx","","ONNX");	// Facenet-light pretrained with VGGFace2 dataset
	cv::Ptr<SVM> svm = SVM::create();


	string op;
	while (true){
		cout << "command: ";
		cin >> op;
		if (op == "id"){
			//idcard();
			for (int i = 1; i <= 20; i++){
				cout<<i<<endl;
				ftp_pull("id_database/" + to_string(i) + ".jpg", "images_idcard/" + to_string(i) + ".jpg");
			}
		}
		else if (op == "face"){
			face(svm, embedder);
		}
		else if (op == "quit"){
			cout << "[Done] Goodbye!"<< endl;
			exit(0);
		}
		else {
			cout << "[ERROR] Invalid command!"<< endl;
		}
	}

	return 0;
}


void face(const cv::Ptr<SVM>& svm, cv::dnn::Net& embedder){
	cout << "[INFO] pulling photos from ftp..." << endl;
	for (int i = 0; i < 20; i++){
		ftp_pull("face_database/" + photolist[i], "database/" + photolist[i]);
	}

	extract(embedder);

	train(svm);

	recognize(svm, embedder);
}

/* Extract image feature vectors by DL model induction*/
void extract(cv::dnn::Net& embedder){
	cout << "[INFO] Quantifying faces..." << endl;
	for (int i = 0; i < 20; i++){
		cout << "[INFO] processing image: " << i+1 << "/20" << endl;	// i -> i+1
		string img_path = "dataset/" + photolist[i];
cout<<img_path<<endl;		
		int intName = i / (20 / num_of_people) + 1;

		cv::Mat image = cv::imread(img_path);
		cv::Rect rect(170, 70, 300, 300);
		cv::Mat face = image(rect);
		// cv::imshow("face", face);
		// cv::waitKey(0);
		cv::Size face_size = cv::Size(96,96);				// Input size of the model should be 96x96
		cv::Scalar mean = cv::Scalar(0,0,0);

		cv::Mat face_blob = cv::dnn::blobFromImage(face, 1.0, face_size, mean, true, false, 0);
		embedder.setInput(face_blob);
		cv::Mat vec = embedder.forward();				// Shallow copy cause vec changes, remains to fixed
		known_names.push_back(intName);
		known_embeddings.push_back(vec.reshape(1, 1).clone());
		// cout << "name: " << known_names[i] << endl;
		// cout << "vec: " << known_embeddings[i] << endl;
	}
}

/* Train the SVM using face features extracted */
void train(const cv::Ptr<SVM>& svm){
    cout << "[INFO] Encoding labels..." << endl;
	int cols = known_embeddings[0].cols;

	trainingDataMat = cv::Mat(known_embeddings.size(), cols, CV_32FC1);
	for (int i = 0; i < known_embeddings.size(); i++){
		known_embeddings[i].row(0).copyTo(trainingDataMat.row(i));
	}
	labels = cv::Mat(known_names.size(), 1, CV_32SC1, &known_names[0]);	// must be integer
	// cout << "svm data: " << trainingDataMat << endl;
	// cout << "svm lable: " << labels << endl;

	svm->setType(SVM::C_SVC);
	svm->setKernel(SVM::LINEAR);
	svm->setTermCriteria(cv::TermCriteria(cv::TermCriteria::MAX_ITER, 100, 1e-6));

	cout << "[INFO] Training SVM model..." << endl;
	svm->trainAuto(trainingDataMat, ROW_SAMPLE, labels);			// Segfault. guess: if global var, text section not enough
	cout << "[INFO] Done." << endl;
}

/* Classify people through svm*/
void recognize(const cv::Ptr<SVM>& svm, cv::dnn::Net& embedder){
	cout << "[INFO] Recognizing..." << endl;

	string path = "image_face/target.jpg";
	cv::Mat image = cv::imread(path);
	cv::Rect rect(170, 70, 300, 300);
	cv::Mat face = image(rect);
	// cv::imshow("face", face);
	// cv::waitKey(0);

	cv::Size face_size = cv::Size(96,96);
	cv::Scalar mean = cv::Scalar(0,0,0);

	cv::Mat face_blob = cv::dnn::blobFromImage(face, 1.0, face_size, mean, true, false, 0);
	embedder.setInput(face_blob);
	cv::Mat vec = embedder.forward();

	cv::Mat result = cv::Mat();
	svm->predict(vec, result);
	cout << "result: " << result << endl;				// The result is already the name (1-5)
}


void idcard(){
	cout << "[INFO] Creating threads by pthread_create..." << endl;
	
	int status;
    	int num[] = { 1, 6, 11, 16 };
	for (int i=0; i<4; i++){
		status = pthread_create(&tid[i], NULL, &pull_ocr, num + i);				// doSomeThing is a test function
		if (status){
			cout << "[ERROR] Failed to create thread " << i+1 << ".  Return status: " << status << endl;
		}
		else {
			cout << "[INFO] Successfully create thread " << i+1 << endl;
		}
	}
	pthread_join(tid[0], NULL);
	pthread_join(tid[1], NULL);
	pthread_join(tid[2], NULL);
	pthread_join(tid[3], NULL);

    ofstream outfile;
    outfile.open("result.txt");

    for (int i = 0; i < 4; i++){
        string fname = to_string(num[i]) + ".txt";
        ifstream infile;
        infile.open(fname);

        string id;
        for (int j = 0; j < 5; j++){
            infile >> id;
            outfile << id << endl;
        }
    }

    outfile.close();

    ftp_push("result.txt", "result.txt");
	cout << "[INFO] ID done!" << endl;
}

void ocr_load(vector<cv::Mat>& refs){
	cout << "[INFO] loading reference..." << endl;
	string img_path = "ocr_ref.png";

	cv::Mat digits = cv::imread(img_path);
	cv::Mat tmp;
	cv::cvtColor(digits, tmp, cv::COLOR_BGR2GRAY);
	cv::threshold(tmp, digits, 127, 255, cv::THRESH_BINARY_INV);

	vector< vector<cv::Point> > contours;
	vector< cv::Vec4i > hierarchy;
	cv::findContours(digits, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

	for (int i = 0; i < contours.size(); i++){
		cv::Rect bounding = cv::boundingRect(contours[i]);
		cv::Mat tmp = digits(bounding);
		// cv::imshow("roi", tmp);
		// cv::waitKey(0);
		cv::Mat roi;
		cv::resize(tmp, roi, cv::Size(10, 16));
		refs.push_back(roi.clone());
	}
}

string ocr(string fpath, vector<cv::Mat>& refs){
	cv::Mat image = cv::imread(fpath);
	cv::Rect rect(45, 150, 400, 40);
	cv::Mat digits = image(rect);
	// cv::imshow("digits", digits);
	// cv::waitKey(0);
	cv::Mat tmp;
	cv::cvtColor(digits, tmp, cv::COLOR_BGR2GRAY);
	cv::threshold(tmp, digits, 127, 255, cv::THRESH_BINARY_INV);

	vector< vector<cv::Point> > contours;
	vector< cv::Vec4i > hierarchy;
	cv::findContours(digits, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
	// cout << "contours: " << endl;
	// for (int i = 0; i < contours.size(); i++){
	//     cout << contours[i] << endl;
	// }
	// cout << "hierarchy: " << endl;
	// for (int i = 0; i < hierarchy.size(); i++){
	//     cout << hierarchy[i] << endl;
	// }

	vector<char> output;
	for (int i = 0; i < contours.size(); i++){
		cv::Rect bounding = cv::boundingRect(contours[i]);
		cv::Mat tmp = digits(bounding);
		// cv::imshow("roi", tmp);
		// cv::waitKey(0);
		cv::Mat roi;
		cv::resize(tmp, roi, cv::Size(10, 16));

		int num = -1;
		int mxscore = -1;
		for (int j = 0; j < refs.size(); j++){
			int res_rows = roi.rows - refs[j].rows + 1;
			int res_cols = roi.cols - refs[j].cols + 1;
			cv::Mat res(res_rows, res_cols, CV_32FC1);
			cv::matchTemplate(roi, refs[j], res, cv::TM_CCOEFF);
			double min, score;
			cv::minMaxLoc(res, &min, &score);
			if (score > mxscore){
				mxscore = score;
				num = 10 - j;
			}
		}

		if (num == 10){
			output.push_back('X');
		}
		else{
			output.push_back('0' + num);
		}
	}

	string res = "";
	for (int i = output.size() - 1; i >= 0; i--){
		res += output[i];
	}
	return res;
}

/* void pull_ocr(int a, int b, vector<cv::Mat>& refs){
	ofstream outfile;
    	outfile.open(to_string(a) + ".txt");

	for (int i = a; i <= b; i++){
		string fname = to_string(i) + ".jpg";
		ftp_pull("id_database/" + fname, "images_idcard/" + fname);
		string res = ocr("images_idcard/" + fname, refs);
		outfile << res << endl;
	}
	
	outfile.close();
} */

void *pull_ocr(void* args){
    	int a = *(int*)args;
	cout<<a<<endl;
	ofstream outfile;
    	outfile.open(to_string(a) + ".txt");

	for (int i = a; i < a+5; i++){
		string fname = to_string(i) + ".jpg";
		ftp_pull("id_database/" + fname, "images_idcard/" + fname);
		string res = ocr("images_idcard/" + fname, refs);
		outfile << res << endl;
	}
	
	outfile.close();
	return NULL;
}


void ftp_pull(const string& remote, const string& local){
	if (!enable_ftp) return;

	//system(("curl -s ftp://" + ftpip + "/" + remote + " -u " + userpass + " -o " + local).c_str());
	// curl ftp://malu.me/size.zip –u name:passwd -o size.zip

	FtpDownload("ftp://" + ftpip + "/" + remote, local, user, pass);
}

void ftp_push(const string& local, const string& remote){
	if (!enable_ftp) return;
	
	//system(("curl -s -u " + userpass + " -T " + local + " ftp://" + ftpip + "/" + remote).c_str());
	// curl –u name:passwd -T size.mp3 ftp://malu.me/mp3/

	FtpUpload("ftp://" + ftpip + "/" + remote, local, user, pass);
}











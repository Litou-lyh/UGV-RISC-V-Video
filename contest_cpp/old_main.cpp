#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <unistd.h>
#include <sys/types.h>    
#include <sys/wait.h>
#include <opencv2/opencv.hpp>
#include <opencv2/dnn/dnn.hpp>
#include <opencv2/ml/ml.hpp>
#include <opencv2/core/types.hpp>
#include <pthread.h>
using namespace cv::ml;
using namespace std;

void init();

void face(const cv::Ptr<SVM>& svm, cv::dnn::Net& embedder);
void extract(cv::dnn::Net& embedder);
void train(const cv::Ptr<SVM>& svm);
void recognize(const cv::Ptr<SVM>& svm, cv::dnn::Net& embedder);

void idcard();
void ocr_load(vector<cv::Mat>& refs);
void ocr(vector<cv::Mat>& refs);
void pull_ocr(int a, int b, vector<cv::Mat>& refs);

void ftp_pull(const string& remote, const string& local);
void ftp_push(const string& local, const string& remote);

void *doSomeThing(void *arg);


vector<int> known_names = vector<int>();							// Names of people
vector<cv::Mat> known_embeddings = vector<cv::Mat>();			// Collections of image features from dnn net

cv::Mat trainingDataMat;
cv::Mat labels;

vector<cv::Mat> refs = vector<cv::Mat>();

pthread_t tid[3];
int range[][2] = { {1, 7}, {8, 14}, {15, 20} };

string ftpip = "";
string userpass = ":";

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
			idcard();
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
	extract(embedder);

	train(svm);

	recognize(svm, embedder);
}

/* Extract image feature vectors by DL model induction*/
void extract(cv::dnn::Net& embedder){
	cout << "[INFO] Quantifying faces..." << endl;

	string dataset_path = "dataset";					// no need of '/' at the end of the path
	vector<cv::String> img_paths;
	cv::glob(dataset_path, img_paths, true);
	cout << "[INFO] Total image: " << img_paths.size() << endl;

	for (int i = 0; i < img_paths.size(); i++){
		cout << "[INFO] processing image: " << i+1 << "/20" << endl;	// i -> i+1
		string img_path = img_paths[i];
		cout << img_path << endl;

		int p2 = img_path.find_last_of('/');
		int p1 = img_path.substr(0, p2).find_last_of('/');
		string name = img_path.substr(p1 + 1, p2 - p1 - 1);
		int intName = stoi(name);

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
	for (int i=0; i<3; i++){
		status = pthread_create(&tid[i], NULL, &doSomeThing, NULL);				// doSomeThing is a test function
		if (status){
			cout << "[ERROR] Failed to create thread " << i << ".  Return status: " << status << endl;
		}
		else cout << "[INFO] Successfully create thread " << i << endl;
	}
	pthread_join(tid[0], NULL);
	pthread_join(tid[1], NULL);
	pthread_join(tid[2], NULL);
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

void pull_ocr(int a, int b, vector<cv::Mat>& refs){
	ofstream outfile;
    outfile.open(itos(a) + ".txt");

	for (int i = a; i <= b; i++){
		string fname = itos(i) + ".jpg";
		ftp_pull("id_database/" + fname, "images_idcard/" + fname);
		string res = ocr("images_idcard/" + fname, refs);
		outfile << res << endl;
	}
	
	outfile.close();
}


void ftp_pull(const string& remote, const string& local){
	system("curl ftp://" + ftpid + "/" + remote + " -u " + userpass + " -o " + local);
	// curl ftp://malu.me/size.zip –u name:passwd -o size.zip
}

void ftp_push(const string& local, const string& remote){
	system("curl -u " + userpass + " -T " + local + " ftp://" + ftpip + "/" + remote);
	// curl –u name:passwd -T size.mp3 ftp://malu.me/mp3/
}




void *doSomeThing(void *arg)
{
    unsigned long i = 0;
    pthread_t id = pthread_self();

    if(pthread_equal(id, tid[0]))
    {
        printf("\n First thread processing\n"); 
    }
    else
    {
        printf("\n Second thread processing\n");
    }
    
    for(i=0; i<(0xFFFFFFFF); i++);

    return NULL;
}










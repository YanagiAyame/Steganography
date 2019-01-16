#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <vector>
#include <iostream>
#include <random>

// �u���b�N�� ( 2 �ׂ̂��� )
constexpr int window_size = 4;

// �摜�f�[�^�ɈӖ��f�[�^��D�荞�ށA���邢�͎��o�� ( ���g���̈� )
// img : �摜�f�[�^�Adata : �Ӗ��f�[�^�Ainv ? �摜�f�[�^����Ӗ��f�[�^�����o�� : �D�荞��
auto steganography(cv::Mat img, std::vector<bool>& data,bool inv=0) {
	const auto Row = img.rows;
	const auto Col = img.cols;

	img.convertTo(img, CV_32F, 1.0/255);

	int k = 0;
	for (int j = 0; j + window_size < Row; j += window_size) {
		for (int i = 0; i + window_size < Col; i += window_size) {

			if (k >= data.size()) {
				goto BREAK;
			}

			cv::Mat cut_img(img,cv::Rect(i, j, window_size, window_size));

			std::vector<cv::Mat>cut_planes;
			cv::split(cut_img, cut_planes);

			for (int c = 0; c < cut_planes.size(); ++c) {
				// �F��D��I�ɕω�������(�ɓ݊��Ȃ̂�)
				if (cut_planes.size() == 3)c = 2;

				cv::dct(cut_planes[c], cut_planes[c]);

				if (inv) {
					if (cut_planes[c].at<float>(window_size - 1, window_size - 1) > 0.1)data[k] = 1;
					else data[k] = 0;
				}
				else {
					cut_planes[c].at<float>(window_size - 1, window_size - 1) = data[k] * 0.2;
				}

				cv::idct(cut_planes[c], cut_planes[c]);
			}
			++k;

			cv::merge(cut_planes, cut_img);
		}
	}
BREAK:
	img.convertTo(img, CV_8U, 255);
	return img;
}

// img : ���f�[�^�As : �摜��PATH
bool read_img(cv::Mat& mat_img, const std::string& s) {
	mat_img = cv::imread(s);

	if (mat_img.empty()) return false;
	else return true;
}

bool output_img(const cv::Mat& mat_img) {
	if (mat_img.empty()) return false;

	cv::namedWindow("Image");
	cv::imshow("Image", mat_img);
	cv::waitKey(0);

	return true;
}

auto input_data(const int length) {
	std::vector<bool>data(length);
	std::cout << "Input string." << std::endl;
	std::string input;
	std::cin >> input;
	int idx = 0;
	for(auto c:input){
		for (int i = 0; i < CHAR_BIT; ++i) {
			data[idx++] = c & 1;
			c >>= 1;
		}
	}
	return data;
}

void out_str(const std::vector<bool>&data) {
	std::string str;
	int idx = 0;
	while (1) {
		char c = 0;
		for (int i = 0; i < CHAR_BIT; ++i) {
			c += (data[idx++] << i);
			if (idx >= data.size()) {
				goto BREAK;
			}
		}
		if (c == '\0')break;
		str += c;
	}
	BREAK:
	std::cout << str << std::endl;
}

auto make_random_data(const int length){
	std::vector<bool>data(length);
	std::random_device rnd;     // �񌈒�I�ȗ���������
	std::mt19937 mt(rnd());            // �����Z���k�E�c�C�X�^��32�r�b�g�ŁA�����͏����V�[�h
	for (int i = 0; i < length; ++i) {
		if (mt() > UINT32_MAX / 2)data[i] = 1;
		else data[i] = 0;
	}
	return data;
}

auto input_path() {
	std::cout << "Wright down the PATH(image file)." << std::endl;
	std::string path;
	std::cin >> path;
	return path;
}

auto pick_data() {
	std::cout << "read or write?( r / w )" << std::endl;
	char c;
	while (1) {
		std::cin >> c;
		if (c == 'r')return true;
		else if(c=='w')return false;
	}
}

int main(void)
{
	// �摜�̃p�X
	const std::string img_path = input_path();

	// �X�e�S�f�[�^
	cv::Mat box;

	// �摜�̓ǂݎ��
	if (!read_img(box, img_path)) {
		std::cout << "Failed : Cannot read image file." << std::endl;
		return -1;
	}

	const int max_bits = (box.rows / window_size)*(box.cols / window_size);

	if (pick_data()) {
		std::vector<bool> data(max_bits);
		steganography(box, data, 1);
		out_str(data);
	}
	else {
		// �e�X�g�p
		std::vector<bool> data = make_random_data(max_bits);

		//std::vector<bool> data = input_data(max_bits);

		auto origin = data;

		auto stego_img = steganography(box, data);

		steganography(stego_img, data, 1);

		int error = 0;
		for (int i = 0; i < max_bits; ++i) {
			if (data[i] != origin[i])error++;
		}
		std::cout << "error = " << error << "/" << max_bits << std::endl;

		//out_str(data);

		// �摜�̏o�͂���ѕۑ�
		if (!output_img(stego_img)) {
			std::cout << "Failed : Cannot output image file." << std::endl;
			return -1;
		}
		else {
			cv::imwrite("StegoImg.png", stego_img);
		}
	}
}
// Connected Component Labeling (CCL) - Label Equivalence

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <queue>
#include <list>
#include <algorithm>
#include <utility>
#include <cmath>
#include <functional>
#include <cstring>
#include <cmath>
#include <limits>

#define NOMINMAX

namespace nf_ccl {

void init_CCL(int L[], int R[], int N)
{
	for (int id = 0; id < N; id++) L[id] = R[id] = id;
}

inline int diff(int d1, int d2)
{
    return d1 - d2;
}

bool scanning(int D[], int L[], int R[], int N, int W)
{
	int m = false;

	for (int id = 0; id < N; id++) {
		int Did = D[id];
		int label = N;
		if (id - W >= 0 && diff(Did, D[id-W]) == 0) label = std::min(label, L[id-W]);
		if (id + W < N  && diff(Did, D[id+W]) == 0) label = std::min(label, L[id+W]);
		int r = id % W;
		if (r           && diff(Did, D[id-1]) == 0) label = std::min(label, L[id-1]);
		if (r + 1 != W  && diff(Did, D[id+1]) == 0) label = std::min(label, L[id+1]);

		if (label < L[id]) {
			R[L[id]] = label;
			m = true;
		}
	}

	return m;
}

void analysis(int D[], int L[], int R[], int N)
{
	for (int id = 0; id < N; id++) {
		int label = L[id];
		int ref;
		if (label == id) {
			do { label = R[ref = label]; } while (ref ^ label);
			R[id] = label;
		}
	}
}

void labeling(int D[], int L[], int R[], int N)
{
	for (int id = 0; id < N; id++) L[id] = R[R[L[id]]];
}

class CCL {
public:
	std::vector<int> ccl(std::vector<int>& data, int W);
};

std::vector<int> CCL::ccl(std::vector<int>& data, int W)
{
	int* D = static_cast<int*>(&data[0]);
	int N = data.size();
	int* L = new int[N];
	int* R = new int[N];

	init_CCL(L, R, N);

	for (;;) {
		if (scanning(D, L, R, N, W)) {
			analysis(D, L, R, N);
			labeling(D, L, R, N);
		} else break;
	}

	std::vector<int> result(L, L + N);

	delete [] L;
	delete [] R;

	return result;
}

int get_action(char c)
{
    switch (c) {
        case '#':
            return 1;
        default:
            break;
    }
    return 0;
}

void read_data(const std::string filename, std::vector<int>& data, int& W)
{
	std::fstream fs(filename.c_str(), std::ios_base::in);
	std::string line;
	std::vector<int> v;
	std::stringstream ss;

	while (getline(fs, line)) {
        W = line.length();
		ss.str("");  ss.clear();
		char c;
		for (ss.str(line), v.clear(); ss >> c; data.push_back(get_action(c)));
	}
}

#if 0
int main(int argc, char* argv[])
{
	std::ios_base::sync_with_stdio(false);

	if (argc < 2) {
		std::cerr << "Usage: " << argv[0] << " input_file" << std::endl;
		exit(1);
	}

	std::vector<int> data;
	int W;
	read_data(argv[1], data, W);

	CCL ccl;

	std::vector<int> result(ccl.ccl(data, W));

	std::cout << "Size: " << result.size() << std::endl; /// number of pixels
	std::cout << "Width: " << W << std::endl; /// width
    std::vector<int> memo;
	for (int i = 0; i < static_cast<int>(result.size()) / W; i++) {
		for (int j = 0; j < W; j++) {
            //std::cout << result[i*W+j] << " ";
            if (data[i*W+j] == 0 && find(memo.begin(), memo.end(), result[i*W+j]) == memo.end()) {
                memo.push_back(result[i*W+j]);
            }
        }
		//std::cout << std::endl;
	}
    for (auto x : memo) {
        std::cout << x << std::endl;
    }

	return 0;
}
#endif

} // namespace nf_ccl
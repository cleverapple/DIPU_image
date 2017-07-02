#include "ImageProcess.h"

void main() {
	int state;
	DIPU a;

	while (1) {
		try {
			//state = a.ImageProcess();
			state = a.test();
		}
		catch (...) {
			cout << "error" << state << endl;
			return;
		}
		waitKey(0);
		int getKey = getchar();
		if (getKey == '1') return;
	}
	return;
}


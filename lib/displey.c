#include <stdint.h>

void pixbufDrawLine(int x0, int y0, int x1, int y1) {
	int delta_x = ABS(x1 - x0);
	int delta_y = ABS(y1 - y0);
	int error = 0;
	
	if (delta_x >= delta_y) {
		//x-direction
		if (x0 > x1) {
			SWAP(x0, x1, int);
			SWAP(y0, y1, int);
		}
		int dir_y = SIGN(y1 - y0);
		int delta_err = delta_y + 1;
		//printf("delta_err = %d; delta_x = %d \r\n", delta_err, delta_x);
		int y = y0;
		for (int x=x0; x<=x1; x++) {
			pixbufSetPixel(x, y, true);
			error += delta_err;
			if (error >= delta_x + 1) {
				y += dir_y;
				error -= (delta_x + 1);
			}
		}
	} else { //y-direction
		if (y0 > y1) {
			SWAP(x0, x1, int);
			SWAP(y0, y1, int);
		}
		int dir_x = SIGN(x1 - x0);
		int delta_err = delta_x + 1;
		int x = x0;
		for (int y=y0; y<=y1; y++) {
			pixbufSetPixel(x, y, true);
			error += delta_err;
			if (error >= delta_y + 1) {
				x += dir_x;
				error -= (delta_y + 1);
			}
		}
	}
}

void pixbufDrawCircle(int x0, int y0, int r) {
	pixbufSetPixel(x0, y0, true);
	int x = x0;
	int y = y0 + r;
	int32_t r2 = r * r;
	int dx[3] = {1, 1, 0};
	int dy[3] = {0, -1, -1};
	while ((x < x0 + r) || (y > y0)) {
		// Выбираем пиксель, расстояние от которого до центра ~ r с минимальной погрешностью
		int n_best = 0;
		int r_new = SQR(x + dx[0] - x0) + SQR(y + dy[0] - y0);
		int best_error = ABS(r_new - r2);
		for (int n=1; n<3; n++) {
			r_new = SQR(x + dx[n] - x0) + SQR(y + dy[n] - y0);
			int error = ABS(r_new - r2);
			if (error < best_error)
				n_best = n;
		}
		pixbufSetPixel(x, y, true);
		// Mirror symmetry
		pixbufSetPixel(x, 2 * y0 - y, true);
		pixbufSetPixel(2 * x0 - x, y, true);
		pixbufSetPixel(2* x0 - x, 2*y0 - y, true);
		x += dx[n_best];
		y += dy[n_best];
	}
	// 2 Additional points
	pixbufSetPixel(x0 + r, y0, true);
	pixbufSetPixel(x0 - r, y0, true);
}
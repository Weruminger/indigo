//
//  indigo_guider_utils.c
//  indigo
//
//  Created by Peter Polakovic on 14/04/2019.
//  Copyright © 2019 CloudMakers, s. r. o. All rights reserved.
//
//  Based on libguider by Rumen Bogdanovski
//  Copyright © 2015 Rumen Bogdanovski All rights reserved.
//

#include <stdlib.h>
#include <math.h>
#include <stdio.h>

#include "indigo_bus.h"
#include "indigo_ccd_driver.h"
#include "indigo_guider_utils.h"

#define RE (0)
#define IM (1)
#define PI_2 (6.2831853071795864769252867665590057683943L)

static void _fft(const int n, const int offset, const int delta, const double (*x)[2], double (*X)[2], double (*_X)[2]);

static void fft(const int n, const double (*x)[2], double (*X)[2]) {
	double (*_X)[2] = malloc(2 * n * sizeof(double));
	_fft(n, 0, 1, x, X, _X);
	free(_X);
}

static void _fft(const int n, const int offset, const int delta, const double (*x)[2], double (*X)[2], double (*_X)[2]) {
	int n2 = n / 2;
	int k;
	double ccos, csin;
	int k00, k01, k10, k11;
	double tmp0, tmp1;
	if (n != 2) {
		_fft(n2, offset, 2 * delta, x, _X, X);
		_fft(n2, offset + delta, 2 * delta, x, _X, X);
		for (k = 0; k < n2; k++) {
			k00 = offset + k * delta;
			k01 = k00 + n2 * delta;
			k10 = offset + 2 * k * delta;
			k11 = k10 + delta;
			ccos = cos(PI_2 * k / (double)n);
			csin = sin(PI_2 * k / (double)n);
			tmp0 = ccos * _X[k11][RE] + csin * _X[k11][IM];
			tmp1 = ccos * _X[k11][IM] - csin * _X[k11][RE];
			X[k01][RE] = _X[k10][RE] - tmp0;
			X[k01][IM] = _X[k10][IM] - tmp1;
			X[k00][RE] = _X[k10][RE] + tmp0;
			X[k00][IM] = _X[k10][IM] + tmp1;
		}
	} else {
		k00 = offset;
		k01 = k00 + delta;
		X[k01][RE] = x[k00][RE] - x[k01][RE];
		X[k01][IM] = x[k00][IM] - x[k01][IM];
		X[k00][RE] = x[k00][RE] + x[k01][RE];
		X[k00][IM] = x[k00][IM] + x[k01][IM];
	}
}

static void ifft(const int n, const double (*X)[2], double (*x)[2]) {
	int n2 = n / 2;
	int i;
	double tmp0, tmp1;
	fft(n, X, x);
	x[0][RE] = x[0][RE] / n;
	x[0][IM] = x[0][IM] / n;
	x[n2][RE] = x[n2][RE] / n;
	x[n2][IM] = x[n2][IM] / n;
	for (i = 1; i < n2; i++) {
		tmp0 = x[i][RE] / n;
		tmp1 = x[i][IM] / n;
		x[i][RE] = x[n-i][RE] / n;
		x[i][IM] = x[n-i][IM] / n;
		x[n-i][RE] = tmp0;
		x[n-i][IM] = tmp1;
	}
}

//static void corellate(const int n, const double (*x1)[2], const double (*x2)[2], double (*c)[2]) {
//	int i;
//	double (*X1)[2] = malloc(2 * n * sizeof(double));
//	double (*X2)[2] = malloc(2 * n * sizeof(double));
//	double (*C)[2] = malloc(2 * n * sizeof(double));
//	fft(n, x1, X1);
//	fft(n, x2, X2);
//	/* pointwise multiply X1 conjugate with X2 here */
//	for (i = 0; i < n; i++) {
//		C[i][RE] = X1[i][RE] * X2[i][RE] + X1[i][IM] * X2[i][IM];
//		C[i][IM] = X1[i][IM] * X2[i][RE] - X1[i][RE] * X2[i][IM];
//	}
//	ifft(n, C, c);
//	free(X1);
//	free(X2);
//	free(C);
//}

static void corellate_fft(const int n, const double (*X1)[2], const double (*X2)[2], double (*c)[2]) {
	int i;
	double (*C)[2] = malloc(2 * n * sizeof(double));
	/* pointwise multiply X1 conjugate with X2 here, store in X1 */
	for (i = 0; i < n; i++) {
		C[i][RE] = X1[i][RE] * X2[i][RE] + X1[i][IM] * X2[i][IM];
		C[i][IM] = X1[i][IM] * X2[i][RE] - X1[i][RE] * X2[i][IM];
	}
	ifft(n, C, c);
	free(C);
}

static double find_distance(const int n, const double (*c)[2]) {
	int i;
	const int n2 = n / 2;
	int max=0;
	int prev, next;
	for (i = 0; i < n; i++) {
		max = (c[i][0] > c[max][0]) ? i : max;
	}
	/* find previous and next positions to calculate quadratic interpolation */
	if ((max == 0) || (max == n2)) {
		prev = n - 1;
		next = 1;
	} else if (max == (n-1)) {
		prev = n - 2;
		next = 0;
	} else {
		prev = max - 1;
		next = max + 1;
	}
	/* find subpixel offset of the maximum position using quadratic interpolation */
	double max_subp = (c[next][RE] - c[prev][RE]) / (2 * (2 * c[max][RE] - c[next][RE] - c[prev][RE]));
//	INDIGO_DEBUG(indigo_debug("max_subp = %5.2f max: %d -> %5.2f %5.2f %5.2f\n", max_subp, max, c[prev][0], c[max][0], c[next][0]));
	if (max == n2) {
		return max_subp;
	} else if (max > n2) {
		return (double)((max - n) + max_subp);
	} else if (max < n2) {
		return (double)(max + max_subp);
	} else { /* should not be reached */
		return 0;
	}
}

static int next_power_2(const int n) {
	int k = 1;
	while (k < n)
		k <<= 1;
	return k;
}

indigo_result indigo_centroid_frame_digest(indigo_raw_type raw_type, const void *data, const int width, const int height, indigo_frame_digest *c) {
	int i, ci, li, max;
	double m10, m01, m00;
	if ((width < 3) || (height < 3))
		return INDIGO_FAILED;
	if ((data == NULL) || (c == NULL))
		return INDIGO_FAILED;
	m10 = m01 = m00 = 0;
	ci = li = 0;
	max = width * height;
	for (i=0; i < max; i++) {
		switch (raw_type) {
			case INDIGO_RAW_MONO8: {
				int value = ((uint8_t *)data)[i];
				m10 += ci * value;
				m01 += li * value;
				m00 += value;
				break;
			}
			case INDIGO_RAW_MONO16: {
				int value = ((uint16_t *)data)[i];
				m10 += ci * value;
				m01 += li * value;
				m00 += value;
				break;
			}
			case INDIGO_RAW_RGB24: {
				int value = ((uint8_t *)data)[i] + ((uint8_t *)data)[i + 1] + ((uint8_t *)data)[i + 2];
				m10 += ci * value;
				m01 += li * value;
				m00 += value;
				break;
			}
			case INDIGO_RAW_RGB48: {
				int value = ((uint16_t *)data)[i] + ((uint16_t *)data)[i + 1] + ((uint16_t *)data)[i + 2];
				m10 += ci * value;
				m01 += li * value;
				m00 += value;
				break;
			}
		}
		ci++;
		if (ci == width) {
			ci = 0;
			li++;
		}
	}
	c->width = width;
	c->height = height;
	c->centroid_x = m10 / m00;
	c->centroid_y = m01 / m00;
	c->algorithm = centroid;
//	INDIGO_DEBUG(indigo_debug("c->centroid_x =%5.2f, c->centroid_y =%5.2f\n", c->centroid_x, c->centroid_y));
	return INDIGO_OK;
}

indigo_result indigo_donuts_frame_digest(indigo_raw_type raw_type, const void *data, const int width, const int height, indigo_frame_digest *c) {
	int i, ci, li, max;
	double (*col_x)[2];
	double (*col_y)[2];
	if ((width < 3) || (height < 3))
		return INDIGO_FAILED;
	if ((data == NULL) || (c == NULL))
		return INDIGO_FAILED;
	c->width = next_power_2(width);
	c->height = next_power_2(height);
	c->fft_x = malloc(2 * c->width * sizeof(double));
	c->fft_y = malloc(2 * c->height * sizeof(double));
	col_x = calloc(2 * c->width * sizeof(double), 1);
	col_y = calloc(2 * c->height * sizeof(double), 1);	
	/* collapse the frame in X and Y directions */
	ci = 0;
	li = 0;
	max = width * height;
	for (i=0; i < max; i++) {
		
		switch (raw_type) {
			case INDIGO_RAW_MONO8: {
				int value = ((uint8_t *)data)[i];
				col_x[ci][RE] += value;
				col_y[li][RE] += value;
				break;
			}
			case INDIGO_RAW_MONO16: {
				int value = ((uint16_t *)data)[i];
				col_x[ci][RE] += value;
				col_y[li][RE] += value;
				break;
			}
			case INDIGO_RAW_RGB24: {
				int value = ((uint8_t *)data)[i] + ((uint8_t *)data)[i + 1] + ((uint8_t *)data)[i + 2];
				col_x[ci][RE] += value;
				col_y[li][RE] += value;
				break;
			}
			case INDIGO_RAW_RGB48: {
				int value = ((uint16_t *)data)[i] + ((uint16_t *)data)[i + 1] + ((uint16_t *)data)[i + 2];
				col_x[ci][RE] += value;
				col_y[li][RE] += value;
				break;
			}
		}
		ci++;
		if (ci == width) {
			ci = 0;
			li++;
		}
	}
//	printf("col_x:");
//	for (i=0; i < fdigest->width; i++) {
//		printf(" %5.2f",col_x[i][RE]);
//	}
//	printf("\n");
//	printf("col_y:");
//	for (i=0; i < fdigest->height; i++) {
//		printf(" %5.2f",col_y[i][RE]);
//	}
//	printf("\n");
	fft(c->width, col_x, c->fft_x);
	fft(c->height, col_y, c->fft_y);
	c->algorithm = donuts;
	free(col_x);
	free(col_y);
	return INDIGO_OK;
}

indigo_result indigo_calculate_drift(const indigo_frame_digest *ref, const indigo_frame_digest *new, double *drift_x, double *drift_y) {
	if (ref == NULL || new == NULL || drift_x == NULL || drift_y == NULL)
		return INDIGO_FAILED;
	if ((ref->width != new->width) || (ref->height != new->height))
		return INDIGO_FAILED;
	if (ref->algorithm == centroid) {
		*drift_x = ref->centroid_x - new->centroid_x;
		*drift_y = ref->centroid_y - new->centroid_y;
		return INDIGO_OK;
	}
	if (ref->algorithm == donuts) {
		double (*c_buf)[2];
		int max_dim = (ref->width > ref->height) ? ref->width : ref->height;
		c_buf = malloc(2 * max_dim * sizeof(double));
		/* find X correction */
		corellate_fft(ref->width, ref->fft_x, new->fft_x, c_buf);
		*drift_x = find_distance(ref->width, c_buf);
		/* find Y correction */
		corellate_fft(ref->height, ref->fft_y, new->fft_y, c_buf);
		*drift_y = find_distance(ref->height, c_buf);
		free(c_buf);
		return INDIGO_OK;
	}
	return INDIGO_FAILED;
}

indigo_result indigo_delete_frame_digest(indigo_frame_digest *fdigest) {
	if (fdigest) {
		if (fdigest->algorithm == donuts) {
			if (fdigest->fft_x)
				free(fdigest->fft_x);
			if (fdigest->fft_y)
				free(fdigest->fft_y);
		}
		fdigest->width = 0;
		fdigest->height = 0;
		fdigest->algorithm = none;
		return INDIGO_OK;
	}
	return INDIGO_FAILED;
}

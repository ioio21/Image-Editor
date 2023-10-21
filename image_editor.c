// Copyright Tudor Ioana-Octavia 311CAa 2021-2022

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

#define SIZE 256

double edge[3][3] = {
	{-1, -1, -1},
	{-1, 8, -1},
	{-1, -1, -1}
};

double sharpen[3][3] = {
	{0, -1, 0},
	{-1, 5, -1},
	{0, -1, 0}
};

double box_blur[3][3] = {
	{1.0 / 9, 1.0 / 9, 1.0 / 9},
	{1.0 / 9, 1.0 / 9, 1.0 / 9},
	{1.0 / 9, 1.0 / 9, 1.0 / 9}
};

double gaussian_blur[3][3] = {
	{1.0 / 16, 1.0 / 8, 1.0 / 16},
	{1.0 / 8, 1.0 / 4, 1.0 / 8},
	{1.0 / 16, 1.0 / 8, 1.0 / 16}
};

// function for dynamic allocation of a matrix
double **alloc_matrix(int lines, int columns)
{
	// alloc lines
	double **matrix = (double **)calloc(lines, sizeof(double *));
	// verify the allocation
	if (!matrix) {
		free(matrix);
		return NULL;
	}

	// alloc columns
	for (int i = 0; i < lines; i++) {
		matrix[i] = calloc(columns, sizeof(double));

		if (!matrix[i]) {
			while (--i >= 0)
				free(matrix[i]);
			free(matrix);
			return NULL;
		}
	}
	return matrix;
}

// function for freeing the memory used by a matrix
void free_matrix(int lines, double **matrix)
{
	for (int i = 0; i < lines; i++)
		free(matrix[i]);
	free(matrix);
}

double **load_ascii(char *name, int *dimensions,
					char *type, int *actual_values)
{
	FILE *f = NULL;

	f = fopen(name, "rt");
	if (!f) {
		printf("Failed to load %s\n", name);
		return NULL;
	}

	// because we red with handle load the first three lines, skip them
	char skip;
	int skip_lines = 3;
	while (skip_lines != 0) {
		fscanf(f, "%c", &skip);
		if (skip == '\n')
			skip_lines--;
	}

	int rows = dimensions[0];
	int columns = dimensions[1];

	// if there's P3 type, multiply the collumns by 3
	if (strncmp(type, "P3", 2) == 0)
		columns *= 3;

	// Alloc matrix
	double **picture = NULL;
	picture = (double **)malloc(rows * sizeof(double *));
	for (int i = 0; i < rows; i++)
		picture[i] = (double *)malloc(columns * sizeof(double));

	// read the values
	for (int i = 0; i < rows; i++)
		for (int j = 0; j < columns; j++) {
			int nr;
			fscanf(f, "%d", &nr);
			picture[i][j] = (double)nr;
		}
	fclose(f);
	// update the new values
	actual_values[0] = 0;
	actual_values[1] = rows;
	actual_values[2] = actual_values[0];
	actual_values[3] = dimensions[1];

	// keep it for free
	dimensions[2] = rows;

	printf("Loaded %s\n", name);
	return picture;
}

// function for loading an image from a binary file
double **binary_img(char *name, int *dimensions,
					char *type, int *actual_values)
{
	FILE *f = NULL;
	f = fopen(name, "rb");

	if (!f) {
		printf("Failed to load %s\n", name);
		return NULL;
	}
	// because we red with handle load the first three lines, skip them
	char skip;
	int skip_lines = 3;
	while (skip_lines != 0) {
		fscanf(f, "%c", &skip);
		if (skip == '\n')
			skip_lines--;
	}
	int rows = dimensions[0];
	int columns = dimensions[1];

	// if there's P3 type, multiply the collumns by 3
	if (strncmp(type, "P6", 2) == 0)
		columns *= 3;

	// Alloc matrix
	double **picture = NULL;
	picture = (double **)malloc(rows * sizeof(double *));
	for (int i = 0; i < rows; i++)
		picture[i] = (double *)malloc(columns * sizeof(double));

	unsigned char read;
	for (int i = 0; i < rows; i++)
		for (int j = 0; j < columns; j++) {
			size_t retva = fread(&read, sizeof(unsigned char), 1, f);
			if ((int)retva < -1)
				exit(-1);
			picture[i][j] = (int)read;
		}
	fclose(f);
	// update the new values
	actual_values[0] = 0;
	actual_values[1] = rows;
	actual_values[2] = actual_values[0];
	actual_values[3] = dimensions[1];

	// keep it for free
	int val = dimensions[0];
	dimensions[2] = val;

	printf("Loaded %s\n", name);
	return picture;
}

void all_select(int *actual_size, int *dimensions)
{
	int row = dimensions[0];
	int col = dimensions[1];
	actual_size[0] = 0;
	actual_size[1] = row;
	actual_size[2] = 0;
	actual_size[3] = col;
	printf("Selected ALL\n");
}

int handle_load(char *name, int *dimensions, char *type, int *vals)
{
	FILE *f = NULL;

	// in this function we try to open the file
	f = fopen(name, "rt");
	if (!f) {
		printf("Failed to load %s\n", name);
		return 0;
	}
	// if succed, just read the type and size of matrix and the max value
	int rows, columns;
	fscanf(f, "%s %d %d %d", type, &columns, &rows, &vals[0]);
	dimensions[0] = rows;
	dimensions[1] = columns;
	fclose(f);
	return 1;
}

void save_binary(char *name, double **picture, int *dimensions, char *type,
				 int *vals)
{
	FILE *f = NULL;
	if (name[strlen(name) - 1] == ' ')
		name[strlen(name) - 1] = '\0';
	f = fopen(name, "wb");
	if (!f) {
		printf("No image loaded\n");
		return;
	}

	int rows = dimensions[0];
	int columns = dimensions[1];
	if (strncmp(type, "P3", 2) == 0 || strncmp(type, "P6", 2) == 0)
		columns *= 3;

	// Change the image number if there's for ascii
	if (strncmp(type, "P3", 2) == 0)
		strcpy(type, "P6");

	if (strncmp(type, "P2", 2) == 0)
		strcpy(type, "P5");

	// write to output
	fprintf(f, "%s\n%d %d\n%d\n", type, dimensions[1], rows, vals[0]);

	// write the elements of matrix
	for (int i = 0; i < rows; i++)
		for (int j = 0; j < columns; j++) {
			unsigned char write = (unsigned char)(round(picture[i][j]));
			fwrite(&write, sizeof(unsigned char), 1, f);
		}

	fclose(f);
	printf("Saved %s\n", name);
}

void get_strings(char *line, char tokens[][70])
{
	int counter = 0, j = 0;

	for (int i = 0;; i++) {
		if (line[i] != ' ') {
			tokens[counter][j] = line[i];
			j++;
		} else {
			tokens[counter][j++] = '\0';
			counter++;
			j = 0;
		}
		if (line[i] == '\0')
			break;
	}
}

void do_select(int *actual_values, int *dimensions, int x1, int y1,
			   int x2, int y2)
{
	int col_0, col_1, row_0, row_1;

	//rows
	row_0 = (y1 < y2) ? y1 : y2;
	row_1 = (y1 < y2) ? y2 : y1;
	// columns
	col_0 = (x1 < x2) ? x1 : x2;
	col_1 = (x1 < x2) ? x2 : x1;
	// verify if the coordinates are valid
	if (col_0 >= 0 && row_0 >= 0 && col_1 <= dimensions[1] &&
		row_1 <= dimensions[0] && (col_0 != col_1 && row_0 != row_1)) {
		if (row_0 != col_0 || row_1 != col_1) {
			// save the area
			// lines
			actual_values[0] = row_0;
			actual_values[1] = row_1;

			// columns
			actual_values[2] = col_0;
			actual_values[3] = col_1;
			printf("Selected %d %d %d %d\n", x1, y1, x2, y2);
		} else {
			// save the area
			// lines
			actual_values[0] = row_0;
			actual_values[1] = row_1;

			// columns
			actual_values[2] = col_0;
			actual_values[3] = col_1;
			printf("Selected %d %d %d %d\n", row_0, col_0, row_1, col_1);
		}
	} else {
		printf("Invalid set of coordinates\n");
		return;
	}
}

void save_ascii(char *name, double **picture, int *dimensions, char *type,
				int *vals)
{
	FILE *f = NULL;
	name[strlen(name) - 6] = '\0';
	f = fopen(name, "wt");
	if (!f) {
		printf("No image loaded\n");
		return;
	}

	int rows = dimensions[0];
	int columns = dimensions[1];

	if (strncmp(type, "P3", 2) == 0 || strncmp(type, "P6", 2) == 0)
		columns *= 3;

	// change the magic number
	if (strncmp(type, "P6", 2) == 0)
		strcpy(type, "P3");

	if (strncmp(type, "P5", 2) == 0)
		strcpy(type, "P2");

	// write the image
	fprintf(f, "%s\n%d %d\n%d\n", type,
			dimensions[1], dimensions[0], vals[0]);

	//write the image
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < columns; j++)
			fprintf(f, "%d ", (int)round(picture[i][j]));
		fprintf(f, "\n");
	}

	fclose(f);
	printf("Saved %s\n", name);
}

double **handle_crop(double **picture, int *actual_size,
					 int *dimensions, char *type)
{
	int ac_3 = actual_size[2];
	int ac_4 = actual_size[3];
	int ac_1 = actual_size[0];
	int ac_2 = actual_size[1];

	int cols = dimensions[1];
	// if there is P3 or P6 multiply by 3
	if (strncmp(type, "P3", 2) == 0 || strncmp(type, "P6", 2) == 0) {
		cols *= 3;
		ac_3 *= 3;
		ac_4 *= 3;
	}

	double **new_image = (double **)malloc((ac_2 - ac_1) * sizeof(int *));
	for (int i = 0; i < ac_2 - ac_1; i++)
		new_image[i] = (double *)malloc((ac_4 - ac_3) * sizeof(double));

	// // copy the elements
	for (int i = ac_1; i < ac_2; i++) {
		for (int j = ac_3; j < ac_4; j++) {
			int new_i = i - ac_1;
			int new_j = j - ac_3;
			new_image[new_i][new_j] = picture[i][j];
		}
	}

	free_matrix(dimensions[2], picture);

	picture = (double **)malloc((ac_2 - ac_1) * sizeof(double *));
	for (int i = 0; i < ac_2 - ac_1; i++)
		picture[i] = (double *)malloc((ac_4 - ac_3) * sizeof(double));

	int r = ac_2 - ac_1;
	int c = ac_4 - ac_3;
	for (int i = 0; i < r; i++)
		for (int j = 0; j < c; j++)
			picture[i][j] = new_image[i][j];

	free_matrix(r, new_image);
	if (strncmp(type, "P3", 2) != 0 && strncmp(type, "P6", 2) != 0) {
		dimensions[0] = ac_2 - ac_1;
		dimensions[1] = ac_4 - ac_3;
	} else {
		dimensions[0] = ac_2 - ac_1;
		dimensions[1] = (int)((ac_4 - ac_3) / 3);
	}
	printf("Image cropped\n");

	actual_size[0] = 0;
	actual_size[1] = dimensions[0];
	actual_size[2] = 0;
	actual_size[3] = dimensions[1];
	dimensions[2] = dimensions[0];
	return picture;
}

int check_angle(int how_much)
{
	int get_module = abs(how_much);
	int how_many_times = 0;
	// check if it is a multiple of 90 degrees
	if (get_module % 90 == 0 &&
		get_module >= 0 &&
		get_module <= 360) {
		printf("Rotated %d\n", how_much);

		// check how many times we have to rotate
		if (how_much == 0 || get_module == 360)
			return how_many_times;
		if (how_much == -90 || how_much == 270)
			return how_many_times + 1;
		if (get_module == 180)
			return how_many_times + 2;
		if (how_much == 90 || how_much == -270)
			return how_many_times + 3;
	}
	printf("Unsupported rotation angle\n");
	return 0;
}

void rotate_90_color(double **picture, int *actual_values)
{
	int v0 = actual_values[0];
	int v1 = actual_values[1];
	int v2 = actual_values[2];
	int v3 = actual_values[3];

	// alloc matrix for auxiliary
	double **new_image;
	new_image = (double **)malloc((v1 - v0) * sizeof(double *));
	for (int i = 0; i < v1 - v0; i++)
		new_image[i] = (double *)malloc((v3 - v2) * sizeof(double));

	// copy image
	for (int i = v0; i < v1; i++) {
		for (int j = v2; j < v3; j++) {
			int new_i = j - v2;
			int new_j = i - v0;
			new_image[new_i][new_j] = picture[i][j];
		}
	}
	// reverse matrix
	for (int i = 0; i < (v1 - v0) / 2; i++) {
		for (int j = 0; j < (v1 - v0); j++) {
			int idx = (v1 - v0) - i - 1;
			int temp = new_image[i][j];
			new_image[i][j] = new_image[idx][j];
			new_image[idx][j] = temp;
		}
	}

	// copy back
	for (int i = v0; i < v1; i++) {
		for (int j = v2; j < v3; j++) {
			int new_i = i - v0;
			int new_j = j - v2;
			picture[i][j] = new_image[new_i][new_j];
		}
	}

	free_matrix(v1 - v0, new_image);
}

void r_color_left(double **picture, int *actual_values)
{
	int v0 = actual_values[0];
	int v1 = actual_values[1];
	int v2 = 3 * actual_values[2];
	int v3 = 3 * actual_values[3];
	int row = actual_values[1] - actual_values[0];

	// dynamic allocation of auxiliary matrix
	double **new_image;
	new_image = (double **)malloc((v1 - v0) * sizeof(double *));
	for (int i = 0; i < v1 - v0; i++)
		new_image[i] = (double *)malloc((v3 - v2) * sizeof(double));

	// copy image
	for (int i = v0; i < v1; i++) {
		for (int j = v2; j < v3; j++) {
			int new_i = i - v0;
			int new_j = j - v2;
			new_image[new_i][new_j] = picture[i][j];
		}
	}
	// transpose matrix
	for (int i = 0; i < row; i++) {
		for (int j = i + 1; j < row; j++) {
			int m_i = i * 3;
			int m_j = j * 3;
			int temp = new_image[i][m_j];
			new_image[i][m_j] = new_image[j][m_i];
			new_image[j][m_i] = temp;

			temp = new_image[i][m_j + 1];
			new_image[i][m_j + 1] = new_image[j][m_i + 1];
			new_image[j][m_i + 1] = temp;

			temp = new_image[i][m_j + 2];
			new_image[i][m_j + 2] = new_image[j][m_i + 2];
			new_image[j][m_i + 2] = temp;
		}
	}
	// reverse columns
	int new_r = row / 2;
	int new_r2 = row * 3;
	for (int i = 0; i < new_r; i++) {
		for (int j = 0; j < new_r2; j++) {
			int temp = new_image[i][j];
			int idx = row - i - 1;
			new_image[i][j] = new_image[idx][j];
			new_image[idx][j] = temp;
		}
	}

	// copy to image
	for (int i = v0; i < v1; i++) {
		for (int j = v2; j < v3; j++) {
			int new_i = i - v0;
			int new_j = j - v2;
			picture[i][j] = new_image[new_i][new_j];
		}
	}

	free_matrix(v1 - v0, new_image);
}

// check how is the image
void r_left_90(double **picture, int *actual_values, char *type)
{
	if (strncmp(type, "P3", 2) != 0 && strncmp(type, "P6", 2) != 0)
		rotate_90_color(picture, actual_values);
	else
		r_color_left(picture, actual_values);
}

//function which rotates the matrix
void rotate_area(int how, double **picture, int *actual_values, char *type)
{
	int how_much = 0;
	// check if it is 90 * k
	int absH = abs(how);
	int vdiff1 = actual_values[1] - actual_values[0];
	int vdiff2 = actual_values[3] - actual_values[2];
	if (absH % 90 == 0 && absH >= 0 && absH <= 360) {
		// verify if it issquare
		if (abs(vdiff1) == abs(vdiff2)) {
			if (how == 0 || how == 360 || how == -360) {
				printf("Rotated %d\n", how);
				return;
			}
			if (how == -90 || how == 270)
				how_much += 1;
			if (how == 180 || how == -180)
				how_much += 2;
			if (how == 90 || how == -270)
				how_much += 3;

			for (int i = 0; i < how_much; i++)
				r_left_90(picture, actual_values, type);

			printf("Rotated %d\n", how);
			return;
		}
		printf("The selection must be square\n");
		return;
	}
	printf("Unsupported rotation angle\n");
}

// function to rotate matrix 90 grades
double **do_all(double **picture, int *dimensions,
				int *actual_values, char *type)
{
	if (strncmp(type, "P3", 2) == 0 || strncmp(type, "P6", 2) == 0) {
		int row = dimensions[1], col = dimensions[0];
		double **new_image = (double **)malloc(row * sizeof(double *));
		for (int i = 0; i < row; i++)
			new_image[i] = (double *)malloc(3 * col * sizeof(double));
		for (int i = 0; i < row; i++) {
			for (int j = 0; j < col; j++) {
				int new_i = i * 3, new_j = j * 3;
				new_image[i][new_j] = picture[j][new_i];
				new_image[i][new_j + 1] = picture[j][new_i + 1];
				new_image[i][new_j + 2] = picture[j][new_i + 2];
			}
		}
		int new_row = row / 2, new_col = col * 3;
		for (int i = 0; i < new_row; i++) {
			for (int j = 0; j < new_col; j++) {
				int temp = new_image[i][j], idx = row - i - 1;
				new_image[i][j] = new_image[idx][j];
				new_image[idx][j] = temp;
			}
		}
		int aux = dimensions[2];
		dimensions[0] = row;
		dimensions[1] = col;
		dimensions[2] = row;
		free_matrix(aux, picture);
		picture = (double **)malloc(row * sizeof(double *));
		for (int i = 0; i < row; i++)
			picture[i] = (double *)malloc(3 * col * sizeof(double));
		for (int i = 0; i < row; i++) {
			int new_col = col * 3;
			for (int j = 0; j < new_col; j++)
				picture[i][j] = new_image[i][j];
		}
		actual_values[0] = 0;
		actual_values[1] = row;
		actual_values[2] = 0;
		actual_values[3] = col;
		free_matrix(row, new_image);
		return picture;
	} else if (strncmp(type, "P2", 2) == 0 || strncmp(type, "P5", 2) == 0) {
		int row = dimensions[1], col = dimensions[0];
		double **new_image = (double **)malloc(row * sizeof(double *));
		for (int i = 0; i < row; i++)
			new_image[i] = (double *)malloc(col * sizeof(double));
		for (int i = 0; i < row; i++)
			for (int j = 0; j < col; j++)
				new_image[i][j] = picture[j][i];
		// reverse columns
		int new_r = row / 2;
		for (int i = 0; i < new_r; i++) {
			for (int j = 0; j < col; j++) {
				int temp = new_image[i][j], idx = row - i - 1;
				new_image[i][j] = new_image[idx][j];
				new_image[idx][j] = temp;
			}
		}
		int aux = dimensions[2];
		dimensions[0] = row;
		dimensions[1] = col;
		dimensions[2] = row;
		free_matrix(aux, picture);
		picture = (double **)malloc(row * sizeof(double *));
		for (int i = 0; i < row; i++)
			picture[i] = (double *)malloc(col * sizeof(double));
		for (int i = 0; i < row; i++)
			for (int j = 0; j < col; j++)
				picture[i][j] = new_image[i][j];
		actual_values[0] = 0;
		actual_values[1] = row;
		actual_values[2] = 0;
		actual_values[3] = col;
		free_matrix(row, new_image);
		return picture;
	}
	return NULL;
}

void apply(double **picture, int *actual_values, char *type,
		   double filter[3][3], char *finish, int *size)
{
	if (strcmp(type, "P2") == 0 || strcmp(type, "P5") == 0) {
		printf("Easy, Charlie Chaplin\n");
		return;
	}

	double **new_matrix = NULL;
	int rows = size[0];
	int cols = size[1];
	new_matrix = (double **)malloc(rows * sizeof(double *));
	for (int i = 0; i < rows; i++)
		new_matrix[i] = (double *)malloc(3 * cols * sizeof(double));

	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols * 3; j++)
			new_matrix[i][j] = picture[i][j];
	}

if (strcmp(type, "P3") == 0 || strcmp(type, "P6") == 0) {
	int j_first = actual_values[2] * 3;
	int j_last = actual_values[3] * 3;
	for (int i = actual_values[0]; i < actual_values[1]; i++) {
		for (int j = j_first; j < j_last; j += 3) {
			if (i == 0 || i == rows - 1 ||
				j == 0 || j == (cols * 3 - 3)) {
				new_matrix[i][j] = picture[i][j];
				new_matrix[i][j + 1] = picture[i][j + 1];
				new_matrix[i][j + 2] = picture[i][j + 2];
			} else {
				double red = 0, green = 0, blue = 0;
				for (int x = i - 1; x <= i + 1; ++x) {
					for (int y = (j - 3); y <= (j + 3); y += 3) {
						red += ((double)picture[x][y] *
								filter[x - (i - 1)][(y - (j - 3)) / 3]);
						green += ((double)picture[x][y + 1] *
								filter[x - (i - 1)][(y - (j - 3)) / 3]);
						blue += ((double)picture[x][y + 2] *
								filter[x - (i - 1)][(y - (j - 3)) / 3]);
					}
				}
				if (red > 255.f)
					red = 255;
				else if (red < 0)
					red = 0;

				if (blue > 255.f)
					blue = 255;
				else if (blue < 0)
					blue = 0;

				if (green > 255.f)
					green = 255;
				else if (green < 0)
					green = 0;
				new_matrix[i][j] = (red);
				new_matrix[i][j + 1] = (green);
				new_matrix[i][j + 2] = (blue);
			}
		}
	}
}

	for (int i = 0; i < rows; i++)
		for (int j = 0; j < cols * 3; j++)
			picture[i][j] = new_matrix[i][j];

	free_matrix(rows, new_matrix);
	printf("APPLY %s done\n", finish);
}

int get_nr_params(char *line, int nr_params)
{
	for (size_t i = 0; i < strlen(line); i++) {
		if (line[i] == ' ')
			nr_params++;
	}

	return nr_params;
}

void handle_apply(int isloaded, char *line, double **picture,
				  int *actual_vales, char *type, int *dimensions)
{
	int nr_params = get_nr_params(line, 1);
	if (isloaded == 0) {
		printf("No image loaded\n");
	} else {
		if (strncmp(line + 6, "SHARPEN", 8) == 0) {
			apply(picture, actual_vales, type, sharpen, line + 6, dimensions);
		} else if (strncmp(line + 6, "EDGE", 4) == 0) {
			apply(picture, actual_vales, type, edge, line + 6, dimensions);
		} else if (strncmp(line + 6, "BLUR", 4) == 0) {
			apply(picture, actual_vales, type, box_blur, line + 6, dimensions);
		} else if (strncmp(line + 6, "GAUSSIAN_BLUR", 14) == 0) {
			apply(picture, actual_vales, type,
				  gaussian_blur, line + 6, dimensions);
		} else {
			if (nr_params != 1)
				printf("APPLY parameter invalid\n");
			else
				printf("Invalid command\n");
		}
	}
}

void handle_rotate(int isloaded, char *line, int *actual_vales,
				   double ***picture, char *type, int *dimensions)
{
	if (isloaded == 1) {
		int how_much = atoi(line);
		if (actual_vales[0] != 0 || actual_vales[1] != dimensions[0] ||
			actual_vales[2] != 0 || actual_vales[3] != dimensions[1]) {
			rotate_area(how_much, *picture, actual_vales, type);
		} else {
			int doC = check_angle(how_much);
			while (doC > 0) {
				*picture = do_all(*picture, dimensions, actual_vales, type);
				doC--;
			}
		}
	} else {
		printf("No image loaded\n");
	}
}

int main(void)
{
	double **picture;
	char line[SIZE], type[30], params[30][70];
	int dimensions[5], vals[5], isloaded = 0, actual_vales[5];
	while (1) {
		memset(line, 0, SIZE);
		fgets(line, SIZE, stdin);
		if (line[strlen(line) - 1] == '\n')
			line[strlen(line) - 1] = '\0';
		if (strncmp(line, "LOAD", 4) == 0) {
			if (!handle_load(line + 5, dimensions, type, vals)) {
				if (isloaded == 1 && dimensions[2] != 0)
					free_matrix(dimensions[2], picture);
				isloaded = 0;
			} else {
				if (isloaded == 1 && dimensions[2] != 0)
					free_matrix(dimensions[2], picture);
				if (strcmp(type, "P2") == 0 || strcmp(type, "P3") == 0)
					picture = load_ascii(line + 5, dimensions,
										 type, actual_vales);
				else if (strcmp(type, "P5") == 0 || strcmp(type, "P6") == 0)
					picture = binary_img(line + 5, dimensions,
										 type, actual_vales);

				isloaded = 1;
			}
		} else if (strncmp(line, "EXIT", 4) == 0) {
			if (isloaded == 0) {
				printf("No image loaded\n");
				return 0;
			}
			free_matrix(dimensions[2], picture);
			return 0;
		} else if (strncmp(line, "SAVE", 4) == 0) {
			if (isloaded == 1) {
				int nr_params = get_nr_params(line, 0);
				int res = strncmp((line + strlen(line) - 5), "ascii", 5);
				if (nr_params == 1 || (nr_params == 2 && res != 0))
					save_binary(line + 5, picture, dimensions, type, vals);
				else
					save_ascii(line + 5, picture, dimensions, type, vals);
			} else {
				printf("No image loaded\n");
			}
		} else if (strncmp(line, "SELECT", 6) == 0) {
			if (isloaded == 1) {
				int nr_params = get_nr_params(line, 0);
				get_strings(line, params);
				if (strncmp(line + 7, "ALL", 3) == 0) {
					all_select(actual_vales, dimensions);
				} else if (nr_params == 4 && isdigit(params[4][0]) != 0) {
					int x1 = (int)atoi(params[1]), y1 = (int)atoi(params[2]);
					int x2 = (int)atoi(params[3]), y2 = (int)atoi(params[4]);
					do_select(actual_vales, dimensions, x1, y1, x2, y2);
				} else if (nr_params == 4 && params[1][0] == '-') {
					printf("Invalid set of coordinates\n");
				} else if (nr_params != 4 || nr_params != 1) {
					printf("Invalid command\n");
				}
			} else {
				printf("No image loaded\n");
			}
		} else if (strncmp(line, "CROP", 4) == 0) {
			if (isloaded == 1)
				picture = handle_crop(picture, actual_vales,
									  dimensions, type);
			else
				printf("No image loaded\n");
		} else if (strncmp(line, "ROTATE", 6) == 0) {
			handle_rotate(isloaded, line + 7, actual_vales,
						  &picture, type, dimensions);
		} else if (strncmp(line, "APPLY", 5) == 0) {
			handle_apply(isloaded, line, picture,
						 actual_vales, type, dimensions);
		} else {
			printf("Invalid command\n");
		}
	}
	return 0;
}

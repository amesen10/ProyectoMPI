#include <mpi.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <list>
using namespace std;

#define DEBUG

void uso(string nombre_prog);

void obt_args(char* argv[], int& numeroPersonas, double& infeccion, double& recuperacion, int& duracion, double& infectadas, int& size);



void simulacion() {}


int main(int argc, char* argv[]) {
	int mid;
	int cnt_proc;
	MPI_Status mpi_status;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &mid);
	MPI_Comm_size(MPI_COMM_WORLD, &cnt_proc);

#ifdef debug
	if (mid == 0)
		cin.ignore();
	MPI_Barrier(MPI_COMM_WORLD);
#endif


	//aqui va el codigo
	int numeroPersonas, duracion, size, matrizSize;
	double infeccion, recuperacion, infectadas;
	obt_args(argv, numeroPersonas, infeccion, recuperacion, duracion, infectadas, size);

	//si el usuario pone 1 entonces la matriz es de 100, si es 2 entonces es de 500 si es 3 entonces es de 1000
	if (size == 1) {
		matrizSize = 100;

	}
	else if (size == 2) {
		matrizSize = 500;
	}
	else if (size == 3) {
		matrizSize = 1000;
	}
	else if (size == 4) {
		matrizSize = 6;
	}

	cout << "su matriz es de tama;o " << matrizSize << " x " << matrizSize << endl << endl;

	vector<vector<list<int>>>matriz;

	matriz.resize(matrizSize);

	for (auto &it : matriz)
	{
		it.resize(matrizSize);
	}








	MPI_Barrier(MPI_COMM_WORLD);
	cin.ignore();
	MPI_Finalize();

	return 0;
}


void uso(string nombre_prog) {
	cerr << nombre_prog.c_str() << " secuencia de parametros de entrada " << endl;
	exit(0);
}

void obt_args(char* argv[], int& numeroPersonas, double& infeccion, double& recuperacion, int& duracion, double& infectadas, int& size) {

	numeroPersonas = strtol(argv[1], NULL, 10);
	infeccion = stod(argv[2]);
	recuperacion = stod(argv[3]);
	duracion = strtol(argv[4], NULL, 10);
	infectadas = stod(argv[5]);
	size = strtol(argv[6], NULL, 10);

	if (numeroPersonas < 0)
		cout << "\t";


}
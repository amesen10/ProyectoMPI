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
	{
		do{
		cout << "\t Numero de personas invalido, digite otra cifra [0, 10.000.000]: ";
		cin>>numeroPersonas;
		while(numeroPersonas < 0);
	}
	if (infeccion < 0 || infeccion>1)
	{
		do{
		cout << "\t Probabilidad infecciosa invalida, digite otra cifra [0, 1]: ";
		cin>>infeccion;
		while(infeccion < 0 || infeccion>1);
	}
	if (recuperacion < 0 || recuperacion>1)
	{
		do{
		cout << "\t Probabilidad de recuperacion invalida, digite otra cifra [0, 1]: ";
		cin>>recuperacion;
		while(recuperacion < 0 || recuperacion>1);
	}
	if (duracion < 5 || duracion>50)
	{
		do{
		cout << "\t Duracion infecciosa maxima invalida, digite otra cifra [0, 50]: ";
		cin>>duracion;
		while(duracion < 5 || duracion>50);
	}
	if (infectadas < 0 || infectadas>1)
	{
		do{
		cout << "\t Porcentaje personas incialmente infectadas invalido, digite otra cifra [0, 10]: ";
		cin>>infectadas;
		while(infectadas < 0 || infectadas>1);
	}
	if (size < 1 || size>3)
	{
		do{
			cout << "\t Tamano invalido, digite otra cifra: \n"
			 << "\t    1) 100x100"	<<endl
			 << "\t    2) 500x500"	<<endl
			 << "\t    3) 1000x1000"<<endl;
			cin>>size;
		}while(size < 1 || size>3);
	}
	switch(size)
	{
		case 1: size=6/*100*/; break;
		case 2: size=500; break;
		case 3: size=1000; break;
	}
}
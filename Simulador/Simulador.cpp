#include <mpi.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <list>
#include <random>
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
	int nPersonas, duracion, size, tics;
	int tInfectadas, tSanas, tCuradas, tInmunes, tMuertas;	//t=total
	double infeccion, recuperacion, infectadas;
	double tPared;	//t=tiempo
	int veces;	//Será el numero de personas entre la cantidad de procesos
	//int randomXY;
	obt_args(argv, nPersonas, infeccion, recuperacion, duracion, infectadas, size);

	//cout << "su matriz es de tamano "<< size<<"x"<<size << endl << endl;
	default_random_engine generator;
	uniform_int_distribution <int> distributionXY(0, size-1);
	uniform_int_distribution <int> distribution12(1, 2);

	int *matriz = (int*)calloc(nPersonas *4+1, sizeof(int));	//guardar -1 en el ultimo para saber que allí termina
	
	if(mid==0)
		veces = nPersonas / cnt_proc;	//Se hace la division una sola vez par ahorrar por cuestiones de eficiencia en tiempo
	MPI_Bcast(&veces, 1, MPI_INT, 0, MPI_COMM_WORLD);
	
	int nInicialInfectados = nPersonas * infectadas;
	//cout << "II: " << nInicialInfectados<<endl;
	/*   Iniciar Infeccion   (Infectados iniciales)*/
	for (int iter = 0; iter < nInicialInfectados*4; iter+=4)
	{
		*(matriz + iter) = 3;							//Estado 3 (Infectado)
		*(matriz + iter + 1) = 0;						//Dias infectados
		*(matriz + iter + 2) = distributionXY(generator);	//Posición en Eje-X
		*(matriz + iter + 3) = distributionXY(generator);	//Posición en Eje-Y
		cout<<*(matriz + iter)
			<<" "<<*(matriz + iter + 1)
			<<" X "<<*(matriz + iter + 2) 
			<<" Y "<<*(matriz + iter + 3)			
			<<"\t\t";
	}
	//cout << "II: " << nInicialInfectados << endl;

	/*Paralelizar bien este FOR que le asigna a una persona un espacio*/

	for (int iter = nInicialInfectados*4; iter < (nPersonas*4)/cnt_proc; ++iter)
	{
//#  ifdef DEBUG
//		cout << "iter = " << iter << endl;
//#  endif
		*(matriz + iter) = distribution12(generator);		//Estado 1 (Inmune) o 2 (Sano)
		*(matriz + iter + 1) = 0;							//Dias infectados
		*(matriz + iter + 2) = distributionXY(generator);	//Posición en Eje-X
		*(matriz + iter + 3) = distributionXY(generator);	//Posición en Eje-Y
		cout << *(matriz + iter)
			<< " " << *(matriz + iter + 1)
			<< " X " << *(matriz + iter + 2)
			<< " Y " << *(matriz + iter + 3)
			<< "\t\t";
	}

	//poner iteradores X y Y para desplazarse por "matriz"

	//vector<vector<list<int>>>matriz;

	/*matriz.resize(matrizSize);

	for (auto &it : matriz)
	{
		it.resize(matrizSize);
	}*/

	MPI_Barrier(MPI_COMM_WORLD);
	if (mid == 0)
	{
		//cout << endl << endl << "Time: " << elapsed << "s" << endl;
		cin.ignore();
	}
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
		do {
			cout << "\t Numero de personas invalido, digite otra cifra [0, 10.000.000]: ";
			cin >> numeroPersonas;
		} while (numeroPersonas < 0);
	}
	if (infeccion < 0 || infeccion>1)
	{
		do {
			cout << "\t Probabilidad infecciosa invalida, digite otra cifra [0, 1]: ";
			cin >> infeccion;
		} while (infeccion < 0 || infeccion>1);
	}
	if (recuperacion < 0 || recuperacion>1)
	{
		do {
			cout << "\t Probabilidad de recuperacion invalida, digite otra cifra [0, 1]: ";
			cin >> recuperacion;
		} while (recuperacion < 0 || recuperacion>1);
	}
	if (duracion < 5 || duracion>50)
	{
		do {
			cout << "\t Duracion infecciosa maxima invalida, digite otra cifra [0, 50]: ";
			cin >> duracion;
		} while (duracion < 5 || duracion>50);
	}
	if (infectadas < 0 || infectadas>1)
	{
		do {
			cout << "\t Porcentaje personas incialmente infectadas invalido, digite otra cifra [0, 10]: ";
			cin >> infectadas;
		} while (infectadas < 0 || infectadas>1);
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
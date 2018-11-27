#include <mpi.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <list>
#include <random>
using namespace std;

#define DEBUG
# define T 4 
void uso(string nombre_prog);

void obt_args(char* argv[], int& numeroPersonas, double& infeccion, double& recuperacion, int& duracion, double& infectadas, int& size);

void iniciar(int *matriz, int& nPersonas, int& nInicialInfectados, int &size)
{
	default_random_engine generator;
	uniform_int_distribution <int> distributionXY(0, size - 1);
	uniform_int_distribution <int> distribution12(1, 2);
	for (int iter = 0; iter < nInicialInfectados * T; iter += T)
	{
		*(matriz + iter) = 3;							//Estado 3 (Infectado)
		*(matriz + iter + 1) = 0;						//Dias infectados
		*(matriz + iter + 2) = distributionXY(generator);	//Posición en Eje-X
		*(matriz + iter + 3) = distributionXY(generator);	//Posición en Eje-Y
	}
	for (int iter = nInicialInfectados * 4; iter<nPersonas * T; iter += T)
	{
		*(matriz + iter) = distribution12(generator);		//Estado 1 (Inmune) o 2 (Sano)
		*(matriz + iter + 1) = 0;							//Dias infectados
		*(matriz + iter + 2) = distributionXY(generator);	//Posición en Eje-X
		*(matriz + iter + 3) = distributionXY(generator);	//Posición en Eje-Y
	}
}

void imprimir(int const *matriz, int& nPersonas)
{
	for (int iter = 0; iter < nPersonas * T; iter += T)
	{
		cout << *(matriz + iter)
			<< " " << *(matriz + iter + 1)
			<< " X " << *(matriz + iter + 2)
			<< " Y " << *(matriz + iter + 3)
			<< "\t\t";
	}
}

void validar(int *matriz, int& nPersonas, int& cnt_proc, int& iter,int& duracion, recuperacion, int& nInfectados)
{
	int enfermosRestantes=0;
	validar(matriz, nPersonas, tInfectadas, tSanas, tCuradas, tInmunes, tMuertas);

	for (iter; iter < (nPersonas / cnt_proc)*T; iter+=T)
	{
		if (*(matriz + iter) == 3)	//Si está infectado:
		{	//	1-Busca infectar a los que pueda
			for (int i = 0; i < nPersonas*T; i += T)
			{
				
				if (*(matriz + iter + 2) == *(matriz + i + 2) &&	//Si el enfermo está en la misma celda que el enfermo y además está sano
					*(matriz + iter + 3) == *(matriz + i + 3) &&
					*(matriz + i) == 2	)
				{
					//Hacer el calculo de la probabilidad y asignar el nuevo estado en caso de darse el contagio
					*(matriz + iter + 1) = 3;
				}
				//NOTA: Falta el incremento de probabilidad por haber varios enfermos en una misma celda >:)

			}
			//	2-Verifica si ya es tiempo de morir o de recuperarse
			if (*(matriz + iter + 1) == duracion)
			{
				if()//Calcular probabilidad de recuperacion o de muerte de acuerdo a  @recuperacion
				*(matriz + iter + 1) = 0;		//Se actualiza el estado a Muerto=0
												//Incrementar muertos totales (tMuertas)
				else
					*(matriz + iter + 1) = 1;	//Se convierte a Inmune
												//Actualizar el contador de Curadas
				
			}	
		}
		
	}

	//return enfermosRestantes;
}

void simulacion(int *matriz, int& nPersonas, int& cnt_proc, int& nInfectados, int& size) 
{
	default_random_engine generator;
	uniform_int_distribution <int> distributionXY(-1, 1);
	for (int iter = 0; iter < nPersonas*T; iter += T)
	{
		*(matriz + iter) = *(matriz + iter);
		if (*(matriz + iter) == 3)							//Estado 3 (Infectado)
			matriz[iter + 1] = matriz[iter + 1] + 1;	//Dias infectados
		else
			matriz[iter + 1] = matriz[iter + 1];
		*(matriz + iter + 2) = (*(matriz + iter + 2) + distributionXY(generator)) % size; //Movimiento en Eje-X
		*(matriz + iter + 3) = (*(matriz + iter + 3) + distributionXY(generator)) % size; //Movimiento en Eje-Y
	}
	cout << "\n\n MOVIMIENTO\n\n";
	for (int iter = 0; iter < nPersonas * T; iter += T)
	{
		cout << *(matriz + iter)
			<< " " << *(matriz + iter + 1)
			<< " X " << *(matriz + iter + 2)
			<< " Y " << *(matriz + iter + 3)
			<< "\t\t";
	}
}

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

	obt_args(argv, nPersonas, infeccion, recuperacion, duracion, infectadas, size);
	

	int *matriz = new int[nPersonas*4];	//guardar -1 en el ultimo para saber que allí termina

	MPI_Bcast(&veces, 1, MPI_INT, 0, MPI_COMM_WORLD);
	//#ifdef DEBUG
	//		cout << "Veces = " << veces << endl;
	//	#  endif
	int nInicialInfectados = nPersonas * infectadas;
	//int tamanio = (nPersonas - nInicialInfectados) / (cnt_proc - 1) * 5;
	//int *matrizTemporal = (int*)calloc(tamanio, sizeof(int));
	int enfermosRestantes = nInicialInfectados;
	if (mid == 0)
	{
		iniciar(matriz, nPersonas, nInicialInfectados, size);	//Comienza la simulación
		imprimir(matriz, nPersonas);
	}

	while (enfermosRestantes != 0)
	{
		MPI_Bcast(matriz, nPersonas - 1, MPI_INT, 0, MPI_COMM_WORLD);	//Comparte la matriz con todos los procesos
		//VALIDAR
		//Cada proceso debe ejecutar una parte nPersonas/#procesos y recorrer todo el vector para contagiar o morir
		validar(matriz, nPersonas, cnt_proc, nPersonas/cnt_proc*mid/*INICIO*/, duracion, recuperacion, tInfectadas, tSanas, tCuradas, tInmunes, tMuertas);
		//SIMULAR
		if (mid == 0)
			simulacion(matriz, nPersonas, cnt_proc, nInicialInfectados, size);

		if (mid == 0)
		{
			imprimir(matriz, nPersonas);
		}
		--enfermosRestantes;
		
		++tics;
	}
	//hacer un reduce a tics

	
#ifdef DEBUG	//Impresion en otro proceso par acomprobar el Bcast	
	if (mid == 1)
		for (int iter = 0; iter < nPersonas * T; iter += T)
		{
			cout << *(matriz + iter)
				<< " " << *(matriz + iter + 1)
				<< " X " << *(matriz + iter + 2)
				<< " Y " << *(matriz + iter + 3)
				<< "\t\t";
		}
#endif



	MPI_Barrier(MPI_COMM_WORLD);
	free(matriz);		//Liberación de la memoria ocupada
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
		do {
			cout << "\t Tamano invalido, digite otra cifra: \n"
				<< "\t    1) 100x100" << endl
				<< "\t    2) 500x500" << endl
				<< "\t    3) 1000x1000" << endl;
			cin >> size;
		} while (size < 1 || size>3);
	}
	switch (size)
	{
	case 1: size = 6/*100*/; break;
	case 2: size = 500; break;
	case 3: size = 1000; break;
	}
}
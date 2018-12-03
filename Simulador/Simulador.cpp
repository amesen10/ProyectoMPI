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

	//aqui estamos llenando el vector con los enfermos que se tienen como parametro
	for (int iter = 0; iter < nInicialInfectados * T; iter += T)
	{
		*(matriz + iter) = 3;							//Estado 3 (Infectado)
		*(matriz + iter + 1) = 0;						//Dias infectados
		*(matriz + iter + 2) = distributionXY(generator);	//Posición en Eje-X
		*(matriz + iter + 3) = distributionXY(generator);	//Posición en Eje-Y
	}

	//llenamos el vector con la demas cantidad de personas que no estan infectadas, se hace un random que decide si es inmune o sano
	for (int iter = nInicialInfectados * 4; iter<nPersonas * T; iter += T)
	{
		*(matriz + iter) = distribution12(generator);		//Estado 1 (Inmune) o 2 (Sano)
		*(matriz + iter + 1) = 0;							//Dias infectados
		*(matriz + iter + 2) = distributionXY(generator);	//Posición en Eje-X
		*(matriz + iter + 3) = distributionXY(generator);	//Posición en Eje-Y
	}
}

//metodo que imprime el vector
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

//metodo que verifica si una persona se puede contagiar y dependiendo de cuantos enfermos esten en esa posicion aumenta la posibilidad de enfermarse de los sanos
void validar(int *arreglito, int *matriz, int& nPersonas, int& cnt_proc, int& iter, int& duracion, double& recuperacion, double& infeccion, int& nInfectados, int& enfermosRestantes)
{
	default_random_engine generator;
	uniform_real_distribution <double> proba(0, 1);
	//int enfermosRestantes=0;
	enfermosRestantes=0;
	int  posicionEnfermos=1, contador=0;
	for (contador; contador < (nPersonas / cnt_proc)*T; contador += T)
	{
		posicionEnfermos = 1;
		/*cout<<"ARREGLITO\n" << *(arreglito + iter)
			<< " " << *(arreglito + iter + 1)
			<< " X " << *(arreglito + iter + 2)
			<< " Y " << *(arreglito + iter + 3)
			<< "\t\t";*/

		if (*(arreglito + contador) == 3)	//Si está infectado:
		{	//	1-Busca infectar a los que pueda
			++enfermosRestantes;
			//cout << "Infectado\n";
			for (int i = 0; i < nPersonas*T; i += T)		//Busca los enfermos en la misma posición que el enfermo actual
			{

				if (*(arreglito + contador + 2) == *(matriz + i + 2) &&	
					*(arreglito + contador + 3) == *(matriz + i + 3) &&
					*(matriz + i) == 3)
				{
					++posicionEnfermos;
				}

			}
			
			for (int i = 0; i < nPersonas*T; i += T)
			{
				
				if (*(arreglito + contador + 2) == *(matriz + i + 2) &&	//Si el enfermo está en la misma celda que el enfermo y además está sano
					*(arreglito + contador + 3) == *(matriz + i + 3) &&
					*(matriz + i) == 2	)
				{
					//Hacer el calculo de la probabilidad y asignar el nuevo estado en caso de darse el contagio
					if (proba(generator)*posicionEnfermos < infeccion)
					{
						*(arreglito + contador + 1) = 3;
						++enfermosRestantes;
						cout << "\t INFECTA" << endl;
					}
						
				}
			}
			//	2-Verifica si ya es tiempo de morir o de recuperarse
			if (*(arreglito + contador + 1) == duracion)
			{
				if (proba(generator) < recuperacion)//Calcular probabilidad de recuperacion o de muerte de acuerdo a  @recuperacion
				{
					*(arreglito + contador + 1) = 0;		//Se actualiza el estado a Muerto=0
					cout << "\t MUERE" << endl;	//Incrementar muertos totales (tMuertas)
				}
				else
				{
					*(arreglito + contador + 1) = 1;	//Se convierte a Inmune i.e. se sana
					cout << "\t CURA" << endl;	//Actualizar el contador de Curadas
				}
				--enfermosRestantes;
			}	
		} 
	}
	//return enfermosRestantes;
}

//mueve x y y, si esta infectado aumenta el tiempo que ha estado infectado
void simulacion(int *arreglito, int *matriz, int& nPersonas, int& cnt_proc, int& nInfectados, int& size, int inicio)
{
	default_random_engine generator;
	uniform_int_distribution <int> distributionXY(-1, 1);
	for (int iter = 0; iter < nPersonas/cnt_proc*T; iter += T)
	{
		
		if (*(arreglito + iter) == 3)							//Estado 3 (Infectado)
			arreglito[iter + 1] = arreglito[iter + 1] + 1;	//Dias infectados
		
		*(arreglito + iter + 2) = (*(arreglito + iter + 2) + distributionXY(generator)) % size; //Movimiento en Eje-X
		if (*(arreglito + iter + 2) < 0)
			*(arreglito + iter + 2) = size-1;
		*(arreglito + iter + 3) = (*(arreglito + iter + 3) + distributionXY(generator)) % size; //Movimiento en Eje-Y
		if (*(arreglito + iter + 3) < 0)
			*(arreglito + iter + 3) = size-1;
	}
	cout << "\n\n MOVIMIENTO\n\n";
	for (int iter = 0; iter < nPersonas/cnt_proc * T; iter += T)
	{
		cout << *(arreglito + iter)
			<< " " << *(arreglito + iter + 1)
			<< " X " << *(arreglito + iter + 2)
			<< " Y " << *(arreglito + iter + 3)
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
	int nPersonas, duracion, size, tics=0;
	int tInfectadas, tSanas, tCuradas, tInmunes, tMuertas;	//t=total
	double infeccion, recuperacion, infectadas;
	double tPared;	//t=tiempo
	int veces;	//Será el numero de personas entre la cantidad de procesos

	obt_args(argv, nPersonas, infeccion, recuperacion, duracion, infectadas, size);
	

	int *matriz = new int[nPersonas*4];	//guardar -1 en el ultimo para saber que allí termina
	//int *matriz2 = new int[nPersonas * 4];

	//MPI_Bcast(&veces, 1, MPI_INT, 0, MPI_COMM_WORLD);
	//#ifdef DEBUG
	//		cout << "Veces = " << veces << endl;
	//	#  endif
	int nInicialInfectados = nPersonas * infectadas;
	//int tamanio = (nPersonas - nInicialInfectados) / (cnt_proc - 1) * 5;
	//int *matrizTemporal = (int*)calloc(tamanio, sizeof(int));
	int enfermosRestantes = 0, enfermosTic=0;
	if (mid == 0)		//Creación de la matriz en el proceso principal
	{
		cout << "\t MATRIZ INICIAL" << endl << endl;
		iniciar(matriz, nPersonas, nInicialInfectados, size);	//Comienza la simulación
		imprimir(matriz, nPersonas);
	}

	do
	{
		cout << "Enfermos Tic: " << enfermosTic <<" @ "<<mid<<endl<<endl;
		MPI_Bcast(matriz, nPersonas - 1, MPI_INT, 0, MPI_COMM_WORLD);	//Comparte la matriz con todos los procesos

		int *temp = new int[nPersonas*T];
		int *arreglito = new int[(nPersonas / cnt_proc) *T];
		//temp=matriz;
		MPI_Scatter(matriz, nPersonas/cnt_proc*T, MPI_INT, arreglito, nPersonas / cnt_proc * T, MPI_INT, 0, MPI_COMM_WORLD);

		//VALIDAR: Realiza los contagios, sanaciones o muertes de cada persona
		int inicio = (int)((nPersonas / cnt_proc) * mid*T);
		validar(arreglito, matriz, nPersonas, cnt_proc, inicio, duracion, recuperacion, infeccion, tInfectadas, enfermosTic/*, tSanas, tCuradas, tInmunes, tMuertas*/);
		
		
		//SIMULAR: realiza los movimientos
		simulacion(arreglito,matriz, nPersonas, cnt_proc, nInicialInfectados, size, inicio);

		if (mid == 0)
		{
			cout << "\nIMPRESION MATRIZ" << endl;
			imprimir(matriz, nPersonas);
		}

		MPI_Allreduce(&enfermosTic, &enfermosRestantes, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);	/*Provoca que se encicle, ver nota al final*/
		//--enfermosRestantes;
		cout << "Enfermos restantes: " << enfermosRestantes << " @ " << mid << endl;
		++tics;
		//poner una instruccion que todos los procesos compartan lo que modificaron
		/* NOTA (POSIBLE SOLUCION):
				Crear un arreglo temporal que guarde el rango de la matriz que usa cada proceso, 
				para luego con un AllGather juntarlos todos y guardarlo en la, o en otra matriz, 
				final hacer un Bcast para luego hacer otra tic
		*/
		//int *temp = new int[nPersonas*T];

		/*
				OTRA POSIBLE SOLUCION:
			crear un arreglo temporal que tenga toda la matriz para poder ir comparando
			con un scatter, distribuir la matriz entre los procesos y recorrerla como en validar
			finalizar con un allgather a matriz
		*/

		MPI_Allgather(arreglito, (nPersonas / cnt_proc) *T, MPI_INT, matriz, (nPersonas / cnt_proc) *T, MPI_INT, MPI_COMM_WORLD);
		/*if (mid == 0)
		{
			MPI_Gather(arreglito, (nPersonas / cnt_proc) *T, MPI_INT, temp, (nPersonas / cnt_proc) *T, MPI_INT,0, MPI_COMM_WORLD);
			
		}
		else
		{
			MPI_Gather(arreglito, (nPersonas / cnt_proc) *T, MPI_INT, temp, (nPersonas / cnt_proc) *T, MPI_INT, 0, MPI_COMM_WORLD);
		}
		MPI_Barrier(MPI_COMM_WORLD);*/
			//matriz = temp;
		free(temp);
		free(arreglito);
		//++enfermosRestantes;
	} while (enfermosRestantes !=0);
	//hacer un reduce a tics

	MPI_Barrier(MPI_COMM_WORLD);
#ifdef DEBUG	//Impresion en otro proceso par acomprobar el Bcast	
	if (mid == 1)
	{
		cout << "MATRIZ DESDE PROCESO FINAL" << endl;
		imprimir(matriz, nPersonas);
	}
#endif
	free(matriz);		//Liberación de la memoria ocupada
	int n;
	MPI_Allreduce(&tics, &n, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
	if (mid == 0)
	{
		
		//cout << endl << endl << "Time: " << elapsed << "s" << endl;
		cout << endl << "TICS totales: " <<tics<< endl;
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
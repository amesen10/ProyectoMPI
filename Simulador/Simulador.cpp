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
		*(matriz + iter + 2) = distributionXY(generator);	//Posici�n en Eje-X
		*(matriz + iter + 3) = distributionXY(generator);	//Posici�n en Eje-Y
	}

	//llenamos el vector con la demas cantidad de personas que no estan infectadas, se hace un random que decide si es inmune o sano
	for (int iter = nInicialInfectados * 4; iter<nPersonas * T; iter += T)
	{
		*(matriz + iter) = distribution12(generator);		//Estado 1 (Inmune) o 2 (Sano)
		*(matriz + iter + 1) = 0;							//Dias infectados
		*(matriz + iter + 2) = distributionXY(generator);	//Posici�n en Eje-X
		*(matriz + iter + 3) = distributionXY(generator);	//Posici�n en Eje-Y
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

//M�todo que verifica si una persona se puede contagiar y dependiendo de cuantos enfermos esten en esa posicion aumenta la posibilidad de enfermarse de los sanos
void validar(int *arreglito, int *matriz, int& nPersonas, int& cnt_proc, int& iter, int& duracion, double& recuperacion, double& infeccion, int& nInfectados, int& tCuradas, int& tMuertas, int& tSanas)
{
	/*NOTA: los enfermosRestantes se pueden estar contando doble, mejor hacer un solo ciclo que busque cu�ntos enfermos hay, incluso se pdoria hacer en otro m�todo*/
	default_random_engine generator;
	uniform_real_distribution <double> proba(0, 1);
	//int enfermosRestantes=0;
	//enfermosRestantes=0;
	int  posicionEnfermos=1, contador=0;
	for (contador; contador < (nPersonas / cnt_proc)*T; contador += T)
	{	
		/*cout<<"ARREGLITO\n" << *(arreglito + iter)
			<< " " << *(arreglito + iter + 1)
			<< " X " << *(arreglito + iter + 2)
			<< " Y " << *(arreglito + iter + 3)
			<< "\t\t";*/		
		if (*(arreglito + contador) == 2)	//Persona sana
			++tSanas;
		else
		{
			posicionEnfermos = 1;
			if (*(arreglito + contador) == 3)	//Si est� infectado:
			{	//	1-Busca infectar a los que pueda
				//++enfermosRestantes;
				//cout << "Infectado\n";
				for (int i = 0; i < nPersonas*T; i += T)		//Busca los enfermos en la misma posici�n que el enfermo actual
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

					if (*(arreglito + contador + 2) == *(matriz + i + 2) &&	//Si el enfermo est� en la misma celda que el enfermo y adem�s est� sano
						*(arreglito + contador + 3) == *(matriz + i + 3) &&
						*(matriz + i) == 2)
					{
						//Hacer el calculo de la probabilidad y asignar el nuevo estado en caso de darse el contagio
						if (proba(generator)*posicionEnfermos < infeccion)
						{
							*(arreglito + contador + 1) = 3;
							//++enfermosRestantes;
							++nInfectados;
							--tSanas;
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
						++tMuertas;
					}
					else
					{
						*(arreglito + contador + 1) = 1;	//Se convierte a Inmune i.e. se sana
						cout << "\t CURA" << endl;	//Actualizar el contador de Curadas
						++tCuradas;
					}
					//--enfermosRestantes;
				}
			}
		}
			
	}
	//return enfermosRestantes;
}

//Mueve a la persona si no est� muerta; en caso de estar infectada, aumenta el tiempo que ha estado infectado
void simulacion(int *arreglito, int *matriz, int& nPersonas, int& cnt_proc, int& nInfectados, int& size, int inicio)
{
	default_random_engine generator;
	uniform_int_distribution <int> distributionXY(0, 1);
	for (int iter = 0; iter < nPersonas/cnt_proc*T; iter += T)
	{
		if (*(arreglito + iter) == 3)							//Estado 3 (Infectado)
			arreglito[iter + 1] = arreglito[iter + 1] + 1;	//Dias infectados
		
		if (*(arreglito + iter) != 0)	//Si la persona no est� muerta (Estado 0) se desplaza por el espacio 
		{
			if (!distributionXY(generator))	//se mover� un espacio a la derecha
			{
				*(arreglito + iter + 2) = (*(arreglito + iter + 2) -1); //Movimiento en Eje-X
				if (*(arreglito + iter + 2) < 0)
					*(arreglito + iter + 2) = size - 1;
			}
			else   //Movimiento hacia la izquierda
				*(arreglito + iter + 2) = (*(arreglito + iter + 2) + 1) % size;
			
			if (!distributionXY(generator))	//se mover� un espacio hacia abajo
			{
				*(arreglito + iter + 3) = (*(arreglito + iter + 3) -1); //Movimiento en Eje-Y
				if (*(arreglito + iter + 3) < 0)
					*(arreglito + iter + 3) = size - 1;
			}
			else	//se mover� un espacio hacia arriba
				*(arreglito + iter + 3) = (*(arreglito + iter + 3) +1) % size;
			
		}
	}
	/*cout << "\n\n MOVIMIENTO\n\n";
	for (int iter = 0; iter < nPersonas/cnt_proc * T; iter += T)
	{
		cout << *(arreglito + iter)
			<< " " << *(arreglito + iter + 1)
			<< " X " << *(arreglito + iter + 2)
			<< " Y " << *(arreglito + iter + 3)
			<< "\t\t";
	}*/
}

int cuentaInfectados(int *arreglito, int& nPersonas, int& cnt_proc)
{
	int enfermosRestantes = 0;
	for (int i=0; i < (nPersonas / cnt_proc)*T; i += T)
	{
		if (*(arreglito + i) == 3)
			++enfermosRestantes;
	}
	return enfermosRestantes;
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
	int tInfectadas, tSanas, tCuradas, tInmunes, tMuertas;	//Contadores para las estad�sticas de cada tic
	int infectadasT=0, sanasT=0, curadasT=0, inmunesT=0, muertasT=0;	//Contadores para las estad�sticas finales
	double infeccion, recuperacion, infectadas;
	double tPared;	//t=tiempo
	int veces;	//Ser� el numero de personas entre la cantidad de procesos

	obt_args(argv, nPersonas, infeccion, recuperacion, duracion, infectadas, size);
	
	int *matriz = new int[nPersonas*4];	//puntero al a un arreglo que simula dos dimensiones por donde se van a desplazar las personas
	//int *matriz2 = new int[nPersonas * 4];

	int nInicialInfectados = nPersonas * infectadas;
	//int *matrizTemporal = (int*)calloc(tamanio, sizeof(int));
	
	if (mid == 0)		//Creaci�n de la matriz en el proceso principal
	{
		cout << "\t MATRIZ INICIAL" << endl << endl;
		iniciar(matriz, nPersonas, nInicialInfectados, size);	//Comienza la simulaci�n
		imprimir(matriz, nPersonas);
	}
	int enfermosRestantes = 0, enfermosTic = 0;
	do
	{
		enfermosTic = 0; tInfectadas = 0, tCuradas=0, tSanas=0, tMuertas=0;			//Establece los valores para las estadisticas de cada tic
		cout << "Enfermos Tic: " << enfermosTic <<" @ "<<mid<<endl<<endl;
		MPI_Bcast(matriz, nPersonas - 1, MPI_INT, 0, MPI_COMM_WORLD);	//Comparte el espacio (matriz) con todos los procesos

		int *temp = new int[nPersonas*T];
		int *arreglito = new int[(nPersonas / cnt_proc) *T];
		//temp=matriz;
		MPI_Scatter(matriz, nPersonas/cnt_proc*T, MPI_INT, arreglito, nPersonas / cnt_proc * T, MPI_INT, 0, MPI_COMM_WORLD);		//Se separa el espacio total entre los procesos para realizar las verificaciones correspondientes

		//VALIDAR: Realiza los contagios, sanaciones o muertes de cada persona
		int inicio = (int)((nPersonas / cnt_proc) * mid*T);
		validar(arreglito, matriz, nPersonas, cnt_proc, inicio, duracion, recuperacion, infeccion, tInfectadas, tCuradas, tMuertas, tSanas);
		
		/*
		
		Llamar a la funci�n cuentaInfectados 
		//al descomentar esto deber�a de correr
		enfermosTic=cuentaInfectados(int *arreglito, int& nPersonas, int& cnt_proc);

		*/

		//SIMULAR: realiza los movimientos
		simulacion(arreglito,matriz, nPersonas, cnt_proc, nInicialInfectados, size, inicio);

		if (mid == 0)
		{
			cout << "\nIMPRESION MATRIZ" << endl;
			imprimir(matriz, nPersonas);
		}

		//Provoca que se encicle, ver nota al final (Ver. 4-12, no se encicla)
		MPI_Allreduce(&enfermosTic, &enfermosRestantes, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);		//Reduce al contador de enfermos restantes por el espacio
			//Se incrementa el acumulador total de personas infectadas
		cout << "Enfermos restantes: " << enfermosRestantes << " @ " << mid << endl;

		
		MPI_Allgather(arreglito, (nPersonas / cnt_proc) *T, MPI_INT, matriz, (nPersonas / cnt_proc) *T, MPI_INT, MPI_COMM_WORLD);

		//Actualizaci�n de la cantidad de personas que se han infectado, curado o muerto y la cantidad de sanos que hay en el tic actual
		infectadasT += tInfectadas;
		curadasT += tCuradas;
		muertasT += tMuertas;
		sanasT += tSanas;
		++tics;		//Se incrementa el contador que indica la cantidad de tics
		/*
				OTRA POSIBLE SOLUCION:
			crear un arreglo temporal que tenga toda la matriz para poder ir comparando
			con un scatter, distribuir la matriz entre los procesos y recorrerla como en validar
			finalizar con un allgather a matriz
		*/

		
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
		//enviar al archivo de salida y al buffer los resultados de la TIC actual
	} while (enfermosRestantes !=0);
	
	//Camabiar los parametros de entrada y salida, hacer para muertas, inmunes, infectadas y sanas
	
	MPI_Barrier(MPI_COMM_WORLD);
	//Reducci�n de la cantidad de personas que se han infectado, curado o muerto a lo largo de la simulaci�n, as� como la cantidad de sanos restantes
	int sanos=0, muertos = 0, infectados = 0, curados = 0;
	MPI_Reduce(&infectadasT, &infectados, MPI_INT, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);	//Reduce el total de infectados
	MPI_Reduce(&sanasT, &sanos, MPI_INT, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);				//Reduce el total de sanos
	MPI_Reduce(&muertasT, &muertos, MPI_INT, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);			//Reduce el total de muertos
	MPI_Reduce(&tCuradas, &curados, MPI_INT, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);			//Reduce el total de curados
#ifdef DEBUG	//Impresion en otro proceso par acomprobar el Bcast	
	if (mid == 1)
	{
		cout << "MATRIZ DESDE PROCESO FINAL" << endl;
		imprimir(matriz, nPersonas);
	}
#endif
	free(matriz);		//Liberaci�n de la memoria ocupada
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
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>

#define BSIZE 2048

void
print_separator ()
{  
  printf ("---------------------------------------------------------------\n");
}



void
header ()					// header busca en el directorio el dato a imprimir, en ese caso es el nombre de la pc, que lo guarda en el buffer para luego imprimirlo
{
	//separador y definiciones
	print_separator ();
	FILE *fi;
	char buffer [BSIZE];
	
	//obtiene e imprime la información que se pide
	fi = fopen ("/proc/sys/kernel/hostname","r");		//lee la dirección y la guarda en 
	fgets (buffer, BSIZE, fi);							//guarda el dato en buffer (fgets lee la linea del lugar indicado y lo guarda en buffer)
	printf ("Pc name: %s\n", buffer);					//imprime lo que hay en el buffer como el nombre de la PC
	
	//al terminar cierra el archivo
	fclose (fi);

	//el tiempo actual
	time_t thetime;			
	time (&thetime);
	printf ("Date: %s", ctime(&thetime));				//imprime la fecha actual
	
	print_separator ();
}



void
cpu_info ()								//aplicando el mismo concepto, imprimimos la información del cpu
{
	//separador y definiciones
	print_separator ();
	FILE* fi;
	char buffer [BSIZE];
	
	//obtiene e imprime la información que se pide
	fi = fopen ("/proc/cpuinfo", "r");
	printf ("Processor info: \n");
	while (fgets(buffer, BSIZE, fi) != NULL)			//imprime mienstras alla información
	{
		if (strstr (buffer, "vendor_id"))				//si es el tipo de equipo, lo imprime
		{
			printf ("%s", buffer);
		}

		if (strstr (buffer, "model name"))				//si es el modelo del equipo, lo imprime
		{
			printf ("%s", buffer);
		}
	}													//cuando imprime ambos datos, sale
	
	//al terminar cierra el archivo
	fclose (fi);
	print_separator ();
}



void
kernel_version ()								//buscamos la informacion del kernel y la imprimimos
{
	//separador y definiciones
	print_separator ();
	FILE* fi;
	char buffer [BSIZE];
	
	//obtiene e imprime la información que se pide
	fi = fopen ("/proc/version", "r");
	fgets (buffer, BSIZE, fi);
	printf ("Kernel version: %s", buffer);		//imprime lo que hay en el buffer como el nombre de la PC
	
	//al terminar cierra el archivo
	fclose (fi);
	print_separator ();
}


void
print_time ()
{
	//separador y definiciones
	print_separator ();
	FILE* fi;
	double uptime;
	
	//obtiene la información y guarda en una variable uptime lo que encuentra
	fi = fopen ("/proc/uptime", "r");
	fscanf (fi, "%lf\n", &uptime);			//esta variable tiene el tiempo que lleva andando el sistema operativo, pero en una sola unidad, que será segundos, por lo que necesitamos pasarlo a dias, horas, minutos y segundos
	
	//al terminar cierra el archivo
	fclose (fi);
	
	//calculo del tiempo
	long time = (long) uptime; 
	const long minute = 60;
	const long hour = minute * 60;
	const long day = hour * 24;
	printf ("Time since system start-up: %ld days, %ld:%02ld:%02ld\n", 
					time / day, (time % day) / hour, (time % hour) / minute, time % minute);       //al dividir el valor de tiempo obtenido, lo obtenemos días, horas, minutos y segundos
	print_separator ();
}


void
file_systems ()							//en esta funcion imprime una lista de directorios del kernel
{
	//separador y definiciones
	print_separator ();
	FILE* fi;
	char buffer [BSIZE];
	
	//obtiene e imprime la información que se pide
	fi = fopen ("/proc/filesystems", "r");
	printf ("Kernel supported filesystems:\n");
	while (fgets(buffer, BSIZE, fi) != NULL)			//mientras haya informacion, la imprime
	{
		printf ("%s", buffer);
	}
	
	//al terminar cierra el archivo
	fclose (fi);
	print_separator ();
}

void
context_btime_proc ()								//este método recupera la cantidad de cambios de contextos, la fecha de inicio del sistema y la cantidad de procesos creados desde el inicio del sistema
{
	//separador y definiciones
	print_separator ();
	FILE* fi;
	fi = fopen ("/proc/stat", "r");
	char buffer[BSIZE];
	char* match;
	int epoch;
	int context;
	int processes;

	
	while (fgets(buffer, BSIZE, fi) != NULL)				//mientras haya datos, imprime  
	{
		if (strstr (buffer,"ctxt"))
		{
			match = (strstr (buffer, "ctxt"));
			sscanf (match, "ctxt %i", &context);
			printf ("Context changes: %i\n", context);		//busca la cantidad de cambios de contexto y los impirme
		}
		if (strstr (buffer,"btime"))
		{
			match = (strstr (buffer, "btime"));
			sscanf (match, "btime %d", &epoch);
			time_t thetime;
			thetime = epoch;
			printf ("System start-up at: %s", ctime(&thetime));			//calcula el tiempo desde que inició el sistema y lo imprime
		}
		if (strstr (buffer, "processes"))
		{
			match = (strstr(buffer, "processes"));
			sscanf (match, "processes %i", &processes);
			printf ("Created processes since system start-up: %i\n", processes);		//calcula los procesos creados desde la creación del sistema y los imprime
		}
	}
	
	//al terminar cierra el archivo
	fclose (fi);
	print_separator ();	

}


void
time_usage ()							//imprime la informacion de los datos de usuario, como tambien kernel e idle
{
	//separador y definiciones
	print_separator ();
	printf ("CPU use: \n");
	FILE* fi;
	int user;
	int kernel;
	int idle;
	int dummy;
	
	//obtiene la informacion y la guarda en diversas variables
	fi = fopen ("/proc/stat", "r");
	fscanf (fi,"%*s %d %d %d %d", &user, &dummy, &kernel, &idle);
	
	//cuando termina, cierra el archivo e imprime los distintos datos
	fclose (fi);
	printf ("User: %d \n", user);
	printf ("Kernel: %d \n", kernel);
	printf ("Idle: %d \n", idle);
	print_separator ();
}


void
disk_stats ()								//busca e imprime los datos de disco
{
	//separador y definiciones
	print_separator ();
	FILE* fi;
	char buffer [BSIZE];
	size_t bytes_read;
	char* match;
	int reads;
	int writes;
	
	
	fi = fopen ("/proc/diskstats", "r");
	bytes_read = fread (buffer, 1, sizeof (buffer), fi);			//leo los datos del discos y los guarda en el buffer
	fclose (fi);
	
	//toma los datos, los asigna a variables reads y writes
	buffer [bytes_read] = '\0';
	match = strstr (buffer, "sda");												//como la cantidad de cpu es variable, tiene que buscar el primer valor y devolverlo, no un número entero
	sscanf (match, "sda %*i %*i %*i %i %*i %*i %*i %i", &reads, &writes);		//cuantas escrituras y lecturas tuvo el disco
	printf ("Disk requests: %i\n", (reads + writes));							//imprime las interacciones totales del disco
	print_separator ();
}


void
mem_info ()							//imprime tamaño de memoria, tanto libre como total
{
	//separador y definiciones
	print_separator ();
	FILE* fi;
	char buffer [BSIZE];
	size_t bytes_read;
	char* match;
	int mem_total;
	int mem_available;
	
	
	fi = fopen ("/proc/meminfo", "r");
	bytes_read = fread (buffer, 1, sizeof (buffer), fi);			//leo los datos de memoria y los guardo en el buffer

	//cuando termino, cierro el archivo
	fclose (fi);
	
	//guardo el tamaño de memoria y la memoria disponible
	buffer[bytes_read] = '\0';
	match = strstr (buffer, "MemTotal");
	sscanf (match, "MemTotal: %i", &mem_total);
	match = strstr (buffer, "MemAvailable");
	sscanf (match, "MemAvailable: %i", &mem_available);
	printf ("Available/total memory: %i / %i kB\n", mem_available, mem_total);  //imprimo la proporción entre memoria total y disponible
	print_separator ();
}


void
load_avg ()									//imprime la carga promedio 
{
	//separador y definiciones
	print_separator ();
	FILE* fi;
	char buffer [BSIZE];
	
	//busca el dato, lo guarda y luego lo imprime
	fi = fopen ("/proc/loadavg", "r");
	fgets (buffer, 5, fi);
	printf ("Load average: %s\n", buffer);
	
	//al termina, cierra el archivo
	fclose (fi);
	print_separator ();
}


void
b ()					//esta funcion imprime los valores iniciales
{
	header ();						//nombre de la pc y la fecha
	cpu_info ();					//tipo y modelo del equipo
	kernel_version ();				//version de kernel
	print_time ();					//tiempo que lleva andando el sistema operativo
	file_systems ();				//los archivos de sistema que emplea
}


void
c ()					//esta funcion imprime los valores iniciales (funcion b) y el tiempo de uso 
{
	b ();						//todo lo de arriba
	time_usage ();				//tiempo de uso
	context_btime_proc ();
}


void
d (char **argv, char *argument, int ptr)				//esta función imprime los valores iniciales, el tiempo de uso y durante un tiempo, los valores de disco, memoria y carga, dando el teimpo en segudos que el sistema está dormido
{
	int interval;
	int duration;
	interval = atoi (argument);
	duration = atoi (*(argv + ptr));

	c ();											//todo lo de arriba

	while (duration > 0)							//mientras funcione
	{
		disk_stats ();									//datos del disco
		mem_info ();									//memoria total y libre
		load_avg ();									//carga promedio
		printf ("\n%d seconds interval\n", interval);	//intervalo de tiempo en segundos que está dormido
		sleep (interval);
		duration -= interval;							//va disminuyendo la duración
	}

}

void 
option_list ()					
{
	printf ("Valid options:\n-s\n-l <interval> <duration(secs)>\n\n");
}

int
main (int argc, char **argv)				//acá empieza el programa
{
	int option;	 
	if (argc == 1)							//si tiene una única entrada, imprime datos y se va
	{
		b ();							
		printf ("\n");
		exit (EXIT_SUCCESS);
	}

	while ((option = getopt (argc, argv, "sl:")) != -1)			//mientras esté andando, va valuando
	{	   

		switch (option) 	
		{
			case 's':   								//si se ejecuta -s llama al método c()
				if (argc == 2)							//cuando reciba el -s, valúa que entren 2 argumentos
				{
					c ();
				}
				else
				{
					option_list ();
				}
				break;

			case 'l':									//si se ejecuta -l llama al método d()
				if (argc == 4)							//cuando entra -l, espera que entren 4 argumentos
				{
					d (argv,optarg,optind);
				}
				else
				{
					option_list ();
				}
				break;

			default:
				option_list ();
				return 0;
	  } 
	}

	printf ("\n");
	return 0;
}

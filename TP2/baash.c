#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <pwd.h>

int 
parse_command(char* argv [], char* str);

void 
get_env_path (char *path_directories []);

void 
get_file_path (char* file, char* path_directories [], char* file_path);

int 
background (char* argv []);

int 
redirection (char* argv [], char redirection_file []);

void 
redirect_out (char redirection_file []);

int 
check_pipe (char* argv [], char* argv_left [], char* argv_right []);

void 
make_pipe (char* argv_left [], char* argv_right [], char* path_directories []);

/** Rutina principal de baash. Imprime un prompt y espera a que se ingrese un
		comando para ser interpretado. Si el comando es válido, lo ejecuta con los
		parámetros correspondientes, creando un proceso hijo para ello. 
*/
int 
main () 
{	
	size_t bufsize = 256;
	char* buf = malloc (bufsize);
	struct passwd *result;
	int argc;
	int size = 20;
	char username [20];
	char* login_pc;
	char* argv [20];
	char* argv_left [20];
	char* argv_right [20];
	struct passwd pass;
	char command [256];
	char* path_directories [20];
	char file_path [256];
	int background_flag = 0;
	int redirection_flag = 0;
	char redirection_file [60];
	int pipe_flag = 0;
	
	get_env_path (path_directories);
	int pwuid = getpwuid_r (getuid (), &pass, buf, bufsize, &result);
	if (result == NULL) 
		{
	    if (pwuid == 0)
		    {
		    	printf("pwuid no encontrada\n");
		    }
	    else 
		    {
		      perror("getpwnam_r");
		    }
		}
	gethostname (username, size);
	login_pc = pass.pw_name;

	while (1)
		{
			printf ("%s@%s %s $ ", login_pc, username, getcwd (NULL,100));
			fgets (command, 256, stdin);
			
			if (feof (stdin))
				{
					printf ("\nTermina por Ctrl-D\n");
					return 0;
				}

			if (!strcmp (command, "\n"))
				{
					continue;
				}
			
			else
				{
					argc = parse_command (argv, command);
					background_flag = background (argv);

					if (background_flag)
						{
							argv [argc - 1] = NULL;
							argc--;
						}

					if (!strcmp (argv [0], "exit"))
						{
							printf ("Termina por exit\n");
							exit(0);
						}

					if (!strcmp (argv [0], "cd"))
						{
							chdir (argv [1]);
							continue;
						}
					
					get_file_path (argv [0], path_directories, file_path);
					redirection_flag = redirection (argv, redirection_file);
					pipe_flag = check_pipe (argv, argv_left, argv_right);
					
					int pid_flag;
					int pid = fork ();
					pid_flag = -1;

					if (pid < 0) 
						{
							perror ("Creando al hijo");
							exit (1);
						}
					else if (pid == 0)
						{
							if (redirection_flag == 1)
								{
									freopen (redirection_file, "r", stdin);
								}
							else if (redirection_flag == 2)
								{
									redirect_out (redirection_file);
								}
							else if (pipe_flag)
								{
									make_pipe (argv_left, argv_right, path_directories);
								}
							if (!pipe_flag)
								{
									execv (file_path, argv);
									perror (file_path);
									exit (1);
								}
						}
					else if (pid > 0)
						{
							pid_flag = -1;
						}
					if (background_flag)
						{
							waitpid (pid, &pid_flag, WNOHANG);
						}
					else
						{
							waitpid (pid, &pid_flag, 0);
						}
				}
		}
}

/**	Parsea el string ingresado por stdin, separando el comando propiamente
		dicho (almacenado en argv [0]) de los argumentos (almacenados de argv [1]
		en adelante).
*/
int 
parse_command (char* argv [], char* str)
{
	int args = 0;
	char* save_ptr;	

	argv [0] = strtok_r (str, " \n", &save_ptr);

	for (args = 1; args < 20; args++)
		{
			argv [args] = strtok_r (NULL, " \n", &save_ptr);
			if (argv [args] == NULL)
				{
					break;
				}
		}
	
	return args;
}

/** Parsea la variable de entorno PATH y almacena los distintos directorios en
		path_directories []
*/
void
get_env_path (char *path_directories [])
{
	char* save_ptr;
	char* env_path = getenv ("PATH");
	path_directories [0] = strtok_r (env_path, ":", &save_ptr);

	for (int i = 1; i < 20; i++)
		{
			path_directories [i] = strtok_r (NULL, ":", &save_ptr);		

			if (path_directories [i] == NULL)
				{
					break;
				}
		}
	
	strtok_r (NULL, ":", &save_ptr);
}

/** Verifica si el comando en la variable file es un camino absoluto,
		relativo, o implica una búsqueda en la variable de entorno PATH. En los
		dos primeros casos, copia directamente file en file_path. En el tercero,
		realiza la búsqueda y, si encuentra un camino al archivo, lo copia en
		file_path 
*/
void 
get_file_path (char* file, char* path_directories[], char* file_path)
{
	char returnPath [50];
	strcpy (returnPath, file);
	
	if(file [0] == '/' || file [0] == '.' )
		{
			strcpy (file_path, file);
			return;
		}
	
	int i;
	char path_aux [50];

	for (i = 0; i < 20; i++)
		{
			if (path_directories [i] == NULL)
				{
					break;
				}
			strcpy (path_aux, path_directories [i]);
			strcat (path_aux, "/");
			strcat (path_aux, file);
			int file_access = access (path_aux, F_OK);
			if (!file_access)
				{
					strcpy (file_path, path_aux);
					return;
				}
		}
	
	return;
}

/** Verifica si el comando ingresado en argv [] quiere ser ejecutado en
		segundo plano, denotado por un caracter "&" al final del mismo. Devuelve
		1 si se quiere ejecutar en segundo plano, o 0 en caso contrario.
*/
int 
background (char* argv [])
{
	int i;

	for (i = 0; i < 20; i++)
		{
			if (argv [i] == NULL)
			break;
		}

	if (!strcmp (argv [i - 1], "&"))
		{
			return 1;
		}

	return 0;
}

/**	Verifica si se quiere hacer una redirección en el comando ingresado en
		argv [], denotado por un caracter "<" si se quiere redireccionar la
		entrada, o por ">" si se quiere redireccionar la salida. De realiarse una
		redirección, copia el nombre del archivo utilizado para la redirección en
		redirection_file []. Devuelve 0 si no hay redirección, 1 si se 
		redirecciona la entrada, o 2 si se redirecciona la salida.
*/
int
redirection (char* argv [], char redirection_file [])
{
	for (int i = 0; i < 20; i++)
		{
			if (argv [i] == NULL)
				{
					redirection_file = NULL;

					return 0;
				}
			else if (!strcmp (argv [i], "<"))
				{
					strcpy (redirection_file, argv [i + 1]);
					argv [i] = NULL;
					argv [i + 1] = NULL;

					return 1;
				}
			else if (!strcmp (argv [i], ">"))
				{
					strcpy (redirection_file, argv [i + 1]);
					argv [i] = NULL;
					argv [i + 1] = NULL;

					return 2;
				}
		}

	return 0;
} 

/**	Realiza la redirección de salida, hacia el archivo en redirection_file [].
*/
void 
redirect_out (char redirection_file [])
{
	int fid;
	int flags, perm;

	flags = O_WRONLY | O_CREAT | O_TRUNC;
	perm = S_IWUSR | S_IRUSR;
	fid = open (redirection_file, flags, perm);

	if (fid < 0) 
		{
			perror("open");
			exit(1);
		}
	close(STDOUT_FILENO);
	
	if (dup (fid) < 0) 
		{
			perror("dup");
			exit(1);
		}
	close(fid);
}

/** Verifica si se quiere realizar un pipe en el comando ingresado en argv []
		(denotado por un caracter "|") y, de ser así, separa argv [] en dos
		comandos (argv_left [] y argv_right []), entre los cuales se realizará el
		pipe. Devuelve 1 si se quiere realizar un pipe, o 0 en caso contrario.
*/
int
check_pipe (char* argv [], char* argv_left [], char* argv_right [])
{
	int arg_num_left, arg_num_right;
	
	for (arg_num_left = 0; argv [arg_num_left] != NULL; arg_num_left++)
		{
			int comp = strcmp ("|", argv [arg_num_left]);
			if(comp == 0)
				{
					break;
				}

			argv_left [arg_num_left] = (char*) malloc (strlen (argv [arg_num_left] + 1));
			strcpy(argv_left [arg_num_left], argv [arg_num_left]);
		}
	argv_left [arg_num_left] = '\0';
	
	if (argv [arg_num_left] == NULL)
		{
			return 0;
		}
	arg_num_left++;
	
	for (arg_num_right = 0; argv [arg_num_left] != NULL; arg_num_right++)
		{
			if (argv [arg_num_left] == NULL)
				{
					break;
				}

			argv_right [arg_num_right] = (char*) malloc (strlen (argv [arg_num_left] + 1));
			strcpy (argv_right [arg_num_right], argv [arg_num_left]);
			arg_num_left++;
		}
	argv_right [arg_num_right] = '\0';
	
	return 1;
}

/** Ejecuta los comandos en argv_left [] y argv_right [] y establece un pipe
		entre ellos
*/
void
make_pipe (char* argv_left [], char* argv_right [], char* path_directories [])
{
	char file_path [256];
	int fd [2];

	pipe (fd);
	if (fork() == 0) 
		{
			close (fd [0]);
			dup2 (fd [1], 1); 
			close (fd [1]);
			get_file_path (argv_left [0], path_directories, file_path);
			execv (file_path, argv_left);
			perror (file_path);
			exit (1);
		} 
	else 
		{
			close (fd [1]);
			dup2 (fd [0], 0);
			close (fd [0]);
			get_file_path (argv_right [0], path_directories, file_path);
			execv (file_path, argv_right);
			perror (file_path);
			exit (1);
		}
}
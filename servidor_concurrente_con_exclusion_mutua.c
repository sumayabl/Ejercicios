#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <pthread.h>

int contador;
//Estructura necesaria para acceso excluyente
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

//Estrucutras para lista sockets
typedef struct {
	int socket;
}Sockets;

typedef struct {
	int num;
	Sockets sockets[100];	
}ListaSockets;

ListaSockets lista;

//Funcion anadir socket a la lista (INICIAR SESION)
int PonSocket (ListaSockets *lista, int socket)
{
	if (lista->num == 100) //no se puede añadir mas usuarios, devuelve -1
		return -1;
	else{
		lista->sockets[lista->num].socket = socket;//Añadimos en la posicion num y devuelve 0
		lista->num++;
		return 0;
	}
	
}

//Funcion obtener posicion socket en la lista
int DamePosicion (ListaSockets *lista, int socket)
{
	int i=0;
	int encontrado = 0;
	while ((i < lista->num) && !encontrado)
	{
		if (lista->sockets[i].socket = socket)
			encontrado = 1;
		if (!encontrado)
			i=i+1;
	}
	if (encontrado)
		return i; //Cuando encuentra la i(posicion), la devuelve
	else
		return -1;
}

//Funcion eliminar socket
int EliminarSocket (ListaSockets *lista, int socket)
{
	int pos = DamePosicion (lista, socket); //Obtenemos pos para eliminar usuario de la lista
	if (pos == -1)
		return -1;
	else
	{
		int i;
		for (i=pos; i < lista->num-1; i++)
		{
			lista->sockets[i].socket = lista->sockets[i+1].socket;
		}
		lista->num--;
		return 0;
	}
}

void *AtenderCliente (void *socket)
{
	int sock_conn;
	int *s;
	s= (int *) socket;
	sock_conn= *s;
	
	//int socket_conn = * (int *) socket;
	
	char peticion[512];
	char respuesta[512];
	int ret;
	
	
	int terminar =0;
	// Entramos en un bucle para atender todas las peticiones de este cliente
	//hasta que se desconecte
	while (terminar ==0)
	{
		// Ahora recibimos la petici?n
		ret=read(sock_conn,peticion, sizeof(peticion));
		printf ("Recibido\n");
		
		// Tenemos que a?adirle la marca de fin de string 
		// para que no escriba lo que hay despues en el buffer
		peticion[ret]='\0';
		
		
		printf ("Peticion: %s\n",peticion);
		
		// vamos a ver que quieren
		char *p = strtok( peticion, "/");
		int codigo =  atoi (p);
		// Ya tenemos el c?digo de la petici?n
		char nombre[20];
		
		if ((codigo !=0)&&(codigo!=4))
		{
			p = strtok( NULL, "/");
			
			strcpy (nombre, p);
			// Ya tenemos el nombre
			printf ("Codigo: %d, Nombre: %s\n", codigo, nombre);
		}
		
		if (codigo ==0){ //petici?n de desconexi?n
			terminar=1;
		    //funcion quitar socket
		    EliminarSocket (&lista, sock_conn);
		}
		else if (codigo ==4)
			sprintf (respuesta,"%d",contador);
		else if (codigo ==1) //piden la longitd del nombre
			sprintf (respuesta,"%d",strlen (nombre));
		else if (codigo ==2)
			// quieren saber si el nombre es bonito
			if((nombre[0]=='M') || (nombre[0]=='S'))
			strcpy (respuesta,"SI");
			else
				strcpy (respuesta,"NO");
			else //quiere saber si es alto
			{
				p = strtok( NULL, "/");
				float altura =  atof (p);
				if (altura > 1.70)
					sprintf (respuesta, "%s: eres alto",nombre);
				else
					sprintf (respuesta, "%s: eresbajo",nombre);
			}
			
			if (codigo !=0)
			{
				
				printf ("Respuesta: %s\n", respuesta);
				// Enviamos respuesta
				write (sock_conn,respuesta, strlen(respuesta));
			}
			if ((codigo ==1)||(codigo==2)|| (codigo==3))
			{
				pthread_mutex_lock( &mutex ); //No me interrumpas ahora
				contador = contador +1;
				pthread_mutex_unlock( &mutex); //ya puedes interrumpirme
			}
			
	}
	// Se acabo el servicio para este cliente
	close(sock_conn); 
	
}


int main(int argc, char *argv[])
{
	
	int sock_conn, sock_listen;
	struct sockaddr_in serv_adr;
	
	// INICIALITZACIONS
	// Obrim el socket
	if ((sock_listen = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		printf("Error creant socket");
	// Fem el bind al port
	
	
	memset(&serv_adr, 0, sizeof(serv_adr));// inicialitza a zero serv_addr
	serv_adr.sin_family = AF_INET;
	
	// asocia el socket a cualquiera de las IP de la m?quina. 
	//htonl formatea el numero que recibe al formato necesario
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	// establecemos el puerto de escucha
	serv_adr.sin_port = htons(9300);
	if (bind(sock_listen, (struct sockaddr *) &serv_adr, sizeof(serv_adr)) < 0)
		printf ("Error al bind");
	
	if (listen(sock_listen, 3) < 0)
		printf("Error en el Listen");
	
	contador =0;
	int i;

	pthread_t thread;
	i=0;
	// Bucle para atender a 5 clientes
	for (;;){
		printf ("Escuchando\n");
		
		sock_conn = accept(sock_listen, NULL, NULL);
		printf ("He recibido conexion\n");
		
		PonSocket (&lista, sock_conn);
		//sock_conn es el socket que usaremos para este cliente
		
		// Crear thead y decirle lo que tiene que hacer
		
		pthread_create (&thread, NULL, AtenderCliente,&lista.sockets[lista.num-1]);
		i=i+1;
		
	}
	
	//for (i=0; i<5; i++)
	//pthread_join (thread[i], NULL);
	
	
	
}

#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static const int n = 5;

sem_t taken_chair, barber_sleeping, cutting_hair, available_chairs;
static bool everyone_done = false;

// Declaración de las funciones usadas en el programa
void *barber_handler(void *arg);
void *client_handler(void *arg);
void print_barber(const char *);
void print_client(const int, const char *);

int main()
{
	sem_init(&available_chairs, 0, n); // Inicializar el semáforo de las sillas disponibles
	sem_init(&taken_chair, 0, 1);	   // Inicializar el semáforo de la silla a ser ocupada por un cliente cuando le corten el pelo
	sem_init(&cutting_hair, 0, 0);	   // Inicializar el semáforo cuando el barbero esta cortando el pelo
	sem_init(&barber_sleeping, 0, 0);  // Inicializar el semáforo de cuando el barbero esta dormido
	pthread_t barber, clients[n];	   // Hilos respectivos del productor y del consumidor

	pthread_create(&barber, NULL, barber_handler, NULL); // Inicializamos el hilo del barbero y su función

	for (int i = 0; i < n; ++i)
		pthread_create(&clients[i], NULL, client_handler, (void *)&i); // Creamos cada uno de los hilos y pasamos un identificador para cada cliente
	for (int i = 0; i < n; ++i)
		pthread_join(clients[i], NULL); // Esperamos a que terminé el hilo de cada cliente

	everyone_done = true;		// Una vez que termina con todos se convierte en true
	sem_post(&barber_sleeping); // Le decimos al barbero que se ponga a dormir de nuevo
	pthread_join(barber, NULL); // Esperamos a que termine el hilo del barbero
	return 0;
}

// Función ejecutada por el barbero
void *barber_handler(void *arg)
{
	while (!everyone_done) // Mientras que existan clientes esperando, el barbero va a estar disponible
	{
		print_barber("Durmiendo");
		sem_wait(&barber_sleeping); // Esperamos a que el barbero este durmiendo
		if (!everyone_done)			// Mientras que exista algun cliente esperando
		{
			print_barber("Cortando el cabello");
			sem_post(&cutting_hair); // Notificamos que el barbero dejo de cortar el cabello a un cliente
			print_barber("Termino de cortar el cabello");
		}
	}
	print_barber("Salió de la barbería");
}

void *client_handler(void *arg)
{
	char output[80];
	int num = *(int *)arg; // Obtenemos el identificador del cliente
	print_client(num, "llego a la barberia");

	sem_wait(&available_chairs); // El cliente intenta toma una silla de espera
	print_client(num, "se sienta en la sala de espera");
	sem_wait(&taken_chair);		 // El cliente intenta tomar la silla del barbero para ser atendido
	sem_post(&available_chairs); // El cliente libera una silla de espera
	sleep(1);					 // Mientras que espera por su turno

	print_client(num, "leventa al barbero");
	sem_post(&barber_sleeping); // Notifica que el barbero no esta dormido
	sem_wait(&cutting_hair);	// Espera a que le corten el pelo
	sem_post(&taken_chair);		// Libera la silla donde le cortaron el cabello
	sem_post(&barber_sleeping); // El barbero es liberado y se pone a dormir

	print_client(num, "se fue");
}

void print_barber(const char *message)
{
	printf("[Barbero]: %s\n", message);
}

void print_client(const int id, const char *message)
{
	printf("[Cliente %d]: %s\n", id, message);
}

// Grupo Dijkstra:
// ** Edmilson Domingues
// ** Gabriel Nascimento
// ** Martina Brehm

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <unistd.h>

#define ESCRITORES    100 //quantidade de escritores
#define LEITORES    100 //quantidade de leitores
#define N 16  // número de blocos K
#define K 4 // tamanho do bloco

typedef struct tarefa_escrita{
	long Posicao;
	char*  dados; 
	long tamanho;
}tarefa_escrita;

typedef struct tarefa_leitura{
	long Posicao;
	long tamanho;
}tarefa_leitura;

	char memoria_total [N*K];
	pthread_mutex_t LE[N];
	pthread_mutex_t foto;
	pthread_mutex_t rc[N]; // leituras simultâneas.
	long rcounter[N]; // leituras simultâneas.
	long escritas_realizadas = 0;  // sinaliza final das escritas, para pode gravar arquivo.

void *escritor(void *arg)
{
	tarefa_escrita *tarefa;
	tarefa = (tarefa_escrita *) arg;
	
	long endereco_inicial;
	long endereco_final;
	long Ninicial;
	long Nfinal;
    
	endereco_inicial = tarefa->Posicao;
	endereco_final = endereco_inicial + tarefa->tamanho -1;
	Ninicial = endereco_inicial/K;
	Nfinal = endereco_final/K;
	long delta = Nfinal - Ninicial +1;
	long N_da_vez;
	long i;
	
	N_da_vez = Ninicial;
	for (i=0; i < delta; i++)
		{
			pthread_mutex_lock(&LE[N_da_vez]);
			N_da_vez++;				
		}
		
	pthread_mutex_lock(&foto);
	for (i=0; i<tarefa->tamanho; i++)
		{
			memoria_total[endereco_inicial + i]=tarefa->dados [i];
		}
	pthread_mutex_unlock(&foto);
		
	N_da_vez = Ninicial;
	for (i=0; i < delta; i++)
		{
			pthread_mutex_unlock(&LE[N_da_vez]);
			N_da_vez++;
		}
}

void *leitor(void *arg)
{
	tarefa_leitura *tarefa;
	tarefa = (tarefa_leitura *) arg;
	
	long endereco_inicial;
	long endereco_final;
	long Ninicial;
	long Nfinal;
    
	endereco_inicial = tarefa->Posicao;
	endereco_final = endereco_inicial + tarefa->tamanho -1;
	Ninicial = endereco_inicial/K;
	Nfinal = endereco_final/K;
	long delta = Nfinal - Ninicial +1;
	long N_da_vez;
	long i;
	
	N_da_vez = Ninicial;
	for (i=0; i < delta; i++)
		{
			pthread_mutex_lock(&rc[N_da_vez]); // leituras simultâneas.
			rcounter[N_da_vez]++; // leituras simultâneas.
			if (rcounter[N_da_vez] == 1) // leituras simultâneas.
			{
			pthread_mutex_lock(&LE[N_da_vez]);
			}
			pthread_mutex_unlock(&rc[N_da_vez]); // leituras simultâneas.
			N_da_vez++;
		}

	pthread_mutex_lock(&foto);
	for (i=0; i<tarefa->tamanho; i++)
		{
			printf("%c", memoria_total[endereco_inicial + i]);
		}
	printf("\n");
	pthread_mutex_unlock(&foto);
	
	N_da_vez = Ninicial;
	for (i=0; i < delta; i++)
		{
			pthread_mutex_lock(&rc[N_da_vez]); // leituras simultâneas.
			rcounter[N_da_vez]--; // leituras simultâneas.
			if (rcounter[N_da_vez] == 0) // leituras simultâneas.
			{
				pthread_mutex_unlock(&LE[N_da_vez]);		
			}
			pthread_mutex_unlock(&rc[N_da_vez]); // leituras simultâneas.
			N_da_vez++;
			
		}
}

void *logmemoria(void *arg)
{
	long endereco_inicial = 0;
	long i;
	FILE *pont_arq; // cria ponteiro para o arquivo
	char espelho_memoria_total[N*K];
  
	pont_arq = fopen("./sisop.txt", "w");
	if(pont_arq == NULL)
	{
	printf("Erro na abertura do arquivo!");
	}

	while(1)
	{		
				pthread_mutex_lock(&foto);
				for (i=0; i<N*K; i++)
					{
						espelho_memoria_total[endereco_inicial + i] = memoria_total[endereco_inicial + i];
					}
				pthread_mutex_unlock(&foto);
				
				fprintf(pont_arq, &espelho_memoria_total[endereco_inicial]);
				fprintf(pont_arq,"\n");
				if (escritas_realizadas == 999999)
				{
					break;
				}
	}
			fclose(pont_arq);
 			printf("\nDados gravados com sucesso!\n");
}

int main(int argc, char *argv[]){
	pthread_t escrever[ESCRITORES];  
	pthread_t ler[LEITORES];
	pthread_t log;
	tarefa_escrita *tarefa_corrente_escrita[ESCRITORES];
	tarefa_leitura *tarefa_corrente_leitura[LEITORES];

	tarefa_corrente_escrita[0] = malloc(sizeof(tarefa_escrita));
	tarefa_corrente_escrita[0] -> Posicao = 0;
	tarefa_corrente_escrita[0] -> dados = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
	tarefa_corrente_escrita[0] -> tamanho = 40;

	tarefa_corrente_escrita[1] = malloc(sizeof(tarefa_escrita));
	tarefa_corrente_escrita[1] -> Posicao = 10;
	tarefa_corrente_escrita[1] -> dados = "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB";
	tarefa_corrente_escrita[1] -> tamanho = 40;

	tarefa_corrente_escrita[2] = malloc(sizeof(tarefa_escrita));
	tarefa_corrente_escrita[2] -> Posicao = 20;
	tarefa_corrente_escrita[2] -> dados = "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC";
	tarefa_corrente_escrita[2] -> tamanho = 44;

	tarefa_corrente_leitura[0] = malloc(sizeof(tarefa_leitura));
	tarefa_corrente_leitura[0] -> Posicao = 0;
	tarefa_corrente_leitura[0] -> tamanho = 40;

	tarefa_corrente_leitura[1] = malloc(sizeof(tarefa_leitura));
	tarefa_corrente_leitura[1] -> Posicao = 10;
	tarefa_corrente_leitura[1] -> tamanho = 40;
	
	tarefa_corrente_leitura[2] = malloc(sizeof(tarefa_leitura));
	tarefa_corrente_leitura[2] -> Posicao = 20;
	tarefa_corrente_leitura[2] -> tamanho = 44;

	long i;
	for (i=0; i<N; i++)
	{
		rcounter[i] = 0;
	}	
	
	pthread_create(&log, NULL, logmemoria, NULL);
	
	long max = 0;
	if (ESCRITORES > LEITORES)
	{
		max = ESCRITORES;
	} else {
		max = LEITORES;
	}
	for (i=0; i < max; i++)
	{
		if (i < ESCRITORES)
		{
			//printf("Thread escrita %ld sendo criada\n", i);
			if (i%3 == 0)
			{
				pthread_create(&escrever[i], NULL, escritor, (void *) tarefa_corrente_escrita[0]);
			} else 
				if (i%3 ==1){
					pthread_create(&escrever[i], NULL, escritor, (void *) tarefa_corrente_escrita[1]);
				} else {
					pthread_create(&escrever[i], NULL, escritor, (void *) tarefa_corrente_escrita[2]);
				}
		}
		if (i < LEITORES)
		{
			//printf("Thread leitura %ld sendo criada\n", i);
			if (i%3 == 0)
			{
				pthread_create(&ler[i], NULL, leitor, (void *) tarefa_corrente_leitura[0]);
			} else 
				if (i%3 ==1){
					pthread_create(&ler[i], NULL, leitor, (void *) tarefa_corrente_leitura[1]);
				} else {
					pthread_create(&ler[i], NULL, leitor, (void *) tarefa_corrente_leitura[2]);
				}
		}
	}
				
	//printf("Aguardando término das threads\n");
	for(i=0; i<ESCRITORES; i++)
		pthread_join(escrever[i], NULL);

	//printf("Aguardando término das threads\n");
	for(i=0; i<LEITORES; i++)
		pthread_join(ler[i], NULL);	
		
	escritas_realizadas = 999999;
		
	printf("Todas as threads de escritores e leitores foram finalizadas\n");
		
	pthread_join(log, NULL);
			
	printf("Todas as threads foram finalizadas\n");
	exit (0);
}





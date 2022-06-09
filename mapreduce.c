#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <math.h>
#include <time.h>
#include <dirent.h>

#define TAM_PALAVRA 32
#define NUM_WORKERS 8
#define case_sensitive 0
// Semaforos
sem_t mutex_cpy;
sem_t mutex_reduce_dest;
sem_t mutex_shuffle;
sem_t mutex_strlen;
sem_t mutex_fgets;

// Item do mapa <chave, valor>
typedef struct {
    char *chave;
    int valor;
} item;
   
typedef struct map_args {
    int* partition;
} thread_args;
// Construtor do item do mapa
item faz_item(char *c, int v){
    item i;
    i.valor = v;
    i.chave = c;
    return i;
}
void shuffle(item i);
void shift_index(int* indices, int i_shift, int shift);
void print_resposta(int num_itens, char *chaves[], int *valores);
void *reduce(void *arg);
long int acha_tam_arq(char file_name[]);
void get_arquivos();
int get_tam_entrada();
int** split(int* indices);
char* fgetw(FILE* stream, int read_end);
void* map(int arqi, int i, int arqf, int f);
void load_partitions(int*** th_splits);
void* load_map(void* arg);
void init_map_args(thread_args* arg, int* partition);
void call_intermediate_files();
// ZONA CRITICA
int num_arq=0;
char**  arquivos_nomes;
FILE **arqs;
FILE *res_map_files[NUM_WORKERS];
FILE *out_red_files[NUM_WORKERS];
int tam_particao;
int *tam_cada_arq;
int tam_total;
item *itens_separados[NUM_WORKERS];
int indice_shuffle[NUM_WORKERS];
int palavras_do_reduce[NUM_WORKERS];
// ZONA CRITICA

void print_resposta(int num_itens, char *chaves[], int *valores){
    int i, j;
 
    for(i=0; i<num_itens; i++){
        if(valores[i] > 0){
            printf("%s: %d\n", chaves[i], valores[i]);
        }
    }
}
void write_ofile(int num_itens, char *chaves[], int *valores, int ind){
    int alg = log10(NUM_WORKERS)+1;
    char* arquivo_nome = (char*)malloc((alg+5)*sizeof(char));
    sprintf(arquivo_nome, "%s%d%s", "o", ind, ".txt");
    FILE* o_arq = (FILE*)calloc(1, sizeof(FILE));
    o_arq = fopen(arquivo_nome, "w+");
    for(int i=0; i<num_itens; i++){
        if(valores[i] > 0){
            fprintf(o_arq,"%s %d\n", chaves[i], valores[i]);
        }
    }
    fclose(o_arq);
    free(arquivo_nome);
    free(chaves);
}
void *reduce(void *arg){
    int indice = (*(int *)arg);
    item* itens_mapeados = (item*)malloc(palavras_do_reduce[indice]*sizeof(item));
    int i, j;
    char **chaves = (char**)malloc(palavras_do_reduce[indice] * sizeof(char*));
    int valores[palavras_do_reduce[indice]];
    char *linha = (char*)malloc((TAM_PALAVRA+3) * sizeof(char));
    char *chave_lida = (char*)malloc((TAM_PALAVRA) * sizeof(char));
    int valor_lido;
    int num_itens = 0;

    // LER OS ARQUIVOS
    fseek(res_map_files[indice], 0L, SEEK_SET);
    sem_wait(&mutex_fgets);
    while(fgets(linha, ((TAM_PALAVRA+3) * sizeof(char)), res_map_files[indice]) != NULL) {
        //printf("passou\n");
        sem_post(&mutex_fgets);
        sscanf(linha, "%s %d", chave_lida, &valor_lido);
        if(isalpha(chave_lida[0])){
            chaves[num_itens] = (char*)malloc((TAM_PALAVRA+1) * sizeof(char));
            sem_wait(&mutex_cpy);
            strcpy(chaves[num_itens], chave_lida);
            sem_post(&mutex_cpy);
            valores[num_itens] = valor_lido;
            num_itens++;
        }
        sem_wait(&mutex_fgets);
    }
    sem_post(&mutex_fgets);
    free(linha);
    free(chave_lida);
    // FIM DA LEITURA
    for(i=0; i<num_itens; i++){
        if(valores[i] > 0){
            for(j=i+1; j<num_itens; j++){
                if(strcmp(chaves[i], chaves[j]) == 0){
                    valores[i] += valores[j];
                    valores[j] = 0;
                }
            }
        }
    }      
    print_resposta(num_itens, chaves, valores);
    write_ofile(num_itens, chaves, valores, indice);
} 

long int acha_tam_arq(char file_name[]) {
    FILE* fp = fopen(file_name, "r");
  
    if (fp == NULL) {
        printf("File Not Found!\n");
        return -1;
    }

    fseek(fp, 0L, SEEK_END);
  
    long int res = ftell(fp);
  
    fclose(fp);
  
    return res;
}

// Arquivos de input, podem ser alterados e/ou adicionados
void get_arquivos(){
    arqs = (FILE**)calloc(1, sizeof(FILE*));
    arquivos_nomes = (char**)malloc(sizeof(char*));
    int i=0, j=0;
    char* arquivo_nome = (char*)malloc(6*sizeof(char));
    sprintf(arquivo_nome, "%s%d%s", "t", i, ".txt");

    while(arqs[i] = fopen(arquivo_nome, "r")){
        arqs = (FILE**)realloc(arqs, (i+2)*sizeof(FILE*));
        arquivos_nomes = (char**)realloc(arquivos_nomes, (i+1)*sizeof(char*));
        arquivos_nomes[i] = arquivo_nome;
        num_arq++;
        i++;
        if((i+1)%10==0){
            j++;
        }
        arquivo_nome = (char*)malloc((6+j)*sizeof(char));
        sprintf(arquivo_nome, "%s%d%s", "t", i, ".txt");
    }
}

int get_tam_entrada(){
    int tam=0, i;
    item itens;
    tam_cada_arq = (int*)calloc(num_arq, sizeof(int));
    for(i=0; i<num_arq; i++){
        fseek(arqs[i], 0L, SEEK_END);
        tam_cada_arq[i] = ftell(arqs[i]);
        fseek(arqs[i], 0L, SEEK_SET);
        tam += tam_cada_arq[i];
    }
    return tam;
}
int** split(int* indices){
    int arq=0, index=0;
    int current_file=0;
    int ind_geral;
    int acum=0;
    int shift=0;
    char char_atual, char_anterior=0;
    int** threads_arq_i;
    threads_arq_i = (int**)malloc(NUM_WORKERS*sizeof(int*));
    for(int i=0;i<NUM_WORKERS;i++){
        *(threads_arq_i+i) = (int*)malloc(2*sizeof(int));
    }
    for(int i = 0;i < NUM_WORKERS;i++){
        ind_geral = indices[i];
        for(int j = arq;j < num_arq;j++){
            if(!current_file){
                acum+=tam_cada_arq[j];
            }
            if(acum-1>= ind_geral){
                index = ind_geral-(acum-tam_cada_arq[j]);

                fseek(arqs[j], (long int) index, SEEK_SET);
                char_atual = fgetc(arqs[j]);

                if(isalpha(char_atual) &&
                    !fseek(arqs[j], (long int) index-1, SEEK_SET)
                    && isalpha(fgetc(arqs[j]))){
                    fseek(arqs[j], (long int) index-1, SEEK_SET);
                    char_anterior = fgetc(arqs[j]);
                    fseek(arqs[j], (long int) index+1, SEEK_SET);
                    while(isalpha(char_anterior)){
                        char_anterior = fgetc(arqs[j]);
                        index+=1;
                        shift+=1;
                        if(feof(arqs[j])){
                            //pegou no meio da ultima palavra do arq;
                            index=0;
                            j++;
                            break;
                        }
                    }
                    current_file=1;
                }
                else if(index<tam_cada_arq[j]){
                    current_file=1;
                }
                arq=j;
                break;
            }
            current_file=0;
        }
        if(shift){
            shift_index(indices, i+1, shift);
            shift=0;
        }
        if(arq>=num_arq){
            break;
        }
    }
    return threads_arq_i;
}

void shift_index(int* indices, int i_shift, int shift){
    for(int i=i_shift; i<NUM_WORKERS;i++){
        indices[i]+=i_shift;
    }
}

//le a proxima palavra a partir da posiçao do ponteiro do arquivo
char* fgetw(FILE* stream, int read_end){
    if(feof(stream)){
        return 0;
    }
    char* str;
    char* tok;
    char* ptr;
    char c;
    int last_char=0;
    int mem=sizeof(char);
    str = (char*)calloc(mem, sizeof(char));
    tok = (char*)calloc(1, sizeof(char));
    ptr = (char*)calloc(1, sizeof(char));

    while(c = fgetc(stream)){
        if(feof(stream) || (int)ftell(stream)>read_end){
            break;
        }
        if(!case_sensitive && c >= 'A' && c <='Z'){
            c += 32;
        }
        *ptr = c;
        tok = strtok(ptr, " #*()%@[]{};|,.-!_/1234567890\n\"\'\t\v\f\r\b\r");
        if(!tok && last_char!=0 && isalpha(last_char)){
            break;
        }
        else if(tok){
            mem+=sizeof(char);
            str = (char*)realloc(str, mem);
            last_char = *tok;
            strncat(str, (const char*)tok, sizeof(char));
        }
    }
    free(ptr);
    free(tok);
    return str;
}

void* map(int arqi, int i, int arqf, int f){
    char* word;
    item item;
    FILE* fptr;
    int read_end;
    fptr = (FILE*)calloc(1, sizeof(FILE));
    for(int arq_ind=arqi ; arq_ind<=arqf ; arq_ind++){
        read_end=tam_cada_arq[arq_ind];
        if(arq_ind==arqf){
            read_end=f;
        }
        //printf("%d\n", read_end);
        //usleep(100000);
        fptr = fopen(arquivos_nomes[arq_ind], "r");
        //printf("abriu %s\n", arquivos_nomes[arq_ind]);
        //usleep(100000);
        fseek(fptr, i, SEEK_SET);
        while((int)ftell(fptr)<read_end){
            ////printf("pos %d\n", (int)ftell(fptr));
            if(word = fgetw(fptr, read_end)){
                shuffle(faz_item(word, 1));
            }
        }
        fclose(fptr);
        i=0;
    }
}
void shuffle(item i){
    int dest = 0;
    sem_wait(&mutex_strlen);
    for(int j =0; j<strlen(i.chave);j++){
        sem_post(&mutex_strlen);
        dest+= i.chave[j];
        sem_wait(&mutex_strlen);
    }
    sem_post(&mutex_strlen);
    dest=dest%NUM_WORKERS;

    sem_wait(&mutex_reduce_dest);
    palavras_do_reduce[dest]++;
    sem_post(&mutex_reduce_dest);

    sem_wait(&mutex_shuffle);
    fprintf(res_map_files[dest],"%s %d\n", i.chave, i.valor);
    sem_post(&mutex_shuffle);
}
void load_partitions(int*** th_splits){
    for(int i=0;i<NUM_WORKERS;i++){
        *(*th_splits+i) = (int*)realloc(*(*th_splits+i), 4*sizeof(int));
        if(i==NUM_WORKERS-1){
            *(*(*th_splits+i)+2) = num_arq-1;
            *(*(*th_splits+i)+3) = tam_cada_arq[num_arq-1];
            break;
        }    
        *(*(*th_splits+i)+2) = *(*(*th_splits+i+1));
        *(*(*th_splits+i)+3) = *(*(*th_splits+i+1)+1);
    }
}
void* load_map(void* arg){
    thread_args* args;
    args = (thread_args*) arg;
    map(args->partition[0], args->partition[1], args->partition[2], args->partition[3]);
    free(arg);
    return NULL;
}
void init_map_args(thread_args* arg, int* partition){
    (arg)->partition = partition;
}

void call_intermediate_files(){
    int algs = log10(NUM_WORKERS)+1; 
    char *arquivo_nome = (char*)malloc((7+algs)*sizeof(char));
    
    for(int i=0; i < NUM_WORKERS; i++){
        sprintf(arquivo_nome, "%s%d%s", "res", i, ".txt");
        // Abre pra escrever e ler
        res_map_files[i] = fopen(arquivo_nome,"w+");
    }
    free(arquivo_nome);
}
int main(int argc, char **argv) {
    clock_t start, end;
    start = clock();
    int j;
    item i;
    double cpu_time_used;
    pthread_t tid[NUM_WORKERS];
    get_arquivos();
    tam_total = get_tam_entrada();
    // Separa a partição de cada Worker
    tam_particao = tam_total / NUM_WORKERS;
    int* indices = (int*)malloc(NUM_WORKERS*sizeof(int));
    for(int k=0;k<NUM_WORKERS;k++){
        indices[k]=tam_particao*k;
        palavras_do_reduce[k] = 0;
    }
    int** ind = split(indices);
    load_partitions(&ind);
    free(indices);
    //--------------------------
    sem_init(&mutex_shuffle, 0, 1);
    sem_init(&mutex_cpy, 0, 1);
    sem_init(&mutex_reduce_dest, 0, 1);
    sem_init(&mutex_strlen, 0, 1);
    sem_init(&mutex_fgets, 0, 1);
    call_intermediate_files();    
    for(j=0; j<NUM_WORKERS; j++){
        thread_args* arg;
        arg = (thread_args*)malloc(sizeof(thread_args));
        init_map_args(arg, ind[j]);
        pthread_create(&(tid[j]), NULL, load_map, (void*)arg);
    }
    for (j=0; j<NUM_WORKERS;j++){
        pthread_join(tid[j], NULL);
    }
    // Reduce:
    printf("Word Counts:\n");
    for(j=0; j<NUM_WORKERS; j++){
        int* i = malloc(sizeof(int));
        *i = j;
        pthread_create(&(tid[j]), NULL, reduce, i);
    }   
    // Barreira:
    int k;
    for(k=0; k<NUM_WORKERS; k++){
        pthread_join(tid[k], NULL);
    }
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("finished in %f seconds\n", cpu_time_used);
    return 0;
}
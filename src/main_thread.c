#include<stdio.h>
#define N 100
int main(int argc, char* argv[]){
    DMA_init();
    DMA_init();

    int tab[N];
    for(int i=0; i<N; i++){
        tab[i] = i;
    }

    uintptr_t job1_id = DMA_job(tab[0], tab[10], 10);
    show_jobs();
    uintptr_t job2_id = DMA_job(tab2, tab3, N);
    show_jobs();
    


    
    return 0;
}
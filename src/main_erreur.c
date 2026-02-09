#include<stdio.h>
#define N 100
int main(int argc, char* argv[]){
    DMA_init();

    int tab1[N], tab2[N], tab3[N];
    for(int i = 0; i < N; i++) {
        tab1[i] = i;
        tab2[i] = 0;
        tab3[i] = N-i;    
    }

    uintptr_t job1_id = DMA_job(tab1[5], tab2[5], 10);
    show_jobs();
    uintptr_t job2_id = DMA_job(tab2, tab3, N);
    show_jobs();
    


    
    return 0;
}
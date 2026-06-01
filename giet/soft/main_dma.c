#include "stdio.h"
#include "dma.h"
#include "dma_sim.h"
#include "config.h"

#define TAB_SIZE 10

int verification(int* t1, int* t2, size_t size) {
	for (size_t i =0; i<size; i++) {
		if (t1[i] != t2[i]) {
			return 0;
		}
	}
	return 1;
}

__attribute__ ((constructor)) int main(void) {
	tty_printf ("Hello I am submitting jobs\n");

	int t1[TAB_SIZE], t2[TAB_SIZE];
	int delay = 0, cpt = 0;
	int j1;

	barrier_init(0, NB_PROCS); // Barriere pour initialisation du tableau de jobs
	barrier_wait(0);	

	show_jobs();

	// Soumission de jobs
	tty_printf("Job is going to be submitted at cycle %d\n", proctime());
	j1 = (int)DMA_job(t1, t2, TAB_SIZE);
	tty_printf("Job submitted at cycle %d\n", proctime());

	if (j1 < 0) {
		exit(1);
	}
	show_jobs();

	// Transferts de mémoire témoins, permettent de réecrire le cache
	int t3[10], t4[10];

	tty_printf("Copying array of size 10 at cycle %d\n", proctime());
	for (int i = 0; i<10; i++) {
		t3[i] = t4[i];
	}
	tty_printf("Ended copying array of size 10 at cycle %d\n", proctime());

	int t5[100], t6[100];

	tty_printf("Copying array of size 100 at cycle %d\n", proctime());
	for (int i = 0; i<100; i++) {
		t5[i] = t6[i];
	}
	tty_printf("Ended copying array of size 100 at cycle %d\n", proctime());

	int t7[1000], t8[1000];

	tty_printf("Copying array of size 1000 at cycle %d\n", proctime());
	for (int i = 0; i<1000; i++) {
		t7[i] = t8[i];
	}
	tty_printf("Ended copying array of size 1000 at cycle %d\n", proctime());

	int t9[2000], t10[2000];

	tty_printf("Copying array of size 2000 at cycle %d\n", proctime());
	for (int i = 0; i<2000; i++) {
		t9[i] = t10[i];
	}
	tty_printf("Ended copying array of size 2000 at cycle %d\n", proctime());

	int t11[3000], t12[3000];

	tty_printf("Copying array of size 3000 at cycle %d\n", proctime());
	for (int i = 0; i<3000; i++) {
		t11[i] = t12[i];
	}
	tty_printf("Ended copying array of size 3000 at cycle %d\n", proctime());

	// Terminaison du job
	int done = 0;
	// Sortie si job fini ou timeout
	while(done <= 0 && cpt < 5) {
		delay++;
		delay = delay%100000;
		if(delay == 0){

			tty_printf("Job is going to be acknowledged at cycle %d\n", proctime());
			done = DMA_done(j1);
			tty_printf("Job acknowledged at cycle %d\n", proctime());

			cpt++;
			show_jobs();
		}
	}
	show_jobs();

	if(!verification(t1, t2, TAB_SIZE)) {
		tty_printf("Verification échouée\n");
	} else {
		tty_printf("Verification réussie\n");
	}

	tty_printf("CPT = %d\n", cpt);

	// exit(0);
	return 0;
}

__attribute__ ((constructor)) int main_dma(void) {
	tty_printf ("Hello I am the DMA\n");

	barrier_init(0, NB_PROCS); // Barriere pour initialisation du tableau de jobs
	DMA_init();
	barrier_wait(0);	

	DMA_sim(0);

	return 0;
}

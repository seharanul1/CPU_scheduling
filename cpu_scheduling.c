#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PRC_NUM 50
#define MAX_RECORD_NUM 100000
#define TIME_QUANTUM 5
#define AGING 100
#define TRUE 1
#define FALSE 0

typedef struct process {
	int pid;               //Process ID
	int cpu_burst_t;     // CPU burst time
	int arrival_t;       // Arrival time
	int priority;           // Priority 1-30

	int io_num;           //I/O request
	int io_burst_t;      // I/O burst time
	int io_strt_t[7];

	int terminated_t;
	int waiting_t;
	int turnaround_t;

} prc;

typedef struct cpu_scheduling_algorithm {

	int id;
	char* name;
	prc RQ[MAX_PRC_NUM];
	prc FQ[MAX_PRC_NUM];
	prc TQ[MAX_RECORD_NUM];
	int num_in_RQ;
	int num_in_FQ;
	int num_in_TQ;

	double CPU_used_t;

	double fin_t;
	double CPU_util;
	double ave_waiting_t;   //Average waiting time
	double ave_turn_t;      //Average turnaround time
	int cont_s_t;

	double preem_fin_t;
	double preem_CPU_util;
	double preem_ave_waiting_t;   //Average waiting time
	double preem_ave_turn_t;      //Average turnaround time
	int preem_cont_s_t;

} sche;


sche FCFS, SJF, PRIO, RR, PRIO_ag;
sche* Alg[5] = { &FCFS, &SJF, &PRIO, &RR, &PRIO_ag };
prc cpu_idle = { 51, 0, 0, 31, 0, 0, 0, 0, 0 };
prc io_idle = { 52, 0, 0, 31, 0, 0, 0, 0, 0 };
prc df_prc = { -1, 0, 0, 40, 0, 0, 0, 0, 0 };

prc NEW_Queue[MAX_PRC_NUM];
prc Wait_Queue[MAX_PRC_NUM];
int wait_queue_num = 0;
prc JOB_Queue[MAX_PRC_NUM];
int job_queue_num;
prc IO_TQ[MAX_RECORD_NUM];
int IO_TQ_num = 0;

int total_prc_num;

void create_process();
int config(int type);

void schedule(sche* alg, int preem);
int JOB_schedule(int time, int type);

void run_prc(sche* sche_alg, prc* running_p);
int IO_request(prc* a);
int seek_ag(prc* RQ, int num_in_Q, int time);

int simul_IO(int time, int type);
void simul_FCFS();
void simul_SJF(int preem);
void simul_PRIO(int preem);
void simul_RR();
void simul_PRIO_ag(int preem);

void rec_sche_rslt(sche* alg, int preem);
void evaluation();

void print_PRC(sche* alg, int type);
void print_Gantt_chart(prc* queue, int num);

void sort_Queue(prc *pS, int strt, int end, int type);
void cnvrt(prc *pA, prc *pB);
void enQueue(prc* q, int num, prc a);
void deQueue(prc* q, int num);
void copy_Queue(prc* from, prc* to, int num);


int main() {

	do {
		printf("Enter the total number of processes to simulate (MAX %d)  ", MAX_PRC_NUM);
		scanf("%d", &total_prc_num);
	} while (total_prc_num > MAX_PRC_NUM);

	config(0);

 	create_process();


    schedule(&FCFS, FALSE);		// FCFS
    schedule(&SJF, FALSE);        // Non - Preemptive SJF
    schedule(&SJF, TRUE);       // Preemptive SJF
	schedule(&PRIO, FALSE);;    // Non - Preemptive Priority
 	schedule(&PRIO, TRUE);    // Preemptive Priority
   	schedule(&RR, FALSE);     // Round Robin
 	schedule(&PRIO_ag, FALSE);
  	schedule(&PRIO_ag, TRUE);

	evaluation();

}


void create_process() {
	int n;
	for (n = 0; n < total_prc_num; ++n) {
		prc p;
		p.pid = n;                  //Process ID : 1-50
		p.cpu_burst_t = rand() % 30 + 1;    // 1~30
		p.arrival_t = rand() % 50;       // Arrival time
		p.priority = rand() % 35 + 1;        // Priority 1-35

		p.io_num = 0;
		p.io_strt_t[0] = p.cpu_burst_t;
		int x;
		for(x = 0; x < 5 && (p.io_strt_t[p.io_num + 1] = p.io_strt_t[p.io_num] - 1 - rand() % 5) > 0 && rand() % 3 == 0; x++) {
			++p.io_num;
		}
	 	p.io_burst_t = (p.io_num == 0 ? 0 : rand() % 10 + 1);
		p.io_strt_t[0] = p.io_burst_t;
		p.io_strt_t[6] = 0;

		p.terminated_t = 0;
		p.waiting_t = 0;
		p.turnaround_t = 0;

		NEW_Queue[n] = p;
	}
	int i, k;
	for (i = 0; i < 5; ++i)
		for (k = 0; k < total_prc_num; ++k)
			(*Alg[i]).CPU_used_t += NEW_Queue[k].cpu_burst_t;
	print_PRC(NULL, 0);
}


int config(int type) {
	int i, n;
	if (type == 0) {
		for (i = 0; i < 5; ++i) {
			(*Alg[i]).id = i;
			(*Alg[i]).CPU_util = (*Alg[i]).ave_turn_t = (*Alg[i]).ave_waiting_t = (*Alg[i]).cont_s_t = 0;
			(*Alg[i]).preem_ave_turn_t = (*Alg[i]).preem_ave_waiting_t = (*Alg[i]).preem_CPU_util = (*Alg[i]).preem_cont_s_t = 0;
		 	(*Alg[i]).CPU_used_t = 0.0;
		}
		for (n = 0; n < MAX_PRC_NUM; ++n) {
			prc p;
			p.pid = -1;                  //Process ID
			p.cpu_burst_t = 0;
			p.arrival_t = 0;
			p.priority = 40;

			p.io_num = 0;
			p.io_burst_t = 0;      // I/O burst time

			p.terminated_t = 0;
			p.waiting_t = 0;
			p.turnaround_t = 0;

			NEW_Queue[n] = p;
		}
	}
	for (i = 0; i < 5; ++i) {
		(*Alg[i]).num_in_RQ = (*Alg[i]).num_in_FQ = (*Alg[i]).num_in_TQ = 0;
	}
	job_queue_num = total_prc_num;
	copy_Queue(NEW_Queue, JOB_Queue, MAX_PRC_NUM);
	sort_Queue(JOB_Queue, 0, total_prc_num - 1, 0);
	wait_queue_num = 0;
	Wait_Queue[0].io_strt_t[6] = 0;
	IO_TQ_num = 0;
	return TRUE;
}


int JOB_schedule (int time, int type) {
	int admitted_prc = FALSE;
	if (job_queue_num != 0) {
		while (JOB_Queue[0].arrival_t == time) {
			switch (type) {
			case 0: enQueue(FCFS.RQ, FCFS.num_in_RQ++, JOB_Queue[0]); break;
			case 1: enQueue(SJF.RQ, SJF.num_in_RQ++, JOB_Queue[0]); break;
			case 2: enQueue(PRIO.RQ, PRIO.num_in_RQ++, JOB_Queue[0]); break;
			case 3: enQueue(RR.RQ, RR.num_in_RQ++, JOB_Queue[0]); break;
			case 4: enQueue(PRIO_ag.RQ, PRIO_ag.num_in_RQ++, JOB_Queue[0]); break;
			}
			printf("time %d : P%d >> to ready queue\n", time, JOB_Queue[0].pid);
			deQueue(JOB_Queue, job_queue_num--);
			admitted_prc = TRUE;
		}
	}
	return admitted_prc;
}

void schedule(sche* alg, int preem) {
	printf("\nSimulation starts...\n");
	config(1);
	if ((*alg).id == FCFS.id) {
		printf("================================================== FCFS ===================================================\n");
		simul_FCFS();
	}
	else if ((*alg).id == SJF.id) {
		if (preem)
			printf("============================================= Preemptive SJF  =============================================\n");
		else
			printf("=========================================== Non-Preemptive SJF ============================================\n");
		simul_SJF(preem);
	}
	else if ((*alg).id == PRIO.id) {
		if (preem)
			printf("=========================================== Preemptive Priority ===========================================\n");
		else
			printf("========================================= Non-Preemptive Priority =========================================\n");
		simul_PRIO(preem);
	}
	else if ((*alg).id == RR.id) {
		printf("============================================= Round Robin ==============================================\n");
		printf("time quantumn : %d\n", TIME_QUANTUM);
		simul_RR();
	}
	else if ((*alg).id == PRIO_ag.id) {
		if (preem)
			printf("===================================== Preemptive Priority with aging ======================================\n");
		else
			printf("=================================== Non-Preemptive Priority with aging ====================================\n");
		simul_PRIO_ag(preem);
	}
	printf("=========================================================================================================\n\n");
	print_Gantt_chart((*alg).TQ, (*alg).num_in_TQ);
	printf("=========== FOR I/O =======================================================================================\n");
	print_Gantt_chart(IO_TQ, IO_TQ_num);
	rec_sche_rslt(alg, preem);
	print_PRC(alg, 1);
}

int IO_request(prc* p) {
	if ((*p).io_strt_t[6] < (*p).io_num && (*p).cpu_burst_t == (*p).io_strt_t[(*p).io_strt_t[6] + 1]) {
		++(*p).io_strt_t[6];
		return TRUE;
	}
	return FALSE;
}

int simul_IO(int time, int type) {
	int termed_prc = FALSE;
	if (wait_queue_num != 0) {
		if (Wait_Queue[0].io_burst_t > 0) {
			enQueue(IO_TQ, IO_TQ_num++, Wait_Queue[0]);
			--Wait_Queue[0].io_burst_t;
		}
		else if (Wait_Queue[0].io_burst_t == 0) {
			Wait_Queue[0].io_burst_t = Wait_Queue[0].io_strt_t[0];
			Wait_Queue[0].waiting_t -= time;    // I/O complete time
			switch (type) {
			case 0: enQueue(FCFS.RQ, FCFS.num_in_RQ++, Wait_Queue[0]); break;
			case 1: enQueue(SJF.RQ, SJF.num_in_RQ++, Wait_Queue[0]); break;
			case 2: enQueue(PRIO.RQ, PRIO.num_in_RQ++, Wait_Queue[0]); break;
			case 3: enQueue(RR.RQ, RR.num_in_RQ++, Wait_Queue[0]); break;
			case 4: enQueue(PRIO_ag.RQ, PRIO_ag.num_in_RQ++, Wait_Queue[0]); break;
			}
			termed_prc = TRUE;
			printf("time %d : P%d >> I/O complete, to ready queue\n", time, Wait_Queue[0].pid);
			deQueue(Wait_Queue, wait_queue_num--);
		}
	}
	else
		enQueue(IO_TQ, IO_TQ_num++, io_idle);
	return termed_prc;
}

void run_prc(sche* alg, prc* running_p) {
	enQueue((*alg).TQ, (*alg).num_in_TQ++, *running_p);
	(*running_p).cpu_burst_t--;
}

void simul_FCFS() {
	int time = 1;
	prc* prc_run;
	while ((job_queue_num + FCFS.num_in_RQ + wait_queue_num) != 0) {
		while (FCFS.num_in_RQ == 0) {
			if (!JOB_schedule(time, FCFS.id) && !simul_IO(time, FCFS.id)) {
		 		printf("time %d : CPU idle\n", time);
				enQueue(FCFS.TQ, FCFS.num_in_TQ++, cpu_idle);
				++time;
			}
		}
		JOB_schedule(time, FCFS.id);
		prc_run = &FCFS.RQ[0];
		printf("time %d : P%d >>>>>>>>>>>>>>>>>>>>>>>>>>>>>> start running\n", time, (*prc_run).pid);
		for (; (*prc_run).cpu_burst_t > 0 && !IO_request(prc_run); ++time) {
			run_prc(&FCFS, prc_run);
			JOB_schedule(time, FCFS.id);
			simul_IO(time, FCFS.id);
		}
		if ((*prc_run).cpu_burst_t == 0) {
			(*prc_run).terminated_t = time;
			FCFS.FQ[FCFS.num_in_FQ++] = *prc_run;
			printf("time %d : P%d >>>>>>>>>>>>>>>>>>>>>>>>>>>>>> terminated\n", time, (*prc_run).pid);
		}
		else {
			(*prc_run).waiting_t += time;
			enQueue(Wait_Queue, wait_queue_num++, (*prc_run));
			printf("time %d : P%d >> I/O request\n", time, (*prc_run).pid);
		}
		deQueue(FCFS.RQ, FCFS.num_in_RQ--);
	}
	FCFS.fin_t = time;
}

void simul_SJF(int preem) {
	int time = 1;
	prc* prc_run;
	while ((job_queue_num + SJF.num_in_RQ + wait_queue_num) != 0) {
		while (SJF.num_in_RQ == 0) {
			if (!JOB_schedule(time, SJF.id) && !simul_IO(time, SJF.id)) {
				printf("time %d : CPU idle\n", time);
				enQueue(SJF.TQ, SJF.num_in_TQ++, cpu_idle);
				++time;
			}
		}
		JOB_schedule(time, SJF.id);
		sort_Queue(SJF.RQ, 0, SJF.num_in_RQ - 1, 1);
		prc_run = &SJF.RQ[0];
		int run_pid = (*prc_run).pid;
		printf("time %d : P%d >>>>>>>>>>>>>>>>>>>>>>>>>>>>>> start running\n", time, (*prc_run).pid);
		for (; (*prc_run).cpu_burst_t > 0 && !IO_request(prc_run); ++time) {
			if (JOB_schedule(time, SJF.id) || simul_IO(time, SJF.id)) {
				if (preem) {
					sort_Queue(SJF.RQ, 0, SJF.num_in_RQ - 1, 1);
				}
				else
					sort_Queue(SJF.RQ, 1, SJF.num_in_RQ - 1, 1);
			}
			if ((*prc_run).pid == run_pid)
				run_prc(&SJF, prc_run);
			else {
				printf("---> P%d preempted by P%d\n", run_pid, (*prc_run).pid); break;
			}
		}
		if ((*prc_run).cpu_burst_t == 0) {
			(*prc_run).terminated_t = time;
			SJF.FQ[SJF.num_in_FQ++] = *prc_run;
			printf("time %d : P%d >>>>>>>>>>>>>>>>>>>>>>>>>>>>>> terminated\n", time, (*prc_run).pid);
		}
		else if ((*prc_run).pid == run_pid){
			(*prc_run).waiting_t += time;
			enQueue(Wait_Queue, wait_queue_num++, (*prc_run));
			printf("time %d : P%d >> I/O request\n", time, (*prc_run).pid);
		}
		if ((*prc_run).pid == run_pid)
			deQueue(SJF.RQ, SJF.num_in_RQ--);
	}
	if (preem)
		SJF.preem_fin_t = time;
	else
		SJF.fin_t = time;
}

void simul_PRIO(int preem) {
	int time = 1;
	prc* prc_run;
	while ((job_queue_num + PRIO.num_in_RQ + wait_queue_num) != 0) {
		while (PRIO.num_in_RQ == 0) {
			if (!JOB_schedule(time, PRIO.id) && !simul_IO(time, PRIO.id)) {
				printf("time %d : CPU idle\n", time);
				enQueue(PRIO.TQ, PRIO.num_in_TQ++, cpu_idle);
				++time;
			}
		}
		JOB_schedule(time, PRIO.id);
		sort_Queue(PRIO.RQ, 0, PRIO.num_in_RQ - 1, 2);
		prc_run = &PRIO.RQ[0];
		int run_pid = (*prc_run).pid;
		printf("time %d : P%d >>>>>>>>>>>>>>>>>>>>>>>>>>>>>> start running\n", time, (*prc_run).pid);
		for (; (*prc_run).cpu_burst_t > 0 && !IO_request(prc_run); ++time) {
			if (JOB_schedule(time, PRIO.id) || simul_IO(time, PRIO.id)) {
				if (preem)
					sort_Queue(PRIO.RQ, 0, PRIO.num_in_RQ - 1, 2);
				else
					sort_Queue(PRIO.RQ, 1, PRIO.num_in_RQ - 1, 2);
			}
			if ((*prc_run).pid == run_pid)
				run_prc(&PRIO, prc_run);
			else {
				printf("---> P%d preempted by P%d\n", run_pid, (*prc_run).pid); break;
			}
		}
		if ((*prc_run).cpu_burst_t == 0) {
			(*prc_run).terminated_t = time;
			PRIO.FQ[PRIO.num_in_FQ++] = *prc_run;
			printf("time %d : P%d >>>>>>>>>>>>>>>>>>>>>>>>>>>>>> terminated\n", time, (*prc_run).pid);
		}
		else if ((*prc_run).pid == run_pid) {
			(*prc_run).waiting_t += time;
			enQueue(Wait_Queue, wait_queue_num++, (*prc_run));
			printf("time %d : P%d >> I/O request\n", time, (*prc_run).pid);
		}
		if ((*prc_run).pid == run_pid)
			deQueue(PRIO.RQ, PRIO.num_in_RQ--);
	}
	if (preem)
		PRIO.preem_fin_t = time;
	else
		PRIO.fin_t = time;
}

void simul_RR() {
	int time = 1, strt_t;
	prc* prc_run;
	while ((job_queue_num + RR.num_in_RQ + wait_queue_num) != 0) {
		while (RR.num_in_RQ == 0) {
			if (!JOB_schedule(time, RR.id) && !simul_IO(time, RR.id)) {
				printf("time %d : CPU idle\n", time);
				enQueue(RR.TQ, RR.num_in_TQ++, cpu_idle);
				++time;
			}
		}
		JOB_schedule(time, RR.id);
		prc_run = &RR.RQ[0];
		printf("time %d : P%d >>>>>>>>>>>>>>>>>>>>>>>>>>>>>> start running\n", time, (*prc_run).pid);
		for (strt_t = time; (*prc_run).cpu_burst_t > 0 && time - strt_t < TIME_QUANTUM && !IO_request(prc_run); ++time) {
			run_prc(&RR, prc_run);
			JOB_schedule(time, RR.id);
			simul_IO(time, RR.id);
		}
		if ((*prc_run).cpu_burst_t == 0) {
			(*prc_run).terminated_t = time;
			RR.FQ[RR.num_in_FQ++] = (*prc_run);
			printf("time %d : P%d >>>>>>>>>>>>>>>>>>>>>>>>>>>>>> terminated\n", time, (*prc_run).pid);
		}
		else if (time - strt_t == TIME_QUANTUM) {
			enQueue(RR.RQ, RR.num_in_RQ++, (*prc_run));
			printf("time %d : P%d >> time quantum expired\n", time, (*prc_run).pid);
		}
		else {
			(*prc_run).waiting_t += time;
			enQueue(Wait_Queue, wait_queue_num++, (*prc_run));
			printf("time %d : P%d >> I/O request\n", time, (*prc_run).pid);
		}
		deQueue(RR.RQ, RR.num_in_RQ--);
	}
	RR.fin_t = time;
}

int seek_ag(prc* RQ, int num_in_Q, int time) {
	int rslt = FALSE;
	int i;
	for (i = 0; i < num_in_Q; ++i)
		if (time - RQ[i].arrival_t == 100 && RQ[i].priority - 5 > 0) {
			if (!rslt)
				printf("time %d aging : ", time);
			RQ[i].priority -= 5;
			RQ[i].arrival_t += 100;
			rslt = TRUE;
			printf("P%d ", RQ[i].pid);
		printf("%d -> %d", RQ[i].priority+5, RQ[i].priority);
		}
	if (rslt)
		printf("\n");
	return rslt;
}

void simul_PRIO_ag (int preem) {
	int time = 1;
	prc* prc_run;
	while ((job_queue_num + PRIO_ag.num_in_RQ + wait_queue_num) != 0) {
		while (PRIO_ag.num_in_RQ == 0) {
			if (!JOB_schedule(time, PRIO_ag.id) && !simul_IO(time, PRIO_ag.id)) {
				printf("time %d : CPU idle\n", time);
				enQueue(PRIO_ag.TQ, PRIO_ag.num_in_TQ++, cpu_idle);
				++time;
			}
		}
		JOB_schedule(time, PRIO_ag.id);
	 	seek_ag(PRIO_ag.RQ, PRIO_ag.num_in_RQ, time);

		sort_Queue(PRIO_ag.RQ, 0, PRIO_ag.num_in_RQ - 1, 2);
		prc_run = &PRIO_ag.RQ[0];
		int run_pid = (*prc_run).pid;
		printf("time %d : P%d >>>>>>>>>>>>>>>>>>>>>>>>>>>>>> start running\n", time, (*prc_run).pid);
		for (; (*prc_run).cpu_burst_t > 0 && !IO_request(prc_run); ++time) {
			if (JOB_schedule(time, PRIO_ag.id) || simul_IO(time, PRIO_ag.id) || seek_ag(PRIO_ag.RQ, PRIO_ag.num_in_RQ, time)) {
				if (preem)
					sort_Queue(PRIO_ag.RQ, 0, PRIO_ag.num_in_RQ - 1, 2);
				else
					sort_Queue(PRIO_ag.RQ, 1, PRIO_ag.num_in_RQ - 1, 2);
			}
			if ((*prc_run).pid == run_pid)
				run_prc(&PRIO_ag, prc_run);
			else {
				printf("---> P%d preempted by P%d\n", run_pid, (*prc_run).pid); break;
			}
		}
		if ((*prc_run).cpu_burst_t == 0) {
			(*prc_run).terminated_t = time;
			PRIO_ag.FQ[PRIO_ag.num_in_FQ++] = *prc_run;
			printf("time %d : P%d >>>>>>>>>>>>>>>>>>>>>>>>>>>>>> terminated\n", time, (*prc_run).pid);
		}
		else if ((*prc_run).pid == run_pid) {
			(*prc_run).waiting_t += time;
			enQueue(Wait_Queue, wait_queue_num++, (*prc_run));
			printf("time %d : P%d >> I/O request\n", time, (*prc_run).pid);
		}
		if ((*prc_run).pid == run_pid)
			deQueue(PRIO_ag.RQ, PRIO_ag.num_in_RQ--);
	}
	if (preem)
		PRIO_ag.preem_fin_t = time;
	else
		PRIO_ag.fin_t = time;
}

void rec_sche_rslt(sche* alg, int preem) {
	int i;
	sort_Queue((*alg).FQ, 0, total_prc_num - 1, 3);
	for (i = 0; i < total_prc_num; i++) {
		(*alg).FQ[i].cpu_burst_t = NEW_Queue[i].cpu_burst_t;
		(*alg).FQ[i].arrival_t = NEW_Queue[i].arrival_t;
		if ((*alg).FQ[i].io_num == 0)
			(*alg).FQ[i].io_burst_t = 0;
	}
	sort_Queue((*alg).FQ, 0, total_prc_num - 1, 4);
	for (i = 0; i < total_prc_num; i++) {
		(*alg).FQ[i].turnaround_t = (*alg).FQ[i].terminated_t - (*alg).FQ[i].arrival_t;
		(*alg).FQ[i].waiting_t += (*alg).FQ[i].turnaround_t - (*alg).FQ[i].cpu_burst_t;
		if (preem) {
			(*alg).preem_ave_waiting_t += (*alg).FQ[i].waiting_t;
			(*alg).preem_ave_turn_t += (*alg).FQ[i].turnaround_t;
		}
		else {
			(*alg).ave_waiting_t += (*alg).FQ[i].waiting_t;
			(*alg).ave_turn_t += (*alg).FQ[i].turnaround_t;
		}
	}
	if (preem) {
		(*alg).preem_ave_waiting_t /= total_prc_num;
		(*alg).preem_ave_turn_t /= total_prc_num;
		(*alg).preem_CPU_util = (*alg).CPU_used_t / ((*alg).preem_fin_t - 1) * 100.0;
	}
	else {
		(*alg).ave_waiting_t /= total_prc_num;
		(*alg).ave_turn_t /= total_prc_num;
		(*alg).CPU_util = (*alg).CPU_used_t / ((*alg).fin_t - 1) * 100.0;
	}
}

void evaluation() {
	int i, n;
	printf("Simulation finished...\n\n\n");
	printf("=============================================== Evaluation ==============================================\n");
	printf("        Algorithm        average waiting time   average turnaround time   finish time    CPU utilization\n");
	printf("--------------------------------------------------------------------------------------------------------\n");
	char* alg_name[8] = { "          FCFS                 ","   Non-Preemptive SJF           ","     Preemptive SJF               " ,
						"Non-Preemptive Priority        ", "  Preemptive Priority             ", "         RR                    ",
				"Non-Preemptive Priority with aging", "Preemptive Priority with aging  " };
	printf("%s     %.2f              %.2f                   %.0f             %.2f\n", alg_name[0], (*Alg[0]).ave_waiting_t, (*Alg[0]).ave_turn_t, (*Alg[0]).fin_t, (*Alg[0]).CPU_util);
	for (i = 1, n = 1; n < 5; i++) {
		printf("%s    %.2f               %.2f                   %.0f             %.2f\n", alg_name[n++], (*Alg[i]).ave_waiting_t, (*Alg[i]).ave_turn_t, (*Alg[i]).fin_t, (*Alg[i]).CPU_util);
		printf("%s  %.2f               %.2f                   %.0f             %.2f\n ", alg_name[n++], (*Alg[i]).preem_ave_waiting_t, (*Alg[i]).preem_ave_turn_t, (*Alg[i]).preem_fin_t, (*Alg[i]).preem_CPU_util);
	}
	printf("%s    %.2f               %.2f                   %.0f             %.2f\n", alg_name[5], (*Alg[3]).ave_waiting_t, (*Alg[3]).ave_turn_t, (*Alg[3]).fin_t, (*Alg[3]).CPU_util);
	printf("%s    %.2f             %.2f                   %.0f             %.2f\n", alg_name[6], (*Alg[4]).ave_waiting_t, (*Alg[4]).ave_turn_t, (*Alg[4]).fin_t, (*Alg[4]).CPU_util);
	printf("%s  %.2f               %.2f                   %.0f             %.2f\n ", alg_name[7], (*Alg[4]).preem_ave_waiting_t, (*Alg[4]).preem_ave_turn_t,(*Alg[4]).preem_fin_t, (*Alg[4]).preem_CPU_util);
	printf("=========================================================================================================\n\n\n");
}

void print_PRC(sche* alg, int type) {
	int i;
	switch(type) {
	case 0:
		printf("\n============================================= PROCESS LIST ===============================================\n");
		printf("   pid      Arrival time      CPU burst time      I/O burst time    number of I/O request     Priority\n");
		printf("----------------------------------------------------------------------------------------------------------\n");
		for (i = 0; i < total_prc_num; ++i) {
			prc p = NEW_Queue[i];
			printf("    %d            %d                    %d                  %d               %d                  %d\n",
				p.pid, p.arrival_t, p.cpu_burst_t, p.io_burst_t, p.io_num, p.priority);
		}
		printf("==========================================================================================================\n\n");
		printf("Process number : %d\nTime quantum : %d\nAging every %d time in ready queue since arrival\n\n", total_prc_num, TIME_QUANTUM, AGING);
		break;
	case 1:
		printf("====================================== PROCESS after Scheduling =========================================\n");
		printf("          pid              finish time              waiting time               turaround time     \n");
		printf("---------------------------------------------------------------------------------------------------------\n");
		for (i = 0; i < total_prc_num; ++i) {
			prc p = (*alg).FQ[i];
			printf("           %d                   %d                       %d                        %d               \n",
				p.pid, p.terminated_t, p.waiting_t, p.turnaround_t);
		}
		printf("==========================================================================================================\n");
		break;
	}
}

void print_Gantt_chart(prc* queue, int num) {
	int i;
	printf("============================================= Gantt Chart ===============================================\n");
	for (i = 0; i < num; ++i) {
		if (queue[i].pid == cpu_idle.pid || queue[i].pid == io_idle.pid)
			printf("il.");
		else if (queue[i].pid < 10)
			printf("0%d.", queue[i].pid);
		else
			printf("%d.", +queue[i].pid);
	}
	printf("\n=========================================================================================================\n\n");
}

void sort_Queue(prc *pS, int strt, int end, int type) {
	int x, wall = strt;
	if (strt >= end)
		return;
	cnvrt(pS + strt, pS + (strt + end) / 2);
	for (x = strt + 1; x <= end; ++x) {
		switch (type) {
			case 0:
				if (pS[x].arrival_t < pS[strt].arrival_t)
					cnvrt(pS + x, pS + ++wall); break;
			case 1:
				if (pS[x].cpu_burst_t < pS[strt].cpu_burst_t)
					cnvrt(pS + x, pS + ++wall);
				else if (pS[x].cpu_burst_t == pS[strt].cpu_burst_t && pS[x].arrival_t < pS[strt].arrival_t)
					cnvrt(pS + x, pS + ++wall); break;
			case 2:
				if (pS[x].priority < pS[strt].priority)
					cnvrt(pS + x, pS + ++wall);
				else if (pS[x].priority == pS[strt].priority && pS[x].arrival_t < pS[strt].arrival_t)
					cnvrt(pS + x, pS + ++wall); break;
			case 3:
				if ( pS[x].pid < pS[strt].pid)
					cnvrt(pS + x, pS + ++wall); break;
			case 4:
				if (pS[x].terminated_t < pS[strt].terminated_t)
					cnvrt(pS + x, pS + ++wall); break;
		}
	}
	cnvrt(pS + strt, pS + wall);
	sort_Queue(pS, strt, wall - 1, type);
	sort_Queue(pS, wall + 1, end, type);
}

void cnvrt(prc *pA, prc *pB) {
	prc tmp;
	tmp = *pA;
	*pA = *pB;
	*pB = tmp;
}

void enQueue(prc* q, int num, prc a) {
	if (num + 1 < MAX_RECORD_NUM)
		q[num] = a;
	else
		printf(".......................%d, error:Queue is already full. pid %d cannot be enqueued.\n", num, a.pid);
}


void deQueue(prc* q, int num) {
	int i;
	if (num - 1 >= 0) {
		for (i = 1; i < num; ++i)
			q[i - 1] = q[i];
		q[num - 1] = df_prc;
	}
	else
		printf(".......................%d, error:Queue is empty.\n", num);
}

void copy_Queue(prc* from, prc* to, int num) {
	int i;
	for (i = 0; i < num; i++)
		to[i] = from[i];
}

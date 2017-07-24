/*==============================================================================
Project: BSF-simulator
Theme:   BSF-framework
Module:  mpipp.cpp (main)
Author:  Nadezhda Ezhova
Date:    2016-11-09
==============================================================================*/

#include <mpi.h>
#include "stdafx.h"

using namespace std;

struct Order {  // The order structure
	bool exit;		// An exit condition
	char message[_MASTER_ORD_LEN];
};

struct Report { // Result message structure
	char message[_SLAVE_REP_LEN];
};

/* ----------- Параметры MPI ----------- */
static int	_rank;	             // Number of the MPI-process
static int	_size;	             // Number of MPI-processes
static int	K;                   // Number of slaves 
static int	_master;             // Number of master-process
static Report *_reportBox;	     // An array of results
static Order  *_orderBox;	       // An array of orders
static MPI_Status  _status;      // MPI service variable
static MPI_Request _request;     // MPI service variable

static double L = 2E-5; // latency
static double t_s = 0;  // time that the master-node is engaged in sending one order to one slave-node excluding the latency
static double t_r = 0;  // total time that the master-node is engaged in receiving the results from all the slave-nodes excluding the latency
static double t_w = 0;  // total time that a slave-nodes are engaged in execution of an orders
static double t_p = 0;  // total time that the master-node is engaged in evaluating the results received from all the slave-nodes

int main(int argc, char **argv)
{
	Init();          // Initialization

	if (_rank == 0) {
		Master();      // Master-section
	}
	else {
		Slave();       // Slave-section
	}
	
	MPI_Finalize();  // Finalization
	
	return 0;
}

static void Init() { // Initialization

	int rc = MPI_Init(NULL, NULL);

	if (rc != MPI_SUCCESS) {
		printf("Error starting MPI program. Terminating! \n");
		MPI_Abort(MPI_COMM_WORLD, rc);
	};

	MPI_Comm_rank(MPI_COMM_WORLD, &_rank);	// assign to _rank varible number of the MPI-process
	MPI_Comm_size(MPI_COMM_WORLD, &_size);	// assign to _size varible the number of MPI-processes

	K = _size - 1;  // number of slaves
	_master = 0;	  // number of the master

	_reportBox = (Report*)malloc(_size * sizeof(Report));
	_orderBox  = (Order* )malloc(_size * sizeof(Order));
}

static void Master() { // Master-section	

	// Output of initial data
	cout << "MPI size = " << _size << endl;
	cout << "Number of slaves = " << K << endl;
	cout << "Master rank = " << _master << endl << endl;

	cout << "ORDER LENGTH: " << _MASTER_ORD_LEN << " byte, " << _MASTER_ORD_LEN / 1024 << " Kb, " << (double) _MASTER_ORD_LEN / 1024 / 1024 << " Mb" << endl;
	cout << "REPORT LENGTH: " << _SLAVE_REP_LEN << " byte, " << _SLAVE_REP_LEN / 1024 / 1024 << " Mb" << endl << endl;

	double start_time, end_time;
	double send_start, send_end, receive_start, receive_end;
	
	MPI_Status*  statuses = (MPI_Status*) malloc(K * sizeof(MPI_Status));
	MPI_Request* requests = (MPI_Request*) malloc(K * sizeof(MPI_Request));

	MPI_Barrier(MPI_COMM_WORLD);
	//-------------------------- Begin of Iterative Process --------------------------
	start_time = MPI_Wtime(); 
	
	for (int i = 0; i < _MLIC; i++) {

		send_start = MPI_Wtime();
	
		for (int rank = 1; rank < _size; rank++) { 
			MPI_Isend(                    // sending the order to slaves
				&_orderBox[rank],           // address of first byte of the message
				sizeof(Order),              // number of bytes sending
				MPI_BYTE,                   // data type
				rank,                       // number of the receiving node
				0,                          // message id
				MPI_COMM_WORLD,             // communicator
				&requests[rank-1]);
		};

		MPI_Waitall(K, requests, statuses);  // Synchronization 
		send_end = MPI_Wtime();

		MPI_Barrier(MPI_COMM_WORLD);         // Synchronization 
		receive_start = MPI_Wtime();
		
		for (int rank = 1; rank < _size; rank++) { 
			MPI_Irecv(                      // receiving the results from slaves
				&_reportBox[rank],            // address of first byte of the message
				sizeof(Report) / K,           // number of bytes receiving
				MPI_BYTE,                     // data type
				rank,                         // number of the sending node
				0,                            // message id
				MPI_COMM_WORLD,               // communicator
				&requests[rank-1]);
		};
		
		MPI_Waitall(K, requests, statuses);		// Synchronization 
		receive_end = MPI_Wtime();

		t_s = (send_end - send_start) / K - L;
		t_r = receive_end - receive_start - (K * L);
				
		// Simulation of the results evaluation
		#ifdef _CLUSTER
			usleep(_PROCESS_TIME);
		#else
			Sleep(_PROCESS_TIME); 
		#endif
	};
	
	end_time = MPI_Wtime(); 
  //--------------------------- End of Iterative Process ---------------------------
	
	double runtime = end_time - start_time;

	t_w = _WORK_TIME / 1E6;
	t_p = _PROCESS_TIME / 1E6;
	
	assert(t_s != 0);
	double v = log10(t_w / t_s);
	
	// Results output
	cout << "LATENCY = " << L << endl;
	cout << "t_s = " << t_s << endl;
	cout << "t_w = " << t_w << endl;    
	cout << "t_r = " << t_r << endl;
	cout << "t_p = " << t_p << endl << endl;

	cout << "Real runtime = " << runtime << endl;
	cout << "Estimated runtime = " << K*(L+t_s)+t_w/K+K*L + t_r+t_p << endl << endl;
	
	cout << "v = log10(t_w / t_s) = " << v << endl << endl;

	cout << "======================================" << endl;
	cout << "t_s" << "\t" <<  "v"  << "\t" << "runtime"<< endl;
	cout << t_s << "\t" << v << "\t" << runtime << endl;
};

static void Slave() { // Slave-section
	
	MPI_Barrier(MPI_COMM_WORLD);     // Synchronization 
	
	//-------------------------- Begin of Iterative Process --------------------------
	for (int i = 0; i < _MLIC; i++) {
		MPI_Recv(                      // Receiving the order
			&_orderBox[_rank],           // address of first byte of the message
			sizeof(Order),               // number of bytes receiving
			MPI_BYTE,                    // data type
			_master,                     // number of the sending node
			0,                           // message id
			MPI_COMM_WORLD,              // communicator
			&_status);

		double start_t_w_div_K = MPI_Wtime();
		
		// Simulation of the order processing
		#ifdef _CLUSTER
			usleep(_WORK_TIME / K);
		#else
			Sleep(worktime);
		#endif
		double end_t_w_div_K = MPI_Wtime();

		t_w = K * (end_t_w_div_K - start_t_w_div_K);

		if (_rank == 1)
			cout << "Real t_w = " <<  t_w << endl;

		MPI_Barrier(MPI_COMM_WORLD);    // Synchronization 
		
		MPI_Send(                       // Sending the result
			&_reportBox[_rank],           // address of first byte of the message
			sizeof(Report) / K,           // number of bytes sending
			MPI_BYTE,                     // data type
			_master,                      // number of the receiving node
			0,                            // message id
			MPI_COMM_WORLD);              // communicator
	};
	//--------------------------- End of Iterative Process ---------------------------
};

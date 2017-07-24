/*==============================================================================
Project: BSF-simulator
Theme:   BSF-framework
Module:  Parameters.h
Author:  Nadezhda Ezhova
Date:    2016-12-27
==============================================================================*/

#pragma once

/* ========================= Compilation modes ======================== */

#define _CLUSTER

/* ========================= Programm parameters ====================== */
#define _MASTER_ORD_LEN (110*1024*1024)  // Order array length (bytes)
#define _SLAVE_REP_LEN  (1*1024*1024)    // Result array length (bytes)

#define _MLIC	1  // Master Loop Iteration Count

#define _WORK_TIME_MICROS     1E+6  // t_w - total time that a slave-nodes are engaged in execution of an orders
#define _PROCESS_TIME_MICROS  1E+5  // t_p - total time that the master-node is engaged in evaluating the results received from all the slave-nodes

#ifdef _CLUSTER
#define _WORK_TIME     _WORK_TIME_MICROS        // t_w (microseconds - OS Linux)
#define _PROCESS_TIME  _PROCESS_TIME_MICROS     // t_p (microseconds - OS Linux)
#else
#define _WORK_TIME    (_WORK_TIME_MICROS*10E+3)     // t_w (milliseconds - OS Windows)
#define _PROCESS_TIME (_PROCESS_TIME_MICROS*10E+3)  // t_p (milliseconds - OS Windows)
#endif

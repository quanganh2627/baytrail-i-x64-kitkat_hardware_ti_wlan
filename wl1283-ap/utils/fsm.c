/***************************************************************************
**+----------------------------------------------------------------------+**
**|                                ****                                  |**
**|                                ****                                  |**
**|                                ******o***                            |**
**|                          ********_///_****                           |**
**|                           ***** /_//_/ ****                          |**
**|                            ** ** (__/ ****                           |**
**|                                *********                             |**
**|                                 ****                                 |**
**|                                  ***                                 |**
**|                                                                      |**
**|     Copyright (c) 1998 - 2009 Texas Instruments Incorporated         |**
**|                        ALL RIGHTS RESERVED                           |**
**|                                                                      |**
**| Permission is hereby granted to licensees of Texas Instruments       |**
**| Incorporated (TI) products to use this computer program for the sole |**
**| purpose of implementing a licensee product based on TI products.     |**
**| No other rights to reproduce, use, or disseminate this computer      |**
**| program, whether in part or in whole, are granted.                   |**
**|                                                                      |**
**| TI makes no representation or warranties with respect to the         |**
**| performance of this computer program, and specifically disclaims     |**
**| any responsibility for any damages, special or consequential,        |**
**| connected with the use of this program.                              |**
**|                                                                      |**
**+----------------------------------------------------------------------+**
***************************************************************************/
/** \file fsm.c
 *  \brief finite state machine source code
 *
 *  \see fsm.h
 */


/***************************************************************************/
/*																		   */
/*		MODULE:	fsm.c													   */
/*    PURPOSE:	Finite State Machine source code						   */
/*																	 	   */
/***************************************************************************/

#define __FILE_ID__  FILE_ID_127
#include "tidef.h"
#include "osApi.h"
#include "report.h"
#include "fsm.h"

/* Constants */

/* Enumerations */

/* Typedefs */

/* Structures */

/* External data definitions */

/* External functions definitions */

/* Function prototypes */

/**
*
* fsm_Init  - Initialize the FSM structure
*
* \b Description:
*
* Init The FSM structure. If matrix argument is NULL, allocate memory for
* new matrix.
*
* \b ARGS:
*
*  O   - pFsm - the generated FSM module  \n
*  I   - noOfStates - Number of states in the module \n
*  I   - noOfStates - Number of events in the module \n
*  I/O - matrix - the state event matrix
*  I   - transFunc - Transition finction for the state machine \n
*
* \b RETURNS:
*
*  TI_OK on success, TI_NOK on failure
*
* \sa fsm_Event
*/
TI_STATUS fsm_Create(TI_HANDLE				hOs,
                     fsm_stateMachine_t		**pFsm,
                     TI_UINT8					MaxNoOfStates,
                     TI_UINT8					MaxNoOfEvents)
{
    /* check for perliminary conditions */
    if ((pFsm == NULL) || (MaxNoOfStates == 0) || (MaxNoOfEvents == 0))
    {
        return TI_NOK;
    }

    /* allocate memory for FSM context */
    *pFsm = (fsm_stateMachine_t *)os_memoryAlloc(hOs, sizeof(fsm_stateMachine_t));
    if (*pFsm == NULL)
    {
        return TI_NOK;
    }
    os_memoryZero(hOs, (*pFsm), sizeof(fsm_stateMachine_t));

    /* allocate memory for FSM matrix */
    (*pFsm)->stateEventMatrix = (fsm_Matrix_t)os_memoryAlloc(hOs, MaxNoOfStates * MaxNoOfEvents * sizeof(fsm_actionCell_t));
    if ((*pFsm)->stateEventMatrix == NULL)
    {
        os_memoryFree(hOs, *pFsm, sizeof(fsm_stateMachine_t));
        return TI_NOK;
    }
    os_memoryZero(hOs, (*pFsm)->stateEventMatrix,
                  (MaxNoOfStates * MaxNoOfEvents * sizeof(fsm_actionCell_t)));
    /* update pFsm structure with parameters */
    (*pFsm)->MaxNoOfStates = MaxNoOfStates;
    (*pFsm)->MaxNoOfEvents = MaxNoOfEvents;

    return(TI_OK);
}

/**
*
* fsm_Unload  - free all memory allocated to FSM structure
*
* \b Description:
*
* Unload the FSM structure.
*
* \b ARGS:
*
*  O   - pFsm - the generated FSM module  \n
*  I   - noOfStates - Number of states in the module \n
*  I   - noOfStates - Number of events in the module \n
*  I/O - matrix - the state event matrix
*  I   - transFunc - Transition finction for the state machine \n
*
* \b RETURNS:
*
*  TI_OK on success, TI_NOK on failure
*
* \sa fsm_Event
*/
TI_STATUS fsm_Unload(TI_HANDLE				hOs,
                     fsm_stateMachine_t		*pFsm)
{
    /* check for perliminary conditions */
    if (pFsm == NULL)
    {
        return TI_NOK;
    }

    /* free memory of FSM matrix */
    if (pFsm->stateEventMatrix != NULL)
    {
        os_memoryFree(hOs, pFsm->stateEventMatrix,
                      pFsm->MaxNoOfStates * pFsm->MaxNoOfEvents * sizeof(fsm_actionCell_t));
    }

    /* free memory for FSM context (no need to check for null) */
    os_memoryFree(hOs, pFsm, sizeof(fsm_stateMachine_t));

    return(TI_OK);
}

/**
*
* fsm_Init  - Initialize the FSM structure
*
* \b Description:
*
* Init The FSM structure. If matrix argument is NULL, allocate memory for
* new matrix.
*
* \b ARGS:
*
*  O   - pFsm - the generated FSM module  \n
*  I   - noOfStates - Number of states in the module \n
*  I   - noOfStates - Number of events in the module \n
*  I/O - matrix - the state event matrix
*  I   - transFunc - Transition finction for the state machine \n
*
* \b RETURNS:
*
*  TI_OK on success, TI_NOK on failure
*
* \sa fsm_Event
*/
TI_STATUS fsm_Config(fsm_stateMachine_t	*pFsm,
                     fsm_Matrix_t			pMatrix,
                     TI_UINT8					ActiveNoOfStates,
                     TI_UINT8					ActiveNoOfEvents,
                     fsm_eventActivation_t	transFunc,
                     TI_HANDLE				hOs)
{
    /* check for perliminary conditions */
    if ((pFsm == NULL) ||
            (pMatrix == NULL))
    {
        return TI_NOK;
    }

    if ((ActiveNoOfStates > pFsm->MaxNoOfStates) ||
            (ActiveNoOfEvents > pFsm->MaxNoOfEvents))
    {
        return TI_NOK;
    }

    /* copy matrix to FSM context */
    os_memoryCopy(hOs, (void *)pFsm->stateEventMatrix, (void *)pMatrix,
                  ActiveNoOfStates * ActiveNoOfEvents * sizeof(fsm_actionCell_t));

    /* update pFsm structure with parameters */
    pFsm->ActiveNoOfStates = ActiveNoOfStates;
    pFsm->ActiveNoOfEvents = ActiveNoOfEvents;
    pFsm->transitionFunc = transFunc;
    return(TI_OK);
}

/**
*
* fsm_Event  - perform event transition in the matrix
*
* \b Description:
*
* Perform event transition in the matrix
*
* \b ARGS:
*
*  I   - pFsm - the generated FSM module  \n
*  I/O - currentState - current state of the SM \n
*  I   - event - event causing transition \n
*  I   - pData - data for activation function \n
*
* \b RETURNS:
*
*  TI_OK on success, TI_NOK on failure
*
* \sa fsm_Init
*/
TI_STATUS fsm_Event (fsm_stateMachine_t  *pFsm,
                     TI_UINT8            *currentState,
                     TI_UINT8            event,
                     void                *pData)
{
    TI_UINT8    oldState;
    TI_STATUS   status;

    /* check for FSM existance */
    if (pFsm == NULL)
    {
        return TI_NOK;
    }

    /* boundary check */
    if ((*currentState >= pFsm->ActiveNoOfStates) || (event >= pFsm->ActiveNoOfEvents))
    {
        return TI_NOK;
    }

    oldState = *currentState;
    /* update current state */
    *currentState = pFsm->stateEventMatrix[(*currentState * pFsm->ActiveNoOfEvents) + event].nextState;

    /* activate transition function */
    if ((*pFsm->stateEventMatrix[(oldState * pFsm->ActiveNoOfEvents) + event].actionFunc) == NULL)
    {
        return TI_NOK;
    }
    status = (*pFsm->stateEventMatrix[(oldState * pFsm->ActiveNoOfEvents) + event].actionFunc)(pData);

    return status;
}


/**
*
* fsm_GetNextState  - Retrun the next state for a given current state and an event.
*
* \b Description:
*
* Retrun the next state for a given current state and an event.
*
* \b ARGS:
*
*  I   - pFsm - the generated FSM module  \n
*  I   - currentState - current state of the SM \n
*  I   - event - event causing transition \n
*  O   - nextState - returned next state \n
*
* \b RETURNS:
*
*  TI_OK on success, TI_NOK on failure
*
* \sa
*/
TI_STATUS fsm_GetNextState(fsm_stateMachine_t		*pFsm,
                           TI_UINT8					currentState,
                           TI_UINT8					event,
                           TI_UINT8					*nextState)
{
    if (pFsm != NULL)
    {
        if ((currentState < pFsm->ActiveNoOfStates) && (event < pFsm->ActiveNoOfEvents))
        {
            *nextState = pFsm->stateEventMatrix[(currentState * pFsm->ActiveNoOfEvents) + event].nextState;
            return(TI_OK);
        }
    }

    return(TI_NOK);
}

TI_STATUS action_nop(void *pData)
{
    return TI_OK;
}

/* list.c
 *
 * This Module implements the List API
 */

/*****************************************************************************
* Copyright (c) 2012-2020 by Rambus, Inc. and/or its subsidiaries.
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 2 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*****************************************************************************/

/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
 */

// List API
#include "list.h"


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Default configuration
#include "c_list.h"

// Driver Framework Basic Definitions API
#include "basic_defs.h"


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

typedef struct
{
    // List head
    List_Element_t * Head_p;

    // List tail
    List_Element_t * Tail_p;

    // Number of elements in the list
    unsigned int ElementCount;

} List_t;

/*----------------------------------------------------------------------------
 * Local variables
 */

// Statically allocated list instances. This will be deprecated in future.
static List_t List [LIST_MAX_NOF_INSTANCES];


/*----------------------------------------------------------------------------
 * List_Init
 *
 */
List_Status_t
List_Init(
        const unsigned int ListID,
        void * const ListInstance_p)
{
#ifdef LIST_STRICT_ARGS
    if (ListID >= LIST_MAX_NOF_INSTANCES)
        return LIST_ERROR_BAD_ARGUMENT;
#endif // LIST_STRICT_ARGS

    // Initialize the list instance
    {
        List_t * List_p;

        if (ListInstance_p)
            List_p = (List_t*)ListInstance_p;
        else
            List_p = &List[ListID];

        List_p->ElementCount    = 0;
        List_p->Head_p          = NULL;
        List_p->Tail_p          = NULL;
    }

    return LIST_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * List_Uninit
 *
 */
List_Status_t
List_Uninit(
        const unsigned int ListID,
        void * const ListInstance_p)
{
#ifdef LIST_STRICT_ARGS
    if (ListID >= LIST_MAX_NOF_INSTANCES)
        return LIST_ERROR_BAD_ARGUMENT;
#endif // LIST_STRICT_ARGS

    // Un-initialize the list instance
    {
        List_t * List_p;

        if (ListInstance_p)
            List_p = (List_t*)ListInstance_p;
        else
            List_p = &List[ListID];

        List_p->ElementCount    = 0;
        List_p->Head_p          = NULL;
        List_p->Tail_p          = NULL;
    }

    return LIST_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * List_AddToHead
 *
 */
List_Status_t
List_AddToHead(
        const unsigned int ListID,
        void * const ListInstance_p,
        List_Element_t * const Element_p)
{
#ifdef LIST_STRICT_ARGS
    if (ListID >= LIST_MAX_NOF_INSTANCES)
        return LIST_ERROR_BAD_ARGUMENT;

    if (Element_p == NULL)
        return LIST_ERROR_BAD_ARGUMENT;
#endif // LIST_STRICT_ARGS

    // Add the element at the list head
    {
        List_Element_t * TempElement_p;
        List_t * List_p;

        if (ListInstance_p)
            List_p = (List_t*)ListInstance_p;
        else
            List_p = &List[ListID];

        TempElement_p           = List_p->Head_p;
        List_p->Head_p          = Element_p;

        // Previous element in the list, this is a head
        Element_p->Internal[0]  = NULL;

        // Next element in the list
        Element_p->Internal[1]  = TempElement_p;

        // Check if this is the first element
        if (List_p->ElementCount == 0)
            List_p->Tail_p = List_p->Head_p;
        else
            // Link the old head to the new head
            TempElement_p->Internal[0] = Element_p;

        List_p->ElementCount++;
    }

    return LIST_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * List_RemoveFromTail
 *
 */
List_Status_t
List_RemoveFromTail(
        const unsigned int ListID,
        void * const ListInstance_p,
        List_Element_t ** const Element_pp)
{
#ifdef LIST_STRICT_ARGS
    if (ListID >= LIST_MAX_NOF_INSTANCES)
        return LIST_ERROR_BAD_ARGUMENT;

    if (Element_pp == NULL)
        return LIST_ERROR_BAD_ARGUMENT;

#endif // LIST_STRICT_ARGS

    // Remove the element from the list tail
    {
        List_Element_t * TempElement_p;
        List_t * List_p;

        if (ListInstance_p)
            List_p = (List_t*)ListInstance_p;
        else
            List_p = &List[ListID];

#ifdef LIST_STRICT_ARGS
        if (List_p->ElementCount == 0)
            return LIST_ERROR_BAD_ARGUMENT;
#endif // LIST_STRICT_ARGS

        // Get the previous for the tail element in the list
        TempElement_p = (List_Element_t*)List_p->Tail_p->Internal[0];

        List_p->Tail_p->Internal[0] = NULL;
        List_p->Tail_p->Internal[1] = NULL;
        *Element_pp                 = List_p->Tail_p;

        // Set the new tail
        List_p->Tail_p              = TempElement_p;

        // Check if this is the last element
        if (List_p->ElementCount == 1)
            List_p->Head_p = NULL;
        else
            // New tail must have no next element
            List_p->Tail_p->Internal[1] = NULL;

        List_p->ElementCount--;
    }

    return LIST_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * List_RemoveAnywhere
 */
List_Status_t
List_RemoveAnywhere(
        const unsigned int ListID,
        void * const ListInstance_p,
        List_Element_t * const Element_p)
{
#ifdef LIST_STRICT_ARGS
    if (ListID >= LIST_MAX_NOF_INSTANCES)
        return LIST_ERROR_BAD_ARGUMENT;

    if (Element_p == NULL)
        return LIST_ERROR_BAD_ARGUMENT;
#endif // LIST_STRICT_ARGS

    // Remove the element from the list tail
    {
        List_Element_t * PrevElement_p, * NextElement_p;
        List_t * List_p;

        if (ListInstance_p)
            List_p = (List_t*)ListInstance_p;
        else
            List_p = &List[ListID];

#ifdef LIST_STRICT_ARGS
        if (List_p->ElementCount == 0)
            return LIST_ERROR_BAD_ARGUMENT;

        // Check element belongs to this list
        {
            unsigned int i;
            List_Element_t * TempElement_p = List_p->Head_p;

            for (i = 0; i < List_p->ElementCount; i++)
            {
                List_Element_t * p;

                if (TempElement_p == Element_p)
                    break; // Found

                p = TempElement_p->Internal[1];
                if (p)
                    TempElement_p = p; // not end of list yet
                else
                    return LIST_ERROR_BAD_ARGUMENT; // Not found
            }

            if (TempElement_p != Element_p)
                return LIST_ERROR_BAD_ARGUMENT; // Not found
        }
#endif // LIST_STRICT_ARGS

        PrevElement_p = Element_p->Internal[0];
        NextElement_p = Element_p->Internal[1];

        Element_p->Internal[0] = NULL;
        Element_p->Internal[1] = NULL;

        if (PrevElement_p)
            PrevElement_p->Internal[1] = NextElement_p;
        else
            List_p->Head_p = NextElement_p;

        if (NextElement_p)
            NextElement_p->Internal[0] = PrevElement_p;
        else
            List_p->Tail_p = PrevElement_p;

        List_p->ElementCount--;
    }

    return LIST_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * List_GetListElementCount
 *
 */
List_Status_t
List_GetListElementCount(
        const unsigned int ListID,
        void * const ListInstance_p,
        unsigned int * const Count_p)
{
#ifdef LIST_STRICT_ARGS
    if (ListID >= LIST_MAX_NOF_INSTANCES)
        return LIST_ERROR_BAD_ARGUMENT;

    if (Count_p == NULL)
        return LIST_ERROR_BAD_ARGUMENT;
#endif // LIST_STRICT_ARGS

    {
        List_t * List_p;

        if (ListInstance_p)
            List_p = (List_t*)ListInstance_p;
        else
            List_p = &List[ListID];

        *Count_p = List_p->ElementCount;
    }

    return LIST_STATUS_OK;
}


#ifdef LIST_FULL_API
/*----------------------------------------------------------------------------
 * List_RemoveFromHead
 *
 */
List_Status_t
List_RemoveFromHead(
        const unsigned int ListID,
        void * const ListInstance_p,
        List_Element_t ** const Element_pp)
{
    List_t * List_p;

    if (ListInstance_p)
        List_p = (List_t*)ListInstance_p;
    else
        List_p = &List[ListID];

#ifdef LIST_STRICT_ARGS
    if (ListID >= LIST_MAX_NOF_INSTANCES)
        return LIST_ERROR_BAD_ARGUMENT;

    if (Element_pp == NULL)
        return LIST_ERROR_BAD_ARGUMENT;

    if (List_p->ElementCount == 0)
        return LIST_ERROR_BAD_ARGUMENT;
#endif // LIST_STRICT_ARGS

    // Remove the element from the list head
    {
        List_Element_t * TempElement_p;

        // Get the next for the head element in the list
        TempElement_p = (List_Element_t*)List_p->Head_p->Internal[1];

        List_p->Head_p->Internal[0] = NULL;
        List_p->Head_p->Internal[1] = NULL;
        *Element_pp                 = List_p->Head_p;

        List_p->Head_p              = TempElement_p;

        // Check if this is the last element
        if (List_p->ElementCount == 1)
            List_p->Tail_p = NULL;
        else
            List_p->Head_p->Internal[0] = NULL;

        List_p->ElementCount--;
    }

    return LIST_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * List_GetHead
 */
List_Status_t
List_GetHead(
        const unsigned int ListID,
        void * const ListInstance_p,
        const List_Element_t ** const Element_pp)
{
#ifdef LIST_STRICT_ARGS
    if (ListID >= LIST_MAX_NOF_INSTANCES)
        return LIST_ERROR_BAD_ARGUMENT;

    if (Element_pp == NULL)
        return LIST_ERROR_BAD_ARGUMENT;
#endif // LIST_STRICT_ARGS

    // Get the list head
    {
        List_t * List_p;

        if (ListInstance_p)
            List_p = (List_t*)ListInstance_p;
        else
            List_p = &List[ListID];

        *Element_pp = List_p->Head_p;
    }

    return LIST_STATUS_OK;
}
#endif // LIST_FULL_API


/*----------------------------------------------------------------------------
 * List_GetInstanceByteCount
 *
 * Gets the memory size of the list instance (in bytes) excluding the list
 * elements memory size. This list memory size can be used to allocate a list
 * instance and pass a pointer to it subsequently to the List_*() functions.
 *
 * This function is re-entrant and can be called any time.
 *
 * Return Values
 *     Size of the list administration memory in bytes.
 */
unsigned int
List_GetInstanceByteCount(void)
{
    return sizeof(List_t);
}


/* end of file list.c */

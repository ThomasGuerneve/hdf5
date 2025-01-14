/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright by The HDF Group.                                               *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of HDF5.  The full HDF5 copyright notice, including     *
 * terms governing use, modification, and redistribution, is contained in    *
 * the COPYING file, which can be found at the root of the source code       *
 * distribution tree, or in https://support.hdfgroup.org/ftp/HDF5/releases.  *
 * If you do not have access to either file, you may request a copy from     *
 * help@hdfgroup.org.                                                        *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*
 * Purpose:     The Virtual Object Layer as described in documentation.
 *              The pupose is to provide an abstraction on how to access the
 *              underlying HDF5 container, whether in a local file with
 *              a specific file format, or remotely on other machines, etc...
 */

/****************/
/* Module Setup */
/****************/

#include "H5VLmodule.h"         /* This source code file is part of the H5VL module */


/***********/
/* Headers */
/***********/

#include "H5private.h"          /* Generic Functions                    */
#include "H5Eprivate.h"         /* Error handling                       */
#include "H5Iprivate.h"         /* IDs                                  */
#include "H5Pprivate.h"         /* Property lists                       */
#include "H5VLpkg.h"            /* Virtual Object Layer                 */

/* VOL connectors */
#include "H5VLnative.h"         /* Native VOL connector                 */

/****************/
/* Local Macros */
/****************/


/******************/
/* Local Typedefs */
/******************/


/********************/
/* Local Prototypes */
/********************/


/*********************/
/* Package Variables */
/*********************/


/*****************************/
/* Library Private Variables */
/*****************************/


/*******************/
/* Local Variables */
/*******************/



/*-------------------------------------------------------------------------
 * Function:    H5VLregister_connector
 *
 * Purpose:     Registers a new VOL connector as a member of the virtual object
 *              layer class.
 *
 *              VIPL_ID is a VOL initialization property list which must be
 *              created with H5Pcreate(H5P_VOL_INITIALIZE) (or H5P_DEFAULT).
 *
 * Return:      Success:    A VOL connector ID which is good until the
 *                          library is closed or the connector is
 *                          unregistered.
 *
 *              Failure:    H5I_INVALID_HID
 *
 *-------------------------------------------------------------------------
 */
hid_t
H5VLregister_connector(const H5VL_class_t *cls, hid_t vipl_id)
{
    hid_t ret_value = H5I_INVALID_HID;          /* Return value */

    FUNC_ENTER_API(H5I_INVALID_HID)
    H5TRACE2("i", "*xi", cls, vipl_id);

    /* Check arguments */
    if (!cls)
        HGOTO_ERROR(H5E_ARGS, H5E_UNINITIALIZED, H5I_INVALID_HID, "VOL connector class pointer cannot be NULL")
    if (!cls->name)
        HGOTO_ERROR(H5E_VOL, H5E_CANTREGISTER, H5I_INVALID_HID, "VOL connector class name cannot be the NULL pointer")
    if (0 == HDstrlen(cls->name))
        HGOTO_ERROR(H5E_VOL, H5E_CANTREGISTER, H5I_INVALID_HID, "VOL connector class name cannot be the empty string")
    if (cls->info_cls.copy && !cls->info_cls.free)
        HGOTO_ERROR(H5E_VOL, H5E_CANTREGISTER, H5I_INVALID_HID, "VOL connector must provide free callback for VOL info objects when a copy callback is provided")
    if (cls->wrap_cls.get_wrap_ctx && !cls->wrap_cls.free_wrap_ctx)
        HGOTO_ERROR(H5E_VOL, H5E_CANTREGISTER, H5I_INVALID_HID, "VOL connector must provide free callback for object wrapping contexts when a get callback is provided")

   /* Check VOL initialization property list */
    if(H5P_DEFAULT == vipl_id)
        vipl_id = H5P_VOL_INITIALIZE_DEFAULT;
    else
        if(TRUE != H5P_isa_class(vipl_id, H5P_VOL_INITIALIZE))
            HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, H5I_INVALID_HID, "not a VOL initialize property list")

    /* Register connector */
    if((ret_value = H5VL__register_connector(cls, TRUE, vipl_id)) < 0)
        HGOTO_ERROR(H5E_VOL, H5E_CANTREGISTER, H5I_INVALID_HID, "unable to register VOL connector")

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5VLregister_connector() */


/*-------------------------------------------------------------------------
 * Function:    H5VLregister_connector_by_name
 *
 * Purpose:     Registers a new VOL connector as a member of the virtual object
 *              layer class.
 *
 *              VIPL_ID is a VOL initialization property list which must be
 *              created with H5Pcreate(H5P_VOL_INITIALIZE) (or H5P_DEFAULT).
 *
 * Return:      Success:    A VOL connector ID which is good until the
 *                          library is closed or the connector is
 *                          unregistered.
 *
 *              Failure:    H5I_INVALID_HID
 *
 *-------------------------------------------------------------------------
 */
hid_t
H5VLregister_connector_by_name(const char *name, hid_t vipl_id)
{
    hid_t ret_value = H5I_INVALID_HID;          /* Return value */

    FUNC_ENTER_API(H5I_INVALID_HID)
    H5TRACE2("i", "*si", name, vipl_id);

    /* Check arguments */
    if (!name)
        HGOTO_ERROR(H5E_ARGS, H5E_UNINITIALIZED, H5I_INVALID_HID, "null VOL connector name is disallowed")
    if (0 == HDstrlen(name))
        HGOTO_ERROR(H5E_ARGS, H5E_UNINITIALIZED, H5I_INVALID_HID, "zero-length VOL connector name is disallowed")

   /* Check VOL initialization property list */
    if(H5P_DEFAULT == vipl_id)
        vipl_id = H5P_VOL_INITIALIZE_DEFAULT;
    else
        if(TRUE != H5P_isa_class(vipl_id, H5P_VOL_INITIALIZE))
            HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, H5I_INVALID_HID, "not a VOL initialize property list")

    /* Register connector */
    if((ret_value = H5VL__register_connector_by_name(name, TRUE, vipl_id)) < 0)
        HGOTO_ERROR(H5E_VOL, H5E_CANTREGISTER, H5I_INVALID_HID, "unable to register VOL connector")

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5VLregister_connector_by_name() */


/*-------------------------------------------------------------------------
 * Function:    H5VLregister_connector_by_value
 *
 * Purpose:     Registers a new VOL connector as a member of the virtual object
 *              layer class.
 *
 *              VIPL_ID is a VOL initialization property list which must be
 *              created with H5Pcreate(H5P_VOL_INITIALIZE) (or H5P_DEFAULT).
 *
 * Return:      Success:    A VOL connector ID which is good until the
 *                          library is closed or the connector is
 *                          unregistered.
 *
 *              Failure:    H5I_INVALID_HID
 *
 *-------------------------------------------------------------------------
 */
hid_t
H5VLregister_connector_by_value(H5VL_class_value_t value, hid_t vipl_id)
{
    hid_t ret_value = H5I_INVALID_HID;          /* Return value */

    FUNC_ENTER_API(H5I_INVALID_HID)
    H5TRACE2("i", "VCi", value, vipl_id);

    /* Check arguments */
    if(value < 0)
        HGOTO_ERROR(H5E_ARGS, H5E_UNINITIALIZED, H5I_INVALID_HID, "negative VOL connector value is disallowed")

   /* Check VOL initialization property list */
    if(H5P_DEFAULT == vipl_id)
        vipl_id = H5P_VOL_INITIALIZE_DEFAULT;
    else
        if(TRUE != H5P_isa_class(vipl_id, H5P_VOL_INITIALIZE))
            HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, H5I_INVALID_HID, "not a VOL initialize property list")

    /* Register connector */
    if((ret_value = H5VL__register_connector_by_value(value, TRUE, vipl_id)) < 0)
        HGOTO_ERROR(H5E_VOL, H5E_CANTREGISTER, H5I_INVALID_HID, "unable to register VOL connector")

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5VLregister_connector_by_value() */


/*-------------------------------------------------------------------------
 * Function:    H5VLis_connector_registered
 *
 * Purpose:     Tests whether a VOL class has been registered or not
 *
 * Return:      >0 if a VOL connector with that name has been registered
 *              0 if a VOL connector with that name has NOT been registered
 *              <0 on errors
 *
 * Programmer:  Dana Robinson
 *              June 17, 2017
 *
 *-------------------------------------------------------------------------
 */
htri_t
H5VLis_connector_registered(const char *name)
{
    htri_t ret_value = FALSE;           /* Return value */

    FUNC_ENTER_API(FAIL)
    H5TRACE1("t", "*s", name);

    /* Check if connector with this name is registered */
    if((ret_value = H5VL__is_connector_registered(name)) < 0)
        HGOTO_ERROR(H5E_VOL, H5E_CANTGET, FAIL, "can't check for VOL")

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5VLis_connector_registered() */


/*-------------------------------------------------------------------------
 * Function:    H5VLget_connector_id
 *
 * Purpose:     Retrieves the ID for a registered VOL connector.
 *
 * Return:      A valid VOL connector ID if a connector by that name has
 *              been registered. This ID will need to be closed using
 *              H5VLclose().
 *
 *              H5I_INVALID_HID on error or if a VOL connector of that
 *              name has not been registered.
 *
 * Programmer:  Dana Robinson
 *              June 17, 2017
 *
 *-------------------------------------------------------------------------
 */
hid_t
H5VLget_connector_id(const char *name)
{
    hid_t ret_value = H5I_INVALID_HID;  /* Return value */

    FUNC_ENTER_API(H5I_INVALID_HID)
    H5TRACE1("i", "*s", name);

    /* Get connector ID with this name */
    if((ret_value = H5VL__get_connector_id(name, TRUE)) < 0)
        HGOTO_ERROR(H5E_VOL, H5E_CANTGET, H5I_INVALID_HID, "can't get VOL id")

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5VLget_connector_id() */


/*-------------------------------------------------------------------------
 * Function:    H5VLget_connector_name
 *
 * Purpose:     Returns the connector name for the VOL associated with the
 *              object or file ID.
 *
 *              This works like other calls where the caller must provide a
 *              buffer of the appropriate size for the library to fill in.
 *              i.e., passing in a NULL pointer for NAME will return the
 *              required size of the buffer.
 *
 * Return:      Success:        The length of the connector name
 *
 *              Failure:        Negative
 *
 *-------------------------------------------------------------------------
 */
ssize_t
H5VLget_connector_name(hid_t obj_id, char *name/*out*/, size_t size)
{
    ssize_t    ret_value = -1;

    FUNC_ENTER_API(FAIL)
    H5TRACE3("Zs", "ixz", obj_id, name, size);

    /* Call internal routine */
    if((ret_value = H5VL__get_connector_name(obj_id, name, size)) < 0)
        HGOTO_ERROR(H5E_VOL, H5E_CANTGET, FAIL, "Can't get connector name")

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5VLget_connector_name() */


/*-------------------------------------------------------------------------
 * Function:    H5VLclose
 *
 * Purpose:     Closes a VOL connector ID.  This in no way affects
 *              file access property lists which have been defined to use
 *              this VOL connector or files which are already opened under with
 *              this connector.
 *
 * Return:      Success:    Non-negative
 *              Failure:    Negative
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5VLclose(hid_t vol_id)
{
    herr_t ret_value = SUCCEED;       /* Return value */

    FUNC_ENTER_API(FAIL)
    H5TRACE1("e", "i", vol_id);

    /* Check args */
    if(NULL == H5I_object_verify(vol_id, H5I_VOL))
        HGOTO_ERROR(H5E_VOL, H5E_BADTYPE, FAIL, "not a VOL connector")

    /* Decrement the ref count on the ID, possibly releasing the VOL connector */
    if(H5I_dec_app_ref(vol_id) < 0)
        HGOTO_ERROR(H5E_VOL, H5E_CANTDEC, FAIL, "unable to close VOL connector ID")

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5VLclose() */


/*-------------------------------------------------------------------------
 * Function:    H5VLunregister_connector
 *
 * Purpose:     Removes a VOL connector ID from the library. This in no way affects
 *              file access property lists which have been defined to use
 *              this VOL connector or files which are already opened under with
 *              this connector.
 *
 *              The native VOL connector cannot be unregistered and attempts
 *              to do so are considered an error.
 *
 * Return:      Success:    Non-negative
 *
 *              Failure:    Negative
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5VLunregister_connector(hid_t vol_id)
{
    hid_t native_id = H5I_INVALID_HID;
    herr_t ret_value = SUCCEED;       /* Return value */

    FUNC_ENTER_API(FAIL)
    H5TRACE1("e", "i", vol_id);

    /* Check arguments */
    if(NULL == H5I_object_verify(vol_id, H5I_VOL))
        HGOTO_ERROR(H5E_VOL, H5E_BADTYPE, FAIL, "not a VOL connector ID")

    /* For the time being, we disallow unregistering the native VOL connector */
    if(H5I_INVALID_HID == (native_id = H5VL__get_connector_id(H5VL_NATIVE_NAME, FALSE)))
        HGOTO_ERROR(H5E_VOL, H5E_CANTGET, FAIL, "unable to find the native VOL connector ID")
    if(vol_id == native_id)
        HGOTO_ERROR(H5E_VOL, H5E_BADVALUE, FAIL, "unregistering the native VOL connector is not allowed")

    /* The H5VL_class_t struct will be freed by this function */
    if(H5I_dec_app_ref(vol_id) < 0)
        HGOTO_ERROR(H5E_VOL, H5E_CANTDEC, FAIL, "unable to unregister VOL connector")

done:
    if(native_id != H5I_INVALID_HID)
        if(H5I_dec_ref(native_id) < 0)
            HGOTO_ERROR(H5E_VOL, H5E_CANTDEC, FAIL, "unable to decrement count on native_id")

    FUNC_LEAVE_API(ret_value)
} /* end H5VLunregister_connector() */


/*---------------------------------------------------------------------------
 * Function:    H5VLcmp_connector_cls
 *
 * Purpose:     Compares two connector classes (based on their value field)
 *
 * Note:        This routine is _only_ for HDF5 VOL connector authors!  It is
 *              _not_ part of the public API for HDF5 application developers.
 *
 * Return:      Success:    Non-negative, *cmp set to a value like strcmp
 *
 *              Failure:    Negative, *cmp unset
 *
 *---------------------------------------------------------------------------
 */
herr_t
H5VLcmp_connector_cls(int *cmp, hid_t connector_id1, hid_t connector_id2)
{
    H5VL_class_t *cls1, *cls2;  /* connectors for IDs */
    herr_t ret_value = SUCCEED; /* Return value */

    FUNC_ENTER_API(FAIL)
    H5TRACE3("e", "*Isii", cmp, connector_id1, connector_id2);

    /* Check args and get class pointers */
    if(NULL == (cls1 = (H5VL_class_t *)H5I_object_verify(connector_id1, H5I_VOL)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a VOL connector ID")
    if(NULL == (cls2 = (H5VL_class_t *)H5I_object_verify(connector_id2, H5I_VOL)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a VOL connector ID")

    /* Compare the two VOL connector classes */
    if(H5VL_cmp_connector_cls(cmp, cls1, cls2) < 0)
        HGOTO_ERROR(H5E_VOL, H5E_CANTCOMPARE, FAIL, "can't compare connector classes")

done:
    FUNC_LEAVE_API(ret_value)
} /* H5VLcmp_connector_cls() */


/*---------------------------------------------------------------------------
 * Function:    H5VLwrap_register
 *
 * Purpose:     Wrap an internal object with a "wrap context" and register an
 *              hid_t for the resulting object.
 *
 * Note:        This routine is mainly targeted toward wrapping objects for
 *              iteration routine callbacks (i.e. the callbacks from H5Aiterate*,
 *              H5Literate* / H5Lvisit*, and H5Ovisit* ).
 *
 * Return:      Success:    Non-negative hid_t for the object.
 *              Failure:    Negative (H5I_INVALID_HID)
 *
 *---------------------------------------------------------------------------
 */
hid_t
H5VLwrap_register(void *obj, H5I_type_t type)
{
    hid_t ret_value;            /* Return value */

    /* Use FUNC_ENTER_API_NOINIT here, so the API context doesn't get reset */
    FUNC_ENTER_API_NOINIT
    H5TRACE2("i", "*xIt", obj, type);

    /* Check args */
    if(type <= H5I_BADID || type >= H5I_NTYPES)
        HGOTO_ERROR(H5E_VOL, H5E_BADRANGE, H5I_INVALID_HID, "invalid type number")
    if(NULL == obj)
        HGOTO_ERROR(H5E_VOL, H5E_BADVALUE, H5I_INVALID_HID, "obj is NULL")

    /* Wrap the object and register an ID for it */
    if((ret_value = H5VL_wrap_register(type, obj, TRUE)) < 0)
        HGOTO_ERROR(H5E_VOL, H5E_CANTREGISTER, H5I_INVALID_HID, "unable to wrap object")

done:
    FUNC_LEAVE_API_NOINIT(ret_value)
} /* H5VLwrap_register() */


/*---------------------------------------------------------------------------
 * Function:    H5VLobject
 *
 * Purpose:     Retrieve the object pointer associated with an hid_t for a.
 *              VOL object.
 *
 * Note:        This routine is mainly targeted toward unwrapping objects for
 *              testing.
 *
 * Return:      Success:    Object pointer
 *              Failure:    NULL
 *
 *---------------------------------------------------------------------------
 */
void *
H5VLobject(hid_t id)
{
    void *ret_value;            /* Return value */

    FUNC_ENTER_API(NULL)
    H5TRACE1("*x", "i", id);

    /* Retrieve the object pointer for the ID */
    if(NULL == (ret_value = H5VL_object(id)))
        HGOTO_ERROR(H5E_VOL, H5E_CANTGET, NULL, "unable to retrieve object")

done:
    FUNC_LEAVE_API(ret_value)
} /* H5VLobject() */


/*---------------------------------------------------------------------------
 * Function:    H5VLretrieve_lib_state
 *
 * Purpose:     Retrieves a copy of the internal state of the HDF5 library,
 *              so that it can be restored later.
 *
 * Note:        This routine is _only_ for HDF5 VOL connector authors!  It is
 *              _not_ part of the public API for HDF5 application developers.
 *
 * Return:      Success:    Non-negative, *state set
 *              Failure:    Negative, *state unset
 *
 * Programmer:  Quincey Koziol
 *              Thursday, January 10, 2019
 *
 *---------------------------------------------------------------------------
 */
herr_t
H5VLretrieve_lib_state(void **state)
{
    herr_t ret_value = SUCCEED; /* Return value */

    /* Must use this, to avoid modifying the API context stack in FUNC_ENTER */
    FUNC_ENTER_API_NOINIT
    H5TRACE1("e", "**x", state);

    /* Check args */
    if(NULL == state)
        HGOTO_ERROR(H5E_VOL, H5E_BADVALUE, FAIL, "invalid state pointer")

    /* Retrieve the library state */
    if(H5VL_retrieve_lib_state(state) < 0)
        HGOTO_ERROR(H5E_VOL, H5E_CANTGET, FAIL, "can't retrieve library state")

done:
    FUNC_LEAVE_API_NOINIT(ret_value)
} /* H5VLretrieve_lib_state() */


/*---------------------------------------------------------------------------
 * Function:    H5VLrestore_lib_state
 *
 * Purpose:     Restores the internal state of the HDF5 library.
 *
 * Note:        This routine is _only_ for HDF5 VOL connector authors!  It is
 *              _not_ part of the public API for HDF5 application developers.
 *
 * Return:      Success:    Non-negative
 *              Failure:    Negative
 *
 * Programmer:  Quincey Koziol
 *              Thursday, January 10, 2019
 *
 *---------------------------------------------------------------------------
 */
herr_t
H5VLrestore_lib_state(const void *state)
{
    herr_t ret_value = SUCCEED; /* Return value */

    /* Must use this, to avoid modifying the API context stack in FUNC_ENTER */
    FUNC_ENTER_API_NOINIT
    H5TRACE1("e", "*x", state);

    /* Check args */
    if(NULL == state)
        HGOTO_ERROR(H5E_VOL, H5E_BADVALUE, FAIL, "invalid state pointer")

    /* Restore the library state */
    if(H5VL_restore_lib_state(state) < 0)
        HGOTO_ERROR(H5E_VOL, H5E_CANTSET, FAIL, "can't restore library state")

done:
    FUNC_LEAVE_API_NOINIT(ret_value)
} /* H5VLrestore_lib_state() */


/*---------------------------------------------------------------------------
 * Function:    H5VLreset_lib_state
 *
 * Purpose:     Resets the internal state of the HDF5 library, undoing the
 *              affects of H5VLrestore_lib_state.
 *
 * Note:        This routine is _only_ for HDF5 VOL connector authors!  It is
 *              _not_ part of the public API for HDF5 application developers.
 *
 * Note:        This routine must be called as a "pair" with
 *              H5VLrestore_lib_state.  It can be called before / after /
 *              independently of H5VLfree_lib_state.
 *
 * Return:      Success:    Non-negative
 *              Failure:    Negative
 *
 * Programmer:  Quincey Koziol
 *              Saturday, February 23, 2019
 *
 *---------------------------------------------------------------------------
 */
herr_t
H5VLreset_lib_state(void)
{
    herr_t ret_value = SUCCEED; /* Return value */

    /* Must use this, to avoid modifying the API context stack in FUNC_ENTER */
    FUNC_ENTER_API_NOINIT
    H5TRACE0("e","");

    /* Reset the library state */
    if(H5VL_reset_lib_state() < 0)
        HGOTO_ERROR(H5E_VOL, H5E_CANTRESET, FAIL, "can't reset library state")

done:
    FUNC_LEAVE_API_NOINIT(ret_value)
} /* H5VLreset_lib_state() */


/*---------------------------------------------------------------------------
 * Function:    H5VLfree_lib_state
 *
 * Purpose:     Free a retrieved library state.
 *
 * Note:        This routine is _only_ for HDF5 VOL connector authors!  It is
 *              _not_ part of the public API for HDF5 application developers.
 *
 * Note:        This routine must be called as a "pair" with
 *              H5VLretrieve_lib_state.
 *
 * Return:      Success:    Non-negative
 *              Failure:    Negative
 *
 * Programmer:  Quincey Koziol
 *              Thursday, January 10, 2019
 *
 *---------------------------------------------------------------------------
 */
herr_t
H5VLfree_lib_state(void *state)
{
    herr_t ret_value = SUCCEED; /* Return value */

    FUNC_ENTER_API(FAIL)
    H5TRACE1("e", "*x", state);

    /* Check args */
    if(NULL == state)
        HGOTO_ERROR(H5E_VOL, H5E_BADVALUE, FAIL, "invalid state pointer")

    /* Free the library state */
    if(H5VL_free_lib_state(state) < 0)
        HGOTO_ERROR(H5E_VOL, H5E_CANTRELEASE, FAIL, "can't free library state")

done:
    FUNC_LEAVE_API(ret_value)
} /* H5VLfree_lib_state() */


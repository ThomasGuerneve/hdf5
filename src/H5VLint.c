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
#include "H5CXprivate.h"        /* API Contexts                         */
#include "H5Eprivate.h"         /* Error handling                       */
#include "H5FLprivate.h"        /* Free lists                           */
#include "H5Iprivate.h"         /* IDs                                  */
#include "H5MMprivate.h"        /* Memory management                    */
#include "H5PLprivate.h"        /* Plugins                              */
#include "H5Tprivate.h"         /* Datatypes                            */
#include "H5VLpkg.h"            /* Virtual Object Layer                 */

/* VOL connectors */
#include "H5VLnative.h"         /* Native VOL connector                 */
#include "H5VLpassthru.h"       /* Pass-through VOL connector           */


/****************/
/* Local Macros */
/****************/


/******************/
/* Local Typedefs */
/******************/

/* Object wrapping context info */
typedef struct H5VL_wrap_ctx_t {
    unsigned rc;                /* Ref. count for the # of times the context was set / reset */
    H5VL_t *connector;          /* VOL connector for "outermost" class to start wrap */
    void *obj_wrap_ctx;         /* "wrap context" for outermost connector */
} H5VL_wrap_ctx_t;

/* Information needed for iterating over the registered VOL connector hid_t IDs.
 * The name or value of the new VOL connector that is being registered is
 * stored in the name (or value) field and the found_id field is initialized to
 * H5I_INVALID_HID (-1).  If we find a VOL connector with the same name / value,
 * we set the found_id field to the existing ID for return to the function.
 */
typedef struct {
    /* IN */
    H5VL_get_connector_kind_t kind;     /* Which kind of connector search to make */
    union {
        const char *name;               /* The name of the VOL connector to check */
        H5VL_class_value_t value;       /* The value of the VOL connector to check */
    } u;

    /* OUT */
    hid_t found_id;                     /* The connector ID, if we found a match */
} H5VL_get_connector_ud_t;


/********************/
/* Package Typedefs */
/********************/


/********************/
/* Local Prototypes */
/********************/
static herr_t H5VL__free_cls(H5VL_class_t *cls);
static int H5VL__get_connector_cb(void *obj, hid_t id, void *_op_data);
static herr_t H5VL__set_def_conn(void);
static void *H5VL__wrap_obj(void *obj, H5I_type_t obj_type);
static H5VL_object_t *H5VL__new_vol_obj(H5I_type_t type, void *object,
    H5VL_t *vol_connector, hbool_t wrap_obj);
static int64_t H5VL__conn_inc_rc(H5VL_t *connector);
static int64_t H5VL__conn_dec_rc(H5VL_t *connector);
static void *H5VL__object(hid_t id, H5I_type_t obj_type);
static herr_t H5VL__free_vol_wrapper(H5VL_wrap_ctx_t *vol_wrap_ctx);


/*********************/
/* Package Variables */
/*********************/

/* Package initialization variable */
hbool_t H5_PKG_INIT_VAR = FALSE;


/*****************************/
/* Library Private Variables */
/*****************************/


/*******************/
/* Local Variables */
/*******************/

/* VOL ID class */
static const H5I_class_t H5I_VOL_CLS[1] = {{
    H5I_VOL,                    /* ID class value */
    0,                          /* Class flags */
    0,                          /* # of reserved IDs for class */
    (H5I_free_t)H5VL__free_cls  /* Callback routine for closing objects of this class */
}};

/* Declare a free list to manage the H5VL_class_t struct */
H5FL_DEFINE_STATIC(H5VL_class_t);

/* Declare a free list to manage the H5VL_t struct */
H5FL_DEFINE(H5VL_t);

/* Declare a free list to manage the H5VL_object_t struct */
H5FL_DEFINE(H5VL_object_t);

/* Declare a free list to manage the H5VL_wrap_ctx_t struct */
H5FL_DEFINE_STATIC(H5VL_wrap_ctx_t);

/* Default VOL connector */
static H5VL_connector_prop_t H5VL_def_conn_s = {-1, NULL};



/*-------------------------------------------------------------------------
 * Function:    H5VL_init_phase1
 *
 * Purpose:     Initialize the interface from some other package.  This should
 *		be followed with a call to H5VL_init_phase2 after the H5P
 *		interface is completely set up, finish setting up the H5VL
 *		information.
 *
 *
 * Return:      Success:    Non-negative
 *              Failure:    Negative
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5VL_init_phase1(void)
{
    herr_t ret_value = SUCCEED;   /* Return value */

    FUNC_ENTER_NOAPI(FAIL)

    /* FUNC_ENTER() does all the work */

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5VL_init_phase1() */


/*-------------------------------------------------------------------------
 * Function:    H5VL_init_phase2
 *
 * Purpose:     Finish initializing the interface from some other package.
 *
 * Note:	This is broken out as a separate routine to avoid a circular
 *		reference with the H5P package.
 *
 * Return:      Success:    Non-negative
 *              Failure:    Negative
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5VL_init_phase2(void)
{
    herr_t ret_value = SUCCEED;   /* Return value */

    FUNC_ENTER_NOAPI(FAIL)

    /* Set up the default VOL connector in the default FAPL */
    if(H5VL__set_def_conn() < 0)
        HGOTO_ERROR(H5E_VOL, H5E_CANTSET, FAIL, "unable to set default VOL connector")

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5VL_init_phase2() */


/*-------------------------------------------------------------------------
 * Function:	H5VL__init_package
 *
 * Purpose:     Initialize interface-specific information
 *
 * Return:      Success:    Non-negative
 *
 *              Failure:    Negative
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5VL__init_package(void)
{
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_PACKAGE

    /* Initialize the atom group for the VL IDs */
    if(H5I_register_type(H5I_VOL_CLS) < 0)
        HGOTO_ERROR(H5E_VOL, H5E_CANTINIT, FAIL, "unable to initialize H5VL interface")

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5VL__init_package() */


/*-------------------------------------------------------------------------
 * Function:	H5VL_term_package
 *
 * Purpose:     Terminate various H5VL objects
 *
 * Return:      Success:    Positive if anything was done that might
 *                          affect other interfaces; zero otherwise.
 *              Failure:    Negative
 *
 *-------------------------------------------------------------------------
 */
int
H5VL_term_package(void)
{
    int	n = 0;

    FUNC_ENTER_NOAPI_NOINIT_NOERR

    if(H5_PKG_INIT_VAR) {
        if(H5VL_def_conn_s.connector_id > 0) {
            /* Release the default VOL connector */
            (void)H5VL_conn_free(&H5VL_def_conn_s);
            H5VL_def_conn_s.connector_id = -1;
            H5VL_def_conn_s.connector_info = NULL;
            n++;
        } /* end if */
        else {
            if(H5I_nmembers(H5I_VOL) > 0) {
                /* Unregister all VOL connectors */
                (void)H5I_clear_type(H5I_VOL, FALSE, FALSE);
                n++;
            } /* end if */
            else {
                /* Destroy the VOL connector ID group */
                n += (H5I_dec_type_ref(H5I_VOL) > 0);

                /* Mark interface as closed */
                if(0 == n)
                    H5_PKG_INIT_VAR = FALSE;
            } /* end else */
        } /* end else */
    } /* end if */

    FUNC_LEAVE_NOAPI(n)
} /* end H5VL_term_package() */


/*-------------------------------------------------------------------------
 * Function:    H5VL__free_cls
 *
 * Purpose:     Frees a file VOL class struct and returns an indication of
 *              success. This function is used as the free callback for the
 *              virtual object layer object identifiers
 *              (c.f.: H5VL_init_interface).
 *
 * Return:      SUCCEED/FAIL
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5VL__free_cls(H5VL_class_t *cls)
{
    herr_t ret_value = SUCCEED;

    FUNC_ENTER_STATIC

    /* Sanity check */
    HDassert(cls);

    /* Shut down the VOL connector */
    if(cls->terminate && cls->terminate() < 0)
        HGOTO_ERROR(H5E_VOL, H5E_CANTCLOSEOBJ, FAIL, "VOL connector did not terminate cleanly")

    /* Release the class */
    H5MM_xfree((void *)cls->name);      /* Casting away const OK -QAK */
    H5FL_FREE(H5VL_class_t, cls);

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5VL__free_cls() */


/*-------------------------------------------------------------------------
 * Function:    H5VL__get_connector_cb
 *
 * Purpose:     Callback routine to search through registered VOLs
 *
 * Return:      Success:    H5_ITER_STOP if the class and op_data name
 *                          members match. H5_ITER_CONT otherwise.
 *              Failure:    Can't fail
 *
 * Programmer:  Dana Robinson
 *              June 22, 2017
 *
 *-------------------------------------------------------------------------
 */
static int
H5VL__get_connector_cb(void *obj, hid_t id, void *_op_data)
{
    H5VL_get_connector_ud_t *op_data = (H5VL_get_connector_ud_t *)_op_data; /* User data for callback */
    H5VL_class_t *cls = (H5VL_class_t *)obj;
    int ret_value = H5_ITER_CONT;     /* Callback return value */

    FUNC_ENTER_STATIC_NOERR

    if(H5VL_GET_CONNECTOR_BY_NAME == op_data->kind) {
        if(0 == HDstrcmp(cls->name, op_data->u.name)) {
            op_data->found_id = id;
            ret_value = H5_ITER_STOP;
        } /* end if */
    } /* end if */
    else {
        HDassert(H5VL_GET_CONNECTOR_BY_VALUE == op_data->kind);
        if(cls->value == op_data->u.value) {
            op_data->found_id = id;
            ret_value = H5_ITER_STOP;
        } /* end if */
    } /* end else */

    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5VL__get_connector_cb() */


/*-------------------------------------------------------------------------
 * Function:    H5VL__set_def_conn
 *
 * Purpose:     Parses a string that contains the default VOL connector for
 *              the library.
 *
 * Note:	Usually from the environment variable "HDF5_VOL_CONNECTOR",
 *		but could be from elsewhere.
 *
 * Return:      Success:    0
 *              Failure:    -1
 *
 * Programmer:  Jordan Henderson
 *              November 2018
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5VL__set_def_conn(void)
{
    H5P_genplist_t *def_fapl;           /* Default file access property list */
    H5P_genclass_t *def_fapclass;       /* Default file access property class */
    const char *env_var;                /* Environment variable for default VOL connector */
    char *buf = NULL;                   /* Buffer for tokenizing string */
    hid_t connector_id = -1;            /* VOL conntector ID */
    void *vol_info = NULL;              /* VOL connector info */
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_STATIC

    /* Sanity check */
    HDassert(H5VL_def_conn_s.connector_id == (-1));
    HDassert(H5VL_def_conn_s.connector_info == NULL);

    /* Check for environment variable set */
    env_var = HDgetenv("HDF5_VOL_CONNECTOR");

    /* Only parse the string if it's set */
    if(env_var && *env_var) {
        char *lasts = NULL;             /* Context pointer for strtok_r() call */
        const char *tok = NULL;         /* Token from strtok_r call */
        htri_t connector_is_registered; /* Whether connector is already registered */

        /* Duplicate the string to parse, as it is modified as we go */
        if(NULL == (buf = H5MM_strdup(env_var)))
            HGOTO_ERROR(H5E_VOL, H5E_CANTALLOC, FAIL, "can't allocate memory for environment variable string")

        /* Get the first 'word' of the environment variable.
         * If it's nothing (environment variable was whitespace) return error.
         */
        if(NULL == (tok = HDstrtok_r(buf, " \t\n\r", &lasts)))
            HGOTO_ERROR(H5E_VOL, H5E_BADVALUE, FAIL, "VOL connector environment variable set empty?")

        /* First, check to see if the connector is already registered */
        if((connector_is_registered = H5VL__is_connector_registered(tok)) < 0)
            HGOTO_ERROR(H5E_VOL, H5E_CANTGET, FAIL, "can't check if VOL connector already registered")
        else if(connector_is_registered) {
            /* Retrieve the ID of the already-registered VOL connector */
            if((connector_id = H5VL__get_connector_id(tok, FALSE)) < 0)
                HGOTO_ERROR(H5E_VOL, H5E_CANTGET, FAIL, "can't get VOL connector ID")
        } /* end else-if */
        else {
            /* Check for VOL connectors that ship with the library */
            if(!HDstrcmp(tok, "native")) {
                connector_id = H5VL_NATIVE;
                if(H5I_inc_ref(connector_id, FALSE) < 0)
                    HGOTO_ERROR(H5E_VOL, H5E_CANTINC, FAIL, "can't increment VOL connector refcount")
            } /* end if */
            else if(!HDstrcmp(tok, "pass_through")) {
                connector_id = H5VL_PASSTHRU;
                if(H5I_inc_ref(connector_id, FALSE) < 0)
                    HGOTO_ERROR(H5E_VOL, H5E_CANTINC, FAIL, "can't increment VOL connector refcount")
            } /* end else-if */
            else {
                /* Register the VOL connector */
                /* (NOTE: No provisions for vipl_id currently) */
                if((connector_id = H5VL__register_connector_by_name(tok, FALSE, H5P_DEFAULT)) < 0)
                    HGOTO_ERROR(H5E_VOL, H5E_CANTREGISTER, FAIL, "can't register connector")
            } /* end else */
        } /* end else */

        /* Was there any connector info specified in the environment variable? */
        if(NULL != (tok = HDstrtok_r(NULL, " \t\n\r", &lasts)))
            if(H5VL__connector_str_to_info(tok, connector_id, &vol_info) < 0)
                HGOTO_ERROR(H5E_VOL, H5E_CANTDECODE, FAIL, "can't deserialize connector info")

        /* Set the default VOL connector */
        H5VL_def_conn_s.connector_id = connector_id;
        H5VL_def_conn_s.connector_info = vol_info;
    } /* end if */
    else {
        /* Set the default VOL connector */
        H5VL_def_conn_s.connector_id = H5_DEFAULT_VOL;
        H5VL_def_conn_s.connector_info = NULL;

        /* Increment the ref count on the default connector */
        if(H5I_inc_ref(H5VL_def_conn_s.connector_id, FALSE) < 0)
            HGOTO_ERROR(H5E_VOL, H5E_CANTINC, FAIL, "can't increment VOL connector refcount")
    } /* end else */

    /* Get default file access pclass */
    if(NULL == (def_fapclass = (H5P_genclass_t *)H5I_object(H5P_FILE_ACCESS)))
        HGOTO_ERROR(H5E_VOL, H5E_BADATOM, FAIL, "can't find object for default file access property class ID")

    /* Change the default VOL for the default file access pclass */
    if(H5P_reset_vol_class(def_fapclass, &H5VL_def_conn_s) < 0)
        HGOTO_ERROR(H5E_VOL, H5E_CANTSET, FAIL, "can't set default VOL connector for default file access property class")

    /* Get default file access plist */
    if(NULL == (def_fapl = (H5P_genplist_t *)H5I_object(H5P_FILE_ACCESS_DEFAULT)))
        HGOTO_ERROR(H5E_VOL, H5E_BADATOM, FAIL, "can't find object for default fapl ID")

    /* Change the default VOL for the default FAPL */
    if(H5P_set_vol(def_fapl, H5VL_def_conn_s.connector_id, H5VL_def_conn_s.connector_info) < 0)
        HGOTO_ERROR(H5E_VOL, H5E_CANTSET, FAIL, "can't set default VOL connector for default FAPL")

done:
    /* Clean up on error */
    if(ret_value < 0) {
        if(vol_info)
            if(H5VL_free_connector_info(connector_id, vol_info) < 0)
                HDONE_ERROR(H5E_VOL, H5E_CANTRELEASE, FAIL, "can't free VOL connector info")
        if(connector_id >= 0)
            /* The H5VL_class_t struct will be freed by this function */
            if(H5I_dec_ref(connector_id) < 0)
                HDONE_ERROR(H5E_VOL, H5E_CANTDEC, FAIL, "unable to unregister VOL connector")
    } /* end if */

    /* Clean up */
    H5MM_xfree(buf);

    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5VL__set_def_conn() */


/*-------------------------------------------------------------------------
 * Function:    H5VL__wrap_obj
 *
 * Purpose:     Wraps a library object with possible VOL connector wrappers, to
 *		match the VOL connector stack for the file.
 *
 * Return:      Success:        Wrapped object pointer
 *              Failure:        NULL
 *
 * Programmer:	Quincey Koziol
 *		Friday, October  7, 2018
 *
 *-------------------------------------------------------------------------
 */
static void *
H5VL__wrap_obj(void *obj, H5I_type_t obj_type)
{
    H5VL_wrap_ctx_t *vol_wrap_ctx = NULL;   /* Object wrapping context */
    void *ret_value = NULL;                 /* Return value */

    FUNC_ENTER_STATIC

    /* Check arguments */
    HDassert(obj);

    /* Retrieve the VOL object wrapping context */
    if(H5CX_get_vol_wrap_ctx((void **)&vol_wrap_ctx) < 0)
        HGOTO_ERROR(H5E_VOL, H5E_CANTGET, NULL, "can't get VOL object wrap context")

    /* If there is a VOL object wrapping context, wrap the object */
    if(vol_wrap_ctx) {
        /* Wrap object, using the VOL callback */
        if(NULL == (ret_value = H5VL_wrap_object(vol_wrap_ctx->connector->cls, vol_wrap_ctx->obj_wrap_ctx, obj, obj_type)))
            HGOTO_ERROR(H5E_VOL, H5E_CANTGET, NULL, "can't wrap object")
    } /* end if */
    else
        ret_value = obj;

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5VL__wrap_obj() */


/*-------------------------------------------------------------------------
 * Function:    H5VL__new_vol_obj
 *
 * Purpose:     Creates a new VOL object, to use when registering an ID.
 *
 * Return:      Success:        VOL object pointer
 *              Failure:        NULL
 *
 * Programmer:	Quincey Koziol
 *		Friday, October  7, 2018
 *
 *-------------------------------------------------------------------------
 */
static H5VL_object_t *
H5VL__new_vol_obj(H5I_type_t type, void *object, H5VL_t *vol_connector, hbool_t wrap_obj)
{
    H5VL_object_t  *new_vol_obj = NULL;     /* Pointer to new VOL object                    */
    H5VL_object_t  *ret_value = NULL;       /* Return value                                 */

    FUNC_ENTER_STATIC

    /* Check arguments */
    HDassert(object);
    HDassert(vol_connector);

    /* Make sure type number is valid */
    if(type != H5I_ATTR && type != H5I_DATASET && type != H5I_DATATYPE
            && type != H5I_FILE && type != H5I_GROUP && type != H5I_MAP)
        HGOTO_ERROR(H5E_VOL, H5E_BADVALUE, NULL, "invalid type number")

    /* Create the new VOL object */
    if(NULL == (new_vol_obj = H5FL_CALLOC(H5VL_object_t)))
        HGOTO_ERROR(H5E_VOL, H5E_CANTALLOC, NULL, "can't allocate memory for VOL object")
    new_vol_obj->connector = vol_connector;
    if(wrap_obj) {
        if(NULL == (new_vol_obj->data = H5VL__wrap_obj(object, type)))
            HGOTO_ERROR(H5E_VOL, H5E_CANTCREATE, NULL, "can't wrap library object")
    } /* end if */
    else
        new_vol_obj->data = object;

    /* Bump the reference count on the VOL connector */
    H5VL__conn_inc_rc(vol_connector);

    /* If this is a datatype, we have to hide the VOL object under the H5T_t pointer */
    if(H5I_DATATYPE == type) {
        if(NULL == (ret_value = (H5VL_object_t *)H5T_construct_datatype(new_vol_obj)))
            HGOTO_ERROR(H5E_VOL, H5E_CANTINIT, NULL, "can't construct datatype object")
    } /* end if */
    else
        ret_value = (H5VL_object_t *)new_vol_obj;

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5VL__new_vol_obj() */


/*-------------------------------------------------------------------------
 * Function:    H5VL_conn_copy
 *
 * Purpose:     Copy VOL connector ID & info.
 *
 * Note:        This is an "in-place" copy.
 *
 * Return:      Success:        Non-negative
 *              Failure:        Negative
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5VL_conn_copy(H5VL_connector_prop_t *connector_prop)
{
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_NOAPI(FAIL)

    if(connector_prop) {
        /* Copy the connector ID & info, if there is one */
        if(connector_prop->connector_id > 0) {
            /* Increment the reference count on connector ID and copy connector info */
            if(H5I_inc_ref(connector_prop->connector_id, FALSE) < 0)
                HGOTO_ERROR(H5E_PLIST, H5E_CANTINC, FAIL, "unable to increment ref count on VOL connector ID")

            /* Copy connector info, if it exists */
            if(connector_prop->connector_info) {
                H5VL_class_t *connector;           /* Pointer to connector */
                void *new_connector_info = NULL;   /* Copy of connector info */

                /* Retrieve the connector for the ID */
                if(NULL == (connector = (H5VL_class_t *)H5I_object(connector_prop->connector_id)))
                    HGOTO_ERROR(H5E_PLIST, H5E_BADTYPE, FAIL, "not a VOL connector ID")

                /* Allocate and copy connector info */
                if(H5VL_copy_connector_info(connector, &new_connector_info, connector_prop->connector_info) < 0)
                    HGOTO_ERROR(H5E_PLIST, H5E_CANTCOPY, FAIL, "connector info copy failed")

                /* Set the connector info to the copy */
                connector_prop->connector_info = new_connector_info;
            } /* end if */
        } /* end if */
    } /* end if */

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5VL_conn_copy() */


/*-------------------------------------------------------------------------
 * Function:    H5VL_conn_free
 *
 * Purpose:     Free VOL connector ID & info.
 *
 * Return:      Success:        Non-negative
 *              Failure:        Negative
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5VL_conn_free(const H5VL_connector_prop_t *connector_prop)
{
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_NOAPI(FAIL)

    if(connector_prop) {
        /* Free the connector info (if it exists) and decrement the ID */
        if(connector_prop->connector_id > 0) {
            if(connector_prop->connector_info)
                /* Free the connector info */
                if(H5VL_free_connector_info(connector_prop->connector_id, (void *)connector_prop->connector_info) < 0)        /* Casting away const OK - QAK */
                    HGOTO_ERROR(H5E_VOL, H5E_CANTRELEASE, FAIL, "unable to release VOL connector info object")

            /* Decrement reference count for connector ID */
            if(H5I_dec_ref(connector_prop->connector_id) < 0)
                HGOTO_ERROR(H5E_VOL, H5E_CANTDEC, FAIL, "can't decrement reference count for connector ID")
        } /* end if */
    } /* end if */

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5VL_conn_free() */


/*-------------------------------------------------------------------------
 * Function:    H5VL_register
 *
 * Purpose:     VOL-aware version of H5I_register. Constructs an H5VL_object_t
 *              from the passed-in object and registers that. Does the right
 *              thing with datatypes, which are complicated under the VOL.
 *
 * Return:      Success:    A valid HDF5 ID
 *              Failure:    H5I_INVALID_HID
 *
 *-------------------------------------------------------------------------
 */
hid_t
H5VL_register(H5I_type_t type, void *object, H5VL_t *vol_connector, hbool_t app_ref)
{
    H5VL_object_t  *vol_obj     = NULL;         /* VOL object wrapper for library object */
    hid_t           ret_value   = H5I_INVALID_HID;      /* Return value */

    FUNC_ENTER_NOAPI(H5I_INVALID_HID)

    /* Check arguments */
    HDassert(object);
    HDassert(vol_connector);

    /* Set up VOL object for the passed-in data */
    /* (Does not wrap object, since it's from a VOL callback) */
    if(NULL == (vol_obj = H5VL__new_vol_obj(type, object, vol_connector, FALSE)))
        HGOTO_ERROR(H5E_VOL, H5E_CANTCREATE, FAIL, "can't create VOL object")
    
    /* Register VOL object as _object_ type, for future object API calls */
    if((ret_value = H5I_register(type, vol_obj, app_ref)) < 0)
        HGOTO_ERROR(H5E_VOL, H5E_CANTREGISTER, H5I_INVALID_HID, "unable to atomize handle")

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5VL_register() */


/*-------------------------------------------------------------------------
 * Function:    H5VL_register_using_existing_id
 *
 * Purpose:     Registers an OBJECT in a TYPE with the supplied ID for it.
 *              This routine will check to ensure the supplied ID is not already
 *              in use, and ensure that it is a valid ID for the given type, 
 *              but will NOT check to ensure the OBJECT is not already
 *              registered (thus, it is possible to register one object under
 *              multiple IDs).
 *
 * NOTE:        Intended for use in refresh calls, where we have to close
 *              and re-open the underlying data, then hook the VOL object back
 *              up to the original ID.
 *
 * Return:      SUCCEED/FAIL
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5VL_register_using_existing_id(H5I_type_t type, void *object, H5VL_t *vol_connector, hbool_t app_ref, hid_t existing_id)
{
    H5VL_object_t  *new_vol_obj = NULL;     /* Pointer to new VOL object                    */
    herr_t          ret_value = SUCCEED;    /* Return value                                 */

    FUNC_ENTER_NOAPI(FAIL)

    /* Check arguments */
    HDassert(object);
    HDassert(vol_connector);

    /* Set up VOL object for the passed-in data */
    /* (Wraps object, since it's a library object) */
    if(NULL == (new_vol_obj = H5VL__new_vol_obj(type, object, vol_connector, TRUE)))
        HGOTO_ERROR(H5E_VOL, H5E_CANTCREATE, FAIL, "can't create VOL object")

    /* Call the underlying H5I function to complete the registration */
    if(H5I_register_using_existing_id(type, new_vol_obj, app_ref, existing_id) < 0)
        HGOTO_ERROR(H5E_VOL, H5E_CANTREGISTER, FAIL, "can't register object under existing ID")

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5VL_register_using_existing_id() */


/*-------------------------------------------------------------------------
 * Function:	H5VL_register_using_vol_id
 *
 * Purpose:     Utility function to create a user ID for an object created
 *              or opened through the VOL. Uses the VOL connector's ID to
 *              get the connector information instead of it being passed in.
 *
 * Return:      Success:    A valid HDF5 ID
 *              Failure:    H5I_INVALID_HID 
 *
 *-------------------------------------------------------------------------
 */
hid_t
H5VL_register_using_vol_id(H5I_type_t type, void *obj, hid_t connector_id, hbool_t app_ref)
{
    H5VL_class_t    *cls = NULL;
    H5VL_t          *connector = NULL;       /* VOL connector struct */
    hid_t           ret_value = H5I_INVALID_HID;

    FUNC_ENTER_NOAPI(FAIL)

    /* Get the VOL class object from the connector's ID */
    if(NULL == (cls = (H5VL_class_t *)H5I_object_verify(connector_id, H5I_VOL)))
        HGOTO_ERROR(H5E_VOL, H5E_BADTYPE, H5I_INVALID_HID, "not a VOL connector ID")

    /* Setup VOL info struct */
    if(NULL == (connector = H5FL_CALLOC(H5VL_t)))
        HGOTO_ERROR(H5E_VOL, H5E_CANTALLOC, H5I_INVALID_HID, "can't allocate VOL info struct")
    connector->cls = cls;
    connector->id = connector_id;
    if(H5I_inc_ref(connector->id, FALSE) < 0)
        HGOTO_ERROR(H5E_VOL, H5E_CANTINC, H5I_INVALID_HID, "unable to increment ref count on VOL connector")

    /* Get an ID for the VOL object */
    if((ret_value = H5VL_register(type, obj, connector, app_ref)) < 0)
        HGOTO_ERROR(H5E_VOL, H5E_CANTREGISTER, H5I_INVALID_HID, "unable to register object handle")

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5VL_register_using_vol_id() */


/*-------------------------------------------------------------------------
 * Function:    H5VL__conn_inc_rc
 *
 * Purpose:     Wrapper to increment the ref. count on a connector.
 *
 * Return:      Current ref. count (can't fail)
 *
 * Programmer:  Quincey Koziol
 *              February 23, 2019
 *
 *-------------------------------------------------------------------------
 */
static int64_t
H5VL__conn_inc_rc(H5VL_t *connector)
{
    FUNC_ENTER_STATIC_NOERR

    /* Check arguments */
    HDassert(connector);

    /* Increment refcount for connector */
    connector->nrefs++;

    FUNC_LEAVE_NOAPI(connector->nrefs)
} /* end H5VL__conn_inc_rc() */


/*-------------------------------------------------------------------------
 * Function:    H5VL__conn_dec_rc
 *
 * Purpose:     Wrapper to decrement the ref. count on a connector.
 *
 * Return:      Current ref. count (>=0) on success, <0 on failure
 *
 * Programmer:  Quincey Koziol
 *              February 23, 2019
 *
 *-------------------------------------------------------------------------
 */
static int64_t
H5VL__conn_dec_rc(H5VL_t *connector)
{
    int64_t ret_value = -1;     /* Return value */

    FUNC_ENTER_STATIC

    /* Check arguments */
    HDassert(connector);

    /* Decrement refcount for connector */
    connector->nrefs--;

    /* Check for last reference */
    if(0 == connector->nrefs) {
        if(H5I_dec_ref(connector->id) < 0)
            HGOTO_ERROR(H5E_VOL, H5E_CANTDEC, FAIL, "unable to decrement ref count on VOL connector")
        H5FL_FREE(H5VL_t, connector);

        /* Set return value */
        ret_value = 0;
    } /* end if */
    else
        /* Set return value */
        ret_value = connector->nrefs;

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5VL__conn_dec_rc() */


/*-------------------------------------------------------------------------
 * Function:    H5VL_free_object
 *
 * Purpose:     Wrapper to unregister an object ID with a VOL aux struct
 *              and decrement ref count on VOL connector ID
 *
 * Return:      SUCCEED/FAIL
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5VL_free_object(H5VL_object_t *vol_obj)
{
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_NOAPI(FAIL)

    /* Check arguments */
    HDassert(vol_obj);

    /* Decrement refcount on connector */
    if(H5VL__conn_dec_rc(vol_obj->connector) < 0)
        HGOTO_ERROR(H5E_VOL, H5E_CANTDEC, FAIL, "unable to decrement ref count on VOL connector")

    vol_obj = H5FL_FREE(H5VL_object_t, vol_obj);

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5VL_free_object() */


/*-------------------------------------------------------------------------
 * Function:    H5VL_register_connector
 *
 * Purpose:     Registers a new VOL connector as a member of the virtual object
 *              layer class.
 *
 * Return:      Success:    A VOL connector ID which is good until the
 *                          library is closed or the connector is unregistered.
 *
 *              Failure:    H5I_INVALID_HID 
 *
 * Programmer:  Dana Robinson
 *              June 22, 2017
 *
 *-------------------------------------------------------------------------
 */
hid_t
H5VL_register_connector(const void *_cls, hbool_t app_ref, hid_t vipl_id)
{
    const H5VL_class_t	*cls = (const H5VL_class_t *)_cls;
    H5VL_class_t	*saved = NULL;
    hid_t		ret_value = H5I_INVALID_HID;

    FUNC_ENTER_NOAPI(H5I_INVALID_HID)

    /* Check arguments */
    HDassert(cls);

    /* Copy the class structure so the caller can reuse or free it */
    if(NULL == (saved = H5FL_MALLOC(H5VL_class_t)))
        HGOTO_ERROR(H5E_VOL, H5E_CANTALLOC, H5I_INVALID_HID, "memory allocation failed for VOL connector class struct")
    H5MM_memcpy(saved, cls, sizeof(H5VL_class_t));
    if(NULL == (saved->name = H5MM_strdup(cls->name)))
        HGOTO_ERROR(H5E_VOL, H5E_CANTALLOC, H5I_INVALID_HID, "memory allocation failed for VOL connector name")

    /* Initialize the VOL connector */
    if(cls->initialize && cls->initialize(vipl_id) < 0)
        HGOTO_ERROR(H5E_VOL, H5E_CANTINIT, H5I_INVALID_HID, "unable to init VOL connector")

    /* Create the new class ID */
    if((ret_value = H5I_register(H5I_VOL, saved, app_ref)) < 0)
        HGOTO_ERROR(H5E_VOL, H5E_CANTREGISTER, H5I_INVALID_HID, "unable to register VOL connector ID")

done:
    if(ret_value < 0 && saved) {
        if(saved->name)
            H5MM_xfree((void *)(saved->name));  /* Casting away const OK -QAK */

        H5FL_FREE(H5VL_class_t, saved);
    } /* end if */

    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5VL_register_connector() */


/*-------------------------------------------------------------------------
 * Function:    H5VL__register_connector
 *
 * Purpose:     Registers a new VOL connector as a member of the virtual object
 *              layer class.
 *
 * Return:      Success:    A VOL connector ID which is good until the
 *                          library is closed or the connector is
 *                          unregistered.
 *
 *              Failure:    H5I_INVALID_HID
 *
 * Programmer:  Dana Robinson
 *              June 22, 2017
 *
 *-------------------------------------------------------------------------
 */
hid_t
H5VL__register_connector(const H5VL_class_t *cls, hbool_t app_ref, hid_t vipl_id)
{
    H5VL_get_connector_ud_t op_data;        /* Callback info for connector search */
    hid_t ret_value = H5I_INVALID_HID;      /* Return value */

    FUNC_ENTER_PACKAGE

    /* Set up op data for iteration */
    op_data.kind = H5VL_GET_CONNECTOR_BY_NAME;
    op_data.u.name = cls->name;
    op_data.found_id = H5I_INVALID_HID;

    /* Check if connector is already registered */
    if(H5I_iterate(H5I_VOL, H5VL__get_connector_cb, &op_data, TRUE) < 0)
        HGOTO_ERROR(H5E_VOL, H5E_BADITER, H5I_INVALID_HID, "can't iterate over VOL IDs")

    /* Increment the ref count on the existing VOL connector ID, if it's already registered */
    if(op_data.found_id != H5I_INVALID_HID) {
        if(H5I_inc_ref(op_data.found_id, app_ref) < 0)
            HGOTO_ERROR(H5E_VOL, H5E_CANTINC, H5I_INVALID_HID, "unable to increment ref count on VOL connector")
        ret_value = op_data.found_id;
    } /* end if */
    else {
        /* Create a new class ID */
        if((ret_value = H5VL_register_connector(cls, app_ref, vipl_id)) < 0)
            HGOTO_ERROR(H5E_VOL, H5E_CANTREGISTER, H5I_INVALID_HID, "unable to register VOL connector")
    } /* end else */

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5VL__register_connector() */


/*-------------------------------------------------------------------------
 * Function:    H5VL__register_connector_by_name
 *
 * Purpose:     Registers a new VOL connector as a member of the virtual object
 *              layer class.
 *
 * Return:      Success:    A VOL connector ID which is good until the
 *                          library is closed or the connector is
 *                          unregistered.
 *
 *              Failure:    H5I_INVALID_HID
 *
 * Programmer:  Dana Robinson
 *              June 22, 2017
 *
 *-------------------------------------------------------------------------
 */
hid_t
H5VL__register_connector_by_name(const char *name, hbool_t app_ref, hid_t vipl_id)
{
    H5VL_get_connector_ud_t op_data;        /* Callback info for connector search */
    hid_t ret_value = H5I_INVALID_HID;      /* Return value */

    FUNC_ENTER_PACKAGE

    /* Set up op data for iteration */
    op_data.kind = H5VL_GET_CONNECTOR_BY_NAME;
    op_data.u.name = name;
    op_data.found_id = H5I_INVALID_HID;

    /* Check if connector is already registered */
    if(H5I_iterate(H5I_VOL, H5VL__get_connector_cb, &op_data, TRUE) < 0)
        HGOTO_ERROR(H5E_VOL, H5E_BADITER, H5I_INVALID_HID, "can't iterate over VOL ids")

    /* If connector alread registered, increment ref count on ID and return ID */
    if(op_data.found_id != H5I_INVALID_HID) {
        if(H5I_inc_ref(op_data.found_id, app_ref) < 0)
            HGOTO_ERROR(H5E_VOL, H5E_CANTINC, H5I_INVALID_HID, "unable to increment ref count on VOL connector")
        ret_value = op_data.found_id;
    } /* end if */
    else {
        H5PL_key_t key;
        const H5VL_class_t *cls;

        /* Try loading the connector */
        key.vol.kind = H5VL_GET_CONNECTOR_BY_NAME;
        key.vol.u.name = name;
        if(NULL == (cls = (const H5VL_class_t *)H5PL_load(H5PL_TYPE_VOL, &key)))
            HGOTO_ERROR(H5E_VOL, H5E_CANTINIT, H5I_INVALID_HID, "unable to load VOL connector")

        /* Register the connector we loaded */
        if((ret_value = H5VL_register_connector(cls, app_ref, vipl_id)) < 0)
            HGOTO_ERROR(H5E_VOL, H5E_CANTREGISTER, H5I_INVALID_HID, "unable to register VOL connector ID")
    } /* end else */

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5VL__register_connector_by_name() */


/*-------------------------------------------------------------------------
 * Function:    H5VL__register_connector_by_value
 *
 * Purpose:     Registers a new VOL connector as a member of the virtual object
 *              layer class.
 *
 * Return:      Success:    A VOL connector ID which is good until the
 *                          library is closed or the connector is
 *                          unregistered.
 *
 *              Failure:    H5I_INVALID_HID
 *
 * Programmer:  Dana Robinson
 *              June 22, 2017
 *
 *-------------------------------------------------------------------------
 */
hid_t
H5VL__register_connector_by_value(H5VL_class_value_t value, hbool_t app_ref, hid_t vipl_id)
{
    H5VL_get_connector_ud_t op_data;        /* Callback info for connector search */
    hid_t ret_value = H5I_INVALID_HID;      /* Return value */

    FUNC_ENTER_PACKAGE

    /* Set up op data for iteration */
    op_data.kind = H5VL_GET_CONNECTOR_BY_VALUE;
    op_data.u.value = value;
    op_data.found_id = H5I_INVALID_HID;

    /* Check if connector is already registered */
    if(H5I_iterate(H5I_VOL, H5VL__get_connector_cb, &op_data, TRUE) < 0)
        HGOTO_ERROR(H5E_VOL, H5E_BADITER, H5I_INVALID_HID, "can't iterate over VOL ids")

    /* If connector alread registered, increment ref count on ID and return ID */
    if(op_data.found_id != H5I_INVALID_HID) {
        if(H5I_inc_ref(op_data.found_id, app_ref) < 0)
            HGOTO_ERROR(H5E_VOL, H5E_CANTINC, H5I_INVALID_HID, "unable to increment ref count on VOL connector")
        ret_value = op_data.found_id;
    } /* end if */
    else {
        H5PL_key_t key;
        const H5VL_class_t *cls;

        /* Try loading the connector */
        key.vol.kind = H5VL_GET_CONNECTOR_BY_VALUE;
        key.vol.u.value = value;
        if(NULL == (cls = (const H5VL_class_t *)H5PL_load(H5PL_TYPE_VOL, &key)))
            HGOTO_ERROR(H5E_VOL, H5E_CANTINIT, H5I_INVALID_HID, "unable to load VOL connector")

        /* Register the connector we loaded */
        if((ret_value = H5VL_register_connector(cls, app_ref, vipl_id)) < 0)
            HGOTO_ERROR(H5E_VOL, H5E_CANTREGISTER, H5I_INVALID_HID, "unable to register VOL connector ID")
    } /* end else */

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5VL__register_connector_by_value() */


/*-------------------------------------------------------------------------
 * Function:    H5VL__is_connector_registered
 *
 * Purpose:     Checks if a connector with a particular name is registered.
 *
 * Return:      Success:    0
 *              Failure:    -1
 *
 * Programmer:  Dana Robinson
 *              June 17, 2017
 *
 *-------------------------------------------------------------------------
 */
htri_t
H5VL__is_connector_registered(const char *name)
{
    H5VL_get_connector_ud_t op_data;    /* Callback info for connector search */
    htri_t ret_value = FALSE;           /* Return value */

    FUNC_ENTER_PACKAGE

    /* Set up op data for iteration */
    op_data.kind = H5VL_GET_CONNECTOR_BY_NAME;
    op_data.u.name = name;
    op_data.found_id = H5I_INVALID_HID;

    /* Find connector with name */
    if(H5I_iterate(H5I_VOL, H5VL__get_connector_cb, &op_data, TRUE) < 0)
        HGOTO_ERROR(H5E_VOL, H5E_BADITER, FAIL, "can't iterate over VOL connectors")

    /* Found a connector with that name */
    if(op_data.found_id != H5I_INVALID_HID)
        ret_value = TRUE;

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5VL__is_connector_registered() */


/*-------------------------------------------------------------------------
 * Function:    H5VL__get_connector_id
 *
 * Purpose:     Retrieves the ID for a registered VOL connector.
 *
 * Return:      Positive if the VOL class has been registered
 *              Negative on error (if the class is not a valid class or not registered)
 *
 * Programmer:  Dana Robinson
 *              June 17, 2017
 *
 *-------------------------------------------------------------------------
 */
hid_t
H5VL__get_connector_id(const char *name, hbool_t is_api)
{
    H5VL_get_connector_ud_t op_data;        /* Callback info for connector search */
    hid_t ret_value = H5I_INVALID_HID;      /* Return value */

    FUNC_ENTER_PACKAGE

    /* Set up op data for iteration */
    op_data.kind = H5VL_GET_CONNECTOR_BY_NAME;
    op_data.u.name = name;
    op_data.found_id = H5I_INVALID_HID;

    /* Find connector with name */
    if(H5I_iterate(H5I_VOL, H5VL__get_connector_cb, &op_data, TRUE) < 0)
        HGOTO_ERROR(H5E_VOL, H5E_BADITER, H5I_INVALID_HID, "can't iterate over VOL connectors")

    /* Found a connector with that name */
    if(op_data.found_id != H5I_INVALID_HID) {
        if(H5I_inc_ref(op_data.found_id, is_api) < 0)
            HGOTO_ERROR(H5E_FILE, H5E_CANTINC, H5I_INVALID_HID, "unable to increment ref count on VOL connector")
        ret_value = op_data.found_id;
    } /* end if */

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5VL__get_connector_id() */


/*-------------------------------------------------------------------------
 * Function:    H5VL__connector_str_to_info
 *
 * Purpose:     Deserializes a string into a connector's info object
 *
 * Return:      Success:    Non-negative
 *              Failure:    Negative
 *
 * Programmer:  Quincey Koziol
 *              March 2, 2019
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5VL__connector_str_to_info(const char *str, hid_t connector_id, void **info)
{
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_PACKAGE

    /* Only deserialize string, if it's non-NULL */
    if(str) {
        H5VL_class_t *cls;                  /* VOL connector's class struct */

        /* Check args and get class pointer */
        if(NULL == (cls = (H5VL_class_t *)H5I_object_verify(connector_id, H5I_VOL)))
            HGOTO_ERROR(H5E_VOL, H5E_BADTYPE, FAIL, "not a VOL connector ID")

        /* Allow the connector to deserialize info */
        if(cls->info_cls.from_str) {
            if((cls->info_cls.from_str)(str, info) < 0)
                HGOTO_ERROR(H5E_VOL, H5E_CANTUNSERIALIZE, FAIL, "can't deserialize connector info")
        } /* end if */
        else
            *info = NULL;
    } /* end if */
    else
        *info = NULL;

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5VL__connector_str_to_info() */


/*-------------------------------------------------------------------------
 * Function:    H5VL__get_connector_name
 *
 * Purpose:     Private version of H5VLget_connector_name
 *
 * Return:      Success:        The length of the connector name
 *              Failure:        Negative
 *
 *-------------------------------------------------------------------------
 */
ssize_t
H5VL__get_connector_name(hid_t id, char *name /*out*/, size_t size)
{
    H5VL_object_t       *vol_obj;
    const H5VL_class_t  *cls;
    size_t              len;
    ssize_t             ret_value = -1;

    FUNC_ENTER_PACKAGE

    /* get the object pointer */
    if (NULL == (vol_obj = H5VL_vol_object(id)))
        HGOTO_ERROR(H5E_VOL, H5E_BADTYPE, FAIL, "invalid VOL identifier")

    cls = vol_obj->connector->cls;

    len = HDstrlen(cls->name);
    if(name) {
        HDstrncpy(name, cls->name, MIN(len + 1, size));
        if(len >= size)
            name[size - 1] = '\0';
    } /* end if */

    /* Set the return value for the API call */
    ret_value = (ssize_t)len;

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5VL__get_connector_name() */


/*-------------------------------------------------------------------------
 * Function:    H5VL_vol_object
 *
 * Purpose:     Utility function to return the object pointer associated with
 *              a hid_t. This routine is the same as H5I_object for all types
 *              except for named datatypes, where the vol_obj is returned that 
 *              is attached to the H5T_t struct.
 *
 * Return:      Success:        object pointer
 *              Failure:        NULL
 *
 *-------------------------------------------------------------------------
 */
H5VL_object_t *
H5VL_vol_object(hid_t id)
{
    void            *obj = NULL;
    H5I_type_t      obj_type;
    H5VL_object_t   *ret_value = NULL;

    FUNC_ENTER_NOAPI(NULL)

    obj_type = H5I_get_type(id);
    if (H5I_FILE == obj_type || H5I_GROUP == obj_type || H5I_ATTR == obj_type || 
            H5I_DATASET == obj_type || H5I_DATATYPE == obj_type) {
        /* Get the object */
        if (NULL == (obj = H5I_object(id)))
            HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "invalid identifier")

        /* if this is a datatype, get the VOL object attached to the H5T_t struct */
        if (H5I_DATATYPE == obj_type)
            if (NULL == (obj = H5T_get_named_type((H5T_t *)obj)))
                HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "not a named datatype")
    } /* end if */
    else
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "invalid identifier type to function")

    ret_value = (H5VL_object_t *)obj;

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5VL_vol_object() */


/*-------------------------------------------------------------------------
 * Function:    H5VL_object_data
 *
 * Purpose:     Correctly retrieve the 'data' field for a VOL object (H5VL_object),
 *              even for nested / stacked VOL connectors.
 *
 * Return:      Success:        object pointer
 *              Failure:        NULL
 *
 *-------------------------------------------------------------------------
 */
void *
H5VL_object_data(const H5VL_object_t *vol_obj)
{
    void *ret_value = NULL;

    FUNC_ENTER_NOAPI_NOINIT_NOERR

    /* Check for 'get_object' callback in connector */
    if(vol_obj->connector->cls->wrap_cls.get_object)
        ret_value = (vol_obj->connector->cls->wrap_cls.get_object)(vol_obj->data);
    else
        ret_value = vol_obj->data;

    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5VL_object_data() */


/*-------------------------------------------------------------------------
 * Function:    H5VL_object_unwrap
 *
 * Purpose:     Correctly unwrap the 'data' field for a VOL object (H5VL_object),
 *              even for nested / stacked VOL connectors.
 *
 * Return:      Success:        Object pointer
 *              Failure:        NULL
 *
 *-------------------------------------------------------------------------
 */
void *
H5VL_object_unwrap(const H5VL_object_t *vol_obj)
{
    void *ret_value = NULL;

    FUNC_ENTER_NOAPI(NULL)

    if(NULL == (ret_value = H5VL_unwrap_object(vol_obj->connector->cls, vol_obj->data)))
        HGOTO_ERROR(H5E_VOL, H5E_CANTGET, NULL, "can't unwrap object")

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5VL_object_unwrap() */


/*-------------------------------------------------------------------------
 * Function:    H5VL__object
 *
 * Purpose:     Internal function to return the VOL object pointer associated
 *              with an hid_t.
 *
 * Return:      Success:    object pointer
 *              Failure:    NULL
 *
 *-------------------------------------------------------------------------
 */
static void *
H5VL__object(hid_t id, H5I_type_t obj_type)
{
    H5VL_object_t   *vol_obj = NULL;    
    void            *ret_value = NULL;

    FUNC_ENTER_STATIC

    /* Get the underlying object */
    switch(obj_type) {
        case H5I_GROUP:
        case H5I_DATASET:
        case H5I_FILE:            
        case H5I_ATTR:
        case H5I_MAP:
            /* get the object */
            if (NULL == (vol_obj = (H5VL_object_t *)H5I_object(id)))
                HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "invalid identifier")
            break;

        case H5I_DATATYPE:
            {
                H5T_t *dt = NULL;

                /* get the object */
                if (NULL == (dt = (H5T_t *)H5I_object(id)))
                    HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "invalid identifier")

                /* Get the actual datatype object that should be the vol_obj */
                if (NULL == (vol_obj = H5T_get_named_type(dt)))
                    HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "not a named datatype")
                break;
            }

        case H5I_UNINIT:
        case H5I_BADID:
        case H5I_DATASPACE:
        case H5I_VFL:
        case H5I_VOL:
        case H5I_GENPROP_CLS:
        case H5I_GENPROP_LST:
        case H5I_ERROR_CLASS:
        case H5I_ERROR_MSG:
        case H5I_ERROR_STACK:
        case H5I_SPACE_SEL_ITER:
        case H5I_NTYPES:
        default:
            HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "unknown data object type")
    } /* end switch */

    /* Set the return value */
    ret_value = H5VL_object_data(vol_obj);

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5VL__object() */


/*-------------------------------------------------------------------------
 * Function:    H5VL_object
 *
 * Purpose:     Utility function to return the VOL object pointer associated with
 *              a hid_t.
 *
 * Return:      Success:    object pointer
 *              Failure:    NULL
 *
 *-------------------------------------------------------------------------
 */
void *
H5VL_object(hid_t id)
{
    void            *ret_value = NULL;

    FUNC_ENTER_NOAPI(NULL)

    /* Get the underlying object */
    if(NULL == (ret_value = H5VL__object(id, H5I_get_type(id))))
        HGOTO_ERROR(H5E_VOL, H5E_CANTGET, NULL, "can't retrieve object for ID")

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5VL_object() */


/*-------------------------------------------------------------------------
 * Function:    H5VL_object_verify
 *
 * Purpose:     Utility function to return the VOL object pointer associated
 *              with an identifier.
 *
 * Return:      Success:    object pointer
 *              Failure:    NULL
 *
 *-------------------------------------------------------------------------
 */
void *
H5VL_object_verify(hid_t id, H5I_type_t obj_type)
{
    void            *ret_value = NULL;

    FUNC_ENTER_NOAPI(NULL)

    /* Check of ID of correct type */
    if(obj_type != H5I_get_type(id))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "invalid identifier")

    /* Get the underlying object */
    if(NULL == (ret_value = H5VL__object(id, obj_type)))
        HGOTO_ERROR(H5E_ARGS, H5E_CANTGET, NULL, "can't retrieve object for ID")

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5VL_object_verify() */


/*-------------------------------------------------------------------------
 * Function:    H5VL_cmp_connector_cls
 *
 * Purpose:     Compare VOL class for a connector
 *
 * Note:        Sets *cmp_value positive if VALUE1 is greater than VALUE2,
 *		negative if VALUE2 is greater than VALUE1, and zero if VALUE1
 *              and VALUE2 are equal (like strcmp).
 *
 * Return:      SUCCEED / FAIL
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5VL_cmp_connector_cls(int *cmp_value, const H5VL_class_t *cls1, const H5VL_class_t *cls2)
{
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_NOAPI(FAIL)

    /* Sanity checks */
    HDassert(cls1);
    HDassert(cls2);

    /* Compare connector "values" */
    if(cls1->value < cls2->value) {
        *cmp_value = -1;
        HGOTO_DONE(SUCCEED)
    } /* end if */
    if(cls1->value > cls2->value) {
        *cmp_value = 1;
        HGOTO_DONE(SUCCEED)
    } /* end if */
    HDassert(cls1->value == cls2->value);

    /* Compare connector names */
    if(cls1->name == NULL && cls2->name != NULL) {
        *cmp_value = -1;
        HGOTO_DONE(SUCCEED)
    } /* end if */
    if(cls1->name != NULL && cls2->name == NULL) {
        *cmp_value = 1;
        HGOTO_DONE(SUCCEED)
    } /* end if */
    if(0 != (*cmp_value = HDstrcmp(cls1->name, cls2->name)))
        HGOTO_DONE(SUCCEED)

    /* Compare connector VOL API versions */
    if(cls1->version < cls2->version) {
        *cmp_value = -1;
        HGOTO_DONE(SUCCEED)
    } /* end if */
    if(cls1->version > cls2->version) {
        *cmp_value = 1;
        HGOTO_DONE(SUCCEED)
    } /* end if */
    HDassert(cls1->version == cls2->version);

    /* Compare connector info */
    if(cls1->info_cls.size < cls2->info_cls.size) {
        *cmp_value = -1;
        HGOTO_DONE(SUCCEED)
    } /* end if */
    if(cls1->info_cls.size > cls2->info_cls.size) {
        *cmp_value = 1;
        HGOTO_DONE(SUCCEED)
    } /* end if */
    HDassert(cls1->info_cls.size == cls2->info_cls.size);

    /* Set comparison value to 'equal' */
    *cmp_value = 0;

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5VL_cmp_connector_cls() */


/*-------------------------------------------------------------------------
 * Function:    H5VL_retrieve_lib_state
 *
 * Purpose:     Retrieve the state of the library.
 *
 * Note:        Currently just retrieves the API context state, but could be
 *		expanded in the future.
 *
 * Return:      Success:    Non-negative, *state set
 *              Failure:    Negative, *state unset
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5VL_retrieve_lib_state(void **state)
{
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_NOAPI(FAIL)

    /* Sanity checks */
    HDassert(state);

    /* Retrieve the API context state */
    if(H5CX_retrieve_state((H5CX_state_t **)state) < 0)
        HGOTO_ERROR(H5E_VOL, H5E_CANTGET, FAIL, "can't get API context state")

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5VL_retrieve_lib_state() */


/*-------------------------------------------------------------------------
 * Function:    H5VL_restore_lib_state
 *
 * Purpose:     Restore the state of the library.
 *
 * Note:        Currently just restores the API context state, but could be
 *		expanded in the future.
 *
 * Return:      SUCCEED / FAIL
 *
 * Programmer:	Quincey Koziol
 *		Thursday, January 10, 2019
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5VL_restore_lib_state(const void *state)
{
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_NOAPI(FAIL)

    /* Sanity checks */
    HDassert(state);

    /* Push a new API context on the stack */
    if(H5CX_push() < 0)
        HGOTO_ERROR(H5E_VOL, H5E_CANTSET, FAIL, "can't push API context")

    /* Restore the API context state */
    if(H5CX_restore_state((const H5CX_state_t *)state) < 0)
        HGOTO_ERROR(H5E_VOL, H5E_CANTSET, FAIL, "can't set API context state")

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5VL_restore_lib_state() */


/*-------------------------------------------------------------------------
 * Function:    H5VL_reset_lib_state
 *
 * Purpose:     Reset the state of the library, undoing affects of
 *		H5VL_restore_lib_state.
 *
 * Note:        Currently just resets the API context state, but could be
 *		expanded in the future.
 *
 * Note:	This routine must be called as a "pair" with
 * 		H5VL_restore_lib_state.  It can be called before / after /
 * 		independently of H5VL_free_lib_state.
 *
 * Return:      SUCCEED / FAIL
 *
 * Programmer:	Quincey Koziol
 *		Saturday, February 23, 2019
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5VL_reset_lib_state(void)
{
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_NOAPI(FAIL)

    /* Pop the API context off the stack */
    if(H5CX_pop() < 0)
        HGOTO_ERROR(H5E_VOL, H5E_CANTRESET, FAIL, "can't pop API context")

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5VL_reset_lib_state() */


/*-------------------------------------------------------------------------
 * Function:    H5VL_free_lib_state
 *
 * Purpose:     Free a library state.
 *
 * Note:	This routine must be called as a "pair" with
 * 		H5VL_retrieve_lib_state.
 *
 * Return:      SUCCEED / FAIL
 *
 * Programmer:	Quincey Koziol
 *		Thursday, January 10, 2019
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5VL_free_lib_state(void *state)
{
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_NOAPI(FAIL)

    /* Sanity checks */
    HDassert(state);

    /* Free the API context state */
    if(H5CX_free_state((H5CX_state_t *)state) < 0)
        HGOTO_ERROR(H5E_VOL, H5E_CANTRELEASE, FAIL, "can't free API context state")

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5VL_free_lib_state() */


/*-------------------------------------------------------------------------
 * Function:    H5VL__free_vol_wrapper
 *
 * Purpose:     Free object wrapping context for VOL connector
 *
 * Return:      SUCCEED / FAIL
 *
 * Programmer:	Quincey Koziol
 *		Wednesday, January  9, 2019
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5VL__free_vol_wrapper(H5VL_wrap_ctx_t *vol_wrap_ctx)
{
    herr_t ret_value = SUCCEED;                 /* Return value */

    FUNC_ENTER_STATIC

    /* Sanity check */
    HDassert(vol_wrap_ctx);
    HDassert(0 == vol_wrap_ctx->rc);
    HDassert(vol_wrap_ctx->connector);
    HDassert(vol_wrap_ctx->connector->cls);

    /* If there is a VOL connector object wrapping context, release it */
    if(vol_wrap_ctx->obj_wrap_ctx)
        /* Release the VOL connector's object wrapping context */
        if((*vol_wrap_ctx->connector->cls->wrap_cls.free_wrap_ctx)(vol_wrap_ctx->obj_wrap_ctx) < 0)
            HGOTO_ERROR(H5E_VOL, H5E_CANTRELEASE, FAIL, "unable to release connector's object wrapping context")

    /* Decrement refcount on connector */
    if(H5VL__conn_dec_rc(vol_wrap_ctx->connector) < 0)
        HGOTO_ERROR(H5E_VOL, H5E_CANTDEC, FAIL, "unable to decrement ref count on VOL connector")

    /* Release object wrapping context */
    H5FL_FREE(H5VL_wrap_ctx_t, vol_wrap_ctx);

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5VL__free_vol_wrapper() */


/*-------------------------------------------------------------------------
 * Function:    H5VL_set_vol_wrapper
 *
 * Purpose:     Set up object wrapping context for current VOL connector
 *
 * Return:      SUCCEED / FAIL
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5VL_set_vol_wrapper(void *obj, H5VL_t *connector)
{
    H5VL_wrap_ctx_t *vol_wrap_ctx = NULL;       /* Object wrapping context */
    void *obj_wrap_ctx = NULL;          /* VOL connector's wrapping context */
    herr_t ret_value = SUCCEED;   /* Return value */

    FUNC_ENTER_NOAPI(FAIL)

    /* Sanity checks */
    HDassert(obj);
    HDassert(connector);

    /* Retrieve the VOL object wrap context */
    if(H5CX_get_vol_wrap_ctx((void **)&vol_wrap_ctx) < 0)
        HGOTO_ERROR(H5E_VOL, H5E_CANTGET, FAIL, "can't get VOL object wrap context")

    /* Check for existing wrapping context */
    if(NULL == vol_wrap_ctx) {
        /* Check if the connector can create a wrap context */
        if(connector->cls->wrap_cls.get_wrap_ctx) {
            /* Sanity check */
            HDassert(connector->cls->wrap_cls.free_wrap_ctx);

            /* Get the wrap context from the connector */
            if((connector->cls->wrap_cls.get_wrap_ctx)(obj, &obj_wrap_ctx) < 0)
                HGOTO_ERROR(H5E_VOL, H5E_CANTGET, FAIL, "can't retrieve VOL connector's object wrap context")
        } /* end if */

        /* Allocate VOL object wrapper context */
        if(NULL == (vol_wrap_ctx = H5FL_MALLOC(H5VL_wrap_ctx_t)))
            HGOTO_ERROR(H5E_VOL, H5E_CANTALLOC, FAIL, "can't allocate VOL wrap context")

        /* Increment the outstanding objects that are using the connector */
        H5VL__conn_inc_rc(connector);

        /* Set up VOL object wrapper context */
        vol_wrap_ctx->rc = 1;
        vol_wrap_ctx->connector = connector;
        vol_wrap_ctx->obj_wrap_ctx = obj_wrap_ctx;
    } /* end if */
    else
        /* Incremeent ref count on existing wrapper context */
        vol_wrap_ctx->rc++;

    /* Save the wrapper context */
    if(H5CX_set_vol_wrap_ctx(vol_wrap_ctx) < 0)
        HGOTO_ERROR(H5E_VOL, H5E_CANTSET, FAIL, "can't set VOL object wrap context")

done:
    if(ret_value < 0 && vol_wrap_ctx)
        /* Release object wrapping context */
        H5FL_FREE(H5VL_wrap_ctx_t, vol_wrap_ctx);

    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5VL_set_vol_wrapper() */


/*-------------------------------------------------------------------------
 * Function:    H5VL_inc_vol_wrapper
 *
 * Purpose:     Increment refcount on object wrapping context
 *
 * Return:      SUCCEED / FAIL
 *
 * Programmer:	Quincey Koziol
 *		Wednesday, January  9, 2019
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5VL_inc_vol_wrapper(void *_vol_wrap_ctx)
{
    H5VL_wrap_ctx_t *vol_wrap_ctx = (H5VL_wrap_ctx_t *)_vol_wrap_ctx; /* VOL object wrapping context */
    herr_t ret_value = SUCCEED;                 /* Return value */

    FUNC_ENTER_NOAPI(FAIL)

    /* Check for valid, active VOL object wrap context */
    if(NULL == vol_wrap_ctx)
        HGOTO_ERROR(H5E_VOL, H5E_BADVALUE, FAIL, "no VOL object wrap context?")
    if(0 == vol_wrap_ctx->rc)
        HGOTO_ERROR(H5E_VOL, H5E_BADVALUE, FAIL, "bad VOL object wrap context refcount?")

    /* Increment ref count on wrapping context */
    vol_wrap_ctx->rc++;

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5VL_inc_vol_wrapper() */


/*-------------------------------------------------------------------------
 * Function:    H5VL_dec_vol_wrapper
 *
 * Purpose:     Decrement refcount on object wrapping context, releasing it
 *		if the refcount drops to zero.
 *
 * Return:      SUCCEED / FAIL
 *
 * Programmer:	Quincey Koziol
 *		Wednesday, January  9, 2019
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5VL_dec_vol_wrapper(void *_vol_wrap_ctx)
{
    H5VL_wrap_ctx_t *vol_wrap_ctx = (H5VL_wrap_ctx_t *)_vol_wrap_ctx; /* VOL object wrapping context */
    herr_t ret_value = SUCCEED;                 /* Return value */

    FUNC_ENTER_NOAPI(FAIL)

    /* Check for valid, active VOL object wrap context */
    if(NULL == vol_wrap_ctx)
        HGOTO_ERROR(H5E_VOL, H5E_BADVALUE, FAIL, "no VOL object wrap context?")
    if(0 == vol_wrap_ctx->rc)
        HGOTO_ERROR(H5E_VOL, H5E_BADVALUE, FAIL, "bad VOL object wrap context refcount?")

    /* Decrement ref count on wrapping context */
    vol_wrap_ctx->rc--;

    /* Release context if the ref count drops to zero */
    if(0 == vol_wrap_ctx->rc)
        if(H5VL__free_vol_wrapper(vol_wrap_ctx) < 0)
            HGOTO_ERROR(H5E_VOL, H5E_CANTRELEASE, FAIL, "unable to release VOL object wrapping context")

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5VL_dec_vol_wrapper() */


/*-------------------------------------------------------------------------
 * Function:    H5VL_reset_vol_wrapper
 *
 * Purpose:     Reset object wrapping context for current VOL connector
 *
 * Return:      SUCCEED / FAIL
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5VL_reset_vol_wrapper(void)
{
    H5VL_wrap_ctx_t *vol_wrap_ctx = NULL;       /* Object wrapping context */
    herr_t ret_value = SUCCEED;                 /* Return value */

    FUNC_ENTER_NOAPI(FAIL)

    /* Retrieve the VOL object wrap context */
    if(H5CX_get_vol_wrap_ctx((void **)&vol_wrap_ctx) < 0)
        HGOTO_ERROR(H5E_VOL, H5E_CANTGET, FAIL, "can't get VOL object wrap context")

    /* Check for VOL object wrap context */
    if(NULL == vol_wrap_ctx)
        HGOTO_ERROR(H5E_VOL, H5E_BADVALUE, FAIL, "no VOL object wrap context?")

    /* Decrement ref count on wrapping context */
    vol_wrap_ctx->rc--;

    /* Release context if the ref count drops to zero */
    if(0 == vol_wrap_ctx->rc) {
        /* Release object wrapping context */
        if(H5VL__free_vol_wrapper(vol_wrap_ctx) < 0)
            HGOTO_ERROR(H5E_VOL, H5E_CANTRELEASE, FAIL, "unable to release VOL object wrapping context")

        /* Reset the wrapper context */
        if(H5CX_set_vol_wrap_ctx(NULL) < 0)
            HGOTO_ERROR(H5E_VOL, H5E_CANTSET, FAIL, "can't set VOL object wrap context")
    } /* end if */
    else
        /* Save the updated wrapper context */
        if(H5CX_set_vol_wrap_ctx(vol_wrap_ctx) < 0)
            HGOTO_ERROR(H5E_VOL, H5E_CANTSET, FAIL, "can't set VOL object wrap context")

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5VL_reset_vol_wrapper() */


/*-------------------------------------------------------------------------
 * Function:    H5VL_wrap_register
 *
 * Purpose:     Wrap an object and register an ID for it
 *
 * Return:      SUCCEED / FAIL
 *
 *-------------------------------------------------------------------------
 */
hid_t
H5VL_wrap_register(H5I_type_t type, void *obj, hbool_t app_ref)
{
    H5VL_wrap_ctx_t *vol_wrap_ctx = NULL;       /* Object wrapping context */
    void *new_obj;                      /* Newly wrapped object */
    hid_t ret_value = H5I_INVALID_HID;  /* Return value */

    FUNC_ENTER_NOAPI(H5I_INVALID_HID)

    /* Sanity check */
    HDassert(obj);

    /* If the datatype is already VOL-managed, the datatype's vol_obj
     * field will get clobbered later, so disallow this.
     */
    if(type == H5I_DATATYPE)
        if(TRUE == H5T_already_vol_managed((const H5T_t *)obj))
            HGOTO_ERROR(H5E_VOL, H5E_BADTYPE, H5I_INVALID_HID, "can't wrap an uncommitted datatype")

    /* Wrap the object with VOL connector info */
    if(NULL == (new_obj = H5VL__wrap_obj(obj, type)))
        HGOTO_ERROR(H5E_VOL, H5E_CANTCREATE, H5I_INVALID_HID, "can't wrap library object")

    /* Retrieve the VOL object wrapping context */
    if(H5CX_get_vol_wrap_ctx((void **)&vol_wrap_ctx) < 0)
        HGOTO_ERROR(H5E_VOL, H5E_CANTGET, H5I_INVALID_HID, "can't get VOL object wrap context")
    if(NULL == vol_wrap_ctx || NULL == vol_wrap_ctx->connector)
        HGOTO_ERROR(H5E_VOL, H5E_BADVALUE, H5I_INVALID_HID, "VOL object wrap context or its connector is NULL???")

    /* Get an ID for the object */
    if((ret_value = H5VL_register_using_vol_id(type, new_obj, vol_wrap_ctx->connector->id, app_ref)) < 0)
        HGOTO_ERROR(H5E_VOL, H5E_CANTREGISTER, H5I_INVALID_HID, "unable to get an ID for the object")

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5VL_wrap_register() */


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

#include "vol_group_test.h"

static int test_create_group_under_root(void);
static int test_create_group_under_existing_group(void);
static int test_create_group_invalid_params(void);
static int test_create_anonymous_group(void);
static int test_create_anonymous_group_invalid_params(void);
static int test_open_nonexistent_group(void);
static int test_open_group_invalid_params(void);
static int test_close_group_invalid_id(void);
static int test_group_property_lists(void);
static int test_get_group_info(void);
static int test_get_group_info_invalid_params(void);
static int test_flush_group(void);
static int test_flush_group_invalid_params(void);
static int test_refresh_group(void);
static int test_refresh_group_invalid_params(void);

/*
 * The array of group tests to be performed.
 */
static int (*group_tests[])(void) = {
        test_create_group_under_root,
        test_create_group_under_existing_group,
        test_create_group_invalid_params,
        test_create_anonymous_group,
        test_create_anonymous_group_invalid_params,
        test_open_nonexistent_group,
        test_open_group_invalid_params,
        test_close_group_invalid_id,
        test_group_property_lists,
        test_get_group_info,
        test_get_group_info_invalid_params,
        test_flush_group,
        test_flush_group_invalid_params,
        test_refresh_group,
        test_refresh_group_invalid_params,
};

/*
 * A test to check that a group can be created under the root group.
 */
static int
test_create_group_under_root(void)
{
    hid_t file_id = H5I_INVALID_HID, fapl_id = H5I_INVALID_HID;
    hid_t group_id = H5I_INVALID_HID;

    TESTING("creation of group under the root group")

    if ((fapl_id = h5_fileaccess()) < 0)
        TEST_ERROR

    if ((file_id = H5Fopen(vol_test_filename, H5F_ACC_RDWR, fapl_id)) < 0) {
        H5_FAILED();
        printf("    couldn't open file\n");
        goto error;
    }

    /* Create the group under the root group of the file */
    if ((group_id = H5Gcreate2(file_id, GROUP_CREATE_UNDER_ROOT_GNAME, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        printf("    couldn't create group\n");
        goto error;
    }

    /*
     * XXX: Using both relative and absolute pathnames.
     */

    if (H5Gclose(group_id) < 0)
        TEST_ERROR
    if (H5Pclose(fapl_id) < 0)
        TEST_ERROR
    if (H5Fclose(file_id) < 0)
        TEST_ERROR

    PASSED();

    return 0;

error:
    H5E_BEGIN_TRY {
        H5Gclose(group_id);
        H5Pclose(fapl_id);
        H5Fclose(file_id);
    } H5E_END_TRY;

    return 1;
}

/*
 * A test to check that a group can be created under an existing
 * group which is not the root group.
 */
static int
test_create_group_under_existing_group(void)
{
    hid_t file_id = H5I_INVALID_HID;
    hid_t parent_group_id = H5I_INVALID_HID, new_group_id = H5I_INVALID_HID;
    hid_t fapl_id = H5I_INVALID_HID;

    TESTING("creation of group under existing group using a relative path")

    if ((fapl_id = h5_fileaccess()) < 0)
        TEST_ERROR

    if ((file_id = H5Fopen(vol_test_filename, H5F_ACC_RDWR, fapl_id)) < 0) {
        H5_FAILED();
        printf("    couldn't open file\n");
        goto error;
    }

    /* Open the already-existing parent group in the file */
    if ((parent_group_id = H5Gopen2(file_id, GROUP_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        printf("    couldn't open group\n");
        goto error;
    }

    /* Create a new Group under the already-existing parent Group using a relative path */
    if ((new_group_id = H5Gcreate2(parent_group_id, GROUP_CREATE_UNDER_GROUP_REL_GNAME, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        printf("    couldn't create group using relative path\n");
        goto error;
    }

    /*
     * XXX: Use both relative and absolute pathnames.
     */

    if (H5Gclose(parent_group_id) < 0)
        TEST_ERROR
    if (H5Gclose(new_group_id) < 0)
        TEST_ERROR
    if (H5Pclose(fapl_id) < 0)
        TEST_ERROR
    if (H5Fclose(file_id) < 0)
        TEST_ERROR

    PASSED();

    return 0;

error:
    H5E_BEGIN_TRY {
        H5Gclose(new_group_id);
        H5Gclose(parent_group_id);
        H5Pclose(fapl_id);
        H5Fclose(file_id);
    } H5E_END_TRY;

    return 1;
}

/*
 * A test to check that a group can't be created when H5Gcreate
 * is passed invalid parameters.
 */
static int
test_create_group_invalid_params(void)
{
    hid_t file_id = H5I_INVALID_HID, fapl_id = H5I_INVALID_HID;
    hid_t group_id = H5I_INVALID_HID;

    TESTING("H5Gcreate with invalid parameters"); puts("");

    TESTING_2("H5Gcreate with an invalid loc_id")

    if ((fapl_id = h5_fileaccess()) < 0)
        TEST_ERROR

    if ((file_id = H5Fopen(vol_test_filename, H5F_ACC_RDWR, fapl_id)) < 0) {
        H5_FAILED();
        printf("    couldn't open file '%s'\n", vol_test_filename);
        goto error;
    }

    H5E_BEGIN_TRY {
        group_id = H5Gcreate2(H5I_INVALID_HID, GROUP_CREATE_INVALID_PARAMS_GROUP_NAME, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    } H5E_END_TRY;

    if (group_id >= 0) {
        H5_FAILED();
        printf("    created group with invalid loc_id!\n");
        goto error;
    }

    PASSED();

    TESTING_2("H5Gcreate with an invalid group name")

    H5E_BEGIN_TRY {
        group_id = H5Gcreate2(file_id, NULL, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    } H5E_END_TRY;

    if (group_id >= 0) {
        H5_FAILED();
        printf("    created group with invalid name!\n");
        goto error;
    }

    H5E_BEGIN_TRY {
        group_id = H5Gcreate2(file_id, "", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    } H5E_END_TRY;

    if (group_id >= 0) {
        H5_FAILED();
        printf("    created group with invalid name!\n");
        goto error;
    }

    PASSED();

    TESTING_2("H5Gcreate with an invalid LCPL")

    H5E_BEGIN_TRY {
        group_id = H5Gcreate2(file_id, GROUP_CREATE_INVALID_PARAMS_GROUP_NAME, H5I_INVALID_HID, H5P_DEFAULT, H5P_DEFAULT);
    } H5E_END_TRY;

    if (group_id >= 0) {
        H5_FAILED();
        printf("    created group with invalid LCPL!\n");
        goto error;
    }

    PASSED();

    TESTING_2("H5Gcreate with an invalid GCPL")

    H5E_BEGIN_TRY {
        group_id = H5Gcreate2(file_id, GROUP_CREATE_INVALID_PARAMS_GROUP_NAME, H5P_DEFAULT, H5I_INVALID_HID, H5P_DEFAULT);
    } H5E_END_TRY;

    if (group_id >= 0) {
        H5_FAILED();
        printf("    created group with invalid GCPL!\n");
        goto error;
    }

    PASSED();

    TESTING_2("H5Gcreate with an invalid GAPL")

    H5E_BEGIN_TRY {
        group_id = H5Gcreate2(file_id, GROUP_CREATE_INVALID_PARAMS_GROUP_NAME, H5P_DEFAULT, H5P_DEFAULT, H5I_INVALID_HID);
    } H5E_END_TRY;

    if (group_id >= 0) {
        H5_FAILED();
        printf("    created group with invalid GAPL!\n");
        goto error;
    }

    if (H5Pclose(fapl_id) < 0)
        TEST_ERROR
    if (H5Fclose(file_id) < 0)
        TEST_ERROR

    PASSED();

    return 0;

error:
    H5E_BEGIN_TRY {
        H5Gclose(group_id);
        H5Pclose(fapl_id);
        H5Fclose(file_id);
    } H5E_END_TRY;

    return 1;
}

/*
 * A test to check that an anonymous group can be created with
 * H5Gcreate_anon.
 */
static int
test_create_anonymous_group(void)
{
    hid_t file_id = H5I_INVALID_HID;
    hid_t container_group = H5I_INVALID_HID, new_group_id = H5I_INVALID_HID;
    hid_t fapl_id = H5I_INVALID_HID;

    TESTING("creation of anonymous group")

    if ((fapl_id = h5_fileaccess()) < 0)
        TEST_ERROR

    if ((file_id = H5Fopen(vol_test_filename, H5F_ACC_RDWR, fapl_id)) < 0) {
        H5_FAILED();
        printf("    couldn't open file\n");
        goto error;
    }

    if ((container_group = H5Gopen2(file_id, GROUP_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        printf("    couldn't open group\n");
        goto error;
    }

    if ((new_group_id = H5Gcreate_anon(file_id, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        printf("    couldn't create anonymous group\n");
        goto error;
    }

    if (H5Gclose(new_group_id) < 0)
        TEST_ERROR
    if (H5Gclose(container_group) < 0)
        TEST_ERROR
    if (H5Pclose(fapl_id) < 0)
        TEST_ERROR
    if (H5Fclose(file_id) < 0)
        TEST_ERROR

    PASSED();

    return 0;

error:
    H5E_BEGIN_TRY {
        H5Gclose(new_group_id);
        H5Gclose(container_group);
        H5Pclose(fapl_id);
        H5Fclose(file_id);
    } H5E_END_TRY;

    return 1;
}

/*
 * A test to check that an anonymous group can't be created
 * when H5Gcreate_anon is passed invalid parameters.
 */
static int
test_create_anonymous_group_invalid_params(void)
{
    hid_t file_id = H5I_INVALID_HID;
    hid_t container_group = H5I_INVALID_HID, new_group_id = H5I_INVALID_HID;
    hid_t fapl_id = H5I_INVALID_HID;

    TESTING("H5Gcreate_anon with invalid parameters"); puts("");

    TESTING_2("H5Gcreate_anon with an invalid loc_id")

    if ((fapl_id = h5_fileaccess()) < 0)
        TEST_ERROR

    if ((file_id = H5Fopen(vol_test_filename, H5F_ACC_RDWR, fapl_id)) < 0) {
        H5_FAILED();
        printf("    couldn't open file\n");
        goto error;
    }

    if ((container_group = H5Gopen2(file_id, GROUP_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        printf("    couldn't open group\n");
        goto error;
    }

    H5E_BEGIN_TRY {
        new_group_id = H5Gcreate_anon(H5I_INVALID_HID, H5P_DEFAULT, H5P_DEFAULT);
    } H5E_END_TRY;

    if (new_group_id >= 0) {
        H5_FAILED();
        printf("    created anonymous group with invalid loc_id!\n");
        goto error;
    }

    PASSED();

    TESTING_2("H5Gcreate_anon with an invalid GCPL")

    H5E_BEGIN_TRY {
        new_group_id = H5Gcreate_anon(container_group, H5I_INVALID_HID, H5P_DEFAULT);
    } H5E_END_TRY;

    if (new_group_id >= 0) {
        H5_FAILED();
        printf("    created anonymous group with invalid GCPL!\n");
        goto error;
    }

    PASSED();

    TESTING_2("H5Gcreate_anon with an invalid GAPL")

    H5E_BEGIN_TRY {
        new_group_id = H5Gcreate_anon(container_group, H5P_DEFAULT, H5I_INVALID_HID);
    } H5E_END_TRY;

    if (new_group_id >= 0) {
        H5_FAILED();
        printf("    created anonymous group with invalid GAPL!\n");
        goto error;
    }

    if (H5Gclose(container_group) < 0)
        TEST_ERROR
    if (H5Pclose(fapl_id) < 0)
        TEST_ERROR
    if (H5Fclose(file_id) < 0)
        TEST_ERROR

    PASSED();

    return 0;

error:
    H5E_BEGIN_TRY {
        H5Gclose(new_group_id);
        H5Gclose(container_group);
        H5Pclose(fapl_id);
        H5Fclose(file_id);
    } H5E_END_TRY;

    return 1;
}

/*
 * A test to check that a group which doesn't exist cannot
 * be opened.
 */
static int
test_open_nonexistent_group(void)
{
    hid_t file_id = H5I_INVALID_HID, fapl_id = H5I_INVALID_HID;
    hid_t group_id = H5I_INVALID_HID;

    TESTING("for failure when opening a nonexistent group")

    if ((fapl_id = h5_fileaccess()) < 0)
        TEST_ERROR

    if ((file_id = H5Fopen(vol_test_filename, H5F_ACC_RDWR, fapl_id)) < 0) {
        H5_FAILED();
        printf("    couldn't open file\n");
        goto error;
    }

    H5E_BEGIN_TRY {
        group_id = H5Gopen2(file_id, OPEN_NONEXISTENT_GROUP_TEST_GNAME, H5P_DEFAULT);
    } H5E_END_TRY;

    if (group_id >= 0) {
        H5_FAILED();
        printf("    opened non-existent group!\n");
        goto error;
    }

    if (H5Pclose(fapl_id) < 0)
        TEST_ERROR
    if (H5Fclose(file_id) < 0)
        TEST_ERROR

    PASSED();

    return 0;

error:
    H5E_BEGIN_TRY {
        H5Gclose(group_id);
        H5Pclose(fapl_id);
        H5Fclose(file_id);
    } H5E_END_TRY;

    return 1;
}

/*
 * A test to check that a group can't be opened when H5Gopen
 * is passed invalid parameters.
 */
static int
test_open_group_invalid_params(void)
{
    hid_t file_id = H5I_INVALID_HID, fapl_id = H5I_INVALID_HID;
    hid_t group_id = H5I_INVALID_HID;

    TESTING("H5Gopen with invalid parameters"); puts("");

    TESTING_2("H5Gopen with an invalid loc_id")

    if ((fapl_id = h5_fileaccess()) < 0)
        TEST_ERROR

    if ((file_id = H5Fopen(vol_test_filename, H5F_ACC_RDWR, fapl_id)) < 0) {
        H5_FAILED();
        printf("    couldn't open file '%s'\n", vol_test_filename);
        goto error;
    }

    H5E_BEGIN_TRY {
        group_id = H5Gopen2(H5I_INVALID_HID, GROUP_TEST_GROUP_NAME, H5P_DEFAULT);
    } H5E_END_TRY;

    if (group_id >= 0) {
        H5_FAILED();
        printf("    opened group using an invalid loc_id!\n");
        goto error;
    }

    PASSED();

    TESTING_2("H5Gopen with an invalid group name")

    H5E_BEGIN_TRY {
        group_id = H5Gopen2(file_id, NULL, H5P_DEFAULT);
    } H5E_END_TRY;

    if (group_id >= 0) {
        H5_FAILED();
        printf("    opened group using an invalid name!\n");
        goto error;
    }

    H5E_BEGIN_TRY {
        group_id = H5Gopen2(file_id, "", H5P_DEFAULT);
    } H5E_END_TRY;

    if (group_id >= 0) {
        H5_FAILED();
        printf("    opened group using an invalid name!\n");
        goto error;
    }

    PASSED();

    TESTING_2("H5Gopen with an invalid GAPL")

    H5E_BEGIN_TRY {
        group_id = H5Gopen2(file_id, GROUP_TEST_GROUP_NAME, H5I_INVALID_HID);
    } H5E_END_TRY;

    if (group_id >= 0) {
        H5_FAILED();
        printf("    opened group using an invalid GAPL!\n");
        goto error;
    }

    if (H5Pclose(fapl_id) < 0)
        TEST_ERROR
    if (H5Fclose(file_id) < 0)
        TEST_ERROR

    PASSED();

    return 0;

error:
    H5E_BEGIN_TRY {
        H5Gclose(group_id);
        H5Pclose(fapl_id);
        H5Fclose(file_id);
    } H5E_END_TRY;

    return 1;
}

/*
 * A test to check that H5Gclose doesn't succeed for an
 * invalid group ID.
 */
static int
test_close_group_invalid_id(void)
{
    herr_t err_ret = -1;
    hid_t  fapl_id = H5I_INVALID_HID;

    TESTING("H5Gclose with an invalid group ID")

    if ((fapl_id = h5_fileaccess()) < 0)
        TEST_ERROR

    H5E_BEGIN_TRY {
        err_ret = H5Gclose(H5I_INVALID_HID);
    } H5E_END_TRY;

    if (err_ret >= 0) {
        H5_FAILED();
        printf("    close a group with an invalid ID!\n");
        goto error;
    }

    if (H5Pclose(fapl_id) < 0)
        TEST_ERROR

    PASSED();

    return 0;

error:
    H5E_BEGIN_TRY {
        H5Pclose(fapl_id);
    } H5E_END_TRY;

    return 1;
}

/*
 * A test to check that a VOL connector stores and can retrieve a valid
 * copy of a GCPL used at group creation time.
 */
static int
test_group_property_lists(void)
{
    size_t dummy_prop_val = GROUP_PROPERTY_LIST_TEST_DUMMY_VAL;
    hid_t  file_id = H5I_INVALID_HID, fapl_id = H5I_INVALID_HID;
    hid_t  container_group = H5I_INVALID_HID;
    hid_t  group_id1 = H5I_INVALID_HID, group_id2 = H5I_INVALID_HID;
    hid_t  gcpl_id1 = H5I_INVALID_HID, gcpl_id2 = H5I_INVALID_HID;

    TESTING("group property list operations")

    if ((fapl_id = h5_fileaccess()) < 0)
        TEST_ERROR

    if ((file_id = H5Fopen(vol_test_filename, H5F_ACC_RDWR, fapl_id)) < 0) {
        H5_FAILED();
        printf("    couldn't open file\n");
        goto error;
    }

    if ((container_group = H5Gopen2(file_id, GROUP_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        printf("    couldn't open container group\n");
        goto error;
    }

    if ((gcpl_id1 = H5Pcreate(H5P_GROUP_CREATE)) < 0) {
        H5_FAILED();
        printf("    couldn't create GCPL\n");
        goto error;
    }

    if (H5Pset_local_heap_size_hint(gcpl_id1, dummy_prop_val) < 0) {
        H5_FAILED();
        printf("    couldn't set property on GCPL\n");
        goto error;
    }

    /* Create the group in the file */
    if ((group_id1 = H5Gcreate2(container_group, GROUP_PROPERTY_LIST_TEST_GROUP_NAME1, H5P_DEFAULT, gcpl_id1, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        printf("    couldn't create group\n");
        goto error;
    }

    /* Create the second group using H5P_DEFAULT for the GCPL */
    if ((group_id2 = H5Gcreate2(container_group, GROUP_PROPERTY_LIST_TEST_GROUP_NAME2, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        printf("    couldn't create group\n");
        goto error;
    }

    if (H5Pclose(gcpl_id1) < 0)
        TEST_ERROR

    /* Try to retrieve copies of the two property lists, one which has the property set and one which does not */
    if ((gcpl_id1 = H5Gget_create_plist(group_id1)) < 0) {
        H5_FAILED();
        printf("    couldn't get GCPL\n");
        goto error;
    }

    if ((gcpl_id2 = H5Gget_create_plist(group_id2)) < 0) {
        H5_FAILED();
        printf("    couldn't get GCPL\n");
        goto error;
    }

    /* Ensure that property list 1 has the property set and property list 2 does not */
    dummy_prop_val = 0;

    if (H5Pget_local_heap_size_hint(gcpl_id1, &dummy_prop_val) < 0) {
        H5_FAILED();
        printf("    couldn't retrieve GCPL property value\n");
        goto error;
    }

    if (dummy_prop_val != GROUP_PROPERTY_LIST_TEST_DUMMY_VAL) {
        H5_FAILED();
        printf("    retrieved GCPL property value '%llu' did not match expected value '%llu'\n",
                (unsigned long long) dummy_prop_val, (unsigned long long) GROUP_PROPERTY_LIST_TEST_DUMMY_VAL);
        goto error;
    }

    dummy_prop_val = 0;

    if (H5Pget_local_heap_size_hint(gcpl_id2, &dummy_prop_val) < 0) {
        H5_FAILED();
        printf("    couldn't retrieve GCPL property value\n");
        goto error;
    }

    if (dummy_prop_val == GROUP_PROPERTY_LIST_TEST_DUMMY_VAL) {
        H5_FAILED();
        printf("    retrieved GCPL property value '%llu' matched control value '%llu' when it shouldn't have\n",
                (unsigned long long) dummy_prop_val, (unsigned long long) GROUP_PROPERTY_LIST_TEST_DUMMY_VAL);
        goto error;
    }

    if (H5Pclose(gcpl_id1) < 0)
        TEST_ERROR
    if (H5Pclose(gcpl_id2) < 0)
        TEST_ERROR

    /* Now see if we can still retrieve copies of the property lists upon opening
     * (instead of creating) a group. If they were reconstructed properly upon file
     * open, the creation property lists should also have the same test values
     * as set before.
     */
    if (H5Gclose(group_id1) < 0)
        TEST_ERROR
    if (H5Gclose(group_id2) < 0)
        TEST_ERROR

    if ((group_id1 = H5Gopen2(container_group, GROUP_PROPERTY_LIST_TEST_GROUP_NAME1, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        printf("    couldn't open group\n");
        goto error;
    }

    if ((group_id2 = H5Gopen2(container_group, GROUP_PROPERTY_LIST_TEST_GROUP_NAME2, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        printf("    couldn't open group\n");
        goto error;
    }

    if ((gcpl_id1 = H5Gget_create_plist(group_id1)) < 0) {
        H5_FAILED();
        printf("    couldn't get property list\n");
        goto error;
    }

    if ((gcpl_id2 = H5Gget_create_plist(group_id2)) < 0) {
        H5_FAILED();
        printf("    couldn't get property list\n");
        goto error;
    }

    /* Re-check the property values */
    dummy_prop_val = 0;

    if (H5Pget_local_heap_size_hint(gcpl_id1, &dummy_prop_val) < 0) {
        H5_FAILED();
        printf("    couldn't retrieve GCPL property value\n");
        goto error;
    }

    if (dummy_prop_val != GROUP_PROPERTY_LIST_TEST_DUMMY_VAL) {
        H5_FAILED();
        printf("    retrieved GCPL property value '%llu' did not match expected value '%llu'\n",
                (unsigned long long) dummy_prop_val, (unsigned long long) GROUP_PROPERTY_LIST_TEST_DUMMY_VAL);
        goto error;
    }

    dummy_prop_val = 0;

    if (H5Pget_local_heap_size_hint(gcpl_id2, &dummy_prop_val) < 0) {
        H5_FAILED();
        printf("    couldn't retrieve GCPL property value\n");
        goto error;
    }

    if (dummy_prop_val == GROUP_PROPERTY_LIST_TEST_DUMMY_VAL) {
        H5_FAILED();
        printf("    retrieved GCPL property value '%llu' matched control value '%llu' when it shouldn't have\n",
                (unsigned long long) dummy_prop_val, (unsigned long long) GROUP_PROPERTY_LIST_TEST_DUMMY_VAL);
        goto error;
    }

    if (H5Pclose(gcpl_id1) < 0)
        TEST_ERROR
    if (H5Pclose(gcpl_id2) < 0)
        TEST_ERROR
    if (H5Gclose(group_id1) < 0)
        TEST_ERROR
    if (H5Gclose(group_id2) < 0)
        TEST_ERROR
    if (H5Gclose(container_group) < 0)
        TEST_ERROR
    if (H5Pclose(fapl_id) < 0)
        TEST_ERROR
    if (H5Fclose(file_id) < 0)
        TEST_ERROR

    PASSED();

    return 0;

error:
    H5E_BEGIN_TRY {
        H5Pclose(gcpl_id1);
        H5Pclose(gcpl_id2);
        H5Gclose(group_id1);
        H5Gclose(group_id2);
        H5Gclose(container_group);
        H5Pclose(fapl_id);
        H5Fclose(file_id);
    } H5E_END_TRY;

    return 1;
}

/*
 * A test for the functionality of H5Gget_info.
 */
static int
test_get_group_info(void)
{
    H5G_info_t group_info;
    hid_t      file_id = H5I_INVALID_HID, fapl_id = H5I_INVALID_HID;

    TESTING("retrieval of group info"); puts("");

    TESTING_2("retrieval of group info with H5Gget_info")

    if ((fapl_id = h5_fileaccess()) < 0)
        TEST_ERROR

    if ((file_id = H5Fopen(vol_test_filename, H5F_ACC_RDWR, fapl_id)) < 0) {
        H5_FAILED();
        printf("    couldn't open file '%s'\n", vol_test_filename);
        goto error;
    }

    if (H5Gget_info(file_id, &group_info) < 0) {
        H5_FAILED();
        printf("    couldn't get group info\n");
        goto error;
    }

    /*
     * XXX: Can't really check any info except perhaps the number of links.
     */

    PASSED();

    TESTING_2("retrieval of group info with H5Gget_info_by_name")

    if (H5Gget_info_by_name(file_id, "/", &group_info, H5P_DEFAULT) < 0) {
        H5_FAILED();
        printf("    couldn't get group info by name\n");
        goto error;
    }

    PASSED();

    TESTING_2("retrieval of group info with H5Gget_info_by_idx")

    if (H5Gget_info_by_idx(file_id, "/", H5_INDEX_NAME, H5_ITER_INC, 0, &group_info, H5P_DEFAULT) < 0) {
        H5_FAILED();
        printf("    couldn't get group info by index\n");
        goto error;
    }

    if (H5Pclose(fapl_id) < 0)
        TEST_ERROR
    if (H5Fclose(file_id) < 0)
        TEST_ERROR

    PASSED();

    return 0;

error:
    H5E_BEGIN_TRY {
        H5Pclose(fapl_id);
        H5Fclose(file_id);
    } H5E_END_TRY;

    return 1;
}

/*
 * A test to check that a group's info can't be retrieved when
 * H5Gget_info(_by_name/_by_idx) is passed invalid parameters.
 */
static int
test_get_group_info_invalid_params(void)
{
    H5G_info_t group_info;
    herr_t     err_ret = -1;
    hid_t      file_id = H5I_INVALID_HID, fapl_id = H5I_INVALID_HID;

    TESTING("H5Gget_info with invalid parameters"); puts("");

    TESTING_2("H5Gget_info with an invalid loc_id")

    if ((fapl_id = h5_fileaccess()) < 0)
        TEST_ERROR

    if ((file_id = H5Fopen(vol_test_filename, H5F_ACC_RDWR, fapl_id)) < 0) {
        H5_FAILED();
        printf("    couldn't open file '%s'\n", vol_test_filename);
        goto error;
    }

    H5E_BEGIN_TRY {
        err_ret = H5Gget_info(H5I_INVALID_HID, &group_info);
    } H5E_END_TRY;

    if (err_ret >= 0) {
        H5_FAILED();
        printf("    retrieved info of group using H5Gget_info with an invalid loc_id!\n");
        goto error;
    }

    PASSED();

    TESTING_2("H5Gget_info with an invalid group info pointer")

    H5E_BEGIN_TRY {
        err_ret = H5Gget_info(file_id, NULL);
    } H5E_END_TRY;

    if (err_ret >= 0) {
        H5_FAILED();
        printf("    retrieved info of group using H5Gget_info with invalid group info pointer!\n");
        goto error;
    }

    PASSED();

    TESTING("H5Gget_info_by_name with invalid parameters"); puts("");

    TESTING_2("H5Gget_info_by_name with an invalid loc_id")

    H5E_BEGIN_TRY {
        err_ret = H5Gget_info_by_name(H5I_INVALID_HID, ".", &group_info, H5P_DEFAULT);
    } H5E_END_TRY;

    if (err_ret >= 0) {
        H5_FAILED();
        printf("    retrieved info of group using H5Gget_info_by_name with an invalid loc_id!\n");
        goto error;
    }

    PASSED();

    TESTING_2("H5Gget_info_by_name with an invalid group name")

    H5E_BEGIN_TRY {
        err_ret = H5Gget_info_by_name(file_id, NULL, &group_info, H5P_DEFAULT);
    } H5E_END_TRY;

    if (err_ret >= 0) {
        H5_FAILED();
        printf("    retrieved info of group using H5Gget_info_by_name with an invalid name!\n");
        goto error;
    }

    H5E_BEGIN_TRY {
        err_ret = H5Gget_info_by_name(file_id, "", &group_info, H5P_DEFAULT);
    } H5E_END_TRY;

    if (err_ret >= 0) {
        H5_FAILED();
        printf("    retrieved info of group using H5Gget_info_by_name with an invalid name!\n");
        goto error;
    }

    PASSED();

    TESTING_2("H5Gget_info_by_name with an invalid group info pointer")

    H5E_BEGIN_TRY {
        err_ret = H5Gget_info_by_name(file_id, ".", NULL, H5P_DEFAULT);
    } H5E_END_TRY;

    if (err_ret >= 0) {
        H5_FAILED();
        printf("    retrieved info of group using H5Gget_info_by_name with an invalid group info pointer!\n");
        goto error;
    }

    PASSED();

    TESTING_2("H5Gget_info_by_name with an invalid LAPL")

    H5E_BEGIN_TRY {
        err_ret = H5Gget_info_by_name(file_id, ".", &group_info, H5I_INVALID_HID);
    } H5E_END_TRY;

    if (err_ret >= 0) {
        H5_FAILED();
        printf("    retrieved info of group using H5Gget_info_by_name with an invalid LAPL!\n");
        goto error;
    }

    PASSED();

    TESTING("H5Gget_info_by_idx with invalid parameters"); puts("");

    TESTING_2("H5Gget_info_by_idx with an invalid loc_id")

    H5E_BEGIN_TRY {
        err_ret = H5Gget_info_by_idx(H5I_INVALID_HID, ".", H5_INDEX_NAME, H5_ITER_INC, 0, &group_info, H5P_DEFAULT);
    } H5E_END_TRY;

    if (err_ret >= 0) {
        H5_FAILED();
        printf("    retrieved info of group using H5Gget_info_by_idx with an invalid loc_id!\n");
        goto error;
    }

    PASSED();

    TESTING_2("H5Gget_info_by_idx with an invalid group name")

    H5E_BEGIN_TRY {
        err_ret = H5Gget_info_by_idx(file_id, NULL, H5_INDEX_NAME, H5_ITER_INC, 0, &group_info, H5P_DEFAULT);
    } H5E_END_TRY;

    if (err_ret >= 0) {
        H5_FAILED();
        printf("    retrieved info of group using H5Gget_info_by_idx with an invalid group name!\n");
        goto error;
    }

    H5E_BEGIN_TRY {
        err_ret = H5Gget_info_by_idx(file_id, "", H5_INDEX_NAME, H5_ITER_INC, 0, &group_info, H5P_DEFAULT);
    } H5E_END_TRY;

    if (err_ret >= 0) {
        H5_FAILED();
        printf("    retrieved info of group using H5Gget_info_by_idx with an invalid group name!\n");
        goto error;
    }

    PASSED();

    TESTING_2("H5Gget_info_by_idx with an invalid index type")

    H5E_BEGIN_TRY {
        err_ret = H5Gget_info_by_idx(file_id, ".", H5_INDEX_UNKNOWN, H5_ITER_INC, 0, &group_info, H5P_DEFAULT);
    } H5E_END_TRY;

    if (err_ret >= 0) {
        H5_FAILED();
        printf("    retrieved info of group using H5Gget_info_by_idx with an invalid index type!\n");
        goto error;
    }

    H5E_BEGIN_TRY {
        err_ret = H5Gget_info_by_idx(file_id, ".", H5_INDEX_N, H5_ITER_INC, 0, &group_info, H5P_DEFAULT);
    } H5E_END_TRY;

    if (err_ret >= 0) {
        H5_FAILED();
        printf("    retrieved info of group using H5Gget_info_by_idx with an invalid index type!\n");
        goto error;
    }

    PASSED();

    TESTING_2("H5Gget_info_by_idx with an invalid iteration order")

    H5E_BEGIN_TRY {
        err_ret = H5Gget_info_by_idx(file_id, ".", H5_INDEX_NAME, H5_ITER_UNKNOWN, 0, &group_info, H5P_DEFAULT);
    } H5E_END_TRY;

    if (err_ret >= 0) {
        H5_FAILED();
        printf("    retrieved info of group using H5Gget_info_by_idx with an invalid iteration order!\n");
        goto error;
    }

    H5E_BEGIN_TRY {
        err_ret = H5Gget_info_by_idx(file_id, ".", H5_INDEX_NAME, H5_ITER_N, 0, &group_info, H5P_DEFAULT);
    } H5E_END_TRY;

    if (err_ret >= 0) {
        H5_FAILED();
        printf("    retrieved info of group using H5Gget_info_by_idx with an invalid iteration order!\n");
        goto error;
    }

    PASSED();

    TESTING_2("H5Gget_info_by_idx with an invalid group info pointer")

    H5E_BEGIN_TRY {
        err_ret = H5Gget_info_by_idx(file_id, ".", H5_INDEX_NAME, H5_ITER_INC, 0, NULL, H5P_DEFAULT);
    } H5E_END_TRY;

    if (err_ret >= 0) {
        H5_FAILED();
        printf("    retrieved info of group using H5Gget_info_by_idx with an invalid group info pointer!\n");
        goto error;
    }

    PASSED();

    TESTING_2("H5Gget_info_by_idx with an invalid LAPL")

    H5E_BEGIN_TRY {
        err_ret = H5Gget_info_by_idx(file_id, ".", H5_INDEX_NAME, H5_ITER_INC, 0, &group_info, H5I_INVALID_HID);
    } H5E_END_TRY;

    if (err_ret >= 0) {
        H5_FAILED();
        printf("    retrieved info of group using H5Gget_info_by_idx with an invalid LAPL!\n");
        goto error;
    }

    if (H5Pclose(fapl_id) < 0)
        TEST_ERROR
    if (H5Fclose(file_id) < 0)
        TEST_ERROR

    PASSED();

    return 0;

error:
    H5E_BEGIN_TRY {
        H5Pclose(fapl_id);
        H5Fclose(file_id);
    } H5E_END_TRY;

    return 1;
}

/*
 * A test for H5Gflush.
 */
static int
test_flush_group(void)
{
    TESTING("H5Gflush")

    SKIPPED();

    return 0;
}

/*
 * A test to check that H5Gflush fails when it
 * is passed invalid parameters.
 */
static int
test_flush_group_invalid_params(void)
{
    TESTING("H5Gflush with invalid parameters")

    SKIPPED();

    return 0;
}

/*
 * A test for H5Grefresh.
 */
static int
test_refresh_group(void)
{
    TESTING("H5Grefresh")

    SKIPPED();

    return 0;
}

/*
 * A test to check that H5Grefresh fails when it
 * is passed invalid parameters.
 */
static int
test_refresh_group_invalid_params(void)
{
    TESTING("H5Grefresh with invalid parameters")

    SKIPPED();

    return 0;
}

int
vol_group_test(void)
{
    size_t i;
    int    nerrors;

    printf("**********************************************\n");
    printf("*                                            *\n");
    printf("*              VOL Group Tests               *\n");
    printf("*                                            *\n");
    printf("**********************************************\n\n");

    for (i = 0, nerrors = 0; i < ARRAY_LENGTH(group_tests); i++) {
        nerrors += (*group_tests[i])() ? 1 : 0;
    }

    printf("\n");

done:
    return nerrors;
}

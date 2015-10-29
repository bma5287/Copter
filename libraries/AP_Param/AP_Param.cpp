// -*- tab-width: 4; Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*-
/*
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

//
//

// total up and check overflow
// check size of group var_info

/// @file   AP_Param.cpp
/// @brief  The AP variable store.


#include <AP_Common/AP_Common.h>
#include <AP_HAL/AP_HAL.h>
#include <AP_Math/AP_Math.h>
#include <AP_Progmem/AP_Progmem.h>
#include <StorageManager/StorageManager.h>

#include <math.h>
#include <string.h>

extern const AP_HAL::HAL &hal;

#define ENABLE_DEBUG 0

#if ENABLE_DEBUG
 # define Debug(fmt, args ...)  do {hal.console->printf("%s:%d: " fmt "\n", __FUNCTION__, __LINE__, ## args); } while(0)
#else
 # define Debug(fmt, args ...)
#endif

// some useful progmem macros
#define PGM_UINT8(addr) pgm_read_byte((const char *)addr)
#define PGM_UINT16(addr) pgm_read_word((const uint16_t *)addr)
#define PGM_FLOAT(addr) pgm_read_float((const float *)addr)
#define PGM_POINTER(addr) pgm_read_pointer((const void *)addr)

// the 'GROUP_ID' of a element of a group is the 18 bit identifier
// used to distinguish between this element of the group and other
// elements of the same group. It is calculated using a bit shift per
// level of nesting, so the first level of nesting gets 6 bits the 2nd
// level gets the next 6 bits, and the 3rd level gets the last 6
// bits. This limits groups to having at most 64 elements.
#define GROUP_ID(grpinfo, base, i, shift) ((base)+(((uint16_t)PGM_UINT8(&grpinfo[i].idx))<<(shift)))

// Note about AP_Vector3f handling.
// The code has special cases for AP_Vector3f to allow it to be viewed
// as both a single 3 element vector and as a set of 3 AP_Float
// variables. This is done to make it possible for MAVLink to see
// vectors as parameters, which allows users to save their compass
// offsets in MAVLink parameter files. The code involves quite a few
// special cases which could be generalised to any vector/matrix type
// if we end up needing this behaviour for other than AP_Vector3f


// static member variables for AP_Param.
//

// number of rows in the _var_info[] table
uint8_t AP_Param::_num_vars;

// storage and naming information about all types that can be saved
const AP_Param::Info *AP_Param::_var_info;

struct AP_Param::param_override *AP_Param::param_overrides = NULL;
uint16_t AP_Param::num_param_overrides = 0;

// storage object
StorageAccess AP_Param::_storage(StorageManager::StorageParam);


// write to EEPROM
void AP_Param::eeprom_write_check(const void *ptr, uint16_t ofs, uint8_t size)
{
    _storage.write_block(ofs, ptr, size);
}

// write a sentinal value at the given offset
void AP_Param::write_sentinal(uint16_t ofs)
{
    struct Param_header phdr;
    phdr.type = _sentinal_type;
    phdr.key  = _sentinal_key;
    phdr.group_element = _sentinal_group;
    eeprom_write_check(&phdr, ofs, sizeof(phdr));
}

// erase all EEPROM variables by re-writing the header and adding
// a sentinal
void AP_Param::erase_all(void)
{
    struct EEPROM_header hdr;

    Debug("erase_all");

    // write the header
    hdr.magic[0] = k_EEPROM_magic0;
    hdr.magic[1] = k_EEPROM_magic1;
    hdr.revision = k_EEPROM_revision;
    hdr.spare    = 0;
    eeprom_write_check(&hdr, 0, sizeof(hdr));

    // add a sentinal directly after the header
    write_sentinal(sizeof(struct EEPROM_header));
}

// validate a group info table
bool AP_Param::check_group_info(const struct AP_Param::GroupInfo *  group_info,
                                uint16_t *                          total_size,
                                uint8_t                             group_shift,
                                uint8_t                             prefix_length)
{
    uint8_t type;
    int8_t max_idx = -1;
    for (uint8_t i=0;
         (type=PGM_UINT8(&group_info[i].type)) != AP_PARAM_NONE;
         i++) {
#ifdef AP_NESTED_GROUPS_ENABLED
        if (type == AP_PARAM_GROUP) {
            // a nested group
            const struct GroupInfo *ginfo = (const struct GroupInfo *)PGM_POINTER(&group_info[i].group_info);
            if (group_shift + _group_level_shift >= _group_bits) {
                Debug("double group nesting in %s", group_info[i].name);
                return false;
            }
            if (ginfo == NULL ||
                !check_group_info(ginfo, total_size, group_shift + _group_level_shift, prefix_length + strlen(group_info[i].name))) {
                return false;
            }
            continue;
        }
#endif // AP_NESTED_GROUPS_ENABLED
        uint8_t idx = PGM_UINT8(&group_info[i].idx);
        if (idx >= (1<<_group_level_shift)) {
            Debug("idx too large (%u) in %s", idx, group_info[i].name);
            return false;
        }
        if ((int8_t)idx <= max_idx) {
            Debug("indexes must be in increasing order in %s", group_info[i].name);
            return false;
        }
        max_idx = (int8_t)idx;
        uint8_t size = type_size((enum ap_var_type)type);
        if (size == 0) {
            Debug("invalid type in %s", group_info[i].name);
            return false;
        }
        if (prefix_length + strlen(group_info[i].name) > 16) {
            Debug("suffix is too long in %s", group_info[i].name);
            return false;
        }
        (*total_size) += size + sizeof(struct Param_header);
    }
    return true;
}

// check for duplicate key values
bool AP_Param::duplicate_key(uint8_t vindex, uint8_t key)
{
    for (uint8_t i=vindex+1; i<_num_vars; i++) {
        uint8_t key2 = PGM_UINT8(&_var_info[i].key);
        if (key2 == key) {
            // no duplicate keys allowed
            return true;
        }
    }
    return false;
}

// validate the _var_info[] table
bool AP_Param::check_var_info(void)
{
    uint16_t total_size = sizeof(struct EEPROM_header);

    for (uint8_t i=0; i<_num_vars; i++) {
        uint8_t type = PGM_UINT8(&_var_info[i].type);
        uint8_t key = PGM_UINT8(&_var_info[i].key);
        if (type == AP_PARAM_GROUP) {
            if (i == 0) {
                // first element can't be a group, for first() call
                return false;
            }
            const struct GroupInfo *group_info = (const struct GroupInfo *)PGM_POINTER(&_var_info[i].group_info);
            if (group_info == NULL ||
                !check_group_info(group_info, &total_size, 0, strlen(_var_info[i].name))) {
                return false;
            }
        } else {
            uint8_t size = type_size((enum ap_var_type)type);
            if (size == 0) {
                // not a valid type - the top level list can't contain
                // AP_PARAM_NONE
                return false;
            }
            total_size += size + sizeof(struct Param_header);
        }
        if (duplicate_key(i, key)) {
            return false;
        }
    }

    // we no longer check if total_size is larger than _eeprom_size,
    // as we allow for more variables than could fit, relying on not
    // saving default values

    return true;
}


// setup the _var_info[] table
bool AP_Param::setup(void)
{
    struct EEPROM_header hdr;

    Debug("setup %u vars", (unsigned)_num_vars);

    // check the header
    _storage.read_block(&hdr, 0, sizeof(hdr));
    if (hdr.magic[0] != k_EEPROM_magic0 ||
        hdr.magic[1] != k_EEPROM_magic1 ||
        hdr.revision != k_EEPROM_revision) {
        // header doesn't match. We can't recover any variables. Wipe
        // the header and setup the sentinal directly after the header
        Debug("bad header in setup - erasing");
        erase_all();
    }

    return true;
}

// check if AP_Param has been initialised
bool AP_Param::initialised(void)
{
    return _var_info != NULL;
}

// find the info structure given a header and a group_info table
// return the Info structure and a pointer to the variables storage
const struct AP_Param::Info *AP_Param::find_by_header_group(struct Param_header phdr, void **ptr,
                                                            uint8_t vindex,
                                                            const struct GroupInfo *group_info,
                                                            uint8_t group_base,
                                                            uint8_t group_shift)
{
    uint8_t type;
    for (uint8_t i=0;
         (type=PGM_UINT8(&group_info[i].type)) != AP_PARAM_NONE;
         i++) {
#ifdef AP_NESTED_GROUPS_ENABLED
        if (type == AP_PARAM_GROUP) {
            // a nested group
            if (group_shift + _group_level_shift >= _group_bits) {
                // too deeply nested - this should have been caught by
                // setup() !
                return NULL;
            }
            const struct GroupInfo *ginfo = (const struct GroupInfo *)PGM_POINTER(&group_info[i].group_info);
            const struct AP_Param::Info *ret = find_by_header_group(phdr, ptr, vindex, ginfo,
                                                                    GROUP_ID(group_info, group_base, i, group_shift),
                                                                    group_shift + _group_level_shift);
            if (ret != NULL) {
                return ret;
            }
            continue;
        }
#endif // AP_NESTED_GROUPS_ENABLED
        if (GROUP_ID(group_info, group_base, i, group_shift) == phdr.group_element && type == phdr.type) {
            // found a group element
            *ptr = (void*)(PGM_POINTER(&_var_info[vindex].ptr) + PGM_UINT16(&group_info[i].offset));
            return &_var_info[vindex];
        }
    }
    return NULL;
}

// find the info structure given a header
// return the Info structure and a pointer to the variables storage
const struct AP_Param::Info *AP_Param::find_by_header(struct Param_header phdr, void **ptr)
{
    // loop over all named variables
    for (uint8_t i=0; i<_num_vars; i++) {
        uint8_t type = PGM_UINT8(&_var_info[i].type);
        uint8_t key = PGM_UINT8(&_var_info[i].key);
        if (key != phdr.key) {
            // not the right key
            continue;
        }
        if (type == AP_PARAM_GROUP) {
            const struct GroupInfo *group_info = (const struct GroupInfo *)PGM_POINTER(&_var_info[i].group_info);
            return find_by_header_group(phdr, ptr, i, group_info, 0, 0);
        }
        if (type == phdr.type) {
            // found it
            *ptr = (void*)PGM_POINTER(&_var_info[i].ptr);
            return &_var_info[i];
        }
    }
    return NULL;
}

// find the info structure for a variable in a group
const struct AP_Param::Info *AP_Param::find_var_info_group(const struct GroupInfo * group_info,
                                                           uint8_t                  vindex,
                                                           uint8_t                  group_base,
                                                           uint8_t                  group_shift,
                                                           uint32_t *               group_element,
                                                           const struct GroupInfo **group_ret,
                                                           uint8_t *                idx) const
{
    uintptr_t base = PGM_POINTER(&_var_info[vindex].ptr);
    uint8_t type;
    for (uint8_t i=0;
         (type=PGM_UINT8(&group_info[i].type)) != AP_PARAM_NONE;
         i++) {
        uintptr_t ofs = PGM_POINTER(&group_info[i].offset);
#ifdef AP_NESTED_GROUPS_ENABLED
        if (type == AP_PARAM_GROUP) {
            const struct GroupInfo *ginfo = (const struct GroupInfo *)PGM_POINTER(&group_info[i].group_info);
            // a nested group
            if (group_shift + _group_level_shift >= _group_bits) {
                // too deeply nested - this should have been caught by
                // setup() !
                return NULL;
            }
            const struct AP_Param::Info *info;
            info = find_var_info_group(ginfo, vindex,
                                       GROUP_ID(group_info, group_base, i, group_shift),
                                       group_shift + _group_level_shift,
                                       group_element,
                                       group_ret,
                                       idx);
            if (info != NULL) {
                return info;
            }
        } else // Forgive the poor formatting - if continues below.
#endif // AP_NESTED_GROUPS_ENABLED
        if ((uintptr_t) this == base + ofs) {
            *group_element = GROUP_ID(group_info, group_base, i, group_shift);
            *group_ret = &group_info[i];
            *idx = 0;
            return &_var_info[vindex];
        } else if (type == AP_PARAM_VECTOR3F &&
                   (base+ofs+sizeof(float) == (uintptr_t) this ||
                    base+ofs+2*sizeof(float) == (uintptr_t) this)) {
            // we are inside a Vector3f. We need to work out which
            // element of the vector the current object refers to.
            *idx = (((uintptr_t) this) - (base+ofs))/sizeof(float);
            *group_element = GROUP_ID(group_info, group_base, i, group_shift);
            *group_ret = &group_info[i];
            return &_var_info[vindex];
        }
    }
    return NULL;
}

// find the info structure for a variable
const struct AP_Param::Info *AP_Param::find_var_info(uint32_t *                 group_element,
                                                     const struct GroupInfo **  group_ret,
                                                     uint8_t *                  idx)
{
    for (uint8_t i=0; i<_num_vars; i++) {
        uint8_t type = PGM_UINT8(&_var_info[i].type);
        uintptr_t base = PGM_POINTER(&_var_info[i].ptr);
        if (type == AP_PARAM_GROUP) {
            const struct GroupInfo *group_info = (const struct GroupInfo *)PGM_POINTER(&_var_info[i].group_info);
            const struct AP_Param::Info *info;
            info = find_var_info_group(group_info, i, 0, 0, group_element, group_ret, idx);
            if (info != NULL) {
                return info;
            }
        } else if (base == (uintptr_t) this) {
            *group_element = 0;
            *group_ret = NULL;
            *idx = 0;
            return &_var_info[i];
        } else if (type == AP_PARAM_VECTOR3F &&
                   (base+sizeof(float) == (uintptr_t) this ||
                    base+2*sizeof(float) == (uintptr_t) this)) {
            // we are inside a Vector3f. Work out which element we are
            // referring to.
            *idx = (((uintptr_t) this) - base)/sizeof(float);
            *group_element = 0;
            *group_ret = NULL;
            return &_var_info[i];
        }
    }
    return NULL;
}


// find the info structure for a variable
const struct AP_Param::Info *AP_Param::find_var_info_token(const ParamToken &token,
                                                           uint32_t *                 group_element,
                                                           const struct GroupInfo **  group_ret,
                                                           uint8_t *                  idx) const
{
    uint8_t i = token.key;
    uint8_t type = PGM_UINT8(&_var_info[i].type);
    uintptr_t base = PGM_POINTER(&_var_info[i].ptr);
    if (type == AP_PARAM_GROUP) {
        const struct GroupInfo *group_info = (const struct GroupInfo *)PGM_POINTER(&_var_info[i].group_info);
        const struct AP_Param::Info *info;
        info = find_var_info_group(group_info, i, 0, 0, group_element, group_ret, idx);
        if (info != NULL) {
            return info;
        }
    } else if (base == (uintptr_t) this) {
        *group_element = 0;
        *group_ret = NULL;
        *idx = 0;
        return &_var_info[i];
    } else if (type == AP_PARAM_VECTOR3F &&
               (base+sizeof(float) == (uintptr_t) this ||
                base+2*sizeof(float) == (uintptr_t) this)) {
        // we are inside a Vector3f. Work out which element we are
        // referring to.
        *idx = (((uintptr_t) this) - base)/sizeof(float);
        *group_element = 0;
        *group_ret = NULL;
        return &_var_info[i];
    }
    return NULL;
}

// return the storage size for a AP_PARAM_* type
uint8_t AP_Param::type_size(enum ap_var_type type)
{
    switch (type) {
    case AP_PARAM_NONE:
    case AP_PARAM_GROUP:
        return 0;
    case AP_PARAM_INT8:
        return 1;
    case AP_PARAM_INT16:
        return 2;
    case AP_PARAM_INT32:
        return 4;
    case AP_PARAM_FLOAT:
        return 4;
    case AP_PARAM_VECTOR3F:
        return 3*4;
    case AP_PARAM_VECTOR6F:
        return 6*4;
    case AP_PARAM_MATRIX3F:
        return 3*3*4;
    }
    Debug("unknown type %u\n", type);
    return 0;
}

// scan the EEPROM looking for a given variable by header content
// return true if found, along with the offset in the EEPROM where
// the variable is stored
// if not found return the offset of the sentinal
// if the sentinal isn't found either, the offset is set to 0xFFFF
bool AP_Param::scan(const AP_Param::Param_header *target, uint16_t *pofs)
{
    struct Param_header phdr;
    uint16_t ofs = sizeof(AP_Param::EEPROM_header);
    while (ofs < _storage.size()) {
        _storage.read_block(&phdr, ofs, sizeof(phdr));
        if (phdr.type == target->type &&
            phdr.key == target->key &&
            phdr.group_element == target->group_element) {
            // found it
            *pofs = ofs;
            return true;
        }
        // note that this is an ||, not an &&, as this makes us more
        // robust to power off while adding a variable to EEPROM
        if (phdr.type == _sentinal_type ||
            phdr.key == _sentinal_key ||
            phdr.group_element == _sentinal_group) {
            // we've reached the sentinal
            *pofs = ofs;
            return false;
        }
        ofs += type_size((enum ap_var_type)phdr.type) + sizeof(phdr);
    }
    *pofs = 0xffff;
    Debug("scan past end of eeprom");
    return false;
}

/**
 * add a _X, _Y, _Z suffix to the name of a Vector3f element
 * @param buffer
 * @param buffer_size
 * @param idx Suffix: 0 --> _X; 1 --> _Y; 2 --> _Z; (other --> undefined)
 */
void AP_Param::add_vector3f_suffix(char *buffer, size_t buffer_size, uint8_t idx) const
{
    const size_t len = strnlen(buffer, buffer_size);
    if (len + 2 <= buffer_size) {
        buffer[len] = '_';
        buffer[len + 1] = static_cast<char>('X' + idx);
        if (len + 3 <= buffer_size) {
            buffer[len + 2] = 0;
        }
    }
}

// Copy the variable's whole name to the supplied buffer.
//
// If the variable is a group member, prepend the group name.
//
void AP_Param::copy_name_token(const ParamToken &token, char *buffer, size_t buffer_size, bool force_scalar) const
{
    uint32_t group_element;
    const struct GroupInfo *ginfo;
    uint8_t idx;
    const struct AP_Param::Info *info = find_var_info_token(token, &group_element, &ginfo, &idx);
    if (info == NULL) {
        *buffer = 0;
        Debug("no info found");
        return;
    }
    strncpy(buffer, info->name, buffer_size);
    if (ginfo != NULL) {
        uint8_t len = strnlen(buffer, buffer_size);
        if (len < buffer_size) {
            strncpy(&buffer[len], ginfo->name, buffer_size-len);
        }
        if ((force_scalar || idx != 0) && AP_PARAM_VECTOR3F == PGM_UINT8(&ginfo->type)) {
            // the caller wants a specific element in a Vector3f
            add_vector3f_suffix(buffer, buffer_size, idx);
        }
    } else if ((force_scalar || idx != 0) && AP_PARAM_VECTOR3F == PGM_UINT8(&info->type)) {
        add_vector3f_suffix(buffer, buffer_size, idx);
    }
}

// Find a variable by name in a group
AP_Param *
AP_Param::find_group(const char *name, uint8_t vindex, const struct GroupInfo *group_info, enum ap_var_type *ptype)
{
    uint8_t type;
    for (uint8_t i=0;
         (type=PGM_UINT8(&group_info[i].type)) != AP_PARAM_NONE;
         i++) {
#ifdef AP_NESTED_GROUPS_ENABLED
        if (type == AP_PARAM_GROUP) {
            const struct GroupInfo *ginfo = (const struct GroupInfo *)PGM_POINTER(&group_info[i].group_info);
            AP_Param *ap = find_group(name, vindex, ginfo, ptype);
            if (ap != NULL) {
                return ap;
            }
        } else
#endif // AP_NESTED_GROUPS_ENABLED
        if (strcasecmp(name, group_info[i].name) == 0) {
            uintptr_t p = PGM_POINTER(&_var_info[vindex].ptr);
            *ptype = (enum ap_var_type)type;
            return (AP_Param *)(p + PGM_POINTER(&group_info[i].offset));
        } else if (type == AP_PARAM_VECTOR3F) {
            // special case for finding Vector3f elements
            uint8_t suffix_len = strnlen(group_info[i].name, AP_MAX_NAME_SIZE);
            if (strncmp(name, group_info[i].name, suffix_len) == 0 &&
                name[suffix_len] == '_' &&
                (name[suffix_len+1] == 'X' ||
                 name[suffix_len+1] == 'Y' ||
                 name[suffix_len+1] == 'Z')) {
                uintptr_t p = PGM_POINTER(&_var_info[vindex].ptr);
                AP_Float *v = (AP_Float *)(p + PGM_POINTER(&group_info[i].offset));
                *ptype = AP_PARAM_FLOAT;
                switch (name[suffix_len+1]) {
                case 'X':
                    return (AP_Float *)&v[0];
                case 'Y':
                    return (AP_Float *)&v[1];
                case 'Z':
                    return (AP_Float *)&v[2];
                }
            }
        }
    }
    return NULL;
}


// Find a variable by name.
//
AP_Param *
AP_Param::find(const char *name, enum ap_var_type *ptype)
{
    for (uint8_t i=0; i<_num_vars; i++) {
        uint8_t type = PGM_UINT8(&_var_info[i].type);
        if (type == AP_PARAM_GROUP) {
            uint8_t len = strnlen(_var_info[i].name, AP_MAX_NAME_SIZE);
            if (strncmp(name, _var_info[i].name, len) != 0) {
                continue;
            }
            const struct GroupInfo *group_info = (const struct GroupInfo *)PGM_POINTER(&_var_info[i].group_info);
            AP_Param *ap = find_group(name + len, i, group_info, ptype);
            if (ap != NULL) {
                return ap;
            }
            // we continue looking as we want to allow top level
            // parameter to have the same prefix name as group
            // parameters, for example CAM_P_G
        } else if (strcasecmp(name, _var_info[i].name) == 0) {
            *ptype = (enum ap_var_type)type;
            return (AP_Param *)PGM_POINTER(&_var_info[i].ptr);
        }
    }
    return NULL;
}

/*
  find the def_value for a variable by name
*/
const float *
AP_Param::find_def_value_ptr(const char *name)
{
    enum ap_var_type ptype;
    AP_Param *vp = find(name, &ptype);
    if (vp == NULL) {
        return NULL;
    }
    uint32_t group_element;
    const struct GroupInfo *ginfo;
    uint8_t gidx;
    const struct AP_Param::Info *info = vp->find_var_info(&group_element, &ginfo, &gidx);
    if (info == NULL) {
        return NULL;
    }
    if (ginfo != NULL) {
        return &ginfo->def_value;
    }
    return &info->def_value;
}

// Find a variable by index. Note that this is quite slow.
//
AP_Param *
AP_Param::find_by_index(uint16_t idx, enum ap_var_type *ptype, ParamToken *token)
{
    AP_Param *ap;
    uint16_t count=0;
    for (ap=AP_Param::first(token, ptype);
         ap && count < idx;
         ap=AP_Param::next_scalar(token, ptype)) {
        count++;
    }
    return ap;    
}

// Find a object by name.
//
AP_Param *
AP_Param::find_object(const char *name)
{
    for (uint8_t i=0; i<_num_vars; i++) {
        if (strcasecmp(name, _var_info[i].name) == 0) {
            return (AP_Param *)PGM_POINTER(&_var_info[i].ptr);
        }
    }
    return NULL;
}


// Save the variable to EEPROM, if supported
//
bool AP_Param::save(bool force_save)
{
    uint32_t group_element = 0;
    const struct GroupInfo *ginfo;
    uint8_t idx;
    const struct AP_Param::Info *info = find_var_info(&group_element, &ginfo, &idx);
    const AP_Param *ap;

    if (info == NULL) {
        // we don't have any info on how to store it
        return false;
    }

    struct Param_header phdr;

    // create the header we will use to store the variable
    if (ginfo != NULL) {
        phdr.type = PGM_UINT8(&ginfo->type);
    } else {
        phdr.type = PGM_UINT8(&info->type);
    }
    phdr.key  = PGM_UINT8(&info->key);
    phdr.group_element = group_element;

    ap = this;
    if (phdr.type != AP_PARAM_VECTOR3F && idx != 0) {
        // only vector3f can have non-zero idx for now
        return false;
    }
    if (idx != 0) {
        ap = (const AP_Param *)((uintptr_t)ap) - (idx*sizeof(float));
    }

    // scan EEPROM to find the right location
    uint16_t ofs;
    if (scan(&phdr, &ofs)) {
        // found an existing copy of the variable
        eeprom_write_check(ap, ofs+sizeof(phdr), type_size((enum ap_var_type)phdr.type));
        return true;
    }
    if (ofs == (uint16_t) ~0) {
        return false;
    }

    // if the value is the default value then don't save
    if (phdr.type <= AP_PARAM_FLOAT) {
        float v1 = cast_to_float((enum ap_var_type)phdr.type);
        float v2;
        if (ginfo != NULL) {
            v2 = get_default_value(&ginfo->def_value);
        } else {
            v2 = get_default_value(&info->def_value);
        }
        if (is_equal(v1,v2) && !force_save) {
            return true;
        }
        if (phdr.type != AP_PARAM_INT32 &&
            (fabsf(v1-v2) < 0.0001f*fabsf(v1))) {
            // for other than 32 bit integers, we accept values within
            // 0.01 percent of the current value as being the same
            return true;
        }
    }

    if (ofs+type_size((enum ap_var_type)phdr.type)+2*sizeof(phdr) >= _storage.size()) {
        // we are out of room for saving variables
        hal.console->println("EEPROM full");
        return false;
    }

    // write a new sentinal, then the data, then the header
    write_sentinal(ofs + sizeof(phdr) + type_size((enum ap_var_type)phdr.type));
    eeprom_write_check(ap, ofs+sizeof(phdr), type_size((enum ap_var_type)phdr.type));
    eeprom_write_check(&phdr, ofs, sizeof(phdr));
    return true;
}

// Load the variable from EEPROM, if supported
//
bool AP_Param::load(void)
{
    uint32_t group_element = 0;
    const struct GroupInfo *ginfo;
    uint8_t idx;
    const struct AP_Param::Info *info = find_var_info(&group_element, &ginfo, &idx);
    if (info == NULL) {
        // we don't have any info on how to load it
        return false;
    }

    struct Param_header phdr;

    // create the header we will use to match the variable
    if (ginfo != NULL) {
        phdr.type = PGM_UINT8(&ginfo->type);
    } else {
        phdr.type = PGM_UINT8(&info->type);
    }
    phdr.key  = PGM_UINT8(&info->key);
    phdr.group_element = group_element;

    // scan EEPROM to find the right location
    uint16_t ofs;
    if (!scan(&phdr, &ofs)) {
        // if the value isn't stored in EEPROM then set the default value
        if (ginfo != NULL) {
            uintptr_t base = PGM_POINTER(&info->ptr);
            set_value((enum ap_var_type)phdr.type, (void*)(base + PGM_UINT16(&ginfo->offset)),
                      get_default_value(&ginfo->def_value));
        } else {
            set_value((enum ap_var_type)phdr.type, (void*)PGM_POINTER(&info->ptr), 
                      get_default_value(&info->def_value));
        }
        return false;
    }

    if (phdr.type != AP_PARAM_VECTOR3F && idx != 0) {
        // only vector3f can have non-zero idx for now
        return false;
    }

    AP_Param *ap;
    ap = this;
    if (idx != 0) {
        ap = (AP_Param *)((uintptr_t)ap) - (idx*sizeof(float));
    }

    // found it
    _storage.read_block(ap, ofs+sizeof(phdr), type_size((enum ap_var_type)phdr.type));
    return true;
}

bool AP_Param::configured_in_storage(void)
{
    uint32_t group_element = 0;
    const struct GroupInfo *ginfo;
    uint8_t idx;
    const struct AP_Param::Info *info = find_var_info(&group_element, &ginfo, &idx);
    if (info == NULL) {
        // we don't have any info on how to load it
        return false;
    }

    struct Param_header phdr;

    // create the header we will use to match the variable
    if (ginfo != NULL) {
        phdr.type = PGM_UINT8(&ginfo->type);
    } else {
        phdr.type = PGM_UINT8(&info->type);
    }
    phdr.key  = PGM_UINT8(&info->key);
    phdr.group_element = group_element;

    // scan EEPROM to find the right location
    uint16_t ofs;

    // only vector3f can have non-zero idx for now
    return scan(&phdr, &ofs) && (phdr.type == AP_PARAM_VECTOR3F || idx == 0);
}

bool AP_Param::configured_in_defaults_file(void)
{
    uint32_t group_element = 0;
    const struct GroupInfo *ginfo;
    uint8_t idx;
    const struct AP_Param::Info *info = find_var_info(&group_element, &ginfo, &idx);
    if (info == NULL) {
        // we don't have any info on how to load it
        return false;
    }

    const float* def_value_ptr;

    if (ginfo != NULL) {
        def_value_ptr = &ginfo->def_value;
    } else {
        def_value_ptr = &info->def_value;
    }

    for (uint16_t i=0; i<num_param_overrides; i++) {
        if (def_value_ptr == param_overrides[i].def_value_ptr) {
            return true;
        }
    }

    return false;
}

// set a AP_Param variable to a specified value
void AP_Param::set_value(enum ap_var_type type, void *ptr, float value)
{
    switch (type) {
    case AP_PARAM_INT8:
        ((AP_Int8 *)ptr)->set(value);
        break;
    case AP_PARAM_INT16:
        ((AP_Int16 *)ptr)->set(value);
        break;
    case AP_PARAM_INT32:
        ((AP_Int32 *)ptr)->set(value);
        break;
    case AP_PARAM_FLOAT:
        ((AP_Float *)ptr)->set(value);
        break;
    default:
        break;
    }
}

// load default values for scalars in a group. This does not recurse
// into other objects. This is a static function that should be called
// in the objects constructor
void AP_Param::setup_object_defaults(const void *object_pointer, const struct GroupInfo *group_info)
{
    uintptr_t base = (uintptr_t)object_pointer;
    uint8_t type;
    for (uint8_t i=0;
         (type=PGM_UINT8(&group_info[i].type)) != AP_PARAM_NONE;
         i++) {
        if (type <= AP_PARAM_FLOAT) {
            void *ptr = (void *)(base + PGM_UINT16(&group_info[i].offset));
            set_value((enum ap_var_type)type, ptr, get_default_value(&group_info[i].def_value));
        }
    }
}

// set a value directly in an object. This should only be used by
// example code, not by mainline vehicle code
void AP_Param::set_object_value(const void *object_pointer, 
                                const struct GroupInfo *group_info, 
                                const char *name, float value)
{
    uintptr_t base = (uintptr_t)object_pointer;
    uint8_t type;
    for (uint8_t i=0;
         (type=PGM_UINT8(&group_info[i].type)) != AP_PARAM_NONE;
         i++) {
        if (strcmp(name, group_info[i].name) == 0 && type <= AP_PARAM_FLOAT) {
            void *ptr = (void *)(base + PGM_UINT16(&group_info[i].offset));
            set_value((enum ap_var_type)type, ptr, value);
        }
    }
}


// load default values for all scalars in a sketch. This does not
// recurse into sub-objects
void AP_Param::setup_sketch_defaults(void)
{
    setup();
    for (uint8_t i=0; i<_num_vars; i++) {
        uint8_t type = PGM_UINT8(&_var_info[i].type);
        if (type <= AP_PARAM_FLOAT) {
            void *ptr = (void*)PGM_POINTER(&_var_info[i].ptr);
            set_value((enum ap_var_type)type, ptr, get_default_value(&_var_info[i].def_value));
        }
    }
}


// Load all variables from EEPROM
//
bool AP_Param::load_all(void)
{
    struct Param_header phdr;
    uint16_t ofs = sizeof(AP_Param::EEPROM_header);

    /*
      if the HAL specifies a defaults parameter file then override
      defaults using that file
     */
#ifdef HAL_PARAM_DEFAULTS_PATH
    load_defaults_file(HAL_PARAM_DEFAULTS_PATH);
#endif

    while (ofs < _storage.size()) {
        _storage.read_block(&phdr, ofs, sizeof(phdr));
        // note that this is an || not an && for robustness
        // against power off while adding a variable
        if (phdr.type == _sentinal_type ||
            phdr.key == _sentinal_key ||
            phdr.group_element == _sentinal_group) {
            // we've reached the sentinal
            return true;
        }

        const struct AP_Param::Info *info;
        void *ptr;

        info = find_by_header(phdr, &ptr);
        if (info != NULL) {
            _storage.read_block(ptr, ofs+sizeof(phdr), type_size((enum ap_var_type)phdr.type));
        }

        ofs += type_size((enum ap_var_type)phdr.type) + sizeof(phdr);
    }

    // we didn't find the sentinal
    Debug("no sentinal in load_all");
    return false;
}


// return the first variable in _var_info
AP_Param *AP_Param::first(ParamToken *token, enum ap_var_type *ptype)
{
    token->key = 0;
    token->group_element = 0;
    token->idx = 0;
    if (_num_vars == 0) {
        return NULL;
    }
    if (ptype != NULL) {
        *ptype = (enum ap_var_type)PGM_UINT8(&_var_info[0].type);
    }
    return (AP_Param *)(PGM_POINTER(&_var_info[0].ptr));
}

/// Returns the next variable in a group, recursing into groups
/// as needed
AP_Param *AP_Param::next_group(uint8_t vindex, const struct GroupInfo *group_info,
                               bool *found_current,
                               uint8_t group_base,
                               uint8_t group_shift,
                               ParamToken *token,
                               enum ap_var_type *ptype)
{
    enum ap_var_type type;
    for (uint8_t i=0;
         (type=(enum ap_var_type)PGM_UINT8(&group_info[i].type)) != AP_PARAM_NONE;
         i++) {
#ifdef AP_NESTED_GROUPS_ENABLED
        if (type == AP_PARAM_GROUP) {
            // a nested group
            const struct GroupInfo *ginfo = (const struct GroupInfo *)PGM_POINTER(&group_info[i].group_info);
            AP_Param *ap;
            ap = next_group(vindex, ginfo, found_current, GROUP_ID(group_info, group_base, i, group_shift),
                            group_shift + _group_level_shift, token, ptype);
            if (ap != NULL) {
                return ap;
            }
        } else
#endif // AP_NESTED_GROUPS_ENABLED
        {
            if (*found_current) {
                // got a new one
                token->key = vindex;
                token->group_element = GROUP_ID(group_info, group_base, i, group_shift);
                token->idx = 0;
                if (ptype != NULL) {
                    *ptype = type;
                }
                return (AP_Param*)(PGM_POINTER(&_var_info[vindex].ptr) + PGM_UINT16(&group_info[i].offset));
            }
            if (GROUP_ID(group_info, group_base, i, group_shift) == token->group_element) {
                *found_current = true;
                if (type == AP_PARAM_VECTOR3F && token->idx < 3) {
                    // return the next element of the vector as a
                    // float
                    token->idx++;
                    if (ptype != NULL) {
                        *ptype = AP_PARAM_FLOAT;
                    }
                    uintptr_t ofs = (uintptr_t)PGM_POINTER(&_var_info[vindex].ptr) + PGM_UINT16(&group_info[i].offset);
                    ofs += sizeof(float)*(token->idx - 1u);
                    return (AP_Param *)ofs;
                }
            }
        }
    }
    return NULL;
}

/// Returns the next variable in _var_info, recursing into groups
/// as needed
AP_Param *AP_Param::next(ParamToken *token, enum ap_var_type *ptype)
{
    uint8_t i = token->key;
    bool found_current = false;
    if (i >= _num_vars) {
        // illegal token
        return NULL;
    }
    enum ap_var_type type = (enum ap_var_type)PGM_UINT8(&_var_info[i].type);

    // allow Vector3f to be seen as 3 variables. First as a vector,
    // then as 3 separate floats
    if (type == AP_PARAM_VECTOR3F && token->idx < 3) {
        token->idx++;
        if (ptype != NULL) {
            *ptype = AP_PARAM_FLOAT;
        }
        return (AP_Param *)(((token->idx - 1u)*sizeof(float))+(uintptr_t)PGM_POINTER(&_var_info[i].ptr));
    }

    if (type != AP_PARAM_GROUP) {
        i++;
        found_current = true;
    }
    for (; i<_num_vars; i++) {
        type = (enum ap_var_type)PGM_UINT8(&_var_info[i].type);
        if (type == AP_PARAM_GROUP) {
            const struct GroupInfo *group_info = (const struct GroupInfo *)PGM_POINTER(&_var_info[i].group_info);
            AP_Param *ap = next_group(i, group_info, &found_current, 0, 0, token, ptype);
            if (ap != NULL) {
                return ap;
            }
        } else {
            // found the next one
            token->key = i;
            token->group_element = 0;
            token->idx = 0;
            if (ptype != NULL) {
                *ptype = type;
            }
            return (AP_Param *)(PGM_POINTER(&_var_info[i].ptr));
        }
    }
    return NULL;
}

/// Returns the next scalar in _var_info, recursing into groups
/// as needed
AP_Param *AP_Param::next_scalar(ParamToken *token, enum ap_var_type *ptype)
{
    AP_Param *ap;
    enum ap_var_type type;
    while ((ap = next(token, &type)) != NULL && type > AP_PARAM_FLOAT) ;
    if (ap != NULL && ptype != NULL) {
        *ptype = type;
    }
    return ap;
}


/// cast a variable to a float given its type
float AP_Param::cast_to_float(enum ap_var_type type) const
{
    switch (type) {
    case AP_PARAM_INT8:
        return ((AP_Int8 *)this)->cast_to_float();
    case AP_PARAM_INT16:
        return ((AP_Int16 *)this)->cast_to_float();
    case AP_PARAM_INT32:
        return ((AP_Int32 *)this)->cast_to_float();
    case AP_PARAM_FLOAT:
        return ((AP_Float *)this)->cast_to_float();
    default:
        return NAN;
    }
}


// print the value of all variables
void AP_Param::show(const AP_Param *ap, const char *s,
                    enum ap_var_type type, AP_HAL::BetterStream *port)
{
    switch (type) {
    case AP_PARAM_INT8:
        port->printf("%s: %d\n", s, (int)((AP_Int8 *)ap)->get());
        break;
    case AP_PARAM_INT16:
        port->printf("%s: %d\n", s, (int)((AP_Int16 *)ap)->get());
        break;
    case AP_PARAM_INT32:
        port->printf("%s: %ld\n", s, (long)((AP_Int32 *)ap)->get());
        break;
    case AP_PARAM_FLOAT:
        port->printf("%s: %f\n", s, (double)((AP_Float *)ap)->get());
        break;
    default:
        break;
    }
}

// print the value of all variables
void AP_Param::show(const AP_Param *ap, const ParamToken &token,
                    enum ap_var_type type, AP_HAL::BetterStream *port)
{
    char s[AP_MAX_NAME_SIZE+1];
    ap->copy_name_token(token, s, sizeof(s), true);
    s[AP_MAX_NAME_SIZE] = 0;
    show(ap, s, type, port);
}

// print the value of all variables
void AP_Param::show_all(AP_HAL::BetterStream *port, bool showKeyValues)
{
    ParamToken token;
    AP_Param *ap;
    enum ap_var_type type;

    for (ap=AP_Param::first(&token, &type);
         ap;
         ap=AP_Param::next_scalar(&token, &type)) {
        if (showKeyValues) {
            port->printf("Key %i: Index %i: GroupElement %i  :  ", token.key, token.idx, token.group_element);
        }
        show(ap, token, type, port);
    }
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat"
// convert one old vehicle parameter to new object parameter
void AP_Param::convert_old_parameter(const struct ConversionInfo *info)
{

    // find the old value in EEPROM.
    uint16_t pofs;
    AP_Param::Param_header header;
    header.type = PGM_UINT8(&info->type);
    header.key = PGM_UINT8(&info->old_key);
    header.group_element = PGM_UINT8(&info->old_group_element);
    if (!scan(&header, &pofs)) {
        // the old parameter isn't saved in the EEPROM. It was
        // probably still set to the default value, which isn't stored
        // no need to convert
        return;
    }

    // load the old value from EEPROM
    uint8_t old_value[type_size((enum ap_var_type)header.type)];
    _storage.read_block(old_value, pofs+sizeof(header), sizeof(old_value));
    const AP_Param *ap = (const AP_Param *)&old_value[0];

    // find the new variable in the variable structures
    enum ap_var_type ptype;
    AP_Param *ap2;
    ap2 = find(&info->new_name[0], &ptype);
    if (ap2 == NULL) {
        hal.console->printf("Unknown conversion '%s'\n", info->new_name);
        return;
    }

    // see if we can load it from EEPROM
    if (ap2->load()) {
        // the new parameter already has a value set by the user, or
        // has already been converted
        return;
    }

    // see if they are the same type
    if (ptype == (ap_var_type)header.type) {
        // copy the value over only if the new parameter does not already
        // have the old value (via a default).
        if (memcmp(ap2, ap, sizeof(old_value)) != 0) {
            memcpy(ap2, ap, sizeof(old_value));
            // and save
            ap2->save();
        }
    } else if (ptype <= AP_PARAM_FLOAT && header.type <= AP_PARAM_FLOAT) {
        // perform scalar->scalar conversion
        float v = ap->cast_to_float((enum ap_var_type)header.type);
        if (!is_equal(v,ap2->cast_to_float(ptype))) {
            // the value needs to change
            set_value(ptype, ap2, v);
            ap2->save();
        }
    } else {
        // can't do vector<->scalar conversion, or different vector types
        hal.console->printf("Bad conversion type '%s'\n", info->new_name);
    }
}
#pragma GCC diagnostic pop


// convert old vehicle parameters to new object parametersv
void AP_Param::convert_old_parameters(const struct ConversionInfo *conversion_table, uint8_t table_size)
{
    for (uint8_t i=0; i<table_size; i++) {
        convert_old_parameter(&conversion_table[i]);
    }
}

/*
  set a parameter to a float value
 */
void AP_Param::set_float(float value, enum ap_var_type var_type)
{
    if (isnan(value) || isinf(value)) {
        return;
    }

    // add a small amount before casting parameter values
    // from float to integer to avoid truncating to the
    // next lower integer value.
    float rounding_addition = 0.01f;
        
    // handle variables with standard type IDs
    if (var_type == AP_PARAM_FLOAT) {
        ((AP_Float *)this)->set(value);
    } else if (var_type == AP_PARAM_INT32) {
        if (value < 0) rounding_addition = -rounding_addition;
        float v = value+rounding_addition;
        v = constrain_float(v, -2147483648.0, 2147483647.0);
        ((AP_Int32 *)this)->set(v);
    } else if (var_type == AP_PARAM_INT16) {
        if (value < 0) rounding_addition = -rounding_addition;
        float v = value+rounding_addition;
        v = constrain_float(v, -32768, 32767);
        ((AP_Int16 *)this)->set(v);
    } else if (var_type == AP_PARAM_INT8) {
        if (value < 0) rounding_addition = -rounding_addition;
        float v = value+rounding_addition;
        v = constrain_float(v, -128, 127);
        ((AP_Int8 *)this)->set(v);
    }
}


#if HAL_OS_POSIX_IO == 1
#include <stdio.h>

/*
  parse a parameter file line
 */
bool AP_Param::parse_param_line(char *line, char **vname, float &value)
{
    if (line[0] == '#') {
        return false;
    }
    char *saveptr = NULL;
    char *pname = strtok_r(line, ", =\t", &saveptr);
    if (pname == NULL) {
        return false;
    }
    if (strlen(pname) > AP_MAX_NAME_SIZE) {
        return false;
    }
    const char *value_s = strtok_r(NULL, ", =\t", &saveptr);
    if (value_s == NULL) {
        return false;
    }
    value = atof(value_s);
    *vname = pname;
    return true;
}

/*
  load a default set of parameters from a file
 */
bool AP_Param::load_defaults_file(const char *filename)
{
    FILE *f = fopen(filename, "r");
    if (f == NULL) {
        return false;
    }
    char line[100];

    /*
      work out how many parameter default structures to allocate
     */
    uint16_t num_defaults = 0;
    while (fgets(line, sizeof(line)-1, f)) {
        char *pname;
        float value;
        if (!parse_param_line(line, &pname, value)) {
            continue;
        }
        if (!find_def_value_ptr(pname)) {
            fclose(f);
            return false;
        }
        num_defaults++;
    }
    fclose(f);

    if (param_overrides != NULL) {
        free(param_overrides);
    }
    num_param_overrides = 0;

    param_overrides = new param_override[num_defaults];
    if (param_overrides == NULL) {
        return false;
    }

    /* 
       re-open to avoid possible seek issues with NuttX
     */
    f = fopen(filename, "r");
    if (f == NULL) {
        return false;
    }

    uint16_t idx = 0;
    while (fgets(line, sizeof(line)-1, f)) {
        char *pname;
        float value;
        if (!parse_param_line(line, &pname, value)) {
            continue;
        }
        const float *def_value_ptr = find_def_value_ptr(pname);
        if (!def_value_ptr) {
            fclose(f);
            return false;
        }
        param_overrides[idx].def_value_ptr = def_value_ptr;
        param_overrides[idx].value = value;
        idx++;
        enum ap_var_type var_type;
        AP_Param *vp = AP_Param::find(pname, &var_type);
        if (!vp) {
            fclose(f);
            return false;
        }
        vp->set_float(value, var_type);
    }
    fclose(f);

    num_param_overrides = num_defaults;

    return true;
}

#endif // HAL_OS_POSIX_IO

/* 
   find a default value given a pointer to a default value in flash
 */
float AP_Param::get_default_value(const float *def_value_ptr)
{
    for (uint16_t i=0; i<num_param_overrides; i++) {
        if (def_value_ptr == param_overrides[i].def_value_ptr) {
            return param_overrides[i].value;
        }
    }
    return PGM_FLOAT(def_value_ptr);
}


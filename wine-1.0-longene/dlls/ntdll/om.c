/*
 *	Object management functions
 *
 * Copyright 1999, 2000 Juergen Schmied
 * Copyright 2005 Vitaliy Margolen
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include "config.h"

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_IO_H
# include <io.h>
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#include "ntstatus.h"
#define WIN32_NO_STATUS
#include "wine/debug.h"
#include "windef.h"
#include "winternl.h"
#include "ntdll_misc.h"
#include "wine/server.h"
#include "wine/log.h"

WINE_DEFAULT_DEBUG_CHANNEL(ntdll);


/*
 *	Generic object functions
 */

/******************************************************************************
 * NtQueryObject [NTDLL.@]
 * ZwQueryObject [NTDLL.@]
 */
NTSTATUS WINAPI NtQueryObject(IN HANDLE handle,
                              IN OBJECT_INFORMATION_CLASS info_class,
                              OUT PVOID ptr, IN ULONG len, OUT PULONG used_len)
{
    NTSTATUS status;

    TRACE("(%p,0x%08x,%p,0x%08x,%p): stub\n",
          handle, info_class, ptr, len, used_len);

    __asm__ __volatile__ (
            "movl $0x86,%%eax\n\t"
            "lea 8(%%ebp),%%edx\n\t"
            "int $0x2E\n\t"
            :"=a" (status)
            );
    return status;
}

/******************************************************************
 *		NtSetInformationObject [NTDLL.@]
 *		ZwSetInformationObject [NTDLL.@]
 *
 */
NTSTATUS WINAPI NtSetInformationObject(IN HANDLE handle,
                                       IN OBJECT_INFORMATION_CLASS info_class,
                                       IN PVOID ptr, IN ULONG len)
{
    NTSTATUS status;

    TRACE("(%p,0x%08x,%p,0x%08x): stub\n",
          handle, info_class, ptr, len);

    __asm__ __volatile__ (
            "movl $0xB9,%%eax\n\t"
            "lea 8(%%ebp),%%edx\n\t"
            "int $0x2E\n\t"
            :"=a" (status)
            );
    return status;
}

/******************************************************************************
 *  NtQuerySecurityObject	[NTDLL.@]
 *
 * An ntdll analogue to GetKernelObjectSecurity().
 *
 */
NTSTATUS WINAPI
NtQuerySecurityObject(
	IN HANDLE Object,
	IN SECURITY_INFORMATION RequestedInformation,
	OUT PSECURITY_DESCRIPTOR pSecurityDescriptor,
	IN ULONG Length,
	OUT PULONG ResultLength)
{
    PISECURITY_DESCRIPTOR_RELATIVE psd = pSecurityDescriptor;
    NTSTATUS status;
    unsigned int buffer_size = 512;
    BOOLEAN need_more_memory = FALSE;

    TRACE("(%p,0x%08x,%p,0x%08x,%p)\n",
	Object, RequestedInformation, pSecurityDescriptor, Length, ResultLength);

    do
    {
        char *buffer = RtlAllocateHeap(GetProcessHeap(), 0, buffer_size);
        if (!buffer)
            return STATUS_NO_MEMORY;

        SERVER_START_REQ( get_security_object )
        {
            req->handle = Object;
            req->security_info = RequestedInformation;
            wine_server_set_reply( req, buffer, buffer_size );
            status = wine_server_call( req );
            if (status == STATUS_SUCCESS)
            {
                struct security_descriptor *sd = (struct security_descriptor *)buffer;
                if (reply->sd_len)
                {
                    *ResultLength = sizeof(SECURITY_DESCRIPTOR_RELATIVE) +
                        sd->owner_len + sd->group_len + sd->sacl_len + sd->dacl_len;
                    if (Length >= *ResultLength)
                    {
                        psd->Revision = SECURITY_DESCRIPTOR_REVISION;
                        psd->Sbz1 = 0;
                        psd->Control = sd->control | SE_SELF_RELATIVE;
                        psd->Owner = sd->owner_len ? sizeof(SECURITY_DESCRIPTOR_RELATIVE) : 0;
                        psd->Group = sd->group_len ? sizeof(SECURITY_DESCRIPTOR_RELATIVE) + sd->owner_len : 0;
                        psd->Sacl = sd->sacl_len ? sizeof(SECURITY_DESCRIPTOR_RELATIVE) + sd->owner_len + sd->group_len : 0;
                        psd->Dacl = sd->dacl_len ? sizeof(SECURITY_DESCRIPTOR_RELATIVE) + sd->owner_len + sd->group_len + sd->sacl_len : 0;
                        /* owner, group, sacl and dacl are the same type as in the server
                         * and in the same order so we copy the memory in one block */
                        memcpy((char *)pSecurityDescriptor + sizeof(SECURITY_DESCRIPTOR_RELATIVE),
                               buffer + sizeof(struct security_descriptor),
                               sd->owner_len + sd->group_len + sd->sacl_len + sd->dacl_len);
                    }
                    else
                        status = STATUS_BUFFER_TOO_SMALL;
                }
                else
                {
                    *ResultLength = sizeof(SECURITY_DESCRIPTOR_RELATIVE);
                    if (Length >= *ResultLength)
                    {
                        memset(psd, 0, sizeof(*psd));
                        psd->Revision = SECURITY_DESCRIPTOR_REVISION;
                        psd->Control = SE_SELF_RELATIVE;
                    }
                    else
                        status = STATUS_BUFFER_TOO_SMALL;
                }
            }
            else if (status == STATUS_BUFFER_TOO_SMALL)
            {
                buffer_size = reply->sd_len;
                need_more_memory = TRUE;
            }
        }
        SERVER_END_REQ;
        RtlFreeHeap(GetProcessHeap(), 0, buffer);
    } while (need_more_memory);

    return status;
}


/******************************************************************************
 *  NtDuplicateObject		[NTDLL.@]
 *  ZwDuplicateObject		[NTDLL.@]
 */
NTSTATUS WINAPI NtDuplicateObject( HANDLE source_process, HANDLE source,
                                   HANDLE dest_process, PHANDLE dest,
                                   ACCESS_MASK access, ULONG attributes, ULONG options )
{
    NTSTATUS ret;

    __asm__ __volatile__ (
            "movl $0x34,%%eax\n\t"
            "lea 8(%%ebp),%%edx\n\t"
            "int $0x2E\n\t"
            :"=a" (ret)
            );
    return ret;
}

/**************************************************************************
 *                 NtClose				[NTDLL.@]
 *
 * Close a handle reference to an object.
 * 
 * PARAMS
 *  Handle [I] handle to close
 *
 * RETURNS
 *  Success: ERROR_SUCCESS.
 *  Failure: An NTSTATUS error code.
 */
NTSTATUS WINAPI NtClose( HANDLE Handle )
{
    NTSTATUS ret;
    
    LOG(LOG_FILE, 0, 0, "handle %p\n", Handle);
    server_remove_fd_from_cache( Handle );

    __asm__ __volatile__ (
            "movl $0x11,%%eax\n\t"
            "lea 8(%%ebp),%%edx\n\t"
            "int $0x2E\n\t"
            :"=a" (ret)
            );

    LOG(LOG_FILE, 0, ret, "return %x\n", ret);
    return ret;
}

/*
 *	Directory functions
 */

/**************************************************************************
 * NtOpenDirectoryObject [NTDLL.@]
 * ZwOpenDirectoryObject [NTDLL.@]
 *
 * Open a namespace directory object.
 * 
 * PARAMS
 *  DirectoryHandle  [O] Destination for the new directory handle
 *  DesiredAccess    [I] Desired access to the directory
 *  ObjectAttributes [I] Structure describing the directory
 *
 * RETURNS
 *  Success: ERROR_SUCCESS.
 *  Failure: An NTSTATUS error code.
 */
NTSTATUS WINAPI NtOpenDirectoryObject(PHANDLE DirectoryHandle, ACCESS_MASK DesiredAccess,
                                      POBJECT_ATTRIBUTES ObjectAttributes)
{
    NTSTATUS ret;
#ifndef UNIFIE_KERNEL
    TRACE("(%p,0x%08x)\n", DirectoryHandle, DesiredAccess);
    dump_ObjectAttributes(ObjectAttributes);

    if (!DirectoryHandle) return STATUS_ACCESS_VIOLATION;
    if (!ObjectAttributes) return STATUS_INVALID_PARAMETER;
    /* Have to test it here because server won't know difference between
     * ObjectName == NULL and ObjectName == "" */
    if (!ObjectAttributes->ObjectName)
    {
        if (ObjectAttributes->RootDirectory)
            return STATUS_OBJECT_NAME_INVALID;
        else
            return STATUS_OBJECT_PATH_SYNTAX_BAD;
    }

    SERVER_START_REQ(open_directory)
    {
        req->access = DesiredAccess;
        req->attributes = ObjectAttributes->Attributes;
        req->rootdir = ObjectAttributes->RootDirectory;
        if (ObjectAttributes->ObjectName)
            wine_server_add_data(req, ObjectAttributes->ObjectName->Buffer,
                                 ObjectAttributes->ObjectName->Length);
        ret = wine_server_call( req );
        *DirectoryHandle = reply->handle;
    }
    SERVER_END_REQ;
#else
    __asm__ __volatile__ (
            "movl $0x55,%%eax\n\t"
            "lea 8(%%ebp),%%edx\n\t"
            "int $0x2E\n\t"
            :"=a" (ret)
            );
#endif
    return ret;
}

/******************************************************************************
 *  NtCreateDirectoryObject	[NTDLL.@]
 *  ZwCreateDirectoryObject	[NTDLL.@]
 *
 * Create a namespace directory object.
 * 
 * PARAMS
 *  DirectoryHandle  [O] Destination for the new directory handle
 *  DesiredAccess    [I] Desired access to the directory
 *  ObjectAttributes [I] Structure describing the directory
 *
 * RETURNS
 *  Success: ERROR_SUCCESS.
 *  Failure: An NTSTATUS error code.
 */
NTSTATUS WINAPI NtCreateDirectoryObject(PHANDLE DirectoryHandle, ACCESS_MASK DesiredAccess,
                                        POBJECT_ATTRIBUTES ObjectAttributes)
{
    NTSTATUS ret;
    TRACE("(%p,0x%08x)\n", DirectoryHandle, DesiredAccess);
    dump_ObjectAttributes(ObjectAttributes);

    if (!DirectoryHandle) return STATUS_ACCESS_VIOLATION;

    SERVER_START_REQ(create_directory)
    {
        req->access = DesiredAccess;
        req->attributes = ObjectAttributes ? ObjectAttributes->Attributes : 0;
        req->rootdir = ObjectAttributes ? ObjectAttributes->RootDirectory : 0;
        if (ObjectAttributes && ObjectAttributes->ObjectName)
            wine_server_add_data(req, ObjectAttributes->ObjectName->Buffer,
                                 ObjectAttributes->ObjectName->Length);
        ret = wine_server_call( req );
        *DirectoryHandle = reply->handle;
    }
    SERVER_END_REQ;
    return ret;
}

/******************************************************************************
 * NtQueryDirectoryObject [NTDLL.@]
 * ZwQueryDirectoryObject [NTDLL.@]
 *
 * Read information from a namespace directory.
 * 
 * PARAMS
 *  handle        [I]   Handle to a directory object
 *  buffer        [O]   Buffer to hold the read data
 *  size          [I]   Size of the buffer in bytes
 *  single_entry  [I]   If TRUE, return a single entry, if FALSE, return as many as fit in the buffer
 *  restart       [I]   If TRUE, start scanning from the start, if FALSE, scan from Context
 *  context       [I/O] Indicates what point of the directory the scan is at
 *  ret_size      [O]   Caller supplied storage for the number of bytes written (or NULL)
 *
 * RETURNS
 *  Success: ERROR_SUCCESS.
 *  Failure: An NTSTATUS error code.
 */
NTSTATUS WINAPI NtQueryDirectoryObject(HANDLE handle, PDIRECTORY_BASIC_INFORMATION buffer,
                                       ULONG size, BOOLEAN single_entry, BOOLEAN restart,
                                       PULONG context, PULONG ret_size)
{
    NTSTATUS ret;

    __asm__ __volatile__ (
            "movl $0x76,%%eax\n\t"
            "lea 8(%%ebp),%%edx\n\t"
            "int $0x2E\n\t"
            :"=a" (ret)
            );

    return ret;
}

/*
 *	Link objects
 */

/******************************************************************************
 *  NtOpenSymbolicLinkObject	[NTDLL.@]
 *  ZwOpenSymbolicLinkObject	[NTDLL.@]
 *
 * Open a namespace symbolic link object.
 * 
 * PARAMS
 *  LinkHandle       [O] Destination for the new symbolic link handle
 *  DesiredAccess    [I] Desired access to the symbolic link
 *  ObjectAttributes [I] Structure describing the symbolic link
 *
 * RETURNS
 *  Success: ERROR_SUCCESS.
 *  Failure: An NTSTATUS error code.
 */
NTSTATUS WINAPI NtOpenSymbolicLinkObject(OUT PHANDLE LinkHandle, IN ACCESS_MASK DesiredAccess,
                                         IN POBJECT_ATTRIBUTES ObjectAttributes)
{
    NTSTATUS ret;
    TRACE("(%p,0x%08x,%p)\n",LinkHandle, DesiredAccess, ObjectAttributes);
    dump_ObjectAttributes(ObjectAttributes);

    if (!LinkHandle) return STATUS_ACCESS_VIOLATION;
    if (!ObjectAttributes) return STATUS_INVALID_PARAMETER;
    /* Have to test it here because server won't know difference between
     * ObjectName == NULL and ObjectName == "" */
    if (!ObjectAttributes->ObjectName)
    {
        if (ObjectAttributes->RootDirectory)
            return STATUS_OBJECT_NAME_INVALID;
        else
            return STATUS_OBJECT_PATH_SYNTAX_BAD;
    }

    SERVER_START_REQ(open_symlink)
    {
        req->access = DesiredAccess;
        req->attributes = ObjectAttributes->Attributes;
        req->rootdir = ObjectAttributes->RootDirectory;
        if (ObjectAttributes->ObjectName)
            wine_server_add_data(req, ObjectAttributes->ObjectName->Buffer,
                                 ObjectAttributes->ObjectName->Length);
        ret = wine_server_call( req );
        *LinkHandle = reply->handle;
    }
    SERVER_END_REQ;
    return ret;
}

/******************************************************************************
 *  NtCreateSymbolicLinkObject	[NTDLL.@]
 *  ZwCreateSymbolicLinkObject	[NTDLL.@]
 *
 * Open a namespace symbolic link object.
 * 
 * PARAMS
 *  SymbolicLinkHandle [O] Destination for the new symbolic link handle
 *  DesiredAccess      [I] Desired access to the symbolic link
 *  ObjectAttributes   [I] Structure describing the symbolic link
 *  TargetName         [I] Name of the target symbolic link points to
 *
 * RETURNS
 *  Success: ERROR_SUCCESS.
 *  Failure: An NTSTATUS error code.
 */
NTSTATUS WINAPI NtCreateSymbolicLinkObject(OUT PHANDLE SymbolicLinkHandle,IN ACCESS_MASK DesiredAccess,
	                                   IN POBJECT_ATTRIBUTES ObjectAttributes,
                                           IN PUNICODE_STRING TargetName)
{
    NTSTATUS ret;
    TRACE("(%p,0x%08x,%p, -> %s)\n", SymbolicLinkHandle, DesiredAccess, ObjectAttributes,
                                      debugstr_us(TargetName));
    dump_ObjectAttributes(ObjectAttributes);

    if (!SymbolicLinkHandle || !TargetName) return STATUS_ACCESS_VIOLATION;
    if (!TargetName->Buffer) return STATUS_INVALID_PARAMETER;

    SERVER_START_REQ(create_symlink)
    {
        req->access = DesiredAccess;
        req->attributes = ObjectAttributes ? ObjectAttributes->Attributes : 0;
        req->rootdir = ObjectAttributes ? ObjectAttributes->RootDirectory : 0;
        if (ObjectAttributes && ObjectAttributes->ObjectName)
        {
            req->name_len = ObjectAttributes->ObjectName->Length;
            wine_server_add_data(req, ObjectAttributes->ObjectName->Buffer,
                                 ObjectAttributes->ObjectName->Length);
        }
        else
            req->name_len = 0;
        wine_server_add_data(req, TargetName->Buffer, TargetName->Length);
        ret = wine_server_call( req );
        *SymbolicLinkHandle = reply->handle;
    }
    SERVER_END_REQ;
    return ret;
}

/******************************************************************************
 *  NtQuerySymbolicLinkObject	[NTDLL.@]
 *  ZwQuerySymbolicLinkObject	[NTDLL.@]
 *
 * Query a namespace symbolic link object target name.
 * 
 * PARAMS
 *  LinkHandle     [I] Handle to a symbolic link object
 *  LinkTarget     [O] Destination for the symbolic link target
 *  ReturnedLength [O] Size of returned data
 *
 * RETURNS
 *  Success: ERROR_SUCCESS.
 *  Failure: An NTSTATUS error code.
 */
NTSTATUS WINAPI NtQuerySymbolicLinkObject(IN HANDLE LinkHandle, IN OUT PUNICODE_STRING LinkTarget,
                                          OUT PULONG ReturnedLength OPTIONAL)
{
    NTSTATUS ret;
    TRACE("(%p,%p,%p)\n", LinkHandle, LinkTarget, ReturnedLength);

    __asm__ __volatile__ (
            "movl $0x8C,%%eax\n\t"
            "lea 8(%%ebp),%%edx\n\t"
            "int $0x2E\n\t"
            :"=a" (ret)
            );
    return ret;
}

/******************************************************************************
 *  NtAllocateUuids   [NTDLL.@]
 */
NTSTATUS WINAPI NtAllocateUuids(
        PULARGE_INTEGER Time,
        PULONG Range,
        PULONG Sequence)
{
    FIXME("(%p,%p,%p), stub.\n", Time, Range, Sequence);
	return 0;
}

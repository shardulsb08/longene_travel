/*
 * file.h
 *
 * Copyright (C) 2006  Insigme Co., Ltd
 *
 * This software has been developed while working on the Linux Unified Kernel
 * project (http://www.longene.org) in the Insigma Research Institute,  
 * which is a subdivision of Insigma Co., Ltd (http://www.insigma.com.cn).
 * 
 * The project is sponsored by Insigma Co., Ltd.
 *
 * The authors can be reached at linux@insigma.com.cn.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of  the GNU General  Public License as published by the
 * Free Software Foundation; either version 2 of the  License, or (at your
 * option) any later version.
 *
 * Revision History:
 *   Jan 2006 - Created.
 */
 
/*
 * file.h: win32 file definition
 * Refered to Kernel-win32 code
 */

#ifndef _FILE_H
#define _FILE_H

#include <linux/module.h>
#include "object.h"
#include "winternl.h"
#include "win32.h"
#include "process.h"
#include "wineserver/file.h"

#ifdef CONFIG_UNIFIED_KERNEL
#define FILE_DEVICE_NAMED_PIPE          0x00000011
#define FILE_ANY_ACCESS                 0
#define METHOD_BUFFERED                 0

#define CTL_CODE(DeviceType, Function, Method, Access) (              \
                (DWORD)((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) \
                )

#define FILE_OPEN			0x00000001
#define FILE_CREATE			0x00000002
#define FILE_OPEN_IF			0x00000003
#define FILE_OVERWRITE			0x00000004
#define FILE_OVERWRITE_IF		0x00000005

#define FILE_FLAG_WRITE_THROUGH		0x80000000
#define FILE_FLAG_OVERLAPPED		0x40000000
#define FILE_FLAG_NO_BUFFERING		0x20000000
#define FILE_FLAG_RANDOM_ACCESS		0x10000000
#define FILE_FLAG_SEQUENTIAL_SCAN	0x08000000
#define FILE_FLAG_DELETE_ON_CLOSE	0x04000000
#define FILE_FLAG_BACKUP_SEMANTICS	0x02000000
#define FILE_FLAG_POSIX_SEMANTICS	0x01000000
#define FILE_FLAG_OPEN_REPARSE_POINT	0x00200000
#define FILE_FLAG_OPEN_NO_RECALL	0x00100000

#define FILE_ATTRIBUTE_READONLY         0x00000001L
#define FILE_ATTRIBUTE_HIDDEN           0x00000002L
#define FILE_ATTRIBUTE_SYSTEM           0x00000004L
#define FILE_ATTRIBUTE_LABEL            0x00000008L  /* Not in Windows API */
#define FILE_ATTRIBUTE_DIRECTORY        0x00000010L
#define FILE_ATTRIBUTE_ARCHIVE          0x00000020L
#define FILE_ATTRIBUTE_NORMAL           0x00000080L
#define FILE_ATTRIBUTE_TEMPORARY        0x00000100L
#define FILE_ATTRIBUTE_ATOMIC_WRITE     0x00000200L
#define FILE_ATTRIBUTE_XACTION_WRITE    0x00000400L
#define FILE_ATTRIBUTE_COMPRESSED       0x00000800L
#define FILE_ATTRIBUTE_OFFLINE		0x00001000L

#define CREATE_NEW			1
#define CREATE_ALWAYS			2
#define OPEN_EXISTING			3
#define OPEN_ALWAYS			4
#define TRUNCATE_EXISTING		5

#define GENERIC_READ			0x80000000L
#define GENERIC_WRITE			0x40000000L
#define GENERIC_EXECUTE			0x20000000L
#define GENERIC_ALL			0x10000000L

#define FILE_SHARE_READ			0x00000001
#define FILE_SHARE_WRITE		0x00000002
#define FILE_SHARE_DELETE		0x00000004

struct file_args {
	IN ACCESS_MASK	DesiredAccess;
	IN POBJECT_ATTRIBUTES	ObjectAttributes;
	OUT PIO_STATUS_BLOCK	IoStatusBlock;
	IN PLARGE_INTEGER	AllocationSize OPTIONAL;
	IN ULONG	FileAttributes;
	IN ULONG	ShareAccess;
	IN ULONG	CreateDisposition;
	IN ULONG	CreateOptions;
	IN PVOID	EaBuffer OPTIONAL;
	IN ULONG	EaLength;
};

typedef struct win32_file_ctrl win32_file_ctrl;

/*
 * process-access file object definition
 */
struct win32_file {
	CSHORT  Type;
	CSHORT  Size;
	PVOID  DeviceObject;
	/* PDEVICE_OBJECT  DeviceObject; */
	PVOID  Vpb;
	/*  PVPB  Vpb; */
	PVOID  FsContext;
	PVOID  FsContext2;
	PVOID  SectionObjectPointer;
	/*  PSECTION_OBJECT_POINTERS  SectionObjectPointer; */
	PVOID  PrivateCacheMap;
	NTSTATUS  FinalStatus;
	struct win32_file  *RelatedFileObject;
	BOOLEAN  LockOperation;
	BOOLEAN  DeletePending;
	BOOLEAN  ReadAccess;
	BOOLEAN  WriteAccess;
	BOOLEAN  DeleteAccess;
	BOOLEAN  SharedRead;
	BOOLEAN  SharedWrite;
	BOOLEAN  SharedDelete;
	ULONG  Flags;
	UNICODE_STRING  FileName;
	LARGE_INTEGER  CurrentByteOffset;
	ULONG  Waiters;
	ULONG  Busy;
	PVOID  LastLock;
	struct kevent  Lock;
	struct kevent  Event;
	PVOID CompletionContext;
	/*  PIO_COMPLETION_CONTEXT  CompletionContext; */

	struct list_head	wf_ctllist;	/* file control list next */
	win32_file_ctrl		*wf_control;	/* the file control object */

	int                     wf_fd;
	int                     wf_doc; /* delete on close, no longer needed, into Flags */
	char                    wf_unlink_name[MAX_PATH];

	/* wf_file is considered deprecated, don't use it
	 * use wf_fd instead
	 * */
	struct file		*wf_file; 	/* the Linux file */
	__u32			wf_access;	/* file access mode */
	__u32			wf_sharing;	/* sharing mode */
	__u32			wf_attrs;	/* file open attributes */
};

/*
 * file-control object definition
 * - links together win32_file objects that use the same file
 */
struct win32_file_ctrl {
	struct list_head	wfc_accessors;	/* who's accessing this file */
	spinlock_t		wfc_lock;	/* govern access to list */
};


#define FILE_NOTIFY_CHANGE_FILE_NAME	0x00000001
#define FILE_NOTIFY_CHANGE_DIR_NAME	0x00000002
#define FILE_NOTIFY_CHANGE_NAME		0x00000003
#define FILE_NOTIFY_CHANGE_ATTRIBUTES	0x00000004
#define FILE_NOTIFY_CHANGE_SIZE		0x00000008
#define FILE_NOTIFY_CHANGE_LAST_WRITE	0x00000010
#define FILE_NOTIFY_CHANGE_LAST_ACCESS	0x00000020
#define FILE_NOTIFY_CHANGE_CREATION	0x00000040
#define FILE_NOTIFY_CHANGE_EA		0x00000080
#define FILE_NOTIFY_CHANGE_SECURITY	0x00000100
#define FILE_NOTIFY_CHANGE_STREAM_NAME	0x00000200
#define FILE_NOTIFY_CHANGE_STREAM_SIZE	0x00000400
#define FILE_NOTIFY_CHANGE_STREAM_WRITE	0x00000800
#define FILE_NOTIFY_VALID_MASK		0x00000fff
#define FILE_NOTIFY_ALL                 (FILE_NOTIFY_VALID_MASK)

typedef struct _FILE_NOTIFY_INFORMATION {
	DWORD NextEntryOffset;
	DWORD Action;
	DWORD FileNameLength;
	WCHAR FileName[1];
} FILE_NOTIFY_INFORMATION,*PFILE_NOTIFY_INFORMATION;


NTSTATUS
SERVICECALL
NtCreateFile(
		OUT PHANDLE FileHandle,
		IN ACCESS_MASK DesiredAccess,
		IN POBJECT_ATTRIBUTES ObjectAttributes,
		OUT PIO_STATUS_BLOCK IoStatusBlock,
		IN PLARGE_INTEGER AllocationSize OPTIONAL,
		IN ULONG FileAttributes,
		IN ULONG ShareAccess,
		IN ULONG CreateDisposition,
		IN ULONG CreateOptions,
		IN PVOID EaBuffer OPTIONAL,
		IN ULONG EaLength);

NTSTATUS
SERVICECALL
NtOpenFile(
		OUT PHANDLE FileHandle,
		IN ACCESS_MASK DesiredAccess,
		IN POBJECT_ATTRIBUTES ObjectAttributes,
		OUT PIO_STATUS_BLOCK IoStatusBlock,
		IN ULONG ShareAccess,
		IN ULONG OpenOptions
		);

NTSTATUS
SERVICECALL
NtReadFile(IN HANDLE FileHandle,
		IN HANDLE Event OPTIONAL,
		IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
		IN PVOID ApcContext OPTIONAL,
		OUT PIO_STATUS_BLOCK IoStatusBlock,
		OUT PVOID Buffer,
		IN ULONG Length,
		IN PLARGE_INTEGER ByteOffset OPTIONAL, /* NOT optional for asynch. operations! */
		IN PULONG Key OPTIONAL);

NTSTATUS
SERVICECALL
NtWriteFile (IN HANDLE FileHandle,
             IN HANDLE Event OPTIONAL,
             IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
             IN PVOID ApcContext OPTIONAL,
             OUT PIO_STATUS_BLOCK IoStatusBlock,
             IN PVOID Buffer,
             IN ULONG Length,
             IN PLARGE_INTEGER ByteOffset OPTIONAL, /* NOT optional for asynch. operations! */
             IN PULONG Key OPTIONAL);

NTSTATUS SERVICECALL
NtSetInformationFile(HANDLE FileHandle,
                     PIO_STATUS_BLOCK IoStatusBlock,
                     PVOID FileInformation,
                     ULONG Length,
                     FILE_INFORMATION_CLASS FileInformationClass);

NTSTATUS SERVICECALL
NtQueryInformationFile(HANDLE FileHandle,
                     PIO_STATUS_BLOCK IoStatusBlock,
                     PVOID FileInformation,
                     ULONG Length,
                     FILE_INFORMATION_CLASS FileInformationClass);

NTSTATUS SERVICECALL
NtFlushBuffersFile(IN HANDLE FileHandle,
		 OUT PIO_STATUS_BLOCK IoStatusBlock);

NTSTATUS
SERVICECALL
NtFsControlFile(
        IN HANDLE DeviceHandle,
        IN HANDLE Event OPTIONAL,
        IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
        IN PVOID ApcContext OPTIONAL,
        OUT PIO_STATUS_BLOCK IoStatusBlock,
        IN ULONG IoControlCode,
        IN PVOID InputBuffer,
        IN ULONG InputBufferSize,
        OUT PVOID OutputBuffer,
        IN ULONG OutputBufferSize);


#define FSCTL_REQUEST_OPLOCK_LEVEL_1    CTL_CODE(FILE_DEVICE_FILE_SYSTEM,  0, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_REQUEST_OPLOCK_LEVEL_2    CTL_CODE(FILE_DEVICE_FILE_SYSTEM,  1, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_REQUEST_BATCH_OPLOCK      CTL_CODE(FILE_DEVICE_FILE_SYSTEM,  2, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_OPLOCK_BREAK_ACKNOWLEDGE  CTL_CODE(FILE_DEVICE_FILE_SYSTEM,  3, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_OPBATCH_ACK_CLOSE_PENDING CTL_CODE(FILE_DEVICE_FILE_SYSTEM,  4, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_OPLOCK_BREAK_NOTIFY       CTL_CODE(FILE_DEVICE_FILE_SYSTEM,  5, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_LOCK_VOLUME               CTL_CODE(FILE_DEVICE_FILE_SYSTEM,  6, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_UNLOCK_VOLUME             CTL_CODE(FILE_DEVICE_FILE_SYSTEM,  7, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DISMOUNT_VOLUME           CTL_CODE(FILE_DEVICE_FILE_SYSTEM,  8, METHOD_BUFFERED, FILE_ANY_ACCESS)
/* decommissioned fsctl value                                              9                                     */
#define FSCTL_IS_VOLUME_MOUNTED         CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 10, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_IS_PATHNAME_VALID         CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 11, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_MARK_VOLUME_DIRTY         CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 12, METHOD_BUFFERED, FILE_ANY_ACCESS)
/* decommissioned fsctl value                                             13                                     */
#define FSCTL_QUERY_RETRIEVAL_POINTERS  CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 14,  METHOD_NEITHER, FILE_ANY_ACCESS)
#define FSCTL_GET_COMPRESSION           CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 15, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_SET_COMPRESSION           CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 16, METHOD_BUFFERED, \
						 FILE_READ_DATA | FILE_WRITE_DATA)
/* decommissioned fsctl value                                             17                                     */
/* decommissioned fsctl value                                             18                                     */
#define FSCTL_MARK_AS_SYSTEM_HIVE       CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 19,  METHOD_NEITHER, FILE_ANY_ACCESS)
#define FSCTL_OPLOCK_BREAK_ACK_NO_2     CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 20, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_INVALIDATE_VOLUMES        CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 21, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_QUERY_FAT_BPB             CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 22, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_REQUEST_FILTER_OPLOCK     CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 23, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_FILESYSTEM_GET_STATISTICS CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 24, METHOD_BUFFERED, FILE_ANY_ACCESS)
/* Start: _WIN32_WINNT >= 0x0400 */
#define FSCTL_GET_NTFS_VOLUME_DATA      CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 25, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_GET_NTFS_FILE_RECORD      CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 26, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_GET_VOLUME_BITMAP         CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 27,  METHOD_NEITHER, FILE_ANY_ACCESS)
#define FSCTL_GET_RETRIEVAL_POINTERS    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 28,  METHOD_NEITHER, FILE_ANY_ACCESS)
#define FSCTL_MOVE_FILE                 CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 29, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_IS_VOLUME_DIRTY           CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 30, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_GET_HFS_INFORMATION       CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 31, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_ALLOW_EXTENDED_DASD_IO    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 32, METHOD_NEITHER,  FILE_ANY_ACCESS)
/* End: _WIN32_WINNT >= 0x0400 */

/* Start: _WIN32_WINNT >= 0x0500 */
#define FSCTL_READ_PROPERTY_DATA        CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 33, METHOD_NEITHER, FILE_ANY_ACCESS)
#define FSCTL_WRITE_PROPERTY_DATA       CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 34, METHOD_NEITHER, FILE_ANY_ACCESS)
#define FSCTL_FIND_FILES_BY_SID         CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 35, METHOD_NEITHER, FILE_ANY_ACCESS)
/* decommissioned fsctl value                                             36                                     */
#define FSCTL_DUMP_PROPERTY_DATA        CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 37,  METHOD_NEITHER, FILE_ANY_ACCESS)
#define FSCTL_SET_OBJECT_ID             CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 38, METHOD_BUFFERED, FILE_WRITE_DATA)
#define FSCTL_GET_OBJECT_ID             CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 39, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DELETE_OBJECT_ID          CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 40, METHOD_BUFFERED, FILE_WRITE_DATA)
#define FSCTL_SET_REPARSE_POINT         CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 41, METHOD_BUFFERED, FILE_WRITE_DATA)
#define FSCTL_GET_REPARSE_POINT         CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 42, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DELETE_REPARSE_POINT      CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 43, METHOD_BUFFERED, FILE_WRITE_DATA)
#define FSCTL_ENUM_USN_DATA             CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 44,  METHOD_NEITHER, FILE_READ_DATA)
#define FSCTL_SECURITY_ID_CHECK         CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 45,  METHOD_NEITHER, FILE_READ_DATA)
#define FSCTL_READ_USN_JOURNAL          CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 46,  METHOD_NEITHER, FILE_READ_DATA)
#define FSCTL_SET_OBJECT_ID_EXTENDED    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 47, METHOD_BUFFERED, FILE_WRITE_DATA
#define FSCTL_CREATE_OR_GET_OBJECT_ID   CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 48, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_SET_SPARSE                CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 49, METHOD_BUFFERED, FILE_WRITE_DATA)
#define FSCTL_SET_ZERO_DATA             CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 50, METHOD_BUFFERED, FILE_WRITE_DATA)
#define FSCTL_QUERY_ALLOCATED_RANGES    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 51,  METHOD_NEITHER, FILE_READ_DATA)
#define FSCTL_ENABLE_UPGRADE            CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 52, METHOD_BUFFERED, FILE_WRITE_DATA)
#define FSCTL_SET_ENCRYPTION            CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 53, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_ENCRYPTION_FSCTL_IO       CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 54,  METHOD_NEITHER, FILE_ANY_ACCESS)
#define FSCTL_WRITE_RAW_ENCRYPTED       CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 55,  METHOD_NEITHER, FILE_ANY_ACCESS)
#define FSCTL_READ_RAW_ENCRYPTED        CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 56,  METHOD_NEITHER, FILE_ANY_ACCESS)
#define FSCTL_CREATE_USN_JOURNAL        CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 57,  METHOD_NEITHER, FILE_READ_DATA)
#define FSCTL_READ_FILE_USN_DATA        CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 58,  METHOD_NEITHER, FILE_READ_DATA)
#define FSCTL_WRITE_USN_CLOSE_RECORD    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 59,  METHOD_NEITHER, FILE_READ_DATA)
#define FSCTL_EXTEND_VOLUME             CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 60, METHOD_BUFFERED, FILE_ANY_ACCESS)
/* End: _WIN32_WINNT >= 0x0500 */

#define FSCTL_PIPE_ASSIGN_EVENT         CTL_CODE(FILE_DEVICE_NAMED_PIPE, 0, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_PIPE_DISCONNECT           CTL_CODE(FILE_DEVICE_NAMED_PIPE, 1, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_PIPE_LISTEN               CTL_CODE(FILE_DEVICE_NAMED_PIPE, 2, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_PIPE_PEEK                 CTL_CODE(FILE_DEVICE_NAMED_PIPE, 3, METHOD_BUFFERED, FILE_READ_DATA)
#define FSCTL_PIPE_QUERY_EVENT          CTL_CODE(FILE_DEVICE_NAMED_PIPE, 4, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_PIPE_TRANSCEIVE           CTL_CODE(FILE_DEVICE_NAMED_PIPE, 5, METHOD_NEITHER, \
						 FILE_READ_DATA | FILE_WRITE_DATA)
#define FSCTL_PIPE_WAIT                 CTL_CODE(FILE_DEVICE_NAMED_PIPE, 6, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_PIPE_IMPERSONATE          CTL_CODE(FILE_DEVICE_NAMED_PIPE, 7, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_PIPE_SET_CLIENT_PROCESS   CTL_CODE(FILE_DEVICE_NAMED_PIPE, 8, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_PIPE_QUERY_CLIENT_PROCESS CTL_CODE(FILE_DEVICE_NAMED_PIPE, 9, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_PIPE_INTERNAL_READ        CTL_CODE(FILE_DEVICE_NAMED_PIPE, 2045, METHOD_BUFFERED, FILE_READ_DATA)
#define FSCTL_PIPE_INTERNAL_WRITE       CTL_CODE(FILE_DEVICE_NAMED_PIPE, 2046, METHOD_BUFFERED, FILE_WRITE_DATA)
#define FSCTL_PIPE_INTERNAL_TRANSCEIVE  CTL_CODE(FILE_DEVICE_NAMED_PIPE, 2047, METHOD_NEITHER, \
						 FILE_READ_DATA | FILE_WRITE_DATA)
#define FSCTL_PIPE_INTERNAL_READ_OVFLOW CTL_CODE(FILE_DEVICE_NAMED_PIPE, 2048, METHOD_BUFFERED, FILE_READ_DATA)

#define IOCTL_STORAGE_BASE FILE_DEVICE_MASS_STORAGE
#define IOCTL_STORAGE_CHECK_VERIFY       CTL_CODE(IOCTL_STORAGE_BASE, 0x0200, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_STORAGE_MEDIA_REMOVAL      CTL_CODE(IOCTL_STORAGE_BASE, 0x0201, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_STORAGE_EJECT_MEDIA        CTL_CODE(IOCTL_STORAGE_BASE, 0x0202, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_STORAGE_LOAD_MEDIA         CTL_CODE(IOCTL_STORAGE_BASE, 0x0203, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_STORAGE_RESERVE            CTL_CODE(IOCTL_STORAGE_BASE, 0x0204, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_STORAGE_RELEASE            CTL_CODE(IOCTL_STORAGE_BASE, 0x0205, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_STORAGE_FIND_NEW_DEVICES   CTL_CODE(IOCTL_STORAGE_BASE, 0x0206, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_STORAGE_EJECTION_CONTROL   CTL_CODE(IOCTL_STORAGE_BASE, 0x0250, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_STORAGE_MCN_CONTROL        CTL_CODE(IOCTL_STORAGE_BASE, 0x0251, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_STORAGE_GET_MEDIA_TYPES    CTL_CODE(IOCTL_STORAGE_BASE, 0x0300, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_STORAGE_GET_MEDIA_TYPES_EX CTL_CODE(IOCTL_STORAGE_BASE, 0x0301, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_STORAGE_RESET_BUS          CTL_CODE(IOCTL_STORAGE_BASE, 0x0400, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_STORAGE_RESET_DEVICE       CTL_CODE(IOCTL_STORAGE_BASE, 0x0401, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_STORAGE_GET_DEVICE_NUMBER  CTL_CODE(IOCTL_STORAGE_BASE, 0x0420, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_DISK_BASE                 FILE_DEVICE_DISK
#define IOCTL_DISK_GET_DRIVE_GEOMETRY   CTL_CODE(IOCTL_DISK_BASE, 0x0000, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DISK_GET_PARTITION_INFO   CTL_CODE(IOCTL_DISK_BASE, 0x0001, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_DISK_SET_PARTITION_INFO   CTL_CODE(IOCTL_DISK_BASE, 0x0002, METHOD_BUFFERED, \
						 FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_DISK_GET_DRIVE_LAYOUT     CTL_CODE(IOCTL_DISK_BASE, 0x0003, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_DISK_SET_DRIVE_LAYOUT     CTL_CODE(IOCTL_DISK_BASE, 0x0004, METHOD_BUFFERED, \
	       					 FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_DISK_VERIFY               CTL_CODE(IOCTL_DISK_BASE, 0x0005, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DISK_FORMAT_TRACKS        CTL_CODE(IOCTL_DISK_BASE, 0x0006, METHOD_BUFFERED, \
						 FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_DISK_REASSIGN_BLOCKS      CTL_CODE(IOCTL_DISK_BASE, 0x0007, METHOD_BUFFERED, \
						 FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_DISK_PERFORMANCE          CTL_CODE(IOCTL_DISK_BASE, 0x0008, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DISK_IS_WRITABLE          CTL_CODE(IOCTL_DISK_BASE, 0x0009, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DISK_LOGGING              CTL_CODE(IOCTL_DISK_BASE, 0x000a, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DISK_FORMAT_TRACKS_EX     CTL_CODE(IOCTL_DISK_BASE, 0x000b, METHOD_BUFFERED, \
						 FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_DISK_HISTOGRAM_STRUCTURE  CTL_CODE(IOCTL_DISK_BASE, 0x000c, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DISK_HISTOGRAM_DATA       CTL_CODE(IOCTL_DISK_BASE, 0x000d, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DISK_HISTOGRAM_RESET      CTL_CODE(IOCTL_DISK_BASE, 0x000e, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DISK_REQUEST_STRUCTURE    CTL_CODE(IOCTL_DISK_BASE, 0x000f, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DISK_REQUEST_DATA         CTL_CODE(IOCTL_DISK_BASE, 0x0010, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DISK_CHECK_VERIFY         CTL_CODE(IOCTL_DISK_BASE, 0x0200, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_DISK_MEDIA_REMOVAL        CTL_CODE(IOCTL_DISK_BASE, 0x0201, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_DISK_EJECT_MEDIA          CTL_CODE(IOCTL_DISK_BASE, 0x0202, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_DISK_LOAD_MEDIA           CTL_CODE(IOCTL_DISK_BASE, 0x0203, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_DISK_RESERVE              CTL_CODE(IOCTL_DISK_BASE, 0x0204, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_DISK_RELEASE              CTL_CODE(IOCTL_DISK_BASE, 0x0205, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_DISK_FIND_NEW_DEVICES     CTL_CODE(IOCTL_DISK_BASE, 0x0206, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_DISK_GET_MEDIA_TYPES      CTL_CODE(IOCTL_DISK_BASE, 0x0300, METHOD_BUFFERED, FILE_ANY_ACCESS)


/* Start: For _WIN32_WINNT >= 0x0400 */
#define IOCTL_DISK_CONTROLLER_NUMBER    CTL_CODE(IOCTL_DISK_BASE, 0x0011, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define SMART_GET_VERSION               CTL_CODE(IOCTL_DISK_BASE, 0x0020, METHOD_BUFFERED, FILE_READ_ACCESS)
#define SMART_SEND_DRIVE_COMMAND        CTL_CODE(IOCTL_DISK_BASE, 0x0021, METHOD_BUFFERED, \
						 FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define SMART_RCV_DRIVE_DATA            CTL_CODE(IOCTL_DISK_BASE, 0x0022, METHOD_BUFFERED, \
						 FILE_READ_ACCESS | FILE_WRITE_ACCESS)
/* End: For _WIN32_WINNT >= 0x0400 */

#define IOCTL_SERIAL_LSRMST_INSERT      CTL_CODE(FILE_DEVICE_SERIAL_PORT,31,METHOD_BUFFERED,FILE_ANY_ACCESS)

static inline int get_fd_from_fo (const struct win32_file *fo)
{
    return fo->wf_fd;
}

#endif /* CONFIG_UNIFIED_KERNEL */
#endif /* _FILE_H */

#include <ntifs.h>
#include <ntddk.h>

NTSTATUS ntCreateFile(WCHAR *szFileName);
NTSTATUS ntCreateDirectory(WCHAR *szDirName);

NTSTATUS ntWriteFile(WCHAR *szFileName);
NTSTATUS ntReadFile(WCHAR *szFileName);
NTSTATUS ntCopyFile(const WCHAR *src, const WCHAR *dst);
NTSTATUS ntMoveFile(const WCHAR *src, const WCHAR *dst);
NTSTATUS ntDeleteFile(const WCHAR *szFileName);//删除 (设置属性的方式)
NTSTATUS ntNormalDeleteFile(const WCHAR *szFileName);//普通删除

ULONG ntGetFileAttributes(const WCHAR *szFileName);
NTSTATUS ntSetFileAttributes(WCHAR *szFileName);

VOID DriverUnload(PDRIVER_OBJECT pDriverObject)
{
	NTSTATUS ntStatus = 0;
	WCHAR *szFileName1 = L"\\??\\C:\\DriStu\\1.txt";//采用硬编码，写死文件，为了测试
	WCHAR *szFileName2 = L"\\??\\C:\\DriStu\\2.txt";//同上

	ntStatus = ntDeleteFile(szFileName1);
	if (!NT_SUCCESS(ntStatus))
	{
		DbgPrint("ntDeleteFile() failed %d \n", ntStatus);
	}
	ntStatus = ntDeleteFile(szFileName2);
	if (!NT_SUCCESS(ntStatus))
	{
		DbgPrint("ntDeleteFile() failed%ws,%x\n", szFileName2, ntStatus);
		return;
	}
	DbgPrint("Driver Unloaded\n");

}

NTSTATUS FileOper(VOID);

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegPath)
{
	DbgPrint("Driver begin\n");
	pDriverObject->DriverUnload = DriverUnload;
	FileOper();
	return STATUS_SUCCESS;
}

NTSTATUS FileOper(VOID)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;
	ULONG ulAttributes = 0;
	WCHAR *szDirName = L"\\??\\C:\\DriStu\\Mallocfree\\";
	WCHAR *szFileName1 = L"\\??\\C:\\DriStu\\1.txt";
	WCHAR *szFileName2 = L"\\??\\C:\\DriStu\\2.txt";
	WCHAR *szFileName3 = L"\\??\\C:\\DriStu\\Mallocfree\\3.txt";
	WCHAR *szFileName4 = L"\\??\\C:\\DriStu\\Mallocfree\\4.txt";
	ntStatus = ntCreateFile(szFileName1);
	if (!NT_SUCCESS(ntStatus))
	{
		DbgPrint("ntCreateFile() failed:%x\n", ntStatus);
		return ntStatus;
	}

	ntStatus = ntCreateDirectory(szDirName);
	if (!NT_SUCCESS(ntStatus))
	{
		DbgPrint("ntCreateDirectory() failed %x\n", ntStatus);
		return ntStatus;
	}

	ntStatus = ntWriteFile(szFileName1);
	if (!NT_SUCCESS(ntStatus))
	{
		DbgPrint("ntWriteFile() failed %x\n", ntStatus);
	}

	ntStatus = ntReadFile(szFileName1);
	if (!NT_SUCCESS(ntStatus))
	{
		DbgPrint("ntReadFile() failed:%x\n", ntStatus);
		return ntStatus;
	}

	ntStatus = ntCopyFile(szFileName1, szFileName2);
	if (!NT_SUCCESS(ntStatus))
	{
		DbgPrint("ntCopyFile() failed:%x\n", ntStatus);
		return ntStatus;
	}

	ntStatus = ntCopyFile(szFileName1, szFileName3);
	if (!NT_SUCCESS(ntStatus))
	{
		DbgPrint("ntCopyFile() failed:%x\n", ntStatus);
		return ntStatus;
	}

	ntStatus = ntMoveFile(szFileName1, szFileName4);
	if (!NT_SUCCESS(ntStatus))
	{
		DbgPrint("ntMoveFile() failed:%x\n", ntStatus);
		return ntStatus;
	}

	ulAttributes = ntGetFileAttributes(szFileName1);
	if (ulAttributes & FILE_ATTRIBUTE_DIRECTORY)
	{
		DbgPrint("%S is a directory\n", szFileName1);
	}
	else
	{
		DbgPrint("%S is not a directory\n", szFileName1);

	}

	ulAttributes = ntGetFileAttributes(szDirName);
	if (ulAttributes & FILE_ATTRIBUTE_DIRECTORY)
	{
		DbgPrint("%S is a directory\n", szDirName);
	}
	else
	{
		DbgPrint("%S is not a directory\n", szDirName);

	}

	return ntStatus;

}


NTSTATUS ntCreateFile(WCHAR *szFileName)
{
	OBJECT_ATTRIBUTES objAttrib = { 0 };
	UNICODE_STRING uFileName = { 0 };
	IO_STATUS_BLOCK io_status = { 0 };
	HANDLE hFile = NULL;
	NTSTATUS status = 0;

	RtlInitUnicodeString(&uFileName, szFileName);
	InitializeObjectAttributes(
		&objAttrib,
		&uFileName,
		OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
		NULL,
		NULL
	);

	status = ZwCreateFile(
		&hFile,
		GENERIC_WRITE,
		&objAttrib,
		&io_status,
		NULL,
		FILE_ATTRIBUTE_NORMAL,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		FILE_OPEN_IF,
		FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE,
		NULL,
		0);
	if (NT_SUCCESS(status))
	{
		ZwClose(hFile);
	}
	return status;
}

NTSTATUS ntCreateDirectory(WCHAR *szDirName)
{
	OBJECT_ATTRIBUTES objAttrib = { 0 };
	UNICODE_STRING uDirName = { 0 };
	IO_STATUS_BLOCK io_status = { 0 };
	HANDLE hFile = NULL;
	NTSTATUS status = 0;
	RtlInitUnicodeString(&uDirName, szDirName);
	InitializeObjectAttributes(
		&objAttrib,
		&uDirName,
		OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
		NULL,
		NULL);

	status = ZwCreateFile(
		&hFile,
		GENERIC_READ | GENERIC_WRITE,
		&objAttrib,
		&io_status,
		NULL,
		FILE_ATTRIBUTE_DIRECTORY,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		FILE_OPEN_IF,
		FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
		NULL,
		0
	);
	if (NT_SUCCESS(status))
	{
		ZwClose(hFile);
	}
	return status;
}

ULONG ntGetFileAttributes(const WCHAR *filename)
{
	ULONG dwRtn = 0;
	NTSTATUS ntStatus = STATUS_UNSUCCESSFUL;
	OBJECT_ATTRIBUTES objAttr = { 0 };
	UNICODE_STRING uName = { 0 };
	FILE_NETWORK_OPEN_INFORMATION info = { 0 };

	if (NULL == filename)
	{
		return ntStatus;
	}

	RtlInitUnicodeString(&uName, filename);
	RtlZeroMemory(&info, sizeof(FILE_NETWORK_OPEN_INFORMATION));

	InitializeObjectAttributes(
		&objAttr,
		&uName,
		OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
		NULL,
		NULL);

	ntStatus = ZwQueryFullAttributesFile(
		&objAttr,
		&info
	);
	if (NT_SUCCESS(ntStatus))
	{
		dwRtn = info.FileAttributes;
	}
	if (dwRtn&FILE_ATTRIBUTE_DIRECTORY)
	{
		DbgPrint("%S is a directory\n", filename);
	}
	return dwRtn;

}


NTSTATUS ntSetFileAttributes(WCHAR *szFileName)
{
	UNICODE_STRING uFileName = { 0 };
	OBJECT_ATTRIBUTES objectAttributes = { 0 };
	HANDLE hFile = NULL;
	NTSTATUS ntStatus = 0;
	IO_STATUS_BLOCK ioStatus = { 0 };
	FILE_STANDARD_INFORMATION fsi = { 0 };
	FILE_POSITION_INFORMATION fpi = { 0 };


	RtlInitUnicodeString(&uFileName, szFileName);

	InitializeObjectAttributes(
		&objectAttributes,
		&uFileName,
		OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
		NULL,
		NULL
	);

	ntStatus = ZwCreateFile(
		&hFile,
		GENERIC_READ,
		&objectAttributes,
		&ioStatus,
		NULL,
		FILE_ATTRIBUTE_NORMAL,
		0,
		FILE_OPEN,
		FILE_SYNCHRONOUS_IO_NONALERT,
		NULL,
		0
	);
	if (!NT_SUCCESS(ntStatus))
	{
		return ntStatus;
	}
	ntStatus = ZwQueryInformationFile(
		hFile,
		&ioStatus,
		&fsi,
		sizeof(FILE_STANDARD_INFORMATION),
		FileStandardInformation
	);
	if (!NT_SUCCESS(ntStatus))
	{
		ZwClose(hFile);
		return ntStatus;
	}

	fpi.CurrentByteOffset.QuadPart = 100i64;
	ntStatus = ZwSetInformationFile(
		hFile,
		&ioStatus,
		&fpi,
		sizeof(FILE_POSITION_INFORMATION),
		FilePositionInformation
	);

	ZwClose(hFile);
	return ntStatus;
}


NTSTATUS ntWriteFile(WCHAR *szFileName)
{
	UNICODE_STRING uFileName = { 0 };
	OBJECT_ATTRIBUTES objectAttributes = { 0 };
	NTSTATUS ntStatus = STATUS_SUCCESS;
	HANDLE hFile = NULL;
	IO_STATUS_BLOCK ioStatus = { 0 };
	PUCHAR pBuffer = NULL;

	RtlInitUnicodeString(&uFileName, szFileName);

	InitializeObjectAttributes(
		&objectAttributes,
		&uFileName,
		OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
		NULL,
		NULL
	);

	ntStatus = ZwCreateFile(
		&hFile,
		GENERIC_WRITE,
		&objectAttributes,
		&ioStatus,
		NULL,
		FILE_ATTRIBUTE_NORMAL,
		FILE_SHARE_WRITE,
		FILE_OPEN_IF,
		FILE_SYNCHRONOUS_IO_NONALERT,
		NULL,
		0
	);

	if (!NT_SUCCESS(ntStatus))
	{
		return ntStatus;
	}

	pBuffer = (PUCHAR)ExAllocatePoolWithTag(PagedPool, 1024, 'ELIF');
	if (NULL == pBuffer)
	{
		ZwClose(hFile);
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	RtlZeroMemory(pBuffer, 1024);

	RtlCopyMemory(pBuffer, L"Hello World!", wcslen(L"Hello World!") * sizeof(WCHAR));
	ntStatus = ZwWriteFile(
		hFile,
		NULL,
		NULL,
		NULL,
		&ioStatus,
		pBuffer,
		1024,
		NULL,
		NULL
	);
	ZwClose(hFile);
	ExFreePool(pBuffer);
	return ntStatus;

}

NTSTATUS ntReadFile(WCHAR *szFileName)
{

	NTSTATUS ntStatus = 0;
	UNICODE_STRING uFileName = { 0 };
	OBJECT_ATTRIBUTES objectAttributes = { 0 };
	IO_STATUS_BLOCK ioStatus = { 0 };
	HANDLE hFile = NULL;
	PUCHAR pBuffer = NULL;
	FILE_STANDARD_INFORMATION fsi = { 0 };

	RtlInitUnicodeString(&uFileName, szFileName);
	InitializeObjectAttributes(
		&objectAttributes,
		&uFileName,
		OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
		NULL,
		NULL
	);

	ntStatus = ZwCreateFile(
		&hFile,
		GENERIC_READ,
		&objectAttributes,
		&ioStatus,
		NULL,
		FILE_ATTRIBUTE_NORMAL,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		FILE_OPEN,
		FILE_SYNCHRONOUS_IO_NONALERT,
		NULL,
		0
	);
	if (!NT_SUCCESS(ntStatus))
	{
		return ntStatus;
	}
	ntStatus = ZwQueryInformationFile(
		hFile,
		&ioStatus,
		&fsi,
		sizeof(FILE_STANDARD_INFORMATION),
		FileStandardInformation
	);

	if (!NT_SUCCESS(ntStatus))
	{
		ZwClose(hFile);
		return ntStatus;
	}
	pBuffer = (PUCHAR)ExAllocatePoolWithTag(
		PagedPool, (LONG)fsi.EndOfFile.QuadPart, 'ELIF'
	);
	if (NULL == pBuffer)
	{
		ZwClose(hFile);
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	ntStatus = ZwReadFile(
		hFile,
		NULL,
		NULL,
		NULL,
		&ioStatus,
		pBuffer,
		(LONG)fsi.EndOfFile.QuadPart,
		NULL,
		NULL
	);
	ZwClose(hFile);
	ExFreePool(pBuffer);
	return ntStatus;
}

NTSTATUS ntCopyFile(const WCHAR *src, const WCHAR *dst)
{
	UNICODE_STRING uSrc = { 0 };
	UNICODE_STRING uDst = { 0 };
	NTSTATUS ntStatus = 0;
	HANDLE hSrcFile = NULL;
	HANDLE hDstFile = NULL;
	OBJECT_ATTRIBUTES objectSrcAttrib = { 0 };
	OBJECT_ATTRIBUTES objectDstAttrib = { 0 };
	ULONG uReadSize = 0;
	ULONG uWriteSize = 0;
	ULONG length = 0;
	PVOID pBuffer = NULL;
	IO_STATUS_BLOCK io_status = { 0 };
	LARGE_INTEGER offset = { 0 };

	RtlInitUnicodeString(&uSrc, src);
	RtlInitUnicodeString(&uDst, dst);

	InitializeObjectAttributes(
		&objectSrcAttrib,
		&uSrc,
		OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
		NULL,
		NULL
	);
	InitializeObjectAttributes(
		&objectDstAttrib,
		&uDst,
		OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
		NULL,
		NULL
	);

	ntStatus = ZwCreateFile(
		&hSrcFile,
		FILE_READ_DATA | FILE_READ_ATTRIBUTES,
		&objectSrcAttrib,
		&io_status,
		NULL,
		FILE_ATTRIBUTE_NORMAL,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		FILE_OPEN,
		FILE_SYNCHRONOUS_IO_NONALERT,
		NULL,
		0
	);
	if (!NT_SUCCESS(ntStatus))
	{
		return ntStatus;
	}

	ntStatus = ZwCreateFile(
		&hDstFile,
		GENERIC_WRITE,
		&objectDstAttrib,
		&io_status,
		NULL,
		FILE_ATTRIBUTE_NORMAL,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		FILE_OPEN_IF,
		FILE_SYNCHRONOUS_IO_NONALERT,
		NULL,
		0
	);
	if (!NT_SUCCESS(ntStatus))
	{
		ZwClose(hSrcFile);
		return ntStatus;
	}
	pBuffer = ExAllocatePoolWithTag(PagedPool, 1024, 'ELIF');
	if (NULL == pBuffer)
	{
		ZwClose(hSrcFile);
		ZwClose(hDstFile);
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	while (1)
	{
		ntStatus = ZwReadFile(
			hSrcFile,
			NULL,
			NULL,
			NULL,
			&io_status,
			pBuffer,
			PAGE_SIZE,
			&offset,
			NULL
		);
		if (!NT_SUCCESS(ntStatus))
		{
			if (STATUS_END_OF_FILE == ntStatus)
			{
				ntStatus = STATUS_SUCCESS;
			}
			break;
		}
		length = (ULONG)io_status.Information;

		ntStatus = ZwWriteFile(
			hDstFile,
			NULL,
			NULL,
			NULL,
			&io_status,
			pBuffer,
			length,
			&offset,
			NULL
		);
		if (!NT_SUCCESS(ntStatus))
		{
			break;
		}
		offset.QuadPart += length;

	}
	ExFreePool(pBuffer);
	ZwClose(hSrcFile);
	ZwClose(hDstFile);
	return ntStatus;

}

NTSTATUS ntMoveFile(const WCHAR *src, const WCHAR *dst)
{
	NTSTATUS ntStatus = 0;
	ntStatus = ntCopyFile(src, dst);
	if (NT_SUCCESS(ntStatus))
	{
		ntStatus = ntDeleteFile(src);
	}
	return ntStatus;
}


NTSTATUS ntNormalDeleteFile(const WCHAR *szFileName)//普通删除
{
	NTSTATUS ntStatus = 0;
	OBJECT_ATTRIBUTES objectAttributes = { 0 };
	UNICODE_STRING uFileName = { 0 };

	RtlInitUnicodeString(&uFileName, szFileName);
	InitializeObjectAttributes(
		&objectAttributes,
		&uFileName,
		OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
		NULL,
		NULL
	);
	ntStatus = ZwDeleteFile(&objectAttributes);
	return ntStatus;

}


//1.下面这种删除通过设置属性，ZwCreateFile设置Delete
//2.如果不成功，ZwCreateFile with FILE_READ_ATTRIBUTES  FILE_WRITE_ATTRIBUTES，转入3
//3.如果成功，FILE_ATTRIBUTE_NORMAL
//ZwSetInformationFile,以 SYNCHRONIZE | FILE_WRITE_DATA | DELETE 打开文件




NTSTATUS ntDeleteFile(const WCHAR *szFileName)//删除 (设置属性的方式)
{
	UNICODE_STRING uFileName = { 0 };
	OBJECT_ATTRIBUTES objectAttributes = { 0 };
	HANDLE hFile = NULL;
	IO_STATUS_BLOCK ioStatus = { 0 };
	NTSTATUS ntStatus = 0;
	FILE_DISPOSITION_INFORMATION disInfo = { 0 };

	RtlInitUnicodeString(&uFileName, szFileName);

	InitializeObjectAttributes(
		&objectAttributes,
		&uFileName,
		OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
		NULL,
		NULL
	);

	ntStatus = ZwCreateFile(
		&hFile,
		SYNCHRONIZE | FILE_WRITE_DATA | DELETE,
		&objectAttributes,
		&ioStatus,
		NULL,
		FILE_ATTRIBUTE_NORMAL,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		FILE_OPEN,
		FILE_SYNCHRONOUS_IO_NONALERT | FILE_DELETE_ON_CLOSE,
		NULL,
		0
	);
	if (!NT_SUCCESS(ntStatus))
	{
		if (STATUS_ACCESS_DENIED == ntStatus)
		{
			ntStatus = ZwCreateFile(
				&hFile,
				SYNCHRONIZE | FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES,
				&objectAttributes,
				&ioStatus,
				NULL,
				FILE_ATTRIBUTE_NORMAL,
				FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
				FILE_OPEN,
				FILE_SYNCHRONOUS_IO_NONALERT,
				NULL,
				0
			);

			if (NT_SUCCESS(ntStatus))
			{
				FILE_BASIC_INFORMATION basicInfo = { 0 };

				ntStatus = ZwQueryInformationFile(
					hFile,
					&ioStatus,
					&basicInfo,
					sizeof(basicInfo),
					FileBasicInformation
				);
				if (!NT_SUCCESS(ntStatus))
				{
					DbgPrint("ZwQueryInformationFile(%wZ) failed(%x)\n", &uFileName, ntStatus);

				}
				basicInfo.FileAttributes = FILE_ATTRIBUTE_NORMAL;
				//The file does not have other attributes set. This attribute is valid only if used alone.
				//我的理解是设置为默认，其他属性就都没了
				ntStatus = ZwSetInformationFile(
					hFile,
					&ioStatus,
					&basicInfo,
					sizeof(basicInfo),
					FileBasicInformation
				);
				if (!NT_SUCCESS(ntStatus))
				{
					DbgPrint("ZwSetInformationFile(%wZ) failed(%x)\n", &uFileName, ntStatus);

				}

				ZwClose(hFile);
				ntStatus = ZwCreateFile(
					&hFile,
					SYNCHRONIZE | FILE_WRITE_DATA | DELETE,
					&objectAttributes,
					&ioStatus,
					NULL,
					FILE_ATTRIBUTE_NORMAL,
					FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
					FILE_OPEN,
					FILE_SYNCHRONOUS_IO_NONALERT | FILE_DELETE_ON_CLOSE,
					NULL,
					0
				);

			}

		}

		if (!NT_SUCCESS(ntStatus))
		{
			DbgPrint("ZwCreateFile(%wZ) failed(%x)\n", &uFileName, ntStatus);
			return ntStatus;
		}
	}
	disInfo.DeleteFile = TRUE;
	ntStatus = ZwSetInformationFile(
		hFile,
		&ioStatus,
		&disInfo,
		sizeof(disInfo),
		FileDispositionInformation
	);
	if (!NT_SUCCESS(ntStatus))
	{
		DbgPrint("ZwSetInformationFile(%wZ) failed(%x)\n", &uFileName, ntStatus);
	}
	ZwClose(hFile);
	return ntStatus;
}
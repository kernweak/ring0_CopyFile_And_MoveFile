# ring0_CopyFile_And_MoveFile
0环文件操作，通过修改属性删除文件
删除大概操作如下
//1.下面这种删除通过设置属性，ZwCreateFile设置Delete
//2.如果不成功，ZwCreateFile with FILE_READ_ATTRIBUTES  FILE_WRITE_ATTRIBUTES，转入3
//3.如果成功，FILE_ATTRIBUTE_NORMAL
//ZwSetInformationFile,以 SYNCHRONIZE | FILE_WRITE_DATA | DELETE 打开文件


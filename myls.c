#include <stdlib.h>
#include <stdio.h>  
#include <sys/types.h>  
#include <dirent.h>  
#include <sys/stat.h>  
#include <pwd.h>  
#include <grp.h>  
#include <unistd.h>  
#include <string.h>  
#include "list.h"
#include <getopt.h>
#include <sys/ioctl.h>
#include <termios.h>



#define S_DEFULT		1
#define S_S				2
#define S_t				3

typedef struct fileinfo
{
	char *name;
	struct stat *info;
}FileInfo;

int LS_A = 0;
int LS_a = 0;
int LS_R = 0;
int LS_l = 0;
int LS_S = 0;
int LS_r = 0;
int LS_s = 0;
int LS_d = 0;

int file_length = 0;
int sort_method = S_DEFULT;
int indexdols1 = 0;
int max[100];



static struct option longOpts[] = {
	{ "locaton", 0, NULL, 'l' },
	{ "Aictionary",0 , NULL, 'A' },
	{ "aumber",0 , NULL, 'a' },
	{ "Reverse",0 , NULL, 'R' },
	{ "help", 0, NULL, 'h' },
	{ "Sile",0 ,NULL, 'S'},
	{ 0, 0, 0, 0 }
};


struct column_info
{
	int valid_len;
	int line_len;
	int *col_arr;
};
struct column_info *column_info;

int line_length;


int max_idx;
#define MIN_COLUMN_WIDTH    3



unsigned short int get_ws_col()
{
	struct winsize size;
	ioctl(STDIN_FILENO, TIOCGWINSZ, &size);
	return size.ws_col;
}


void init_column_info(void)
{
	int i;
	int allocate = 0;

	line_length = get_ws_col();
	max_idx = line_length / MIN_COLUMN_WIDTH;
	if (max_idx == 0)
		max_idx = 1;

	if (column_info == NULL)
	{
		column_info = malloc(sizeof(struct column_info) * max_idx);
		if (column_info == NULL) return ;
		allocate = 1;
	}

	for (i = 0; i < max_idx; ++i)
	{
		int j;

		column_info[i].valid_len = 1;
		column_info[i].line_len = (i + 1) * MIN_COLUMN_WIDTH;

		if (allocate)
			column_info[i].col_arr = (int *)malloc(sizeof(int) * (i + 1));
		if(column_info[i].col_arr)
		{
			for (j = 0; j <= i; ++j)
				column_info[i].col_arr[j] = MIN_COLUMN_WIDTH;
		}
	}
}

void indent(int from, int to)
{
	while (from < to)
	{
		putchar(' ');
		from++;
	}
}

void print_many_per_line(MyList *list)
{
	struct column_info *line_fmt;
	int filesno;            /* Index into files. */
	int row;                /* Current row. */
	int max_name_length;    /* Length of longest file name + frills. */
	int name_length;        /* Length of each file name + frills. */
	//	int pos;            /* Current character column. */
	int cols;            /* Number of files across. */
	int rows;            /* Maximum number of files down. */

	int max_cols;
	int files_index = list->length;
	init_column_info();
	FileInfo *fileinfo1 = NULL;
	fileinfo1 = (FileInfo *)malloc(sizeof(FileInfo)*files_index);
	if(fileinfo1 == NULL) return ;
	int i=0;
	MyNode *p = list->first;
	while(p)
	{
		FileInfo *file = (FileInfo *)p->data;
		fileinfo1[i].name = file->name ;
		fileinfo1[i].info = file->info;
		i++;
		p = p->next;
	}
	max_cols = max_idx > files_index ? files_index : max_idx;
	for (filesno = 0; filesno < files_index; ++filesno)
	{
		int i;
		name_length = strlen(fileinfo1[filesno].name);
		for (i = 0; i < max_cols; ++i)
		{
			if (column_info[i].valid_len)
			{
				int idx = filesno / ((files_index + i) / (i + 1));
				int real_length = name_length + (idx == i ? 0 : 2);
				if (real_length > column_info[i].col_arr[idx])
				{
					column_info[i].line_len += (real_length
							- column_info[i].col_arr[idx]);
					column_info[i].col_arr[idx] = real_length;
					column_info[i].valid_len = column_info[i].line_len < line_length;
				}
			}
		}

	}
	for (cols = max_cols; cols > 1; --cols)
	{
		if (column_info[cols - 1].valid_len)
			break;
	}

	line_fmt = &column_info[cols - 1];

	if(cols == 0) 
	{	
		free(fileinfo1);
		return ;
	}
	rows = files_index / cols + (files_index % cols != 0);

	for (row = 0; row < rows; row++)
	{
		int col = 0;
		filesno = row;
		int	pos = 0;

		while (1)
		{
			if(LS_s == 1) printf("%d ",(int) fileinfo1[filesno].info->st_blocks/2);
			if(S_ISDIR((fileinfo1[filesno].info)->st_mode))
			{
				printf("\033[1;34m");
				printf("%s", fileinfo1[filesno].name);
				printf("\033[0m");

			}
			else	if(S_ISCHR((fileinfo1[filesno].info)->st_mode))
			{
				printf("\033[1;33m");
				printf("%s", fileinfo1[filesno].name);
				printf("\033[0m");

			}
			else if(S_ISLNK((fileinfo1[filesno].info)->st_mode))
			{
				printf("\033[1;36m");
				printf("%s", fileinfo1[filesno].name);
				printf("\033[0m");

			}

			else if((fileinfo1[filesno].info)->st_mode & S_IXUSR)
			{
				printf("\033[1;32m");
				printf("%s", fileinfo1[filesno].name);
				printf("\033[0m");
			}

			else 
				printf("%s", fileinfo1[filesno].name);
			name_length = strlen(fileinfo1[filesno].name);
			max_name_length = line_fmt->col_arr[col++];

			filesno += rows;
			if (filesno >= files_index)
				break;

			indent(pos + name_length, pos + max_name_length);
			pos += max_name_length;
		}
		putchar('\n');
	}
	free(fileinfo1);
}



void help()
{
	FILE *fp = fopen("a.txt","r");
	if (fp)
	{
		char b[500];
		while(!feof(fp))
		{
			fgets(b,sizeof(b),fp);	
			printf("%s",b);
		}
		fclose(fp);
	}

}

void freeData(void * p)
{
	FileInfo *data = (FileInfo *)p;
	free(data->name);
	free(data->info);
	free(data);
}



int sortMethod_S(void *data1, void *data2)
{
	FileInfo *file1 = (FileInfo *)data1;
	FileInfo *file2 = (FileInfo *)data2;
	if((int)(file1->info->st_blocks)  > (int) (file2->info->st_blocks))
		return 1;
	return 0;
}

int sortMethod_t(void *data1, void *data2)
{
	FileInfo *file1 = (FileInfo *)data1;
	FileInfo *file2 = (FileInfo *)data2;
	if((long)(file1->info->st_mtime)  > (long) (file2->info->st_mtime))
		return 1;
	return 0;
}




int is_Englist(char *p)
{
	if( *p >='A' && *p <= 'Z')	
		return 1;
	else if( *p >= 'a' && *p <= 'z' )
		return 1;
	else if(*p >= '0' && *p <='9' )
		return 1;
	return 0;
}


int sort_Englist(char a, char b)
{
	double aa = (double) a;
	double bb = (double) b;
	if(a >= 'A' && a<= 'Z')
		aa += 'a' - 'A' + 0.5;
	if(b >= 'A' && b<='Z')
		bb += 'a' - 'A' +0.5;
	if(aa < bb) return -1;
	else if(aa == bb) return 0;
	else return 1;


}

//ls默认排序
int sortMethod_DEFULT(void *data1, void *data2)
{
	FileInfo *file1 = (FileInfo *)data1;
	FileInfo *file2 = (FileInfo *)data2;
	int length1 = strlen(file1->name);
	int length2 = strlen(file2->name);
	int i=0,j=0;
	int point1=0,point2=0;
	while(i<length1 && j<length2)
	{
		if(is_Englist(&file1->name[i])==0  )
		{
			i++;
			point1++;		
		}
		if(is_Englist(&file2->name[j]) == 0)
		{
			point2++;
			j++;
		}
		if(is_Englist(&file1->name[i])==1  && is_Englist(&file2->name[j]) == 1)
		{
			if(sort_Englist(file1->name[i],file2->name[j])== -1)
				return 1;
			else if(sort_Englist(file1->name[i],file2->name[j]) == 1)
				return 0;
			else 
			{
				i++;j++;
			}
		}
	}
	//123.txt.	与 123tx...t
	while(i < length1)
	{
		if(is_Englist(&file1->name[i])==0) 
		{
			point1++;
			i++;
		}

		else 
			break;
	}
	while(j < length2)
	{
		if(is_Englist(&file2->name[j]) ==0)
		{
			point2++;
			j++;
		}
		else 
			break;
	}
	if(i == length1 && j<length2)
		return 1;
	else if(j== length2 && i<length1)
		return 0;
	else
	{
		if(point1 == 0) 
			return 1;
		else if(point2 == 0)
			return 0;
		i=0;j=0;
		while(i<length1 && j<length2)
		{
			if(is_Englist(&file1->name[i])==0 && is_Englist(&file2->name[j])==1)
				return 1;
			else if (is_Englist(&file1->name[i])==1 && is_Englist(&file2->name[j]) == 0)
				return 0;
			else if(is_Englist(&file1->name[i])==1 && is_Englist(&file2->name[j]) == 1)
			{
				i++;j++;
			}
			else
			{
				if(file1->name[i]<file2->name[j])
					return 0;
				else if(file1->name[i]>file2->name[j])
					return 1;
				else 
				{
					i++;j++;
				}
			}
		}
	}
	if(i == length1 && j<length2)
		return 1;
	else
		return 0;
}








// 展示单个文件的详细信息  
void show_file_info(char* filename, struct stat* info_p)  
{ 

	char* uid_to_name(), *ctime(), *gid_to_name(), *filemode();  
	void mode_to_letters();  
	char modestr[11];  
	mode_to_letters(info_p->st_mode, modestr);  
	if(LS_s == 1)
		printf("%*d ",max[0] ,(int) info_p->st_blocks/2);

	printf("%s", modestr);  
	printf(" %*d",max[1], (int) info_p->st_nlink);  
	printf(" %-*s",max[2], uid_to_name(info_p->st_uid));  
	printf(" %-*s",max[3], gid_to_name(info_p->st_gid));  
	printf(" %*ld",max[4], (long) info_p->st_size);  
	printf(" %1.12s", 4  + ctime(&info_p->st_mtime)); 
	if(modestr[0]=='d')
	{
		printf("\033[1;34m");
		printf(" %*s\n",max[6], filename);
		printf("\033[0m");
	}
	else if(modestr[0] == 'c')
	{
		printf("\033[1;33m");
		printf(" %*s\n",max[6], filename);
		printf("\033[0m");

	}
	else if(modestr[0] == 'l')
	{
		printf("\033[1;36m");
		printf(" %*s\n",max[6], filename);
		printf("\033[0m");

	}
	else
	{
		if(modestr[9] == 'x')
		{
			printf("\033[1;32m");
			printf(" %*s\n",max[6], filename);
			printf("\033[0m");
		}
		else
		{
			printf(" %*s\n",max[6], filename);
		}	
	}
}  

void mode_to_letters(int mode, char str[])  
{  
	strcpy(str, "----------");  
	if (S_ISDIR(mode))  
		str[0] = 'd';  
	else if (S_ISCHR(mode))  
		str[0] = 'c';  
	else if (S_ISLNK(mode))  
		str[0] = 'l';  
	else if(S_ISSOCK(mode))
		str[0] = 's';
	if ((mode & S_IRUSR))  
		str[1] = 'r';  
	if ((mode & S_IWUSR))  
		str[2] = 'w';  
	if ((mode & S_IXUSR))  
		str[3] = 'x';  
	if ((mode & S_IRGRP))  
		str[4] = 'r';  
	if ((mode & S_IWGRP))  
		str[5] = 'w';  
	if ((mode & S_IXGRP))  
		str[6] = 'x';  
	if ((mode & S_IROTH))  
		str[7] = 'r';  
	if ((mode & S_IWOTH))  
		str[8] = 'w';   
	if ((mode & S_IXOTH))  
		str[9] = 'x';  
}  

char* uid_to_name(uid_t uid)  
{  
	struct passwd* getpwuid(),* pw_ptr;  
	if((pw_ptr = getpwuid(uid)) == NULL)  
	{  
		static char numstr[10];  
		snprintf(numstr,sizeof(numstr),"%d",uid);  
		return numstr;  
	}  
	else  
	{  
		return pw_ptr->pw_name;  
	}  
}  

char* gid_to_name(gid_t gid)  
{  
	struct group* grp_ptr;  
	if(( grp_ptr = getgrgid(gid)) == NULL)  
	{  
		static char numstr[10]; 
		snprintf(numstr,sizeof(numstr),"%d",gid);  
		return numstr;  
	}  
	else  
	{  
		return grp_ptr->gr_name;  
	}  
}  



//打印文件名
void printData(const void* const p)
{
	const  FileInfo  *pp = (FileInfo *)p;
	if(S_ISDIR(pp->info->st_mode))
	{
		printf("\033[34m");
		printf("%s  ", pp->name);
		printf("\033[0m");
	}
	else if(S_ISCHR(pp->info->st_mode))
	{
		printf("\033[1;33m");
		printf("%s ", pp->name);
		printf("\033[0m");

	}
	else if(S_ISLNK(pp->info->st_mode))
	{
		printf("\033[1;36m");
		printf("%s ", pp->name);
		printf("\033[0m");

	}

	else
	{
		if(pp->info->st_mode & S_IXUSR)
		{
			printf("\033[32m");
			printf("%s  ",pp->name );
			printf("\033[0m");
		}
		else
		{
			printf("%s  ", pp->name);
		}	
	} 
}
//打印详细信息
void printInfo(const void* const p)
{

	const FileInfo *fileinfo = (FileInfo *)p;
	show_file_info(fileinfo->name,fileinfo->info);
}


void totalbloacks(MyList *list)
{
	MyNode *node = list->first;
	int total = 0;
	while(node)
	{
		FileInfo *file = (FileInfo *)node->data;
		total += file->info->st_blocks;
		node = node->next;
	}
	printf("总用量 %d\n",total/2);
}

void alllength(MyList *list)
{
	MyNode *p = list->first;
	for(int i=0; i<100; i++)
		max[i] = 0;
	//int colindex;
	char tmp[100];
	while(p)
	{

		int colindex = 0;
		FileInfo *file = (FileInfo *)p->data;
		sprintf(tmp,"%d",(int)file->info->st_blocks/2); 
		if(max[colindex] < strlen(tmp))
			max[colindex] = strlen(tmp);
		colindex++;
		sprintf(tmp,"%d",(int)file->info->st_nlink); 
		if(max[colindex] < strlen(tmp))
			max[colindex] = strlen(tmp);
		colindex++;
		if(max[colindex] < strlen(uid_to_name(file->info->st_uid)))
			max[colindex] = strlen(uid_to_name(file->info->st_uid));	
		colindex++;
		if(max[colindex] < strlen(gid_to_name(file->info->st_gid)))
			max[colindex] = strlen(gid_to_name(file->info->st_gid));	
		colindex++;
		sprintf(tmp,"%ld",(long)file->info->st_size); 
		if(max[colindex] < strlen(tmp))
			max[colindex] = strlen(tmp);
		colindex++;
		if(max[colindex] < strlen(file->name))
			max[colindex] = strlen(file->name);
		colindex++;
		p= p->next;
	}

}




void printLS(MyList *list, char dirname[])
{


	if(sort_method == S_DEFULT)
		myListQuickSort(list, sortMethod_DEFULT);
	else if(sort_method == S_S)
		myListQuickSort(list, sortMethod_S);
	else if(sort_method == S_t)
		myListQuickSort(list, sortMethod_t);
	if(LS_r == 1)
		myListReverse(list);
	if(LS_R ==1)
	{
		if(indexdols1 != 1) puts("");
		printf("%s:\n",dirname);
	}
	if(file_length == 1 || LS_R == 1 )
	{
		if( LS_l == 1 )
		{
			alllength(list);
			totalbloacks(list);
			myListOutput(list, printInfo);
		}
		else 
		{
			//	myListOutput(list, printData);
			//	puts("");
			print_many_per_line(list);
		}
	}
	else 
	{
		if(LS_l == 1)
		{
			if(indexdols1 != 1) puts("");
			printf("%s:\n",dirname);
			alllength(list);
			totalbloacks(list);
			myListOutput(list, printInfo);
			//	puts("");
		}
		else
		{
			if(indexdols1 != 1) puts("");
			printf("%s:\n",dirname);
			//	myListOutput(list, printData);
			//	puts("");
			print_many_per_line(list);
		}
	}

}


void printLS_d(char *dirname)
{
	if(LS_d == 1)
		printf("%s ",dirname);
	if(indexdols1 == file_length)
		puts("");

}


int findLS(void *p)
{
	FileInfo *file = (FileInfo *)p;
	if(file->name[0]!='.')
		return 1;
	else
		return 0;
}

int findLSA(void *p)
{
	FileInfo *file = (FileInfo *)p;
	if(strcmp(file->name,".") ==0 ||strcmp(file->name,"..")==0)
		return 0;
	else
		return 1;
}


void do_ls1(char dirname[], MyList *list)  
{  
	DIR* dir_ptr;  
	indexdols1++ ;

	if ((dir_ptr = opendir(dirname)) == NULL)  
	{  
		fprintf(stderr, "ls2: cannot open %s \n", dirname);  
	}  
	else  
	{    
		if(LS_d == 1)
		{
			printLS_d(dirname);
			closedir(dir_ptr);  
			return ;
		}
		list = createMyList();

		char *dirs[100];  
		int dir_count = 0;  

		struct dirent* direntp;
		while ((direntp = readdir(dir_ptr)) != NULL)  
		{  

			char complete_d_name[200];  // 文件的完整路径  
			strcpy (complete_d_name,dirname);  
			strcat (complete_d_name,"/");  
			strcat (complete_d_name,direntp->d_name);  

			struct stat *info = NULL;
			info = (struct stat *)malloc(sizeof(struct stat));
			if(info == NULL) return ;  
			if (lstat(complete_d_name, info) == -1)  
			{  
				perror(complete_d_name);  
			}  
			else  
			{  
				char *name = NULL;
				int n =strlen(direntp->d_name);	
				name = (char *)malloc(n+1);
				if(name)
				{

					strcpy(name, direntp->d_name);
					FileInfo *fileinfo = NULL;
					fileinfo = (FileInfo *)malloc(sizeof(FileInfo));
					if(fileinfo == NULL)
					{
						free(info);
						return ;
					}
					if(fileinfo )
					{
						fileinfo->name = name;
						fileinfo->info = info;
						myListInsertDataAtLast(list, fileinfo);	
					}	
				}

			}
			if( LS_R == 1 )	
			{

				if( (strcmp(direntp->d_name,".") == 0  || strcmp(direntp->d_name,"..") == 0))
					continue;
				if(LS_a == 0 && LS_A == 0)
				{
					if(direntp->d_name[0] == '.')
						continue;
				}
				if(S_ISDIR(info->st_mode))
				{
					dirs[dir_count] = (char *)malloc(strlen(complete_d_name)+1);
					strcpy(dirs[dir_count],complete_d_name);
					dir_count++;
				}
			}  
		}
		if(LS_a == 0)
		{
			if(LS_A == 1)
				list = myListFindDataAllNode(list, findLSA, freeData);
			else 
				list = myListFindDataAllNode(list, findLS, freeData);
		}
		printLS(list, dirname);
		freeMyList(list, freeData);

		if(dir_count != 0)
		{ 
			for(int i=0; i<dir_count; i++)
			{
				do_ls1(dirs[i],list);
				free(dirs[i]);
			}
		}

		closedir(dir_ptr);  
	}  
}  


int main(int argc, char * argv[])
{
	int opt;
	char *filename[100];
	for(int i=0; i<100; i++)
		filename[i] = NULL;
	for(int i=0; i<100; i++)
		max[i] = 0;
	int end = 0;
	//	while ((opt = getopt(argc, argv, "laARS")) != -1)
	while ((opt = getopt_long(argc, argv, "lahsARdrtS", longOpts, NULL)) != -1)
	{
		switch (opt) 
		{
			case 'a':
				LS_a = 1;
				break;													
			case 'A':
				LS_A = 1;	
				break;
			case 'l':
				LS_l = 1;
				break;
			case 'R':
				LS_R = 1;	
				break;	
			case 'S':
				sort_method = S_S;
				break;
			case 'h':
				help();
				end = 1;
				break;
			case 'r':
				LS_r = 1;
				break;
			case 't':
				sort_method = S_t;
				break;
			case 's':
				LS_s = 1;
				break;
			case 'd':
				LS_d = 1;
				break;
			case '?':
				printf("Unknown option: %c\n",(char)optopt);
				break;
		}

	}
	if(end == 1 )
		return 0;





	//保存文件名
	for(int i = optind; i < argc; i++)
	{
		char *p = NULL;
		p = (char *)malloc(strlen(argv[i])+1);
		if(p)
		{
			strcpy(p, argv[i]);
			filename[file_length++] = p;
		}
	}



	MyList *list = NULL;

	//创建链表
	if (file_length == 0 )
	{
		char *p = NULL;
		p = (char *)malloc(2);
		if(p)
		{
			strcpy(p, ".");
			filename[file_length++] = p;
			do_ls1(".", list);
		}
	}
	else 
	{
		for(int i = 0; i < file_length; i++)
			do_ls1(filename[i], list);
	}
	for(int i = 0; i < file_length; i++)
	{
		free(filename[i]);
	}
	for (int i = 0; i < max_idx; ++i)
	{
		free(column_info[i].col_arr) ;
	}
	free(column_info);
	return 0;
}

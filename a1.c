#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
int compare(char *s1,char *s2)
{
	if(!(strcmp(s1+ strlen(s1) - strlen(s2), s2) == 0))
		return 1;
	return 0;
}
void list(char *path, int recursiv, char *name_ends_with, int size)
{
	DIR *dir = opendir(path);
	struct dirent *entry;
	struct stat fileMetadata;
	char fullPath[512];

	if (path[strlen(path) - 1] != '/')
		strcat(path, "/");
	
	while ((entry = readdir(dir)) != 0)
	{
		if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
		{
			if (name_ends_with == NULL || compare(entry->d_name,name_ends_with)==0)
			{
				strcpy(fullPath, path);
				strcat(fullPath, entry->d_name);
				stat(fullPath, &fileMetadata);
				if (size == -1)
					printf("%s\n", fullPath);
				else if (fileMetadata.st_size < size && S_ISREG(fileMetadata.st_mode))
					printf("%s\n", fullPath);
				if (recursiv && S_ISDIR(fileMetadata.st_mode))
				{
					list(fullPath, recursiv, name_ends_with, size);
				}
			}
		}
	}
	closedir(dir);
}
void parse(int fd)
{
	char name[13];
	int type=0;
	int offset=0;
	int size=0;
	char magic[3];
	int header_size=0;
	int nrSectiuni=0;
	int version=0;

	lseek(fd, -4, SEEK_END);
	read(fd, &header_size, 2);

	read(fd, magic, 2);
	magic[2]='\0';
	if (strcmp(magic,"8u")!=0)
	{
		printf("ERROR\nwrong magic\n");
		return;
	}
	lseek(fd, -header_size, SEEK_CUR);
	read(fd, &version, 4);

	if (version<16 || version>81)
	{
		printf("ERROR\nwrong version\n");
		return;
	}

	read(fd, &nrSectiuni, 1);

	if (nrSectiuni <4 || nrSectiuni >17)
	{
		printf("ERROR\nwrong sect_nr\n");
		return;
	}

	for (int i = 0;i<nrSectiuni;i++)
	{
		read(fd, name, 12);
		read(fd, &type, 4);
		if (type != 76 && type != 81 && type != 80)
		{
			printf("ERROR\nwrong sect_types\n");
			return;
		}
		read(fd, &offset, 4);
		read(fd, &size, 4);
	}
	printf("SUCCESS\nversion=%d\nnr_sections=%d\n",version,nrSectiuni);

	read(fd, &header_size, 2);
	read(fd, magic, 2);

	lseek(fd, -header_size+5, SEEK_CUR);

	for (int i = 0;i<nrSectiuni;i++)
	{
		read(fd, name, 12);
		name[12]='\0';
		read(fd, &type, 4);
		read(fd, &offset, 4);
		read(fd, &size, 4);
		printf("section%d: %s %d %d\n", i+1 , name, type, size);
	}
}
void invert(char *line)
{
	int i,j=0,n=strlen(line);
	char *inv=malloc(n*sizeof(char));
	for(i=n-1;i>=0;i--)
	{
		inv[j]=line[i];
		j++;
	}
	inv[j]='\0';
	printf("%s\n",inv); 
	free(inv);
}
void extract( char *path, int section, int line)
{
	int fd=-1;
	int header_size=0;
	int offset=0;
	int sect=(section-1)*24;
	int sect_size=0;
	char linie[INT_MAX/1000];
	char aux[INT_MAX/1000];
	char c;

	fd= open(path,O_RDONLY);
	if(fd<0)
	{
		printf("ERROR\ninvalid path|section|line\n");
		return;
	}
	lseek(fd,-4,SEEK_END);
	read(fd,&header_size,2);

	lseek(fd,-header_size+21+sect,SEEK_END);

	read(fd,&offset,4);
	read(fd,&sect_size,4);
	lseek(fd,offset,SEEK_SET);

	printf("SUCCESS\n");

	int nrLinii=0;

	while(sect_size!=0)
	if(read(fd,&c,1))
	{
		if(c=='\r')
		{
			nrLinii++;
		}
		sect_size--;
	}
	nrLinii++;
	lseek(fd,offset,SEEK_SET);
	int i=0;
	while(read(fd,&c,1) && nrLinii!=line-1)
	{
		if(c!='\n')
		{
			linie[i]=c;
			i++;
		}
		else if(c=='\n')
		{
			linie[i]=0;
			strcpy(aux,linie);
			nrLinii--;
			i=0;
		}
	}
	invert(aux);
	close(fd);
}
int isSF(char* path)
{
	int type=0;
	int size=0;
	char magic[3];
	int header_size=0;
	int nrSectiuni=0;
	int version=0;
	int fd=0;
	if((fd=open(path, O_RDONLY))>=0){

	lseek(fd, -4, SEEK_END);
	read(fd, &header_size, 2);

	read(fd, magic, 2);
	magic[2]='\0';
	if (strcmp(magic,"8u")!=0)
	{
		close(fd);
		return 0;
	}
	lseek(fd, -header_size, SEEK_CUR);
	read(fd, &version, 4);
	if (version<16 || version>81)
	{
		close(fd);
		return 0;
	}
	read(fd, &nrSectiuni, 1);
	if (nrSectiuni <4 || nrSectiuni >17)
	{
		close(fd);
		return 0;
	}

	for (int i = 0;i<nrSectiuni;i++)
	{
		lseek(fd, 12, SEEK_CUR);
		read(fd, &type, 4);
		if (type != 76 && type != 81 && type != 80)
		{
			close(fd);
			return 0;
		}
		lseek(fd, 8, SEEK_CUR);
	}
	lseek(fd, -header_size+9, SEEK_CUR);

	for (int i = 0;i<nrSectiuni;i++)
	{
		lseek(fd, 20, SEEK_CUR);
		read(fd, &size, 4);
		if(size>1269){
			close(fd);
			return 0;
		}		
	}
	return 1;
	}
	else{
		printf("ERROR\ninvalid directory path\n");
		return 0;
	}

}
void findall(char *path)
{
	DIR *dir = NULL;
    struct dirent *entry = NULL;
    char fullPath[512];
    struct stat statbuf;

    dir = opendir(path);
    if(dir == NULL) {
        perror("ERROR\ninvalid directory path");
        return;
    }
    while((entry = readdir(dir)) != NULL) {
        if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            snprintf(fullPath, 512, "%s/%s", path, entry->d_name);
            if(lstat(fullPath, &statbuf) == 0) {
                if(S_ISDIR(statbuf.st_mode)) {
                    findall(fullPath);
                }else if(S_ISREG(statbuf.st_mode)){
					if(isSF(fullPath)==1)
						printf("%s\n",fullPath);
				}
            }
        }
    }
    closedir(dir);
}
int main(int argc, char **argv) {
	if (argc >= 2) {
		if (strstr(argv[1], "variant")) {
			printf("27208\n");
		}
		else if (strcmp(argv[1], "list") == 0) {
			int j = 2;
			int recursiv = 0;
			int size = -1;
			char *path = NULL;
			char *name_ends_with = NULL;

			while (j<argc) {
				if (strncmp(argv[j], "path=", 5) == 0)
				{
					path = argv[j] + 5;

				}
				else if (strcmp(argv[j], "recursive") == 0)
				{
					recursiv = 1;
				}
				else if (strncmp(argv[j], "size_smaller=", 13) == 0)
				{
					size = atoi(argv[j] + 13);

				}
				else if (strncmp(argv[j], "name_ends_with=", 15) == 0)
				{
					name_ends_with = argv[j] + 15;
				}
				j++;
			}
			struct stat fileMetadata;
			if (stat(path, &fileMetadata)<0 || !S_ISDIR(fileMetadata.st_mode))
			{
				printf("ERROR\ninvalid directory path\n");
			}
			else
			{
				printf("SUCCESS\n");
				list(path, recursiv, name_ends_with, size);
			}
		}
		else if (strcmp(argv[1], "parse") == 0)
		{
				int fd;
				char *path=NULL;
				if (strncmp(argv[2], "path=", 5) == 0)
				{	
					path = argv[2] + 5;
				}
				if ((fd = open(path, O_RDONLY)) >= 0)
				{
					parse(fd);
					close(fd);
				}
				else
				{
					printf("ERROR\ninvalid file path\n");
				}
		}
		else if(strcmp(argv[1],"extract")==0)
		{
			char *path = NULL;
			int section=0;
			int line = 0;
			int nrArg=2;
			while (nrArg<argc) {
				if (strncmp(argv[nrArg], "path=", 5) == 0)
				{
					path = argv[nrArg] + 5;
				}
				if (strncmp(argv[nrArg], "section=", 8) == 0)
				{
					section = atoi(argv[nrArg] + 8);
				}
				if (strncmp(argv[nrArg], "line=", 5) == 0)
				{
					line = atoi(argv[nrArg] + 5);
				}
				nrArg++;
			}
			extract(path,section,line);
		} else if (strcmp(argv[1], "findall") == 0) 
		{	
			char *path=NULL;
			if (strncmp(argv[2], "path=", 5) == 0)
			{	
				path = argv[2] + 5;
				printf("SUCCESS\n");
				findall(path);
			}
			else
			{
				printf("ERROR\ninvalid directory path\n");
			}
		}	
	}
	return 0;
}


			
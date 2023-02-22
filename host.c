#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include <sys/socket.h>
#include<sys/stat.h>
#include<fcntl.h>
#include <netdb.h>
#include<stddef.h>

#define BUFF_SIZE   65536
#define PORT        10401

int profile_data_nitems = 0;
int MAX_LEN = 70;

char send_message[BUFF_SIZE];
struct date { 
  int y; 
  int m;
  int d;
};

struct profile {
    int id;     
    char school[70];  
    struct date birth;    
    char place[70];  
    char *sub; 
};

struct profile profile_data_store[10000];

int func_max(int i, int j); 
int func_min(int i, int j);
int subst(char *str, char c1, char c2);
int split(char *str, char *ret[], char sep, int max);
int get_line(char *line, FILE *fp);
struct profile *new_profile(struct profile *profiles, char *line, int s);
void exec_command(char cmd1, char cmd2, char *param, int s);
void parse_line(char *line, int s);
void cmd_quit();
void cmd_check(int s);
void cmd_print(int param, int s);
void cmd_read(char *fn, int s);
void cmd_write(char *fn, int s);
void cmd_find(char *param, int s);
void cmd_sort(int param, int s);
void cmd_complex_print(char *param, int s);
void cmd_del(int i, int s);
void cmd_bin_write(char *fn, int s);
void cmd_bin_read(char *fn, int s);
void output_profile(int i, int s);
void save_profile(int i, FILE *fp);
int com_str(char *str, int i);
int comp_str(char *str1, char *str2, int cnt);
void swap_struct(int a, int b);
void quick_sort(int start, int end, int param);
int call_quick(int s, int e, int front, int back, int param);
int sum_date(int i);
void output_10to16(FILE *fp);
void read_bin(FILE *fp);

int main()
{
    int s, new_s,recvsize, sendsize;
    unsigned int len;
    char recv_message[BUFF_SIZE];
    struct sockaddr_in sa;

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == -1)
    {
        printf("error in socket\n");
        return -1;
    }
    
    sa.sin_family = AF_INET;
    sa.sin_port = htons(PORT);
    sa.sin_addr.s_addr = INADDR_ANY;

    bind(s, (struct sockaddr*)&sa, sizeof(sa));
    listen(s, 5);

    while(1)
    {
        len = sizeof(sa);
        new_s = accept(s, (struct sockaddr *)&sa, &len);
        recvsize = recv(new_s, recv_message, BUFF_SIZE, 0);
		while (recvsize > 0)
		{
   		    printf("recv=%s\n", recv_message);
       		parse_line(recv_message, new_s);
			sprintf(send_message, "EOF");
	    	sendsize = send(new_s, send_message, strlen(send_message)/sizeof(char)+1, 0);
            if (sendsize < 0)
            {
                break;
            }
            recvsize = recv(new_s, recv_message, BUFF_SIZE, 0);
		}
        close(new_s);
    }

    close(s);

    return 0;
}

int func_max(int i, int j) {
    if(i > j) {
        return i;
    }
    return j;
}

int func_min(int i, int j) {
    if(i < j) {
        return i;
    }
    return j;
}

int subst(char *str, char c1, char c2)
{
    int n = 0;
    for( ; *str != '\0'; str++) {
        if(*str == c1) {
            *str = c2;
            n++;
        }
    }
    return n;
}

int split(char *str, char *ret[], char sep, int max)
{
    int n;
    int c = subst(str, sep, '\0') + 1;
    if(c > max) {
        return c;   
    }
    for(n = 0; n < c;) {
        ret[n++] = str;  
        while(*str) {
            str++;
        }
        str++;
    }
    return n;
}

int get_line(char *line, FILE *fp)
{
    int max = 1024;
    if(fgets(line, max + 1, fp) == NULL) {
        return 0;
    }
    subst(line, '\n', '\0');
    return 1;
}

struct profile *new_profile(struct profile *profiles, char *line, int s) 
{
    int i, n, j;
    char *ret[5];
    char *birth[3];
    if (split(line, ret, ',', 5) != 5) {
		sprintf(send_message, "Please input 5 elements\n");
	    send(s, send_message, strlen(send_message)/sizeof(char)+1, 0);
        return profiles;
    }
    i = split(ret[2], birth, '-', 3);
    if(i != 3) {
		sprintf(send_message, "Please input element 3 int-int-int\n");
	    send(s, send_message, strlen(send_message)/sizeof(char)+1, 0);
        return profiles;
    }

    for(i = 0; i < 3; i++) {
        n = atoi(birth[i]);
        for(j = 0;;) {
            if(n < 10) {
                break;
            }
            n = n / 10;
            j++;
        }
        j++;
        if((atoi(birth[i]) == 0) || (strlen(birth[i]) != j)) {
			sprintf(send_message, "Please input element 3 int-int-int\n");
		    send(s, send_message, strlen(send_message)/sizeof(char)+1, 0);
            return profiles;
        }
    }
    
    profiles->id = atoi(ret[0]);
    strcpy(profiles->school, ret[1]);
    profiles->birth.y = atoi(birth[0]);
    profiles->birth.m = atoi(birth[1]);
    profiles->birth.d = atoi(birth[2]);
    strcpy(profiles->place, ret[3]);
    (profiles->sub) = (char *)malloc(sizeof(char) * (strlen(ret[4])) + 1);
    if(strlen(ret[4]) > MAX_LEN) {
        MAX_LEN = strlen(ret[4]);
    }
    strcpy(profiles->sub, ret[4]);
    profile_data_nitems++;
    return profiles;
}

void exec_command(char cmd1, char cmd2, char *param, int s)
{
    switch(cmd1) {
    case 'Q':
        cmd_quit(s);
        break;
    case 'C':
        switch(cmd2) {
        case 'P':
            cmd_complex_print(param, s);
            break;
        default:
            cmd_check(s);
        }
        break;
    case 'P':
        cmd_print(atoi(param), s);
        break;
    case 'R':
        cmd_read(param, s);
        break;
    case 'W':
        cmd_write(param, s);
        break;
    case 'F':
        cmd_find(param, s);
        break;
    case 'S':
        cmd_sort(atoi(param), s);
        break;
    case 'D':
        cmd_del(atoi(param), s);
        break;
    case 'B':
        switch(cmd2) {
        case 'W':
            cmd_bin_write(param, s);
            break;
        case 'R':
            cmd_bin_read(param, s);
            break;
        }
        break;
    default:
		sprintf(send_message, "%%%c%c command is not defined.\n", cmd1, cmd2);
        send(s, send_message, strlen(send_message)/sizeof(char)+1, 0);
    }
    return;
}

void parse_line(char *line, int s)
{
    if(*line == '%') {
        exec_command(line[1], line[2], &line[3], s);
    } else {
        new_profile(&profile_data_store[profile_data_nitems], line, s);
    }
    return;
}

void cmd_quit(int s)
{
    int i = 0;
    for (;i < profile_data_nitems;i++) {
        free(profile_data_store[i + 1].sub);
    }
    profile_data_nitems = 0;

    close(s);
    return;
}

void cmd_check(int s)
{
    
    if(profile_data_nitems != 1) {
        sprintf(send_message, "%d profiles\n", profile_data_nitems);
    } else {
        sprintf(send_message, "%d profile\n",  profile_data_nitems);
    }
    send(s, send_message, strlen(send_message)/sizeof(char)+1, 0);
    return;
}

void cmd_print(int param, int s)
{
    int i = 0;
    if(profile_data_nitems == 0) {
		sprintf(send_message, "Datas are not registered.\n");
        send(s, send_message, strlen(send_message)/sizeof(char)+1, 0);
        return;
    }
    if ((param > profile_data_nitems) || (param < (-1) * profile_data_nitems)) {
		sprintf(send_message, "This program has only %d datas now.\n\n", profile_data_nitems);
        send(s, send_message, strlen(send_message)/sizeof(char)+1, 0);
        param = 0;
    }
    if(param == 0) {
        param = profile_data_nitems;
    } else if(param < 0) {
        i = profile_data_nitems + param;
        param = profile_data_nitems;
    }
    for(; i < param; i++) {
        output_profile(i, s);
    }
    return;
}

void cmd_read(char *fn, int s)
{
    FILE *fp;
    char data[65536];
	
    if((fp = fopen(fn, "r")) == NULL) {
		sprintf(send_message, "ERROR has occured.\n");
        send(s, send_message, strlen(send_message)/sizeof(char)+1, 0);
        return;
    }
	
    while(get_line(data, fp)) {
        parse_line(data, s);
    }
	sprintf(send_message, "read\n");
    send(s, send_message, strlen(send_message)/sizeof(char)+1, 0);
    fclose(fp);
    return;
}

void cmd_write(char *fn, int s)
{
    FILE *fp;
    int i = 0;
	
    if(profile_data_nitems <= 0) {
		sprintf(send_message, "This program has no datas.");
        send(s, send_message, strlen(send_message)/sizeof(char)+1, 0);
        return;
    }
    if(comp_str(fn, "\0", 0) == 1) {
        strcpy(fn, "output.csv");
    }
    if((fp = fopen(fn, "w")) == NULL) {
		sprintf(send_message, "ERROR has occured.\n");
        send(s, send_message, strlen(send_message)/sizeof(char)+1, 0);
        return;
    }
    for(;i < profile_data_nitems; i++) {
        save_profile(i, fp);
    }
	sprintf(send_message, "Saved : %s\n", fn);
    send(s, send_message, strlen(send_message)/sizeof(char)+1, 0);
    fclose(fp);
    return;
}

void cmd_find(char *param, int s)
{
    int cnta, i = 0;
	
    for(cnta = 0; cnta < profile_data_nitems; cnta++) {
        if(com_str(param, cnta)) {
            output_profile(cnta, s);
            i++;
        }
    }
    if(i == 0) {
		sprintf(send_message, "None");
    	send(s, send_message, strlen(send_message)/sizeof(char)+1, 0);
    }
    return;
}

void cmd_sort(int param, int s)
{
	
    if(profile_data_nitems <= 0) {
		sprintf(send_message, "This program has no datas.");
        send(s, send_message, strlen(send_message)/sizeof(char)+1, 0);
        return;
    }
    quick_sort(0, profile_data_nitems - 1, param);
    return;
}

void cmd_complex_print(char *param, int s)
{
    int step, i, j;
    char *parameter[2];
	
    if(split(param, parameter, ',', 2) != 2) {
		sprintf(send_message, "Please Input 2 elements\n");
	    send(s, send_message, strlen(send_message)/sizeof(char)+1, 0);
        return;
    }
    i = atoi(parameter[0]) - 1;
    j = atoi(parameter[1]) - 1;
    if(i <= j) {
        step = 1;
        i = func_max(0, i);
        j = func_min(j, profile_data_nitems);
    } else {
        step = -1;
        j = func_max(0, j);
        i = func_min(i, profile_data_nitems);
    }
    while(i != j) {
        output_profile(i, s);
        i += step;
    }
    output_profile(i, s);
    return;
}

void cmd_del(int i, int s)
{
    if((0 >= i) || (i > profile_data_nitems)) {
		sprintf(send_message, "This program has only %d elements.\n", profile_data_nitems);
	    send(s, send_message, strlen(send_message)/sizeof(char)+1, 0);
        return;
    }
    i--;
    while(i < profile_data_nitems) {
        profile_data_store[i] = profile_data_store[i + 1];
        free(profile_data_store[i + 1].sub);
        i++;
    }
    profile_data_nitems--;
    return;
}

void cmd_bin_write(char *fn, int s)
{
    FILE *fp;
    if(profile_data_nitems <= 0)
    {
		sprintf(send_message, "This program has no datas.\n");
	    send(s, send_message, strlen(send_message)/sizeof(char)+1, 0);
        return;
    }
    if(comp_str(fn, "\0", 0) == 1) {
        strcpy(fn, "bin_output.dat");
    }
    if((fp = fopen(fn, "wb")) == NULL) {
		sprintf(send_message, "ERROR has occured.\n");
	    send(s, send_message, strlen(send_message)/sizeof(char)+1, 0);
        return;
    }
    output_10to16(fp);
	sprintf(send_message, "Saved : %s\n", fn);
    send(s, send_message, strlen(send_message)/sizeof(char)+1, 0);
    fclose(fp);
    return;
}

void cmd_bin_read(char *fn, int s)
{
    FILE *fp;
    if((fp = fopen(fn, "rb")) == NULL) {
		sprintf(send_message, "ERROR has occured.\n");
	    send(s, send_message, strlen(send_message)/sizeof(char)+1, 0);
        return;
    }
    sprintf(send_message, "read : %s\n", fn);
    send(s, send_message, strlen(send_message)/sizeof(char)+1, 0);
    read_bin(fp);
    fclose(fp);
    return;
}

void output_profile(int i, int s)
{
	sprintf(send_message, "Id    : %d\nName  : %s\nBirth : %02d-%02d-%02d\nAddr  : %s\nCom.  : %s\n\n", profile_data_store[i].id, profile_data_store[i].school, profile_data_store[i].birth.y, profile_data_store[i].birth.m, profile_data_store[i].birth.d, profile_data_store[i].place, profile_data_store[i].sub);
    send(s, send_message, strlen(send_message)/sizeof(char)+1, 0);
    return;
}

int com_str(char *str, int i)
{
    int cnt;
    char int_str[32 + 1];
    for(cnt = 0; profile_data_store[i].school[cnt] != '\0'; cnt++)
    {
        if(comp_str(str, profile_data_store[i].school, cnt) <= 1)
        {
            return 1;
        }
    }
    for(cnt = 0; profile_data_store[i].place[cnt] != '\0'; cnt++)
    {
        if(comp_str(str, profile_data_store[i].place, cnt) <= 1)
        {
            return 1;
        }
    }
    for(cnt = 0; profile_data_store[i].sub[cnt] != '\0'; cnt++)
    {
        if(comp_str(str, profile_data_store[i].sub, cnt) <= 1)
        {
            return 1;
        }
    }
    sprintf(int_str, "%d", profile_data_store[i].id);
    for(cnt = 0; int_str[cnt] != '\0'; cnt++)
    {
        if(comp_str(str, int_str, cnt) <= 1)
        {
            return 1;
        }
    }
    sprintf(int_str, "%02d-%02d-%02d", profile_data_store[i].birth.y, profile_data_store[i].birth.m, profile_data_store[i].birth.d);
    for(cnt = 0; int_str[cnt] != '\0'; cnt++)
    {
        if(comp_str(str, int_str, cnt) <= 1)
        {
            return 1;
        }
    }
    sprintf(int_str, "%d-%d-%d", profile_data_store[i].birth.y, profile_data_store[i].birth.m, profile_data_store[i].birth.d);
    for(cnt = 0; int_str[cnt] != '\0'; cnt++)
    {
        if(comp_str(str, int_str, cnt) <= 1)
        {
            return 1;
        }
    }
    return 0;
}

int comp_str(char *str1, char *str2, int cnt)
{
    int cnta, cmp;
    for(cnta = cnt, cmp = 0;; cnta++)
    {
        cmp = 0;
        if(str1[cnta - cnt] == '\0') {
            if(str2[cnta] == '\0') {
                cmp = 1;
            }
            break;
        }
        if (str2[cnta] == '\0') {
            cmp = 3;
            break;
        }
        if(str2[cnta] < str1[cnta - cnt]) {
            cmp = 3;
            break;
            }
        if(str2[cnta] != str1[cnta - cnt]) {
            cmp = 2;
            break;
        }
    }
    return cmp;
}

void swap_struct(int a, int b)
{
    struct profile *A, *B, c;
    A = &profile_data_store[a];
    B = &profile_data_store[b];
    c = *A;
    *A = *B;
    *B = c;
    return;
}

void quick_sort(int s, int e, int param)
{
    if(s == e) {
        return;
    }
    int base = s + (e - s) / 2, front = s, back = e;
    switch(param) {
        case 1:
            while(1)
            {
                for(; (e > front) && (profile_data_store[front].id < profile_data_store[base].id); front++);
                for(; (s < back) && (profile_data_store[back].id >= profile_data_store[base].id); back--);
                if(call_quick(s, e, front, back, param))
                {
                    break;
                }
            }
            break;

    case 2:
        while(1)
        {
            for(front = s; (front < back) && ((comp_str(profile_data_store[front].school, profile_data_store[base].school, 0) == 2) || (comp_str(profile_data_store[front].school, profile_data_store[base].school, 0) == 0)); front++);
            for(back = e; (front < back) && ((comp_str(profile_data_store[back].school, profile_data_store[base].school, 0) == 1) || (comp_str(profile_data_store[back].school, profile_data_store[base].school, 0) == 3)); back--);
            if(call_quick(s, e, front, back, param))
            {
                break;
            }
        }
        break;

    case 3:
        while(1)
        {
            for(front = s; (front < back) && (sum_date(front) < sum_date(base)); front++);
            for(back = e; (back > front) && (sum_date(back) >= sum_date(base)); back--);
            if(call_quick(s, e, front, back, param))
            {
                break;
            }
        }
        break;

    case 4:
        while(1)
        {
            for(front = s; (front < back) && ((comp_str(profile_data_store[front].place, profile_data_store[base].place, 0) == 2) || (comp_str(profile_data_store[front].place, profile_data_store[base].place, 0) == 0)); front++);
            for(back = e; (front < back) && ((comp_str(profile_data_store[back].place, profile_data_store[base].place, 0) == 1) || (comp_str(profile_data_store[back].place, profile_data_store[base].place, 0) == 3)); back--);
            if(call_quick(s, e, front, back, param))
            {
                break;
            }
        }
        break;

    case 5:
        while(1)
        {
            for(front = s; (front < back) && ((comp_str(profile_data_store[front].sub, profile_data_store[base].sub, 0) == 2) || (comp_str(profile_data_store[front].sub, profile_data_store[base].sub, 0) == 0)); front++);
            for(back = e; (front < back) && ((comp_str(profile_data_store[back].sub, profile_data_store[base].sub, 0) == 1) || (comp_str(profile_data_store[back].sub, profile_data_store[base].sub, 0) == 3)); back--);
           if(call_quick(s, e, front, back, param))
            {
                break;
            }
        }
        break;
    default:
        fprintf(stderr, "This element is undefined.");
    }
    return;
}

int call_quick(int s, int e, int front, int back, int param)
{
    if(back <= front) {
        swap_struct(front, s + (e - s) / 2);
        quick_sort(s, front, param);
        quick_sort(front + 1, e, param);
        return 1;
    } else {
        swap_struct(front, back);
    }
    return 0;
}

int sum_date(int i)
{
    return (profile_data_store[i].birth.y) * 10000 + (profile_data_store[i].birth.m) * 100 + (profile_data_store[i].birth.d);
}

void save_profile(int i, FILE *fp)
{
    fprintf(fp, "%d,%s,%d-%d-%d,%s,%s\n", profile_data_store[i].id, profile_data_store[i].school, profile_data_store[i].birth.y, profile_data_store[i].birth.m, profile_data_store[i].birth.d, profile_data_store[i].place, profile_data_store[i].sub);
    return;
}

void output_10to16(FILE *fp)
{
    int i, struct_size, sum;
    for(i = 0; i < profile_data_nitems; i++)
    {
        struct_size = sizeof(profile_data_store[i].id);
        fwrite(&struct_size, sizeof(int), 1, fp);
        fwrite(&profile_data_store[i].id, struct_size, 1, fp);
        struct_size = strlen(profile_data_store[i].school)+1;
        fwrite(&struct_size, sizeof(int), 1, fp);
        fwrite(&profile_data_store[i].school, struct_size, 1, fp);
        sum = sum_date(i);
        struct_size = sizeof(sum);
        fwrite(&struct_size, sizeof(int), 1, fp);
        fwrite(&sum, struct_size, 1, fp);
        struct_size = strlen(profile_data_store[i].place) + 1;
        fwrite(&struct_size, sizeof(int), 1, fp);
        fwrite(&profile_data_store[i].place, struct_size, 1, fp);
        struct_size = strlen(profile_data_store[i].sub) + 1;
        fwrite(&struct_size, sizeof(int), 1, fp);
        fwrite(profile_data_store[i].sub, struct_size, 1, fp);
    }
    return;
}

void read_bin(FILE *fp)
{
    int struct_size, sum;
    while(fread(&struct_size, sizeof(int), 1, fp))
    {
        fread(&profile_data_store[profile_data_nitems].id, struct_size, 1, fp);

        fread(&struct_size, struct_size, 1, fp);
        fread(profile_data_store[profile_data_nitems].school, sizeof(char), struct_size / sizeof(char) , fp);
        struct_size = 0;
        fread(&struct_size, sizeof(int), 1, fp);
        fread(&sum, struct_size, 1, fp);
        profile_data_store[profile_data_nitems].birth.y = sum / 10000;
        profile_data_store[profile_data_nitems].birth.m = (sum%10000)/100;
        profile_data_store[profile_data_nitems].birth.d = (sum%10000)%100;

        fread(&struct_size, sizeof(int), 1, fp);
        fread(profile_data_store[profile_data_nitems].place, sizeof(char), struct_size / sizeof(char), fp);
        fread(&struct_size, sizeof(int), 1, fp);
        (profile_data_store[profile_data_nitems].sub) = (char *)malloc(struct_size * sizeof(char) + 1);
        if(struct_size > MAX_LEN) 
        {
            MAX_LEN = struct_size;
        }
        fread(profile_data_store[profile_data_nitems].sub, sizeof(char), struct_size / sizeof(char), fp);
        profile_data_nitems++;
    }
    return;
}

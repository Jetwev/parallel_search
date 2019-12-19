#include<iostream>
#include<stdio.h>
#include<dirent.h>
#include<vector>
#include<string>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<pthread.h>
#include<deque>

struct thr{
    pthread_t thread;
    std::string str_t;
    std::string path;
    int end = 0;
    pthread_mutex_t* m;
    int* pi;
    std::deque<std::string>* q;
};

void listdir(std::string name,int strict_g, std::vector<thr>& threads, int N, std::deque<std::string>*, pthread_mutex_t*);
void *findstr(void* arg);
void PiArray(std::string, int*);
int KMPSearch(struct thr*, std::string, int);

int main(int argc, char** argv){
    std::string direct = ".", str, t;
    int tt = 1;//кол-во потоков
    int strict = 0;
    int end = 0;
    std::deque<std::string> q;
    pthread_mutex_t m;
    pthread_mutex_init(&m, NULL);
    for(int i = 1; i < argc; i++){
        if (argv[i][0] == '$')
            direct = argv[i] + 1;
        else{
            if (strstr(argv[i], "-n"))
                strict = 1;
            else{
                if(strstr(argv[i], "-t")){
                    t = argv[i];
                    tt = atoi(t.c_str()+2);
                }
                else{
                    str += argv[i];
                    str += " ";
                }
            }
        }
    }
    if (str.size() != 0)
        str.erase(str.end()-1);
    else{
        std::cout<< "Empty string"<< std::endl;
        return 0;
    }
    std::vector<thr> threads(tt);
    int pi[str.size()];
    PiArray(str, pi);
    for (int i = 0; i < tt; i++){
        threads[i].str_t = str;
        threads[i].pi = pi;
        threads[i].m = &m;
        threads[i].q = &q;
    }
       
    for( int i = 0; i < tt;i ++)
        pthread_create(&threads[i].thread, NULL, findstr,&threads[i]);
    listdir(direct,strict,threads, tt, &q, &m);
    for(int i = 0; i < tt; i++)
        threads[i].end = 2;
    for(int i = 0; i < tt; i++){
        pthread_join(threads[i].thread, NULL);
    }
    return 0;
}

void PiArray(std::string pat, int* pi)//формирование массива pref_suf
{ 
    int j = 0; 
    pi[0] = 0; 
    int i = 1; 
    while (i < pat.size()) { 
        if (pat[i] == pat[j]) { 
            j++; 
            pi[i] = j; 
            i++; 
        } 
        else
            if (j != 0)
                j = pi[j - 1];
            else
            { 
                pi[i] = 0; 
                i++; 
            } 
    } 
}

void listdir(std::string name,int strict_g, std::vector<thr>& threads, int N, std::deque<std::string>* q, pthread_mutex_t* m)
{
    DIR *dir;
    struct dirent *entry;
    int i = 0;

    if (!(dir = opendir(name.c_str())))
        return;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR) {
            std::string path;
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            path += name;
            path += '/';
            path += entry->d_name;
            if (strict_g == 0)
                listdir(path, strict_g, threads, N, q, m);
        } else {
            if (entry->d_type == DT_REG){
                std::string p;
                p = name;
                p += "/";
                p += entry->d_name;
                pthread_mutex_lock(m);
                (*q).push_front(p);
                pthread_mutex_unlock(m);
            }
        }
    }
    closedir(dir);
}

void *findstr(void* arg){
    FILE* fp;
    char buf[1025];
    int number = 0, flag = 0;
    while((*(struct thr*)arg).end != 2 || !(*(*(struct thr*)arg).q).empty()){
        pthread_mutex_lock((*(struct thr*)arg).m);
        if (!(*(*(struct thr*)arg).q).empty()){
            flag = 1;
            (*(struct thr*)arg).path = (*(*(struct thr*)arg).q).back();
            (*(*(struct thr*)arg).q).pop_back(); 
        }
        pthread_mutex_unlock((*(struct thr*)arg).m);
        if(flag == 1 && (fp = fopen((*(struct thr*)arg).path.c_str(), "r"))){
            while (fgets(buf, 1024, fp) != NULL){
                    if (strstr(buf, "\n")){
                        number++;
                    }
                    //kmp алгоритм
                    if(KMPSearch((struct thr*)arg, buf, number))
                        break;
                }
                number = 0;
                fclose(fp);
            }
        flag = 0;
    }
    
}

int KMPSearch(struct thr* tr, std::string str, int number) //поиск образа в строке
{ 
    int M = (*tr).str_t.size(); 
    int N = str.size();
    int i = 0; // индекс str
    int j = 0; // индекс pat
    while (i < N) { 
        if ((*tr).str_t[j] == str[i]) { 
            j++; 
            i++; 
        } 
        if (j == M) { 
            //образ найден, вывод
            printf("%s\n%d\n%s\n", (*tr).path.c_str(), number, str.c_str());
            printf("\n");
            return 1;
        } 
        else if (i < N && (*tr).str_t[j] != str[i]) {
            if (j != 0) 
                j = (*tr).pi[j - 1]; 
            else
                i = i + 1; 
        } 
    }
    return 0;
}
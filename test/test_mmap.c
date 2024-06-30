#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> //for using sleep()
#include <sys/mman.h>  //for using mmap()

int main(void){
  //Propaties
  int a;
  int b = 1;
  int c = 2;
  int processId;

  int* ptr;

  //Function Declaration
  void parent_process (int* ptr, int a, int b, int c);
  void child_process (int* ptr, int a, int b, int c);

  //プロセス分割前にmapして別領域に保存領域を作成
  ptr = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  //中身をゼロに
  *ptr = 0;
  //aをptrの中身(0)にする。
  a = *ptr;

  //forkでプロセス分割
  if ( ( processId = fork() ) == 0 ){
    //子プロセスの実行
      child_process(ptr,a,b,c);
  }else{
    //親プロセスの実行
      parent_process(ptr,a,b,c);
  }
}


void parent_process(int* ptr, int a, int b, int c){
    while (1) {
        a = *ptr;  //mmapで作った共有領域から値の読み込み
        printf("-parent-\n");
        printf("a = %d\n",a);  //この値のみ変更が反映される
        printf("b = %d\n",b);
        printf("c = %d\n",c);
        sleep(1);
    }
}


void child_process(int* ptr, int a, int b, int c){
    while (1) {
        a++;  //インクリメント
        *ptr = a;  //ptrが指し示す領域(mmapで作った共有領域)に反映
        b++;  //親プロセスに反映されない
        c += 10;  //親プロセスに反映されない
        sleep(2);
    }
}
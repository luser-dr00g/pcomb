#include "pc5.h"

int testre(){
  parser p = regex("[^b-z]+a");
  print_results( parse( p, "aaa" ) ), printf("\n");
  printf( "%d\n", max_result( parse( p, "aa" ) ) );
}

int main(){
  testre();
}

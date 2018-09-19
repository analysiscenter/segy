#include<stdio.h>
#include <cstdlib> 
#include <string.h>
 
using namespace std;

short put_i2(short c_data)
{
  short swapped_value;
  swapped_value = c_data << 8;
  c_data = (c_data >> 8) & 0xFF;
  swapped_value |= c_data;
 
  return swapped_value;
}
 
int swap_x (int c_data)
{
  int swapped_value;
  swapped_value = c_data << 24;
  swapped_value |= (c_data <<  8) & 0xFF0000;
  swapped_value |= (c_data >>  8) & 0xFF00;
  swapped_value |= (c_data >> 24) & 0xFF;
  return swapped_value;
}
 
int main (void)
{
  char outf[25];
  float trace_number;
  long offset,size_file;
  int i,j;
  short int m;//Number of Samples
  int *X,*Y,tmp; //Massives coordinate
  FILE *FL, *FLout;//
  FL = fopen ("c:/projects/gpn/segy/data/1.SGY", "rb");
  if (FL==NULL)
  {
    perror("Error opening data file");
    exit (-1);
  }
  fseek (FL,0,SEEK_END);
  printf("\n\tFile size %ld byte\n", ftell(FL));
  size_file = ftell(FL);
  
  fseek (FL,3216L,SEEK_SET);
  fread (&m,2,1,FL);    
  m = put_i2(m);
  printf ("\tNumber of Samples =%hd\n",m);
  offset = (long) m * 4;
  
  trace_number = (size_file - 3600)/(m*4+240);
  printf ("\tTrace number=%.2f\n",trace_number);
  
  fseek (FL,3600L,SEEK_SET);
  
  X = (int *) malloc (trace_number * sizeof (int));
  Y = (int *) malloc (trace_number * sizeof (int));
  
  printf ("\n\tSCAN COORDINATE\n\n");
  
  for (i=0;i<trace_number;i++)
  {  
    fseek (FL,80L,SEEK_CUR);
    fread (&X[i],4,1,FL);
    X[i] = swap_x (X[i]);
    printf ("%d\tX=%d",i+1,X[i]);
 
  
    fread (&Y[i],4,1,FL);  
 
    Y[i] = swap_x(Y[i]);
    printf ("\tY=%d\n",Y[i]);
  
    fseek (FL,152L,SEEK_CUR);
 
    fseek (FL,offset,SEEK_CUR);
  }
  
  printf ("\n\tSORT COORDINATE\n\n");
  
  for (j=0;j<trace_number;j++)
  {  
    for (i=0;i<trace_number-1;i++)
    {  
      if (X[i] > X[i+1])
      {  
        tmp=X[i+1];
        X[i+1]=X[i];
        X[i]=tmp;
      }  
      if (X[i]==X[i+1])
      {
        if (Y[i] > Y[i+1])
        {
          tmp=Y[i+1];
          Y[i+1]=Y[i];
          Y[i]=tmp;
        }  
      }
    }
  }  
  for (i=0;i<trace_number;i++)
  {
    printf ("%d\tX=%d",i+1,X[i]);
    printf ("\tY=%d\n",Y[i]);
  }
  i=0;
  while (i!=1|| i!=2)
  {  
    printf ("Do you whant save information?\nYes-1 or No-2\n");
    scanf ("%d",&i);
    if (i==1)
    {
      getchar();
      printf ("What is the name of output file: ");
      fgets (outf, 20, stdin);
      outf[strlen(outf)-1]='\0';
      FLout = fopen (outf, "wt");
      if (FLout == NULL)
      {
        printf("XAXA\n");
      }  
      for(i=0;i<trace_number;i++)
      {    
        fprintf(FLout,"%d\t",X[i]);
        fprintf(FLout,"%d\n",Y[i]);
      }
      fclose(FLout);
      return (0);
    }  
    else
      return (0);
  }  
  free (X);
  free (Y);
}
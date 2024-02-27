#include "stdint.h"
#include "string.h"
#include "stdio.h"
#include <stdlib.h>

#define LINEBUFF_SIZE 128
#define CMDTABLE_SIZE 8

uint32_t help(uint32_t para1, uint32_t para2);
uint32_t set_freq_base(uint32_t para1, uint32_t para2);
uint32_t set_freq_end(uint32_t para1, uint32_t para2);
uint32_t set_points(uint32_t para1, uint32_t para2);
uint32_t log_or_lin(uint32_t para1, uint32_t para2);
uint32_t voltage_shift(uint32_t para1, uint32_t para2);
uint32_t command_start_measurement(uint32_t para1, uint32_t para2);
uint32_t command_stop_measurement(uint32_t para1, uint32_t para2);

struct __uartcmd_table
{
  void *pObj;
  const char *cmd_name;
  const char *pDesc;
}uart_cmd_table[CMDTABLE_SIZE]=
{
  {(void*)help, "help", "print supported commands"},
  {(void*)command_start_measurement, "start", "start selected application"},
  {(void*)command_stop_measurement, "stop", "stop selected application"},
  {(void*)set_freq_base, "setbase", "Set start freq"},
  {(void*)set_freq_end, "setend", "Set end freq"},
  {(void*)set_points, "points", "Set number of points"},
  {(void*)log_or_lin, "character", "Choose linear or logarithmic characteristic"},
  {(void*)voltage_shift, "volt", "How much to shift characteristic"},
};


uint32_t help(uint32_t para1, uint32_t para2)
{
  int i = 0;
  printf("*****help menu*****\nbelow are supported commands:\n");
  for(;i<CMDTABLE_SIZE;i++)
  {
    if(uart_cmd_table[i].pObj)
      printf("%-8s --\t%s\n", uart_cmd_table[i].cmd_name, uart_cmd_table[i].pDesc);
  }
  printf("***table end***\n");
  return 0x87654321;
}


char line_buffer[LINEBUFF_SIZE];
uint32_t line_buffer_index = 0;
uint32_t token_count = 0;
void *pObjFound = 0;
uint32_t parameter1, parameter2;

void UARTCmd_RemoveSpaces(void)
{
  int i = 0;
  token_count = 0;
  char flag_found_token = 0;
  while(i<line_buffer_index)
  {
    if(line_buffer[i] == ' ') line_buffer[i] = '\0';
    else break;
    i++;
  }
  if(i == line_buffer_index) return;  /* All spaces... */
  while(i<line_buffer_index)
  {
    if(line_buffer[i] == ' ')
    {
      line_buffer[i] = '\0';
      flag_found_token = 0;
    }
    else
    { 
      if(flag_found_token == 0)
        token_count ++;
      flag_found_token = 1;
    }
    i++;
  }
}

void UARTCmd_MatchCommand(void)
{
  char *pcmd;
  int i = 0;
  pObjFound = 0;
  while(i<line_buffer_index)
  {
    if(line_buffer[i] != '\0')
    {
      pcmd = &line_buffer[i];
      break;
    }
    i++;
  }
  for(i=0;i<CMDTABLE_SIZE;i++)
  {
    if(strcmp(uart_cmd_table[i].cmd_name, pcmd) == 0)
    {
      /* Found you! */
      pObjFound = uart_cmd_table[i].pObj;
      break;
    }
  }
}

/* Translate string 'p' to number, store results in 'Res', return error code */
static uint32_t Str2Num(char *s, uint32_t *Res)
{
  char *p;
  unsigned int base=10;
   
  *Res = strtoul( s, &p, base );

  return 0;
}

void UARTCmd_TranslateParas(void)
{
  char *p = line_buffer;
  parameter1 = 0;
  parameter2 = 0;
  while(*p == '\0') p++;    /* goto command */
  while(*p != '\0') p++;    /* skip command. */
  while(*p == '\0') p++;    /* goto first parameter */
  if(Str2Num(p, &parameter1) != 0) return;
  if(token_count == 2) return;           /* Only one parameter */
  while(*p != '\0') p++;    /* skip first command. */
  while(*p == '\0') p++;    /* goto second parameter */
  Str2Num(p, &parameter2);
}

void UARTCmd_Process(char c)
{
  if(line_buffer_index >= LINEBUFF_SIZE-1)
    line_buffer_index = 0;  /* Error: buffer overflow */
  if( (c == '\r') || (c == '\n'))
  {
    uint32_t res;
    line_buffer[line_buffer_index] = '\0';
    /* Start to process command */
    if(line_buffer_index == 0) 
    {
      line_buffer_index = 0; /* Reset buffer */
      return;  /* No command inputs, return */
    }
    /* Step1, remove space */
    UARTCmd_RemoveSpaces();
    if(token_count == 0)
    {
      line_buffer_index = 0; /* Reset buffer */
      return; /* No valid input */
    }
    /* Step2, match commands */
    UARTCmd_MatchCommand();
    if(pObjFound == 0)
    {
      line_buffer_index = 0; /* Reset buffer */
      return;   /* Command not support */
    }
    if(token_count > 1)           /* There is parameters */
    {
      UARTCmd_TranslateParas();
    }
    /* Step3, call function */
    res = ((uint32_t (*)(uint32_t, uint32_t))(pObjFound))(parameter1, parameter2);
    printf("res:0x%08x\n", res);
    line_buffer_index = 0;  /* Reset buffer */
  }
  else
  {
    line_buffer[line_buffer_index++] = c;
  }
}
